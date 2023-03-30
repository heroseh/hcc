
#include "hcc_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#ifdef HCC_OS_LINUX
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <linux/futex.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#endif

#ifdef __GNUC__
#include <execinfo.h>
#endif

#ifdef HCC_OS_WINDOWS
#include <DbgHelp.h>
#include <Shlwapi.h>
#endif

// ===========================================
//
//
// General
//
//
// ===========================================

void _hcc_assert_failed(const char* cond, const char* file, int line, const char* message, ...) {
	fprintf(stderr, "assertion failed: %s\nmessage: ", cond);

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

_Noreturn uintptr_t _hcc_abort(const char* file, int line, const char* message, ...) {
	fprintf(stderr, "abort: ");

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

HccResult hcc_get_last_global_result(void) {
	return _hcc_tls.result_data.result;
}

void hcc_clear_bail_jmp_loc(void) {
	_hcc_tls.jmp_loc_recursive_set_count -= 1;
	if (_hcc_tls.jmp_loc_recursive_set_count == 0) {
		_hcc_tls.jmp_result_data = NULL;
	}
}

_Noreturn void hcc_bail(HccResultCode code, int32_t value) {
	HccResult result = { code, value, NULL };
	HccCompiler* c = _hcc_tls.c;
	if (c) {
		HccCompilerFlags flags = atomic_load(&c->flags);
		while (!(flags & HCC_COMPILER_FLAGS_IS_RESULT_SET)) {
			HccCompilerFlags next_flags = flags | HCC_COMPILER_FLAGS_IS_RESULT_SET;
			if (atomic_compare_exchange_weak(&c->flags, &flags, next_flags)) {
				if (_hcc_gs.flags & HCC_FLAGS_ENABLE_STACKTRACE) {
					hcc_stacktrace(
						1,
						_hcc_tls.jmp_result_data->result_stacktrace,
						sizeof(_hcc_tls.jmp_result_data->result_stacktrace)
					);
					result.stacktrace = _hcc_tls.jmp_result_data->result_stacktrace;
				}
				_hcc_tls.jmp_result_data->result = result;
				break;
			}
		}
	} else {
		_hcc_tls.jmp_result_data->result = result;
	}

	if (_hcc_tls.w) {
		HccTask* t = hcc_worker_task(_hcc_tls.w);
		if (t) {
			HccTaskFlags flags = atomic_load(&t->flags);
			while (!(flags & HCC_TASK_FLAGS_IS_RESULT_SET)) {
				HccTaskFlags next_flags = flags | HCC_TASK_FLAGS_IS_RESULT_SET;
				if (atomic_compare_exchange_weak(&t->flags, &flags, next_flags)) {
					t->result = result;
				}
			}
			hcc_task_finish(t, true);
		}
	}

	longjmp(_hcc_tls.jmp_loc, 1);
}

// ===========================================
//
//
// Platform Abstraction
//
//
// ===========================================

uint32_t hcc_onebitscount32(uint32_t bits) {
#ifdef __GNUC__
	return __builtin_popcount(bits);
#elif defined(HCC_OS_WINDOWS)
	return __popcnt(bits);
#else
#error "unsupported fartcount"
#endif
}

uint32_t hcc_leastsetbitidx32(uint32_t bits) {
	HCC_ASSERT(bits, "bits cannot be 0");
#ifdef __GNUC__
	return __builtin_ctz(bits);
#elif defined(HCC_OS_WINDOWS)
	unsigned long idx;
	_BitScanForward(&idx, bits);
	return idx;
#else
#error "unsupported fartcount"
#endif
}

uint32_t hcc_mostsetbitidx32(uint32_t bits) {
	HCC_ASSERT(bits, "bits cannot be 0");
#ifdef __GNUC__
	return (sizeof(uint32_t) * 8) - __builtin_clz(bits);
#elif defined(HCC_OS_WINDOWS)
	unsigned long idx;
	_BitScanReverse(&idx, bits);
	return idx;
#else
#error "unsupported fartcount"
#endif
}

void hcc_get_last_system_error_string(char* buf_out, uint32_t buf_out_size) {
#ifdef HCC_OS_WINDOWS
	DWORD error = GetLastError();
	DWORD res = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPTSTR)buf_out, buf_out_size, NULL);
	if (res == 0) {
		error = GetLastError();
		HCC_ABORT("TODO handle error code: %u", error);
	}
#elif _GNU_SOURCE
	int error = errno;
	// GNU version (screw these guys for changing the way this works)
	char* buf = strerror_r(error, buf_out, buf_out_size);
	if (strcmp(buf, "Unknown error") == 0) {
		goto ERROR_1;
	}

	// if its static string then copy it to the buffer
	if (buf != buf_out) {
		strncpy(buf_out, buf, buf_out_size);

		uint32_t size = strlen(buf);
		if (size < buf_out_size) {
			goto ERROR_2;
		}
	}
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int error = errno;
	// use the XSI standard behavior.
	int res = strerror_r(error, buf_out, buf_out_size);
	if (res != 0) {
		int errnum = res;
		if (res == -1)
			errnum = errno;

		if (errnum == EINVAL) {
			goto ERROR_1;
		} else if (errnum == ERANGE) {
			goto ERROR_2;
		}
		HCC_ABORT("unexpected errno: %u", errnum);
	}
#else
#error "unimplemented for this platform"
#endif
	return;
ERROR_1:
	HCC_ABORT("invalid error %u", error);
ERROR_2:
	printf("internal warning: error buffer of size '%u' is not big enough", buf_out_size);
	HCC_ABORT("invalid error %u", error);
}

uint32_t hcc_path_canonicalize_internal(const char* path, char* out_buf) {
#ifdef HCC_OS_LINUX
	if (realpath(path, out_buf) == NULL) {
		return UINT32_MAX;
	}
	return strlen(out_buf);
#elif defined(HCC_OS_WINDOWS)
	DWORD size = GetFullPathNameA(path, MAX_PATH, out_buf, NULL);
	if (size == 0) {
		return UINT32_MAX;
	}
	return size;
#else
#error "unimplemented for this platform"
#endif
}

HccString hcc_path_canonicalize(const char* path) {
	char buf[PATH_MAX];
	HccString new_path;
	new_path.size = _hcc_gs.path_canonicalize_fn(path, buf);
	new_path.data = HCC_ARENA_ALCTOR_ALLOC_ARRAY_THREAD_SAFE(char, &_hcc_gs.arena_alctor, new_path.size + 1);
	memcpy(new_path.data, buf, new_path.size + 1);
	return new_path;
}

bool hcc_path_is_absolute(const char* path) {
#ifdef __unix__
	return path[0] == '/';
#elif defined(HCC_OS_WINDOWS)
	return path[1] == '\\' && path[2] == ':';
#else
#error "unimplemented for this platform"
#endif
}

char* hcc_file_read_all_the_codes(const char* path, uint64_t* size_out) {
#define _HCC_TOKENIZER_LOOK_HEAD_SIZE 4
#ifdef HCC_OS_LINUX
	int fd_flags = O_CLOEXEC | O_RDONLY;
	int mode = 0666;
	int fd = open(path, fd_flags, mode);
	if (fd == -1) {
		hcc_bail(HCC_ERROR_FILE_READ, errno);
	}

	struct stat s = {0};
	if (fstat(fd, &s) != 0) return NULL;
	*size_out = s.st_size;

	uintptr_t size = HCC_INT_ROUND_UP_ALIGN(s.st_size + _HCC_TOKENIZER_LOOK_HEAD_SIZE, _hcc_gs.virt_mem_reserve_align);
	char* bytes;
	hcc_virt_mem_reserve_commit(HCC_ALLOC_TAG_CODE, NULL, size, HCC_VIRT_MEM_PROTECTION_READ_WRITE, (void**)&bytes);

	uint64_t remaining_read_size = s.st_size;
	uint64_t offset = 0;
	while (remaining_read_size) {
		ssize_t bytes_read = read(fd, bytes + offset, remaining_read_size);
		if (bytes_read == (ssize_t)-1) {
			hcc_bail(HCC_ERROR_FILE_READ, errno);
		}
		offset += bytes_read;
		remaining_read_size -= bytes_read;
	}

	close(fd);

	return bytes;
#elif defined(HCC_OS_WINDOWS)
	HANDLE handle = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		hcc_bail(HCC_ERROR_FILE_READ, GetLastError());
	}

	DWORD file_size = GetFileSize(handle, &file_size);
	if (file_size == INVALID_FILE_SIZE) {
		hcc_bail(HCC_ERROR_FILE_READ, GetLastError());
	}
	*size_out = file_size;

	uintptr_t size = HCC_INT_ROUND_UP_ALIGN(file_size + _HCC_TOKENIZER_LOOK_HEAD_SIZE, _hcc_gs.virt_mem_reserve_align);
	char* bytes;
	hcc_virt_mem_reserve_commit(HCC_ALLOC_TAG_CODE, NULL, size, HCC_VIRT_MEM_PROTECTION_READ_WRITE, (void**)&bytes);

	uint64_t remaining_read_size = file_size;
	uint64_t offset = 0;
	while (remaining_read_size) {
		DWORD bytes_read;
		if (!ReadFile(handle, bytes + offset, remaining_read_size, &bytes_read, NULL)) {
			hcc_bail(HCC_ERROR_FILE_READ, GetLastError());
		}
		offset += bytes_read;
		remaining_read_size -= bytes_read;
	}

	CloseHandle(handle);
	return bytes;
#else
#error "unimplemented for this platform"
#endif
}

void hcc_stacktrace(uint32_t ignore_levels_count, char* buf, uint32_t buf_size) {
	uint32_t buf_idx;

#define STACKTRACE_LEVELS_MAX 128
#if defined(__GNUC__)
	void* stacktrace_levels[STACKTRACE_LEVELS_MAX];
	int stacktrace_levels_count = backtrace(stacktrace_levels, STACKTRACE_LEVELS_MAX);

	char** strings = backtrace_symbols(stacktrace_levels, stacktrace_levels_count);
	if (strings == NULL) {
		goto ERR;
	}

	for (int i = ignore_levels_count; i < stacktrace_levels_count; i += 1) {
		buf_idx += snprintf(buf + buf_idx, buf_size - buf_idx, "%u: %s\n", stacktrace_levels_count - i - 1, strings[i]);
	}

	free(strings);
#elif defined(HCC_OS_WINDOWS)
	void* stacktrace_levels[STACKTRACE_LEVELS_MAX];
	HANDLE process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);

	uint32_t stacktrace_levels_count = CaptureStackBackTrace(0, STACKTRACE_LEVELS_MAX, stacktrace_levels, NULL);
	alignas(SYMBOL_INFO) char symbol_buf[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
	SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbol_buf;
	symbol->MaxNameLen   = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	for (uint32_t i = ignore_levels_count; i < stacktrace_levels_count; i += 1) {
		SymFromAddr(process, (DWORD64)stacktrace_levels[i], 0, symbol);
		buf_idx += snprintf(buf + buf_idx, buf_size - buf_idx, "%u: %s - [0x%0llX]\n", stacktrace_levels_count - i - 1, symbol->Name, symbol->Address);
	}
#else
#error "hcc_stacktrace has been unimplemented for this platform"
#endif

	return;
ERR:
	buf[0] = '\0';
}

bool hcc_file_open_read(const char* path, HccIIO* out) {
#ifdef HCC_OS_WINDOWS
	FILE* f;
	errno_t error = fopen_s(&f, path, "rb");
	if (error) {
		return false;
	}
#else
	FILE* f = fopen(path, "rb");
	if (!f) {
		return false;
	}
#endif

	*out = hcc_iio_file(f);
	return true;
}

bool hcc_file_open_write(const char* path, HccIIO* out) {
#ifdef HCC_OS_WINDOWS
	FILE* f;
	errno_t error = fopen_s(&f, path, "wb");
	if (error) {
		return false;
	}
#else
	FILE* f = fopen(path, "wb");
	if (!f) {
		return false;
	}
#endif

	*out = hcc_iio_file(f);
	return true;
}

bool hcc_path_is_relative(const char* path) {
	return !hcc_path_is_absolute(path);
}

bool hcc_path_exists(const char* path) {
#ifdef HCC_OS_LINUX
	return access(path, F_OK) == 0;
#elif defined(HCC_OS_WINDOWS)
	return PathFileExistsA(path);
#else
#error "unimplemented for this platform"
#endif
}

bool hcc_path_is_file(const char* path) {
#ifdef HCC_OS_LINUX
	struct stat s;
	if (stat(path, &s) != 0) {
		return false;
	}
	return S_ISREG(s.st_mode);
#elif defined(HCC_OS_WINDOWS)
	DWORD attr = GetFileAttributesA(path);
	return !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
#error "unimplemented for this platform"
#endif
}

bool hcc_path_is_directory(const char* path) {
#ifdef HCC_OS_LINUX
	struct stat s;
	if (stat(path, &s) != 0) {
		return false;
	}
	return S_ISDIR(s.st_mode);
#elif defined(HCC_OS_WINDOWS)
	DWORD attr = GetFileAttributesA(path);
	return (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
#error "unimplemented for this platform"
#endif
}

bool hcc_make_directory(const char* path) {
#ifdef HCC_OS_LINUX
	if (mkdir(path, 0777) != 0) {
		return false;
	}
#elif defined(HCC_OS_WINDOWS)
	if (!CreateDirectoryA(path, NULL)) {
		return false;
	}
#else
#error "unimplemented for this platform"
#endif
	return true;
}

HccString hcc_path_replace_file_name(HccString parent, HccString file_name) {
	uint32_t parent_copy_size = parent.size;
	while (parent_copy_size) {
		parent_copy_size -= 1;
		if (parent.data[parent_copy_size] == '/' || parent.data[parent_copy_size] == '\\') {
			break;
		}
	}

	uintptr_t new_buf_size = parent_copy_size + file_name.size + 2; // + 2 for / and \0
	char* new_buf = HCC_ARENA_ALCTOR_ALLOC_ARRAY_THREAD_SAFE(char, &_hcc_gs.arena_alctor, new_buf_size);
	memcpy(new_buf, parent.data, parent_copy_size);
	new_buf[parent_copy_size] = '/';
	memcpy(new_buf + parent_copy_size + 1, file_name.data, file_name.size);
	new_buf[new_buf_size - 1] = '\0';

	return hcc_string(new_buf, new_buf_size - 1);
}

uint32_t hcc_logical_cores_count(void) {
#ifdef HCC_OS_LINUX
	return get_nprocs();
#elif defined(HCC_OS_WINDOWS)
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
#else
#error "unimplemented on platform
#endif
}

int hcc_execute_shell_command(const char* shell_command) {
	return system(shell_command);
}

// ===========================================
//
//
// Pointer & Number Utilities
//
//
// ===========================================

uint32_t hcc_uint32_round_to_multiple(uint32_t v, uint32_t multiple) {
	v += multiple / 2;
	uint32_t rem = v % multiple;
	if (v > 0.0f) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

uint32_t hcc_uint32_round_up_to_multiple(uint32_t v, uint32_t multiple) {
	uint32_t rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v + multiple - rem;
	} else {
		return v - rem;
	}
}

uint32_t hcc_uint32_round_down_to_multiple(uint32_t v, uint32_t multiple) {
	uint32_t rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

uint64_t hcc_uint64_round_to_multiple(uint64_t v, uint64_t multiple) {
	v += multiple / 2;
	uint64_t rem = v % multiple;
	if (v > 0.0f) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

uint64_t hcc_uint64_round_up_to_multiple(uint64_t v, uint64_t multiple) {
	uint64_t rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v + multiple - rem;
	} else {
		return v - rem;
	}
}

uint64_t hcc_uint64_round_down_to_multiple(uint64_t v, uint64_t multiple) {
	uint64_t rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

float hcc_float_round_to_multiple(float v, float multiple) {
	v += multiple * 0.5f;
	float rem = fmodf(v, multiple);
	if (v > 0.0f) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

float hcc_float_round_up_to_multiple(float v, float multiple) {
	float rem = fmodf(v, multiple);
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v + multiple - rem;
	} else {
		return v - rem;
	}
}

float hcc_float_round_down_to_multiple(float v, float multiple) {
	float rem = fmodf(v, multiple);
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

float f16tof32(half v) {
	if ((v._bits & 0x7c00) == 0x7c00) { // inf, -inf or nan
		if (v._bits & 0x03ff) return NAN;
		else if (v._bits & 0x8000) return -INFINITY;
		else return INFINITY;
	}

	union { float f; uint32_t u; } t1;
	uint32_t t2;
	uint32_t t3;

	t1.u = v._bits & 0x7fff;      // non-sign bits
	t2 = v._bits & 0x8000;        // sign bit
	t3 = v._bits & 0x7c00;        // exponent

	t1.u <<= 13;                 // align mantissa on MSB
	t2 <<= 16;                   // shift sign bit into position

	t1.u += 0x38000000;          // adjust bias

	t1.u = (t3 == 0 ? 0 : t1.u); // denormals-as-zero

	t1.u |= t2;                  // re-insert sign bit

	return t1.f;
}

half f32tof16(float v) {
	if (isinf(v)) return (half){ ._bits = v < 0.0 ? 0xfc00 : 0x7c00 };
	if (isnan(v)) return (half){ ._bits = 0xffff };

	union { float f; uint32_t u; } vu = { .f = v };
	uint32_t t1;
	uint32_t t2;
	uint32_t t3;

	t1 = vu.u & 0x7fffffff;                // non-sign bits
	t2 = vu.u & 0x80000000;                // sign bit
	t3 = vu.u & 0x7f800000;                // exponent

	t1 >>= 13;                             // align mantissa on MSB
	t2 >>= 16;                             // shift sign bit into position

	t1 -= 0x1c000;                         // adjust bias

	t1 = (t3 < 0x38800000) ? 0 : t1;       // flush-to-zero
	t1 = (t3 > 0x8e000000) ? 0x7bff : t1;  // clamp-to-max
	t1 = (t3 == 0 ? 0 : t1);               // denormals-as-zero

	t1 |= t2;                              // re-insert sign bit

	return (half){ ._bits = t1 };
}

// ===========================================
//
//
// Result
//
//
// ===========================================

const char* hcc_success_strings[HCC_SUCCESS_COUNT] = {
	[HCC_SUCCESS] = "success",
};

const char* hcc_error_strings[HCC_ERROR_COUNT] = {
	[HCC_ERROR_END - HCC_ERROR_ALLOCATION_FAILURE - 1] = "error: allocation_failure",
	[HCC_ERROR_END - HCC_ERROR_COLLECTION_FULL - 1] = "error: collection_full",
	[HCC_ERROR_END - HCC_ERROR_MESSAGES - 1] = "error: messages",
	[HCC_ERROR_END - HCC_ERROR_THREAD_INIT - 1] = "error: thread_init",
	[HCC_ERROR_END - HCC_ERROR_THREAD_WAIT_FOR_TERMINATION - 1] = "error: thread_wait_for_termination",
	[HCC_ERROR_END - HCC_ERROR_SEMAPHORE_GIVE - 1] = "error: semaphore_give",
	[HCC_ERROR_END - HCC_ERROR_SEMAPHORE_TAKE - 1] = "error: semaphore_take",
	[HCC_ERROR_END - HCC_ERROR_MUTEX_LOCK - 1] = "error: mutex_lock",
	[HCC_ERROR_END - HCC_ERROR_MUTEX_UNLOCK - 1] = "error: mutex_unlock",
};

void hcc_result_print(char* what, HccResult result) {
 	printf("%s returned: 0x%x ", what, result.code);
	switch (result.code) {
		case HCC_SUCCESS:
			printf("%s", hcc_success_strings[result.code]);
			break;
		default:
			printf("%s 0x%x", hcc_error_strings[HCC_ERROR_END - result.code - 1], result.value);
			break;
	}

	if (result.stacktrace) {
		printf("\nstacktrace:\n%s", result.stacktrace);
	}
}

// ===========================================
//
//
// Thread
//
//
// ===========================================

void hcc_thread_start(HccThread* thread, HccThreadSetup* setup) {
#ifdef HCC_OS_LINUX
	pthread_attr_t attr;
	int res;
	if ((res = pthread_attr_init(&attr))) {
		hcc_bail(HCC_ERROR_THREAD_INIT, res);
	}

	if ((res = pthread_attr_setstack(&attr, setup->call_stack, setup->call_stack_size))) {
		hcc_bail(HCC_ERROR_THREAD_INIT, res);
	}

	if ((res = pthread_create(&thread->handle, &attr, (void*(*)(void*))setup->thread_main_fn, setup->arg))) {
		hcc_bail(HCC_ERROR_THREAD_INIT, res);
	}

	if ((res = pthread_attr_destroy(&attr))) {
		hcc_bail(HCC_ERROR_THREAD_INIT, res);
	}
#elif defined(HCC_OS_WINDOWS)
	DWORD thread_id;
	thread->handle = CreateThread(
		NULL,                                   // default security attributes
		0,                                      // use default stack size
		(DWORD(HCC_STDCALL *)(void*))setup->thread_main_fn, // thread function name
		setup->arg,                             // argument to thread function
		0,                                      // use default creation flags
		&thread_id);                            // returns the thread identifier

	if (thread->handle == NULL) {
		hcc_bail(HCC_ERROR_THREAD_INIT, GetLastError());
	}
#else
#error "unimplemented API for this platform"
#endif // HCC_OS_LINUX
}

void hcc_thread_wait_for_termination(HccThread* thread) {
#ifdef HCC_OS_LINUX
	int res;
	if ((res = pthread_join(thread->handle, NULL))) {
		hcc_bail(HCC_ERROR_THREAD_WAIT_FOR_TERMINATION, res);
	}
#elif defined(HCC_OS_WINDOWS)
	WaitForSingleObject(thread->handle, INFINITE);
#else
#error "unimplemented API for this platform"
#endif // HCC_OS_LINUX
}

// ===========================================
//
//
// Semaphore
//
//
// ===========================================

void hcc_semaphore_init(HccSemaphore* semaphore, uint32_t initial_value) {
	atomic_store(&semaphore->value, initial_value);
}

void hcc_semaphore_set(HccSemaphore* semaphore, uint32_t value) {
	atomic_store(&semaphore->value, value);
	if (value) {
#ifdef HCC_OS_LINUX
		uint32_t wake_count = 1;
		long res = syscall(SYS_futex, &semaphore->value, FUTEX_WAKE, wake_count, NULL, NULL, 0);
		if (res == -1) {
			hcc_bail(HCC_ERROR_SEMAPHORE_GIVE, errno);
		}
#elif defined(HCC_OS_WINDOWS)
		WakeByAddressSingle(&semaphore->value);
#else
#error "unimplemented API for this platform"
#endif // HCC_OS_LINUX
	}
}

void hcc_semaphore_give(HccSemaphore* semaphore, uint32_t count) {
	if (count == 0) {
		return;
	}

	atomic_fetch_add(&semaphore->value, count);
#ifdef HCC_OS_LINUX
	uint32_t wake_count = count;
	long res = syscall(SYS_futex, &semaphore->value, FUTEX_WAKE, wake_count, NULL, NULL, 0);
	if (res == -1) {
		hcc_bail(HCC_ERROR_SEMAPHORE_GIVE, errno);
	}
#elif defined(HCC_OS_WINDOWS)
	if (count > 1) {
		WakeByAddressAll(&semaphore->value);
	} else {
		WakeByAddressSingle(&semaphore->value);
	}
#else
#error "unimplemented API for this platform"
#endif // HCC_OS_LINUX
}

void hcc_semaphore_take_or_wait_then_take(HccSemaphore* semaphore) {
	atomic_fetch_add(&semaphore->waiters_count, 1);
#ifdef HCC_OS_LINUX
	while (1) {
		uint32_t counter = atomic_load(&semaphore->value);
		while (counter) {
			uint32_t next_counter = counter - 1;
			if (atomic_compare_exchange_weak(&semaphore->value, &counter, next_counter)) {
				goto RETURN;
			}
		}

		uint32_t expected_value = 0;
		long res = syscall(SYS_futex, &semaphore->value, FUTEX_WAIT, expected_value, NULL, NULL, 0);
		if (res == -1 && errno != EAGAIN) {
			hcc_bail(HCC_ERROR_SEMAPHORE_TAKE, errno);
		}
	}
#elif defined(HCC_OS_WINDOWS)
	while (1) {
		uint32_t counter = atomic_load(&semaphore->value);
		while (counter) {
			uint32_t next_counter = counter - 1;
			if (atomic_compare_exchange_weak(&semaphore->value, &counter, next_counter)) {
				goto RETURN;
			}
		}

		uint32_t expected_value = 0;
		if (!WaitOnAddress(&semaphore->value, &expected_value, sizeof(expected_value), INFINITE)) {
			hcc_bail(HCC_ERROR_SEMAPHORE_TAKE, GetLastError());
		}
	}
#else
#error "unimplemented API for this platform"
#endif // HCC_OS_LINUX
RETURN:
	atomic_fetch_sub(&semaphore->waiters_count, 1);
}

uint32_t hcc_semaphore_get_waiters_count(HccSemaphore* semaphore) {
	return atomic_load(&semaphore->waiters_count);
}

// ===========================================
//
//
// Spin Mutex
//
//
// ===========================================

void hcc_spin_mutex_init(HccSpinMutex* mutex) {
	atomic_store(&mutex->is_locked, false);
}

bool hcc_spin_mutex_is_locked(HccSpinMutex* mutex) {
	return atomic_load(&mutex->is_locked);
}

void hcc_spin_mutex_lock(HccSpinMutex* mutex) {
	while (1) {
		uint8_t expected = false;
		if (atomic_compare_exchange_weak(&mutex->is_locked, &expected, true)) {
			break;
		}
		HCC_CPU_RELAX();
	}
}

void hcc_spin_mutex_unlock(HccSpinMutex* mutex) {
	atomic_store(&mutex->is_locked, false);
}

// ===========================================
//
//
// Mutex
//
//
// ===========================================

void hcc_mutex_init(HccMutex* mutex) {
	atomic_store(&mutex->is_locked, false);
}

bool hcc_mutex_is_locked(HccMutex* mutex) {
	return atomic_load(&mutex->is_locked);
}

void hcc_mutex_lock(HccMutex* mutex) {
#ifdef HCC_OS_LINUX
	while (1) {
		uint32_t is_locked = atomic_load(&mutex->is_locked);
		while (!is_locked) {
			if (atomic_compare_exchange_weak(&mutex->is_locked, &is_locked, true)) {
				return;
			}
		}

		uint32_t expected_value = true;
		long res = syscall(SYS_futex, &mutex->is_locked, FUTEX_WAIT, expected_value, NULL, NULL, 0);
		if (res == -1 && errno != EAGAIN) {
			hcc_bail(HCC_ERROR_MUTEX_LOCK, errno);
		}
	}
#elif defined(HCC_OS_WINDOWS)
	while (1) {
		uint32_t is_locked = atomic_load(&mutex->is_locked);
		while (!is_locked) {
			if (atomic_compare_exchange_weak(&mutex->is_locked, &is_locked, true)) {
				return;
			}
		}

		uint32_t expected_value = true;
		if (!WaitOnAddress(&mutex->is_locked, &expected_value, sizeof(expected_value), INFINITE)) {
			hcc_bail(HCC_ERROR_MUTEX_LOCK, GetLastError());
		}
	}
#else
#error "unimplemented API for this platform"
#endif // HCC_OS_LINUX
}

void hcc_mutex_unlock(HccMutex* mutex) {
	atomic_store(&mutex->is_locked, false);
#ifdef HCC_OS_LINUX
	uint32_t wake_count = 1;
	long res = syscall(SYS_futex, &mutex->is_locked, FUTEX_WAKE, wake_count, NULL, NULL, 0);
	if (res == -1) {
		hcc_bail(HCC_ERROR_MUTEX_UNLOCK, errno);
	}
#elif defined(HCC_OS_WINDOWS)
	WakeByAddressSingle(&mutex->is_locked);
#else
#error "unimplemented API for this platform"
#endif // HCC_OS_LINUX
}

// ===========================================
//
//
// Allocation Tagging
//
//
// ===========================================

const char* hcc_alloc_tag_strings[HCC_ALLOC_TAG_COUNT] = {
	[HCC_ALLOC_TAG_NONE] = "NONE",
	[HCC_ALLOC_TAG_CODE] = "CODE",
};

//
// CONTRIBUTOR: we want track virtual memory reserves and releases so the user can
// call a log and iterate over the allocated memory.
//
HCC_CONTRIBUTOR_TASK(memory tracking system)

void hcc_mem_tracker_init(void) {
}

void hcc_mem_tracker_deinit(void) {
}

void hcc_mem_tracker_update(HccAllocMode mode, HccAllocTag tag, void* addr, uintptr_t size) {
	if (_hcc_gs.alloc_event_fn) {
		_hcc_gs.alloc_event_fn(_hcc_gs.alloc_event_userdata, mode, tag, addr, size);
	}
}

HccMemTrackerIter* hcc_mem_tracker_iter_start(void) {
	return NULL;
}

void hcc_mem_tracker_iter_finish(HccMemTrackerIter* iter) {
	HCC_UNUSED(iter);
}

bool hcc_mem_tracker_iter_next(HccMemTrackerIter* iter, HccTrackedMem* out) {
	HCC_UNUSED(iter);
	HCC_UNUSED(out);
	return true;
}

void hcc_mem_tracker_log(HccIIO* iio) {
	HCC_UNUSED(iio);
}

// ===========================================
//
//
// Arena Alctor
//
//
// ===========================================

HccArenaHeader* hcc_arena_alloc(HccAllocTag tag, uint32_t arena_size) {
	HccArenaHeader* arena;
	arena_size = HCC_INT_ROUND_UP_ALIGN(arena_size, _hcc_gs.virt_mem_reserve_align);
	hcc_virt_mem_reserve_commit(tag, NULL, arena_size, HCC_VIRT_MEM_PROTECTION_READ_WRITE, (void**)&arena);

	atomic_store(&arena->pos, sizeof(HccArenaHeader));
	arena->size = arena_size;
	return arena;
}

void hcc_arena_alctor_init(HccArenaAlctor* alctor, HccAllocTag tag, uint32_t arena_size) {
	HccArenaHeader* arena = hcc_arena_alloc(tag, arena_size);
	alctor->arena = arena;
	alctor->tag = tag;
}

void hcc_arena_alctor_deinit(HccArenaAlctor* alctor) {
	uint32_t arena_size = alctor->arena->size;
	while (alctor->arena) {
		alctor->arena = alctor->arena->prev;
		HccArenaHeader* arena = alctor->arena;
		hcc_virt_mem_release(alctor->tag, arena, arena->size);
	}
	alctor->arena = NULL;
}

void* hcc_arena_alctor_alloc(HccArenaAlctor* alctor, uint32_t size, uint32_t align) {
	HccArenaHeader* arena = *(HccArenaHeader**)&alctor->arena;
	if (size > arena->size - sizeof(HccArenaHeader)) {
		//
		// allocation is larger than the all the arenas' block sizes
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, alctor->tag);
	}

	uint32_t pos = *(uint32_t*)&arena->pos;
	while (1) {
		void* ptr = HCC_PTR_ADD(arena, pos);
		ptr = HCC_PTR_ROUND_UP_ALIGN(ptr, align);
		void* end_ptr = HCC_PTR_ADD(ptr, size);
		uint32_t new_pos = HCC_PTR_DIFF(end_ptr, arena);

		if (new_pos > arena->size) {
			HccArenaHeader* new_arena = hcc_arena_alloc(alctor->tag, arena->size);
			new_arena->prev = arena;
			*(HccArenaHeader**)&alctor->arena = new_arena;
			arena = new_arena;
			pos = *(uint32_t*)&arena->pos;
			continue;
		}

		*(uint32_t*)&arena->pos = new_pos;
		return ptr;
	}
}

void* hcc_arena_alctor_alloc_thread_safe(HccArenaAlctor* alctor, uint32_t size, uint32_t align) {
	HccArenaHeader* arena = atomic_load(&alctor->arena);
	if (size > arena->size - sizeof(HccArenaHeader)) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, alctor->tag);
	}

	uint32_t pos = atomic_load(&arena->pos);
	while (1) {
		void* ptr = HCC_PTR_ADD(arena, pos);
		ptr = HCC_PTR_ROUND_UP_ALIGN(ptr, align);
		void* end_ptr = HCC_PTR_ADD(ptr, size);
		uint32_t new_pos = HCC_PTR_DIFF(end_ptr, arena);

		if (new_pos > arena->size) {
			bool expected = false;
			if (atomic_compare_exchange_strong(&alctor->alloc_arena_sync_point, &expected, true)) {
				if (atomic_load(&alctor->arena) == arena) {
					//
					// this thread managed to claim the role of allocating the next arena for the arena allocator
					// and another thread has not just finished allocating a new arena.
					HccArenaHeader* new_arena = hcc_arena_alloc(alctor->tag, arena->size);
					new_arena->prev = arena;
					atomic_store(&alctor->arena, new_arena);
					arena = new_arena;
				}
				atomic_store(&alctor->alloc_arena_sync_point, false);
			} else {
				//
				// this thread has to wait for the thread that has the role of allocating the next arena.
				while (atomic_load(&alctor->alloc_arena_sync_point)) {
					HCC_CPU_RELAX();
				}
				arena = atomic_load(&alctor->arena);
			}

			pos = atomic_load(&arena->pos);
			continue;
		}

		if (atomic_compare_exchange_weak(&arena->pos, &pos, new_pos)) {
			//
			// this thread successfully advanced the cursor and has claim the memory in this arena.
			return ptr;
		}
	}
}

void hcc_arena_alctor_reset(HccArenaAlctor* alctor) {
	uint32_t arena_size = alctor->arena->size;
	hcc_arena_alctor_deinit(alctor);
	alctor->arena = hcc_arena_alloc(alctor->tag, arena_size);
}

// ===========================================
//
//
// Virtual Memory Abstraction
//
//
// ===========================================
//
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
static int _hcc_virt_mem_prot_unix(HccVirtMemProtection prot) {
	switch (prot) {
		case HCC_VIRT_MEM_PROTECTION_NO_ACCESS: return 0;
		case HCC_VIRT_MEM_PROTECTION_READ: return PROT_READ;
		case HCC_VIRT_MEM_PROTECTION_READ_WRITE: return PROT_READ | PROT_WRITE;
		case HCC_VIRT_MEM_PROTECTION_EXEC_READ: return PROT_EXEC | PROT_READ;
		case HCC_VIRT_MEM_PROTECTION_EXEC_READ_WRITE: return PROT_READ | PROT_WRITE | PROT_EXEC;
	}
	return 0;
}
#elif defined(HCC_OS_WINDOWS)
static DWORD _hcc_virt_mem_prot_windows(HccVirtMemProtection prot) {
	switch (prot) {
		case HCC_VIRT_MEM_PROTECTION_NO_ACCESS: return PAGE_NOACCESS;
		case HCC_VIRT_MEM_PROTECTION_READ: return PAGE_READONLY;
		case HCC_VIRT_MEM_PROTECTION_READ_WRITE: return PAGE_READWRITE;
		case HCC_VIRT_MEM_PROTECTION_EXEC_READ: return PAGE_EXECUTE_READ;
		case HCC_VIRT_MEM_PROTECTION_EXEC_READ_WRITE: return PAGE_EXECUTE_READWRITE;
	}
	return 0;
}
#else
#error "unimplemented virtual memory API for this platform"
#endif

#define HCC_DEBUG_ASSERT_PAGE_SIZE(v) HCC_DEBUG_ASSERT((uintptr_t)v % _hcc_gs.virt_mem_page_size == 0, #v " must be aligned to the return value of hcc_virt_mem_page_size")
#define HCC_DEBUG_ASSERT_RESERVE_ALIGN(v) HCC_DEBUG_ASSERT((uintptr_t)v % _hcc_gs.virt_mem_reserve_align == 0, #v " must be aligned to the return value of hcc_virt_mem_reserve_align")
#define HCC_DEBUG_ASSERT_RESERVE_ALIGN_OR_ZERO(v) HCC_DEBUG_ASSERT(v == 0 || (uintptr_t)v % _hcc_gs.virt_mem_reserve_align == 0, #v " must be aligned to the return value of hcc_virt_mem_reserve_align")

void hcc_virt_mem_update_page_size_reserve_align(void) {
#ifdef HCC_OS_LINUX
	long page_size = sysconf(_SC_PAGESIZE);
	HCC_ASSERT(page_size != (long)-1, "error: we failed to get the page size");

	_hcc_gs.virt_mem_page_size = page_size;
	_hcc_gs.virt_mem_reserve_align = page_size;
#elif defined(HCC_OS_WINDOWS)
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	_hcc_gs.virt_mem_page_size = si.dwPageSize;
	_hcc_gs.virt_mem_reserve_align = si.dwAllocationGranularity;
#else
#error "unimplemented virtual memory API for this platform"
#endif
}

uintptr_t hcc_virt_mem_page_size(void) { return _hcc_gs.virt_mem_page_size; }
uintptr_t hcc_virt_mem_reserve_align(void) { return _hcc_gs.virt_mem_reserve_align; }

void hcc_virt_mem_reserve_commit(HccAllocTag tag, void* requested_addr, uintptr_t size, HccVirtMemProtection protection, void** addr_out) {
	HCC_DEBUG_ASSERT_RESERVE_ALIGN_OR_ZERO(requested_addr);
	HCC_DEBUG_ASSERT_RESERVE_ALIGN(size);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int prot = _hcc_virt_mem_prot_unix(protection);

	// MAP_ANON = means map physical memory and not a file. it also means the memory will be initialized to zero
	// MAP_PRIVATE = keep memory private so child process cannot access them
	// MAP_NORESERVE = do not reserve any swap space for this mapping
	void* addr = mmap(requested_addr, size, prot, MAP_ANON | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
	if (addr == MAP_FAILED) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#elif defined(HCC_OS_WINDOWS)
	DWORD prot = _hcc_virt_mem_prot_windows(protection);
	void* addr = VirtualAlloc(requested_addr, size, MEM_RESERVE | MEM_COMMIT, prot);
	if (addr == NULL) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
#error "TODO implement virtual memory for this platform"
#endif

	hcc_mem_tracker_update(HCC_ALLOC_MODE_ALLOC, tag, addr, size);
	*addr_out = addr;
}

void hcc_virt_mem_reserve(HccAllocTag tag, void* requested_addr, uintptr_t size, void** addr_out) {
	HCC_DEBUG_ASSERT_RESERVE_ALIGN_OR_ZERO(requested_addr);
	HCC_DEBUG_ASSERT_RESERVE_ALIGN(size);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	// memory is automatically commited on Unix based OSs,
	// so we will restrict the memory from being accessed on reserved.
	int prot = 0;

	// MAP_ANON = means map physical memory and not a file. it also means the memory will be initialized to zero
	// MAP_PRIVATE = keep memory private so child process cannot access them
	// MAP_NORESERVE = do not reserve any swap space for this mapping
	void* addr = mmap(requested_addr, size, prot, MAP_ANON | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
	if (addr == MAP_FAILED) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#elif defined(HCC_OS_WINDOWS)
	void* addr = VirtualAlloc(requested_addr, size, MEM_RESERVE, PAGE_NOACCESS);
	if (addr == NULL) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
#error "TODO implement virtual memory for this platform"
#endif

	hcc_mem_tracker_update(HCC_ALLOC_MODE_ALLOC, tag, addr, size);
	*addr_out = addr;
}

void hcc_virt_mem_commit(HccAllocTag tag, void* addr, uintptr_t size, HccVirtMemProtection protection) {
	HCC_DEBUG_ASSERT_PAGE_SIZE(addr);
	HCC_DEBUG_ASSERT_PAGE_SIZE(size);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	// memory is automatically commited on Unix based OSs,
	// memory is restricted from being accessed in our hcc_virt_mem_reserve.
	// so lets just apply the protection for the address space.
	// and then advise the OS that these pages will be used soon.
	int prot = _hcc_virt_mem_prot_unix(protection);
	if (mprotect(addr, size, prot) != 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
	madvise(addr, size, MADV_WILLNEED);
#elif defined(HCC_OS_WINDOWS)
	DWORD prot = _hcc_virt_mem_prot_windows(protection);
	if (VirtualAlloc(addr, size, MEM_COMMIT, prot) == NULL) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
#error "TODO implement virtual memory for this platform"
#endif
}

void hcc_virt_mem_protection_set(HccAllocTag tag, void* addr, uintptr_t size, HccVirtMemProtection protection) {
	HCC_DEBUG_ASSERT_PAGE_SIZE(addr);
	HCC_DEBUG_ASSERT_PAGE_SIZE(size);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int prot = _hcc_virt_mem_prot_unix(protection);
	if (mprotect(addr, size, prot) != 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#elif defined(HCC_OS_WINDOWS)
	DWORD prot = _hcc_virt_mem_prot_windows(protection);
	DWORD old_prot; // unused
	if (!VirtualProtect(addr, size, prot, &old_prot)) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
#error "TODO implement virtual memory for this platform"
#endif
}

void hcc_virt_mem_decommit(HccAllocTag tag, void* addr, uintptr_t size) {
	HCC_DEBUG_ASSERT_PAGE_SIZE(addr);
	HCC_DEBUG_ASSERT_PAGE_SIZE(size);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

	//
	// advise the OS that these pages will not be needed.
	// the OS will zero fill these pages before you next access them.
	if (madvise(addr, size, MADV_DONTNEED) != 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}

	// memory is automatically commited on Unix based OSs,
	// so we will restrict the memory from being accessed when we "decommit.
	int prot = 0;
	if (mprotect(addr, size, prot) != 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#elif defined(HCC_OS_WINDOWS)
	if (VirtualFree(addr, size, MEM_DECOMMIT) == 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
#error "TODO implement virtual memory for this platform"
#endif
}

void hcc_virt_mem_release(HccAllocTag tag, void* addr, uintptr_t size) {
#ifdef HCC_OS_WINDOWS
	HCC_DEBUG_ASSERT_RESERVE_ALIGN(addr);
	HCC_DEBUG_ASSERT_RESERVE_ALIGN(size);
#else
	HCC_DEBUG_ASSERT_PAGE_SIZE(addr);
	HCC_DEBUG_ASSERT_PAGE_SIZE(size);
#endif
	hcc_mem_tracker_update(HCC_ALLOC_MODE_DEALLOC, tag, addr, size);

#ifdef HCC_OS_LINUX
	if (munmap(addr, size) != 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#elif defined(HCC_OS_WINDOWS)
	//
	// unfortunately on Windows all memory must be release at once
	// that was reserved with VirtualAlloc.
	if (!VirtualFree(addr, 0, MEM_RELEASE)) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
#error "TODO implement virtual memory for this platform"
#endif
}

void hcc_virt_mem_reset(HccAllocTag tag, void* addr, uintptr_t size) {
	HCC_DEBUG_ASSERT_PAGE_SIZE(addr);
	HCC_DEBUG_ASSERT_PAGE_SIZE(size);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	if (madvise(addr, size, MADV_DONTNEED) != 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#elif defined(HCC_OS_WINDOWS)
	MEMORY_BASIC_INFORMATION mem_info;
	if (VirtualQuery(addr, &mem_info, sizeof(mem_info)) == 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}

	if (VirtualFree(addr, size, MEM_DECOMMIT) == 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}

	if (VirtualAlloc(addr, size, MEM_COMMIT, mem_info.Protect) == NULL) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
#error "TODO implement virtual memory for this platform"
#endif
}

void hcc_virt_mem_force_physical_page_allocation(void* addr, uintptr_t size) {
	HCC_DEBUG_ASSERT_PAGE_SIZE(addr);
	HCC_DEBUG_ASSERT_PAGE_SIZE(size);

	for (uintptr_t offset = 0; offset < size; offset += _hcc_gs.virt_mem_page_size) {
		uintptr_t word = *(volatile uintptr_t*)HCC_PTR_ADD(addr, offset);
	}
}

void hcc_virt_mem_magic_ring_buffer_alloc(HccAllocTag tag, void* requested_addr, uintptr_t size, void** addr_out) {
#ifdef HCC_OS_WINDOWS
	BOOL result;
	HANDLE section = NULL;
	SYSTEM_INFO sysInfo;
	void* ringBuffer = NULL;
	void* placeholder1 = NULL;
	void* placeholder2 = NULL;
	void* view1 = NULL;
	void* view2 = NULL;

	//
	// Reserve a placeholder region where the buffer will be mapped.
	//

	placeholder1 = (PCHAR)VirtualAlloc2(
		NULL,
		requested_addr,
		2 * size,
		MEM_RESERVE | MEM_RESERVE_PLACEHOLDER,
		PAGE_NOACCESS,
		NULL, 0
	);

	if (placeholder1 == NULL) {
		goto ERR;
	}

	//
	// Split the placeholder region into two regions of equal size.
	//

	result = VirtualFree(
		placeholder1,
		size,
		MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER
	);

	if (result == FALSE) {
		goto ERR;
	}

	placeholder2 = (void*)((ULONG_PTR)placeholder1 + size);

	//
	// Create a pagefile-backed section for the buffer.
	//

	section = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		size, NULL
	);

	if (section == NULL) {
		goto ERR;
	}

	//
	// Map the section into the first placeholder region.
	//

	view1 = MapViewOfFile3(
		section,
		NULL,
		placeholder1,
		0,
		size,
		MEM_REPLACE_PLACEHOLDER,
		PAGE_READWRITE,
		NULL, 0
	);

	if (view1 == NULL) {
		goto ERR;
	}

	//
	// Ownership transferred, don't free this now.
	//

	placeholder1 = NULL;

	//
	// Map the section into the second placeholder region.
	//

	view2 = MapViewOfFile3(
		section,
		NULL,
		placeholder2,
		0,
		size,
		MEM_REPLACE_PLACEHOLDER,
		PAGE_READWRITE,
		NULL, 0
	);

	if (view2 == NULL) {
		goto ERR;
	}

	//
	// Success, return both mapped views to the caller.
	//

	ringBuffer = view1;

	if (section != NULL) {
		CloseHandle(section);
	}

	*addr_out = ringBuffer;
	return;
ERR:
	if (section != NULL) {
		CloseHandle(section);
	}

	if (placeholder1 != NULL) {
		VirtualFree(placeholder1, 0, MEM_RELEASE);
	}

	if (placeholder2 != NULL) {
		VirtualFree(placeholder2, 0, MEM_RELEASE);
	}

	if (view1 != NULL) {
		UnmapViewOfFileEx(view1, 0);
	}

	if (view2 != NULL) {
		UnmapViewOfFileEx(view2, 0);
	}

	hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
#else
	void* addr;
	int shm_id;
#ifdef HCC_OS_LINUX
	addr = mmap(requested_addr, size * 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	if (addr == MAP_FAILED) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}

	if (remap_file_pages(HCC_PTR_ADD(addr, size), size, 0, 0, 0)) {
		goto REMAP_ERROR;
	}
#else
	shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0700);
	if (shm_id < 0) goto SHMGET_ERROR;

	munmap(addr, size * 2);

	if (shmat(shm_id, addr, 0) != addr) {
		goto SHMAT_ERROR;
	}

	if (shmat(shm_id, addr + size, 0) != (addr + size)) {
		goto SHMAT2_ERROR;
	}

	if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
		goto SHMCTL_ERROR;
	}
#endif

	*addr_out = addr;
	return;

SHMCTL_ERROR:
	shmdt(HCC_PTR_ADD(addr, size));

SHMAT2_ERROR:
	shmdt(addr);

SHMAT_ERROR:
	shmctl(shm_id, IPC_RMID, NULL);
	hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);

SHMGET_ERROR:
REMAP_ERROR:
	munmap(addr, size * 2);
	hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
#endif
}

void hcc_virt_mem_magic_ring_buffer_dealloc(HccAllocTag tag, void* addr, uintptr_t size) {
#ifdef HCC_OS_WINDOWS
	VirtualFree(addr, 0, MEM_RELEASE);
	VirtualFree(HCC_PTR_ADD(addr, size), 0, MEM_RELEASE);
	UnmapViewOfFile(addr);
	UnmapViewOfFile(HCC_PTR_ADD(addr, size));
	HCC_UNUSED(tag);
#elif defined(HCC_OS_LINUX)
	if (munmap(addr, size * 2) != 0) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#else
	if (shmdt(addr) == -1) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
	if (shmdt(addr + size)) {
		hcc_bail(HCC_ERROR_ALLOCATION_FAILURE, tag);
	}
#endif
}

// ===========================================
//
//
// String
//
//
// ===========================================

bool hcc_string_key_cmp(void* a, void* b, uintptr_t size) {
	HCC_UNUSED(size);
	return hcc_string_eq(*(HccString*)a, *(HccString*)b);
}

// ===========================================
//
//
// Time Utilities
//
//
// ===========================================

HccTime hcc_time_now(HccTimeMode mode) {
#if defined(HCC_OS_LINUX)
	int m;
	switch (mode) {
		case HCC_TIME_MODE_REALTIME: m = CLOCK_REALTIME; break;
		case HCC_TIME_MODE_MONOTONIC: m = CLOCK_MONOTONIC; break;
	}

	struct timespec start;
	clock_gettime(m, &start);
	return (HccTime) { .secs = start.tv_sec, .nanosecs = start.tv_nsec };
#elif defined(HCC_OS_WINDOWS)
	HCC_UNUSED(mode); //TODO
	uint64_t wintime;
	GetSystemTimeAsFileTime((FILETIME*)&wintime);
	wintime -= 116444736000000000ll;  //1jan1601 to 1jan1970
	return (HccTime) { .secs = wintime / 10000000ll, .nanosecs = wintime % 10000000ll * 100 };
#else
#error "TODO implement virtual memory for this platform"
#endif
}

HccDuration hcc_time_elapsed(HccTime time, HccTimeMode mode) {
	HccTime now = hcc_time_now(mode);
	return hcc_time_diff(now, time);
}

HccDuration hcc_time_diff(HccTime to, HccTime from) {
	HCC_ASSERT(to.secs > from.secs || (to.secs == from.secs && to.nanosecs >= from.nanosecs), "'to' time must come after 'from' time");
	HccDuration d = {0};
	if (to.nanosecs < from.nanosecs) {
		d.secs = to.secs - from.secs - 1;
		d.nanosecs = 1000000000 + to.nanosecs - from.nanosecs;
	} else {
		d.secs = to.secs - from.secs;
		d.nanosecs = to.nanosecs - from.nanosecs;
	}

	return d;
}

bool hcc_duration_has_secs(HccDuration duration) {
	return duration.secs > 0;
}

bool hcc_duration_has_millisecs(HccDuration duration) {
	return duration.secs > 0 || duration.nanosecs >= 1000000;
}

bool hcc_duration_has_microsecs(HccDuration duration) {
	return duration.secs > 0 || duration.nanosecs >= 1000;
}

bool hcc_duration_has_nanosecs(HccDuration duration) {
	return duration.secs > 0 || duration.nanosecs > 0;
}

float hcc_duration_to_f32_secs(HccDuration duration) {
    return (float)duration.secs + 1.0e-9 * (float)duration.nanosecs;
}

double hcc_duration_to_f64_secs(HccDuration duration) {
    return (double)duration.secs + 1.0e-9 * (double)duration.nanosecs;
}

float hcc_duration_to_f32_millisecs(HccDuration duration) {
    return (float)1.0e-3 * (float)duration.secs + (float)1.0e-6 * (float)duration.nanosecs;
}

double hcc_duration_to_f64_millisecs(HccDuration duration) {
    return (double)1.0e-3 * (double)duration.secs + (double)1.0e-6 * (double)duration.nanosecs;
}

float hcc_duration_to_f32_microsecs(HccDuration duration) {
    return (float)1.0e-6 * (float)duration.secs + (float)1.0e-3 * (float)duration.nanosecs;
}

double hcc_duration_to_f64_microsecs(HccDuration duration) {
    return (double)1.0e-6 * (double)duration.secs + (double)1.0e-3 * (double)duration.nanosecs;
}

uint64_t hcc_duration_secs(HccDuration duration) {
	return duration.secs;
}

uint64_t hcc_duration_millisecs(HccDuration duration) {
	return (duration.secs * 1000) + (duration.nanosecs / 1000000);
}

uint64_t hcc_duration_microsecs(HccDuration duration) {
	return (duration.secs * 1000000) + (duration.nanosecs / 1000);
}

uint64_t hcc_duration_nanosecs(HccDuration duration) {
	return (duration.secs * 1000000000) + duration.nanosecs;
}

uint64_t hcc_duration_frame_to_fps(HccDuration duration) {
	return 1000000000 / hcc_duration_nanosecs(duration);
}

HccDuration hcc_duration_add(HccDuration a, HccDuration b) {
	HccDuration res;
	res.secs = a.secs + b.secs;
	res.nanosecs = a.nanosecs + b.nanosecs;
	if (res.nanosecs >= 1000000000) {
		res.secs += 1;
		res.nanosecs -= 1000000000;
	}
	return res;
}

HccDuration hcc_duration_sub(HccDuration a, HccDuration b) {
	HccDuration res;
	if (a.secs < b.secs) {
		res.secs = 0;
		res.nanosecs = 0;
		return res;
	}

	res.secs = a.secs - b.secs;

	if (a.nanosecs < b.nanosecs) {
		res.secs -= 1;
		res.nanosecs = 1000000000 - (b.nanosecs - a.nanosecs);
	} else {
		res.nanosecs = a.nanosecs - b.nanosecs;
	}
	return res;
}

bool hcc_duration_lt(HccDuration a, HccDuration b) {
	if (a.secs == b.secs) {
		return a.nanosecs < b.nanosecs;
	} else {
		return a.secs < b.secs;
	}
}

bool hcc_duration_gt(HccDuration a, HccDuration b) {
	if (a.secs == b.secs) {
		return a.nanosecs > b.nanosecs;
	} else {
		return a.secs > b.secs;
	}
}

// ===========================================
//
//
// Hashing
//
//
// ===========================================

HccHash32 hcc_hash_fnv_32(const void* data, uintptr_t size, HccHash32 hash) {
	const char* bytes = data;
	const char* bytes_end = bytes + size;
	while (bytes < bytes_end) {
		hash = hash ^ *bytes;
		hash = hash * 0x01000193;
		bytes += 1;
	}

	return hash;
}

HccHash64 hcc_hash_fnv_64(const void* data, uintptr_t size, HccHash64 hash) {
	const char* bytes = data;
	const char* bytes_end = bytes + size;
	while (bytes < bytes_end) {
		hash = hash ^ *bytes;
		hash = hash * 0x00000100000001B3;
		bytes += 1;
	}

	return hash;
}

void hcc_generate_enum_hashes(char* array_name, char** strings, char** enum_strings, uint32_t enums_count) {
	uint32_t used_hashes[128];
	HCC_ASSERT(enums_count <= HCC_ARRAY_COUNT(used_hashes), "internal error: used_hashes needs to be atleast %u", enums_count);

	for (uint32_t idx = 0; idx < enums_count; idx += 1) {
		char* string = strings[idx];
		uint32_t string_size = strlen(string);
		uint32_t hash = hcc_hash_fnv_32(string, string_size, HCC_HASH_FNV_32_INIT);
		for (uint32_t used_idx = 0; used_idx < idx; used_idx += 1) {
			HCC_ASSERT(used_hashes[used_idx] != hash, "internal error: '%s' and '%s' have the same hash of '0x%x'", strings[idx], strings[used_idx]);
		}
		used_hashes[idx] = hash;
	}

	printf("uint32_t %s[] = {\n", array_name);
	for (uint32_t idx = 0; idx < enums_count; idx += 1) {
		char* enum_string = enum_strings[idx];
		uint32_t hash = used_hashes[idx];
		printf("\t[%s] = 0x%x,\n", enum_string, hash);
	}
	printf("};\n");
}

void hcc_generate_hashes(void) {
	// TODO reenabled this once pp directive is back in
	//hcc_generate_enum_hashes("hcc_pp_directive_hashes", hcc_pp_directive_strings, hcc_pp_directive_enum_strings, HCC_PP_DIRECTIVE_COUNT);
	exit(0);
}

uint32_t hcc_string_to_enum_hashed_find(HccString string, uint32_t* enum_hashes, uint32_t enums_count) {
	uint32_t hash = hcc_hash_fnv_32(string.data, string.size, HCC_HASH_FNV_32_INIT);
	for (uint32_t enum_ = 0; enum_ < enums_count; enum_ += 1) {
		if (enum_hashes[enum_] == hash) {
			return enum_;
		}
	}
	return enums_count;
}

// ===========================================
//
//
// Stack
//
//
// ===========================================

#define HCC_DEBUG_ASSERT_STACK(header, elmt_size_) \
	HCC_DEBUG_ASSERT(header, "stack is not initialized"); \
	HCC_DEBUG_ASSERT(header->magic_number == HCC_STACK_MAGIC_NUMBER, "address '%p' is not a stack", header + 1); \
	HCC_DEBUG_ASSERT(header->elmt_size == elmt_size_, "stack element size mismatch. expected '%zu' but got '%zu'", header->elmt_size == elmt_size_)

HccStack(void) _hcc_stack_init(HccAllocTag tag, uintptr_t grow_count, uintptr_t reserve_cap, uintptr_t elmt_size) {
	HCC_DEBUG_ASSERT_NON_ZERO(grow_count);
	HCC_DEBUG_ASSERT_NON_ZERO(reserve_cap);
	HCC_DEBUG_ASSERT_COMMIT_RESERVE_SIZE(grow_count, reserve_cap);
	uintptr_t commit_size = HCC_INT_ROUND_UP_ALIGN(sizeof(HccStackHeader) + grow_count * elmt_size, _hcc_gs.virt_mem_page_size);
	uintptr_t reserve_size = HCC_INT_ROUND_UP_ALIGN(sizeof(HccStackHeader) + reserve_cap * elmt_size, _hcc_gs.virt_mem_reserve_align);
	HccStackHeader* header;
	hcc_virt_mem_reserve(tag, NULL, reserve_size, (void**)&header);
	hcc_virt_mem_commit(tag, header, commit_size, HCC_VIRT_MEM_PROTECTION_READ_WRITE);

	//
	// initialize the header and pass out the where the elements of the array start
	header->count = 0;
	header->cap = grow_count;
	header->grow_count = grow_count;
	header->reserve_cap = reserve_cap;
	header->tag = tag;
#if HCC_ENABLE_DEBUG_ASSERTIONS
	header->magic_number = HCC_STACK_MAGIC_NUMBER;
	header->elmt_size = elmt_size;
#endif

	return header + 1;
}

void _hcc_stack_deinit(HccStack(void) stack, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	uintptr_t reserve_size = HCC_INT_ROUND_UP_ALIGN(sizeof(HccStackHeader) + header->reserve_cap * elmt_size, _hcc_gs.virt_mem_reserve_align);
	hcc_virt_mem_release(header->tag, header, reserve_size);
}

uintptr_t _hcc_stack_resize(HccStack(void) stack, uintptr_t new_count, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	uintptr_t count = header->count;
	if (new_count > header->cap) {
		if (new_count > header->reserve_cap || header->cap == header->reserve_cap) {
			hcc_bail(HCC_ERROR_COLLECTION_FULL, header->tag);
		}

		uintptr_t new_cap = hcc_uint32_round_up_to_multiple(new_count, header->grow_count);
		new_cap = HCC_MIN(new_cap, header->reserve_cap);
		uintptr_t commit_size = HCC_INT_ROUND_UP_ALIGN(sizeof(HccStackHeader) + header->cap * elmt_size, _hcc_gs.virt_mem_page_size);
		uintptr_t new_commit_size = HCC_INT_ROUND_UP_ALIGN(sizeof(HccStackHeader) + new_cap * elmt_size, _hcc_gs.virt_mem_page_size);
		if (commit_size < new_commit_size) {
			void* old_end_ptr = HCC_PTR_ADD(header, commit_size);
			hcc_virt_mem_commit(header->tag, old_end_ptr, new_commit_size - commit_size, HCC_VIRT_MEM_PROTECTION_READ_WRITE);
		}
		atomic_store(&header->cap, new_cap);
	}
	atomic_store(&header->count, new_count);
	return count;
}

void* _hcc_stack_insert_many(HccStack(void) stack, uintptr_t idx, uintptr_t amount, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	HCC_DEBUG_ASSERT(idx <= header->count, "insert index '%zu' must be less than or equal to count of '%zu'", idx, header->count);

	//
	// extend the count and ensure we don't exceed the capacity
	uintptr_t new_count = header->count + amount;
	uintptr_t count = _hcc_stack_resize(stack, new_count, elmt_size);

	//
	// shift the elements from idx to (idx + amount), to the right to make room for the elements
	void* dst = HCC_PTR_ADD(stack, idx * elmt_size);
	memmove(HCC_PTR_ADD(dst, amount * elmt_size), dst, (count - idx) * elmt_size);

	return dst;
}

uintptr_t _hcc_stack_push_many(HccStack(void) stack, uintptr_t amount, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);

	//
	// extend the count and ensure we don't exceed the capacity
	uintptr_t new_count = header->count + amount;
	uintptr_t insert_idx = _hcc_stack_resize(stack, new_count, elmt_size);
	return insert_idx;
}

uintptr_t _hcc_stack_push_many_thread_safe(HccStack(void) stack, uintptr_t amount, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	//
	// TODO inline _hcc_stack_push_many and _hcc_stack_resize and only
	// lock when we are waiting on another thread to commit more memory.
	//
	hcc_spin_mutex_lock(&header->push_thread_safe_mutex);
	uintptr_t insert_idx = _hcc_stack_push_many(stack, amount, elmt_size);
	hcc_spin_mutex_unlock(&header->push_thread_safe_mutex);
	return insert_idx;
}

void _hcc_stack_pop_many(HccStack(void) stack, uintptr_t amount, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	HCC_DEBUG_ASSERT(amount <= header->count, "cannot pop '%zu' many elements when the array has a count of '%zu'", amount, header->count);
	header->count -= amount;
}

void _hcc_stack_remove_swap_many(HccStack(void) stack, uintptr_t idx, uintptr_t amount, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	HCC_DEBUG_ASSERT(idx <= header->count, "idx '%zu' must be less than or equal to count of '%zu'", idx, header->count);
	uintptr_t end_idx = idx + amount;
	HCC_DEBUG_ASSERT(end_idx <= header->count, "(idx + amount) '%zu' must be less than or equal to count of '%zu'", end_idx, header->count);

	//
	// get the pointer to the elements that are being removed
	void* dst = HCC_PTR_ADD(stack, idx * elmt_size);

	//
	// get the pointer to the elements at the back of the stack that will be moved
	uintptr_t src_idx = header->count;
	header->count -= amount;
	amount = HCC_MIN(amount, header->count);
	src_idx -= amount;

	//
	// replace the removed elements with the elements from the back of the stack
	void* src = HCC_PTR_ADD(stack, src_idx * elmt_size);
	memmove(dst, src, amount * elmt_size);
}

void _hcc_stack_remove_shift_many(HccStack(void) stack, uintptr_t idx, uintptr_t amount, uintptr_t elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT(idx <= header->count, "idx '%zu' must be less than or equal to count of '%zu'", idx, header->count);
	uintptr_t end_idx = idx + amount;
	HCC_DEBUG_ASSERT(end_idx <= header->count, "(idx + amount) '%zu' must be less than or equal to count of '%zu'", end_idx, header->count);

	//
	// get the pointer to the elements that are being removed
	void* dst = HCC_PTR_ADD(stack, idx * elmt_size);

	//
	// now replace the elements by shifting the elements to the right over them.
	if (end_idx < header->count) {
		void* src = HCC_PTR_ADD(dst, amount * elmt_size);
		memmove(dst, src, (header->count - (idx + amount)) * elmt_size);
	}
	header->count -= amount;
}

void hcc_stack_push_char(HccStack(char) stack, char ch) {
	*hcc_stack_push(stack) = ch;
}

HccString hcc_stack_push_string(HccStack(char) stack, HccString string) {
	char* ptr = hcc_stack_push_many(stack, string.size);
	memcpy(ptr, string.data, string.size);
	return hcc_string(ptr, string.size);
}

HccString hcc_stack_push_string_fmtv(HccStack(char) stack, char* fmt, va_list args) {
	va_list args_copy;
	va_copy(args_copy, args);

	// add 1 so we have enough room for the null terminator that vsnprintf always outputs
	// vsnprintf will return -1 on an encoding error.
	uintptr_t size = vsnprintf(NULL, 0, fmt, args_copy) + 1;
	va_end(args_copy);
	HCC_DEBUG_ASSERT(size >= 1, "a vsnprintf encoding error has occurred");

	//
	// resize the stack to have enough room to store the pushed formatted string
	uintptr_t new_count = hcc_stack_count(stack) + size;
	uintptr_t insert_idx = hcc_stack_resize(stack, new_count);

	//
	// now call vsnprintf for real this time, with a buffer
	// to actually copy the formatted string.
	char* ptr = &stack[insert_idx];
	vsnprintf(ptr, size, fmt, args);
	return hcc_string(ptr, size);
}

HccString hcc_stack_push_string_fmt(HccStack(char) stack, char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	HccString string = hcc_stack_push_string_fmtv(stack, fmt, args);
	va_end(args);
	return string;
}

// ===========================================
//
//
// Deque
//
//
// ===========================================

#define HCC_ASSERT_DEQUE(header, elmt_size_) \
	HCC_DEBUG_ASSERT(header->magic_number == HCC_DEQUE_MAGIC_NUMBER, "address '%p' is not a deque", header + 1); \
	HCC_DEBUG_ASSERT(header->elmt_size == elmt_size_, "deque element size mismatch. expected '%zu' but got '%zu'", header->elmt_size == elmt_size_)

HccDeque(void) _hcc_deque_init(HccAllocTag tag, uintptr_t cap, uintptr_t elmt_size) {
	HCC_DEBUG_ASSERT_POWER_OF_TWO(cap);
	uintptr_t size = HCC_INT_ROUND_UP_ALIGN(sizeof(HccDequeHeader) + cap * elmt_size, _hcc_gs.virt_mem_reserve_align);
	HccDequeHeader* header;
	hcc_virt_mem_reserve_commit(tag, NULL, size, HCC_VIRT_MEM_PROTECTION_READ_WRITE, (void**)&header);

	//
	// initialize the header and pass out the where the elements of the array start
	header->front_idx = 0;
	header->back_idx = 0;
	header->cap = cap;
	header->tag = tag;
#if HCC_ENABLE_DEBUG_ASSERTIONS
	header->magic_number = HCC_DEQUE_MAGIC_NUMBER;
	header->elmt_size = elmt_size;
#endif

	return header + 1;
}

void _hcc_deque_deinit(HccDeque(void) deque, uintptr_t elmt_size) {
	HccDequeHeader* header = hcc_deque_header(deque);
	HCC_ASSERT_DEQUE(header, elmt_size);
	uintptr_t size = HCC_INT_ROUND_UP_ALIGN(sizeof(HccDequeHeader) + header->cap * elmt_size, _hcc_gs.virt_mem_reserve_align);
	hcc_virt_mem_release(header->tag, header, size);
}

void _hcc_deque_read(HccDeque(void) deque, uintptr_t idx, uintptr_t amount, void* elmts_out, uintptr_t elmt_size) {
	HccDequeHeader* header = hcc_deque_header(deque);
	HCC_ASSERT_DEQUE(header, elmt_size);
	uintptr_t count = hcc_deque_count(&header);
	HCC_DEBUG_ASSERT_NON_ZERO(amount);
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(idx + amount - 1, count);

	idx = HCC_WRAPPING_ADD(header->front_idx, idx, header->cap);

	if (header->cap < idx + amount) {
		//
		// there is enough elements that the read will
		// exceed the back and cause the idx to loop around.
		// so copy in two parts
		uintptr_t rem_count = header->cap - idx;
		// copy to the end of the buffer
		memcpy(elmts_out, HCC_PTR_ADD(deque, idx * elmt_size), rem_count * elmt_size);
		// copy to the beginning of the buffer
		memcpy(HCC_PTR_ADD(elmts_out, rem_count * elmt_size), deque, ((amount - rem_count) * elmt_size));
	} else {
		//
		// coping the elements can be done in a single copy
		memcpy(elmts_out, HCC_PTR_ADD(deque, idx * elmt_size), amount * elmt_size);
	}
}

void _hcc_deque_write(HccDeque(void) deque, uintptr_t idx, void* elmts, uintptr_t amount, uintptr_t elmt_size) {
	HccDequeHeader* header = hcc_deque_header(deque);
	HCC_ASSERT_DEQUE(header, elmt_size);
	uintptr_t count = hcc_deque_count(&header);
	HCC_DEBUG_ASSERT_NON_ZERO(amount);
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(idx + amount - 1, count);

	idx = HCC_WRAPPING_ADD(header->front_idx, idx, header->cap);

	if (header->cap < idx + amount) {
		//
		// there is enough elements that the write will
		// exceed the back and cause the idx to loop around.
		// so copy in two parts
		uintptr_t rem_count = header->cap - idx;
		// copy to the end of the buffer
		memcpy(HCC_PTR_ADD(deque, idx * elmt_size), elmts, rem_count * elmt_size);
		// copy to the beginning of the buffer
		memcpy(deque, HCC_PTR_ADD(elmts, rem_count * elmt_size), ((amount - rem_count) * elmt_size));
	} else {
		//
		// coping the elements can be done in a single copy
		memcpy(HCC_PTR_ADD(deque, idx * elmt_size), elmts, amount * elmt_size);
	}
}

uintptr_t _hcc_deque_push_front_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size) {
	HccDequeHeader* header = hcc_deque_header(deque);
	HCC_ASSERT_DEQUE(header, elmt_size);
	HCC_DEBUG_ASSERT_NON_ZERO(amount);
	uintptr_t new_count = hcc_deque_count(deque) + amount;
	if (new_count > header->cap) {
		hcc_bail(HCC_ERROR_COLLECTION_FULL, header->tag);
	}

	header->front_idx = HCC_WRAPPING_SUB(header->front_idx, amount, header->cap);
	return 0;
}

uintptr_t _hcc_deque_push_back_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size) {
	HccDequeHeader* header = hcc_deque_header(deque);
	HCC_ASSERT_DEQUE(header, elmt_size);
	HCC_DEBUG_ASSERT_NON_ZERO(amount);
	uintptr_t insert_idx = hcc_deque_count(deque);
	uintptr_t new_count = insert_idx + amount;
	if (new_count > header->cap) {
		hcc_bail(HCC_ERROR_COLLECTION_FULL, header->tag);
	}

	header->back_idx = HCC_WRAPPING_ADD(header->back_idx, amount, header->cap);
	return insert_idx;
}

void _hcc_deque_pop_front_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size) {
	HccDequeHeader* header = hcc_deque_header(deque);
	HCC_ASSERT_DEQUE(header, elmt_size);
	uintptr_t count = hcc_deque_count(deque);
	HCC_DEBUG_ASSERT(amount <= count, "cannot pop '%zu' many elements when the array has a count of '%zu'", amount, count);
	header->front_idx = HCC_WRAPPING_ADD(header->front_idx, amount, header->cap);
}

void _hcc_deque_pop_back_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size) {
	HccDequeHeader* header = hcc_deque_header(deque);
	HCC_ASSERT_DEQUE(header, elmt_size);
	uintptr_t count = hcc_deque_count(deque);
	HCC_DEBUG_ASSERT(amount <= count, "cannot pop '%zu' many elements when the array has a count of '%zu'", amount, count);
	header->back_idx = HCC_WRAPPING_SUB(header->back_idx, amount, header->cap);
}

// ===========================================
//
//
// Hash Table
//
//
// ===========================================

#define HCC_DEBUG_ASSERT_HASH_TABLE(header, elmt_size_) \
	HCC_DEBUG_ASSERT(header->magic_number == HCC_HASH_TABLE_MAGIC_NUMBER, "address '%p' is not a hash table", header + 1); \
	HCC_DEBUG_ASSERT(header->elmt_size == elmt_size_, "hash table element size mismatch. expected '%zu' but got '%zu'", header->elmt_size == elmt_size_)

enum {
	HCC_HASH_TABLE_HASH_EMPTY =        0,
	HCC_HASH_TABLE_HASH_TOMBSTONE =    1,
	HCC_HASH_TABLE_HASH_IS_INSERTING = 2,
	HCC_HASH_TABLE_HASH_START =        3,
};

#if UINTPTR_MAX == 0xFFFFFFFF
#define HCC_HASH_TABLE_BUCKET_ENTRIES_COUNT 16
#define HCC_HASH_TABLE_BUCKET_ENTRIES_SHIFT 4
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define HCC_HASH_TABLE_BUCKET_ENTRIES_COUNT 8
#define HCC_HASH_TABLE_BUCKET_ENTRIES_SHIFT 3
#endif
static_assert(HCC_HASH_TABLE_BUCKET_ENTRIES_COUNT * sizeof(HccHash) % HCC_CACHE_LINE_SIZE == 0, "bucket must be a multiple of a cache line");

HccHashTable(void) _hcc_hash_table_init(HccAllocTag tag, HccHashTableKeyCmpFn key_cmp_fn, HccHashTableKeyHashFn key_hash_fn, uintptr_t cap, uintptr_t elmt_size) {
	HCC_DEBUG_ASSERT_POWER_OF_TWO(cap);
	uintptr_t size = HCC_INT_ROUND_UP_ALIGN(HCC_INT_ROUND_UP_ALIGN(sizeof(HccHashTableHeader) + (cap * elmt_size), HCC_CACHE_LINE_ALIGN) + (cap * sizeof(HccHash)), _hcc_gs.virt_mem_reserve_align);
	HccHashTableHeader* header;
	hcc_virt_mem_reserve_commit(tag, NULL, size, HCC_VIRT_MEM_PROTECTION_READ_WRITE, (void**)&header);

	//
	// initialize the header and pass out the where the elements of the array start
	header->count = 0;
	header->cap = cap;
	header->key_cmp_fn = key_cmp_fn;
	header->key_hash_fn = key_hash_fn;
	header->hashes = HCC_PTR_ROUND_UP_ALIGN(HCC_PTR_ADD((header + 1), cap * elmt_size), HCC_CACHE_LINE_ALIGN);
	header->tag = tag;
#if HCC_ENABLE_DEBUG_ASSERTIONS
	header->magic_number = HCC_HASH_TABLE_MAGIC_NUMBER;
	header->elmt_size = elmt_size;
#endif

	HccHashTable(void) table = header + 1;
	_hcc_hash_table_clear(table, elmt_size);
	return table;
}

void _hcc_hash_table_deinit(HccHashTable(void) table, uintptr_t elmt_size) {
	HccHashTableHeader* header = hcc_hash_table_header(table);
	HCC_DEBUG_ASSERT_HASH_TABLE(header, elmt_size);
	uintptr_t size = HCC_INT_ROUND_UP_ALIGN(HCC_INT_ROUND_UP_ALIGN(sizeof(HccHashTableHeader) + (header->cap * elmt_size), HCC_CACHE_LINE_ALIGN) + (header->cap * sizeof(HccHash)), _hcc_gs.virt_mem_reserve_align);
	hcc_virt_mem_release(header->tag, header, size);
}

void _hcc_hash_table_clear(HccHashTable(void) table, uintptr_t elmt_size) {
	HccHashTableHeader* header = hcc_hash_table_header(table);
	HCC_DEBUG_ASSERT_HASH_TABLE(header, elmt_size);
	uintptr_t size = HCC_INT_ROUND_UP_ALIGN(HCC_INT_ROUND_UP_ALIGN(sizeof(HccHashTableHeader) + (header->cap * elmt_size), HCC_CACHE_LINE_ALIGN) + (header->cap * sizeof(HccHash)), _hcc_gs.virt_mem_reserve_align);
	HccHashTableHeader h = *header;
	hcc_virt_mem_reset(header->tag, header, size);
	*header = h;
	header->count = 0;
}

uintptr_t _hcc_hash_table_find_idx(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size) {
	HccHashTableHeader* header = hcc_hash_table_header(table);
	HCC_DEBUG_ASSERT_HASH_TABLE(header, elmt_size);

	//
	// hash the key and ensure it is no one of the special marker hash values.
	HccHash hash = header->key_hash_fn(key, key_size);
	if (hash < HCC_HASH_TABLE_HASH_START) hash += HCC_HASH_TABLE_HASH_START;

	//
	// divide the entry capacity by the number of entries in a bucket
	// and see what bucket we should start our search in.
	uintptr_t buckets_count = header->cap >> HCC_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
	uintptr_t bucket_idx = hash & (buckets_count - 1);

	uintptr_t step = 1;
	HccAtomic(HccHash)* hashes = header->hashes;
	while (1) {
		//
		// multiply by the number of entries in a bucket to get the position in the entry arrays.
		uintptr_t bucket_entry_start_idx = bucket_idx << HCC_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
		for (uintptr_t entry_idx = bucket_entry_start_idx; entry_idx < bucket_entry_start_idx + HCC_HASH_TABLE_BUCKET_ENTRIES_COUNT; entry_idx += 1) {
			HccHash existing_hash;
			while ((existing_hash = atomic_load(&hashes[entry_idx])) == HCC_HASH_TABLE_HASH_IS_INSERTING) {
				HCC_CPU_RELAX();
			}

			if (existing_hash == hash) {
				//
				// hash matches, check if the key matches
				void* entry_ptr = HCC_PTR_ADD(table, entry_idx * elmt_size);
				if (header->key_cmp_fn(key, entry_ptr, key_size)) {
					// key matches, success!
					return entry_idx;
				}
			} else if (existing_hash == HCC_HASH_TABLE_HASH_EMPTY) {
				// found an empty hash, no other entries have been inserted past this point
				return UINTPTR_MAX;
			}
		}

		//
		// quadratic probing to help reduce clustering of entries
		bucket_idx += step;
		step += 1;
		bucket_idx &= (buckets_count - 1);
	}
}

HccHashTableInsert _hcc_hash_table_find_insert_idx(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size) {
	HccHashTableHeader* header = hcc_hash_table_header(table);
	HCC_DEBUG_ASSERT_HASH_TABLE(header, elmt_size);
	if (atomic_load(&header->count) >= header->cap) {
		hcc_bail(HCC_ERROR_COLLECTION_FULL, header->tag);
	}

	//
	// hash the key and ensure it is no one of the special marker hash values.
	HccHash hash = header->key_hash_fn(key, key_size);
	if (hash < HCC_HASH_TABLE_HASH_START) hash += HCC_HASH_TABLE_HASH_START;

	//
	// divide the entry capacity by the number of entries in a bucket
	// and see what bucket we should start our search in.
	uintptr_t buckets_count = header->cap >> HCC_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
	uintptr_t bucket_idx = hash & (buckets_count - 1);

	uintptr_t step = 1;
	HccAtomic(HccHash)* hashes = header->hashes;
	while (1) {
		//
		// multiply by the number of entries in a bucket to get the position in the entry arrays.
		uintptr_t bucket_entry_start_idx = bucket_idx << HCC_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
		for (uintptr_t entry_idx = bucket_entry_start_idx; entry_idx < bucket_entry_start_idx + HCC_HASH_TABLE_BUCKET_ENTRIES_COUNT; entry_idx += 1) {
TRY_THIS_HASH_AGAIN: {}
			HccHash existing_hash;
			while ((existing_hash = atomic_load(&hashes[entry_idx])) == HCC_HASH_TABLE_HASH_IS_INSERTING) {
				HCC_CPU_RELAX();
			}

			if (existing_hash == hash) {
				// hash matches, check if the key matches
				void* entry_ptr = HCC_PTR_ADD(table, entry_idx * elmt_size);
				if (header->key_cmp_fn(key, entry_ptr, key_size)) {
					// key matches, success!
					return (HccHashTableInsert) { .idx = entry_idx, .is_new = false };
				}
			} else if (existing_hash == HCC_HASH_TABLE_HASH_EMPTY || existing_hash == HCC_HASH_TABLE_HASH_TOMBSTONE) {
				//
				// found an empty or tombstone hash that we can use for our value.
				// but lets fight all the other threads to see if we can take this hash first.
				do {
					if (atomic_compare_exchange_weak(&hashes[entry_idx], &existing_hash, HCC_HASH_TABLE_HASH_IS_INSERTING)) {
						//
						// we won against the other threads, so claim the slot.
						void* entry_ptr = HCC_PTR_ADD(table, entry_idx * elmt_size);
						atomic_fetch_add(&header->count, 1);
						memcpy(entry_ptr, key, key_size ? key_size : sizeof(HccString));
						atomic_store(&hashes[entry_idx], hash);
						return (HccHashTableInsert){ .idx = entry_idx, .is_new = true };
					}
				} while (existing_hash == HCC_HASH_TABLE_HASH_EMPTY || existing_hash == HCC_HASH_TABLE_HASH_TOMBSTONE);

				//
				// another thread stole our entry slot,
				// lets try again to see if it inserted the hash we are looking for.
				goto TRY_THIS_HASH_AGAIN;
			}
		}

		//
		// quadratic probing to help reduce clustering of entries
		bucket_idx += step;
		step += 1;
		bucket_idx &= (buckets_count - 1);
	}

}

bool _hcc_hash_table_remove(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size) {
	uintptr_t entry_idx = _hcc_hash_table_find_idx(table, key, key_size, elmt_size);
	if (entry_idx == UINTPTR_MAX) {
		return false;
	}

	HccHashTableHeader* header = hcc_hash_table_header(table);
	atomic_store(&header->hashes[entry_idx], HCC_HASH_TABLE_HASH_TOMBSTONE);

	return true;
}

bool hcc_u32_key_cmp(void* a, void* b, uintptr_t size) {
	HCC_UNUSED(size);
	return *(uint32_t*)a == *(uint32_t*)b;
}

bool hcc_u64_key_cmp(void* a, void* b, uintptr_t size) {
	HCC_UNUSED(size);
	return *(uint64_t*)a == *(uint64_t*)b;
}

HccHash hcc_data_key_hash(void* key, uintptr_t size) {
	return hcc_hash_fnv(key, size, HCC_HASH_FNV_INIT);
}

HccHash hcc_string_key_hash(void* key, uintptr_t size) {
	HCC_UNUSED(size);

	HccString string = *(HccString*)key;
	return hcc_hash_fnv(string.data, string.size, HCC_HASH_FNV_INIT);
}

HccHash hcc_u32_key_hash(void* key, uintptr_t size) {
	HCC_DEBUG_ASSERT(size == sizeof(uint32_t), "key is not a uint32_t");
	return *(uint32_t*)key;
}

HccHash hcc_u64_key_hash(void* key, uintptr_t size) {
	HCC_DEBUG_ASSERT(size == sizeof(uint64_t), "key is not a uint64_t");
	return *(uint64_t*)key;
}

// ===========================================
//
//
// AML Intrinsic Type
//
//
// ===========================================

const char* hcc_aml_intrinsic_data_type_scalar_strings[HCC_AML_INTRINSIC_DATA_TYPE_SCALAR_COUNT] = {
	[HCC_AML_INTRINSIC_DATA_TYPE_VOID] = "void",
	[HCC_AML_INTRINSIC_DATA_TYPE_BOOL] = "bool",
	[HCC_AML_INTRINSIC_DATA_TYPE_S8] = "s8",
	[HCC_AML_INTRINSIC_DATA_TYPE_S16] = "s16",
	[HCC_AML_INTRINSIC_DATA_TYPE_S32] = "s32",
	[HCC_AML_INTRINSIC_DATA_TYPE_S64] = "s64",
	[HCC_AML_INTRINSIC_DATA_TYPE_U8] = "u8",
	[HCC_AML_INTRINSIC_DATA_TYPE_U16] = "u16",
	[HCC_AML_INTRINSIC_DATA_TYPE_U32] = "u32",
	[HCC_AML_INTRINSIC_DATA_TYPE_U64] = "u64",
	[HCC_AML_INTRINSIC_DATA_TYPE_F16] = "f16",
	[HCC_AML_INTRINSIC_DATA_TYPE_F32] = "f32",
	[HCC_AML_INTRINSIC_DATA_TYPE_F64] = "f64",
};

uint8_t hcc_aml_intrinsic_data_type_scalar_size_aligns[HCC_AML_INTRINSIC_DATA_TYPE_SCALAR_COUNT] = {
	[HCC_AML_INTRINSIC_DATA_TYPE_VOID] = 0,
	[HCC_AML_INTRINSIC_DATA_TYPE_BOOL] = 1,
	[HCC_AML_INTRINSIC_DATA_TYPE_S8] = 1,
	[HCC_AML_INTRINSIC_DATA_TYPE_S16] = 2,
	[HCC_AML_INTRINSIC_DATA_TYPE_S32] = 4,
	[HCC_AML_INTRINSIC_DATA_TYPE_S64] = 8,
	[HCC_AML_INTRINSIC_DATA_TYPE_U8] = 1,
	[HCC_AML_INTRINSIC_DATA_TYPE_U16] = 2,
	[HCC_AML_INTRINSIC_DATA_TYPE_U32] = 4,
	[HCC_AML_INTRINSIC_DATA_TYPE_U64] = 8,
	[HCC_AML_INTRINSIC_DATA_TYPE_F16] = 2,
	[HCC_AML_INTRINSIC_DATA_TYPE_F32] = 4,
	[HCC_AML_INTRINSIC_DATA_TYPE_F64] = 8,
};

// ===========================================
//
//
// AML Scalar Data Type Mask
//
//
// ===========================================

HccString hcc_aml_scalar_data_type_mask_string(HccAMLScalarDataTypeMask mask) {
	static char buf[512];
	uint32_t size = 0;
	while (mask) {
		HccAMLIntrinsicDataType intrinsic_type = hcc_leastsetbitidx32(mask);
		const char* str;
		switch (intrinsic_type) {
			case HCC_AML_INTRINSIC_DATA_TYPE_VOID: str = "void"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: str = "bool"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S8: str = "s8"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S16: str = "s16"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S32: str = "s32"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S64: str = "s64"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U8: str = "u8"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U16: str = "u16"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U32: str = "u32"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U64: str = "u64"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F16: str = "f16"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F32: str = "f32"; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F64: str = "f64"; break;
		}

		size += snprintf(buf + size, sizeof(buf) - size, size ? " %s" : "%s", str);
		mask &= ~(1 << intrinsic_type);
	}

	return hcc_string(buf, size);
}

// ===========================================
//
//
// Resource Data Type
//
//
// ===========================================

const char* hcc_texture_dim_strings_lower[HCC_TEXTURE_DIM_COUNT] = {
	[HCC_TEXTURE_DIM_1D] = "1d",
	[HCC_TEXTURE_DIM_2D] = "2d",
	[HCC_TEXTURE_DIM_3D] = "3d",
	[HCC_TEXTURE_DIM_CUBE] = "cube",
};

const char* hcc_texture_dim_strings_upper[HCC_TEXTURE_DIM_COUNT] = {
	[HCC_TEXTURE_DIM_1D] = "1D",
	[HCC_TEXTURE_DIM_2D] = "2D",
	[HCC_TEXTURE_DIM_3D] = "3D",
	[HCC_TEXTURE_DIM_CUBE] = "CUBE",
};

const char* hcc_resource_access_mode_short_strings_lower[HCC_RESOURCE_ACCESS_MODE_COUNT] = {
	[HCC_RESOURCE_ACCESS_MODE_READ_ONLY] = "ro",
	[HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY] = "wo",
	[HCC_RESOURCE_ACCESS_MODE_READ_WRITE] = "rw",
	[HCC_RESOURCE_ACCESS_MODE_SAMPLE] = "sample",
};

const char* hcc_resource_access_mode_short_strings_upper[HCC_RESOURCE_ACCESS_MODE_COUNT] = {
	[HCC_RESOURCE_ACCESS_MODE_READ_ONLY] = "RO",
	[HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY] = "WO",
	[HCC_RESOURCE_ACCESS_MODE_READ_WRITE] = "RW",
	[HCC_RESOURCE_ACCESS_MODE_SAMPLE] = "SAMPLE",
};

const char* hcc_resource_access_mode_short_strings_title[HCC_RESOURCE_ACCESS_MODE_COUNT] = {
	[HCC_RESOURCE_ACCESS_MODE_READ_ONLY] = "Ro",
	[HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY] = "Wo",
	[HCC_RESOURCE_ACCESS_MODE_READ_WRITE] = "Rw",
	[HCC_RESOURCE_ACCESS_MODE_SAMPLE] = "Sample",
};

// ===========================================
//
//
// Declarations
//
//
// ===========================================

HccDecl hcc_decl_resolve_and_keep_qualifiers(HccCU* cu, HccDecl decl) {
	uint32_t qualifiers_mask = decl & HCC_DATA_TYPE_QUALIFIERS_MASK;
	decl = hcc_decl_resolve_and_strip_qualifiers(cu, decl);
	return decl | qualifiers_mask;
}

HccDecl hcc_decl_resolve_and_strip_qualifiers(HccCU* cu, HccDecl decl) {
	decl = HCC_DATA_TYPE_STRIP_QUALIFIERS(decl);
	while (1) {
		if (HCC_DECL_IS_FORWARD_DECL(decl)) {
			HccHashTable(HccDeclEntry) declarations;
			HccASTForwardDecl* forward_decl = hcc_ast_forward_decl_get(cu, decl);
			HccASTFile* ast_file = forward_decl->ast_file;
			switch (HCC_DECL_TYPE(decl)) {
				case HCC_DECL_FUNCTION:
				case HCC_DECL_GLOBAL_VARIABLE: declarations = ast_file->global_declarations; break;
				case HCC_DATA_TYPE_STRUCT: declarations = ast_file->struct_declarations; break;
				case HCC_DATA_TYPE_UNION: declarations = ast_file->union_declarations; break;
				default: HCC_ABORT("decl type '%u' does not have forward declarations", HCC_DECL_TYPE(decl));
			}

			uintptr_t found_idx = hcc_hash_table_find_idx(declarations, &forward_decl->identifier_string_id);
			if (found_idx == UINTPTR_MAX) {
				return decl;
			}

			HccDecl found_decl = declarations[found_idx].decl;
			if (found_decl == decl) {
				return decl;
			}
			decl = found_decl;
		} else {
			switch (HCC_DECL_TYPE(decl)) {
				case HCC_DATA_TYPE_ENUM:
					return HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_SINT);
				case HCC_DATA_TYPE_TYPEDEF: {
					HccTypedef* typedef_ = hcc_typedef_get(cu, decl);
					decl = typedef_->aliased_data_type;
					break;
				};
				default:
					return decl;
			}
		}
	}
}

HccDataType hcc_decl_function_data_type(HccCU* cu, HccDecl decl) {
	switch (HCC_DECL_TYPE(decl)) {
		case HCC_DECL_FUNCTION:
			return HCC_DECL_IS_FORWARD_DECL(decl) ? hcc_ast_forward_decl_get(cu, decl)->function.function_data_type : hcc_ast_function_get(cu, decl)->function_data_type;
		case HCC_DATA_TYPE_FUNCTION:
			return decl;
		default:
			return 0;
	}
}

HccDecl hcc_decl_return_data_type(HccCU* cu, HccDecl decl) {
	switch (HCC_DECL_TYPE(decl)) {
			return HCC_DECL_IS_FORWARD_DECL(decl) ? hcc_ast_forward_decl_get(cu, decl)->function.return_data_type : hcc_ast_function_get(cu, decl)->return_data_type;
		case HCC_DECL_FUNCTION:
			return HCC_DECL_IS_FORWARD_DECL(decl) ? hcc_ast_forward_decl_get(cu, decl)->function.return_data_type : hcc_ast_function_get(cu, decl)->return_data_type;
		case HCC_DECL_ENUM_VALUE:
			return HCC_DATA_TYPE_AST_BASIC_SINT;
		case HCC_DECL_GLOBAL_VARIABLE:
			return HCC_DECL_IS_FORWARD_DECL(decl) ? hcc_ast_forward_decl_get(cu, decl)->variable.data_type : hcc_ast_global_variable_get(cu, decl)->data_type;
		case HCC_DATA_TYPE_FUNCTION:
			return hcc_function_data_type_get(cu, decl)->return_data_type;
		default:
			if (HCC_DECL_IS_DATA_TYPE(decl)) {
				return decl;
			}
			return 0;
	}
}

uint8_t hcc_decl_function_params_count(HccCU* cu, HccDecl decl) {
	switch (HCC_DECL_TYPE(decl)) {
		case HCC_DECL_FUNCTION:
			return HCC_DECL_IS_FORWARD_DECL(decl) ? hcc_ast_forward_decl_get(cu, decl)->function.params_count : hcc_ast_function_get(cu, decl)->params_count;
		case HCC_DATA_TYPE_FUNCTION:
			return hcc_function_data_type_get(cu, decl)->params_count;
		default:
			return 0;
	}
}

HccASTVariable* hcc_decl_function_params(HccCU* cu, HccDecl decl) {
	switch (HCC_DECL_TYPE(decl)) {
		case HCC_DECL_FUNCTION:
			return HCC_DECL_IS_FORWARD_DECL(decl) ? hcc_ast_forward_decl_get(cu, decl)->function.params : hcc_ast_function_get(cu, decl)->params_and_variables;
		default:
			return 0;
	}
}

HccShaderStage hcc_decl_function_shader_stage(HccCU* cu, HccDecl decl) {
	switch (HCC_DECL_TYPE(decl)) {
		case HCC_DECL_FUNCTION:
			return HCC_DECL_IS_FORWARD_DECL(decl) ? HCC_SHADER_STAGE_NONE : hcc_ast_function_get(cu, decl)->shader_stage;
		default:
			return 0;
	}
}

HccStringId hcc_decl_identifier_string_id(HccCU* cu, HccDecl decl) {
	if (HCC_DECL_IS_FORWARD_DECL(decl)) {
		return hcc_ast_forward_decl_get(cu, decl)->identifier_string_id;
	}

	switch (HCC_DECL_TYPE(decl)) {
		case HCC_DECL_FUNCTION:
			return hcc_ast_function_get(cu, decl)->identifier_string_id;
		case HCC_DECL_GLOBAL_VARIABLE:
			return hcc_ast_global_variable_get(cu, decl)->identifier_string_id;
		case HCC_DECL_ENUM_VALUE:
			return hcc_enum_value_get(cu, decl)->identifier_string_id;
		default:
			return HccStringId(0);
	}
}

HccLocation* hcc_decl_location(HccCU* cu, HccDecl decl) {
	if (HCC_DECL_IS_FORWARD_DECL(decl)) {
		return hcc_ast_forward_decl_get(cu, decl)->identifier_location;
	}

	switch (HCC_DECL_TYPE(decl)) {
		case HCC_DECL_FUNCTION:
			return hcc_ast_function_get(cu, decl)->identifier_location;
		case HCC_DECL_GLOBAL_VARIABLE:
			return hcc_ast_global_variable_get(cu, decl)->identifier_location;
		case HCC_DECL_ENUM_VALUE:
			return hcc_enum_value_get(cu, decl)->identifier_location;
		default:
			if (HCC_DECL_IS_DATA_TYPE(decl)) {
				return hcc_data_type_location(cu, (HccDataType)decl);
			}
			return NULL;
	}
}

// ===========================================
//
//
// Data Type Table
//
//
// ===========================================

void hcc_data_type_table_init(HccCU* cu, HccCUSetup* setup) {
	cu->dtt.arrays = hcc_stack_init(HccArrayDataType, HCC_ALLOC_TAG_DATA_TYPE_TABLE_ARRAYS, setup->dtt.arrays_grow_count, setup->dtt.arrays_reserve_cap);
	cu->dtt.compounds = hcc_stack_init(HccCompoundDataType, HCC_ALLOC_TAG_DATA_TYPE_TABLE_COMPOUNDS, setup->dtt.compounds_grow_count, setup->dtt.compounds_reserve_cap);
	cu->dtt.compound_fields = hcc_stack_init(HccCompoundField, HCC_ALLOC_TAG_DATA_TYPE_TABLE_COMPOUND_FIELDS, setup->dtt.compound_fields_grow_count, setup->dtt.compound_fields_reserve_cap);
	cu->dtt.typedefs = hcc_stack_init(HccTypedef, HCC_ALLOC_TAG_DATA_TYPE_TABLE_TYPEDEFS, setup->dtt.typedefs_grow_count, setup->dtt.typedefs_reserve_cap);
	cu->dtt.enums = hcc_stack_init(HccEnumDataType, HCC_ALLOC_TAG_DATA_TYPE_TABLE_ENUMS, setup->dtt.enums_grow_count, setup->dtt.enums_reserve_cap);
	cu->dtt.enum_values = hcc_stack_init(HccEnumValue, HCC_ALLOC_TAG_DATA_TYPE_TABLE_ENUM_VALUES, setup->dtt.enum_values_grow_count, setup->dtt.enum_values_reserve_cap);
	cu->dtt.pointers = hcc_stack_init(HccPointerDataType, HCC_ALLOC_TAG_DATA_TYPE_TABLE_POINTERS, setup->dtt.pointers_grow_count, setup->dtt.pointers_reserve_cap);
	cu->dtt.functions = hcc_stack_init(HccFunctionDataType, HCC_ALLOC_TAG_DATA_TYPE_TABLE_FUNCTIONS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->dtt.function_params = hcc_stack_init(HccDataType, HCC_ALLOC_TAG_DATA_TYPE_TABLE_FUNCTION_PARAMS, setup->function_params_and_variables_grow_count, setup->function_params_and_variables_reserve_cap);
	cu->dtt.buffers = hcc_stack_init(HccBufferDataType, HCC_ALLOC_TAG_DATA_TYPE_TABLE_BUFFERS, setup->dtt.buffers_grow_count, setup->dtt.buffers_reserve_cap);
	cu->dtt.arrays_dedup_hash_table = hcc_hash_table_init(HccDataTypeDedupEntry, HCC_ALLOC_TAG_DATA_TYPE_TABLE_ARRAYS_DEDUP_HASH_TABLE, hcc_u64_key_cmp, hcc_u64_key_hash, setup->dtt.arrays_reserve_cap);
	cu->dtt.pointers_dedup_hash_table = hcc_hash_table_init(HccDataTypeDedupEntry, HCC_ALLOC_TAG_DATA_TYPE_TABLE_POINTERS_DEDUP_HASH_TABLE, hcc_u64_key_cmp, hcc_u64_key_hash, setup->dtt.pointers_reserve_cap);
	cu->dtt.functions_dedup_hash_table = hcc_hash_table_init(HccDataTypeDedupEntry, HCC_ALLOC_TAG_DATA_TYPE_TABLE_FUNCTIONS_DEDUP_HASH_TABLE, hcc_u64_key_cmp, hcc_u64_key_hash, setup->functions_reserve_cap);
	cu->dtt.buffers_dedup_hash_table = hcc_hash_table_init(HccDataTypeDedupEntry, HCC_ALLOC_TAG_DATA_TYPE_TABLE_BUFFERS_DEDUP_HASH_TABLE, hcc_u64_key_cmp, hcc_u64_key_hash, setup->dtt.buffers_reserve_cap);

	switch (hcc_options_get_u32(cu->options, HCC_OPTION_KEY_TARGET_ARCH)) {
		case HCC_TARGET_ARCH_X86_64:
			switch (hcc_options_get_u32(cu->options, HCC_OPTION_KEY_TARGET_OS)) {
				case HCC_TARGET_OS_LINUX:
					cu->dtt.basic_type_size_and_aligns = hcc_ast_basic_type_size_and_aligns_x86_64_linux;
					cu->dtt.basic_type_int_mins = hcc_ast_basic_type_int_mins_x86_64_linux;
					cu->dtt.basic_type_int_maxes = hcc_ast_basic_type_int_maxes_x86_64_linux;
					break;
				default: HCC_ABORT("internal error: unhandled OS for the X86_64 architecture");
			}
			break;
		default: HCC_ABORT("internal error: unhandled architecture");
	}

	//
	// preallocate all the intrinsic compound data types
	hcc_stack_resize(cu->dtt.compounds, HCC_COMPOUND_DATA_TYPE_IDX_USER_START);
}

void hcc_data_type_table_deinit(HccCU* cu) {
	hcc_stack_deinit(cu->dtt.arrays);
	hcc_stack_deinit(cu->dtt.compounds);
	hcc_stack_deinit(cu->dtt.compound_fields);
	hcc_stack_deinit(cu->dtt.typedefs);
	hcc_stack_deinit(cu->dtt.enums);
	hcc_stack_deinit(cu->dtt.enum_values);
	hcc_stack_deinit(cu->dtt.buffers);
	hcc_stack_deinit(cu->dtt.pointers);
}

HccString hcc_data_type_string(HccCU* cu, HccDataType data_type) {
	HccStringId string_id;
	uint32_t qualifiers_mask = data_type & HCC_DATA_TYPE_QUALIFIERS_MASK;
	data_type = HCC_DATA_TYPE_STRIP_QUALIFIERS(data_type);
	HccString string;
	char buf[1024];
	char buf2[1024];
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		string = hcc_string_c((char*)hcc_ast_data_type_basic_type_strings[HCC_DATA_TYPE_AUX(data_type)]);
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(data_type);
		const char* fmt;
		if (HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(intrinsic_data_type) > 1 && HCC_AML_INTRINSIC_DATA_TYPE_ROWS(intrinsic_data_type) > 1) {
			fmt = "%sx%ux%u";
		} else if (HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(intrinsic_data_type) > 1) {
			fmt = "%sx%u";
		} else {
			fmt = "%s";
		}
		uint32_t string_size = snprintf(buf, sizeof(buf), fmt, hcc_aml_intrinsic_data_type_scalar_strings[HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(intrinsic_data_type)], HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(intrinsic_data_type), HCC_AML_INTRINSIC_DATA_TYPE_ROWS(intrinsic_data_type));
		string = hcc_string(buf, string_size);
	} else {
		bool is_descriptor;
		switch (HCC_DATA_TYPE_TYPE(data_type)) {
			case HCC_DATA_TYPE_TYPEDEF: {
				HccTypedef* typedef_ = hcc_typedef_get(cu, data_type);
				string = hcc_string_table_get(typedef_->identifier_string_id);
				break;
			};
			case HCC_DATA_TYPE_POINTER: {
				HccPointerDataType* d = hcc_pointer_data_type_get(cu, data_type);
				HccString element_string = hcc_data_type_string(cu, d->element_data_type);
				uint32_t string_size = snprintf(buf, sizeof(buf), "%.*s*", (int)element_string.size, element_string.data);
				string = hcc_string(buf, string_size);
				break;
			};
			case HCC_DATA_TYPE_ARRAY: {
				HccArrayDataType* d;
				uint32_t string_size = 0;
				do {
					d = hcc_array_data_type_get(cu, data_type);

					uint64_t element_count = hcc_array_data_type_element_count(cu, d);
					if (element_count == UINT64_MAX) {
						string_size += snprintf(buf + string_size, sizeof(buf) - string_size, "[]");
					} else {
						string_size += snprintf(buf + string_size, sizeof(buf) - string_size, "[%zu]", element_count);
					}

					data_type = hcc_decl_resolve_and_strip_qualifiers(cu, d->element_data_type);
				} while (HCC_DATA_TYPE_IS_ARRAY(data_type));

				HccString element_string = hcc_data_type_string(cu, d->element_data_type);

				char buf2[1024];
				string_size = snprintf(buf2, sizeof(buf2), "%.*s%.*s", (int)element_string.size, element_string.data, string_size, buf);
				string = hcc_string(buf2, string_size);
				break;
			};
			case HCC_DATA_TYPE_STRUCT:
			case HCC_DATA_TYPE_UNION:
			{
				char* compound_name = HCC_DATA_TYPE_IS_STRUCT(data_type) ? "struct" : "union";
				HccString identifier = hcc_string_lit("<anonymous>");
				if (HCC_DATA_TYPE_IS_FORWARD_DECL(data_type)) {
					HccASTForwardDecl* d = hcc_ast_forward_decl_get(cu, data_type);
					identifier = hcc_string_table_get(d->identifier_string_id);
				} else {
					HccCompoundDataType* d = hcc_compound_data_type_get(cu, data_type);
					if (d->identifier_string_id.idx_plus_one) {
						identifier = hcc_string_table_get(d->identifier_string_id);
					}
				}
				uint32_t string_size = snprintf(buf, sizeof(buf), "%s(#%u) %s%.*s", compound_name, HCC_DATA_TYPE_AUX(data_type), HCC_DATA_TYPE_IS_FORWARD_DECL(data_type) ? "[forward_decl] " : "", (int)identifier.size, identifier.data);
				string = hcc_string(buf, string_size);
				break;
			};
			case HCC_DATA_TYPE_ENUM:
			{
				HccEnumDataType* d = hcc_enum_data_type_get(cu, data_type);
				HccString identifier = hcc_string_lit("<anonymous>");
				if (d->identifier_string_id.idx_plus_one) {
					identifier = hcc_string_table_get(d->identifier_string_id);
				}
				uint32_t string_size = snprintf(buf, sizeof(buf), "enum(#%u) %.*s", HCC_DATA_TYPE_AUX(data_type), (int)identifier.size, identifier.data);
				string = hcc_string(buf, string_size);
				break;
			};
			case HCC_DATA_TYPE_FUNCTION:
			{
				HccFunctionDataType* d = hcc_function_data_type_get(cu, data_type);
				uint32_t string_size = snprintf(buf, sizeof(buf), "function_type(#%u)", HCC_DATA_TYPE_AUX(data_type));
				string = hcc_string(buf, string_size);
				break;
			};
			case HCC_DATA_TYPE_RESOURCE:
				is_descriptor = false;
				goto RESOURCE;
			case HCC_DATA_TYPE_RESOURCE_DESCRIPTOR:
				is_descriptor = true;
RESOURCE:   {
				HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(data_type);
				const char* access_mode_string = hcc_resource_access_mode_short_strings_title[HCC_RESOURCE_DATA_TYPE_ACCESS_MODE(resource_data_type)];
				switch (HCC_RESOURCE_DATA_TYPE_TYPE(resource_data_type)) {
					case HCC_RESOURCE_DATA_TYPE_BUFFER: {
						HccBufferDataType* d = hcc_buffer_data_type_get(cu, data_type);
						HccString elmt_string = hcc_data_type_string(cu, d->element_data_type);
						uint32_t string_size = snprintf(buf, sizeof(buf), is_descriptor ? "Hcc%sBufferDescriptor(%.*s)" : "Hcc%sBuffer(%.*s)", access_mode_string, (int)elmt_string.size, elmt_string.data);
						string = hcc_string(buf, string_size);
						break;
					};
					case HCC_RESOURCE_DATA_TYPE_TEXTURE: {
						HccString elmt_string = hcc_data_type_string(cu, HCC_DATA_TYPE(AML_INTRINSIC, HCC_RESOURCE_DATA_TYPE_TEXTURE_INTRINSIC_TYPE(resource_data_type)));
						uint32_t string_size = snprintf(buf, sizeof(buf), is_descriptor ? "Hcc%sTextureDescriptor(%.*s)" : "Hcc%sTexture(%.*s)", access_mode_string, (int)elmt_string.size, elmt_string.data);
						string = hcc_string(buf, string_size);
						break;
					};
					case HCC_RESOURCE_DATA_TYPE_SAMPLER: {
						string = hcc_string_lit("HccRoSampler");
						break;
					};
				}
				break;
			};
			default:
				HCC_ABORT("unhandled data type '%u'", data_type);
		}
	}

	if (qualifiers_mask) {
		const char* const_string = qualifiers_mask & HCC_DATA_TYPE_CONST_QUALIFIER_MASK ? " const" : "";
		const char* volatile_string = qualifiers_mask & HCC_DATA_TYPE_VOLATILE_QUALIFIER_MASK ? " volatile" : "";
		const char* atomic_string = qualifiers_mask & HCC_DATA_TYPE_ATOMIC_QUALIFIER_MASK ? " atomic" : "";
		uint32_t string_size = snprintf(buf2, sizeof(buf2), "%.*s%s%s%s", (int)string.size, string.data, const_string, volatile_string, atomic_string);
		string = hcc_string(buf2, string_size);
	}

	hcc_string_table_deduplicate(string.data, string.size, &string_id);
	return hcc_string_table_get(string_id);
}

void hcc_data_type_size_align(HccCU* cu, HccDataType data_type, uint64_t* size_out, uint64_t* align_out) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);

	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		*size_out = cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)];
		*align_out = cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)];
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(data_type);
		*size_out = hcc_aml_intrinsic_data_type_scalar_size_aligns[HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(intrinsic_data_type)];
		*align_out = hcc_aml_intrinsic_data_type_scalar_size_aligns[HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(intrinsic_data_type)];
		*size_out *= HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(intrinsic_data_type);
		*size_out *= HCC_AML_INTRINSIC_DATA_TYPE_ROWS(intrinsic_data_type);
	} else {
		switch (HCC_DATA_TYPE_TYPE(data_type)) {
			case HCC_DATA_TYPE_STRUCT:
			case HCC_DATA_TYPE_UNION: {
				HccCompoundDataType* d = hcc_compound_data_type_get(cu, data_type);
				*size_out = d->size;
				*align_out = d->align;
				break;
			};
			case HCC_DATA_TYPE_ARRAY: {
				HccArrayDataType* d = hcc_array_data_type_get(cu, data_type);
				HccConstant constant = hcc_constant_table_get(cu, d->element_count_constant_id);
				uint64_t count;
				hcc_constant_as_uint(cu, constant, &count);

				uint64_t size;
				uint64_t align;
				hcc_data_type_size_align(cu, d->element_data_type, &size, &align);

				*size_out = size * count;
				*align_out = align;
				break;
			};
			case HCC_DATA_TYPE_RESOURCE:
				*size_out = sizeof(uint32_t);
				*align_out = alignof(uint32_t);
				break;
			default:
				HCC_ABORT("unhandled data type '%u'", data_type);
		}
	}
}

void hcc_data_type_print_basic(HccCU* cu, HccDataType data_type, void* data, HccIIO* iio) {
	uint64_t uint;
	int64_t sint;
	double float_;
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AST_BASIC_DATA_TYPE_VOID:
				hcc_iio_write_fmt(iio, "void");
				break;
			case HCC_AST_BASIC_DATA_TYPE_BOOL:
				hcc_iio_write_fmt(iio, *(uint8_t*)data ? "true" : "false");
				break;
			case HCC_AST_BASIC_DATA_TYPE_CHAR:
			case HCC_AST_BASIC_DATA_TYPE_SCHAR:
			case HCC_AST_BASIC_DATA_TYPE_SSHORT:
			case HCC_AST_BASIC_DATA_TYPE_SINT:
			case HCC_AST_BASIC_DATA_TYPE_SLONG:
			case HCC_AST_BASIC_DATA_TYPE_SLONGLONG:
				switch (cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
					case sizeof(int8_t): uint = *(int8_t*)data; break;
					case sizeof(int16_t): uint = *(int16_t*)data; break;
					case sizeof(int32_t): uint = *(int32_t*)data; break;
					case sizeof(int64_t): uint = *(int64_t*)data; break;
				}
				hcc_iio_write_fmt(iio, "%zd", uint);
				break;
			case HCC_AST_BASIC_DATA_TYPE_UCHAR:
			case HCC_AST_BASIC_DATA_TYPE_USHORT:
			case HCC_AST_BASIC_DATA_TYPE_UINT:
			case HCC_AST_BASIC_DATA_TYPE_ULONG:
			case HCC_AST_BASIC_DATA_TYPE_ULONGLONG:
				switch (cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
					case sizeof(uint8_t): uint = *(uint8_t*)data; break;
					case sizeof(uint16_t): uint = *(uint16_t*)data; break;
					case sizeof(uint32_t): uint = *(uint32_t*)data; break;
					case sizeof(uint64_t): uint = *(uint64_t*)data; break;
				}
				hcc_iio_write_fmt(iio, "%zu", uint);
				break;
			case HCC_AST_BASIC_DATA_TYPE_FLOAT: float_ = *(float*)data; goto FLOAT;
			case HCC_AST_BASIC_DATA_TYPE_DOUBLE: float_ = *(double*)data; goto FLOAT;
FLOAT:
				hcc_iio_write_fmt(iio, "%f", float_);
				break;
			default: goto ERR;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		switch (HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(HCC_DATA_TYPE_AUX(data_type))) {
			case HCC_AML_INTRINSIC_DATA_TYPE_VOID:
				hcc_iio_write_fmt(iio, "void");
				break;
			case HCC_AML_INTRINSIC_DATA_TYPE_BOOL:
				hcc_iio_write_fmt(iio, *(uint8_t*)data ? "true" : "false");
				break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S8:
			case HCC_AML_INTRINSIC_DATA_TYPE_S16:
			case HCC_AML_INTRINSIC_DATA_TYPE_S32:
			case HCC_AML_INTRINSIC_DATA_TYPE_S64:
				switch (hcc_aml_intrinsic_data_type_scalar_size_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
					case sizeof(int8_t): uint = *(int8_t*)data; break;
					case sizeof(int16_t): uint = *(int16_t*)data; break;
					case sizeof(int32_t): uint = *(int32_t*)data; break;
					case sizeof(int64_t): uint = *(int64_t*)data; break;
				}
				hcc_iio_write_fmt(iio, "%zd", uint);
				break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U8:
			case HCC_AML_INTRINSIC_DATA_TYPE_U16:
			case HCC_AML_INTRINSIC_DATA_TYPE_U32:
			case HCC_AML_INTRINSIC_DATA_TYPE_U64:
				switch (hcc_aml_intrinsic_data_type_scalar_size_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
					case sizeof(uint8_t): uint = *(uint8_t*)data; break;
					case sizeof(uint16_t): uint = *(uint16_t*)data; break;
					case sizeof(uint32_t): uint = *(uint32_t*)data; break;
					case sizeof(uint64_t): uint = *(uint64_t*)data; break;
				}
				hcc_iio_write_fmt(iio, "%zu", uint);
				break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F16: float_ = f16tof32(*(half*)data); goto FLOAT;
			case HCC_AML_INTRINSIC_DATA_TYPE_F32: float_ = *(float*)data; goto FLOAT;
			case HCC_AML_INTRINSIC_DATA_TYPE_F64: float_ = *(double*)data; goto FLOAT;
			default: goto ERR;
		}
	} else {
ERR: {}
		HccString string = hcc_data_type_string(cu, data_type);
		HCC_ABORT("internal error: expected a basic data type but got '%.*s'", (int)string.size, string.data);
	}
}

bool hcc_data_type_is_condition(HccDataType data_type) {
	return HCC_DATA_TYPE_IS_AST_BASIC(data_type) || HCC_DATA_TYPE_IS_POINTER(data_type);
}

uint32_t hcc_data_type_composite_fields_count(HccCU* cu, HccDataType data_type) {
	HccString data_type_string = hcc_data_type_string(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_COMPOSITE(data_type), "internal error: expected a composite type but got '%.*s'", (int)data_type_string.size, data_type_string.data);

	switch (HCC_DATA_TYPE_TYPE(data_type)) {
		case HCC_DATA_TYPE_ARRAY: {
			HccArrayDataType* d = hcc_array_data_type_get(cu, data_type);
			HccConstant constant = hcc_constant_table_get(cu, d->element_count_constant_id);
			uint64_t count;
			hcc_constant_as_uint(cu, constant, &count);
			return count;
		};
		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION: {
			HccCompoundDataType* d = hcc_compound_data_type_get(cu, data_type);
			return d->fields_count;
		};
		default:
			HCC_ABORT("unhandled data type '%u'", data_type);
	}
}

bool hcc_data_type_is_rasterizer_state(HccCU* cu, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);
	return HCC_DATA_TYPE_IS_STRUCT(data_type) && (hcc_compound_data_type_get(cu, data_type)->kind == HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE);
}

bool hcc_data_type_is_fragment_state(HccCU* cu, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);
	return HCC_DATA_TYPE_IS_STRUCT(data_type) && (hcc_compound_data_type_get(cu, data_type)->kind == HCC_COMPOUND_DATA_TYPE_KIND_FRAGMENT_STATE);
}

HccDataType hcc_data_type_strip_pointer(HccCU* cu, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_POINTER(data_type)) {
		HccPointerDataType* d = hcc_pointer_data_type_get(cu, data_type);
		data_type = d->element_data_type;
	}
	return data_type;
}

HccDataType hcc_data_type_strip_all_pointers(HccCU* cu, HccDataType data_type) {
	while (HCC_DATA_TYPE_IS_POINTER(data_type)) {
		HccPointerDataType* d = hcc_pointer_data_type_get(cu, data_type);
		data_type = d->element_data_type;
	}
	return data_type;
}

HccLocation* hcc_data_type_location(HccCU* cu, HccDataType data_type) {
	switch (HCC_DATA_TYPE_TYPE(data_type)) {
		case HCC_DATA_TYPE_TYPEDEF:
			return hcc_typedef_get(cu, data_type)->identifier_location;
		default:
			return NULL;
	}
}

HccDataType hcc_data_type_lower_ast_to_aml(HccCU* cu, HccDataType data_type) {
	uint32_t qualifiers_mask = data_type & HCC_DATA_TYPE_QUALIFIERS_MASK;
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);

	switch (HCC_DATA_TYPE_TYPE(data_type)) {
		case HCC_DATA_TYPE_AST_BASIC: {
			HccASTBasicDataType basic_data_type = HCC_DATA_TYPE_AUX(data_type);
			if (HCC_AST_BASIC_DATA_TYPE_IS_UINT(cu, basic_data_type)) {
				switch (cu->dtt.basic_type_size_and_aligns[basic_data_type]) {
					case sizeof(uint8_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_U8); break;
					case sizeof(uint16_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_U16); break;
					case sizeof(uint32_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_U32); break;
					case sizeof(uint64_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_U64); break;
				}
			} else if (HCC_AST_BASIC_DATA_TYPE_IS_SINT(cu, basic_data_type)) {
				switch (cu->dtt.basic_type_size_and_aligns[basic_data_type]) {
					case sizeof(uint8_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_S8); break;
					case sizeof(uint16_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_S16); break;
					case sizeof(uint32_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_S32); break;
					case sizeof(uint64_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_S64); break;
				}
			} else if (HCC_AST_BASIC_DATA_TYPE_IS_FLOAT(basic_data_type)) {
				switch (cu->dtt.basic_type_size_and_aligns[basic_data_type]) {
					case sizeof(uint16_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_F16); break;
					case sizeof(uint32_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_F32); break;
					case sizeof(uint64_t): data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_F64); break;
				}
			} else {
				switch (basic_data_type) {
					case HCC_AST_BASIC_DATA_TYPE_VOID: data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_VOID); break;
					case HCC_AST_BASIC_DATA_TYPE_BOOL: data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_BOOL); break;
				}
			}
			break;
		};

		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION: {
			uint32_t compound_data_type_idx = HCC_DATA_TYPE_AUX(data_type);
			if (HCC_COMPOUND_DATA_TYPE_IDX_PACKED_AML_START <= compound_data_type_idx && compound_data_type_idx < HCC_COMPOUND_DATA_TYPE_IDX_PACKED_AML_END) {
				data_type = HCC_DATA_TYPE(AML_INTRINSIC, compound_data_type_idx - HCC_COMPOUND_DATA_TYPE_IDX_PACKED_AML_START);
			}
			break;
		};

		case HCC_DATA_TYPE_ARRAY: {
			HccArrayDataType* array_data_type = hcc_array_data_type_get(cu, data_type);
			HccDataType element_data_type = hcc_data_type_lower_ast_to_aml(cu, array_data_type->element_data_type);
			if (element_data_type != array_data_type->element_data_type) {
				data_type = hcc_array_data_type_deduplicate(cu, element_data_type, array_data_type->element_count_constant_id);
			}
			break;
		};

		case HCC_DATA_TYPE_POINTER: {
			HccPointerDataType* pointer_data_type = hcc_pointer_data_type_get(cu, data_type);
			HccDataType element_data_type = hcc_data_type_lower_ast_to_aml(cu, pointer_data_type->element_data_type);
			if (element_data_type != pointer_data_type->element_data_type) {
				data_type = hcc_pointer_data_type_deduplicate(cu, element_data_type);
			}
			break;
		};
	}

	return data_type | qualifiers_mask;
}

HccDataType hcc_data_type_higher_aml_to_ast(HccCU* cu, HccDataType data_type) {
	uint32_t qualifiers_mask = data_type & HCC_DATA_TYPE_QUALIFIERS_MASK;
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);

	switch (HCC_DATA_TYPE_TYPE(data_type)) {
		case HCC_DATA_TYPE_AML_INTRINSIC:
			switch (HCC_DATA_TYPE_AUX(data_type)) {
				case HCC_AML_INTRINSIC_DATA_TYPE_VOID: data_type = HCC_DATA_TYPE_AST_BASIC_VOID; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: data_type = HCC_DATA_TYPE_AST_BASIC_BOOL; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: data_type = HCC_DATA_TYPE_AST_BASIC_SCHAR; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: data_type = HCC_DATA_TYPE_AST_BASIC_UCHAR; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: data_type = HCC_DATA_TYPE_AST_BASIC_SSHORT; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: data_type = HCC_DATA_TYPE_AST_BASIC_USHORT; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32:
					if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_SINT] == 4) {
						data_type = HCC_DATA_TYPE_AST_BASIC_SINT;
					} else if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_SLONG] == 4) {
						data_type = HCC_DATA_TYPE_AST_BASIC_SLONG;
					}
					break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32:
					if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_UINT] == 4) {
						data_type = HCC_DATA_TYPE_AST_BASIC_UINT;
					} else if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_ULONG] == 4) {
						data_type = HCC_DATA_TYPE_AST_BASIC_ULONG;
					}
					break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64:
					if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_SLONG] == 8) {
						data_type = HCC_DATA_TYPE_AST_BASIC_SLONG;
					} else if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_SLONGLONG] == 8) {
						data_type = HCC_DATA_TYPE_AST_BASIC_SLONGLONG;
					}
					break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64:
					if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_ULONG] == 8) {
						data_type = HCC_DATA_TYPE_AST_BASIC_ULONG;
					} else if (cu->dtt.basic_type_size_and_aligns[HCC_AST_BASIC_DATA_TYPE_ULONGLONG] == 8) {
						data_type = HCC_DATA_TYPE_AST_BASIC_ULONGLONG;
					}
					break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F16: data_type = HCC_DATA_TYPE_AST_BASIC_HALF; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F32: data_type = HCC_DATA_TYPE_AST_BASIC_FLOAT; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F64: data_type = HCC_DATA_TYPE_AST_BASIC_DOUBLE; break;
				default: HCC_ABORT("unhandled data type: %u\n", data_type);
			}
			break;
		default: HCC_ABORT("unhandled data type: %u\n", data_type);
	}

	return data_type;
}

HccDataType hcc_data_type_signed_to_unsigned(HccCU* cu, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		HccASTBasicDataType basic_data_type = HCC_DATA_TYPE_AUX(data_type);
		switch (basic_data_type) {
			case HCC_AST_BASIC_DATA_TYPE_CHAR:
				if (hcc_options_is_char_unsigned((cu)->options)) {
					goto ERR;
				}
				hcc_fallthrough;
			case HCC_AST_BASIC_DATA_TYPE_SCHAR:
				return HCC_DATA_TYPE_AST_BASIC_UCHAR;
			case HCC_AST_BASIC_DATA_TYPE_SSHORT:
				return HCC_DATA_TYPE_AST_BASIC_USHORT;
			case HCC_AST_BASIC_DATA_TYPE_SINT:
				return HCC_DATA_TYPE_AST_BASIC_UINT;
			case HCC_AST_BASIC_DATA_TYPE_SLONG:
				return HCC_DATA_TYPE_AST_BASIC_ULONG;
			case HCC_AST_BASIC_DATA_TYPE_SLONGLONG:
				return HCC_DATA_TYPE_AST_BASIC_ULONGLONG;

			default: goto ERR;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(data_type);
		HccAMLIntrinsicDataType scalar_data_type = intrinsic_data_type & HCC_AML_INTRINSIC_DATA_TYPE_SCALAR_MASK;

		switch (scalar_data_type) {
			case HCC_AML_INTRINSIC_DATA_TYPE_S8: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_U8; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S16: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_U16; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S32: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_U32; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S64: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_U64; break;
			default: goto ERR;
		}

		intrinsic_data_type &= ~HCC_AML_INTRINSIC_DATA_TYPE_SCALAR_MASK;
		intrinsic_data_type |= scalar_data_type;
		return HCC_DATA_TYPE(AML_INTRINSIC, intrinsic_data_type);
	}

ERR:{}
	HccString data_type_name = hcc_data_type_string(cu, data_type);
	HCC_UNREACHABLE("internal error: expected a basic/intrinsic type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
}

HccDataType hcc_data_type_unsigned_to_signed(HccCU* cu, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		HccASTBasicDataType basic_data_type = HCC_DATA_TYPE_AUX(data_type);
		switch (basic_data_type) {
			case HCC_AST_BASIC_DATA_TYPE_CHAR:
				if (!hcc_options_is_char_unsigned((cu)->options)) {
					goto ERR;
				}
				hcc_fallthrough;
			case HCC_AST_BASIC_DATA_TYPE_UCHAR:
				return HCC_DATA_TYPE_AST_BASIC_SCHAR;
			case HCC_AST_BASIC_DATA_TYPE_USHORT:
				return HCC_DATA_TYPE_AST_BASIC_SSHORT;
			case HCC_AST_BASIC_DATA_TYPE_UINT:
				return HCC_DATA_TYPE_AST_BASIC_SINT;
			case HCC_AST_BASIC_DATA_TYPE_ULONG:
				return HCC_DATA_TYPE_AST_BASIC_SLONG;
			case HCC_AST_BASIC_DATA_TYPE_ULONGLONG:
				return HCC_DATA_TYPE_AST_BASIC_SLONGLONG;

			default: goto ERR;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(data_type);
		HccAMLIntrinsicDataType scalar_data_type = intrinsic_data_type & HCC_AML_INTRINSIC_DATA_TYPE_SCALAR_MASK;

		switch (scalar_data_type) {
			case HCC_AML_INTRINSIC_DATA_TYPE_U8: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_S8; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U16: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_S16; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U32: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_S32; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U64: scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_S64; break;
			default: goto ERR;
		}

		intrinsic_data_type &= ~HCC_AML_INTRINSIC_DATA_TYPE_SCALAR_MASK;
		intrinsic_data_type |= scalar_data_type;
		return HCC_DATA_TYPE(AML_INTRINSIC, intrinsic_data_type);
	}

ERR:{}
	HccString data_type_name = hcc_data_type_string(cu, data_type);
	HCC_UNREACHABLE("internal error: expected a basic/intrinsic type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
}

HccCanCast hcc_data_type_can_cast(HccCU* cu, HccDataType dst_data_type, HccDataType src_data_type) {
	HccDataType resolved_dst_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, dst_data_type);
	HccDataType resolved_src_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, src_data_type);
	if (resolved_dst_data_type == resolved_src_data_type) {
		return HCC_CAN_CAST_NO_SAME_TYPES;
	}

	if (HCC_DATA_TYPE_IS_AST_BASIC(resolved_dst_data_type) && HCC_DATA_TYPE_IS_AST_BASIC(resolved_src_data_type)) {
		return HCC_CAN_CAST_YES;
	}

	if (HCC_DATA_TYPE_IS_POINTER(resolved_dst_data_type)) {
		if (HCC_DATA_TYPE_IS_POINTER(resolved_src_data_type)) {
			return HCC_CAN_CAST_YES;
		}

		if (HCC_DATA_TYPE_IS_ARRAY(resolved_src_data_type)) {
			return HCC_CAN_CAST_YES;
		}
	}

	return HCC_CAN_CAST_NO_DIFFERENT_TYPES;
}

HccAMLScalarDataTypeMask hcc_data_type_scalar_data_types_mask(HccCU* cu, HccDataType data_type) {
	switch (HCC_DATA_TYPE_TYPE(data_type)) {
		case HCC_DATA_TYPE_ENUM:
			data_type = HCC_DATA_TYPE_AST_BASIC_SINT;
			hcc_fallthrough;
		case HCC_DATA_TYPE_AST_BASIC:
			data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
			hcc_fallthrough;
		case HCC_DATA_TYPE_AML_INTRINSIC: {
			HccAMLScalarDataTypeMask mask = 0;
			HCC_AML_SCALAR_DATA_TYPE_MASK_SET(&mask, HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(HCC_DATA_TYPE_AUX(data_type)));
			return mask;
		};
		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION: {
			if (HCC_DATA_TYPE_IS_FORWARD_DECL(data_type)) {
				return 0;
			} else {
				HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(cu, data_type);
				return compound_data_type->scalar_data_types_mask;
			}
		};

		case HCC_DATA_TYPE_ARRAY: {
			HccArrayDataType* array_data_type = hcc_array_data_type_get(cu, data_type);
			return hcc_data_type_scalar_data_types_mask(cu, array_data_type->element_data_type);
		};

		case HCC_DATA_TYPE_TYPEDEF: {
			HccTypedef* typedef_ = hcc_typedef_get(cu, data_type);
			return hcc_data_type_scalar_data_types_mask(cu, typedef_->aliased_data_type);
		};

		case HCC_DATA_TYPE_POINTER: {
			HccPointerDataType* d = hcc_pointer_data_type_get(cu, data_type);
			return hcc_data_type_scalar_data_types_mask(cu, d->element_data_type);
		};

		default:
			return 0;
	}
}


HccBasicTypeClass hcc_basic_type_class(HccCU* cu, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AST_BASIC_DATA_TYPE_BOOL: return HCC_BASIC_TYPE_CLASS_BOOL;
			case HCC_AST_BASIC_DATA_TYPE_CHAR:
			case HCC_AST_BASIC_DATA_TYPE_SCHAR:
			case HCC_AST_BASIC_DATA_TYPE_SSHORT:
			case HCC_AST_BASIC_DATA_TYPE_SINT:
			case HCC_AST_BASIC_DATA_TYPE_SLONG:
			case HCC_AST_BASIC_DATA_TYPE_SLONGLONG: return HCC_BASIC_TYPE_CLASS_SINT;
			case HCC_AST_BASIC_DATA_TYPE_UCHAR:
			case HCC_AST_BASIC_DATA_TYPE_USHORT:
			case HCC_AST_BASIC_DATA_TYPE_UINT:
			case HCC_AST_BASIC_DATA_TYPE_ULONG:
			case HCC_AST_BASIC_DATA_TYPE_ULONGLONG: return HCC_BASIC_TYPE_CLASS_UINT;
			case HCC_AST_BASIC_DATA_TYPE_FLOAT:
			case HCC_AST_BASIC_DATA_TYPE_DOUBLE: return HCC_BASIC_TYPE_CLASS_FLOAT;
			default: goto ERR;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		switch (HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(HCC_DATA_TYPE_AUX(data_type))) {
			case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: return HCC_BASIC_TYPE_CLASS_BOOL;
			case HCC_AML_INTRINSIC_DATA_TYPE_S8:
			case HCC_AML_INTRINSIC_DATA_TYPE_S16:
			case HCC_AML_INTRINSIC_DATA_TYPE_S32:
			case HCC_AML_INTRINSIC_DATA_TYPE_S64: return HCC_BASIC_TYPE_CLASS_SINT;
			case HCC_AML_INTRINSIC_DATA_TYPE_U8:
			case HCC_AML_INTRINSIC_DATA_TYPE_U16:
			case HCC_AML_INTRINSIC_DATA_TYPE_U32:
			case HCC_AML_INTRINSIC_DATA_TYPE_U64: return HCC_BASIC_TYPE_CLASS_UINT;
			case HCC_AML_INTRINSIC_DATA_TYPE_F16:
			case HCC_AML_INTRINSIC_DATA_TYPE_F32:
			case HCC_AML_INTRINSIC_DATA_TYPE_F64: return HCC_BASIC_TYPE_CLASS_FLOAT;
			default: goto ERR;
		}
	} else {
ERR:{}
		HccString data_type_name = hcc_data_type_string(cu, data_type);
		HCC_UNREACHABLE("internal error: expected a basic type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
	}
}

HccBasic hcc_basic_eval(HccCU* cu, HccASTBinaryOp binary_op, HccDataType data_type, HccBasic left_eval, HccBasic right_eval) {
	HccBasic eval;
	data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
	HCC_ASSERT(HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type), "data type must be convertable to an AML intrinsic type");
	HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(data_type);
	HCC_ASSERT(HCC_AML_INTRINSIC_DATA_TYPE_IS_SCALAR(intrinsic_data_type), "basic eval only work on basic types, not vectors");

	switch (binary_op) {
		case HCC_AST_BINARY_OP_ADD:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ + right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 + right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 + right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 + right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 + right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 + right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 + right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 + right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 + right_eval.u64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F32: eval.f32 = left_eval.f32 + right_eval.f32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F64: eval.f64 = left_eval.f64 + right_eval.f64; break;
			}
			break;
		case HCC_AST_BINARY_OP_SUBTRACT:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ - right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 - right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 - right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 - right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 - right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 - right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 - right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 - right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 - right_eval.u64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F32: eval.f32 = left_eval.f32 - right_eval.f32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F64: eval.f64 = left_eval.f64 - right_eval.f64; break;
			}
			break;
		case HCC_AST_BINARY_OP_MULTIPLY:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ * right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 * right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 * right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 * right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 * right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 * right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 * right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 * right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 * right_eval.u64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F32: eval.f32 = left_eval.f32 * right_eval.f32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F64: eval.f64 = left_eval.f64 * right_eval.f64; break;
			}
			break;
		case HCC_AST_BINARY_OP_DIVIDE:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ / right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 / right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 / right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 / right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 / right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 / right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 / right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 / right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 / right_eval.u64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F32: eval.f32 = left_eval.f32 / right_eval.f32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_F64: eval.f64 = left_eval.f64 / right_eval.f64; break;
			}
			break;
		case HCC_AST_BINARY_OP_MODULO:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ % right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 % right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 % right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 % right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 % right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 % right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 % right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 % right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 % right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_BIT_AND:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ & right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 & right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 & right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 & right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 & right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 & right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 & right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 & right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 & right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_BIT_OR:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ | right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 | right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 | right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 | right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 | right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 | right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 | right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 | right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 | right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_BIT_XOR:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ ^ right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 ^ right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 ^ right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 ^ right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 ^ right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 ^ right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 ^ right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 ^ right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 ^ right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_BIT_SHIFT_LEFT:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ << right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 << right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 << right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 << right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 << right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 << right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 << right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 << right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 << right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_BIT_SHIFT_RIGHT:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ >> right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 >> right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 >> right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 >> right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 >> right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 >> right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 >> right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 >> right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 >> right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_EQUAL:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ == right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 == right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 == right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 == right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 == right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 == right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 == right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 == right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 == right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_NOT_EQUAL:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ != right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 != right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 != right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 != right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 != right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 != right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 != right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 != right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 != right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_LESS_THAN:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ < right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 < right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 < right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 < right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 < right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 < right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 < right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 < right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 < right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_LESS_THAN_OR_EQUAL:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ <= right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 <= right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 <= right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 <= right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 <= right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 <= right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 <= right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 <= right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 <= right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_GREATER_THAN:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ > right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 > right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 > right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 > right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 > right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 > right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 > right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 > right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 > right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_GREATER_THAN_OR_EQUAL:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ >= right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 >= right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 >= right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 >= right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 >= right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 >= right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 >= right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 >= right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 >= right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_LOGICAL_AND:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ && right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 && right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 && right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 && right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 && right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 && right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 && right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 && right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 && right_eval.u64; break;
			}
			break;
		case HCC_AST_BINARY_OP_LOGICAL_OR:
			switch (intrinsic_data_type) {
				case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: eval.bool_ = left_eval.bool_ || right_eval.bool_; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S8: eval.s8 = left_eval.s8 || right_eval.s8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S16: eval.s16 = left_eval.s16 || right_eval.s16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S32: eval.s32 = left_eval.s32 || right_eval.s32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_S64: eval.s64 = left_eval.s64 || right_eval.s64; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U8: eval.u8 = left_eval.u8 || right_eval.u8; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U16: eval.u16 = left_eval.u16 || right_eval.u16; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U32: eval.u32 = left_eval.u32 || right_eval.u32; break;
				case HCC_AML_INTRINSIC_DATA_TYPE_U64: eval.u64 = left_eval.u64 || right_eval.u64; break;
			}
			break;
	}

	return eval;
}

bool hcc_basic_as_bool(HccCU* cu, HccDataType data_type, HccBasic basic) {
	data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
	HCC_ASSERT(HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type), "data type must be convertable to an AML intrinsic type");
	HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(data_type);
	HCC_ASSERT(HCC_AML_INTRINSIC_DATA_TYPE_IS_SCALAR(intrinsic_data_type), "basic eval only work on basic types, not vectors");

	switch (intrinsic_data_type) {
		case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: return basic.bool_;
		case HCC_AML_INTRINSIC_DATA_TYPE_S8: return basic.s8 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_S16: return basic.s16 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_S32: return basic.s32 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_S64: return basic.s64 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_U8: return basic.u8 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_U16: return basic.u16 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_U32: return basic.u32 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_U64: return basic.u64 != 0;
		case HCC_AML_INTRINSIC_DATA_TYPE_F32: return basic.f32 != 0.f;
		case HCC_AML_INTRINSIC_DATA_TYPE_F64: return basic.f64 != 0.0;
	}
	return false;
}

HccBasic hcc_basic_from_sint(HccCU* cu, HccDataType data_type, int64_t value) {
	HccBasic basic;
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AST_BASIC_DATA_TYPE_CHAR:
			case HCC_AST_BASIC_DATA_TYPE_SCHAR:
			case HCC_AST_BASIC_DATA_TYPE_SSHORT:
			case HCC_AST_BASIC_DATA_TYPE_SINT:
			case HCC_AST_BASIC_DATA_TYPE_SLONG:
			case HCC_AST_BASIC_DATA_TYPE_SLONGLONG:
				switch (cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
				case sizeof(int8_t): basic.s8 = value; break;
				case sizeof(int16_t): basic.s16 = value; break;
				case sizeof(int32_t): basic.s32 = value; break;
				case sizeof(int64_t): basic.s64 = value; break;
				}
				break;
			case HCC_AST_BASIC_DATA_TYPE_BOOL: basic.bool_ = value; break;
			case HCC_AST_BASIC_DATA_TYPE_UCHAR:
			case HCC_AST_BASIC_DATA_TYPE_USHORT:
			case HCC_AST_BASIC_DATA_TYPE_UINT:
			case HCC_AST_BASIC_DATA_TYPE_ULONG:
			case HCC_AST_BASIC_DATA_TYPE_ULONGLONG:
				switch (cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
				case sizeof(uint8_t): basic.u8 = value; break;
				case sizeof(uint16_t): basic.u16 = value; break;
				case sizeof(uint32_t): basic.u32 = value; break;
				case sizeof(uint64_t): basic.u64 = value; break;
				}
				break;
			case HCC_AST_BASIC_DATA_TYPE_FLOAT: basic.f32 = value; break;
			case HCC_AST_BASIC_DATA_TYPE_DOUBLE: basic.f64 = value; break;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: basic.bool_ = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S8: basic.s8 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S16: basic.s16 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S32: basic.s32 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S64: basic.s64 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U8: basic.u8 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U16: basic.u16 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U32: basic.u32 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U64: basic.u64 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F32: basic.f32 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F64: basic.f64 = value; break;
			default: goto ERR;
		}
	} else {
ERR: {}
		HccString data_type_name = hcc_data_type_string(cu, data_type);
		HCC_UNREACHABLE("internal error: expected a basic type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
	}

	return basic;
}

HccBasic hcc_basic_from_uint(HccCU* cu, HccDataType data_type, uint64_t value) {
	HccBasic basic;
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AST_BASIC_DATA_TYPE_BOOL: basic.bool_ = value; break;
			case HCC_AST_BASIC_DATA_TYPE_CHAR:
			case HCC_AST_BASIC_DATA_TYPE_SCHAR:
			case HCC_AST_BASIC_DATA_TYPE_SSHORT:
			case HCC_AST_BASIC_DATA_TYPE_SINT:
			case HCC_AST_BASIC_DATA_TYPE_SLONG:
			case HCC_AST_BASIC_DATA_TYPE_SLONGLONG:
			case HCC_AST_BASIC_DATA_TYPE_UCHAR:
			case HCC_AST_BASIC_DATA_TYPE_USHORT:
			case HCC_AST_BASIC_DATA_TYPE_UINT:
			case HCC_AST_BASIC_DATA_TYPE_ULONG:
			case HCC_AST_BASIC_DATA_TYPE_ULONGLONG:
				switch (cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
				case sizeof(uint8_t): basic.u8 = value; break;
				case sizeof(uint16_t): basic.u16 = value; break;
				case sizeof(uint32_t): basic.u32 = value; break;
				case sizeof(uint64_t): basic.u64 = value; break;
				}
				break;
			case HCC_AST_BASIC_DATA_TYPE_FLOAT: basic.f32 = value; break;
			case HCC_AST_BASIC_DATA_TYPE_DOUBLE: basic.f64 = value; break;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: basic.bool_ = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S8: basic.u8 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S16: basic.u16 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S32: basic.u32 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S64: basic.u64 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U8: basic.u8 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U16: basic.u16 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U32: basic.u32 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U64: basic.u64 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F32: basic.f32 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F64: basic.f64 = value; break;
			default: goto ERR;
		}
	} else {
ERR: {}
		HccString data_type_name = hcc_data_type_string(cu, data_type);
		HCC_UNREACHABLE("internal error: expected a basic type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
	}

	return basic;
}

HccBasic hcc_basic_from_float(HccCU* cu, HccDataType data_type, double value) {
	HccBasic basic;
	if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AST_BASIC_DATA_TYPE_CHAR:
			case HCC_AST_BASIC_DATA_TYPE_SCHAR:
			case HCC_AST_BASIC_DATA_TYPE_SSHORT:
			case HCC_AST_BASIC_DATA_TYPE_SINT:
			case HCC_AST_BASIC_DATA_TYPE_SLONG:
			case HCC_AST_BASIC_DATA_TYPE_SLONGLONG:
				switch (cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
				case sizeof(int8_t): basic.s8 = (int8_t)value; break;
				case sizeof(int16_t): basic.s16 = (int16_t)value; break;
				case sizeof(int32_t): basic.s32 = (int32_t)value; break;
				case sizeof(int64_t): basic.s64 = (int64_t)value; break;
				}
				break;
			case HCC_AST_BASIC_DATA_TYPE_BOOL: basic.bool_ = (bool)value; break;
			case HCC_AST_BASIC_DATA_TYPE_UCHAR:
			case HCC_AST_BASIC_DATA_TYPE_USHORT:
			case HCC_AST_BASIC_DATA_TYPE_UINT:
			case HCC_AST_BASIC_DATA_TYPE_ULONG:
			case HCC_AST_BASIC_DATA_TYPE_ULONGLONG:
				switch (cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(data_type)]) {
				case sizeof(uint8_t): basic.u8 = (uint8_t)value; break;
				case sizeof(uint16_t): basic.u16 = (uint16_t)value; break;
				case sizeof(uint32_t): basic.u32 = (uint32_t)value; break;
				case sizeof(uint64_t): basic.u64 = (uint64_t)value; break;
				}
				break;
			case HCC_AST_BASIC_DATA_TYPE_FLOAT: basic.f32 = value; break;
			case HCC_AST_BASIC_DATA_TYPE_DOUBLE: basic.f64 = value; break;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
		switch (HCC_DATA_TYPE_AUX(data_type)) {
			case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: basic.bool_ = (bool)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S8: basic.s8 = (int8_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S16: basic.s16 = (int16_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S32: basic.s32 = (int32_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_S64: basic.s64 = (int64_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U8: basic.u8 = (uint8_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U16: basic.u16 = (uint16_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U32: basic.u32 = (uint32_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_U64: basic.u64 = (uint64_t)value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F32: basic.f32 = value; break;
			case HCC_AML_INTRINSIC_DATA_TYPE_F64: basic.f64 = value; break;
			default: goto ERR;
		}
	} else {
ERR: {}
		HccString data_type_name = hcc_data_type_string(cu, data_type);
		HCC_UNREACHABLE("internal error: expected a basic type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
	}

	return basic;
}

HccArrayDataType* hcc_array_data_type_get(HccCU* cu, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_ARRAY(data_type), "internal error: expected array data type");
	return hcc_stack_get(cu->dtt.arrays, HCC_DATA_TYPE_AUX(data_type));
}

HccDataType hcc_array_data_type_element_data_type(HccArrayDataType* dt) {
	return dt->element_data_type;
}

HccConstantId hcc_array_data_type_element_count_constant_id(HccArrayDataType* dt) {
	return dt->element_count_constant_id;
}

uint64_t hcc_array_data_type_element_count(HccCU* cu, HccArrayDataType* dt) {
	if (dt->element_count_constant_id.idx_plus_one == 0) {
		return UINT64_MAX;
	}
	HccConstant constant = hcc_constant_table_get(cu, dt->element_count_constant_id);
	uint64_t element_count;
	HCC_DEBUG_ASSERT(hcc_constant_as_uint(cu, constant, &element_count), "internal error: array element count is not an unsigned integer");
	return element_count;
}

HccDataType hcc_array_data_type_deduplicate(HccCU* cu, HccDataType element_data_type, HccConstantId element_count_constant_id) {
	element_data_type = hcc_decl_resolve_and_keep_qualifiers(cu, element_data_type);
	uint64_t key = (uint64_t)element_data_type | ((uint64_t)element_count_constant_id.idx_plus_one << 32);
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->dtt.arrays_dedup_hash_table, &key);
	HccDataTypeDedupEntry* entry = &cu->dtt.arrays_dedup_hash_table[insert.idx];
	if (!insert.is_new) {
		uint32_t id;
		while ((id = atomic_load(&entry->id)) == 0) {
			HCC_CPU_RELAX();
		}

		return HCC_DATA_TYPE(ARRAY, id - 1);
	}

	HccArrayDataType* d = hcc_stack_push_thread_safe(cu->dtt.arrays);
	d->element_data_type = element_data_type;
	d->element_count_constant_id = element_count_constant_id;

	uint32_t array_data_types_idx = d - cu->dtt.arrays;
	atomic_store(&entry->id, array_data_types_idx + 1);
	HccDataType data_type = HCC_DATA_TYPE(ARRAY, array_data_types_idx);
	return data_type;
}

HccBufferDataType* hcc_buffer_data_type_get(HccCU* cu, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_BUFFER(data_type), "internal error: expected buffer data type");
	return hcc_stack_get(cu->dtt.buffers, HCC_RESOURCE_DATA_TYPE_AUX(HCC_DATA_TYPE_AUX(data_type)));
}

HccDataType hcc_buffer_data_type_element_data_type(HccBufferDataType* dt) {
	return dt->element_data_type;
}

HccPointerDataType* hcc_pointer_data_type_get(HccCU* cu, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_POINTER(data_type), "internal error: expected pointer data type");
	return hcc_stack_get(cu->dtt.pointers, HCC_DATA_TYPE_AUX(data_type));
}

HccDataType hcc_pointer_data_type_element_data_type(HccPointerDataType* dt) {
	return dt->element_data_type;
}

HccDataType hcc_pointer_data_type_deduplicate(HccCU* cu, HccDataType element_data_type) {
	element_data_type = hcc_decl_resolve_and_keep_qualifiers(cu, element_data_type);
	uint64_t key = element_data_type;
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->dtt.pointers_dedup_hash_table, &key);
	HccDataTypeDedupEntry* entry = &cu->dtt.pointers_dedup_hash_table[insert.idx];
	if (!insert.is_new) {
		uint32_t id;
		while ((id = atomic_load(&entry->id)) == 0) {
			HCC_CPU_RELAX();
		}

		return HCC_DATA_TYPE(POINTER, id - 1);
	}

	HccPointerDataType* d = hcc_stack_push_thread_safe(cu->dtt.pointers);
	d->element_data_type = element_data_type;

	uint32_t pointer_idx = d - cu->dtt.pointers;
	atomic_store(&entry->id, pointer_idx + 1);
	HccDataType data_type = HCC_DATA_TYPE(POINTER, pointer_idx);
	return data_type;
}

HccFunctionDataType* hcc_function_data_type_get(HccCU* cu, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_FUNCTION(data_type), "internal error: expected pointer data type");
	return hcc_stack_get(cu->dtt.functions, HCC_DATA_TYPE_AUX(data_type));
}

HccDataType hcc_function_data_type_return_data_type(HccFunctionDataType* dt) {
	return dt->return_data_type;
}

uint32_t hcc_function_data_type_params_count(HccFunctionDataType* dt) {
	return dt->params_count;
}

HccDataType* hcc_function_data_type_params(HccFunctionDataType* dt) {
	return dt->params;
}

HccDataType hcc_function_data_type_deduplicate(HccCU* cu, HccDataType return_data_type, HccDataType* params, uint32_t params_count, uintptr_t params_stride) {
	if (params_stride == 0) {
		params_stride = sizeof(HccDataType);
	}

	return_data_type = hcc_decl_resolve_and_keep_qualifiers(cu, return_data_type);
	uint64_t key = HCC_HASH_FNV_64_INIT;
	key = hcc_hash_fnv_64(&return_data_type, sizeof(HccDataType), key);
	for (uint32_t param_idx = 0; param_idx < params_count; param_idx += 1) {
		key = hcc_hash_fnv_64(HCC_PTR_ADD(params, param_idx * params_stride), sizeof(HccDataType), key);
	}

	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->dtt.functions_dedup_hash_table, &key);
	HccDataTypeDedupEntry* entry = &cu->dtt.functions_dedup_hash_table[insert.idx];
	if (!insert.is_new) {
		uint32_t id;
		while ((id = atomic_load(&entry->id)) == 0) {
			HCC_CPU_RELAX();
		}

		return HCC_DATA_TYPE(FUNCTION, id - 1);
	}

	HccDataType* dst_params = hcc_stack_push_many_thread_safe(cu->dtt.function_params, params_count);
	for (uint32_t param_idx = 0; param_idx < params_count; param_idx += 1) {
		dst_params[param_idx] = *(HccDataType*)HCC_PTR_ADD(params, param_idx * params_stride);
	}

	HccFunctionDataType* d = hcc_stack_push_thread_safe(cu->dtt.functions);
	d->return_data_type = return_data_type;
	d->params = dst_params;
	d->params_count = params_count;

	uint32_t function_idx = d - cu->dtt.functions;
	atomic_store(&entry->id, function_idx + 1);
	HccDataType data_type = HCC_DATA_TYPE(FUNCTION, function_idx);
	return data_type;
}

HccCompoundDataType* hcc_compound_data_type_get(HccCU* cu, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_COMPOUND(data_type), "internal error: expected compound data type");
	HCC_DEBUG_ASSERT(!HCC_DATA_TYPE_IS_FORWARD_DECL(data_type), "internal error: expected compound data type that is not a forward declaration");
	return hcc_stack_get(cu->dtt.compounds, HCC_DATA_TYPE_AUX(data_type));
}

bool hcc_compound_data_type_is_struct(HccCompoundDataType* dt) {
	return !(dt->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION);
}

bool hcc_compound_data_type_is_union(HccCompoundDataType* dt) {
	return dt->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION;
}

HccLocation* hcc_compound_data_type_identifier_location(HccCompoundDataType* dt) {
	return dt->identifier_location;
}

HccStringId hcc_compound_data_type_identifier_string_id(HccCompoundDataType* dt) {
	return dt->identifier_string_id;
}

HccCompoundField* hcc_compound_data_type_fields(HccCompoundDataType* dt, uint32_t* fields_count_out) {
	*fields_count_out = dt->fields_count;
	return dt->fields;
}

void hcc_compound_data_type_size_align(HccCompoundDataType* dt, uint64_t* size_out, uint64_t* align_out) {
	*size_out = dt->size;
	*align_out = dt->align;
}

HccLocation* hcc_compound_field_identifier_location(HccCompoundField* field) {
	return field->identifier_location;
}

HccStringId hcc_compound_field_identifier_string_id(HccCompoundField* field) {
	return field->identifier_string_id;
}

HccDataType hcc_compound_field_data_type(HccCompoundField* field) {
	return field->data_type;
}

HccEnumDataType* hcc_enum_data_type_get(HccCU* cu, HccDataType data_type) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_ENUM(data_type), "internal error: expected enum data type");
	return hcc_stack_get(cu->dtt.enums, HCC_DATA_TYPE_AUX(data_type));
}

HccLocation* hcc_enum_data_type_identifier_location(HccEnumDataType* dt) {
	return dt->identifier_location;
}

HccStringId hcc_enum_data_type_identifier_string_id(HccEnumDataType* dt) {
	return dt->identifier_string_id;
}

HccEnumValue* hcc_enum_data_type_values(HccEnumDataType* dt, uint32_t* values_count_out) {
	*values_count_out = dt->values_count;
	return dt->values;
}

HccEnumValue* hcc_enum_value_get(HccCU* cu, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_ENUM_VALUE(decl), "internal error: expected a enum value");
	return hcc_stack_get(cu->dtt.enum_values, HCC_DECL_AUX(decl));
}

HccLocation* hcc_enum_value_identifier_location(HccEnumValue* value) {
	return value->identifier_location;
}

HccStringId hcc_enum_value_identifier_string_id(HccEnumValue* value) {
	return value->identifier_string_id;
}

HccConstantId hcc_enum_value_constant_id(HccEnumValue* value) {
	return value->constant_id;
}

int32_t hcc_enum_value(HccCU* cu, HccEnumValue* value) {
	HccConstant constant = hcc_constant_table_get(cu, value->constant_id);
	int32_t v;
	HCC_DEBUG_ASSERT(hcc_constant_as_sint32(cu, constant, &v), "internal error: enum value is not an int");
	return v;
}

HccTypedef* hcc_typedef_get(HccCU* cu, HccDataType data_type) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_TYPEDEF(data_type), "internal error: expected typedef");
	return hcc_stack_get(cu->dtt.typedefs, HCC_DATA_TYPE_AUX(data_type));
}

HccLocation* hcc_typedef_identifier_location(HccTypedef* dt) {
	return dt->identifier_location;
}

HccStringId hcc_typedef_identifier_string_id(HccTypedef* dt) {
	return dt->identifier_string_id;
}

HccDataType hcc_typedef_aliased_data_type(HccTypedef* dt) {
	return dt->aliased_data_type;
}

// ===========================================
//
//
// Constant
//
//
// ===========================================

uint8_t hcc_constant_read_8(HccConstant constant) {
	return *(uint8_t*)constant.data;
}

uint16_t hcc_constant_read_16(HccConstant constant) {
#if HCC_BYTE_ORDER == HCC_LITTLE_ENDIAN
	return
		((uint16_t)((uint8_t*)constant.data)[0] << 0) |
		((uint16_t)((uint8_t*)constant.data)[1] << 8) ;
#else
	return
		((uint16_t)((uint8_t*)constant.data)[0] << 8) |
		((uint16_t)((uint8_t*)constant.data)[1] << 0) ;
#endif
}

uint32_t hcc_constant_read_32(HccConstant constant) {
#if HCC_BYTE_ORDER == HCC_LITTLE_ENDIAN
	return
		((uint32_t)((uint8_t*)constant.data)[0] <<  0) |
		((uint32_t)((uint8_t*)constant.data)[1] <<  8) |
		((uint32_t)((uint8_t*)constant.data)[2] << 16) |
		((uint32_t)((uint8_t*)constant.data)[3] << 24) ;
#else
	return
		((uint32_t)((uint8_t*)constant.data)[0] << 24) |
		((uint32_t)((uint8_t*)constant.data)[1] << 16) |
		((uint32_t)((uint8_t*)constant.data)[2] <<  8) |
		((uint32_t)((uint8_t*)constant.data)[3] <<  0) ;
#endif
}

uint64_t hcc_constant_read_64(HccConstant constant) {
#if HCC_BYTE_ORDER == HCC_LITTLE_ENDIAN
	return
		((uint64_t)((uint8_t*)constant.data)[0] <<  0) |
		((uint64_t)((uint8_t*)constant.data)[1] <<  8) |
		((uint64_t)((uint8_t*)constant.data)[2] << 16) |
		((uint64_t)((uint8_t*)constant.data)[3] << 24) |
		((uint64_t)((uint8_t*)constant.data)[4] << 32) |
		((uint64_t)((uint8_t*)constant.data)[5] << 40) |
		((uint64_t)((uint8_t*)constant.data)[6] << 48) |
		((uint64_t)((uint8_t*)constant.data)[7] << 56) ;
#else
	return
		((uint64_t)((uint8_t*)constant.data)[0] << 56) |
		((uint64_t)((uint8_t*)constant.data)[1] << 48) |
		((uint64_t)((uint8_t*)constant.data)[2] << 40) |
		((uint64_t)((uint8_t*)constant.data)[3] << 32) |
		((uint64_t)((uint8_t*)constant.data)[4] << 24) |
		((uint64_t)((uint8_t*)constant.data)[5] << 16) |
		((uint64_t)((uint8_t*)constant.data)[6] <<  8) |
		((uint64_t)((uint8_t*)constant.data)[7] <<  0) ;
#endif
}

bool hcc_constant_read_int_extend_64(HccCU* cu, HccConstant constant, uint64_t* out) {
	uint32_t size_align = 0;
	bool is_signed = false;
	if (HCC_DATA_TYPE_IS_AST_BASIC(constant.data_type)) {
		HccASTBasicDataType basic_dt = HCC_DATA_TYPE_AUX(constant.data_type);
		if (HCC_AST_BASIC_DATA_TYPE_IS_UINT(cu, basic_dt)) {
			size_align = cu->dtt.basic_type_size_and_aligns[basic_dt];
			is_signed = false;
		} else if (HCC_AST_BASIC_DATA_TYPE_IS_SINT(cu, basic_dt)) {
			size_align = cu->dtt.basic_type_size_and_aligns[basic_dt];
			is_signed = true;
		}
	} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(constant.data_type)) {
		HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(constant.data_type);
		if (HCC_AML_INTRINSIC_DATA_TYPE_IS_UINT(intrinsic_data_type)) {
			size_align = hcc_aml_intrinsic_data_type_scalar_size_aligns[intrinsic_data_type];
			is_signed = false;
		} else if (HCC_AML_INTRINSIC_DATA_TYPE_IS_SINT(intrinsic_data_type)) {
			size_align = hcc_aml_intrinsic_data_type_scalar_size_aligns[intrinsic_data_type];
			is_signed = true;
		}
	}

	if (is_signed) {
		switch (size_align) {
			case sizeof(uint8_t): *out = (int64_t)(int8_t)hcc_constant_read_8(constant); break;
			case sizeof(uint16_t): *out = (int64_t)(int16_t)hcc_constant_read_16(constant); break;
			case sizeof(uint32_t): *out = (int64_t)(int32_t)hcc_constant_read_32(constant); break;
			case sizeof(uint64_t): *out = (int64_t)(int64_t)hcc_constant_read_64(constant); break;
			default: return false;
		}
	} else {
		switch (size_align) {
			case sizeof(uint8_t): *out = hcc_constant_read_8(constant); break;
			case sizeof(uint16_t): *out = hcc_constant_read_16(constant); break;
			case sizeof(uint32_t): *out = hcc_constant_read_32(constant); break;
			case sizeof(uint64_t): *out = hcc_constant_read_64(constant); break;
			default: return false;
		}
	}

	return true;
}

void hcc_constant_print(HccCU* cu, HccConstantId constant_id, HccIIO* iio) {
	HccConstant constant = hcc_constant_table_get(cu, constant_id);
	if (constant.size == 0) {
		HccString data_type_name = hcc_data_type_string(cu, constant.data_type);
		hcc_iio_write_fmt(iio, "%.*s: <ZERO>", (int)data_type_name.size, data_type_name.data);
		return;
	}

	if (HCC_DATA_TYPE_IS_AST_BASIC(constant.data_type) || HCC_DATA_TYPE_IS_AML_INTRINSIC(constant.data_type)) {
		hcc_data_type_print_basic(cu, constant.data_type, constant.data, iio);
	} else {
		HCC_ABORT("unhandled type '%u'", constant.data_type);
	}
}

bool hcc_constant_as_uint(HccCU* cu, HccConstant constant, uint64_t* out) {
	if (!hcc_constant_read_int_extend_64(cu, constant, out)) {
		return false;
	}

	if (HCC_DATA_TYPE_IS_AST_BASIC(constant.data_type) && HCC_AST_BASIC_DATA_TYPE_IS_SINT(cu, HCC_DATA_TYPE_AUX(constant.data_type))) {
		if ((int64_t)*out < 0) {
			return false;
		}
	}

	return true;
}

bool hcc_constant_as_sint(HccCU* cu, HccConstant constant, int64_t* out) {
	if (!hcc_constant_read_int_extend_64(cu, constant, (uint64_t*)out)) {
		return false;
	}

	if (
		HCC_DATA_TYPE_IS_AST_BASIC(constant.data_type) &&
		HCC_AST_BASIC_DATA_TYPE_IS_UINT(cu, HCC_DATA_TYPE_AUX(constant.data_type)) &&
		cu->dtt.basic_type_size_and_aligns[HCC_DATA_TYPE_AUX(constant.data_type)] == sizeof(uint64_t)
	) {
		if ((uint64_t)*out > INT64_MAX) {
			return false;
		}
	}

	return true;
}

bool hcc_constant_as_sint32(HccCU* cu, HccConstant constant, int32_t* out) {
	int64_t value;
	if (!hcc_constant_read_int_extend_64(cu, constant, (uint64_t*)&value)) {
		return false;
	}

	if (value < INT32_MIN || value > INT32_MAX) {
		return false;
	}

	*out = value;
	return true;
}

bool hcc_constant_as_float(HccCU* cu, HccConstant constant, double* out) {
	if (HCC_DATA_TYPE_IS_AST_BASIC(constant.data_type)) {
		switch (constant.data_type) {
			case HCC_AST_BASIC_DATA_TYPE_FLOAT: *out = *(float*)constant.data; return true;
			case HCC_AST_BASIC_DATA_TYPE_DOUBLE: *out = *(double*)constant.data; return true;
			default: {
				uint64_t value;
				if (hcc_constant_read_int_extend_64(cu, constant, &value)) {
					if (HCC_AST_BASIC_DATA_TYPE_IS_SINT(cu, HCC_DATA_TYPE_AUX(constant.data_type))) {
						*out = value;
					} else {
						*out = (int64_t)value;
					}
					return true;
				}
				break;
			};
		}
	}

	return false;
}

// ===========================================
//
//
// Constant Table
//
//
// ===========================================

bool hcc_constant_entry_key_cmp(void* a, void* b, uintptr_t size) {
	HCC_UNUSED(size);
	HccConstantEntry* a_ = a;
	HccConstantEntry* b_ = b;
	return a_->data_type == b_->data_type && a_->size == b_->size && memcmp(a_->data, b_->data, a_->size) == 0;
}

HccHash hcc_constant_key_hash(void* key, uintptr_t size) {
	HCC_DEBUG_ASSERT(size == sizeof(HccConstantEntry), "key is not a HccConstantEntry");

	HccConstantEntry* entry = key;
	HccHash hash = HCC_HASH_FNV_INIT;
	hash = hcc_hash_fnv(&entry->data_type, sizeof(entry->data_type), hash);
	hash = hcc_hash_fnv(&entry->size, sizeof(entry->size), hash);
	hash = hcc_hash_fnv(entry->data, entry->size, hash);
	return hash;
}

void hcc_constant_table_init(HccCU* cu, HccConstantTableSetup* setup) {
	cu->constant_table.entries_hash_table = hcc_hash_table_init(HccConstantEntry, 0, hcc_constant_entry_key_cmp, hcc_constant_key_hash, setup->entries_cap);
	cu->constant_table.data = hcc_stack_init(uint8_t, 0, setup->data_grow_size, setup->data_reserve_size);
	cu->constant_table.composite_fields_buffer = hcc_stack_init(HccConstantId, 0, setup->composite_fields_buffer_grow_count, setup->composite_fields_buffer_reserve_cap);
}

void hcc_constant_table_deinit(HccCU* cu) {
	hcc_hash_table_deinit(cu->constant_table.entries_hash_table);
	hcc_stack_deinit(cu->constant_table.data);
	hcc_stack_deinit(cu->constant_table.composite_fields_buffer);
}

HccConstantId _hcc_constant_table_deduplicate_end(HccCU* cu, HccDataType data_type, void* data, uint32_t data_size, uint32_t data_align) {
	HCC_UNUSED(data_align);
	HCC_DEBUG_ASSERT(data_type != 0, "data type must be set to a non void type");

	HccConstantEntry entry;
	entry.data = data;
	entry.size = data_size;
	entry.data_type = data_type;

	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->constant_table.entries_hash_table, &entry);
	HccConstantEntry* e = &cu->constant_table.entries_hash_table[insert.idx];
	if (insert.is_new) {
		if (data_size) {
			void* ptr = hcc_stack_push_many_thread_safe(cu->constant_table.data, data_size);
			memcpy(ptr, data, data_size);
			e->data = ptr;
		}
		e->size = data_size;
		atomic_store(&e->data_type, data_type);
	} else {
		//
		// if another thread has just inserted this string into the constant table.
		// then wait until the e data type has been assigned
		while (atomic_load(&e->data_type) == 0) {
			HCC_CPU_RELAX();
		}
	}

	return HccConstantId(insert.idx + 1);
}

HccConstantId hcc_constant_table_deduplicate_basic(HccCU* cu, HccDataType data_type, HccBasic* basic) {
	data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);

	HccString data_type_string = hcc_data_type_string(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type), "internal error: expected a basic type but got '%.*s'", (int)data_type_string.size, data_type_string.data);

	uint64_t size;
	uint64_t align;
	hcc_data_type_size_align(cu, data_type, &size, &align);

	return _hcc_constant_table_deduplicate_end(cu, data_type, basic, size, align);
}

HccConstantId* hcc_constant_table_deduplicate_composite_start(HccCU* cu, HccDataType data_type, uint32_t* fields_count_out) {
	HccString data_type_string = hcc_data_type_string(cu, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_COMPOSITE(data_type), "internal error: expected a composite type but got '%.*s'", (int)data_type_string.size, data_type_string.data);

	uint32_t fields_count = hcc_data_type_composite_fields_count(cu, data_type);
	*fields_count_out = fields_count;
	return hcc_stack_push_many(cu->constant_table.composite_fields_buffer, fields_count);
}

HccConstantId hcc_constant_table_deduplicate_composite_end(HccCU* cu, HccDataType data_type, HccConstantId* fields, uint32_t fields_count) {
	HccConstantId constant_id = _hcc_constant_table_deduplicate_end(cu, data_type, fields, fields_count * sizeof(HccConstantId), alignof(HccConstantId));
	hcc_stack_pop_many(cu->constant_table.composite_fields_buffer, fields_count);
	return constant_id;
}

HccConstantId hcc_constant_table_deduplicate_zero(HccCU* cu, HccDataType data_type) {
	data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);

	if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type) && HCC_AML_INTRINSIC_DATA_TYPE_IS_SCALAR(HCC_DATA_TYPE_AUX(data_type))) {
		//
		// basic type's need to store their zero data into the consant table. this is so that
		// when the spirv code is generated it will generate OpConstant instructions for the constants instead of OpConstantNull.
		// this will allow them to be used as indices in OpAccessChain.
		HccBasic zero = {0};
		return hcc_constant_table_deduplicate_basic(cu, data_type, &zero);
	} else {
		return _hcc_constant_table_deduplicate_end(cu, data_type, NULL, 0, 0);
	}
}

HccConstantId hcc_constant_table_deduplicate_one(HccCU* cu, HccDataType data_type) {
	data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);

	if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type) && HCC_AML_INTRINSIC_DATA_TYPE_IS_SCALAR(HCC_DATA_TYPE_AUX(data_type))) {
		HccBasic one = hcc_basic_from_uint(cu, data_type, 1);
		return hcc_constant_table_deduplicate_basic(cu, data_type, &one);
	} else {
		HCC_ABORT("TODO");
	}
}

HccConstantId hcc_constant_table_deduplicate_minus_one(HccCU* cu, HccDataType data_type) {
	data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);

	if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type) && HCC_AML_INTRINSIC_DATA_TYPE_IS_SCALAR(HCC_DATA_TYPE_AUX(data_type))) {
		HccBasic minus_one = hcc_basic_from_sint(cu, data_type, -1);
		return hcc_constant_table_deduplicate_basic(cu, data_type, &minus_one);
	} else {
		HCC_ABORT("TODO");
	}
}

HccConstant hcc_constant_table_get(HccCU* cu, HccConstantId id) {
	HCC_DEBUG_ASSERT_NON_ZERO(id.idx_plus_one);

	HccConstantEntry* entry = &cu->constant_table.entries_hash_table[id.idx_plus_one - 1];

	HccConstant constant;
	constant.data_type = entry->data_type;
	constant.data = entry->data;
	constant.size = entry->size;
	return constant;
}

HccBasic hcc_constant_table_get_basic(HccCU* cu, HccConstantId id) {
	HCC_DEBUG_ASSERT_NON_ZERO(id.idx_plus_one);
	HccConstantEntry* entry = &cu->constant_table.entries_hash_table[id.idx_plus_one - 1];

	HccDataType data_type = hcc_data_type_lower_ast_to_aml(cu, entry->data_type);
	HCC_ASSERT(HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type), "data type must be convertable to an AML intrinsic type");
	HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(data_type);
	HCC_ASSERT(HCC_AML_INTRINSIC_DATA_TYPE_IS_SCALAR(intrinsic_data_type), "basic eval only work on basic types, not vectors");

	HccBasic basic;
	switch (intrinsic_data_type) {
		case HCC_AML_INTRINSIC_DATA_TYPE_BOOL: basic.bool_ = *(bool*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_S8: basic.s8 = *(int8_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_S16: basic.s16 = *(int16_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_S32: basic.s32 = *(int32_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_S64: basic.s64 = *(int64_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_U8: basic.u8 = *(uint8_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_U16: basic.u16 = *(uint16_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_U32: basic.u32 = *(uint32_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_U64: basic.u64 = *(uint64_t*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_F32: basic.f32 = *(float*)entry->data; break;
		case HCC_AML_INTRINSIC_DATA_TYPE_F64: basic.f64 = *(double*)entry->data; break;
	}

	return basic;
}

// ===========================================
//
//
// Globals
//
//
// ===========================================

HccGS _hcc_gs;
thread_local HccTLS _hcc_tls;

// ===========================================
//
//
// IO Interface
//
//
// ===========================================

uintptr_t _hcc_iio_file_read(HccIIO* iio, void* data_out, uintptr_t size) {
	uintptr_t read_size = fread(data_out, 1, size, iio->handle);
	if (ferror(iio->handle)) {
		clearerr(iio->handle);
		return UINTPTR_MAX;
	}
	return read_size;
}

uintptr_t _hcc_iio_file_write(HccIIO* iio, const void* data, uintptr_t size) {
	uintptr_t write_size = fwrite(data, 1, size, iio->handle);
	if (ferror(iio->handle)) {
		clearerr(iio->handle);
		return UINTPTR_MAX;
	}
	return write_size;
}

uintptr_t _hcc_iio_file_write_fmt(HccIIO* iio, const char* fmt, va_list va_args) {
	int write_size = vfprintf(iio->handle, fmt, va_args);
	if (write_size < 0) {
		return UINTPTR_MAX;
	}
	return write_size;
}

void _hcc_iio_file_flush(HccIIO* iio) {
	fflush(iio->handle);
}

void _hcc_iio_file_close(HccIIO* iio) {
	fclose(iio->handle);
}

uintptr_t _hcc_iio_mem_read(HccIIO* iio, void* data_out, uintptr_t size) {
	uintptr_t read_size = HCC_MIN(iio->size - iio->size, size);
	memcpy(data_out, HCC_PTR_ADD(iio->handle, iio->cursor), read_size);
	return read_size;
}

uintptr_t _hcc_iio_mem_write(HccIIO* iio, const void* data, uintptr_t size) {
	uintptr_t write_size = HCC_MIN(iio->size - iio->size, size);
	memcpy(HCC_PTR_ADD(iio->handle, iio->cursor), data, write_size);
	return write_size;
}

uintptr_t _hcc_iio_mem_write_fmt(HccIIO* iio, const char* fmt, va_list va_args) {
	va_list va_args_copy;
	va_copy(va_args_copy, va_args);

	// add 1 so we have enough room for the null terminator that vsnprintf always outputs
	// vsnprintf will return -1 on an encoding error.
	uintptr_t write_size = vsnprintf(NULL, 0, fmt, va_args_copy) + 1;
	va_end(va_args_copy);
	HCC_DEBUG_ASSERT(write_size >= 1, "a vsnprintf encoding error has occurred");

	write_size = HCC_MIN(iio->size - iio->size, write_size);
	if (write_size == 0) {
		return 0;
	}

	//
	// now call vsnprintf for real this time, with a buffer
	// to actually copy the formatted string.
	char* ptr = HCC_PTR_ADD(iio->handle, iio->cursor);
	vsnprintf(ptr, write_size, fmt, va_args);
	return write_size;
}

HccIIO hcc_iio_file(FILE* f) {
	fseek(f, 0, SEEK_END);
	uintptr_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	HccIIO iio = {
		.handle = f,
		.cursor = 0,
		.size = size,
		.read_fn = _hcc_iio_file_read,
		.write_fn = _hcc_iio_file_write,
		.write_fmt_fn = _hcc_iio_file_write_fmt,
		.flush_fn = _hcc_iio_file_flush,
		.close_fn = _hcc_iio_file_close,
	};
	return iio;
}

HccIIO hcc_iio_memory(void* data, uintptr_t size) {
	HccIIO iio = {
		.handle = data,
		.cursor = 0,
		.size = size,
		.read_fn = _hcc_iio_mem_read,
		.write_fn = _hcc_iio_mem_write,
		.write_fmt_fn = _hcc_iio_mem_write_fmt,
		.flush_fn = NULL,
		.close_fn = NULL,
	};
	return iio;
}

bool hcc_iio_get_ascii_colors_enabled(HccIIO* iio) {
	return iio->ascii_colors_enabled;
}

void hcc_iio_set_ascii_colors_enabled(HccIIO* iio, bool enabled) {
	iio->ascii_colors_enabled = enabled;
}

uintptr_t hcc_iio_read(HccIIO* iio, void* data_out, uintptr_t size) {
	return iio->read_fn(iio, data_out, size);
}

uintptr_t hcc_iio_write(HccIIO* iio, const void* data, uintptr_t size) {
	return iio->write_fn(iio, data, size);
}

uintptr_t hcc_iio_write_fmt(HccIIO* iio, const char* fmt, ...) {
	if (!iio) {
		return 0;
	}
	va_list va_args;
	va_start(va_args, fmt);
	uintptr_t write_size = iio->write_fmt_fn(iio, fmt, va_args);
	va_end(va_args);
	return write_size;
}

void hcc_iio_flush(HccIIO* iio) {
	if (iio->flush_fn) {
		iio->flush_fn(iio);
	}
}

void hcc_iio_close(HccIIO* iio) {
	if (iio->close_fn) {
		iio->close_fn(iio);
	}
}

// ===========================================
//
//
// Message: Error & Warn
//
//
// ===========================================

const char* hcc_message_type_lang_strings[HCC_LANG_COUNT][HCC_MESSAGE_TYPE_COUNT] = {
	[HCC_LANG_ENG] = {
		[HCC_MESSAGE_TYPE_ERROR] = "error",
		[HCC_MESSAGE_TYPE_WARN] = "warning",
	},
};

const char* hcc_message_type_ascii_color_code[HCC_MESSAGE_TYPE_COUNT] = {
	[HCC_MESSAGE_TYPE_ERROR] = "\x1b[1;91m",
	[HCC_MESSAGE_TYPE_WARN] = "\x1b[1;93m",
};

const char* hcc_error_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_ERROR_CODE_COUNT] = {
	[HCC_LANG_ENG] = {
		[HCC_ERROR_CODE_NONE] = "internal error: there is no error code set for this error",

		//
		// ATAGEN
		[HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER] = "invalid token '%c' for macro identifier",
		[HCC_ERROR_CODE_INVALID_TOKEN_MACRO_PARAM_IDENTIFIER] = "invalid token '%c' for macro parameter identifier",
		[HCC_ERROR_CODE_DUPLICATE_MACRO_PARAM_IDENTIFIER] = "duplicate macro argument identifier '%.*s'",
		[HCC_ERROR_CODE_INVALID_MACRO_PARAM_DELIMITER] = "expected a ',' to declaring more macro arguments or a ')' to finish declaring macro arguments",
		[HCC_ERROR_CODE_MACRO_PARAM_VA_ARG_NOT_LAST] = "cannot declare another parameter, the vararg '...' parameter must come last",
		[HCC_ERROR_CODE_MACRO_ALREADY_DEFINED] = "the '%.*s' macro has already been defined and is not identical",
		[HCC_ERROR_CODE_INVALID_INCLUDE_OPERAND] = "expected a '<' or '\"' to define the path to the file you wish to include",
		[HCC_ERROR_CODE_TOO_MANY_INCLUDE_OPERANDS] = "too many operands for the '#include' directive",
		[HCC_ERROR_CODE_INCLUDE_PATH_IS_EMPTY] = "no file path was provided for this include directive",
		[HCC_ERROR_CODE_INCLUDE_PATH_DOES_NOT_EXIST] = "cannot find the include file in any of the search paths",
		[HCC_ERROR_CODE_FAILED_TO_OPEN_FILE_FOR_READ] = "failed to open file for read at '%s'",
		[HCC_ERROR_CODE_CONDITION_HAS_NO_PP_TOKENS] = "condition expands to no preprocessor tokens",
		[HCC_ERROR_CODE_INVALID_PP_BINARY_OP] = "'%s' is not a valid preprocessor binary operator",
		[HCC_ERROR_CODE_INVALID_PP_UNARY_EXPR] = "'%s' is not a valid preprocessor unary expression",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE] = "expected a ')' here to finish the expression",
		[HCC_ERROR_CODE_UNDEFINED_IDENTIFIER_IN_PP_EXPR] = "undefined identifier in preprocessor expression",
		[HCC_ERROR_CODE_EXPECTED_COLON_FOR_TERNARY_OP] =  "expected a ':' for the false side of the ternary expression",
		[HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS] = "expected a decimal integer for a custom line number that can be optionally followed by a string literal for a custom file path. eg. #line 210 \"path/to/file.c\"",
		[HCC_ERROR_CODE_PP_LINE_MUST_BE_MORE_THAN_ZERO] = "the decimal integer for #line must be more than 0 and no more than %d",
		[HCC_ERROR_CODE_TOO_MANY_PP_LINE_OPERANDS] = "#line has got too many operands, we only expect a custom line number and optionally a custom file path",
		[HCC_ERROR_CODE_PP_ERROR] = "#error '%.*s'",
		[HCC_ERROR_CODE_INVALID_PP_PRAGMA_OPERAND] = "the first #pragma operand must be an identifier",
		[HCC_ERROR_CODE_PP_PRAGMA_OPERAND_USED_IN_MAIN_FILE] = "#pragma once cannot be used in the main source file",
		[HCC_ERROR_CODE_TOO_MANY_PP_PRAGMA_OPERANDS] = "too many operands for the '#pragma %.*s' directive",
		[HCC_ERROR_CODE_INVALID_TOKEN_PREPROCESSOR_DIRECTIVE] =  "invalid token '%c' for preprocessor directive",
		[HCC_ERROR_CODE_INVALID_PREPROCESSOR_DIRECTIVE] =  "invalid preprocessor directive '#%.*s'",
		[HCC_ERROR_CODE_PP_ENDIF_BEFORE_IF] = "'#%s' must follow an open #if/#ifdef/#ifndef",
		[HCC_ERROR_CODE_PP_ELSEIF_CANNOT_FOLLOW_ELSE] = "'#%s' cannot follow an '#else' in the same preprocessor if chain",
		[HCC_ERROR_CODE_PP_IF_UNTERMINATED] = "missing #endif to match this unterminated '#%s'",
		[HCC_ERROR_CODE_INVALID_PP_CONCAT_OPERANDS] = "'%s' and '%s' doesn't combine into valid token",
		[HCC_ERROR_CODE_TOO_MANY_UNDEF_OPERANDS] = "#%s has too many operands, we only expect an identifier operand to undefine a macro",
		[HCC_ERROR_CODE_TOO_MANY_IFDEF_OPERANDS] = "#%s has too many operands, we only expect an identifier operand check if it is defined or not",
		[HCC_ERROR_CODE_PP_DIRECTIVE_NOT_FIRST_ON_LINE] = "invalid token '#', preprocessor directives must be the first non-whitespace on the line",
		[HCC_ERROR_CODE_INVALID_OCTAL_DIGIT] = "octal digits must be from 0 to 7 inclusively",
		[HCC_ERROR_CODE_MAX_UINT_OVERFLOW] = "integer literal is too large and will overflow a uint64_t integer",
		[HCC_ERROR_CODE_MAX_SINT_OVERFLOW] = "integer literal is too large and will overflow a int64_t integer",
		[HCC_ERROR_CODE_MAX_SINT_OVERFLOW_DECIMAL] = "integer literal is too large and will overflow a int64_t integer, consider using 'u' suffix to promote to an unsigned type. e.g. 1000u",
		[HCC_ERROR_CODE_MAX_FLOAT_OVERFLOW] = "float literal is too large and will overflow a double",
		[HCC_ERROR_CODE_U_SUFFIX_ALREADY_USED] = "the 'u' suffix can only be applied once",
		[HCC_ERROR_CODE_U_SUFFIX_ON_FLOAT] = "the 'u' suffix cannot be applied to a floating point literal",
		[HCC_ERROR_CODE_L_SUFFIX_ON_FLOAT] = "the 'l' suffix must be applied to a double literal not a float",
		[HCC_ERROR_CODE_LONG_DOUBLE_IS_UNSUPPORTED] = "the 'l' suffix for a long double is unsupported",
		[HCC_ERROR_CODE_FLOAT_HAS_DOUBLE_FULL_STOP] = "float literals can only have a single '.'",
		[HCC_ERROR_CODE_FLOAT_MUST_BE_DECIMAL] = "octal and hexidecimal digits are not supported for float literals",
		[HCC_ERROR_CODE_FLOAT_SUFFIX_MUST_FOLLOW_DECIMAL_PLACE] = "the 'f' suffix must come after a decimal place/full stop '.' e.g. 0.f or 1.0f",
		[HCC_ERROR_CODE_INVALID_INTEGER_LITERALS] = "invalid suffix for integer literals",
		[HCC_ERROR_CODE_INVALID_FLOAT_LITERALS] = "invalid suffix for float literals",
		[HCC_ERROR_CODE_NO_BRACKETS_OPEN] = "no brackets are open to close '%s'",
		[HCC_ERROR_CODE_INVALID_CLOSE_BRACKET_PAIR] = "expected to close bracket pair with '%s' but got '%s'",
		[HCC_ERROR_CODE_UNCLOSED_STRING_LITERAL] = "unclosed string literal. close with '%c' or strings spanning to the next line must end the line with '\\'",
		[HCC_ERROR_CODE_MACRO_STARTS_WITH_CONCAT] = "macro cannot start with '##', this operator is used to concatinate two tokens. eg. ident ## ifier and 100. ## f",
		[HCC_ERROR_CODE_STRINGIFY_MUST_BE_MACRO_PARAM] = "macro stringify '#' operator can only be used on a macro parameter",
		[HCC_ERROR_CODE_INVALID_TOKEN] = "invalid token '%c'",
		[HCC_ERROR_CODE_INVALID_TOKEN_HASH_IN_PP_OPERAND] = "invalid token '#', not allowed in preprocessor operand",
		[HCC_ERROR_CODE_EXPECTED_IDENTIFIER_PP_IF_DEFINED] = "expected an 'identifier' of a macro to follow the 'defined' preprocessor unary operator",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_DEFINED] = "expected an ')' to finish the 'defined' preprocessor unary operator",
		[HCC_ERROR_CODE_INVALID_USE_OF_VA_ARGS] = "'__VA_ARGS__' can only be used in a function-like macro that has '...' as it's last parameter",
		[HCC_ERROR_CODE_VA_ARGS_IN_MACRO_PARAMETER] = "'__VA_ARGS__' cannot be used as a macro parameter. use '...' here as the  last parameter to have variable arguments for this macro. and use '__VA_ARGS__' to in the macro itself.",
		[HCC_ERROR_CODE_NOT_ENOUGH_MACRO_ARGUMENTS] = "not enough arguments. expected '%u' but got '%u'",
		[HCC_ERROR_CODE_TOO_MANY_MACRO_ARGUMENTS] = "too many arguments. expected '%u' but got '%u'",

		//
		// ASTGEN
		[HCC_ERROR_CODE_CANNOT_FIND_FIELD] = "cannot find a '%.*s' field in the '%.*s' type",
		[HCC_ERROR_CODE_CANNOT_FIND_FIELD_VECTOR] = "cannot find a '%.*s' field in the '%.*s' vector type",
		[HCC_ERROR_CODE_DUPLICATE_FIELD_IDENTIFIER] = "duplicate field identifier '%.*s' in '%.*s'",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_CONDITION] =  "the condition expression must be convertable to a boolean but got '%.*s'",
		[HCC_ERROR_CODE_MISSING_SEMICOLON] = "missing ';' to end the statement",
		[HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_INTERNAL] = "redefinition of the '%.*s' identifier in this source file",
		[HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_EXTERNAL] = "redefinition of the '%.*s' identifier in this compilation unit",
		[HCC_ERROR_CODE_FUNCTION_PROTOTYPE_MISMATCH] = "function prototype mismatch for '%.*s'",
		[HCC_ERROR_CODE_FUNCTION_BODY_MISMATCH] = "function body mismatch for '%.*s'",
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ENUM] = "expected '{' to declare enum values for enum '%.*s' but got '%s'",
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ENUM_GOT_SEMICOLON] = "named enum '%.*s' cannot be forward declared, please provide enum values like so. enum '%.*s' { ENUM_ONE };",
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_UNNAMED_ENUM] = "expected '{' to declare enum values for an unnamed enum but got '%s'",
		[HCC_ERROR_CODE_REIMPLEMENTATION_DATA_TYPE] = "redefinition of '%.*s' data type",
		[HCC_ERROR_CODE_EMPTY_ENUM] = "cannot have an empty enum, please declare some identifiers inside the {}",
		[HCC_ERROR_CODE_EXPECTED_IDENTIFIER_ENUM_VALUE] = "expected an identifier for the enum value name",
		[HCC_ERROR_CODE_ENUM_VALUE_OVERFLOW] = "enum value overflows a 32 bit signed integer",
		[HCC_ERROR_CODE_ENUM_VALUE_INVALID_FORMAT] = "expected a constant integer value that fits into signed 32 bits",
		[HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR_WITH_EXPLICIT_VALUE] = "expected a ',' to declare another value or a '}' to finish the enum values",
		[HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR] = "expected an '=' to assign a value explicitly, ',' to declare another value or a '}' to finish the enum values",
		[HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_FUNCTION] = "function '%.*s' is not a valid intrinsic for this compiler version",
		[HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_STRUCT] = "'struct %.*s' is not a valid intrinsic for this compiler version",
		[HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_UNION] = "'union %.*s' is not a valid intrinsic for this compiler version",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT] = "only one of these can be used per field: '%s' or '%s'",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT] = "the '%s' keyword cannot be used on this structure declaration",
		[HCC_ERROR_CODE_NOT_AVAILABLE_FOR_UNION] = "the '%s' keyword can only be used on a 'struct' and not a 'union'",
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ANON_STRUCT_TYPE] = "expected '{' to declare fields for an anonymous struct",
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ANON_UNION_TYPE] = "expected '{' to declare fields for an anonymous union",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT_FIELD] = "the '%s' keyword cannot be used on this structure field declaration",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT_FIELD] = "only one of these can be used per field: '%s'",
		[HCC_ERROR_CODE_COMPOUND_FIELD_INVALID_TERMINATOR] = "expected 'type name', 'struct' or 'union' to declare another field or '}' to finish declaring the compound type fields",
		[HCC_ERROR_CODE_COMPOUND_FIELD_MISSING_NAME] = "expected an identifier for the field name",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELDS_COUNT] = "expected intrinsic struct '%.*s' to have '%u' fields but got '%u'",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELD] = "expected this intrinsic field to be '%.*s %.*s' for this compiler version",
		[HCC_ERROR_CODE_INTRINSIC_VECTOR_INVALID_SIZE_AND_ALIGN] = "expected the size and align for the intrinsic vector type '%.*s' to be size '%u' and align '%u' but got size '%u' and align '%u'",
		[HCC_ERROR_CODE_MISSING_RASTERIZER_STATE_SPECIFIER] = "'%s' specifier must be placed before this struct definition before using '%s'",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_ALIGNAS] = "expected '(' to follow _Alignas that contains a type or integer constant. eg. _Alignas(int) or _Alignas(16)",
		[HCC_ERROR_CODE_ALIGNAS_ON_SPECIAL_COMPOUND_DATA_TYPE] = "_Alignas cannot be used on structs declare with the HCC_DEFINE_* macros",
		[HCC_ERROR_CODE_INVALID_ALIGNAS_INT_CONSTANT] = "_Alignas integer constant must be an unsigned value",
		[HCC_ERROR_CODE_INVALID_ALIGNAS_OPERAND] = "invalid _Alignas operand. expected a type or integer constant. eg. _Alignas(int) or _Alignas(16)",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_ALIGNAS] = "expected ')' to follow the type or integer constant for _Alignas",
		[HCC_ERROR_CODE_ALIGNAS_REDUCES_ALIGNMENT] = "_Alignas cannot specify a lower alignment to '%zu' when it already has '%zu'",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_GENERIC] = "expected '(' to specify the _Generic(expression, data_type: expression, ...)",
		[HCC_ERROR_CODE_EXPECTED_COMMA_GENERIC] = "expected ',' to begin the data_type: expression list for _Generic",
		[HCC_ERROR_CODE_EXPECTED_COMMA_OR_PARENTHESIS_CLOSE_GENERIC] = "expected ')' to finish the _Generic or ',' to define the next data_type: expression list entry",
		[HCC_ERROR_CODE_EXPECTED_DATA_TYPE_CASE_GENERIC] = "expected data type for _Generic case but got '%s'",
		[HCC_ERROR_CODE_EXPECTED_COLON_GENERIC] = "expected ':' after data type followed by expression",
		[HCC_ERROR_CODE_EXPECTED_DUPLICATE_CASE_GENERIC] = "duplicate _Generic case for data type '%.*s'",
		[HCC_ERROR_CODE_NO_DATA_TYPE_CASE_GENERIC] = "no case for data type '%.*s' for _Generic",
		[HCC_ERROR_CODE_EXPECTED_NON_VOID_DATA_TYPE] = "expected non 'void' data type",
		[HCC_ERROR_CODE_EXPECTED_TYPE_NAME] = "expected a 'type name' here but got '%s'",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_VECTOR_T] = "expected '(' to specify the arguments eg: __hcc_vector_t(scalar_t, num_comps)",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_VECTOR_T] = "expected ')' to finish the vector type",
		[HCC_ERROR_CODE_EXPECTED_BUILTIN_SCALAR_TYPE] = "expected a builtin scalar type here but got '%s'",
		[HCC_ERROR_CODE_EXPECTED_COMMA_VECTOR_T] = "expected a ',' followed by number of components for the vector type",
		[HCC_ERROR_CODE_EXPECTED_INTEGER_VECTOR_T] = "expected a positive integer between 2 ... 4 to define the number of components for the vector type",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_RESOURCE_TYPE_GENERIC] = "expected '(' to specify the generic type for the '%s' resource type",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_RESOURCE_TYPE_GENERIC] = "expected ')' to follow the resource generic type",
		[HCC_ERROR_CODE_EXPECTED_NO_CONST_QUALIFIERS_FOR_BUFFER_TYPE] = "buffer element data type is does not allow 'const' qualifier, please use the RoBuffer(T) instead",
		[HCC_ERROR_CODE_EXPECTED_NO_VOLATILE_QUALIFIERS_FOR_BUFFER_TYPE] = "buffer element data type is does not allow 'volatile' qualifier",
		[HCC_ERROR_CODE_EXPECTED_NO_TYPE_QUALIFIERS_FOR_TEXTURE_TYPE] = "texture element data type is does not allow 'const', 'volatile' or '_Atomic' qualifier",
		[HCC_ERROR_CODE_INVALID_TEXEL_TYPE] = "expected texel type to be a intrinsic type but got '%.*s'",
		[HCC_ERROR_CODE_INVALID_BUFFER_ELEMENT_TYPE] = "expected the ConstBuffer/RO/RWElementBuffer type to be defined using the HCC_DEFINE_BUFFER_ELEMENT macro but '%.*s' is not",
		[HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE] = "'unsigned' and 'signed' cannot be used on this non-integer type",
		[HCC_ERROR_CODE_COMPLEX_ON_NON_FLOAT_TYPE] = "'_Complex' cannot be used on this non-floating-point type",
		[HCC_ERROR_CODE_MULTIPLE_TYPES_SPECIFIED] = "multiple types specfied",
		[HCC_ERROR_CODE_COMPLEX_UNSUPPORTED_AT_THIS_TIME] = "'_Complex' is unsupported at this time",
		[HCC_ERROR_CODE_UNSIGNED_AND_SIGNED] = "'unsigned' and 'signed' cannot be used together",
		[HCC_ERROR_CODE_DUPLICATE_TYPE_SPECIFIER] = "duplicate type specifier '%s'",
		[HCC_ERROR_CODE_EXPECTED_IDENTIFIER_TYPEDEF] = "expected an 'identifier' for the typedef here but got '%s'",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_TYPEDEF] = "the '%s' keyword cannot be used on this typedef declaration",
		[HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_TYPEDEF] = "'typedef %.*s' is not a valid intrinsic for this compiler version",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_TYPEDEF] = "this intrinsic is supposed to be 'typedef %.*s %.*s' instead of 'typedef %.*s %.*s'",
		[HCC_ERROR_CODE_TYPE_MISMATCH_IMPLICIT_CAST] = "type mismatch '%.*s' is does not implicitly cast to '%.*s'",
		[HCC_ERROR_CODE_TYPE_MISMATCH] = "type mismatch '%.*s' and '%.*s'",
		[HCC_ERROR_CODE_UNSUPPORTED_BINARY_OPERATOR] = "operator '%s' is not supported for data type '%.*s' and '%.*s'",
		[HCC_ERROR_CODE_INVALID_CURLY_EXPR] = "'{' can only be used as the assignment of variable declarations or compound literals",
		[HCC_ERROR_CODE_FIELD_DESIGNATOR_ON_ARRAY_TYPE] = "field designator cannot be used for an the '%.*s' array type, please use '[' instead",
		[HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_DESIGNATOR] = "expected an the field identifier that you wish to initialize from '%.*s'",
		[HCC_ERROR_CODE_ARRAY_DESIGNATOR_ON_COMPOUND_TYPE] = "array designator cannot be used for an the '%.*s' compound type, please use '.' instead",
		[HCC_ERROR_CODE_EXPECTED_INTEGER_FOR_ARRAY_IDX] = "expected a constant unsigned integer here to index a value from the '%.*s' array type",
		[HCC_ERROR_CODE_ARRAY_INDEX_OUT_OF_BOUNDS] = "index is out of bounds for this array, expected a value between '0' - '%zu'",
		[HCC_ERROR_CODE_ARRAY_DESIGNATOR_EXPECTED_SQUARE_BRACE_CLOSE] = "expected ']' to finish the array designator",
		[HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_ARRAY_DESIGNATOR] = "expected an '=' to assign a value or a '[' for an array designator",
		[HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_FIELD_DESIGNATOR] = "expected an '=' to assign a value or a '.' for an field designator",
		[HCC_ERROR_CODE_EXPECTED_ASSIGN] = "expected an '=' to assign a '%.*s' value",
		[HCC_ERROR_CODE_UNARY_OPERATOR_NOT_SUPPORTED] = "unary operator '%s' is not supported for the '%.*s' data type",
		[HCC_ERROR_CODE_UNDECLARED_IDENTIFIER] = "undeclared identifier '%.*s'",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_EXPR] = "expected a ')' here to finish the expression",
		[HCC_ERROR_CODE_INVALID_CAST] = "cannot cast '%.*s' to '%.*s'",
		[HCC_ERROR_CODE_INVALID_CURLY_INITIALIZER_LIST_END] = "expected a '}' to finish the initializer list or a ',' to declare another initializer",
		[HCC_ERROR_CODE_SIZEALIGNOF_TYPE_OPERAND_NOT_WRAPPED] = "the type after '%s' be wrapped in parenthesis. eg. sizeof(uint32_t)",
		[HCC_ERROR_CODE_EXPECTED_EXPR] = "expected an expression here but got '%s'",
		[HCC_ERROR_CODE_NOT_ENOUGH_FUNCTION_ARGS] = "not enough arguments, expected '%u' but got '%u' for '%.*s'",
		[HCC_ERROR_CODE_TOO_MANY_FUNCTION_ARGS] = "too many arguments, expected '%u' but got '%u' for '%.*s'",
		[HCC_ERROR_CODE_INVALID_FUNCTION_ARG_DELIMITER] = "expected a ',' to declaring more function arguments or a ')' to finish declaring function arguments",
		[HCC_ERROR_CODE_ARRAY_SUBSCRIPT_EXPECTED_SQUARE_BRACE_CLOSE] = "expected ']' to finish the array subscript",
		[HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_ACCESS] = "expected an identifier for the field you wish to access from '%.*s'",
		[HCC_ERROR_CODE_MISSING_COLON_TERNARY_OP] = "expected a ':' for the false side of the ternary operator",
		[HCC_ERROR_CODE_PARENTHISES_USED_ON_NON_FUNCTION] = "unexpected '(', this can only be used when the left expression is a function or pointer to a function",
		[HCC_ERROR_CODE_SQUARE_BRACE_USED_ON_NON_ARRAY_DATA_TYPE] = "unexpected '[', this can only be used when the left expression is an array, pointer, RoBuffer or RwBuffer but got '%.*s'",
		[HCC_ERROR_CODE_FULL_STOP_USED_ON_NON_COMPOUND_DATA_TYPE] = "unexpected '.', this can only be used when the left expression is a struct, union or vector type but got '%.*s'",
		[HCC_ERROR_CODE_ARROW_RIGHT_USED_ON_NON_COMPOUND_DATA_TYPE_POINTER] = "unexpected '->', this can only be used when the left expression is a pointer to a struct, union or vector type but got '%.*s'",
		[HCC_ERROR_CODE_CANNOT_ASSIGN_TO_CONST] = "cannot assign to a target that has a constant data type of '%.*s'",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_CONDITION_EXPR] = "expected a '(' for the condition expression",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_CONDITION_EXPR] = "expected a ')' to finish the condition expression",
		[HCC_ERROR_CODE_EXPECTED_ARRAY_SIZE] = "expected an array size here",
		[HCC_ERROR_CODE_EXPECTED_INTEGER_CONSTANT_ARRAY_SIZE] = "expected the expression to resolve to an integer constant for the array size here",
		[HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_NEGATIVE] = "the array size cannot be negative",
		[HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_ZERO] = "the array size cannot be zero",
		[HCC_ERROR_CODE_ARRAY_DECL_EXPECTED_SQUARE_BRACE_CLOSE] = "expected a ']' after the array size expression",
		[HCC_ERROR_CODE_UNSUPPORTED_SPECIFIER] = "'%s' is currently unsupported",
		[HCC_ERROR_CODE_SPECIFIER_ALREADY_BEEN_USED] = "'%s' has already been used for this declaration",
		[HCC_ERROR_CODE_UNUSED_SPECIFIER] = "the '%s' keyword was used, so we are expecting %s for a declaration but got '%s'",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_VARIABLE_DECL] = "the '%s' keyword cannot be used on variable declarations",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_FUNCTION_DECL] = "the '%s' keyword cannot be used on function declarations",
		[HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_LOCAL] = "redefinition of '%.*s' local variable identifier",
		[HCC_ERROR_CODE_STATIC_VARIABLE_INITIALIZER_MUST_BE_CONSTANT] = "variable declaration is static, so this initializer expression must be a constant",
		[HCC_ERROR_CODE_INVALID_VARIABLE_DECL_TERMINATOR] = "expected a ';' to end the declaration or a '=' to assign to the new variable",
		[HCC_ERROR_CODE_INVALID_ELSE] = "expected either 'if' or '{' to follow the 'else' keyword",
		[HCC_ERROR_CODE_INVALID_SWITCH_CONDITION_TYPE] = "switch condition expression must be convertable to a integer type but got '%.*s'",
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_SWITCH_STATEMENT] = "expected a '{' to begin the switch statement",
		[HCC_ERROR_CODE_EXPECTED_WHILE_CONDITION_FOR_DO_WHILE] = "expected 'while' to define the condition of the do while loop",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_FOR] = "expected a '(' to follow 'for' for the operands",
		[HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FOR_VARIABLE_DECL] = "expected an identifier for a variable declaration",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_FOR] = "expected a ')' to finish the for statement condition",
		[HCC_ERROR_CODE_CASE_STATEMENT_OUTSIDE_OF_SWITCH] = "case statement must be inside a switch statement",
		[HCC_ERROR_CODE_SWITCH_CASE_VALUE_MUST_BE_A_CONSTANT] = "the value of a switch case statement must be a constant",
		[HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_CASE] = "':' must follow the constant of the case statement",
		[HCC_ERROR_CODE_DEFAULT_STATMENT_OUTSIDE_OF_SWITCH] = "default case statement must be inside a switch statement",
		[HCC_ERROR_CODE_DEFAULT_STATEMENT_ALREADY_DECLARED] = "default case statement has already been declared",
		[HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_DEFAULT] = "':' must follow the default keyword",
		[HCC_ERROR_CODE_INVALID_BREAK_STATEMENT_USAGE] = "'break' can only be used within a switch statement, a for loop or a while loop",
		[HCC_ERROR_CODE_INVALID_CONTINUE_STATEMENT_USAGE] = "'continue' can only be used within a switch statement, a for loop or a while loop",
		[HCC_ERROR_CODE_MULTIPLE_SHADER_STAGES_ON_FUNCTION] = "only a single shader stage can be specified in a function declaration",
		[HCC_ERROR_CODE_VERTEX_SHADER_MUST_RETURN_RASTERIZER_STATE] = "vertex shader must return a type that was declare with HCC_DEFINE_RASTERIZER_STATE",
		[HCC_ERROR_CODE_FRAGMENT_SHADER_MUST_RETURN_FRAGMENT_STATE] = "fragment shader must return a type that was declare with HCC_DEFINE_FRAGMENT_STATE",
		[HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FUNCTION_PARAM] = "expected an identifier for a function parameter e.g. uint32_t param_identifier",
		[HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_FUNCTION_PARAM] = "redefinition of '%.*s' function parameter identifier",
		[HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_VERTEX] = "invalid function prototype for vertex shader, expected to be 'void vertex(HccVertexSV const* const sv, HccVertexSVOut* const sv_out, BC const *const bc, S *const state_out); where BC is your structure of bundled constants and S defined with HCC_DEFINE_RASTERIZER_STATE or void'",
		[HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_FRAGMENT] = "invalid function prototype for fragment shader, expected to be 'void fragment(HccFragmentSV const* const sv, HccFragmentSVOut* const sv_out, BC const* const bc, S const* const state, F* const frag_out); where BC is your structure of bundled constants, S defined with HCC_DEFINE_RASTERIZER_STATE or void' and F defined with HCC_DEFINE_FRAGMENT_STATE",
		[HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_COMPUTE] = "invalid function prototype for compute shader, expected to be 'void compute(HccComputeSV const* const sv, BC const* const bc); where BC is your structure of bundled constants",
		[HCC_ERROR_CODE_FUNCTION_INVALID_TERMINATOR] = "expected a ',' to declaring more function parameters or a ')' to finish declaring function parameters",
		[HCC_ERROR_CODE_CANNOT_CALL_SHADER_FUNCTION] = "cannot call shaders like regular functions. they can only be used as entry points",
		[HCC_ERROR_CODE_CANNOT_CALL_UNIMPLEMENTED_FUNCTION] = "cannot call a function with no implemention",
		[HCC_ERROR_CODE_UNEXPECTED_TOKEN_FUNCTION_PROTOTYPE_END] = "unexpected token '%s', expected ';' to end the function definition or '{' to define a function body",
		[HCC_ERROR_CODE_UNEXPECTED_TOKEN] = "unexpected token '%s'",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_RASTERIZER_STATE] = "'%.*s' data type is not supported as a RasterizerState field. data type must be an intrinsic type (scalar, vector or matrix)",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_FRAGMENT_STATE] = "'%.*s' data type is not supported as a FragmentState field. data type must be an intrinsic type (scalar, vector or matrix)",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_FUNCTION_PARAM] = "'%.*s' data type is not supported as a function parameter. data type cannot be a HCC_DEFINE_RASTERIZER_STATE, HCC_DEFINE_FRAGMENT_STATE",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_FUNCTION_PARAM_INLINE] = "the function must be 'inline' if you want to use the '%.*s' data type as a function parameter. resources and array data types are only supported with the 'inline' function specifier",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_VARIABLE] = "'%.*s' data type is not supported as a variable. data type cannot be a HCC_DEFINE_RASTERIZER_STATE, HCC_DEFINE_FRAGMENT_STATE",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_POINTER_DATA_TYPE] = "'%.*s' data type is not supported as a pointer data type. data type cannot be a HCC_DEFINE_RASTERIZER_STATE, HCC_DEFINE_FRAGMENT_STATE",
		[HCC_ERROR_CODE_ONLY_SINGLE_POINTERS_ARE_SUPPORTED] = "only a single pointer is supported",
		[HCC_ERROR_CODE_POINTERS_NOT_SUPPORTED] = "pointers are not supported outside of entry point and intrinsics function prototypes",
		[HCC_ERROR_CODE_LOGICAL_ADDRESSED_VAR_USED_BEFORE_ASSIGNED] = "texture, buffer or pointer has been used before it has been assigned too",
		[HCC_ERROR_CODE_LOGICAL_ADDRESSED_CONDITIONALLY_ASSIGNED_BEFORE_USE] = "texture, buffer or pointer has been conditionally assigned too before being used. we need to know these value of this variable at compile time.",
		[HCC_ERROR_CODE_NON_CONST_STATIC_VARIABLE_CANNOT_BE_LOGICALLY_ADDRESSED] = "non-const static variable cannot be a texture, buffer or pointer",
		[HCC_ERROR_CODE_INCOMPLETE_TYPE_USED_BY_VALUE] = "incomplete type '%.*s' has been used by value",
		[HCC_ERROR_CODE_STATIC_AND_EXTERN] = "a declaration cannot be both 'static' and 'extern', please pick one",
		[HCC_ERROR_CODE_THREAD_LOCAL_MUST_BE_GLOBAL] = "'_Thread_local' can only be on global variables",
		[HCC_ERROR_CODE_DISPATCH_GROUP_MUST_BE_GLOBAL] = "'__hcc_dispatch_group' can only be on global variables",
		[HCC_ERROR_CODE_INVOCATION_MUST_BE_GLOBAL] = "'__hcc_invocation' can only be on global variables",
		[HCC_ERROR_CODE_DISPATCH_GROUP_CANNOT_HAVE_INITIALIZER] = "'__hcc_dispatch_group' global variables cannot have initializers and must be uninitialized memory",
		[HCC_ERROR_CODE_STATIC_UNSUPPORTED_ON_SPIRV] = "'static' variables are unsupported by SPIR-V, used HCC_INVOCATION or HCC_DISPATCH_GROUP instead",
		[HCC_ERROR_CODE_THREAD_LOCAL_UNSUPPORTED_ON_SPIRV] = "'_Thread_local' variables are unsupported by SPIR-V, used HCC_INVOCATION or HCC_DISPATCH_GROUP instead",
		[HCC_ERROR_CODE_NOT_ALL_PATHS_RETURN_A_VALUE] = "not all control flow paths return a value, please place a return statement here",
		[HCC_ERROR_CODE_BUNDLED_CONSTANTS_MAX_SIZE_EXCEEDED] = "the maximum bundled constants size of '%u' has been exceed with '%s' with a size of '%u'",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_COMPUTE] = "expected '(' to begin specifing the dispatch group size eg. HCC_COMPUTE(8, 8, 1)",
		[HCC_ERROR_CODE_EXPECTED_NON_ZERO_UINT_COMPUTE] = "expected an non-zero positive integer for the dispatch group size",
		[HCC_ERROR_CODE_EXPECTED_COMMA_COMPUTE] = "expected ',' to define the next dimension of the dispatch group size",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_COMPUTE] = "expected ')' to finish the HCC_COMPUTE specifier",
		[HCC_ERROR_CODE_UNSIZED_ARRAY_REQUIRES_AN_INITIALIZATION] = "a variable that is an unsized array requires initialization, '%.*s %.*s[] = { ... };' or an explicit size '%.*s %.*s[42];'",

		//
		// ASTLINK
		[HCC_ERROR_CODE_LINK_FUNCTION_PROTOTYPE_MISMATCH] = "failed to link the '%.*s' function in two different source files. the function prototype is a mismatch. check to make sure all your parameters are the same type and don't have any samely named struct's or union's with different fields due to conditional compile time code",
		[HCC_ERROR_CODE_LINK_GLOBAL_VARIABLE_MISMATCH] = "failed to link the '%.*s' variable in two different source files. they have a mismatch in their data type. make sure you don't have any samely named struct's or union's with different fields due to conditional compile time code",
		[HCC_ERROR_CODE_LINK_DECLARATION_UNDEFINED] = "failed to link the '%.*s' declartion as a definition does not exist in any source file.",

		//
		// AMLGEN
		[HCC_ERROR_CODE_BUILT_IN_WRITE_ONLY_POINTER_CANNOT_BE_READ] = "can only assign to built in write only pointer, reading from the pointer is not allowed",

		//
		// AMLOPT
		[HCC_ERROR_CODE_FUNCTION_RECURSION] = "function '%.*s' is recursively called! callstack:\n%s",
		[HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED] = "unsupported intrinsic type '%.*s' has been used. if you wish to use this type, please turn on extension support for the follow types: %.*s",
		[HCC_ERROR_CODE_UNION_ONLY_ALLOW_WITH_PHYSICAL_POINTERS] = "the '%.*s' union data type cannot be used unless you enable 'physical pointer' compiler option",
		[HCC_ERROR_CODE_SAMPLE_TEXTURE_WITH_IMPLICIT_MIP_OUTSIDE_OF_FRAGMENT_SHADER] = "function '%.*s' has sample with implicit mip used outside of a fragment shader! callstack:\n%s",
		[HCC_ERROR_CODE_FUNCTION_CANNOT_BE_USED_OUTSIDE_OF_A_FRAGMENT_SHADER] = "function '%.*s' cannot be used outside of a fragment shader! callstack:\n%s",
		[HCC_ERROR_CODE_HLSL_PACKING_NO_STRUCT] = "HLSL packing rules do not allow structs. in future will a proper DXIL backend this error could be worked around",
		[HCC_ERROR_CODE_HLSL_PACKING_NO_UNION] = "HLSL packing rules do not allow unions. in future will a proper DXIL backend this error could be worked around",
		[HCC_ERROR_CODE_HLSL_PACKING_NO_ARRAY] = "HLSL packing rules do not allow arrays. in future will a proper DXIL backend this error could be worked around",
		[HCC_ERROR_CODE_HLSL_PACKING_SIZE_UNDER_4_BYTE] = "HLSL packing rules do not data types under 4 bytes. in future will a proper DXIL backend this error could be worked around",
		[HCC_ERROR_CODE_HLSL_PACKING_IMPLICIT_PADDING] = "HLSL packing rules do not allow implicit padding before field. in future will a proper DXIL backend this error could be worked around",
		[HCC_ERROR_CODE_HLSL_PACKING_OVERFLOW_16_BYTE_BOUNDARY] = "HLSL packing rules do not allow overflowing a 16 byte boundary. in future will a proper DXIL backend this error could be worked around",
	},
};

const char* hcc_warn_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_WARN_CODE_COUNT] = {
	[HCC_LANG_ENG] = {
		[HCC_WARN_CODE_PP_WARNING] = "#warning '%.*s'",

		//
		// astgen
		[HCC_WARN_CODE_CURLY_INITIALIZER_ON_SCALAR] = "'{' should ideally be for structure or array types but got '%.*s'",
		[HCC_WARN_CODE_UNUSED_INITIALIZER_REACHED_END] = "unused initializer, we have reached the end of members for the '%.*s' type",
		[HCC_WARN_CODE_NO_DESIGNATOR_AFTER_DESIGNATOR] = "you should ideally continue using field/array designators after they have been used",
	}
};

void hcc_message_print(HccIIO* iio, HccMessage* message) {
	HccLang lang = HCC_LANG_ENG;
	const char* message_type = hcc_message_type_lang_strings[lang][message->type];
	const char* message_color = hcc_iio_get_ascii_colors_enabled(iio)
		? hcc_message_type_ascii_color_code[message->type]
		: "";
	const char* error_fmt = hcc_iio_get_ascii_colors_enabled(iio)
		? "%s%s\x1b[0m: \x1b[1;97m%.*s\x1b[0m\n"
		: "%s%s: %.*s\n";
	printf(error_fmt, message_color, message_type, (int)message->string.size, message->string.data);

	hcc_message_print_code(iio, message->location);

	if (message->other_location) {
		const char* error_fmt = hcc_iio_get_ascii_colors_enabled(iio)
			? "\x1b[1;97m\noriginally defined here\x1b[0m: \n"
			: "\noriginally defined here: ";
		printf("%s", error_fmt);

		hcc_message_print_code(iio, message->other_location);
	}

	printf("\n");
}

void hcc_message_pushv(HccTask* t, HccMessageType type, HccMessageCode code, HccLocation* location, HccLocation* other_location, va_list va_args) {
	HccMessageSys* sys = &t->message_sys;

	HccMessage* m = hcc_stack_push(sys->elmts);
	sys->used_type_flags |= type;

	m->type = type;
	m->code = code;
	m->location = hcc_stack_push(sys->locations);
	*m->location = *location;
	if (other_location) {
		m->other_location = hcc_stack_push(sys->locations);
		*m->other_location = *other_location;
	} else {
		m->other_location = NULL;
	}

	HccLang lang = HCC_LANG_ENG;
	const char* fmt = NULL;
	switch (type) {
		case HCC_MESSAGE_TYPE_ERROR: fmt = hcc_error_code_lang_fmt_strings[lang][code]; break;
		case HCC_MESSAGE_TYPE_WARN: fmt = hcc_warn_code_lang_fmt_strings[lang][code]; break;
	}

	va_list va_args_copy;
	va_copy(va_args_copy, va_args);

	//
	// add 1 so we have enough room for the null terminator that vsnprintf always outputs
	// vsnprintf will return -1 on an encoding error.
	uintptr_t count = vsnprintf(NULL, 0, fmt, va_args_copy) + 1;
	va_end(va_args_copy);
	HCC_DEBUG_ASSERT(count >= 1, "a vsnprintf encoding error has occurred");

	//
	// resize the stack to have enough room to store the pushed formatted string with the null terminator
	HccString string = hcc_string(hcc_stack_push_many(sys->strings, count), count);

	//
	// now call vsnprintf for real this time, with a buffer
	// to actually copy the formatted string.
	string.size = vsnprintf(string.data, string.size, fmt, va_args);
	m->string = string;
}

void hcc_message_push(HccTask* t, HccMessageType type, HccMessageCode code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_message_pushv(t, type, code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_error_pushv(HccTask* t, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, va_list va_args) {
	hcc_message_pushv(t, HCC_MESSAGE_TYPE_ERROR, error_code, location, other_location, va_args);
}

void hcc_error_push(HccTask* t, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_message_pushv(t, HCC_MESSAGE_TYPE_ERROR, error_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_warn_pushv(HccTask* t, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, va_list va_args) {
	hcc_message_pushv(t, HCC_MESSAGE_TYPE_WARN, warn_code, location, other_location, va_args);
}

void hcc_warn_push(HccTask* t, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_message_pushv(t, HCC_MESSAGE_TYPE_WARN, warn_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_message_print_file_line(HccIIO* iio, HccLocation* location) {
	const char* file_path = location->display_path.data ? location->display_path.data : location->code_file->path_string.data;
	uint32_t display_line = location->display_line ? location->display_line : location->line_start;

	const char* error_fmt = hcc_iio_get_ascii_colors_enabled(iio)
		? "\x1b[1;95mfile\x1b[97m: %s:%u:%u\n\x1b[0m"
		: "file: %s:%u:%u\n";
	printf(error_fmt, file_path, display_line, location->column_start);
}

void hcc_message_print_pasted_buffer(HccIIO* iio, uint32_t line, uint32_t column) {
	const char* error_fmt = hcc_iio_get_ascii_colors_enabled(iio)
		? "\x1b[1;95m<pasted buffer>\x1b[97m: %u:%u\n\x1b[0m"
		: "<pasted buffer>: %u:%u\n";
	printf(error_fmt, line, column);
}

void hcc_message_print_code_line(HccIIO* iio, HccLocation* location, uint32_t display_line_num_size, uint32_t line, uint32_t display_line) {
	uint32_t line_size = hcc_code_file_line_size(location->code_file, line);

	if (line_size == 0) {
		const char* fmt = hcc_iio_get_ascii_colors_enabled(iio)
			? "\x1b[1;94m%*u|\x1b[0m\n"
			: "%*u|\n";
		printf(fmt, display_line_num_size, display_line);
	} else {
		HccCodeFile* code_file = location->code_file;
		uint32_t code_start_idx = code_file->line_code_start_indices[line];
		char* code = &code_file->code.data[code_start_idx];

		char code_without_tabs[1024];
		uint32_t dst_idx = 0;
		uint32_t src_idx = 0;
		for (; dst_idx < HCC_MIN(sizeof(code_without_tabs), line_size); dst_idx += 1, src_idx += 1) {
			char byte = code[src_idx];
			if (byte == '\t') {
				HCC_DEBUG_ASSERT_ARRAY_BOUNDS(dst_idx + 3, sizeof(code_without_tabs));
				//
				// TODO make this a customizable compiler option in HccOption
				code_without_tabs[dst_idx + 0] = ' ';
				code_without_tabs[dst_idx + 1] = ' ';
				code_without_tabs[dst_idx + 2] = ' ';
				code_without_tabs[dst_idx + 3] = ' ';
				dst_idx += 3;
				line_size += 3;
			} else {
				HCC_DEBUG_ASSERT_ARRAY_BOUNDS(dst_idx, sizeof(code_without_tabs));
				code_without_tabs[dst_idx] = byte;
			}
		}

		const char* fmt = hcc_iio_get_ascii_colors_enabled(iio)
			? "\x1b[1;94m%*u|\x1b[0m %.*s\n"
			: "%*u| %.*s\n";
		printf(fmt, display_line_num_size, display_line, line_size, code_without_tabs);
	}
}

void hcc_message_print_code(HccIIO* iio, HccLocation* location) {
	if (location->parent_location) {
		HccLocation* parent_location = location->parent_location;
		hcc_message_print_code(iio, parent_location);
		printf("\n");

		HccPPMacro* macro = location->macro;
		if (macro) {
			HccString ident_string = macro->identifier_string;

			const char* error_fmt = hcc_iio_get_ascii_colors_enabled(iio)
				? "\x1b[1;96mexpanded from macro\x1b[97m: %.*s\n\x1b[0m"
				: "expanded from macro: %.*s\n";
			printf(error_fmt, (int)ident_string.size, ident_string.data);
		}
	}
	hcc_message_print_file_line(iio, location);

	uint32_t error_lines_count = location->line_end - location->line_start;

	//
	// count the number of digits in the largest line number
	uint32_t display_line_num_size = 0;
	uint32_t line = location->line_end + 2;
	while (line) {
		if (line < 10) {
			line = 0;
		} else {
			line /= 10;
		}
		display_line_num_size += 1;
	}

	//
	// TODO make this a customizable compiler option in HccOption
	uint32_t tab_size = 4;

	display_line_num_size = HCC_MAX(display_line_num_size, 5);
	display_line_num_size = hcc_uint32_round_up_to_multiple(display_line_num_size, tab_size) - 2;

	line = location->line_start;
	uint32_t display_line = location->display_line ? location->display_line : location->line_start;
	if (line > 2) {
		hcc_message_print_code_line(iio, location, display_line_num_size, line - 2, display_line - 2);
	}
	if (line > 1) {
		hcc_message_print_code_line(iio, location, display_line_num_size, line - 1, display_line - 1);
	}

	uint32_t column_start = location->column_start;
	uint32_t column_end;
	HccCodeFile* code_file = location->code_file;
	for (uint32_t idx = 0; idx < error_lines_count; idx += 1) {
		uint32_t code_start_idx = code_file->line_code_start_indices[line + idx];
		if (idx + 1 == error_lines_count) {
			column_end = location->column_end;
		} else {
			column_end = hcc_code_file_line_size(code_file, line + idx) + 1;
		}

		hcc_message_print_code_line(iio, location, display_line_num_size, line + idx, display_line + idx);

		//
		// print the padding for to get to the error location on the line
		for (uint32_t i = 0; i < display_line_num_size + 2; i += 1) {
			putchar(' ');
		}
		for (uint32_t i = 0; i < HCC_MAX(column_start, 1) - 1; i += 1) {
			if (code_file->code.data[code_start_idx + i] == '\t') {
				printf("%.*s", tab_size, "        ");
			} else {
				putchar(' ');
			}
		}

		uint32_t column_end_with_tabs = column_end;
		for (uint32_t i = column_start - 1; i < column_end - 1; i += 1) {
			if (code_file->code.data[code_start_idx + i] == '\t') {
				column_end_with_tabs += 3;
			}
		}
		column_end_with_tabs = HCC_MAX(column_end_with_tabs, column_start + 1);

		if (hcc_iio_get_ascii_colors_enabled(iio)) {
			printf("\x1b[1;93m");
		}
		for (uint32_t i = 0; i < column_end_with_tabs - column_start; i += 1) {
			putchar('^');
		}
		if (hcc_iio_get_ascii_colors_enabled(iio)) {
			printf("\x1b[0m");
		}
		printf("\n");
		column_start = 1;
	}

	line = location->line_end - 1;
	uint32_t lines_count = hcc_code_file_lines_count(code_file);
	if (line + 1 < lines_count) {
		hcc_message_print_code_line(iio, location, display_line_num_size, line + 1, display_line + 1);
	}
	if (line + 2 < lines_count) {
		hcc_message_print_code_line(iio, location, display_line_num_size, line + 2, display_line + 2);
	}
}

// ===========================================
//
//
// Intrinsic Declarations
//
//
// ===========================================

const char* hcc_intrinisic_compound_data_type_strings[HCC_COMPOUND_DATA_TYPE_IDX_STRINGS_COUNT] = {
	[HCC_COMPOUND_DATA_TYPE_IDX_HCC_VERTEX_SV] = "HccVertexSV",
	[HCC_COMPOUND_DATA_TYPE_IDX_HCC_VERTEX_SV_OUT] = "HccVertexSVOut",
	[HCC_COMPOUND_DATA_TYPE_IDX_HCC_FRAGMENT_SV] = "HccFragmentSV",
	[HCC_COMPOUND_DATA_TYPE_IDX_HCC_FRAGMENT_SV_OUT] = "HccFragmentSVOut",
	[HCC_COMPOUND_DATA_TYPE_IDX_HCC_COMPUTE_SV] = "HccComputeSV",
};

const char* hcc_intrinisic_function_strings[HCC_FUNCTION_IDX_STRINGS_COUNT] = {
	[HCC_FUNCTION_IDX_F16TOF32] = "f16tof32",
	[HCC_FUNCTION_IDX_F16TOF64] = "f16tof64",
	[HCC_FUNCTION_IDX_F32TOF16] = "f32tof16",
	[HCC_FUNCTION_IDX_F64TOF16] = "f64tof16",
	[HCC_FUNCTION_IDX_PACK_F16X2_F32X2] = "pack_f16x2_f32x2",
	[HCC_FUNCTION_IDX_UNPACK_F16X2_F32X2] = "unpack_f16x2_f32x2",
	[HCC_FUNCTION_IDX_PACK_U16X2_F32X2] = "pack_u16x2_f32x2",
	[HCC_FUNCTION_IDX_UNPACK_U16X2_F32X2] = "unpack_u16x2_f32x2",
	[HCC_FUNCTION_IDX_PACK_S16X2_F32X2] = "pack_s16x2_f32x2",
	[HCC_FUNCTION_IDX_UNPACK_S16X2_F32X2] = "unpack_s16x2_f32x2",
	[HCC_FUNCTION_IDX_PACK_U8X4_F32X4] = "pack_u8x4_f32x4",
	[HCC_FUNCTION_IDX_UNPACK_U8X4_F32X4] = "unpack_u8x4_f32x4",
	[HCC_FUNCTION_IDX_PACK_S8X4_F32X4] = "pack_s8x4_f32x4",
	[HCC_FUNCTION_IDX_UNPACK_S8X4_F32X4] = "unpack_s8x4_f32x4",
	[HCC_FUNCTION_IDX_DISCARD_FRAGMENT] = "discard_fragment",
	[HCC_FUNCTION_IDX_MEMORY_BARRIER_RESOURCE] = "memory_barrier_resource",
	[HCC_FUNCTION_IDX_MEMORY_BARRIER_DISPATCH_GROUP] = "memory_barrier_dispatch_group",
	[HCC_FUNCTION_IDX_MEMORY_BARRIER_ALL] = "memory_barrier_all",
	[HCC_FUNCTION_IDX_CONTROL_BARRIER_RESOURCE] = "control_barrier_resource",
	[HCC_FUNCTION_IDX_CONTROL_BARRIER_DISPATCH_GROUP] = "control_barrier_dispatch_group",
	[HCC_FUNCTION_IDX_CONTROL_BARRIER_ALL] = "control_barrier_all",
};

const char* hcc_intrinisic_function_many_strings[HCC_FUNCTION_MANY_COUNT] = {
	[HCC_FUNCTION_MANY_ANY] = "any",
	[HCC_FUNCTION_MANY_ALL] = "all",
	[HCC_FUNCTION_MANY_ADD] = "add",
	[HCC_FUNCTION_MANY_SUB] = "sub",
	[HCC_FUNCTION_MANY_MUL] = "mul",
	[HCC_FUNCTION_MANY_DIV] = "div",
	[HCC_FUNCTION_MANY_MOD] = "mod",
	[HCC_FUNCTION_MANY_EQ] = "eq",
	[HCC_FUNCTION_MANY_NEQ] = "neq",
	[HCC_FUNCTION_MANY_LT] = "lt",
	[HCC_FUNCTION_MANY_LTEQ] = "lteq",
	[HCC_FUNCTION_MANY_GT] = "gt",
	[HCC_FUNCTION_MANY_GTEQ] = "gteq",
	[HCC_FUNCTION_MANY_NOT] = "not",
	[HCC_FUNCTION_MANY_NEG] = "neg",
	[HCC_FUNCTION_MANY_BITNOT] = "bitnot",
	[HCC_FUNCTION_MANY_MIN] = "min",
	[HCC_FUNCTION_MANY_MAX] = "max",
	[HCC_FUNCTION_MANY_CLAMP] = "clamp",
	[HCC_FUNCTION_MANY_SIGN] = "sign",
	[HCC_FUNCTION_MANY_ABS] = "abs",
	[HCC_FUNCTION_MANY_BITAND] = "bitand",
	[HCC_FUNCTION_MANY_BITOR] = "bitor",
	[HCC_FUNCTION_MANY_BITXOR] = "bitxor",
	[HCC_FUNCTION_MANY_BITSHL] = "bitshl",
	[HCC_FUNCTION_MANY_BITSHR] = "bitshr",
	[HCC_FUNCTION_MANY_FMA] = "fma",
	[HCC_FUNCTION_MANY_FLOOR] = "floor",
	[HCC_FUNCTION_MANY_CEIL] = "ceil",
	[HCC_FUNCTION_MANY_ROUND] = "round",
	[HCC_FUNCTION_MANY_TRUNC] = "trunc",
	[HCC_FUNCTION_MANY_FRACT] = "fract",
	[HCC_FUNCTION_MANY_RADIANS] = "radians",
	[HCC_FUNCTION_MANY_DEGREES] = "degrees",
	[HCC_FUNCTION_MANY_STEP] = "step",
	[HCC_FUNCTION_MANY_SMOOTHSTEP] = "smoothstep",
	[HCC_FUNCTION_MANY_BITSTO] = "bitsto",
	[HCC_FUNCTION_MANY_BITSFROM] = "bitsfrom",
	[HCC_FUNCTION_MANY_SIN] = "sin",
	[HCC_FUNCTION_MANY_COS] = "cos",
	[HCC_FUNCTION_MANY_TAN] = "tan",
	[HCC_FUNCTION_MANY_ASIN] = "asin",
	[HCC_FUNCTION_MANY_ACOS] = "acos",
	[HCC_FUNCTION_MANY_ATAN] = "atan",
	[HCC_FUNCTION_MANY_SINH] = "sinh",
	[HCC_FUNCTION_MANY_COSH] = "cosh",
	[HCC_FUNCTION_MANY_TANH] = "tanh",
	[HCC_FUNCTION_MANY_ASINH] = "asinh",
	[HCC_FUNCTION_MANY_ACOSH] = "acosh",
	[HCC_FUNCTION_MANY_ATANH] = "atanh",
	[HCC_FUNCTION_MANY_ATAN2] = "atan2",
	[HCC_FUNCTION_MANY_POW] = "pow",
	[HCC_FUNCTION_MANY_EXP] = "exp",
	[HCC_FUNCTION_MANY_LOG] = "log",
	[HCC_FUNCTION_MANY_EXP2] = "exp2",
	[HCC_FUNCTION_MANY_LOG2] = "log2",
	[HCC_FUNCTION_MANY_SQRT] = "sqrt",
	[HCC_FUNCTION_MANY_RSQRT] = "rsqrt",
	[HCC_FUNCTION_MANY_ISINF] = "isinf",
	[HCC_FUNCTION_MANY_ISNAN] = "isnan",
	[HCC_FUNCTION_MANY_LERP] = "lerp",
	[HCC_FUNCTION_MANY_CROSS] = "cross",
	[HCC_FUNCTION_MANY_DOT] = "dot",
	[HCC_FUNCTION_MANY_LEN] = "len",
	[HCC_FUNCTION_MANY_NORM] = "norm",
	[HCC_FUNCTION_MANY_REFLECT] = "reflect",
	[HCC_FUNCTION_MANY_REFRACT] = "refract",
	[HCC_FUNCTION_MANY_DDX] = "ddx",
	[HCC_FUNCTION_MANY_DDY] = "ddy",
	[HCC_FUNCTION_MANY_FWIDTH] = "fwidth",
	[HCC_FUNCTION_MANY_DDX_FINE] = "ddx_fine",
	[HCC_FUNCTION_MANY_DDY_FINE] = "ddy_fine",
	[HCC_FUNCTION_MANY_FWIDTH_FINE] = "fwidth_fine",
	[HCC_FUNCTION_MANY_DDX_COARSE] = "ddx_coarse",
	[HCC_FUNCTION_MANY_DDY_COARSE] = "ddy_coarse",
	[HCC_FUNCTION_MANY_FWIDTH_COARSE] = "fwidth_coarse",
	[HCC_FUNCTION_MANY_QUAD_SWAP_X] = "quad_swap_x",
	[HCC_FUNCTION_MANY_QUAD_SWAP_Y] = "quad_swap_y",
	[HCC_FUNCTION_MANY_QUAD_SWAP_DIAGONAL] = "quad_swap_diagonal",
	[HCC_FUNCTION_MANY_QUAD_READ_THREAD] = "quad_read_thread",
	[HCC_FUNCTION_MANY_QUAD_ANY] = "quad_any",
	[HCC_FUNCTION_MANY_QUAD_ALL] = "quad_all",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ANY] = "wave_active_any",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ALL] = "wave_active_all",
	[HCC_FUNCTION_MANY_WAVE_READ_THREAD] = "wave_read_thread",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ALL_EQUAL] = "wave_active_all_equal",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_MIN] = "wave_active_min",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_MAX] = "wave_active_max",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_SUM] = "wave_active_sum",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_SUM] = "wave_active_prefix_sum",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PRODUCT] = "wave_active_product",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_PRODUCT] = "wave_active_prefix_product",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_COUNT_BITS] = "wave_active_count_bits",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_COUNT_BITS] = "wave_active_prefix_count_bits",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_AND] = "wave_active_bit_and",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_OR] = "wave_active_bit_or",
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_XOR] = "wave_active_bit_xor",
	[HCC_FUNCTION_MANY_ATOMIC_LOAD] = "atomic_load",
	[HCC_FUNCTION_MANY_ATOMIC_STORE] = "atomic_store",
	[HCC_FUNCTION_MANY_ATOMIC_EXCHANGE] = "atomic_exchange",
	[HCC_FUNCTION_MANY_ATOMIC_COMPARE_EXCHANGE] = "atomic_compare_exchange",
	[HCC_FUNCTION_MANY_ATOMIC_ADD] = "atomic_add",
	[HCC_FUNCTION_MANY_ATOMIC_SUB] = "atomic_sub",
	[HCC_FUNCTION_MANY_ATOMIC_MIN] = "atomic_min",
	[HCC_FUNCTION_MANY_ATOMIC_MAX] = "atomic_max",
	[HCC_FUNCTION_MANY_ATOMIC_BIT_AND] = "atomic_bit_and",
	[HCC_FUNCTION_MANY_ATOMIC_BIT_OR] = "atomic_bit_or",
	[HCC_FUNCTION_MANY_ATOMIC_BIT_XOR] = "atomic_bit_xor",
	[HCC_FUNCTION_MANY_LOAD_TEXTURE] = "load_texture",
	[HCC_FUNCTION_MANY_FETCH_TEXTURE] = "fetch_texture",
	[HCC_FUNCTION_MANY_SAMPLE_TEXTURE] = "sample_texture",
	[HCC_FUNCTION_MANY_SAMPLE_MIP_BIAS_TEXTURE] = "sample_mip_bias_texture",
	[HCC_FUNCTION_MANY_SAMPLE_MIP_GRADIENT_TEXTURE] = "sample_mip_gradient_texture",
	[HCC_FUNCTION_MANY_SAMPLE_MIP_LEVEL_TEXTURE] = "sample_mip_level_texture",
	[HCC_FUNCTION_MANY_GATHER_RED_TEXTURE] = "gather_red_texture",
	[HCC_FUNCTION_MANY_GATHER_GREEN_TEXTURE] = "gather_green_texture",
	[HCC_FUNCTION_MANY_GATHER_BLUE_TEXTURE] = "gather_blue_texture",
	[HCC_FUNCTION_MANY_GATHER_ALPHA_TEXTURE] = "gather_alpha_texture",
	[HCC_FUNCTION_MANY_STORE_TEXTURE] = "store_texture",
};

HccManyTypeClass hcc_intrinisic_function_many_support[HCC_FUNCTION_MANY_COUNT] = {
	[HCC_FUNCTION_MANY_ANY] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_ALL] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_ADD] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_SUB] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_MUL] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_DIV] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_MOD] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_EQ] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_NEQ] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_LT] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_LTEQ] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_GT] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_GTEQ] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_NOT] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_NEG] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITNOT] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_MIN] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_MAX] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_CLAMP] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_SIGN] = HCC_MANY_TYPE_CLASS_SIGNED_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ABS] = HCC_MANY_TYPE_CLASS_SIGNED_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITAND] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITOR] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITXOR] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITSHL] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITSHR] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_FMA] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_FLOOR] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_CEIL] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ROUND] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_TRUNC] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_FRACT] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_RADIANS] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_DEGREES] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_STEP] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_SMOOTHSTEP] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITSTO] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_BITSFROM] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_SIN] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_COS] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_TAN] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ASIN] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ACOS] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATAN] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_SINH] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_COSH] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_TANH] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ASINH] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ACOSH] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATANH] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATAN2] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_POW] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_EXP] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_LOG] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_EXP2] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_LOG2] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_SQRT] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_RSQRT] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ISINF] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ISNAN] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_LERP] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_CROSS] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_DOT] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_LEN] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_NORM] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_REFLECT] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_REFRACT] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_VECTOR,
	[HCC_FUNCTION_MANY_DDX] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_DDY] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_FWIDTH] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_DDX_FINE] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_DDY_FINE] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_FWIDTH_FINE] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_DDX_COARSE] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_DDY_COARSE] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_FWIDTH_COARSE] = HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_QUAD_SWAP_X] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_QUAD_SWAP_Y] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_QUAD_SWAP_DIAGONAL] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_QUAD_READ_THREAD] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_QUAD_ANY] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_QUAD_ALL] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ANY] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ALL] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_READ_THREAD] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ALL_EQUAL] = HCC_MANY_TYPE_CLASS_SCALARS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_MIN] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_MAX] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_SUM] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_SUM] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PRODUCT] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_PRODUCT] = HCC_MANY_TYPE_CLASS_NUMBERS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_COUNT_BITS] = HCC_MANY_TYPE_CLASS_BOOL | HCC_MANY_TYPE_CLASS_OPS_SCALAR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_COUNT_BITS] = HCC_MANY_TYPE_CLASS_BOOL | HCC_MANY_TYPE_CLASS_OPS_SCALAR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_AND] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_OR] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_XOR] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_LOAD] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_STORE] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_EXCHANGE] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_FLOAT | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_COMPARE_EXCHANGE] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_ADD] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_SUB] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_MIN] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_MAX] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_BIT_AND] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_BIT_OR] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_ATOMIC_BIT_XOR] = HCC_MANY_TYPE_CLASS_INTS | HCC_MANY_TYPE_CLASS_OPS_SCALAR_VECTOR,
	[HCC_FUNCTION_MANY_LOAD_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_FETCH_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_MIP_BIAS_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_MIP_GRADIENT_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_MIP_LEVEL_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_RED_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_GREEN_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_BLUE_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_ALPHA_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
	[HCC_FUNCTION_MANY_STORE_TEXTURE] = HCC_MANY_TYPE_CLASS_TEXTURE,
};

HccAMLOp hcc_intrinisic_function_many_aml_ops[HCC_FUNCTION_MANY_COUNT] = {
	[HCC_FUNCTION_MANY_ANY] = HCC_AML_OP_ANY,
	[HCC_FUNCTION_MANY_ALL] = HCC_AML_OP_ALL,
	[HCC_FUNCTION_MANY_ADD] = HCC_AML_OP_ADD,
	[HCC_FUNCTION_MANY_SUB] = HCC_AML_OP_SUBTRACT,
	[HCC_FUNCTION_MANY_MUL] = HCC_AML_OP_MULTIPLY,
	[HCC_FUNCTION_MANY_DIV] = HCC_AML_OP_DIVIDE,
	[HCC_FUNCTION_MANY_MOD] = HCC_AML_OP_MODULO,
	[HCC_FUNCTION_MANY_EQ] = HCC_AML_OP_EQUAL,
	[HCC_FUNCTION_MANY_NEQ] = HCC_AML_OP_NOT_EQUAL,
	[HCC_FUNCTION_MANY_LT] = HCC_AML_OP_LESS_THAN,
	[HCC_FUNCTION_MANY_LTEQ] = HCC_AML_OP_LESS_THAN_OR_EQUAL,
	[HCC_FUNCTION_MANY_GT] = HCC_AML_OP_GREATER_THAN,
	[HCC_FUNCTION_MANY_GTEQ] = HCC_AML_OP_GREATER_THAN_OR_EQUAL,
	[HCC_FUNCTION_MANY_NOT] = HCC_AML_OP_NO_OP,
	[HCC_FUNCTION_MANY_NEG] = HCC_AML_OP_NEGATE,
	[HCC_FUNCTION_MANY_BITNOT] = HCC_AML_OP_NO_OP,
	[HCC_FUNCTION_MANY_MIN] = HCC_AML_OP_MIN,
	[HCC_FUNCTION_MANY_MAX] = HCC_AML_OP_MAX,
	[HCC_FUNCTION_MANY_CLAMP] = HCC_AML_OP_CLAMP,
	[HCC_FUNCTION_MANY_SIGN] = HCC_AML_OP_SIGN,
	[HCC_FUNCTION_MANY_ABS] = HCC_AML_OP_ABS,
	[HCC_FUNCTION_MANY_BITAND] = HCC_AML_OP_BIT_AND,
	[HCC_FUNCTION_MANY_BITOR] = HCC_AML_OP_BIT_OR,
	[HCC_FUNCTION_MANY_BITXOR] = HCC_AML_OP_BIT_XOR,
	[HCC_FUNCTION_MANY_BITSHL] = HCC_AML_OP_BIT_SHIFT_LEFT,
	[HCC_FUNCTION_MANY_BITSHR] = HCC_AML_OP_BIT_SHIFT_RIGHT,
	[HCC_FUNCTION_MANY_FMA] = HCC_AML_OP_FMA,
	[HCC_FUNCTION_MANY_FLOOR] = HCC_AML_OP_FLOOR,
	[HCC_FUNCTION_MANY_CEIL] = HCC_AML_OP_CEIL,
	[HCC_FUNCTION_MANY_ROUND] = HCC_AML_OP_ROUND,
	[HCC_FUNCTION_MANY_TRUNC] = HCC_AML_OP_TRUNC,
	[HCC_FUNCTION_MANY_FRACT] = HCC_AML_OP_FRACT,
	[HCC_FUNCTION_MANY_RADIANS] = HCC_AML_OP_RADIANS,
	[HCC_FUNCTION_MANY_DEGREES] = HCC_AML_OP_DEGREES,
	[HCC_FUNCTION_MANY_STEP] = HCC_AML_OP_STEP,
	[HCC_FUNCTION_MANY_SMOOTHSTEP] = HCC_AML_OP_SMOOTHSTEP,
	[HCC_FUNCTION_MANY_SIN] = HCC_AML_OP_SIN,
	[HCC_FUNCTION_MANY_COS] = HCC_AML_OP_COS,
	[HCC_FUNCTION_MANY_TAN] = HCC_AML_OP_TAN,
	[HCC_FUNCTION_MANY_ASIN] = HCC_AML_OP_ASIN,
	[HCC_FUNCTION_MANY_ACOS] = HCC_AML_OP_ACOS,
	[HCC_FUNCTION_MANY_ATAN] = HCC_AML_OP_ATAN,
	[HCC_FUNCTION_MANY_SINH] = HCC_AML_OP_SINH,
	[HCC_FUNCTION_MANY_COSH] = HCC_AML_OP_COSH,
	[HCC_FUNCTION_MANY_TANH] = HCC_AML_OP_TANH,
	[HCC_FUNCTION_MANY_ASINH] = HCC_AML_OP_ASINH,
	[HCC_FUNCTION_MANY_ACOSH] = HCC_AML_OP_ACOSH,
	[HCC_FUNCTION_MANY_ATANH] = HCC_AML_OP_ATANH,
	[HCC_FUNCTION_MANY_ATAN2] = HCC_AML_OP_ATAN2,
	[HCC_FUNCTION_MANY_POW] = HCC_AML_OP_POW,
	[HCC_FUNCTION_MANY_EXP] = HCC_AML_OP_EXP,
	[HCC_FUNCTION_MANY_LOG] = HCC_AML_OP_LOG,
	[HCC_FUNCTION_MANY_EXP2] = HCC_AML_OP_EXP2,
	[HCC_FUNCTION_MANY_LOG2] = HCC_AML_OP_LOG2,
	[HCC_FUNCTION_MANY_SQRT] = HCC_AML_OP_SQRT,
	[HCC_FUNCTION_MANY_RSQRT] = HCC_AML_OP_RSQRT,
	[HCC_FUNCTION_MANY_ISINF] = HCC_AML_OP_ISINF,
	[HCC_FUNCTION_MANY_ISNAN] = HCC_AML_OP_ISNAN,
	[HCC_FUNCTION_MANY_LERP] = HCC_AML_OP_LERP,
	[HCC_FUNCTION_MANY_CROSS] = HCC_AML_OP_CROSS,
	[HCC_FUNCTION_MANY_DOT] = HCC_AML_OP_DOT,
	[HCC_FUNCTION_MANY_LEN] = HCC_AML_OP_LEN,
	[HCC_FUNCTION_MANY_NORM] = HCC_AML_OP_NORM,
	[HCC_FUNCTION_MANY_REFLECT] = HCC_AML_OP_REFLECT,
	[HCC_FUNCTION_MANY_REFRACT] = HCC_AML_OP_REFRACT,
	[HCC_FUNCTION_MANY_DDX] = HCC_AML_OP_DDX,
	[HCC_FUNCTION_MANY_DDY] = HCC_AML_OP_DDY,
	[HCC_FUNCTION_MANY_FWIDTH] = HCC_AML_OP_FWIDTH,
	[HCC_FUNCTION_MANY_DDX_FINE] = HCC_AML_OP_DDX_FINE,
	[HCC_FUNCTION_MANY_DDY_FINE] = HCC_AML_OP_DDY_FINE,
	[HCC_FUNCTION_MANY_FWIDTH_FINE] = HCC_AML_OP_FWIDTH_FINE,
	[HCC_FUNCTION_MANY_DDX_COARSE] = HCC_AML_OP_DDX_COARSE,
	[HCC_FUNCTION_MANY_DDY_COARSE] = HCC_AML_OP_DDY_COARSE,
	[HCC_FUNCTION_MANY_FWIDTH_COARSE] = HCC_AML_OP_FWIDTH_COARSE,
	[HCC_FUNCTION_MANY_QUAD_SWAP_X] = HCC_AML_OP_QUAD_SWAP_X,
	[HCC_FUNCTION_MANY_QUAD_SWAP_Y] = HCC_AML_OP_QUAD_SWAP_Y,
	[HCC_FUNCTION_MANY_QUAD_SWAP_DIAGONAL] = HCC_AML_OP_QUAD_SWAP_DIAGONAL,
	[HCC_FUNCTION_MANY_QUAD_READ_THREAD] = HCC_AML_OP_QUAD_READ_THREAD,
	[HCC_FUNCTION_MANY_QUAD_ANY] = HCC_AML_OP_QUAD_ANY,
	[HCC_FUNCTION_MANY_QUAD_ALL] = HCC_AML_OP_QUAD_ALL,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ANY] = HCC_AML_OP_WAVE_ACTIVE_ANY,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ALL] = HCC_AML_OP_WAVE_ACTIVE_ALL,
	[HCC_FUNCTION_MANY_WAVE_READ_THREAD] = HCC_AML_OP_WAVE_READ_THREAD,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_ALL_EQUAL] = HCC_AML_OP_WAVE_ACTIVE_ALL_EQUAL,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_MIN] = HCC_AML_OP_WAVE_ACTIVE_MIN,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_MAX] = HCC_AML_OP_WAVE_ACTIVE_MAX,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_SUM] = HCC_AML_OP_WAVE_ACTIVE_SUM,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_SUM] = HCC_AML_OP_WAVE_ACTIVE_PREFIX_SUM,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PRODUCT] = HCC_AML_OP_WAVE_ACTIVE_PRODUCT,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_PRODUCT] = HCC_AML_OP_WAVE_ACTIVE_PREFIX_PRODUCT,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_COUNT_BITS] = HCC_AML_OP_WAVE_ACTIVE_COUNT_BITS,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_PREFIX_COUNT_BITS] = HCC_AML_OP_WAVE_ACTIVE_PREFIX_COUNT_BITS,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_AND] = HCC_AML_OP_WAVE_ACTIVE_BIT_AND,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_OR] = HCC_AML_OP_WAVE_ACTIVE_BIT_OR,
	[HCC_FUNCTION_MANY_WAVE_ACTIVE_BIT_XOR] = HCC_AML_OP_WAVE_ACTIVE_BIT_XOR,
	[HCC_FUNCTION_MANY_ATOMIC_LOAD] = HCC_AML_OP_ATOMIC_LOAD,
	[HCC_FUNCTION_MANY_ATOMIC_STORE] = HCC_AML_OP_ATOMIC_STORE,
	[HCC_FUNCTION_MANY_ATOMIC_EXCHANGE] = HCC_AML_OP_ATOMIC_EXCHANGE,
	[HCC_FUNCTION_MANY_ATOMIC_COMPARE_EXCHANGE] = HCC_AML_OP_ATOMIC_COMPARE_EXCHANGE,
	[HCC_FUNCTION_MANY_ATOMIC_ADD] = HCC_AML_OP_ATOMIC_ADD,
	[HCC_FUNCTION_MANY_ATOMIC_SUB] = HCC_AML_OP_ATOMIC_SUB,
	[HCC_FUNCTION_MANY_ATOMIC_MIN] = HCC_AML_OP_ATOMIC_MIN,
	[HCC_FUNCTION_MANY_ATOMIC_MAX] = HCC_AML_OP_ATOMIC_MAX,
	[HCC_FUNCTION_MANY_ATOMIC_BIT_AND] = HCC_AML_OP_ATOMIC_BIT_AND,
	[HCC_FUNCTION_MANY_ATOMIC_BIT_OR] = HCC_AML_OP_ATOMIC_BIT_OR,
	[HCC_FUNCTION_MANY_ATOMIC_BIT_XOR] = HCC_AML_OP_ATOMIC_BIT_XOR,
	[HCC_FUNCTION_MANY_LOAD_TEXTURE] = HCC_AML_OP_LOAD_TEXTURE,
	[HCC_FUNCTION_MANY_FETCH_TEXTURE] = HCC_AML_OP_FETCH_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_TEXTURE] = HCC_AML_OP_SAMPLE_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_MIP_BIAS_TEXTURE] = HCC_AML_OP_SAMPLE_MIP_BIAS_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_MIP_GRADIENT_TEXTURE] = HCC_AML_OP_SAMPLE_MIP_GRADIENT_TEXTURE,
	[HCC_FUNCTION_MANY_SAMPLE_MIP_LEVEL_TEXTURE] = HCC_AML_OP_SAMPLE_MIP_LEVEL_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_RED_TEXTURE] = HCC_AML_OP_GATHER_RED_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_GREEN_TEXTURE] = HCC_AML_OP_GATHER_GREEN_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_BLUE_TEXTURE] = HCC_AML_OP_GATHER_BLUE_TEXTURE,
	[HCC_FUNCTION_MANY_GATHER_ALPHA_TEXTURE] = HCC_AML_OP_GATHER_ALPHA_TEXTURE,
	[HCC_FUNCTION_MANY_STORE_TEXTURE] = HCC_AML_OP_STORE_TEXTURE,
};

