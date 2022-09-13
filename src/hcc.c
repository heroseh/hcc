#include "hcc.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>

// ===========================================
//
//
// General
//
//
// ===========================================

static HccCompiler* _hcc_compiler_ptr;

const char* hcc_alloc_tag_strings[HCC_ALLOC_TAG_COUNT] = {
	[HCC_ALLOC_TAG_MESSAGES] = "HCC_ALLOC_TAG_MESSAGES",
	[HCC_ALLOC_TAG_MESSAGE_STRINGS] = "HCC_ALLOC_TAG_MESSAGE_STRINGS",
	[HCC_ALLOC_TAG_STRING_TABLE_DATA] = "HCC_ALLOC_TAG_STRING_TABLE_DATA",
	[HCC_ALLOC_TAG_STRING_TABLE_ENTRIES] = "HCC_ALLOC_TAG_STRING_TABLE_ENTRIES",
	[HCC_ALLOC_TAG_CONSTANT_TABLE_DATA] = "HCC_ALLOC_TAG_CONSTANT_TABLE_DATA",
	[HCC_ALLOC_TAG_CONSTANT_TABLE_ENTRIES] = "HCC_ALLOC_TAG_CONSTANT_TABLE_ENTRIES",
	[HCC_ALLOC_TAG_STRING_BUFFER] = "HCC_ALLOC_TAG_STRING_BUFFER",
	[HCC_ALLOC_TAG_INCLUDE_PATHS] = "HCC_ALLOC_TAG_INCLUDE_PATHS",
	[HCC_ALLOC_TAG_CODE_FILES] = "HCC_ALLOC_TAG_CODE_FILES",
	[HCC_ALLOC_TAG_PATH_TO_CODE_FILE_ID_MAP] = "HCC_ALLOC_TAG_PATH_TO_CODE_FILE_ID_MAP",
	[HCC_ALLOC_TAG_CODE_FILE_LINE_CODE_START_INDICES] = "HCC_ALLOC_TAG_CODE_FILE_LINE_CODE_START_INDICES",
	[HCC_ALLOC_TAG_CODE_FILE_PP_IF_SPANS] = "HCC_ALLOC_TAG_CODE_FILE_PP_IF_SPANS",
	[HCC_ALLOC_TAG_CODE] = "HCC_ALLOC_TAG_CODE",
	[HCC_ALLOC_TAG_PP_TOKENS] = "HCC_ALLOC_TAG_PP_TOKENS",
	[HCC_ALLOC_TAG_PP_TOKEN_LOCATION_INDICES] = "HCC_ALLOC_TAG_PP_TOKEN_LOCATION_INDICES",
	[HCC_ALLOC_TAG_PP_TOKEN_VALUES] = "HCC_ALLOC_TAG_PP_TOKEN_VALUES",
	[HCC_ALLOC_TAG_PP_MACROS] = "HCC_ALLOC_TAG_PP_MACROS",
	[HCC_ALLOC_TAG_PP_MACRO_PARAMS] = "HCC_ALLOC_TAG_PP_MACRO_PARAMS",
	[HCC_ALLOC_TAG_PP_MACRO_ARGS_STACK] = "HCC_ALLOC_TAG_PP_MACRO_ARGS_STACK",
	[HCC_ALLOC_TAG_PP_EXPAND_STACK] = "HCC_ALLOC_TAG_PP_EXPAND_STACK",
	[HCC_ALLOC_TAG_PP_STRINGIFY_BUFFER] = "HCC_ALLOC_TAG_PP_STRINGIFY_BUFFER",
	[HCC_ALLOC_TAG_PP_IF_SPAN_STACK] = "HCC_ALLOC_TAG_PP_IF_SPAN_STACK",
	[HCC_ALLOC_TAG_PP_MACRO_DECLARATIONS] = "HCC_ALLOC_TAG_PP_MACRO_DECLARATIONS",
	[HCC_ALLOC_TAG_TOKENGEN_TOKENS] = "HCC_ALLOC_TAG_TOKENGEN_TOKENS",
	[HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATION_INDICES] = "HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATION_INDICES",
	[HCC_ALLOC_TAG_TOKENGEN_TOKEN_VALUES] = "HCC_ALLOC_TAG_TOKENGEN_TOKEN_VALUES",
	[HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATIONS] = "HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATIONS",
	[HCC_ALLOC_TAG_TOKENGEN_PAUSED_FILE_STACK] = "HCC_ALLOC_TAG_TOKENGEN_PAUSED_FILE_STACK",
	[HCC_ALLOC_TAG_TOKENGEN_OPEN_BRACKET_STACK] = "HCC_ALLOC_TAG_TOKENGEN_OPEN_BRACKET_STACK",
	[HCC_ALLOC_TAG_ASTGEN_FUNCTION_PARAMS_AND_VARIABLES] = "HCC_ALLOC_TAG_ASTGEN_FUNCTION_PARAMS_AND_VARIABLES",
	[HCC_ALLOC_TAG_ASTGEN_FUNCTIONS] = "HCC_ALLOC_TAG_ASTGEN_FUNCTIONS",
	[HCC_ALLOC_TAG_ASTGEN_EXPRS] = "HCC_ALLOC_TAG_ASTGEN_EXPRS",
	[HCC_ALLOC_TAG_ASTGEN_EXPR_LOCATIONS] = "HCC_ALLOC_TAG_ASTGEN_EXPR_LOCATIONS",
	[HCC_ALLOC_TAG_ASTGEN_GLOBAL_VARIABLES] = "HCC_ALLOC_TAG_ASTGEN_GLOBAL_VARIABLES",
	[HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_FUNCTION_INDICES] = "HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_FUNCTION_INDICES",
	[HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_STATIC_VARIABLES] = "HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_STATIC_VARIABLES",
	[HCC_ALLOC_TAG_ASTGEN_ENTRY_POINT_FUNCTION_INDICES] = "HCC_ALLOC_TAG_ASTGEN_ENTRY_POINT_FUNCTION_INDICES",
	[HCC_ALLOC_TAG_ASTGEN_USED_FUNCTION_INDICES] = "HCC_ALLOC_TAG_ASTGEN_USED_FUNCTION_INDICES",
	[HCC_ALLOC_TAG_ASTGEN_RECURSIVE_FUNCTION_INDICES_STACK] = "HCC_ALLOC_TAG_ASTGEN_RECURSIVE_FUNCTION_INDICES_STACK",
	[HCC_ALLOC_TAG_ASTGEN_ARRAY_DATA_TYPES] = "HCC_ALLOC_TAG_ASTGEN_ARRAY_DATA_TYPES",
	[HCC_ALLOC_TAG_ASTGEN_COMPOUND_DATA_TYPES] = "HCC_ALLOC_TAG_ASTGEN_COMPOUND_DATA_TYPES",
	[HCC_ALLOC_TAG_ASTGEN_COMPOUND_FIELDS] = "HCC_ALLOC_TAG_ASTGEN_COMPOUND_FIELDS",
	[HCC_ALLOC_TAG_ASTGEN_TYPEDEFS] = "HCC_ALLOC_TAG_ASTGEN_TYPEDEFS",
	[HCC_ALLOC_TAG_ASTGEN_ENUM_DATA_TYPES] = "HCC_ALLOC_TAG_ASTGEN_ENUM_DATA_TYPES",
	[HCC_ALLOC_TAG_ASTGEN_ENUM_VALUES] = "HCC_ALLOC_TAG_ASTGEN_ENUM_VALUES",
	[HCC_ALLOC_TAG_ASTGEN_ORDERED_DATA_TYPES] = "HCC_ALLOC_TAG_ASTGEN_ORDERED_DATA_TYPES",
	[HCC_ALLOC_TAG_ASTGEN_GLOBAL_DECLARATIONS] = "HCC_ALLOC_TAG_ASTGEN_GLOBAL_DECLARATIONS",
	[HCC_ALLOC_TAG_ASTGEN_STRUCT_DECLARATIONS] = "HCC_ALLOC_TAG_ASTGEN_STRUCT_DECLARATIONS",
	[HCC_ALLOC_TAG_ASTGEN_UNION_DECLARATIONS] = "HCC_ALLOC_TAG_ASTGEN_UNION_DECLARATIONS",
	[HCC_ALLOC_TAG_ASTGEN_ENUM_DECLARATIONS] = "HCC_ALLOC_TAG_ASTGEN_ENUM_DECLARATIONS",
	[HCC_ALLOC_TAG_ASTGEN_COMPOUND_TYPE_FIND_FIELDS] = "HCC_ALLOC_TAG_ASTGEN_COMPOUND_TYPE_FIND_FIELDS",
	[HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_CURLYS] = "HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_CURLYS",
	[HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_ELMTS] = "HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_ELMTS",
	[HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZERS] = "HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZERS",
	[HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZER_ELMT_INDICES] = "HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZER_ELMT_INDICES",
	[HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_STRINGS] = "HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_STRINGS",
	[HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_VAR_INDICES] = "HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_VAR_INDICES",
	[HCC_ALLOC_TAG_ASTGEN_FIELD_NAME_TO_TOKEN_IDX] = "HCC_ALLOC_TAG_ASTGEN_FIELD_NAME_TO_TOKEN_IDX",
	[HCC_ALLOC_TAG_IRGEN_FUNCTIONS] = "HCC_ALLOC_TAG_IRGEN_FUNCTIONS",
	[HCC_ALLOC_TAG_IRGEN_BASIC_BLOCKS] = "HCC_ALLOC_TAG_IRGEN_BASIC_BLOCKS",
	[HCC_ALLOC_TAG_IRGEN_VALUES] = "HCC_ALLOC_TAG_IRGEN_VALUES",
	[HCC_ALLOC_TAG_IRGEN_INSTRUCTIONS] = "HCC_ALLOC_TAG_IRGEN_INSTRUCTIONS",
	[HCC_ALLOC_TAG_IRGEN_OPERANDS] = "HCC_ALLOC_TAG_IRGEN_OPERANDS",
	[HCC_ALLOC_TAG_IRGEN_FUNCTION_CALL_PARAM_DATA_TYPES] = "HCC_ALLOC_TAG_IRGEN_FUNCTION_CALL_PARAM_DATA_TYPES",
	[HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_DATA_TYPES] = "HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_DATA_TYPES",
	[HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_ENTRIES] = "HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_ENTRIES",
	[HCC_ALLOC_TAG_SPIRVGEN_OUT_CAPABILITIES] = "HCC_ALLOC_TAG_SPIRVGEN_OUT_CAPABILITIES",
	[HCC_ALLOC_TAG_SPIRVGEN_OUT_ENTRY_POINTS] = "HCC_ALLOC_TAG_SPIRVGEN_OUT_ENTRY_POINTS",
	[HCC_ALLOC_TAG_SPIRVGEN_OUT_DEBUG_INFO] = "HCC_ALLOC_TAG_SPIRVGEN_OUT_DEBUG_INFO",
	[HCC_ALLOC_TAG_SPIRVGEN_OUT_ANNOTATIONS] = "HCC_ALLOC_TAG_SPIRVGEN_OUT_ANNOTATIONS",
	[HCC_ALLOC_TAG_SPIRVGEN_OUT_TYPES_VARIABLES_CONSTANTS] = "HCC_ALLOC_TAG_SPIRVGEN_OUT_TYPES_VARIABLES_CONSTANTS",
	[HCC_ALLOC_TAG_SPIRVGEN_OUT_FUNCTIONS] = "HCC_ALLOC_TAG_SPIRVGEN_OUT_FUNCTIONS",
};

//
// the padded zero bytes at the end of the file we add so we can look ahead in the tokenizer comparisions
#define _HCC_TOKENIZER_LOOK_HEAD_SIZE 4

#define HCC_DEBUG_CODE_SPAN_PUSH_POP 0
#define HCC_DEBUG_CODE_PREPROCESSOR 0
#define HCC_DEBUG_CODE_IF_SPAN 0

void _hcc_assert_failed(const char* cond, const char* file, int line, const char* message, ...) {
	fprintf(stderr, "assertion failed: %s\nmessage: ", cond);

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

noreturn Uptr _hcc_abort(const char* file, int line, const char* message, ...) {
	fprintf(stderr, "abort: ");

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

// ===========================================
//
//
// Platform Abstraction
//
//
// ===========================================

U32 onebitscount32(U32 bits) {
#ifdef __GNUC__
	return __builtin_popcount(bits);
#else
#error "unsupported fartcount"
#endif
}

U32 leastsetbitidx32(U32 bits) {
#ifdef __GNUC__
	return __builtin_ctz(bits);
#else
#error "unsupported fartcount"
#endif
}

void hcc_get_last_system_error_string(char* buf_out, U32 buf_out_size) {
#if _WIN32
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

		U32 size = strsize(buf);
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

bool hcc_file_exist(char* path) {
#ifdef __linux__
	return access(path, F_OK) == 0;
#else
#error "unimplemented for this platform"
#endif
}

bool hcc_change_working_directory(char* path) {
#ifdef __linux__
	return chdir(path) == 0;
#else
#error "unimplemented for this platform"
#endif
}

bool hcc_change_working_directory_to_same_as_this_file(char* path) {
	char* end = strrchr(path, '/');
	char path_buf[1024];
	if (end == NULL) {
		path_buf[0] = '/';
		path_buf[1] = '\0';
	} else {
		U32 size = end - path + 1;
		HCC_DEBUG_ASSERT_ARRAY_BOUNDS(size, sizeof(path_buf));
		memcpy(path_buf, path, size);
		path_buf[size] = '\0';
	}

	if (!hcc_change_working_directory(path_buf)) {
		char error_buf[512];
		hcc_get_last_system_error_string(error_buf, sizeof(error_buf));
		HCC_ABORT("internal error: failed to change the working directory to '%s': %s", path_buf, error_buf);
	}

	return true;
}

char* hcc_path_canonicalize(char* path) {
#ifdef __linux__
	char* new_path = malloc(PATH_MAX);
	if (realpath(path, new_path) == NULL) {
		char error_buf[512];
		hcc_get_last_system_error_string(error_buf, sizeof(error_buf));
		HCC_ABORT("internal error: failed to locate file at '%s': %s", path, error_buf);
	}
	return new_path;
#else
#error "unimplemented for this platform"
#endif
}

bool hcc_path_is_absolute(char* path) {
#ifdef __unix__
	return path[0] == '/';
#else
#error "unimplemented for this platform"
#endif
}

char* hcc_file_read_all_the_codes(char* path, U64* size_out) {
#ifdef __linux__
	int fd_flags = O_CLOEXEC | O_RDONLY;
	int mode = 0666;
	int fd = open(path, fd_flags, mode);
	if (fd == -1) {
		return NULL;
	}

	struct stat s = {0};
	if (fstat(fd, &s) != 0) return NULL;
	*size_out = s.st_size;

	char* bytes = HCC_ALLOC(HCC_ALLOC_TAG_CODE, s.st_size + _HCC_TOKENIZER_LOOK_HEAD_SIZE, alignof(char));

	U64 remaining_read_size = s.st_size;
	U64 offset = 0;
	while (remaining_read_size) {
		ssize_t bytes_read = read(fd, bytes + offset, remaining_read_size);
		if (bytes_read == (ssize_t)-1) {
			return NULL;
		}
		offset += bytes_read;
		remaining_read_size -= bytes_read;
	}

	close(fd);

	return bytes;
#else
#error "unimplemented for this platform"
#endif
}

// ===========================================
//
//
// Stack
//
//
// ===========================================

#define HCC_DEBUG_ASSERT_STACK(header, elmt_size_) \
	HCC_DEBUG_ASSERT(header->magic_number == HCC_STACK_MAGIC_NUMBER, "address '%p' is not a stack", header + 1); \
	HCC_DEBUG_ASSERT(header->elmt_size == elmt_size_, "stack element size mismatch. expected '%zu' but got '%zu'", header->elmt_size == elmt_size_)

HccStack(void) _hcc_stack_init(Uptr cap, HccAllocTag tag, Uptr elmt_size) {
	HCC_DEBUG_ASSERT_NON_ZERO(cap);
	Uptr size = sizeof(HccStackHeader) + cap * elmt_size;
	HccStackHeader* header = HCC_ALLOC(tag, size, alignof(void*));
	if (!header) {
		hcc_compiler_bail_allocation_failure(_hcc_compiler_ptr, tag);
	}

	//
	// initialize the header and pass out the where the elements of the array start
	header->count = 0;
	header->cap = cap;
	header->tag = tag;
#if HCC_DEBUG_ASSERTIONS
	header->magic_number = HCC_STACK_MAGIC_NUMBER;
	header->elmt_size = elmt_size;
#endif

	return header + 1;
}

void _hcc_stack_deinit(HccStack(void) stack, HccAllocTag tag, Uptr elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	Uptr size = sizeof(HccStackHeader) + header->cap * elmt_size;
	HCC_DEALLOC(tag, header, size, alignof(void*));
}

Uptr _hcc_stack_resize(HccStack(void) stack, Uptr new_count, Uptr elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	Uptr count = header->count;
	HCC_DEBUG_ASSERT_ARRAY_RESIZE(count, header->cap);
	header->count = new_count;
	return count;
}

void* _hcc_stack_insert_many(HccStack(void) stack, Uptr idx, Uptr amount, Uptr elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	HCC_DEBUG_ASSERT(idx <= header->count, "insert index '%zu' must be less than or equal to count of '%zu'", idx, header->count);

	//
	// extend the count and ensure we don't exceed the capacity
	Uptr new_count = header->count + amount;
	if (new_count > header->cap) {
		hcc_compiler_bail_collection_is_full(_hcc_compiler_ptr, header->tag);
	}
	Uptr count = _hcc_stack_resize(stack, new_count, elmt_size);

	//
	// shift the elements from idx to (idx + amount), to the right to make room for the elements
	void* dst = HCC_PTR_ADD(stack, idx * elmt_size);
	memmove(HCC_PTR_ADD(dst, amount * elmt_size), dst, (count - idx) * elmt_size);

	return dst;
}

Uptr _hcc_stack_push_many(HccStack(void) stack, Uptr amount, Uptr elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);

	//
	// extend the count and ensure we don't exceed the capacity
	Uptr new_count = header->count + amount;
	if (new_count > header->cap) {
		hcc_compiler_bail_collection_is_full(_hcc_compiler_ptr, header->tag);
	}

	Uptr insert_idx = _hcc_stack_resize(stack, new_count, elmt_size);
	return insert_idx;
}

void _hcc_stack_pop_many(HccStack(void) stack, Uptr amount, Uptr elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	HCC_DEBUG_ASSERT(amount <= header->count, "cannot pop '%zu' many elements when the array has a count of '%zu'", amount, header->count);
	header->count -= amount;
}

void _hcc_stack_remove_swap_many(HccStack(void) stack, Uptr idx, Uptr amount, Uptr elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT_STACK(header, elmt_size);
	HCC_DEBUG_ASSERT(idx <= header->count, "idx '%zu' must be less than or equal to count of '%zu'", idx, header->count);
	Uptr end_idx = idx + amount;
	HCC_DEBUG_ASSERT(end_idx <= header->count, "(idx + amount) '%zu' must be less than or equal to count of '%zu'", end_idx, header->count);

	//
	// get the pointer to the elements that are being removed
	void* dst = HCC_PTR_ADD(stack, idx * elmt_size);

	//
	// get the pointer to the elements at the back of the stack that will be moved
	Uptr src_idx = header->count;
	header->count -= amount;
	amount = HCC_MIN(amount, header->count);
	src_idx -= amount;

	//
	// replace the removed elements with the elements from the back of the stack
	void* src = HCC_PTR_ADD(stack, src_idx * elmt_size);
	memmove(dst, src, amount * elmt_size);
}

void _hcc_stack_remove_shift_many(HccStack(void) stack, Uptr idx, Uptr amount, Uptr elmt_size) {
	HccStackHeader* header = hcc_stack_header(stack);
	HCC_DEBUG_ASSERT(idx <= header->count, "idx '%zu' must be less than or equal to count of '%zu'", idx, header->count);
	Uptr end_idx = idx + amount;
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

void hcc_stack_push_char(HccStack(char) stk, char ch) {
	*hcc_stack_push(stk) = ch;
}

HccString hcc_stack_push_string(HccStack(char) stk, HccString string) {
	char* ptr = hcc_stack_push_many(stk, string.size);
	memcpy(ptr, string.data, string.size);
	return hcc_string(ptr, string.size);
}

HccString hcc_stack_push_string_fmtv(HccStack(char) stack, char* fmt, va_list args) {
	va_list args_copy;
	va_copy(args_copy, args);

	// add 1 so we have enough room for the null terminator that vsnprintf always outputs
	// vsnprintf will return -1 on an encoding error.
	Uptr size = vsnprintf(NULL, 0, fmt, args_copy) + 1;
	va_end(args_copy);
	HCC_DEBUG_ASSERT(size >= 1, "a vsnprintf encoding error has occurred");

	//
	// resize the stack to have enough room to store the pushed formatted string
	Uptr new_count = hcc_stack_count(stack) + size;
	Uptr insert_idx = hcc_stack_resize(stack, new_count);

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
// Hashing
//
//
// ===========================================

U32 hcc_fnv_hash_32(char* bytes, U32 byte_count, U32 hash) {
	char* bytes_end = bytes + byte_count;
	while (bytes < bytes_end) {
		hash = hash ^ *bytes;
		hash = hash * 0x01000193;
		bytes += 1;
	}
	return hash;
}

void hcc_generate_enum_hashes(char* array_name, char** strings, char** enum_strings, U32 enums_count) {
	U32 used_hashes[128];
	HCC_ASSERT(enums_count <= HCC_ARRAY_COUNT(used_hashes), "internal error: used_hashes needs to be atleast %u", enums_count);

	for (U32 idx = 0; idx < enums_count; idx += 1) {
		char* string = strings[idx];
		U32 string_size = strlen(string);
		U32 hash = hcc_fnv_hash_32(string, string_size, HCC_FNV_HASH_32_INITIAL);
		for (U32 used_idx = 0; used_idx < idx; used_idx += 1) {
			HCC_ASSERT(used_hashes[used_idx] != hash, "internal error: '%s' and '%s' have the same hash of '0x%x'", strings[idx], strings[used_idx]);
		}
		used_hashes[idx] = hash;
	}

	printf("U32 %s[] = {\n", array_name);
	for (U32 idx = 0; idx < enums_count; idx += 1) {
		char* enum_string = enum_strings[idx];
		U32 hash = used_hashes[idx];
		printf("\t[%s] = 0x%x,\n", enum_string, hash);
	}
	printf("};\n");
}

void hcc_generate_hashes() {
	hcc_generate_enum_hashes("hcc_pp_directive_hashes", hcc_pp_directive_strings, hcc_pp_directive_enum_strings, HCC_PP_DIRECTIVE_COUNT);
	exit(0);
}

U32 hcc_string_to_enum_hashed_find(HccString string, U32* enum_hashes, U32 enums_count) {
	U32 hash = hcc_fnv_hash_32(string.data, string.size, HCC_FNV_HASH_32_INITIAL);
	for (U32 enum_ = 0; enum_ < enums_count; enum_ += 1) {
		if (enum_hashes[enum_] == hash) {
			return enum_;
		}
	}
	return enums_count;
}


// ===========================================
//
//
// Hash Table
//
//
// ===========================================

void hcc_hash_table_init(HccHashTable* hash_table, U32 cap, HccAllocTag tag) {
	hash_table->keys = HCC_ALLOC_ARRAY(U32, tag, cap);
	if (hash_table->keys == NULL) {
		hcc_compiler_bail_allocation_failure(_hcc_compiler_ptr, tag);
	}
	HCC_ZERO_ELMT_MANY(hash_table->keys, cap);

	hash_table->values = HCC_ALLOC_ARRAY(U32, tag, cap);
	if (hash_table->values == NULL) {
		hcc_compiler_bail_allocation_failure(_hcc_compiler_ptr, tag);
	}

	hash_table->count = 0;
	hash_table->cap = cap;
}

bool hcc_hash_table_find(HccHashTable* hash_table, U32 key, U32* value_out) {
	for (uint32_t idx = 0; idx < hash_table->count; idx += 1) {
		U32 found_key = hash_table->keys[idx];
		if (found_key == key) {
			if (value_out) *value_out = hash_table->values[idx];
			return true;
		}

	}
	return false;
}

bool hcc_hash_table_find_or_insert(HccHashTable* hash_table, U32 key, U32** value_ptr_out) {
	for (uint32_t idx = 0; idx < hash_table->count; idx += 1) {
		U32 found_key = hash_table->keys[idx];
		if (found_key == key) {
			*value_ptr_out = &hash_table->values[idx];
			return true;
		}
	}

	hash_table->keys[hash_table->count] = key;
	*value_ptr_out = &hash_table->values[hash_table->count];
	HCC_ASSERT(hash_table->count < hash_table->cap, "hash table full");
	hash_table->count += 1;
	return false;
}

bool hcc_hash_table_remove(HccHashTable* hash_table, U32 key, U32* value_out) {
	for (uint32_t idx = 0; idx < hash_table->count; idx += 1) {
		U32 found_key = hash_table->keys[idx];
		if (found_key == key) {
			if (value_out) {
				*value_out = hash_table->values[idx];
			}
			if (idx + 1 < hash_table->count) {
				memmove(&hash_table->keys[idx], &hash_table->keys[idx + 1], (hash_table->cap - idx - 1) * sizeof(U32));
				memmove(&hash_table->values[idx], &hash_table->values[idx + 1], (hash_table->cap - idx - 1) * sizeof(U32));
			}
			hash_table->count -= 1;
			return true;
		}

	}
	return false;
}

void hcc_hash_table_clear(HccHashTable* hash_table) {
	hash_table->count = 0;
}

// ===========================================
//
//
// Token
//
//
// ===========================================

char* hcc_token_strings[HCC_TOKEN_COUNT] = {
	[HCC_TOKEN_EOF] = "end of file",
	[HCC_TOKEN_IDENT] = "identifier",
	[HCC_TOKEN_STRING] = "string",
	[HCC_TOKEN_INCLUDE_PATH_SYSTEM] = "<>",
	[HCC_TOKEN_BACK_SLASH] = "\\",
	[HCC_TOKEN_HASH] = "#",
	[HCC_TOKEN_DOUBLE_HASH] = "##",
	[HCC_TOKEN_MACRO_WHITESPACE] = " ",
	[HCC_TOKEN_MACRO_PARAM] = "macro param",
	[HCC_TOKEN_MACRO_STRINGIFY] = "macro stringify",
	[HCC_TOKEN_MACRO_STRINGIFY_WHITESPACE] = "macro stringify whitespace",
	[HCC_TOKEN_MACRO_CONCAT] = "##",
	[HCC_TOKEN_MACRO_CONCAT_WHITESPACE] = " ##",
	[HCC_TOKEN_CURLY_OPEN] = "{",
	[HCC_TOKEN_CURLY_CLOSE] = "}",
	[HCC_TOKEN_PARENTHESIS_OPEN] = "(",
	[HCC_TOKEN_PARENTHESIS_CLOSE] = ")",
	[HCC_TOKEN_SQUARE_OPEN] = "[",
	[HCC_TOKEN_SQUARE_CLOSE] = "]",
	[HCC_TOKEN_FULL_STOP] = ".",
	[HCC_TOKEN_COMMA] = ",",
	[HCC_TOKEN_SEMICOLON] = ";",
	[HCC_TOKEN_COLON] = ":",
	[HCC_TOKEN_PLUS] = "+",
	[HCC_TOKEN_MINUS] = "-",
	[HCC_TOKEN_FORWARD_SLASH] = "/",
	[HCC_TOKEN_ASTERISK] = "*",
	[HCC_TOKEN_PERCENT] = "%",
	[HCC_TOKEN_AMPERSAND] = "&",
	[HCC_TOKEN_PIPE] = "|",
	[HCC_TOKEN_CARET] = "^",
	[HCC_TOKEN_EXCLAMATION_MARK] = "!",
	[HCC_TOKEN_QUESTION_MARK] = "?",
	[HCC_TOKEN_TILDE] = "~",
	[HCC_TOKEN_EQUAL] = "=",
	[HCC_TOKEN_LESS_THAN] = "<",
	[HCC_TOKEN_GREATER_THAN] = ">",
	[HCC_TOKEN_LOGICAL_AND] = "&&",
	[HCC_TOKEN_LOGICAL_OR] = "||",
	[HCC_TOKEN_LOGICAL_EQUAL] = "==",
	[HCC_TOKEN_LOGICAL_NOT_EQUAL] = "!=",
	[HCC_TOKEN_LESS_THAN_OR_EQUAL] = "<=",
	[HCC_TOKEN_GREATER_THAN_OR_EQUAL] = ">=",
	[HCC_TOKEN_BIT_SHIFT_LEFT] = "<<",
	[HCC_TOKEN_BIT_SHIFT_RIGHT] = ">>",
	[HCC_TOKEN_ADD_ASSIGN] = "+=",
	[HCC_TOKEN_SUBTRACT_ASSIGN] = "-=",
	[HCC_TOKEN_MULTIPLY_ASSIGN] = "*=",
	[HCC_TOKEN_DIVIDE_ASSIGN] = "/=",
	[HCC_TOKEN_MODULO_ASSIGN] = "%=",
	[HCC_TOKEN_BIT_SHIFT_LEFT_ASSIGN] = "<<=",
	[HCC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN] = ">>=",
	[HCC_TOKEN_BIT_AND_ASSIGN] = "&=",
	[HCC_TOKEN_BIT_XOR_ASSIGN] = "^=",
	[HCC_TOKEN_BIT_OR_ASSIGN] = "|=",
	[HCC_TOKEN_INCREMENT] = "++",
	[HCC_TOKEN_DECREMENT] = "--",
	[HCC_TOKEN_LIT_UINT] = "unsigned int",
	[HCC_TOKEN_LIT_ULONG] = "unsigned long",
	[HCC_TOKEN_LIT_ULONGLONG] = "unsigned long long",
	[HCC_TOKEN_LIT_SINT] = "int",
	[HCC_TOKEN_LIT_SLONG] = "long",
	[HCC_TOKEN_LIT_SLONGLONG] = "long long",
	[HCC_TOKEN_LIT_FLOAT] = "float",
	[HCC_TOKEN_LIT_DOUBLE] = "double",
	[HCC_TOKEN_KEYWORD_VOID] = "void",
	[HCC_TOKEN_KEYWORD_BOOL] = "_Bool",
	[HCC_TOKEN_KEYWORD_CHAR] = "char",
	[HCC_TOKEN_KEYWORD_SHORT] = "short",
	[HCC_TOKEN_KEYWORD_INT] = "int",
	[HCC_TOKEN_KEYWORD_LONG] = "long",
	[HCC_TOKEN_KEYWORD_FLOAT] = "float",
	[HCC_TOKEN_KEYWORD_DOUBLE] = "double",
	[HCC_TOKEN_KEYWORD_GENERIC_FLOAT] = "__hcc_generic_float",
	[HCC_TOKEN_KEYWORD_UNSIGNED] = "unsigned",
	[HCC_TOKEN_KEYWORD_SIGNED] = "signed",
	[HCC_TOKEN_KEYWORD_COMPLEX] = "_Complex",
	[HCC_TOKEN_KEYWORD_ATOMIC] = "_Atomic",
	[HCC_TOKEN_KEYWORD_RETURN] = "return",
	[HCC_TOKEN_KEYWORD_IF] = "if",
	[HCC_TOKEN_KEYWORD_ELSE] = "else",
	[HCC_TOKEN_KEYWORD_DO] = "do",
	[HCC_TOKEN_KEYWORD_WHILE] = "while",
	[HCC_TOKEN_KEYWORD_FOR] = "for",
	[HCC_TOKEN_KEYWORD_SWITCH] = "switch",
	[HCC_TOKEN_KEYWORD_CASE] = "case",
	[HCC_TOKEN_KEYWORD_DEFAULT] = "default",
	[HCC_TOKEN_KEYWORD_BREAK] = "break",
	[HCC_TOKEN_KEYWORD_CONTINUE] = "continue",
	[HCC_TOKEN_KEYWORD_TRUE] = "true",
	[HCC_TOKEN_KEYWORD_FALSE] = "false",
	[HCC_TOKEN_KEYWORD_VERTEX] = "vertex",
	[HCC_TOKEN_KEYWORD_FRAGMENT] = "fragment",
	[HCC_TOKEN_KEYWORD_GEOMETRY] = "geometry",
	[HCC_TOKEN_KEYWORD_TESSELLATION] = "tessellation",
	[HCC_TOKEN_KEYWORD_COMPUTE] = "compute",
	[HCC_TOKEN_KEYWORD_MESHTASK] = "meshtask",
	[HCC_TOKEN_KEYWORD_ENUM] = "enum",
	[HCC_TOKEN_KEYWORD_STRUCT] = "struct",
	[HCC_TOKEN_KEYWORD_UNION] = "union",
	[HCC_TOKEN_KEYWORD_TYPEDEF] = "typedef",
	[HCC_TOKEN_KEYWORD_STATIC] = "static",
	[HCC_TOKEN_KEYWORD_CONST] = "const",
	[HCC_TOKEN_KEYWORD_AUTO] = "auto",
	[HCC_TOKEN_KEYWORD_REGISTER] = "register",
	[HCC_TOKEN_KEYWORD_VOLATILE] = "volatile",
	[HCC_TOKEN_KEYWORD_EXTERN] = "extern",
	[HCC_TOKEN_KEYWORD_INLINE] = "inline",
	[HCC_TOKEN_KEYWORD_NO_RETURN] = "_Noreturn",
	[HCC_TOKEN_KEYWORD_SIZEOF] = "sizeof",
	[HCC_TOKEN_KEYWORD_ALIGNOF] = "_Alignof",
	[HCC_TOKEN_KEYWORD_ALIGNAS] = "_Alignas",
	[HCC_TOKEN_KEYWORD_STATIC_ASSERT] = "_Static_assert",
	[HCC_TOKEN_KEYWORD_RESTRICT] = "restrict",
	[HCC_TOKEN_KEYWORD_INTRINSIC] = "__hcc_intrinsic",
	[HCC_TOKEN_KEYWORD_RASTERIZER_STATE] = "__hcc_rasterizer_state",
	[HCC_TOKEN_KEYWORD_FRAGMENT_STATE] = "__hcc_fragment_state",
	[HCC_TOKEN_KEYWORD_POSITION] = "__hcc_position",
	[HCC_TOKEN_KEYWORD_NOINTERP] = "__hcc_nointerp",
	[HCC_TOKEN_KEYWORD_RO_BUFFER] = "ro_buffer",
	[HCC_TOKEN_KEYWORD_RW_BUFFER] = "rw_buffer",
	[HCC_TOKEN_KEYWORD_RO_IMAGE1D] = "ro_image1d",
	[HCC_TOKEN_KEYWORD_RW_IMAGE1D] = "rw_image1d",
	[HCC_TOKEN_KEYWORD_RO_IMAGE2D] = "ro_image2d",
	[HCC_TOKEN_KEYWORD_RW_IMAGE2D] = "rw_image2d",
	[HCC_TOKEN_KEYWORD_RO_IMAGE3D] = "ro_image3d",
	[HCC_TOKEN_KEYWORD_RW_IMAGE3D] = "rw_image3d",
};

U32 hcc_token_num_values(HccToken token) {
	switch (token) {
		case HCC_TOKEN_IDENT:
		case HCC_TOKEN_STRING:
		case HCC_TOKEN_INCLUDE_PATH_SYSTEM:
		case HCC_TOKEN_MACRO_PARAM:
		case HCC_TOKEN_MACRO_STRINGIFY:
		case HCC_TOKEN_MACRO_STRINGIFY_WHITESPACE:
			return 1;
		case HCC_TOKEN_LIT_UINT:
		case HCC_TOKEN_LIT_ULONG:
		case HCC_TOKEN_LIT_ULONGLONG:
		case HCC_TOKEN_LIT_SINT:
		case HCC_TOKEN_LIT_SLONG:
		case HCC_TOKEN_LIT_SLONGLONG:
		case HCC_TOKEN_LIT_FLOAT:
		case HCC_TOKEN_LIT_DOUBLE:
			return 2;
		default:
			return 0;
	}
}

bool hcc_token_concat_is_okay(HccToken before, HccToken after) {
	return (
		(HCC_TOKEN_IS_LIT_NUMBER(before) &&
			(
				HCC_TOKEN_IS_LIT_NUMBER(after) ||
				after == HCC_TOKEN_IDENT
			)
		) ||
		(before == HCC_TOKEN_IDENT &&
			(
				after == HCC_TOKEN_IDENT ||
				HCC_TOKEN_IS_LIT_NUMBER(after)
			)
		) ||
		(before == HCC_TOKEN_EQUAL && after == HCC_TOKEN_EQUAL) ||
		(before == HCC_TOKEN_EXCLAMATION_MARK && after == HCC_TOKEN_EQUAL) ||
		(before == HCC_TOKEN_PLUS &&
			(
				after == HCC_TOKEN_PLUS ||
				after == HCC_TOKEN_EQUAL
			)
		) ||
		(before == HCC_TOKEN_MINUS &&
			(
				after == HCC_TOKEN_MINUS ||
				after == HCC_TOKEN_EQUAL ||
				after == HCC_TOKEN_GREATER_THAN
			)
		) ||
		(before == HCC_TOKEN_ASTERISK && after == HCC_TOKEN_EQUAL) ||
		(before == HCC_TOKEN_FORWARD_SLASH && after == HCC_TOKEN_EQUAL) ||
		(before == HCC_TOKEN_PERCENT && after == HCC_TOKEN_EQUAL) ||
		(before == HCC_TOKEN_AMPERSAND &&
			(
				after == HCC_TOKEN_AMPERSAND ||
				after == HCC_TOKEN_EQUAL
			)
		) ||
		(before == HCC_TOKEN_PIPE &&
			(
				after == HCC_TOKEN_PIPE ||
				after == HCC_TOKEN_EQUAL
			)
		) ||
		(before == HCC_TOKEN_CARET &&
			(
				after == HCC_TOKEN_CARET ||
				after == HCC_TOKEN_EQUAL
			)
		) ||
		(before == HCC_TOKEN_LESS_THAN &&
			(
				after == HCC_TOKEN_LESS_THAN ||
				after == HCC_TOKEN_LESS_THAN_OR_EQUAL ||
				after == HCC_TOKEN_EQUAL
			)
		) ||
		(before == HCC_TOKEN_GREATER_THAN &&
			(
				after == HCC_TOKEN_GREATER_THAN ||
				after == HCC_TOKEN_GREATER_THAN_OR_EQUAL ||
				after == HCC_TOKEN_EQUAL
			)
		) ||
		(before == HCC_TOKEN_EQUAL &&
			(
				after == HCC_TOKEN_GREATER_THAN ||
				after == HCC_TOKEN_GREATER_THAN_OR_EQUAL ||
				after == HCC_TOKEN_EQUAL
			)
		) ||
		(before == HCC_TOKEN_HASH && after == HCC_TOKEN_HASH)
	);
}

U32 hcc_token_cursor_tokens_count(HccTokenCursor* cursor) {
	return cursor->tokens_end_idx - cursor->tokens_start_idx;
}

HccLocation* hcc_token_bag_location_get(HccCompiler* c, HccTokenBag* bag, U32 token_idx) {
	U32 location_idx = *hcc_stack_get(bag->token_location_indices, token_idx) & ~HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG_MASK;
	return hcc_stack_get(c->tokengen.token_locations, location_idx);
}

void hcc_token_bag_stringify_single(HccCompiler* c, HccTokenBag* bag, HccTokenCursor* cursor, HccPPMacro* macro) {
	HccToken token = *hcc_stack_get(bag->tokens, cursor->token_idx);
	cursor->token_idx += 1;
	switch (token) {
		case HCC_TOKEN_IDENT: {
			HccStringId string_id = hcc_stack_get(bag->token_values, cursor->token_value_idx)->string_id;
			cursor->token_value_idx += 1;
			HccString string = hcc_string_table_get(&c->string_table, string_id);
			hcc_stack_push_string(c->pp.stringify_buffer, string);
			break;
		};

		//
		// these cases are only reachable for the debug printing code in hcc_tokengen_print
		case HCC_TOKEN_MACRO_PARAM:
		case HCC_TOKEN_MACRO_STRINGIFY:
		case HCC_TOKEN_MACRO_STRINGIFY_WHITESPACE: {
			U32 param_idx = hcc_stack_get(bag->token_values, cursor->token_value_idx)->macro_param_idx;
			HccStringId string_id = *hcc_stack_get(c->pp.macro_params, macro->params_start_idx + param_idx);
			cursor->token_value_idx += 1;
			if (token == HCC_TOKEN_MACRO_STRINGIFY) {
				hcc_stack_push_char(c->pp.stringify_buffer, '#');
			} else if (token == HCC_TOKEN_MACRO_STRINGIFY_WHITESPACE) {
				hcc_stack_push_string(c->pp.stringify_buffer, hcc_string_lit("# "));
			}
			HccString string = hcc_string_table_get(&c->string_table, string_id);
			hcc_stack_push_string(c->pp.stringify_buffer, string);
			break;
		};

		case HCC_TOKEN_LIT_UINT:
		case HCC_TOKEN_LIT_ULONG:
		case HCC_TOKEN_LIT_ULONGLONG:
		case HCC_TOKEN_LIT_SINT:
		case HCC_TOKEN_LIT_SLONG:
		case HCC_TOKEN_LIT_SLONGLONG:
		case HCC_TOKEN_LIT_FLOAT:
		case HCC_TOKEN_LIT_DOUBLE: {
			cursor->token_value_idx += 1;
			HccStringId string_id = bag->token_values[cursor->token_value_idx].string_id;
			cursor->token_value_idx += 1;

			HccString string = hcc_string_table_get(&c->string_table, string_id);
			hcc_stack_push_string(c->pp.stringify_buffer, string);
			break;
		};

		case HCC_TOKEN_STRING: {
			HccStringId string_id = bag->token_values[cursor->token_value_idx].string_id;
			cursor->token_value_idx += 1;

			HccString string = hcc_string_table_get(&c->string_table, string_id);

			hcc_stack_push_char(c->pp.stringify_buffer, '"');
			for (U32 idx = 0; idx < string.size; idx += 1) {
				char ch = string.data[idx];
				switch (ch) {
					//
					// TODO complete the escape codes
					case '\\':
						hcc_stack_push_char(c->pp.stringify_buffer, '\\');
						hcc_stack_push_char(c->pp.stringify_buffer, '\\');
						break;
					case '\r':
						hcc_stack_push_char(c->pp.stringify_buffer, '\\');
						hcc_stack_push_char(c->pp.stringify_buffer, '\r');
						break;
					case '\n':
						hcc_stack_push_char(c->pp.stringify_buffer, '\\');
						hcc_stack_push_char(c->pp.stringify_buffer, '\n');
						break;
					default:
						hcc_stack_push_char(c->pp.stringify_buffer, ch);
				}
			}
			hcc_stack_push_char(c->pp.stringify_buffer, '"');

			break;
		};
		case HCC_TOKEN_MACRO_CONCAT:
		case HCC_TOKEN_MACRO_CONCAT_WHITESPACE: {
			//
			// tokengen will reorder macro concat so comes before the two operands.
			// so `ident ## ifier` becomes `## ident ifier`
			// so when stringifing, put it back in the order it was in string form.
			hcc_token_bag_stringify_single(c, bag, cursor, macro);
			hcc_stack_push_string(c->pp.stringify_buffer, hcc_string_c(hcc_token_strings[token]));
			if (*hcc_stack_get(bag->tokens, cursor->token_idx) == HCC_TOKEN_MACRO_WHITESPACE) {
				hcc_stack_push_char(c->pp.stringify_buffer, ' ');
				cursor->token_idx += 1;
			}
			hcc_token_bag_stringify_single(c, bag, cursor, macro);
			break;
		};

		default:
			hcc_stack_push_string(c->pp.stringify_buffer, hcc_string_c(hcc_token_strings[token]));
			break;
	}
}

HccToken hcc_token_bag_stringify_single_or_macro_param(HccCompiler* c, HccTokenBag* bag, HccTokenCursor* cursor, U32 args_start_idx, HccTokenBag* args_src_bag, bool false_before_true_after) {
	if (cursor->token_idx >= cursor->tokens_end_idx) {
		return HCC_TOKEN_COUNT;
	}

	HccToken token = *hcc_stack_get(bag->tokens, cursor->token_idx);
	if (token == HCC_TOKEN_MACRO_PARAM) {
		cursor->token_idx += 1;
		U32 param_idx = hcc_stack_get(bag->token_values, cursor->token_value_idx)->macro_param_idx;
		cursor->token_value_idx += 1;
		HccPPMacroArg* arg = hcc_stack_get(c->pp.macro_args_stack, args_start_idx + param_idx);
		HccTokenCursor arg_cursor = arg->cursor;
		if (arg_cursor.tokens_start_idx == arg_cursor.tokens_end_idx) {
			return HCC_TOKEN_COUNT;
		}

		HccToken return_token = *hcc_stack_get(args_src_bag->tokens, false_before_true_after ? arg_cursor.tokens_start_idx : arg_cursor.tokens_end_idx - 1);
		hcc_token_bag_stringify_range(c, args_src_bag, &arg_cursor, NULL);
		return return_token;
	}

	hcc_token_bag_stringify_single(c, bag, cursor, NULL);
	return token;
}

HccStringId hcc_token_bag_stringify_range(HccCompiler* c, HccTokenBag* bag, HccTokenCursor* cursor, HccPPMacro* macro) {
	while (cursor->token_idx < cursor->tokens_end_idx) {
		hcc_token_bag_stringify_single(c, bag, cursor, macro);
	}

	return hcc_string_table_deduplicate(&c->string_table, c->pp.stringify_buffer, hcc_stack_count(c->pp.stringify_buffer));
}

// ===========================================
//
//
// Preprocessor
//
//
// ===========================================

char* hcc_pp_predefined_macro_identifier_strings[HCC_PP_PREDEFINED_MACRO_COUNT] = {
	[HCC_PP_PREDEFINED_MACRO___FILE__] = "__FILE__",
	[HCC_PP_PREDEFINED_MACRO___LINE__] = "__LINE__",
	[HCC_PP_PREDEFINED_MACRO___COUNTER__] = "__COUNTER__",
	[HCC_PP_PREDEFINED_MACRO___HCC__] = "__HCC__",
	[HCC_PP_PREDEFINED_MACRO___HCC_GPU__] = "__HCC_GPU__",
	[HCC_PP_PREDEFINED_MACRO___HCC_X86_64__] = "__HCC_X86_64__",
	[HCC_PP_PREDEFINED_MACRO___HCC_LINUX__] = "__HCC_LINUX__",
	[HCC_PP_PREDEFINED_MACRO___HCC_WINDOWS__] = "__HCC_WINDOWS__",
};

char* hcc_pp_directive_enum_strings[HCC_PP_DIRECTIVE_COUNT] = {
	[HCC_PP_DIRECTIVE_DEFINE] = "HCC_PP_DIRECTIVE_DEFINE",
	[HCC_PP_DIRECTIVE_UNDEF] = "HCC_PP_DIRECTIVE_UNDEF",
	[HCC_PP_DIRECTIVE_INCLUDE] = "HCC_PP_DIRECTIVE_INCLUDE",
	[HCC_PP_DIRECTIVE_IF] = "HCC_PP_DIRECTIVE_IF",
	[HCC_PP_DIRECTIVE_IFDEF] = "HCC_PP_DIRECTIVE_IFDEF",
	[HCC_PP_DIRECTIVE_IFNDEF] = "HCC_PP_DIRECTIVE_IFNDEF",
	[HCC_PP_DIRECTIVE_ELSE] = "HCC_PP_DIRECTIVE_ELSE",
	[HCC_PP_DIRECTIVE_ELIF] = "HCC_PP_DIRECTIVE_ELIF",
	[HCC_PP_DIRECTIVE_ELIFDEF] = "HCC_PP_DIRECTIVE_ELIFDEF",
	[HCC_PP_DIRECTIVE_ELIFNDEF] = "HCC_PP_DIRECTIVE_ELIFNDEF",
	[HCC_PP_DIRECTIVE_ENDIF] = "HCC_PP_DIRECTIVE_ENDIF",
	[HCC_PP_DIRECTIVE_LINE] = "HCC_PP_DIRECTIVE_LINE",
	[HCC_PP_DIRECTIVE_ERROR] = "HCC_PP_DIRECTIVE_ERROR",
	[HCC_PP_DIRECTIVE_WARNING] = "HCC_PP_DIRECTIVE_WARNING",
	[HCC_PP_DIRECTIVE_PRAGMA] = "HCC_PP_DIRECTIVE_PRAGMA",
};

char* hcc_pp_directive_strings[HCC_PP_DIRECTIVE_COUNT] = {
	[HCC_PP_DIRECTIVE_DEFINE] = "define",
	[HCC_PP_DIRECTIVE_UNDEF] = "undef",
	[HCC_PP_DIRECTIVE_INCLUDE] = "include",
	[HCC_PP_DIRECTIVE_IF] = "if",
	[HCC_PP_DIRECTIVE_IFDEF] = "ifdef",
	[HCC_PP_DIRECTIVE_IFNDEF] = "ifndef",
	[HCC_PP_DIRECTIVE_ELSE] = "else",
	[HCC_PP_DIRECTIVE_ELIF] = "elif",
	[HCC_PP_DIRECTIVE_ELIFDEF] = "elifdef",
	[HCC_PP_DIRECTIVE_ELIFNDEF] = "elifndef",
	[HCC_PP_DIRECTIVE_ENDIF] = "endif",
	[HCC_PP_DIRECTIVE_LINE] = "line",
	[HCC_PP_DIRECTIVE_ERROR] = "error",
	[HCC_PP_DIRECTIVE_WARNING] = "warning",
	[HCC_PP_DIRECTIVE_PRAGMA] = "pragma",
};

U32 hcc_pp_directive_hashes[] = {
	[HCC_PP_DIRECTIVE_DEFINE] = 0x6a9f8552,
	[HCC_PP_DIRECTIVE_UNDEF] = 0x75191b51,
	[HCC_PP_DIRECTIVE_INCLUDE] = 0x75597a67,
	[HCC_PP_DIRECTIVE_IF] = 0x39386e06,
	[HCC_PP_DIRECTIVE_IFDEF] = 0x546cc0ed,
	[HCC_PP_DIRECTIVE_IFNDEF] = 0xf1e59c9f,
	[HCC_PP_DIRECTIVE_ELSE] = 0xbdbf5bf0,
	[HCC_PP_DIRECTIVE_ELIF] = 0xc0a5c8c3,
	[HCC_PP_DIRECTIVE_ELIFDEF] = 0x7aeee312,
	[HCC_PP_DIRECTIVE_ELIFNDEF] = 0xdc679c86,
	[HCC_PP_DIRECTIVE_ENDIF] = 0x1c12ad4d,
	[HCC_PP_DIRECTIVE_LINE] = 0x17db1627,
	[HCC_PP_DIRECTIVE_ERROR] = 0x21918751,
	[HCC_PP_DIRECTIVE_WARNING] = 0x792112ef,
	[HCC_PP_DIRECTIVE_PRAGMA] = 0x19fa4625,
};

void hcc_pp_init(HccCompiler* c, HccCompilerSetup* setup) {
	c->pp.macro_token_bag.tokens = hcc_stack_init(HccToken, setup->pp.macro_tokens_cap, HCC_ALLOC_TAG_PP_TOKENS);
	c->pp.macro_token_bag.token_location_indices = hcc_stack_init(U32, setup->pp.macro_tokens_cap, HCC_ALLOC_TAG_PP_TOKEN_LOCATION_INDICES);
	c->pp.macro_token_bag.token_values = hcc_stack_init(HccTokenValue, setup->pp.macro_token_values_cap, HCC_ALLOC_TAG_PP_TOKEN_VALUES);
	c->pp.macros = hcc_stack_init(HccPPMacro, setup->pp.macros_cap, HCC_ALLOC_TAG_PP_MACROS);
	c->pp.macro_params = hcc_stack_init(HccStringId, setup->pp.macro_params_cap, HCC_ALLOC_TAG_PP_MACRO_PARAMS);
	c->pp.macro_args_stack = hcc_stack_init(HccPPMacroArg, setup->pp.macro_args_stack_cap, HCC_ALLOC_TAG_PP_MACRO_ARGS_STACK);
	c->pp.expand_stack = hcc_stack_init(HccPPExpand, setup->pp.expand_stack_cap, HCC_ALLOC_TAG_PP_EXPAND_STACK);
	c->pp.expand_macro_idx_stack = hcc_stack_init(U32, setup->pp.expand_stack_cap, HCC_ALLOC_TAG_PP_EXPAND_STACK);
	c->pp.stringify_buffer = hcc_stack_init(char, setup->pp.stringify_buffer_cap, HCC_ALLOC_TAG_PP_STRINGIFY_BUFFER);
	c->pp.if_span_stack = hcc_stack_init(HccPPIfSpan*, setup->pp.if_stack_cap, HCC_ALLOC_TAG_PP_IF_SPAN_STACK);

	hcc_hash_table_init(&c->pp.macro_declarations, setup->pp.macros_cap, HCC_ALLOC_TAG_PP_MACRO_DECLARATIONS);
	for (HccPPPredefinedMacro m = 0; m < HCC_PP_PREDEFINED_MACRO_COUNT; m += 1) {
		if (m == HCC_PP_PREDEFINED_MACRO___HCC_LINUX__ && c->os != HCC_OS_LINUX) {
			continue;
		}
		if (m == HCC_PP_PREDEFINED_MACRO___HCC_WINDOWS__ && c->os != HCC_OS_WINDOWS) {
			continue;
		}
		if (m == HCC_PP_PREDEFINED_MACRO___HCC_X86_64__ && c->arch != HCC_ARCH_X86_64) {
			continue;
		}

		U32* macro_idx_ptr;
		HccStringId identifier_string_id = { HCC_STRING_ID_PREDEFINED_MACROS_START + m };
		bool found = hcc_hash_table_find_or_insert(&c->pp.macro_declarations, identifier_string_id.idx_plus_one, &macro_idx_ptr);
		HCC_DEBUG_ASSERT(!found, "internal error: predefined macro has already been initialized");
		*macro_idx_ptr = U32_MAX;
	}
}

HccPPMacro* hcc_pp_macro_get(HccCompiler* c, U32 macro_idx) {
	return hcc_stack_get(c->pp.macros, macro_idx);
}

HccPPIfSpan* hcc_pp_if_span_get(HccCompiler* c, U32 if_span_id) {
	HCC_DEBUG_ASSERT_NON_ZERO(if_span_id);
	HccCodeFile* code_file = c->tokengen.location.code_file;
	return hcc_stack_get(code_file->pp_if_spans, if_span_id - 1);
}

U32 hcc_pp_if_span_id(HccCompiler* c, HccPPIfSpan* if_span) {
	HccCodeFile* code_file = c->tokengen.location.code_file;
	HCC_DEBUG_ASSERT(code_file->pp_if_spans <= if_span && if_span <= hcc_stack_get_last(code_file->pp_if_spans), "if_span is out of the array bounds");
	U32 id = (if_span - hcc_stack_get_first(code_file->pp_if_spans)) + 1;
	return id;
}

HccPPIfSpan* hcc_pp_if_span_push(HccCompiler* c, HccPPDirective directive) {
	HccCodeFile* code_file = c->tokengen.location.code_file;
	U32 id = hcc_stack_count(code_file->pp_if_spans) + 1;

	HccPPIfSpan* pp_if_span;
	if (!(code_file->flags & HCC_CODE_FILE_FLAGS_PARSED_ONCE_OR_MORE)) {
#if HCC_DEBUG_CODE_IF_SPAN
		printf("#%s at line %u\n", hcc_pp_directive_strings[directive], c->tokengen.location.line_start);
#endif // HCC_DEBUG_CODE_IF_SPAN
		pp_if_span = hcc_stack_push(code_file->pp_if_spans);
		pp_if_span->directive = directive;
		pp_if_span->location = c->tokengen.location;
		pp_if_span->location.display_line = hcc_tokengen_display_line(c);
		pp_if_span->first_id = hcc_pp_if_span_id(c, pp_if_span);
		pp_if_span->has_else = false;
		pp_if_span->prev_id = 0;
		pp_if_span->next_id = 0;
		pp_if_span->last_id = 0;
	} else {
#if HCC_DEBUG_CODE_IF_SPAN
		printf("refound #%s at line %u\n", hcc_pp_directive_strings[directive], c->tokengen.location.line_start);
#endif // HCC_DEBUG_CODE_IF_SPAN
		code_file->pp_if_span_id += 1;
		pp_if_span = hcc_pp_if_span_get(c, code_file->pp_if_span_id);

		HccLocation location = c->tokengen.location;
		location.display_line = pp_if_span->location.display_line;
		HCC_DEBUG_ASSERT(
			pp_if_span->directive == directive &&
			HCC_CMP_ELMT(&pp_if_span->location, &location),
			"internal error: preprocessor if mismatch from first time this was included"
		);
	}

	return pp_if_span;
}

void hcc_pp_if_found_if(HccCompiler* c, HccPPDirective directive) {
	HccPPIfSpan* pp_if_span = hcc_pp_if_span_push(c, directive);
	*hcc_stack_push(c->pp.if_span_stack) = pp_if_span;
}

HccPPIfSpan* hcc_pp_if_found_if_counterpart(HccCompiler* c, HccPPDirective directive) {
	HccCodeFile* code_file = c->tokengen.location.code_file;
	HccPPIfSpan* counterpart = hcc_pp_if_span_push(c, directive);
	U32 id = hcc_pp_if_span_id(c, counterpart);

	if (!(code_file->flags & HCC_CODE_FILE_FLAGS_PARSED_ONCE_OR_MORE)) {
		HccPPIfSpan* pp_if_span = *hcc_stack_get_last(c->pp.if_span_stack);
		counterpart->first_id = hcc_pp_if_span_id(c, pp_if_span);
		counterpart->prev_id = pp_if_span->last_id;
		if (counterpart->prev_id) {
			HccPPIfSpan* prev = hcc_pp_if_span_get(c, counterpart->prev_id);
			prev->next_id = id;
		} else {
			pp_if_span->next_id = id;
		}
		pp_if_span->last_id = id;
	}

	return counterpart;
}

void hcc_pp_if_found_endif(HccCompiler* c) {
	HccPPIfSpan* pp_if_span = hcc_pp_if_found_if_counterpart(c, HCC_PP_DIRECTIVE_ENDIF);
	pp_if_span->next_id = 0;
	hcc_stack_pop(c->pp.if_span_stack);
}

void hcc_pp_if_found_else(HccCompiler* c, HccPPDirective directive) {
	HccPPIfSpan* pp_if_span = *hcc_stack_get_last(c->pp.if_span_stack);
	pp_if_span->has_else = true;

	pp_if_span = hcc_pp_if_found_if_counterpart(c, directive);
}

void hcc_pp_if_ensure_first_else(HccCompiler* c, HccPPDirective directive) {
	HccPPIfSpan* pp_if_span = *hcc_stack_get_last(c->pp.if_span_stack);

	HccCodeFile* code_file = c->tokengen.location.code_file;
	if (!(code_file->flags & HCC_CODE_FILE_FLAGS_PARSED_ONCE_OR_MORE)) {
		if (pp_if_span->has_else) {
			hcc_tokengen_bail_error_2_ptr(c, HCC_ERROR_CODE_PP_ELSEIF_CANNOT_FOLLOW_ELSE, &c->tokengen.location, &pp_if_span->location, hcc_pp_directive_strings[directive]);
		}
	}
}

void hcc_pp_if_ensure_one_is_open(HccCompiler* c, HccPPDirective directive) {
	if (hcc_stack_count(c->pp.if_span_stack) == 0) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_PP_ENDIF_BEFORE_IF, hcc_pp_directive_strings[directive]);
	}
}

void hcc_pp_eval_binary_op(HccCompiler* c, U32* token_idx_mut, HccBinaryOp* binary_op_type_out, U32* precedence_out) {
	HccToken token = *hcc_stack_get(c->tokengen.token_bag.tokens, *token_idx_mut);
	switch (token) {
		case HCC_TOKEN_ASTERISK:              *binary_op_type_out = HCC_BINARY_OP_MULTIPLY;              *precedence_out = 3;  break;
		case HCC_TOKEN_FORWARD_SLASH:         *binary_op_type_out = HCC_BINARY_OP_DIVIDE;                *precedence_out = 3;  break;
		case HCC_TOKEN_PERCENT:               *binary_op_type_out = HCC_BINARY_OP_MODULO;                *precedence_out = 3;  break;
		case HCC_TOKEN_PLUS:                  *binary_op_type_out = HCC_BINARY_OP_ADD;                   *precedence_out = 4;  break;
		case HCC_TOKEN_MINUS:                 *binary_op_type_out = HCC_BINARY_OP_SUBTRACT;              *precedence_out = 4;  break;
		case HCC_TOKEN_BIT_SHIFT_LEFT:        *binary_op_type_out = HCC_BINARY_OP_BIT_SHIFT_LEFT;        *precedence_out = 5;  break;
		case HCC_TOKEN_BIT_SHIFT_RIGHT:       *binary_op_type_out = HCC_BINARY_OP_BIT_SHIFT_RIGHT;       *precedence_out = 5;  break;
		case HCC_TOKEN_LESS_THAN:             *binary_op_type_out = HCC_BINARY_OP_LESS_THAN;             *precedence_out = 6;  break;
		case HCC_TOKEN_LESS_THAN_OR_EQUAL:    *binary_op_type_out = HCC_BINARY_OP_LESS_THAN_OR_EQUAL;    *precedence_out = 6;  break;
		case HCC_TOKEN_GREATER_THAN:          *binary_op_type_out = HCC_BINARY_OP_GREATER_THAN;          *precedence_out = 6;  break;
		case HCC_TOKEN_GREATER_THAN_OR_EQUAL: *binary_op_type_out = HCC_BINARY_OP_GREATER_THAN_OR_EQUAL; *precedence_out = 6;  break;
		case HCC_TOKEN_LOGICAL_EQUAL:         *binary_op_type_out = HCC_BINARY_OP_EQUAL;                 *precedence_out = 7;  break;
		case HCC_TOKEN_LOGICAL_NOT_EQUAL:     *binary_op_type_out = HCC_BINARY_OP_NOT_EQUAL;             *precedence_out = 7;  break;
		case HCC_TOKEN_AMPERSAND:             *binary_op_type_out = HCC_BINARY_OP_BIT_AND;               *precedence_out = 8;  break;
		case HCC_TOKEN_CARET:                 *binary_op_type_out = HCC_BINARY_OP_BIT_XOR;               *precedence_out = 9;  break;
		case HCC_TOKEN_PIPE:                  *binary_op_type_out = HCC_BINARY_OP_BIT_OR;                *precedence_out = 10; break;
		case HCC_TOKEN_LOGICAL_AND:           *binary_op_type_out = HCC_BINARY_OP_LOGICAL_AND;           *precedence_out = 11; break;
		case HCC_TOKEN_LOGICAL_OR:            *binary_op_type_out = HCC_BINARY_OP_LOGICAL_OR;            *precedence_out = 12; break;
		case HCC_TOKEN_QUESTION_MARK:         *binary_op_type_out = HCC_BINARY_OP_TERNARY;               *precedence_out = 13; break;
		case HCC_TOKEN_COMMA:                 *binary_op_type_out = HCC_BINARY_OP_COMMA;                 *precedence_out = 15; break;

		case HCC_TOKEN_PARENTHESIS_CLOSE:
		case HCC_TOKEN_COLON:
			*binary_op_type_out = HCC_BINARY_OP_ASSIGN;
			*precedence_out = 0;
			break;

		default:
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PP_BINARY_OP, hcc_token_strings[token]);
			break;
	}
}

HccPPEval hcc_pp_eval_unary_expr(HccCompiler* c, U32* token_idx_mut, U32* token_value_idx_mut) {
	HccToken token = *hcc_stack_get(c->tokengen.token_bag.tokens, *token_idx_mut);
	*token_idx_mut += 1;

	HccPPEval eval;
	HccUnaryOp unary_op;
	switch (token) {
		case HCC_TOKEN_LIT_UINT:
		case HCC_TOKEN_LIT_ULONG:
		case HCC_TOKEN_LIT_ULONGLONG: {
			HccConstantId constant_id = hcc_stack_get(c->tokengen.token_bag.token_values, *token_value_idx_mut)->constant_id;
			*token_value_idx_mut += 1;

			HccConstant constant = hcc_constant_table_get(c, constant_id);
			U64 u64;
			HCC_DEBUG_ASSERT(hcc_constant_as_uint(c, constant, &u64), "internal error: expected to be a unsigned int");

			eval.is_signed = false;
			eval.u64 = u64;
			break;
		};

		case HCC_TOKEN_LIT_SINT:
		case HCC_TOKEN_LIT_SLONG:
		case HCC_TOKEN_LIT_SLONGLONG: {
			HccConstantId constant_id = hcc_stack_get(c->tokengen.token_bag.token_values, *token_value_idx_mut)->constant_id;
			*token_value_idx_mut += 1;

			HccConstant constant = hcc_constant_table_get(c, constant_id);
			S64 s64;
			HCC_DEBUG_ASSERT(hcc_constant_as_sint(c, constant, &s64), "internal error: expected to be a signed int");

			eval.is_signed = true;
			eval.s64 = s64;
			break;
		};

		case HCC_TOKEN_PARENTHESIS_OPEN:
			eval = hcc_pp_eval_expr(c, 0, token_idx_mut, token_value_idx_mut);
			token = *hcc_stack_get(c->tokengen.token_bag.tokens, *token_idx_mut);
			if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
				hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE);
			}
			*token_idx_mut += 1;
			break;

		case HCC_TOKEN_TILDE: unary_op = HCC_UNARY_OP_BIT_NOT; goto UNARY;
		case HCC_TOKEN_EXCLAMATION_MARK: unary_op = HCC_UNARY_OP_LOGICAL_NOT; goto UNARY;
		case HCC_TOKEN_PLUS: unary_op = HCC_UNARY_OP_PLUS; goto UNARY;
		case HCC_TOKEN_MINUS: unary_op = HCC_UNARY_OP_NEGATE; goto UNARY;
UNARY:
		{
			eval = hcc_pp_eval_expr(c, 0, token_idx_mut, token_value_idx_mut);
			switch (unary_op) {
				case HCC_TOKEN_TILDE:            eval.u64 = ~eval.u64; break;
				case HCC_TOKEN_EXCLAMATION_MARK: eval.u64 = !eval.u64; break;
				case HCC_TOKEN_PLUS:                                   break;
				case HCC_TOKEN_MINUS:            eval.u64 = -eval.u64; break;
				default: HCC_UNREACHABLE();
			}
			break;
		};

		default:
			if (HCC_TOKEN_IS_KEYWORD(token) || token == HCC_TOKEN_IDENT) {
				//
				// the spec states that any identifier that is not evaluated during macro expansion
				// gets substituded for a 0.
				//
				eval.is_signed = true;
				eval.s64 = 0;

				if (hcc_options_is_enabled(c, HCC_OPTION_PP_UNDEF_EVAL)) {
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_UNDEFINED_IDENTIFIER_IN_PP_EXPR);
				}
			} else {
				*token_idx_mut -= 1;
				hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PP_UNARY_EXPR, hcc_token_strings[token]);
			}
			break;
	}

	return eval;
}

HccPPEval hcc_pp_eval_expr(HccCompiler* c, U32 min_precedence, U32* token_idx_mut, U32* token_value_idx_mut) {
	HccPPEval left_eval = hcc_pp_eval_unary_expr(c, token_idx_mut, token_value_idx_mut);

	while (*token_idx_mut < hcc_stack_count(c->tokengen.token_bag.tokens)) {
		HccBinaryOp binary_op_type;
		U32 precedence;
		hcc_pp_eval_binary_op(c, token_idx_mut, &binary_op_type, &precedence);
		if (binary_op_type == HCC_BINARY_OP_ASSIGN || (min_precedence && min_precedence <= precedence)) {
			return left_eval;
		}
		*token_idx_mut += 1;

		HccPPEval eval;
		if (binary_op_type == HCC_BINARY_OP_TERNARY) {
			HccPPEval true_eval = hcc_pp_eval_expr(c, 0, token_idx_mut, token_value_idx_mut);
			HccToken token = *hcc_stack_get(c->tokengen.token_bag.tokens, *token_idx_mut);
			if (token != HCC_TOKEN_COLON) {
				hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_COLON_FOR_TERNARY_OP);
			}
			*token_idx_mut += 1;
			HccPPEval false_eval = hcc_pp_eval_expr(c, 0, token_idx_mut, token_value_idx_mut);
			eval = left_eval.u64 ? true_eval : false_eval;
		} else {
			HccPPEval right_eval = hcc_pp_eval_expr(c, precedence, token_idx_mut, token_value_idx_mut);
			eval.is_signed = left_eval.is_signed == right_eval.is_signed;
			switch (binary_op_type) {
				case HCC_BINARY_OP_ADD: eval.u64 = left_eval.u64 + right_eval.u64; break;
				case HCC_BINARY_OP_SUBTRACT: eval.u64 = left_eval.u64 - right_eval.u64; break;
				case HCC_BINARY_OP_MULTIPLY: eval.u64 = left_eval.u64 * right_eval.u64; break;
				case HCC_BINARY_OP_DIVIDE:
					if (eval.is_signed) { eval.s64 = left_eval.s64 / right_eval.s64; }
					else {                eval.u64 = left_eval.u64 / right_eval.u64; }
					break;
				case HCC_BINARY_OP_MODULO:
					if (eval.is_signed) { eval.s64 = left_eval.s64 % right_eval.s64; }
					else {                eval.u64 = left_eval.u64 % right_eval.u64; }
					break;
				case HCC_BINARY_OP_BIT_AND:
					if (eval.is_signed) { eval.s64 = left_eval.s64 & right_eval.s64; }
					else {                eval.u64 = left_eval.u64 & right_eval.u64; }
					break;
				case HCC_BINARY_OP_BIT_OR:
					if (eval.is_signed) { eval.s64 = left_eval.s64 | right_eval.s64; }
					else {                eval.u64 = left_eval.u64 | right_eval.u64; }
					break;
				case HCC_BINARY_OP_BIT_XOR:
					if (eval.is_signed) { eval.s64 = left_eval.s64 ^ right_eval.s64; }
					else {                eval.u64 = left_eval.u64 ^ right_eval.u64; }
					break;
				case HCC_BINARY_OP_BIT_SHIFT_LEFT:
					if (eval.is_signed) { eval.s64 = left_eval.s64 << right_eval.s64; }
					else {                eval.u64 = left_eval.u64 << right_eval.u64; }
					break;
				case HCC_BINARY_OP_BIT_SHIFT_RIGHT:
					if (eval.is_signed) { eval.s64 = left_eval.s64 >> right_eval.s64; }
					else {                eval.u64 = left_eval.u64 >> right_eval.u64; }
					break;
				case HCC_BINARY_OP_EQUAL:     eval.u64 = left_eval.u64 == right_eval.u64; break;
				case HCC_BINARY_OP_NOT_EQUAL: eval.u64 = left_eval.u64 != right_eval.u64; break;
				case HCC_BINARY_OP_LESS_THAN:
					if (eval.is_signed) { eval.s64 = left_eval.s64 < right_eval.s64; }
					else {                eval.u64 = left_eval.u64 < right_eval.u64; }
					break;
				case HCC_BINARY_OP_LESS_THAN_OR_EQUAL:
					if (eval.is_signed) { eval.s64 = left_eval.s64 <= right_eval.s64; }
					else {                eval.u64 = left_eval.u64 <= right_eval.u64; }
					break;
				case HCC_BINARY_OP_GREATER_THAN:
					if (eval.is_signed) { eval.s64 = left_eval.s64 > right_eval.s64; }
					else {                eval.u64 = left_eval.u64 > right_eval.u64; }
					break;
				case HCC_BINARY_OP_GREATER_THAN_OR_EQUAL:
					if (eval.is_signed) { eval.s64 = left_eval.s64 >= right_eval.s64; }
					else {                eval.u64 = left_eval.u64 >= right_eval.u64; }
					break;
				case HCC_BINARY_OP_LOGICAL_AND:
					eval.u64 = left_eval.u64 && right_eval.u64;
					break;
				case HCC_BINARY_OP_LOGICAL_OR:
					eval.u64 = left_eval.u64 || right_eval.u64;
					break;
				default:
					HCC_UNREACHABLE();
			}
		}

		left_eval = eval;
	}

	return left_eval;
}

void hcc_pp_ensure_end_of_directive(HccCompiler* c, HccErrorCode error_code, HccPPDirective directive) {
	char byte = c->tokengen.code[c->tokengen.location.code_end_idx];
	char next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
	if (
		byte != '\r' &&
		byte != '\n' &&
		!(byte == '/' && next_byte == '/') &&
		!(byte == '/' && next_byte == '*')
	) {
		if (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
			hcc_tokengen_bail_error_1(c, error_code, hcc_pp_directive_strings[directive]);
		}
	}
}

void hcc_pp_parse_define(HccCompiler* c) {
	//
	// skip the whitespace after the #define
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	//
	// parse the identifier for the macro
	HccString ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER);
	HccStringId identifier_string_id = hcc_string_table_deduplicate(&c->string_table, (char*)ident_string.data, ident_string.size);
	hcc_tokengen_advance_column(c, ident_string.size);
	HccLocation ident_location = c->tokengen.location;

	U32 params_start_idx = hcc_stack_count(c->pp.macro_params);
	U32 params_count = 0;
	bool is_function = false;
	bool has_va_args = false;
	if (c->tokengen.code[c->tokengen.location.code_end_idx] == '(') {
		//
		// we found a open parenthesis right after the identifier
		// so this is a function-like macro.
		// let's collect the parameter names inside of the pair of parenthesis.
		//
		is_function = true;
		hcc_tokengen_advance_column(c, 1);
		hcc_tokengen_consume_whitespace(c);

		if (c->tokengen.code[c->tokengen.location.code_end_idx] != ')') {
			//
			// function-like has parameters
			//
			while (1) { // for each parameter
				params_count += 1;
				HccStringId param_string_id;
				if (
					c->tokengen.code[c->tokengen.location.code_end_idx] == '.' &&
					c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '.' &&
					c->tokengen.code[c->tokengen.location.code_end_idx + 2] == '.'
				) {
					//
					// we found a ellipse '...' aka. the va arg
					//
					hcc_tokengen_advance_column(c, 3);
					has_va_args = true;
					param_string_id.idx_plus_one = HCC_STRING_ID___VA_ARGS__;
				} else {
					//
					// parse the parameter identifier and ensure that it is unique
					//
					HccString ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_PARAM_IDENTIFIER);
					param_string_id = hcc_string_table_deduplicate(&c->string_table, (char*)ident_string.data, ident_string.size);
					hcc_tokengen_advance_column(c, ident_string.size);
					if (param_string_id.idx_plus_one == HCC_STRING_ID___VA_ARGS__) {
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_VA_ARGS_IN_MACRO_PARAMETER);
					}

					for (U32 idx = params_start_idx; idx < hcc_stack_count(c->pp.macro_params); idx += 1) {
						if (hcc_stack_get(c->pp.macro_params, idx)->idx_plus_one == param_string_id.idx_plus_one) {
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_DUPLICATE_MACRO_PARAM_IDENTIFIER, (int)ident_string.size, ident_string.data);
						}
					}
				}

				//
				// push the parameter onto the end of the array
				*hcc_stack_push(c->pp.macro_params) = param_string_id;
				hcc_tokengen_consume_whitespace(c);

				U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
				if (byte == ')') {
					//
					// we found the closing parenthesis so we are finished here
					break;
				}

				if (byte != ',') {
					//
					// huh, we couldn't find a ')' or a ',' after the identifier
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_MACRO_PARAM_DELIMITER);
				}

				if (has_va_args) {
					//
					// a ',' was found after the va arg '...'
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MACRO_PARAM_VA_ARG_NOT_LAST);
				}

				hcc_tokengen_advance_column(c, 1); // skip ','
				hcc_tokengen_consume_whitespace(c);
			}
		}

		hcc_tokengen_advance_column(c, 1); // skip ')'
	}

	hcc_tokengen_consume_whitespace(c);

	U32 tokens_start_idx = hcc_stack_count(c->pp.macro_token_bag.tokens);
	U32 token_values_start_idx = hcc_stack_count(c->pp.macro_token_bag.token_values);
	c->tokengen.macro_is_function = is_function;
	c->tokengen.macro_has_va_arg = has_va_args;
	c->tokengen.macro_param_string_ids = hcc_stack_get_or_null(c->pp.macro_params, params_start_idx);
	c->tokengen.macro_params_count = params_count;
	hcc_tokengen_run(c, &c->pp.macro_token_bag, HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST);
	U32 tokens_end_idx = hcc_stack_count(c->pp.macro_token_bag.tokens);
	U32 token_values_end_idx = hcc_stack_count(c->pp.macro_token_bag.token_values);
	U32 tokens_count = tokens_end_idx - tokens_start_idx;
	U32 token_values_count = token_values_end_idx - token_values_start_idx;

	if (tokens_count && c->pp.macro_token_bag.tokens[tokens_end_idx - 1] == HCC_TOKEN_MACRO_WHITESPACE) {
		tokens_end_idx -= 1;
		tokens_count -= 1;
	}

	U32* macro_idx_ptr;
	if (hcc_hash_table_find_or_insert(&c->pp.macro_declarations, identifier_string_id.idx_plus_one, &macro_idx_ptr)) {
		HccPPMacro* existing_macro = hcc_pp_macro_get(c, *macro_idx_ptr);
		HccToken* macro_tokens = hcc_stack_get_or_null(c->pp.macro_token_bag.tokens, tokens_start_idx);
		HccTokenValue* macro_token_values = hcc_stack_get_or_null(c->pp.macro_token_bag.token_values, token_values_start_idx);
		HccToken* existing_macro_tokens = hcc_stack_get_or_null(c->pp.macro_token_bag.tokens, existing_macro->token_cursor.tokens_start_idx);
		HccTokenValue* existing_macro_token_values = hcc_stack_get_or_null(c->pp.macro_token_bag.token_values, existing_macro->token_cursor.token_value_idx);
		HccStringId* param_string_ids = hcc_stack_get_or_null(c->pp.macro_params, params_start_idx);
		HccStringId* existing_param_string_ids = hcc_stack_get_or_null(c->pp.macro_params, existing_macro->params_start_idx);
		if (
			is_function != existing_macro->is_function ||
			(is_function && (
				params_count != existing_macro->params_count ||
				!HCC_CMP_ELMT_MANY(param_string_ids, existing_param_string_ids, params_count)
			)) ||
			tokens_count != hcc_token_cursor_tokens_count(&existing_macro->token_cursor) ||
			!HCC_CMP_ELMT_MANY(macro_tokens, existing_macro_tokens, tokens_count) ||
			!HCC_CMP_ELMT_MANY(macro_token_values, existing_macro_token_values, token_values_count)
		) {
			HccLocation* l = hcc_stack_push(c->tokengen.token_locations);
			*l = ident_location;
			hcc_tokengen_bail_error_2_ptr(c, HCC_ERROR_CODE_MACRO_ALREADY_DEFINED, l, &existing_macro->location, (int)ident_string.size, ident_string.data);
		}

		//
		// we have found an identical redefinition of a macro,
		// so just removed the parameters and tokens that where parsed and return.
		hcc_stack_resize(c->pp.macro_params, params_start_idx);
		hcc_stack_resize(c->pp.macro_token_bag.tokens, tokens_start_idx);
		hcc_stack_resize(c->pp.macro_token_bag.token_location_indices, tokens_start_idx);
		hcc_stack_resize(c->pp.macro_token_bag.token_values, token_values_start_idx);
		return;
	}
	*macro_idx_ptr = hcc_stack_count(c->pp.macros);

	HccPPMacro* macro = hcc_stack_push(c->pp.macros);
	macro->identifier_string_id = identifier_string_id;
	macro->location = ident_location;
	macro->token_cursor.tokens_start_idx = tokens_start_idx;
	macro->token_cursor.tokens_end_idx = tokens_end_idx;
	macro->token_cursor.token_value_idx = token_values_start_idx;
	macro->token_cursor.token_idx = tokens_start_idx;
	macro->params_start_idx = params_start_idx;
	macro->params_count = params_count;
	macro->is_function = is_function;
	macro->has_va_args = has_va_args;
}

void hcc_pp_parse_undef(HccCompiler* c) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	//
	// parse the identifier for the macro
	HccString ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER);
	HccStringId identifier_string_id = hcc_string_table_deduplicate(&c->string_table, (char*)ident_string.data, ident_string.size);
	hcc_tokengen_advance_column(c, ident_string.size);
	hcc_tokengen_consume_whitespace(c);

	//
	// remove the macro from the hash table. we do not need to error if the macro is not defined.
	U32 macro_idx;
	hcc_hash_table_remove(&c->pp.macro_declarations, identifier_string_id.idx_plus_one, &macro_idx);

	hcc_pp_ensure_end_of_directive(c, HCC_ERROR_CODE_TOO_MANY_UNDEF_OPERANDS, HCC_PP_DIRECTIVE_UNDEF);
}

void hcc_pp_parse_include(HccCompiler* c) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	//
	// run the tokenizer to get the single operand and expand any macros
	HccTokenBag* token_bag = &c->tokengen.token_bag;
	U32 tokens_start_idx = hcc_stack_count(token_bag->tokens);
	U32 token_values_start_idx = hcc_stack_count(token_bag->token_values);
	U32 token_location_indices_start_idx = hcc_stack_count(token_bag->token_location_indices);
	U32 token_location_start_idx = hcc_stack_count(c->tokengen.token_locations);
	hcc_tokengen_run(c, token_bag, HCC_TOKENGEN_RUN_MODE_PP_INCLUDE_OPERAND);

	//
	// error if no operands found
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_INCLUDE_OPERAND);
	}

	//
	// error if more that 1 operands found
	if (tokens_start_idx + 1 != hcc_stack_count(token_bag->tokens)) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_TOO_MANY_INCLUDE_OPERANDS);
	}

	HccToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx);
	if (token != HCC_TOKEN_STRING && token != HCC_TOKEN_INCLUDE_PATH_SYSTEM) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_INCLUDE_OPERAND);
	}

	HccStringId path_string_id = hcc_stack_get(token_bag->token_values, token_values_start_idx)->string_id;
	HccString path_string = hcc_string_table_get(&c->string_table, path_string_id);
	if (path_string.size <= 1) { // <= as it has a null terminator
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INCLUDE_PATH_IS_EMPTY);
	}

	bool search_the_include_paths = false;
	switch (token) {
		case HCC_TOKEN_STRING:
			if (hcc_file_exist(path_string.data)) {
				break;
			}

			//
			// the spec states the #include "" get 'upgraded' to a #include <>
			// if the file does not exist.
			//
			// fallthrough
		case HCC_TOKEN_INCLUDE_PATH_SYSTEM:
			search_the_include_paths = !hcc_path_is_absolute(path_string.data);
			break;
		default:
			HCC_UNREACHABLE("internal error: the token should have been checked above to ensure we never reach here");
	}

	if (search_the_include_paths) {
		//
		// we have a #include <> so search through all of the system library directories
		// and the user defined ones using the -I argument.
		//
		U32 idx;
		U32 count = hcc_stack_count(c->include_paths);
		for (idx = 0; idx < count; idx += 1) {
			//
			// build {include_path}/{path} in the string buffer
			//
			hcc_stack_clear(c->string_buffer);

			HccString include_dir_path = *hcc_stack_get(c->include_paths, idx);
			HCC_DEBUG_ASSERT(include_dir_path.size, "internal error: include directory path cannot be zero sized");

			hcc_stack_push_string(c->string_buffer, include_dir_path);
			if (include_dir_path.data[include_dir_path.size - 1] != '/') {
				*hcc_stack_push(c->string_buffer) = '/';
			}
			hcc_stack_push_string(c->string_buffer, path_string);
			*hcc_stack_push(c->string_buffer) = '\0';

			if (hcc_file_exist(c->string_buffer)) {
				path_string = hcc_string(c->string_buffer, hcc_stack_count(c->string_buffer));
				break;
			}
		}

		if (idx == count) {
			c->tokengen.location = *hcc_token_bag_location_get(c, token_bag, tokens_start_idx);
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INCLUDE_PATH_DOES_NOT_EXIST);
		}
	}

	HccCodeFileId code_file_id;
	HccCodeFile* code_file;
	bool found_file = hcc_code_file_find_or_insert(c, path_string, &code_file_id, &code_file);
	if (!found_file) {
		//
		// file does not exist so read the file into memory
		//
		U64 code_size;
		char* code = hcc_file_read_all_the_codes(path_string.data, &code_size);
		if (code == NULL) {
			char os_error[512];
			hcc_get_last_system_error_string(os_error, sizeof(os_error));
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_FAILED_TO_OPEN_FILE_FOR_READ, path_string.data, os_error);
		}

		code_file->code = hcc_string(code, code_size);
	}

	if (!(code_file->flags & HCC_CODE_FILE_FLAGS_PRAGMA_ONCE)) {
		hcc_tokengen_location_push(c);
		hcc_tokengen_location_setup_new_file(c, code_file, true);
	}


	//
	// remove the added token for when we evaluated the include operand
	// using the call to hcc_tokengen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->token_values, token_values_start_idx);
	hcc_stack_resize(token_bag->token_location_indices, token_location_indices_start_idx);
	hcc_stack_resize(c->tokengen.token_locations, token_location_start_idx);
}

bool hcc_pp_parse_if(HccCompiler* c) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	//
	// run the tokenizer to get the #if condition as a list of tokens
	HccTokenBag* token_bag = &c->tokengen.token_bag;
	U32 tokens_start_idx = hcc_stack_count(token_bag->tokens);
	U32 token_values_start_idx = hcc_stack_count(token_bag->token_values);
	U32 token_location_indices_start_idx = hcc_stack_count(token_bag->token_location_indices);
	U32 token_location_start_idx = hcc_stack_count(c->tokengen.token_locations);
	hcc_tokengen_run(c, token_bag, HCC_TOKENGEN_RUN_MODE_PP_IF_OPERAND);

	//
	// error if no tokens found
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_CONDITION_HAS_NO_PP_TOKENS);
	}

	//
	// evaluate the tokens and compute a boolean value
	U32 token_idx = tokens_start_idx;
	U32 token_value_idx = token_values_start_idx;
	bool is_true = !!hcc_pp_eval_expr(c, 0, &token_idx, &token_value_idx).u64;

	HCC_DEBUG_ASSERT(token_idx == hcc_stack_count(token_bag->tokens), "internal error: preprocessor expression has not been fully evaluated");

	//
	// remove the added expression tokens for the #if operand that were
	// generated by the call to hcc_tokengen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->token_values, token_values_start_idx);
	hcc_stack_resize(token_bag->token_location_indices, token_location_indices_start_idx);
	hcc_stack_resize(c->tokengen.token_locations, token_location_start_idx);

	return is_true;
}

void hcc_pp_parse_defined(HccCompiler* c) {
	hcc_tokengen_advance_column(c, sizeof("defined") - 1);
	hcc_tokengen_consume_whitespace(c);

	bool has_parenthesis = c->tokengen.code[c->tokengen.location.code_end_idx] == '(';
	if (has_parenthesis) {
		hcc_tokengen_advance_column(c, 1); // skip '('
		hcc_tokengen_consume_whitespace(c);
	}

	HccString macro_ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_PP_IF_DEFINED);
	HccStringId macro_string_id = hcc_string_table_deduplicate(&c->string_table, (char*)macro_ident_string.data, macro_ident_string.size);

	bool does_macro_exist = hcc_hash_table_find(&c->pp.macro_declarations, macro_string_id.idx_plus_one, NULL);
	hcc_tokengen_advance_column(c, macro_ident_string.size);

	if (has_parenthesis) {
		hcc_tokengen_consume_whitespace(c);
		if (c->tokengen.code[c->tokengen.location.code_end_idx] != ')') {
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_DEFINED);
		}
		hcc_tokengen_advance_column(c, 1); // skip ')'
	}

	HccConstantId* basic_type_constant_ids = does_macro_exist ? c->basic_type_one_constant_ids : c->basic_type_zero_constant_ids;
	HccTokenValue token_value;
	token_value.constant_id = basic_type_constant_ids[HCC_DATA_TYPE_SINT];

	hcc_tokengen_token_add(c, HCC_TOKEN_LIT_SINT);
	hcc_tokengen_token_value_add(c, token_value);
}

bool hcc_pp_parse_ifdef(HccCompiler* c, HccPPDirective directive) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	HccString ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER);
	HccStringId identifier_string_id = hcc_string_table_deduplicate(&c->string_table, (char*)ident_string.data, ident_string.size);
	hcc_tokengen_advance_column(c, ident_string.size);
	hcc_tokengen_consume_whitespace(c);

	bool is_true = hcc_hash_table_find(&c->pp.macro_declarations, identifier_string_id.idx_plus_one, NULL);
	hcc_pp_ensure_end_of_directive(c, HCC_ERROR_CODE_TOO_MANY_IFDEF_OPERANDS, directive);
	return is_true;
}

void hcc_pp_parse_line(HccCompiler* c) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	//
	// run the tokenizer to get the #line operands as a list of tokens
	HccTokenBag* token_bag = &c->tokengen.token_bag;
	U32 tokens_start_idx = hcc_stack_count(token_bag->tokens);
	U32 token_values_start_idx = hcc_stack_count(token_bag->token_values);
	U32 token_location_indices_start_idx = hcc_stack_count(token_bag->token_location_indices);
	U32 token_location_start_idx = hcc_stack_count(c->tokengen.token_locations);
	hcc_tokengen_run(c, token_bag, HCC_TOKENGEN_RUN_MODE_PP_OPERAND);

	//
	// error if no tokens found
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS);
	}

	//
	// get the custom line number from the token
	S64 custom_line;
	{
		HccToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx);
		if (token != HCC_TOKEN_LIT_SINT) {
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS);
		}
		HccConstantId constant_id = hcc_stack_get(token_bag->token_values, token_values_start_idx)->constant_id;
		HccConstant constant = hcc_constant_table_get(c, constant_id);

		HCC_DEBUG_ASSERT(hcc_constant_as_sint(c, constant, &custom_line), "internal error: expected to be a signed int");
		if (custom_line < 0 || custom_line > S32_MAX) {
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_PP_LINE_MUST_BE_MORE_THAN_ZERO, S32_MAX);
		}
	}

	//
	// get the custom path from the next token if there is one
	HccString custom_path = {0};
	if (tokens_start_idx + 1 < hcc_stack_count(token_bag->tokens)) {
		HccToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx + 1);
		if (token != HCC_TOKEN_STRING) {
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS);
		}
		HccStringId string_id = hcc_stack_get(token_bag->token_values, token_values_start_idx + 2)->string_id;
		custom_path = hcc_string_table_get(&c->string_table, string_id);
	}

	//
	// error if too many operands
	if (tokens_start_idx + 2 < hcc_stack_count(token_bag->tokens)) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_TOO_MANY_PP_LINE_OPERANDS);
	}

	//
	// store the custom line and custom path in the code file we are currently parsing
	c->tokengen.custom_line_dst = custom_line;
	c->tokengen.custom_line_src = c->tokengen.location.line_start;
	if (custom_path.data) {
		c->tokengen.location.display_path = custom_path;
	}

	//
	// remove the added operand tokens that were
	// generated by the call to hcc_tokengen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->token_values, token_values_start_idx);
	hcc_stack_resize(token_bag->token_location_indices, token_location_indices_start_idx);
	hcc_stack_resize(c->tokengen.token_locations, token_location_start_idx);
}

void hcc_pp_parse_error(HccCompiler* c) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	HccString message = hcc_string((char*)&c->tokengen.code[c->tokengen.location.code_end_idx], 0);
	hcc_tokengen_consume_until_any_byte(c, "\n");
	message.size = (char*)&c->tokengen.code[c->tokengen.location.code_end_idx] - message.data;

	hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_PP_ERROR, (int)message.size, message.data);
}

void hcc_pp_parse_warning(HccCompiler* c) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	HccString message = hcc_string((char*)&c->tokengen.code[c->tokengen.location.code_end_idx], 0);
	hcc_tokengen_consume_until_any_byte(c, "\n");
	message.size = (char*)&c->tokengen.code[c->tokengen.location.code_end_idx] - message.data;

	//
	// make a copy of the location for this warning since we overrite this when we continue tokenizing
	HccLocation* location = hcc_stack_push(c->tokengen.token_locations);
	*location = c->tokengen.location;
	location->display_line = hcc_tokengen_display_line(c);

	hcc_warn_push(c, HCC_WARN_CODE_PP_WARNING, location, NULL, (int)message.size, message.data);
}

void hcc_pp_parse_pragma(HccCompiler* c) {
	hcc_tokengen_consume_whitespace(c);
	c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
	c->tokengen.location.column_start = c->tokengen.location.column_end;

	//
	// check if this is STDC which is a thing that the spec states should exist
	if (
		c->tokengen.code[c->tokengen.location.code_end_idx + 0] == 'S' &&
		c->tokengen.code[c->tokengen.location.code_end_idx + 1] == 'T' &&
		c->tokengen.code[c->tokengen.location.code_end_idx + 2] == 'D' &&
		c->tokengen.code[c->tokengen.location.code_end_idx + 3] == 'C'
	) {
		HCC_ABORT("TODO: implement #pragma STDC support");
		return;
	}

	//
	// run the tokenizer to get the #pragma operands as a list of tokens
	HccTokenBag* token_bag = &c->tokengen.token_bag;
	U32 tokens_start_idx = hcc_stack_count(token_bag->tokens);
	U32 token_values_start_idx = hcc_stack_count(token_bag->token_values);
	U32 token_location_indices_start_idx = hcc_stack_count(token_bag->token_location_indices);
	U32 token_location_start_idx = hcc_stack_count(c->tokengen.token_locations);
	hcc_tokengen_run(c, token_bag, HCC_TOKENGEN_RUN_MODE_PP_OPERAND);

	//
	// if no tokens exist then we do nothing
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		return;
	}

	HccToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx);
	if (token != HCC_TOKEN_IDENT) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PP_PRAGMA_OPERAND);
	}

	U32 expected_tokens_count = hcc_stack_count(token_bag->tokens);
	HccStringId ident_string_id = hcc_stack_get(token_bag->token_values, token_values_start_idx)->string_id;
	switch (ident_string_id.idx_plus_one) {
		case HCC_STRING_ID_ONCE: {
			if (hcc_stack_count(c->tokengen.paused_file_stack) == 0) {
				hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_PP_PRAGMA_OPERAND_USED_IN_MAIN_FILE);
			}

			HccCodeFile* code_file = c->tokengen.location.code_file;
			code_file->flags |= HCC_CODE_FILE_FLAGS_PRAGMA_ONCE;
			expected_tokens_count = tokens_start_idx + 1;
			break;
		};
	}

	if (expected_tokens_count != hcc_stack_count(token_bag->tokens)) {
		HccString ident_string = hcc_string_table_get(&c->string_table, ident_string_id);
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_TOO_MANY_PP_PRAGMA_OPERANDS, (int)ident_string.size, ident_string.data);
	}

	//
	// remove the added operand tokens that were
	// generated by the call to hcc_tokengen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->token_values, token_values_start_idx);
	hcc_stack_resize(token_bag->token_location_indices, token_location_indices_start_idx);
	hcc_stack_resize(c->tokengen.token_locations, token_location_start_idx);
}

HccPPDirective hcc_pp_parse_directive_header(HccCompiler* c) {
	hcc_tokengen_advance_column(c, 1); // skip '#'
	hcc_tokengen_consume_whitespace(c);

	U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
	switch (byte) {
		case '\r':
		case '\n':
		case '\0':
			return HCC_PP_DIRECTIVE_COUNT;
	}

	HccString ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_INVALID_TOKEN_PREPROCESSOR_DIRECTIVE);
	hcc_tokengen_advance_column(c, ident_string.size);

	HccPPDirective directive = hcc_string_to_enum_hashed_find(ident_string, hcc_pp_directive_hashes, HCC_PP_DIRECTIVE_COUNT);
	if (directive == HCC_PP_DIRECTIVE_COUNT) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PREPROCESSOR_DIRECTIVE, (int)ident_string.size, ident_string.data);
	}

	return directive;
}

void hcc_pp_parse_directive(HccCompiler* c) {
	HccPPDirective directive = hcc_pp_parse_directive_header(c);
	if (directive == HCC_PP_DIRECTIVE_COUNT) {
		// we only found a '#' on this line, so return
		return;
	}

#if HCC_DEBUG_CODE_PREPROCESSOR
	U32 debug_indent_level = hcc_stack_count(c->pp.if_span_stack);
	U32 debug_line = c->tokengen.location.line_end - 1;
	if (directive == HCC_PP_DIRECTIVE_ENDIF) {
		debug_indent_level -= 1;
	}
#endif

	bool is_skipping_code = false;
	bool is_skipping_until_endif = false;
	switch (directive) {
		case HCC_PP_DIRECTIVE_DEFINE:
			hcc_pp_parse_define(c);
			break;
		case HCC_PP_DIRECTIVE_UNDEF:
			hcc_pp_parse_undef(c);
			break;
		case HCC_PP_DIRECTIVE_INCLUDE:
			hcc_pp_parse_include(c);
			break;
		case HCC_PP_DIRECTIVE_LINE:
			hcc_pp_parse_line(c);
			break;
		case HCC_PP_DIRECTIVE_ERROR:
			hcc_pp_parse_error(c);
			break;
		case HCC_PP_DIRECTIVE_WARNING:
			hcc_pp_parse_warning(c);
			break;
		case HCC_PP_DIRECTIVE_PRAGMA:
			hcc_pp_parse_pragma(c);
			break;
		case HCC_PP_DIRECTIVE_IF: {
			hcc_pp_if_found_if(c, directive);
			is_skipping_code = !hcc_pp_parse_if(c);
			break;
		};
		case HCC_PP_DIRECTIVE_IFDEF:
		case HCC_PP_DIRECTIVE_IFNDEF: {
			hcc_pp_if_found_if(c, directive);
			is_skipping_code = hcc_pp_parse_ifdef(c, directive) != (directive == HCC_PP_DIRECTIVE_IFDEF);
			break;
		};
		case HCC_PP_DIRECTIVE_ENDIF:
			hcc_pp_if_ensure_one_is_open(c, directive);
			hcc_pp_if_found_endif(c);
			break;
		case HCC_PP_DIRECTIVE_ELSE:
		case HCC_PP_DIRECTIVE_ELIF:
		case HCC_PP_DIRECTIVE_ELIFDEF:
		case HCC_PP_DIRECTIVE_ELIFNDEF:
			hcc_pp_if_ensure_one_is_open(c, directive);
			hcc_pp_if_ensure_first_else(c, directive);
			if (directive == HCC_PP_DIRECTIVE_ELSE) {
				hcc_pp_if_found_else(c, directive);
			} else {
				hcc_pp_if_found_if_counterpart(c, directive);
			}

			//
			// we where just in a true block and processing the code as normal.
			// now we have come across some else directive, we just skip until the #endif.
			is_skipping_code = true;
			is_skipping_until_endif = true;
			break;
	}

#if HCC_DEBUG_CODE_PREPROCESSOR
	printf("+(%u)#%-7s at line %u\n", debug_indent_level, hcc_pp_directive_strings[directive], debug_line);
#endif

	if (is_skipping_code) {
		hcc_pp_skip_false_conditional(c, is_skipping_until_endif);
	}
}

void hcc_pp_skip_false_conditional(HccCompiler* c, bool is_skipping_until_endif) {
	bool first_non_white_space_char = false;
	U32 nested_level = hcc_stack_count(c->pp.if_span_stack);
	HccCodeFile* code_file = c->tokengen.location.code_file;
	while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
		if (!(code_file->flags & HCC_CODE_FILE_FLAGS_PARSED_ONCE_OR_MORE)) {
			//
			// TODO test a SIMD optimized version of this
			//
			while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
				U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
				if (byte == '\n') break;
				if (byte == '#') break;
				first_non_white_space_char = false;
				hcc_tokengen_advance_column(c, 1);
			}
		} else {
			HccPPIfSpan* pp_if_span = hcc_pp_if_span_get(c, code_file->pp_if_span_id);
			if (is_skipping_until_endif) {
				HccPPIfSpan* first_pp_if_span = hcc_pp_if_span_get(c, pp_if_span->first_id);
				HccPPIfSpan* last_pp_if_span = hcc_pp_if_span_get(c, first_pp_if_span->last_id);
				c->tokengen.location = last_pp_if_span->location;
				code_file->pp_if_span_id = first_pp_if_span->last_id - 1;
			} else {
				HccPPIfSpan* next_pp_if_span = hcc_pp_if_span_get(c, pp_if_span->next_id);
				c->tokengen.location = next_pp_if_span->location;
				code_file->pp_if_span_id = pp_if_span->next_id - 1;
			}

			//
			// put cursor back to the start of the line
			c->tokengen.location.code_end_idx = c->tokengen.location.code_start_idx;
			c->tokengen.location.column_start = 1;
			c->tokengen.location.column_end = 1;
			first_non_white_space_char = true;
		}

		if (c->tokengen.location.code_end_idx >= c->tokengen.code_size) {
			return;
		}

		U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
		switch (byte) {
			case '\n':
				hcc_tokengen_advance_newline(c);
				c->tokengen.location.line_start = c->tokengen.location.line_end - 1;
				c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
				c->tokengen.location.column_start = 1;
				first_non_white_space_char = true;
				break;
			case '#': {
				if (!first_non_white_space_char) {
					c->tokengen.location.column_end += 1;
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_PP_DIRECTIVE_NOT_FIRST_ON_LINE);
				}

				HccPPDirective directive = hcc_pp_parse_directive_header(c);
				if (directive == HCC_PP_DIRECTIVE_COUNT) {
					// we only found a '#' on this line, so continue;
					continue;
				}

				bool is_finished = false;
				switch (directive) {
					case HCC_PP_DIRECTIVE_DEFINE:
					case HCC_PP_DIRECTIVE_UNDEF:
					case HCC_PP_DIRECTIVE_INCLUDE:
					case HCC_PP_DIRECTIVE_LINE:
					case HCC_PP_DIRECTIVE_ERROR:
					case HCC_PP_DIRECTIVE_WARNING:
					case HCC_PP_DIRECTIVE_PRAGMA:
						//
						// consume until the new line and continue so the operands
						// of these defines do not mess with the skipping code.
						hcc_tokengen_consume_until_any_byte(c, "\n");
						break;
					case HCC_PP_DIRECTIVE_IF:
					case HCC_PP_DIRECTIVE_IFDEF:
					case HCC_PP_DIRECTIVE_IFNDEF: {
						//
						// push any if on the stack so we can keep track of when reach
						// the original if block's #endif or #else
						//
						hcc_pp_if_found_if(c, directive);
						break;
					};
					case HCC_PP_DIRECTIVE_ENDIF:
						if (hcc_stack_count(c->pp.if_span_stack) == nested_level) {
							is_finished = true;
						}
						hcc_pp_if_found_endif(c);
						break;
					case HCC_PP_DIRECTIVE_ELSE:
					case HCC_PP_DIRECTIVE_ELIF:
					case HCC_PP_DIRECTIVE_ELIFDEF:
					case HCC_PP_DIRECTIVE_ELIFNDEF:
						hcc_pp_if_ensure_first_else(c, directive);
						if (directive == HCC_PP_DIRECTIVE_ELSE) {
							hcc_pp_if_found_else(c, directive);
						} else {
							hcc_pp_if_found_if_counterpart(c, directive);
						}

						if (!is_skipping_until_endif && hcc_stack_count(c->pp.if_span_stack) == nested_level) {
							switch (directive) {
								case HCC_PP_DIRECTIVE_ELSE:
									is_finished = true;
									break;
								case HCC_PP_DIRECTIVE_ELIF:
									is_finished = hcc_pp_parse_if(c);
									break;
								case HCC_PP_DIRECTIVE_ELIFDEF:
								case HCC_PP_DIRECTIVE_ELIFNDEF:
									is_finished = hcc_pp_parse_ifdef(c, directive) == (directive == HCC_PP_DIRECTIVE_ELIFDEF);
									break;
							}
						}
						break;
				}

#if HCC_DEBUG_CODE_PREPROCESSOR
				char* plus_or_minus = is_skipping_code ? "-" : "+";
				printf("%s(%u)#%-7s at line %u\n", plus_or_minus, debug_indent_level, hcc_pp_directive_strings[directive], debug_line);
#endif

				if (is_finished) {
					return;
				}

				first_non_white_space_char = false;
				break;
			};
		}
	}
}

void hcc_pp_copy_expand_predefined_macro(HccCompiler* c, HccPPPredefinedMacro predefined_macro) {
	switch (predefined_macro) {
		case HCC_PP_PREDEFINED_MACRO___FILE__: {
			HccTokenValue value = { .string_id = c->tokengen.location.code_file->path_string_id };
			hcc_tokengen_token_add(c, HCC_TOKEN_STRING);
			hcc_tokengen_token_value_add(c, value);
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___LINE__: {
			HccBasic line_num = hcc_basic_from_sint(c, HCC_DATA_TYPE_SINT, c->tokengen.location.line_end - 1);
			HccTokenValue token_value = {
				.constant_id = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SINT, &line_num),
			};
			hcc_tokengen_token_add(c, HCC_TOKEN_LIT_SINT);
			hcc_tokengen_token_value_add(c, token_value);
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___COUNTER__: {
			HccBasic counter = hcc_basic_from_sint(c, HCC_DATA_TYPE_SINT, c->tokengen.__counter__);
			HccTokenValue token_value = {
				.constant_id = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SINT, &counter),
			};
			hcc_tokengen_token_add(c, HCC_TOKEN_LIT_SINT);
			hcc_tokengen_token_value_add(c, token_value);
			c->tokengen.__counter__ += 1;
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___HCC__:
		case HCC_PP_PREDEFINED_MACRO___HCC_GPU__:
		case HCC_PP_PREDEFINED_MACRO___HCC_X86_64__:
		case HCC_PP_PREDEFINED_MACRO___HCC_LINUX__:
		case HCC_PP_PREDEFINED_MACRO___HCC_WINDOWS__:
			return;
	}
}

void hcc_pp_copy_expand_macro_begin(HccCompiler* c, HccPPMacro* macro, HccLocation* macro_callsite_location) {
	HccPPExpand args_expand;
	HccTokenBag* args_src_bag = &c->pp.macro_token_bag;
	if (macro->is_function) {
		HCC_DEBUG_ASSERT(c->tokengen.code[c->tokengen.location.code_end_idx] == '(', "internal error: expected to be on a '(' but got '%c'", c->tokengen.code[c->tokengen.location.code_end_idx]);
		//
		// our arguments are currently in string form.
		// tokenize them before we call into the expand code.
		//
		args_expand.macro = NULL;
		args_expand.cursor.tokens_start_idx = hcc_stack_count(args_src_bag->tokens);
		args_expand.cursor.token_idx = hcc_stack_count(args_src_bag->tokens);
		args_expand.cursor.token_value_idx = hcc_stack_count(args_src_bag->token_values);

		hcc_tokengen_run(c, args_src_bag, HCC_TOKENGEN_RUN_MODE_PP_MACRO_ARGS);
		args_expand.cursor.tokens_end_idx = hcc_stack_count(args_src_bag->tokens);
	}

	//
	// make the location a stable pointer
	*hcc_stack_push(c->tokengen.token_locations) = *macro_callsite_location;
	macro_callsite_location = hcc_stack_get_last(c->tokengen.token_locations);

	HccTokenBag* dst_bag = &c->tokengen.token_bag;
	HccTokenBag* alt_dst_bag = &c->pp.macro_token_bag;
	hcc_pp_copy_expand_macro(c, macro, macro_callsite_location, macro_callsite_location, &args_expand, args_src_bag, dst_bag, alt_dst_bag, HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION);
}

bool hcc_pp_is_callable_macro(HccCompiler* c, HccStringId ident_string_id, U32* macro_idx_out) {
	U32 macro_idx;
	if (!hcc_hash_table_find(&c->pp.macro_declarations, ident_string_id.idx_plus_one, &macro_idx)) {
		return false;
	}

	for (U32 idx = 0; idx < hcc_stack_count(c->pp.expand_macro_idx_stack); idx += 1) {
		if (*hcc_stack_get(c->pp.expand_macro_idx_stack, idx) == macro_idx) {
			return false;
		}
	}

	if (macro_idx_out) {
		*macro_idx_out = macro_idx;
	}

	return true;
}

HccPPExpand* hcc_pp_expand_push_macro(HccCompiler* c, HccPPMacro* macro) {
	HccPPExpand* expand = hcc_stack_push(c->pp.expand_stack);
	*hcc_stack_push(c->pp.expand_macro_idx_stack) = macro - c->pp.macros;
	expand->cursor = macro->token_cursor;
	expand->macro = macro;
	return expand;
}

HccPPExpand* hcc_pp_expand_push_macro_arg(HccCompiler* c, U32 param_idx, U32 args_start_idx, HccLocation** callsite_location_out) {
	HccPPMacroArg* arg = hcc_stack_get(c->pp.macro_args_stack, args_start_idx + param_idx);
	if (callsite_location_out) *callsite_location_out = arg->callsite_location;

	HccPPExpand* expand = hcc_stack_push(c->pp.expand_stack);
	*hcc_stack_push(c->pp.expand_macro_idx_stack) = -1;
	expand->macro = NULL;
	expand->cursor = arg->cursor;
	return expand;
}

void hcc_pp_expand_pop(HccCompiler* c, HccPPExpand* expected_expand) {
	HccPPExpand* popped_ptr = hcc_stack_get_last(c->pp.expand_stack);
	HCC_DEBUG_ASSERT(popped_ptr == expected_expand, "internal error: we expected to pop %p but it was actually %p", expected_expand, popped_ptr);
	hcc_stack_pop(c->pp.expand_stack);
	hcc_stack_pop(c->pp.expand_macro_idx_stack);
}

void hcc_pp_copy_expand_range(HccCompiler* c, HccPPExpand* expand, HccTokenBag* dst_bag, HccTokenBag* src_bag, HccTokenBag* alt_dst_bag, HccLocation* parent_or_child_location, HccLocation* grandparent_location, HccPPExpandFlags flags, HccPPMacro* expand_macro) {
	//
	// copy each token from src_bag into the dst_bag, while looking out for macros to expand into the dst_bag
	//
	while (expand->cursor.token_idx < expand->cursor.tokens_end_idx) {
		HccToken token = *hcc_stack_get(src_bag->tokens, expand->cursor.token_idx);
		if (token == HCC_TOKEN_MACRO_WHITESPACE) {
			if ((flags & HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION) || *hcc_stack_get_last(dst_bag->tokens) == HCC_TOKEN_MACRO_WHITESPACE) {
				expand->cursor.token_idx += 1;
				continue;
			}
		}

		U32 location_idx = *hcc_stack_get(src_bag->token_location_indices, expand->cursor.token_idx);
		bool is_preexpanded_macro_arg = location_idx & HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG_MASK;
		location_idx &= ~HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG_MASK;
		HccLocation* token_location = hcc_stack_get(c->tokengen.token_locations, location_idx);

		location_idx = hcc_stack_count(c->tokengen.token_locations);
		HccLocation* location = hcc_stack_push(c->tokengen.token_locations);
		{
			if ((flags & HCC_PP_EXPAND_FLAGS_IS_ARGS) || !is_preexpanded_macro_arg) {
				//
				// location needs a parent as we are either are:
				//     - copying the contents of argument tokens
				//     - copying a macro and are not on any of the preexpanded macro argument tokens
				//
				HccLocation* parent_location;
				HccLocation* child_location;
				HccPPMacro* macro;
				if ((flags & HCC_PP_EXPAND_FLAGS_IS_ARGS)) {
					// for expanded macro argument tokens:
					// the child is the macro parameter location for all of the argument tokens.
					// the parent is the original argument token location at the callsite
					bool is_same = parent_location == grandparent_location;
					parent_location = hcc_stack_push(c->tokengen.token_locations);
					*parent_location = *token_location;
					if (!is_same && grandparent_location) {
						hcc_pp_attach_to_most_parent(c, parent_location, grandparent_location);
					}
					if (!token_location->macro) {
						parent_location->macro = expand->macro;
					}
					child_location = parent_or_child_location;
					macro = expand_macro;
				} else {
					// for expanded macro tokens:
					// the child is the original macro token location
					// the parent is the whole macro span at the callsite
					parent_location = parent_or_child_location;
					child_location = token_location;
					macro = expand_macro;
				}

				*location = *child_location;
				location->parent_location = parent_location;
				location->macro = macro;
			} else {
				//
				// we are on a argument that has been preexpanded before
				// so there is no need to setup any parent location or macro links
				*location = *token_location;
			}
		}

		if (token == HCC_TOKEN_IDENT) {
			HccStringId ident_string_id = hcc_stack_get(src_bag->token_values, expand->cursor.token_value_idx)->string_id;

			U32 macro_idx;
			if (hcc_pp_is_callable_macro(c, ident_string_id, &macro_idx)) {
				//
				// we found a callable macro, lets expand it into the dst_bag
				//
				HccPPMacro* macro = hcc_pp_macro_get(c, macro_idx);
				bool can_expand = true;
				if (macro->is_function) {
					HccToken* next_token = hcc_stack_get_or_null(src_bag->tokens, expand->cursor.token_idx + 1);
					HccToken* next_next_token = hcc_stack_get_or_null(src_bag->tokens, expand->cursor.token_idx + 2);
					can_expand = next_token &&
						(*next_token == HCC_TOKEN_PARENTHESIS_OPEN ||
							(*next_token == HCC_TOKEN_MACRO_WHITESPACE &&
								next_next_token && *next_next_token == HCC_TOKEN_PARENTHESIS_OPEN));
				}

				if (can_expand) {
					expand->cursor.token_idx += 1; // skip the identifier token
					expand->cursor.token_value_idx += 1;

					HccLocation* callsite_location = location;
					HccPPExpandFlags expand_flags = flags;
					if (expand_flags & HCC_PP_EXPAND_FLAGS_IS_ARGS) {
						expand_flags |= HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS;
						expand_flags &= ~HCC_PP_EXPAND_FLAGS_IS_ARGS;
						callsite_location = location->parent_location;
					}
					hcc_pp_copy_expand_macro(c, macro, callsite_location, location, expand, src_bag, dst_bag, alt_dst_bag, expand_flags);
					continue;
				}
			}
		}

		if ((flags & HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION)) {
			HccToken bracket = token - HCC_TOKEN_BRACKET_START;
			if (bracket < HCC_TOKEN_BRACKET_COUNT) {
				if (bracket % 2 == 0) {
					hcc_tokengen_bracket_open(c, token, location);
				} else {
					hcc_tokengen_bracket_close(c, token, location);
				}
			}
		}

		//
		// copy the token/location/value
		//
		*hcc_stack_push(dst_bag->tokens) = token;

		U32 mask = (flags & (HCC_PP_EXPAND_FLAGS_IS_ARGS | HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS)) ? HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG_MASK : 0;
		*hcc_stack_push(dst_bag->token_location_indices) = location_idx | mask;

		expand->cursor.token_idx += 1;

		U32 num_values = hcc_token_num_values(token);
		for (U32 value_idx = 0; value_idx < num_values; value_idx += 1) {
			*hcc_stack_push(dst_bag->token_values) =
				*hcc_stack_get(src_bag->token_values, expand->cursor.token_value_idx);
			expand->cursor.token_value_idx += 1;
		}
	}
}

void hcc_pp_copy_expand_macro(HccCompiler* c, HccPPMacro* macro, HccLocation* macro_callsite_location, HccLocation* parent_location, HccPPExpand* arg_expand, HccTokenBag* args_src_bag, HccTokenBag* dst_bag, HccTokenBag* alt_dst_bag, HccPPExpandFlags flags) {
	U32 restore_to_tokens_count = hcc_stack_count(alt_dst_bag->tokens);
	U32 restore_to_token_values_count = hcc_stack_count(alt_dst_bag->token_values);
	U32 restore_to_expand_args_count = hcc_stack_count(c->pp.macro_args_stack);
	U32 args_start_idx = U32_MAX;
	HccTokenBag* src_bag = &c->pp.macro_token_bag;

	if (macro->is_function) {
		args_start_idx = hcc_pp_process_macro_args(c, macro, arg_expand, args_src_bag, parent_location);
		HccLocation* final_location = hcc_token_bag_location_get(c, args_src_bag, arg_expand->cursor.tokens_end_idx - 1);
		hcc_location_merge_apply(macro_callsite_location, final_location);
	}

	HccPPExpand* macro_expand;
	if (macro->is_function) {
		//
		// copy a contents of the macro and the expanded argument tokens into the alt_dst_bag
		// this is so that the expanded tokens do not go where the macro needs to be expanded in the dst_bag
		//
		U32 tokens_start_idx = hcc_stack_count(alt_dst_bag->tokens);
		U32 token_values_start_idx = hcc_stack_count(alt_dst_bag->token_values);
		HccTokenCursor cursor = macro->token_cursor;
		while (cursor.token_idx < cursor.tokens_end_idx) {
			HccToken token = *hcc_stack_get(src_bag->tokens, cursor.token_idx);
			switch (token) {
				case HCC_TOKEN_MACRO_PARAM: {
					HccLocation* location = hcc_token_bag_location_get(c, src_bag, cursor.token_idx);
					U32 param_idx = hcc_stack_get(src_bag->token_values, cursor.token_value_idx)->macro_param_idx;

					HccPPExpand* param_arg_expand = hcc_pp_expand_push_macro_arg(c, param_idx, args_start_idx, NULL);
					param_arg_expand->macro = arg_expand->macro;

					HccLocation* grandparent_location = (flags & HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS)
						? parent_location
						: parent_location->parent_location;
					hcc_pp_copy_expand_range(c, param_arg_expand, alt_dst_bag, args_src_bag, dst_bag, location, grandparent_location, HCC_PP_EXPAND_FLAGS_IS_ARGS, macro);
					hcc_pp_expand_pop(c, param_arg_expand);

					cursor.token_idx += 1;
					cursor.token_value_idx += 1;
					continue;
				};

				case HCC_TOKEN_MACRO_STRINGIFY:
				case HCC_TOKEN_MACRO_STRINGIFY_WHITESPACE: {
					U32 param_idx = hcc_stack_get(src_bag->token_values, cursor.token_value_idx)->macro_param_idx;

					HccLocation* callsite_location;
					HccPPExpand* param_arg_expand = hcc_pp_expand_push_macro_arg(c, param_idx, args_start_idx, &callsite_location);

					hcc_stack_clear(c->pp.stringify_buffer);
					HccStringId string_id = hcc_token_bag_stringify_range(c, args_src_bag, &param_arg_expand->cursor, NULL);
					hcc_pp_expand_pop(c, param_arg_expand);

					//
					// push on the location
					*hcc_stack_push(alt_dst_bag->token_location_indices) = hcc_stack_count(c->tokengen.token_locations) | HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG_MASK;
					HccLocation* dst_location = hcc_stack_push(c->tokengen.token_locations);
					*dst_location = *hcc_token_bag_location_get(c, src_bag, cursor.token_idx);
					dst_location->parent_location = callsite_location;
					dst_location->macro = macro;

					*hcc_stack_push(alt_dst_bag->tokens) = HCC_TOKEN_STRING;
					hcc_stack_push(alt_dst_bag->token_values)->string_id = string_id;

					cursor.token_idx += 1;
					cursor.token_value_idx += 1;
					continue;
				};

				case HCC_TOKEN_MACRO_CONCAT:
				case HCC_TOKEN_MACRO_CONCAT_WHITESPACE: {
					cursor.token_idx += 1;

					hcc_stack_clear(c->pp.stringify_buffer);

					HccLocation* before = hcc_token_bag_location_get(c, src_bag, cursor.token_idx);
					HccToken before_token = hcc_token_bag_stringify_single_or_macro_param(c, src_bag, &cursor, args_start_idx, args_src_bag, false);
					U32 before_end_idx = hcc_stack_count(c->pp.stringify_buffer) - 1;
					U32 after_start_idx = hcc_stack_count(c->pp.stringify_buffer);
					if (*hcc_stack_get(src_bag->tokens, cursor.token_idx) == HCC_TOKEN_MACRO_WHITESPACE) {
						cursor.token_idx += 1;
					}

					HccLocation* after = hcc_token_bag_location_get(c, src_bag, cursor.token_idx);
					HccToken after_token = hcc_token_bag_stringify_single_or_macro_param(c, src_bag, &cursor, args_start_idx, args_src_bag, true);
					U32 location_idx = hcc_stack_count(c->tokengen.token_locations);
					HccLocation* dst_location = hcc_stack_push(c->tokengen.token_locations);
					*dst_location = *before;
					hcc_location_merge_apply(dst_location, after);
					dst_location->parent_location = parent_location;
					dst_location->macro = macro;

					HccCodeFile* code_file = c->concat_buffer_code_file;
					hcc_stack_clear(code_file->line_code_start_indices);
					*hcc_stack_push(code_file->line_code_start_indices) = 0;
					*hcc_stack_push(code_file->line_code_start_indices) = 0;
					code_file->code = hcc_string(c->pp.stringify_buffer, hcc_stack_count(c->pp.stringify_buffer));
					hcc_tokengen_location_push(c);
					hcc_tokengen_location_setup_new_file(c, code_file, false);
					c->tokengen.location.parent_location = dst_location;

					if (before_token != HCC_TOKEN_COUNT && after_token != HCC_TOKEN_COUNT) {
						if (!hcc_token_concat_is_okay(before_token, after_token)) {
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_PP_CONCAT_OPERANDS, hcc_token_strings[before_token], hcc_token_strings[after_token]);
						}
					}

					U32 token_location_start_idx = hcc_stack_count(alt_dst_bag->token_location_indices);
					hcc_tokengen_run(c, alt_dst_bag, HCC_TOKENGEN_RUN_MODE_PP_CONCAT);

					//
					// hcc_tokengen_run puts the locations in the token bag but we don't care about them.
					// what we really want is the location that spans the concatination operands and links back to their parents.
					// so just link to it as a stable pointer.
					for (U32 idx = token_location_start_idx; idx < hcc_stack_count(alt_dst_bag->token_location_indices); idx += 1) {
						*hcc_stack_get(alt_dst_bag->token_location_indices, idx) = location_idx;
					}

					//
					// remove the added locations that we don't care about
					hcc_stack_resize(c->tokengen.token_locations, location_idx + 1);
					continue;
				};
				case HCC_TOKEN_MACRO_WHITESPACE:
					if (*hcc_stack_get_last(alt_dst_bag->tokens) == HCC_TOKEN_MACRO_WHITESPACE) {
						cursor.token_idx += 1;
						continue;
					}
					break;
			}

			//
			// copy the token/location/value
			//

			*hcc_stack_push(alt_dst_bag->tokens) = token;
			*hcc_stack_push(alt_dst_bag->token_location_indices) = *hcc_stack_get(src_bag->token_location_indices, cursor.token_idx);
			cursor.token_idx += 1;

			U32 num_values = hcc_token_num_values(token);
			for (U32 value_idx = 0; value_idx < num_values; value_idx += 1) {
				*hcc_stack_push(alt_dst_bag->token_values) =
					*hcc_stack_get(src_bag->token_values, cursor.token_value_idx);
				cursor.token_value_idx += 1;
			}
		}

		//
		// setup the copy expand after so that this same macro can be expanded as a macro argument to itself
		macro_expand = hcc_pp_expand_push_macro(c, macro);
		macro_expand->cursor.token_idx = tokens_start_idx;
		macro_expand->cursor.tokens_end_idx = hcc_stack_count(alt_dst_bag->tokens);
		macro_expand->cursor.token_value_idx = token_values_start_idx;
		src_bag = alt_dst_bag;
	} else {
		//
		// just use the macros' tokens directly since this is a non function macro
		//
		macro_expand = hcc_pp_expand_push_macro(c, macro);
	}


	//
	// expand the macro tokens into the dst_bag
	//
	hcc_pp_copy_expand_range(c, macro_expand, dst_bag, src_bag, alt_dst_bag, parent_location, NULL, flags, macro);
	hcc_pp_expand_pop(c, macro_expand);

	//
	// remove all of the temporary tokens from the alt_dst_bag
	// that where generated for the expansion of the macro.
	//
	hcc_stack_resize(alt_dst_bag->tokens, restore_to_tokens_count);
	hcc_stack_resize(alt_dst_bag->token_location_indices, restore_to_tokens_count);
	hcc_stack_resize(alt_dst_bag->token_values, restore_to_token_values_count);
	hcc_stack_resize(c->pp.macro_args_stack, restore_to_expand_args_count);
}

U32 hcc_pp_process_macro_args(HccCompiler* c, HccPPMacro* macro, HccPPExpand* expand, HccTokenBag* src_bag, HccLocation* parent_location) {
	HCC_DEBUG_ASSERT(
		*hcc_stack_get(src_bag->tokens, expand->cursor.token_idx) == HCC_TOKEN_PARENTHESIS_OPEN ||
			(*hcc_stack_get(src_bag->tokens, expand->cursor.token_idx) == HCC_TOKEN_MACRO_WHITESPACE &&
				*hcc_stack_get(src_bag->tokens, expand->cursor.token_idx + 1) == HCC_TOKEN_PARENTHESIS_OPEN),
		"internal error: expected to be on a '(' for the macro function arguments"
	);
	expand->cursor.token_idx += *hcc_stack_get(src_bag->tokens, expand->cursor.token_idx) == HCC_TOKEN_MACRO_WHITESPACE ? 2 : 1;

	//
	// scan the src_bag tokens and keep a record of the token ranges for each macro argument in
	// the c->pp.macro_args_stack array
	//
	U32 args_start_idx = hcc_stack_count(c->pp.macro_args_stack);
	HccPPMacroArg* arg = hcc_pp_push_macro_arg(c, expand, src_bag, parent_location);
	U32 nested_parenthesis = 0;
	bool reached_va_args = macro->has_va_args && macro->params_count == 1;
	while (1) {
		HccToken token = *hcc_stack_get(src_bag->tokens, expand->cursor.token_idx);
		HccLocation* location = hcc_token_bag_location_get(c, src_bag, expand->cursor.token_idx);

		switch (token) {
			case HCC_TOKEN_COMMA:
				if (nested_parenthesis == 0 && !reached_va_args) {
					//
					// we found a ',' outside of nested_parenthesis,
					// so lets finalize the current argument and start the next one.
					expand->cursor.token_idx += 1;
					hcc_pp_finalize_macro_arg(c, arg, expand, src_bag);

					//
					// start the next argument
					//
					arg = hcc_pp_push_macro_arg(c, expand, src_bag, parent_location);

					U32 args_count = hcc_stack_count(c->pp.macro_args_stack) - args_start_idx;
					if (macro->has_va_args && args_count == macro->params_count) {
						//
						// we have reached the variable argument, so just group all
						// of the arguments that follow into this last argument.
						reached_va_args = true;
					}
					continue;
				}
				break;

			case HCC_TOKEN_PARENTHESIS_OPEN:
				nested_parenthesis += 1;
				expand->cursor.token_idx += 1;
				continue;

			case HCC_TOKEN_PARENTHESIS_CLOSE:
				expand->cursor.token_idx += 1;
				if (nested_parenthesis == 0) {
					goto BREAK;
				}
				nested_parenthesis -= 1;
				continue;
		}

		expand->cursor.token_idx += 1;
		expand->cursor.token_value_idx += hcc_token_num_values(token);
	}
BREAK: {}
	hcc_pp_finalize_macro_arg(c, arg, expand, src_bag);

	U32 args_count = hcc_stack_count(c->pp.macro_args_stack) - args_start_idx;
	if (args_count == 1 && arg->cursor.tokens_start_idx == arg->cursor.tokens_end_idx) {
		args_count = 0;
	}

	if (args_count < macro->params_count) {
		// this is not a error that breaks a rule in the C spec.
		// TODO: see if we want ot have a compiler option to enable this.
		// hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_NOT_ENOUGH_MACRO_ARGUMENTS, macro->params_count, args_count);

		U32 missing_count = macro->params_count - args_count;
		HccPPMacroArg* args = hcc_stack_push_many(c->pp.macro_args_stack, missing_count);
		HCC_ZERO_ELMT_MANY(args, missing_count);
	} else if (args_count > macro->params_count) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_TOO_MANY_MACRO_ARGUMENTS, macro->params_count, args_count);
	}
	return args_start_idx;
}

HccPPMacroArg* hcc_pp_push_macro_arg(HccCompiler* c, HccPPExpand* expand, HccTokenBag* src_bag, HccLocation* parent_location) {
	HccPPMacroArg* arg = hcc_stack_push(c->pp.macro_args_stack);
	arg->cursor.tokens_start_idx = expand->cursor.token_idx;
	arg->cursor.token_idx = expand->cursor.token_idx;
	arg->cursor.token_value_idx = expand->cursor.token_value_idx;
	arg->callsite_location = hcc_stack_push(c->tokengen.token_locations);
	HccLocation* src_location = hcc_token_bag_location_get(c, src_bag, expand->cursor.token_idx);
	*arg->callsite_location = *src_location;
	bool is_same = parent_location == src_location;
	if (!is_same && parent_location) {
		hcc_pp_attach_to_most_parent(c, arg->callsite_location, parent_location);
	}
	arg->callsite_location->macro = expand->macro;
	return arg;
}

void hcc_pp_finalize_macro_arg(HccCompiler* c, HccPPMacroArg* arg, HccPPExpand* expand, HccTokenBag* src_bag) {
	arg->cursor.tokens_end_idx = expand->cursor.token_idx - 1;
	HccLocation* location = hcc_token_bag_location_get(c, src_bag, expand->cursor.token_idx - 2);
	hcc_location_merge_apply(arg->callsite_location, location);
	if (*hcc_stack_get(src_bag->tokens, arg->cursor.tokens_start_idx) == HCC_TOKEN_MACRO_WHITESPACE) {
		arg->cursor.tokens_start_idx += 1;
		arg->cursor.token_idx += 1;
	}

	if (*hcc_stack_get(src_bag->tokens, arg->cursor.tokens_end_idx - 1) == HCC_TOKEN_MACRO_WHITESPACE) {
		arg->cursor.tokens_end_idx -= 1;
	}
}

void hcc_pp_attach_to_most_parent(HccCompiler* c, HccLocation* location, HccLocation* parent_location) {
	HccLocation* a = location;
	while (a) {
		HccLocation* b = parent_location;
		while (b) {
			if (a == b) {
				return;
			}

			b = b->parent_location;
		}
		a = a->parent_location;
	}

	HccLocation* prev_pl = location;
	HccLocation* pl = location;
	while (pl->parent_location) {
		prev_pl = pl;
		pl = pl->parent_location;
	}

	if (prev_pl->parent_location) {
		pl = hcc_stack_push(c->tokengen.token_locations);
		*pl = *prev_pl->parent_location;
		prev_pl->parent_location = pl;
	}
	pl->parent_location = parent_location;
}

// ===========================================
//
//
// Error
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
		// tokengen
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
		[HCC_ERROR_CODE_FAILED_TO_OPEN_FILE_FOR_READ] = "failed to open file for read at '%s': %s",
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
		[HCC_ERROR_CODE_MAX_UINT_OVERFLOW] = "integer literal is too large and will overflow a U64 integer",
		[HCC_ERROR_CODE_MAX_SINT_OVERFLOW] = "integer literal is too large and will overflow a S64 integer",
		[HCC_ERROR_CODE_MAX_SINT_OVERFLOW_DECIMAL] = "integer literal is too large and will overflow a S64 integer, consider using 'u' suffix to promote to an unsigned type. e.g. 1000u",
		[HCC_ERROR_CODE_MAX_FLOAT_OVERFLOW] = "float literal is too large and will overflow a f64",
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
		// astgen
		[HCC_ERROR_CODE_CANNOT_FIND_FIELD] = "cannot find a '%.*s' field in the '%.*s' type",
		[HCC_ERROR_CODE_DUPLICATE_FIELD_IDENTIFIER] = "duplicate field identifier '%.*s' in '%.*s'",
		[HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_CONDITION] =  "the condition expression must be convertable to a boolean but got '%.*s'",
		[HCC_ERROR_CODE_MISSING_SEMICOLON] = "missing ';' to end the statement",
		[HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_GLOBAL] = "redefinition of the '%.*s' identifier",
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ENUM] = "expected '{' to declare enum type values",
		[HCC_ERROR_CODE_REIMPLEMENTATION] = "redefinition of '%.*s'",
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
		[HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_COMPOUND_TYPE] = "expected '{' to declare compound type fields",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT_FIELD] = "the '%s' keyword cannot be used on this structure field declaration",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT_FIELD] = "only one of these can be used per field: '%s' or '%s'",
		[HCC_ERROR_CODE_COMPOUND_FIELD_INVALID_TERMINATOR] = "expected 'type name', 'struct' or 'union' to declare another field or '}' to finish declaring the compound type fields",
		[HCC_ERROR_CODE_COMPOUND_FIELD_MISSING_NAME] = "expected an identifier for the field name",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELDS_COUNT] = "expected intrinsic struct '%.*s' to have '%u' fields but got '%u'",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELD] = "expected this intrinsic field to be '%.*s %.*s' for this compiler version",
		[HCC_ERROR_CODE_INTRINSIC_VECTOR_INVALID_SIZE_AND_ALIGN] = "expected the size and align for the intrinsic vector type '%.*s' to be size '%u' and align '%u' but got size '%u' and align '%u'",
		[HCC_ERROR_CODE_INTRINSIC_MATRIX_INVALID_SIZE_AND_ALIGN] = "expected the size and align for the intrinsic matrix type '%.*s' to be size '%u' and align '%u' but got size '%u' and align '%u'",
		[HCC_ERROR_CODE_MISSING_RASTERIZER_STATE_SPECIFIER] = "'%s' specifier must be placed before this struct definition before using '%s' or '%s'",
		[HCC_ERROR_CODE_POSITION_ALREADY_SPECIFIED] = "'%s' has already been specified once before in rasterizer state structure",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_ALIGNAS] = "expected '(' to follow _Alignas that contains a type or integer constant. eg. _Alignas(int) or _Alignas(16)",
		[HCC_ERROR_CODE_INVALID_ALIGNAS_INT_CONSTANT] = "_Alignas integer constant must be an unsigned value",
		[HCC_ERROR_CODE_INVALID_ALIGNAS_OPERAND] = "invalid _Alignas operand. expected a type or integer constant. eg. _Alignas(int) or _Alignas(16)",
		[HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_ALIGNAS] = "expected ')' to follow the type or integer constant for _Alignas",
		[HCC_ERROR_CODE_ALIGNAS_REDUCES_ALIGNMENT] = "_Alignas cannot specify a lower alignment to '%zu' when it already has '%zu'",
		[HCC_ERROR_CODE_UNSUPPORTED_RASTERIZER_STATE_DATA_TYPE] = "'%.*s' data type is not supported as a the rasterizer state field",
		[HCC_ERROR_CODE_UNSUPPORTED_FRAGMENT_STATE_DATA_TYPE] = "'%.*s' data type is not supported as a the fragment state field",
		[HCC_ERROR_CODE_POSITION_MUST_BE_VEC4_F32] = "field marked with '%s' must be a '%.*s' but got '%.*s'",
		[HCC_ERROR_CODE_POSITION_NOT_SPECIFIED] = "'%s' must be specified for a rasterizer state structure",
		[HCC_ERROR_CODE_EXPECTED_TYPE_NAME] = "expected a 'type name' here but got '%s'",
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
		[HCC_ERROR_CODE_SQUARE_BRACE_USED_ON_NON_ARRAY_DATA_TYPE] = "unexpected '[', this can only be used when the left expression is an array or pointer but got '%.*s'",
		[HCC_ERROR_CODE_FULL_STOP_USED_ON_NON_COMPOUND_DATA_TYPE] = "unexpected '.', this can only be used when the left expression is a struct or union type but got '%.*s'",
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
		[HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_VARIABLE] = "unsupported intrinsic type/s used in the variable data type '%.*s'. if you wish to use this type, please turn on extension support for the follow types: %.*s",
		[HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_PARAM] = "unsupported intrinsic type/s used in a parameter data type '%.*s'. if you wish to use this type, please turn on extension support for the follow types: %.*s",
		[HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR] = "unsupported intrinsic type/s used in an expression data type '%.*s'. if you wish to use this type, please turn on extension support for the follow types: %.*s",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_VARIABLE_DECL] = "the '%s' keyword cannot be used on variable declarations",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_FUNCTION_DECL] = "the '%s' keyword cannot be used on function declarations",
		[HCC_ERROR_CODE_INVALID_SPECIFIER_FUNCTION_PARAM] = "the '%s' keyword cannot be used on function parameters",
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
		[HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_VERTEX_INVALID_COUNT] = "invalid number of paramters, expected vertex function prototype to be 'RasterizerState vertex(HccVertexInput input, Resources resources)'",
		[HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_VERTEX_INVALID_INPUT] = "the first parameter of a vertex shader must be a 'const HccVertexInput'",
		[HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_COUNT] = "invalid number of paramters, expected fragment function prototype to be 'Fragment fragment(HccFragmentInput input, Resources resources, RasterizerState state)'",
		[HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_INPUT] = "the first parameter of a fragment shader must be a 'const HccFragmentInput'",
		[HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_RASTERIZER_STATE] = "the third parameter of a fragment shader must be declare with HCC_DEFINE_RASTERIZER_STATE",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_RETURN_DATA_TYPE] = "expected intrinsic function '%.*s' to have '%.*s' as the return data type but got '%.*s'",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_PARAMS_COUNT] = "expected intrinsic function '%.*s' to have '%u' parameters but got '%u'",
		[HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_PARAM_DATA_TYPE] = "expected intrinsic function '%.*s' parameter '%u' to have '%.*s' as the data type but got '%.*s'",
		[HCC_ERROR_CODE_FUNCTION_INVALID_TERMINATOR] = "expected a ',' to declaring more function parameters or a ')' to finish declaring function parameters",
		[HCC_ERROR_CODE_EXPECTED_SHADER_PARAM_TO_BE_CONST] = "all shader parameters must be defined with const. eg. 'const %.*s %.*s'",
		[HCC_ERROR_CODE_CANNOT_CALL_SHADER_FUNCTION] = "cannot call shaders like regular functions. they can only be used as entry points",
		[HCC_ERROR_CODE_CANNOT_CALL_UNIMPLEMENTED_FUNCTION] = "cannot call a function with no implemention",
		[HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_MISMATCH_PARAM_DATA_TYPE] = "redefinition of function '%.*s' data type mismatch for parameter '%.*s', expected '%.*s' but got '%.*s'",
		[HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_TOO_MANY_PARAMETERS] = "redefinition of function '%.*s' has too many parameters, expected '%u' but got more defined",
		[HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_NOT_ENOUGH_PARAMETERS] = "redefinition of function '%.*s' has not enough parameters, expected '%u' but got '%u'",
		[HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_BODY_ALREADY_DECLARED] = "redefinition of function '%.*s', body has already been declared",
		[HCC_ERROR_CODE_FUNCTION_RECURSION] = "function '%.*s' is recursively called! callstack:\n%s",
		[HCC_ERROR_CODE_UNEXPECTED_TOKEN_FUNCTION_PROTOTYPE_END] = "unexpected token '%s', expected ';' to end the function definition or '{' to define a function body",
		[HCC_ERROR_CODE_UNEXPECTED_TOKEN] = "unexpected token '%s'",
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

void hcc_location_merge_apply(HccLocation* before, HccLocation* after) {
	before->code_end_idx = after->code_end_idx;
	if (before->line_start != after->line_start) {
		before->column_start = after->column_start;
	}
	before->column_end = after->column_end;
	before->line_start = after->line_start;
	before->line_end = after->line_end;
}

void hcc_message_print_file_line(HccCompiler* c, HccLocation* location) {
	const char* file_path = location->display_path.data ? location->display_path.data : location->code_file->path_string.data;
	U32 display_line = location->display_line ? location->display_line : location->line_start;

	const char* error_fmt = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
		? "\x1b[1;95mfile\x1b[97m: %s:%u:%u\n\x1b[0m"
		: "file: %s:%u:%u\n";
	printf(error_fmt, file_path, display_line, location->column_start);
}

void hcc_message_print_pasted_buffer(HccCompiler* c, U32 line, U32 column) {
	const char* error_fmt = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
		? "\x1b[1;95m<pasted buffer>\x1b[97m: %u:%u\n\x1b[0m"
		: "<pasted buffer>: %u:%u\n";
	printf(error_fmt, line, column);
}

void hcc_message_print_code_line(HccCompiler* c, HccLocation* location, U32 display_line_num_size, U32 line, U32 display_line) {
	U32 line_size = hcc_code_file_line_size(location->code_file, line);

	if (line_size == 0) {
		const char* fmt = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
			? "\x1b[1;94m%*u|\x1b[0m\n"
			: "%*u|\n";
		printf(fmt, display_line_num_size, display_line);
	} else {
		HccCodeFile* code_file = location->code_file;
		U32 code_start_idx = code_file->line_code_start_indices[line];
		char* code = &code_file->code.data[code_start_idx];

		char code_without_tabs[1024];
		U32 dst_idx = 0;
		U32 src_idx = 0;
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

		const char* fmt = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
			? "\x1b[1;94m%*u|\x1b[0m %.*s\n"
			: "%*u| %.*s\n";
		printf(fmt, display_line_num_size, display_line, line_size, code_without_tabs);
	}
}

void hcc_message_print_code(HccCompiler* c, HccLocation* location) {
	if (location->parent_location) {
		HccLocation* parent_location = location->parent_location;
		hcc_message_print_code(c, parent_location);
		printf("\n");

		HccPPMacro* macro = location->macro;
		if (macro) {
			HccString ident_string = hcc_string_table_get(&c->string_table, macro->identifier_string_id);

			const char* error_fmt = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
				? "\x1b[1;96mexpanded from macro\x1b[97m: %.*s\n\x1b[0m"
				: "expanded from macro: %.*s\n";
			printf(error_fmt, (int)ident_string.size, ident_string.data);
		}
	}
	hcc_message_print_file_line(c, location);

	U32 error_lines_count = location->line_end - location->line_start;

	//
	// count the number of digits in the largest line number
	U32 display_line_num_size = 0;
	U32 line = location->line_end + 2;
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
	U32 tab_size = 4;

	display_line_num_size = HCC_MAX(display_line_num_size, 5);
	display_line_num_size = HCC_INT_ROUND_UP_ALIGN(display_line_num_size, tab_size) - 2;

	line = location->line_start;
	U32 display_line = location->display_line ? location->display_line : location->line_start;
	if (line > 2) {
		hcc_message_print_code_line(c, location, display_line_num_size, line - 2, display_line - 2);
	}
	if (line > 1) {
		hcc_message_print_code_line(c, location, display_line_num_size, line - 1, display_line - 1);
	}

	U32 column_start = location->column_start;
	U32 column_end;
	HccCodeFile* code_file = location->code_file;
	for (U32 idx = 0; idx < error_lines_count; idx += 1) {
		U32 code_start_idx = code_file->line_code_start_indices[line + idx];
		if (idx + 1 == error_lines_count) {
			column_end = location->column_end;
		} else {
			column_end = hcc_code_file_line_size(code_file, line + idx) + 1;
		}

		hcc_message_print_code_line(c, location, display_line_num_size, line + idx, display_line + idx);

		//
		// print the padding for to get to the error location on the line
		for (U32 i = 0; i < display_line_num_size + 2; i += 1) {
			putchar(' ');
		}
		for (U32 i = 0; i < HCC_MAX(column_start, 1) - 1; i += 1) {
			if (code_file->code.data[code_start_idx + i] == '\t') {
				printf("%.*s", tab_size, "        ");
			} else {
				putchar(' ');
			}
		}

		U32 column_end_with_tabs = column_end;
		for (U32 i = column_start - 1; i < column_end - 1; i += 1) {
			if (code_file->code.data[code_start_idx + i] == '\t') {
				column_end_with_tabs += 3;
			}
		}
		column_end_with_tabs = HCC_MAX(column_end_with_tabs, column_start + 1);

		if (hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)) {
			printf("\x1b[1;93m");
		}
		for (U32 i = 0; i < column_end_with_tabs - column_start; i += 1) {
			putchar('^');
		}
		if (hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)) {
			printf("\x1b[0m");
		}
		printf("\n");
		column_start = 1;
	}

	line = location->line_end - 1;
	U32 lines_count = hcc_code_file_lines_count(code_file);
	if (line + 1 < lines_count) {
		hcc_message_print_code_line(c, location, display_line_num_size, line + 1, display_line + 1);
	}
	if (line + 2 < lines_count) {
		hcc_message_print_code_line(c, location, display_line_num_size, line + 2, display_line + 2);
	}
}

void hcc_message_print(HccCompiler* c, HccMessage* message) {
	HccLang lang = HCC_LANG_ENG;
	const char* message_type = hcc_message_type_lang_strings[lang][message->type];
	const char* message_color = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
		? hcc_message_type_ascii_color_code[message->type]
		: "";
	const char* error_fmt = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
		? "%s%s\x1b[0m: \x1b[1;97m%.*s\x1b[0m\n"
		: "%s%s: %.*s\n";
	printf(error_fmt, message_color, message_type, (int)message->string.size, message->string.data);

	hcc_message_print_code(c, message->location);

	if (message->other_location) {
		const char* error_fmt = hcc_options_is_enabled(c, HCC_OPTION_PRINT_COLOR)
			? "\x1b[1;97m\noriginally defined here\x1b[0m: \n"
			: "\noriginally defined here: ";
		printf("%s", error_fmt);

		hcc_message_print_code(c, message->other_location);
	}

	printf("\n");
}

void hcc_message_pushv(HccCompiler* c, HccMessageType type, HccMessageCode code, HccLocation* location, HccLocation* other_location, va_list va_args) {
	HccMessageSys* sys = &c->message_sys;

	HccMessage* m;
	if (c->message_sys.next_is_deferred) {
		m = hcc_stack_push(sys->deferred_elmts);
		c->message_sys.next_is_deferred = false;
	} else {
		m = hcc_stack_push(sys->elmts);
		sys->used_type_flags |= type;
	}
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
	Uptr count = vsnprintf(NULL, 0, fmt, va_args_copy) + 1;
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

void hcc_message_push(HccCompiler* c, HccMessageType type, HccMessageCode code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_message_pushv(c, type, code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_error_pushv(HccCompiler* c, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, va_list va_args) {
	hcc_message_pushv(c, HCC_MESSAGE_TYPE_ERROR, error_code, location, other_location, va_args);
}

void hcc_error_push(HccCompiler* c, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_message_pushv(c, HCC_MESSAGE_TYPE_ERROR, error_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_warn_pushv(HccCompiler* c, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, va_list va_args) {
	hcc_message_pushv(c, HCC_MESSAGE_TYPE_WARN, warn_code, location, other_location, va_args);
}

void hcc_warn_push(HccCompiler* c, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_message_pushv(c, HCC_MESSAGE_TYPE_WARN, warn_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_message_copy_deferred(HccCompiler* c, U32 start_idx, U32 count) {
	HccMessage* deferred_messages = hcc_stack_get(c->message_sys.deferred_elmts, start_idx);
	HccMessage* dst = hcc_stack_push_many(c->message_sys.elmts, count);
	HCC_COPY_ELMT_MANY(dst, deferred_messages, count);
	c->message_sys.used_type_flags |= HCC_MESSAGE_TYPE_ERROR;
}

// ===========================================
//
//
// Code File
//
//
// ===========================================

HccCodeFile* hcc_code_file_init(HccCompiler* c, HccStringId path_string_id) {
	HccCodeFile* code_file = hcc_stack_push(c->code_files);
	code_file->path_string_id = path_string_id;
	code_file->path_string = hcc_string_table_get(&c->string_table, path_string_id);
	code_file->line_code_start_indices = hcc_stack_init(U32, c->code_file_lines_cap, HCC_ALLOC_TAG_CODE_FILE_LINE_CODE_START_INDICES);
	code_file->pp_if_spans = hcc_stack_init(HccPPIfSpan, c->code_file_pp_if_spans_cap, HCC_ALLOC_TAG_CODE_FILE_PP_IF_SPANS);
	hcc_stack_push_many(code_file->line_code_start_indices, 2);
	return code_file;
}

bool hcc_code_file_find_or_insert(HccCompiler* c, HccString path_string, HccCodeFileId* code_file_id_out, HccCodeFile** code_file_out) {
	char* path_string_c = hcc_path_canonicalize(path_string.data);
	path_string = hcc_string_c(path_string_c);
	path_string.size += 1; // include the null terminator in the string
	HccStringId path_string_id = hcc_string_table_deduplicate(&c->string_table, path_string.data, path_string.size);

	U32* code_file_idx;
	if (hcc_hash_table_find_or_insert(&c->path_to_code_file_id_map, path_string_id.idx_plus_one, &code_file_idx)) {
		*code_file_id_out = ((HccCodeFileId) { .idx_plus_one = *code_file_idx + 1 });
		*code_file_out = hcc_stack_get(c->code_files, *code_file_idx);
		return true;
	}
	*code_file_idx = hcc_stack_count(c->code_files);

	*code_file_out = hcc_code_file_init(c, path_string_id);
	*code_file_id_out = ((HccCodeFileId) { .idx_plus_one = *code_file_idx + 1 });
	return false;
}

U32 hcc_code_file_line_size(HccCodeFile* code_file, U32 line) {
	U32 code_start_idx = *hcc_stack_get(code_file->line_code_start_indices, line);
	U32 code_end_idx;
	if (line >= hcc_code_file_lines_count(code_file)) {
		code_end_idx = code_file->code.size;
	} else {
		code_end_idx = *hcc_stack_get(code_file->line_code_start_indices, line + 1);

		//
		// move back until we reach the final character on a non empty line
		while (code_end_idx) {
			code_end_idx -= 1;
			U8 byte = code_file->code.data[code_end_idx];
			if (byte != '\r' && byte != '\n') {
				code_end_idx += 1;
				break;
			}
		}
	}

	if (code_start_idx < code_end_idx) {
		return code_end_idx - code_start_idx;
	} else {
		return 0;
	}
}

U32 hcc_code_file_lines_count(HccCodeFile* code_file) {
	return hcc_stack_count(code_file->line_code_start_indices) - 1;
}

// ===========================================
//
//
// Declarations
//
//
// ===========================================

U8 hcc_basic_type_size_and_aligns_x86_64_linux[HCC_DATA_TYPE_BASIC_COUNT] = {
	[HCC_DATA_TYPE_BOOL] = 1,
	[HCC_DATA_TYPE_CHAR] = 1,
	[HCC_DATA_TYPE_SCHAR] = 1,
	[HCC_DATA_TYPE_UCHAR] = 1,
	[HCC_DATA_TYPE_SSHORT] = 2,
	[HCC_DATA_TYPE_USHORT] = 2,
	[HCC_DATA_TYPE_SINT] = 4,
	[HCC_DATA_TYPE_UINT] = 4,
	[HCC_DATA_TYPE_SLONG] = 8,
	[HCC_DATA_TYPE_ULONG] = 8,
	[HCC_DATA_TYPE_SLONGLONG] = 8,
	[HCC_DATA_TYPE_ULONGLONG] = 8,
	[HCC_DATA_TYPE_FLOAT] = 4,
	[HCC_DATA_TYPE_DOUBLE] = 8,
};

U64 hcc_basic_type_int_mins_x86_64_linux[HCC_DATA_TYPE_BASIC_COUNT] = {
	[HCC_DATA_TYPE_CHAR] = INT8_MIN,
	[HCC_DATA_TYPE_SCHAR] = INT8_MIN,
	[HCC_DATA_TYPE_SSHORT] = INT16_MIN,
	[HCC_DATA_TYPE_SINT] = INT32_MIN,
	[HCC_DATA_TYPE_SLONG] = INT64_MIN,
	[HCC_DATA_TYPE_SLONGLONG] = INT64_MIN,
	[HCC_DATA_TYPE_UCHAR] = 0,
	[HCC_DATA_TYPE_USHORT] = 0,
	[HCC_DATA_TYPE_UINT] = 0,
	[HCC_DATA_TYPE_ULONG] = 0,
	[HCC_DATA_TYPE_ULONGLONG] = 0,
};

U64 hcc_basic_type_int_maxes_x86_64_linux[HCC_DATA_TYPE_BASIC_COUNT] = {
	[HCC_DATA_TYPE_CHAR] = INT8_MAX,
	[HCC_DATA_TYPE_SCHAR] = INT8_MAX,
	[HCC_DATA_TYPE_SSHORT] = INT16_MAX,
	[HCC_DATA_TYPE_SINT] = INT32_MAX,
	[HCC_DATA_TYPE_SLONG] = INT64_MAX,
	[HCC_DATA_TYPE_SLONGLONG] = INT64_MAX,
	[HCC_DATA_TYPE_UCHAR] = UINT8_MAX,
	[HCC_DATA_TYPE_USHORT] = UINT16_MAX,
	[HCC_DATA_TYPE_UINT] = UINT32_MAX,
	[HCC_DATA_TYPE_ULONG] = UINT64_MAX,
	[HCC_DATA_TYPE_ULONGLONG] = UINT64_MAX,
};

U8 hcc_packed_vec_sizes[HCC_VEC_COUNT] = {
	[HCC_VEC2BOOL] = 2,
	[HCC_VEC2U8] = 2,
	[HCC_VEC2U16] = 4,
	[HCC_VEC2U32] = 8,
	[HCC_VEC2U64] = 16,
	[HCC_VEC2I8] = 2,
	[HCC_VEC2I16] = 4,
	[HCC_VEC2I32] = 8,
	[HCC_VEC2I64] = 16,
	[HCC_VEC2H] = 4,
	[HCC_VEC2F] = 8,
	[HCC_VEC2D] = 16,
	[HCC_VEC3BOOL] = 3,
	[HCC_VEC3U8] = 3,
	[HCC_VEC3U16] = 6,
	[HCC_VEC3U32] = 12,
	[HCC_VEC3U64] = 24,
	[HCC_VEC3I8] = 3,
	[HCC_VEC3I16] = 6,
	[HCC_VEC3I32] = 12,
	[HCC_VEC3I64] = 24,
	[HCC_VEC3H] = 6,
	[HCC_VEC3F] = 12,
	[HCC_VEC3D] = 24,
	[HCC_VEC4BOOL] = 4,
	[HCC_VEC4U8] = 4,
	[HCC_VEC4U16] = 8,
	[HCC_VEC4U32] = 16,
	[HCC_VEC4U64] = 32,
	[HCC_VEC4I8] = 4,
	[HCC_VEC4I16] = 8,
	[HCC_VEC4I32] = 16,
	[HCC_VEC4I64] = 32,
	[HCC_VEC4H] = 8,
	[HCC_VEC4F] = 16,
	[HCC_VEC4D] = 32,
};

U8 hcc_packed_vec_aligns[HCC_VEC_COUNT] = {
	[HCC_VEC2BOOL] = 1,
	[HCC_VEC2U8] = 1,
	[HCC_VEC2U16] = 2,
	[HCC_VEC2U32] = 4,
	[HCC_VEC2U64] = 8,
	[HCC_VEC2I8] = 1,
	[HCC_VEC2I16] = 2,
	[HCC_VEC2I32] = 4,
	[HCC_VEC2I64] = 8,
	[HCC_VEC2H] = 2,
	[HCC_VEC2F] = 4,
	[HCC_VEC2D] = 8,
	[HCC_VEC3BOOL] = 1,
	[HCC_VEC3U8] = 1,
	[HCC_VEC3U16] = 2,
	[HCC_VEC3U32] = 4,
	[HCC_VEC3U64] = 8,
	[HCC_VEC3I8] = 1,
	[HCC_VEC3I16] = 2,
	[HCC_VEC3I32] = 4,
	[HCC_VEC3I64] = 8,
	[HCC_VEC3H] = 2,
	[HCC_VEC3F] = 4,
	[HCC_VEC3D] = 8,
	[HCC_VEC4BOOL] = 1,
	[HCC_VEC4U8] = 1,
	[HCC_VEC4U16] = 2,
	[HCC_VEC4U32] = 4,
	[HCC_VEC4U64] = 8,
	[HCC_VEC4I8] = 1,
	[HCC_VEC4I16] = 2,
	[HCC_VEC4I32] = 4,
	[HCC_VEC4I64] = 8,
	[HCC_VEC4H] = 2,
	[HCC_VEC4F] = 4,
	[HCC_VEC4D] = 8,
};

U8 hcc_vec_size_and_aligns[HCC_VEC_COUNT] = {
	[HCC_VEC2BOOL] = 2,
	[HCC_VEC2U8] = 2,
	[HCC_VEC2U16] = 4,
	[HCC_VEC2U32] = 8,
	[HCC_VEC2U64] = 16,
	[HCC_VEC2I8] = 2,
	[HCC_VEC2I16] = 4,
	[HCC_VEC2I32] = 8,
	[HCC_VEC2I64] = 16,
	[HCC_VEC2H] = 4,
	[HCC_VEC2F] = 8,
	[HCC_VEC2D] = 16,
	[HCC_VEC3BOOL] = 4,
	[HCC_VEC3U8] = 4,
	[HCC_VEC3U16] = 8,
	[HCC_VEC3U32] = 16,
	[HCC_VEC3U64] = 32,
	[HCC_VEC3I8] = 4,
	[HCC_VEC3I16] = 8,
	[HCC_VEC3I32] = 16,
	[HCC_VEC3I64] = 32,
	[HCC_VEC3H] = 8,
	[HCC_VEC3F] = 16,
	[HCC_VEC3D] = 32,
	[HCC_VEC4BOOL] = 4,
	[HCC_VEC4U8] = 4,
	[HCC_VEC4U16] = 8,
	[HCC_VEC4U32] = 16,
	[HCC_VEC4U64] = 32,
	[HCC_VEC4I8] = 4,
	[HCC_VEC4I16] = 8,
	[HCC_VEC4I32] = 16,
	[HCC_VEC4I64] = 32,
	[HCC_VEC4H] = 8,
	[HCC_VEC4F] = 16,
	[HCC_VEC4D] = 32,
};

U8 hcc_packed_mat_sizes[HCC_MAT_COUNT] = {
	[HCC_MAT2X2F] = 16,
	[HCC_MAT2X2D] = 32,
	[HCC_MAT2X3F] = 24,
	[HCC_MAT2X3D] = 48,
	[HCC_MAT2X4F] = 32,
	[HCC_MAT2X4D] = 64,
	[HCC_MAT3X2F] = 24,
	[HCC_MAT3X2D] = 48,
	[HCC_MAT3X3F] = 36,
	[HCC_MAT3X3D] = 72,
	[HCC_MAT3X4F] = 48,
	[HCC_MAT3X4D] = 96,
	[HCC_MAT4X2F] = 32,
	[HCC_MAT4X2D] = 64,
	[HCC_MAT4X3F] = 48,
	[HCC_MAT4X3D] = 96,
	[HCC_MAT4X4F] = 64,
	[HCC_MAT4X4D] = 128,
};

U8 hcc_packed_mat_aligns[HCC_MAT_COUNT] = {
	[HCC_MAT2X2F] = 4,
	[HCC_MAT2X2D] = 8,
	[HCC_MAT2X3F] = 4,
	[HCC_MAT2X3D] = 8,
	[HCC_MAT2X4F] = 4,
	[HCC_MAT2X4D] = 8,
	[HCC_MAT3X2F] = 4,
	[HCC_MAT3X2D] = 8,
	[HCC_MAT3X3F] = 4,
	[HCC_MAT3X3D] = 8,
	[HCC_MAT3X4F] = 4,
	[HCC_MAT3X4D] = 8,
	[HCC_MAT4X2F] = 4,
	[HCC_MAT4X2D] = 8,
	[HCC_MAT4X3F] = 4,
	[HCC_MAT4X3D] = 8,
	[HCC_MAT4X4F] = 4,
	[HCC_MAT4X4D] = 8,
};

U8 hcc_mat_sizes[HCC_MAT_COUNT] = {
	[HCC_MAT2X2F] = 32,
	[HCC_MAT2X2D] = 64,
	[HCC_MAT2X3F] = 48,
	[HCC_MAT2X3D] = 96,
	[HCC_MAT2X4F] = 64,
	[HCC_MAT2X4D] = 128,
	[HCC_MAT3X2F] = 32,
	[HCC_MAT3X2D] = 64,
	[HCC_MAT3X3F] = 48,
	[HCC_MAT3X3D] = 96,
	[HCC_MAT3X4F] = 64,
	[HCC_MAT3X4D] = 128,
	[HCC_MAT4X2F] = 32,
	[HCC_MAT4X2D] = 64,
	[HCC_MAT4X3F] = 48,
	[HCC_MAT4X3D] = 96,
	[HCC_MAT4X4F] = 64,
	[HCC_MAT4X4D] = 128,
};

U8 hcc_mat_aligns[HCC_MAT_COUNT] = {
	[HCC_MAT2X2F] = 16,
	[HCC_MAT2X2D] = 32,
	[HCC_MAT2X3F] = 16,
	[HCC_MAT2X3D] = 32,
	[HCC_MAT2X4F] = 16,
	[HCC_MAT2X4D] = 32,
	[HCC_MAT3X2F] = 16,
	[HCC_MAT3X2D] = 32,
	[HCC_MAT3X3F] = 16,
	[HCC_MAT3X3D] = 32,
	[HCC_MAT3X4F] = 16,
	[HCC_MAT3X4D] = 32,
	[HCC_MAT4X2F] = 16,
	[HCC_MAT4X2D] = 32,
	[HCC_MAT4X3F] = 16,
	[HCC_MAT4X3D] = 32,
	[HCC_MAT4X4F] = 16,
	[HCC_MAT4X4D] = 32,
};

const char* hcc_intrinsic_function_strings[HCC_FUNCTION_IDX_INTRINSIC_END] = {
	//
	// libc-gpu/math.h
	//
	[HCC_FUNCTION_IDX_ISINF] = "__hcc_isinf",
	[HCC_FUNCTION_IDX_ISNAN] = "__hcc_isnan",
	[HCC_FUNCTION_IDX_FMODF] = "fmodf",
	[HCC_FUNCTION_IDX_FMOD] = "fmod",
	[HCC_FUNCTION_IDX_FABSF] = "fabsf",
	[HCC_FUNCTION_IDX_FABS] = "fabs",
	[HCC_FUNCTION_IDX_FLOORF] = "floorf",
	[HCC_FUNCTION_IDX_FLOOR] = "floor",
	[HCC_FUNCTION_IDX_CEILF] = "ceilf",
	[HCC_FUNCTION_IDX_CEIL] = "ceil",
	[HCC_FUNCTION_IDX_ROUNDF] = "roundf",
	[HCC_FUNCTION_IDX_ROUND] = "round",
	[HCC_FUNCTION_IDX_TRUNCF] = "truncf",
	[HCC_FUNCTION_IDX_TRUNC] = "trunc",
	[HCC_FUNCTION_IDX_SINF] = "sinf",
	[HCC_FUNCTION_IDX_SIN] = "sin",
	[HCC_FUNCTION_IDX_COSF] = "cosf",
	[HCC_FUNCTION_IDX_COS] = "cos",
	[HCC_FUNCTION_IDX_TANF] = "tanf",
	[HCC_FUNCTION_IDX_TAN] = "tan",
	[HCC_FUNCTION_IDX_ASINF] = "asinf",
	[HCC_FUNCTION_IDX_ASIN] = "asin",
	[HCC_FUNCTION_IDX_ACOSF] = "acosf",
	[HCC_FUNCTION_IDX_ACOS] = "acos",
	[HCC_FUNCTION_IDX_ATANF] = "atanf",
	[HCC_FUNCTION_IDX_ATAN] = "atan",
	[HCC_FUNCTION_IDX_SINHF] = "sinhf",
	[HCC_FUNCTION_IDX_SINH] = "sinh",
	[HCC_FUNCTION_IDX_COSHF] = "coshf",
	[HCC_FUNCTION_IDX_COSH] = "cosh",
	[HCC_FUNCTION_IDX_TANHF] = "tanhf",
	[HCC_FUNCTION_IDX_TANH] = "tanh",
	[HCC_FUNCTION_IDX_ASINHF] = "asinhf",
	[HCC_FUNCTION_IDX_ASINH] = "asinh",
	[HCC_FUNCTION_IDX_ACOSHF] = "acoshf",
	[HCC_FUNCTION_IDX_ACOSH] = "acosh",
	[HCC_FUNCTION_IDX_ATANHF] = "atanhf",
	[HCC_FUNCTION_IDX_ATANH] = "atanh",
	[HCC_FUNCTION_IDX_ATAN2F] = "atan2f",
	[HCC_FUNCTION_IDX_ATAN2] = "atan2",
	[HCC_FUNCTION_IDX_FMAF] = "fmaf",
	[HCC_FUNCTION_IDX_FMA] = "fma",
	[HCC_FUNCTION_IDX_POWF] = "powf",
	[HCC_FUNCTION_IDX_POW] = "pow",
	[HCC_FUNCTION_IDX_EXPF] = "expf",
	[HCC_FUNCTION_IDX_EXP] = "exp",
	[HCC_FUNCTION_IDX_LOGF] = "logf",
	[HCC_FUNCTION_IDX_LOG] = "log",
	[HCC_FUNCTION_IDX_EXP2F] = "exp2f",
	[HCC_FUNCTION_IDX_EXP2] = "exp2",
	[HCC_FUNCTION_IDX_LOG2F] = "log2f",
	[HCC_FUNCTION_IDX_LOG2] = "log2",
};

char* hcc_function_shader_stage_strings[HCC_FUNCTION_SHADER_STAGE_COUNT] = {
	[HCC_FUNCTION_SHADER_STAGE_NONE] = "none",
	[HCC_FUNCTION_SHADER_STAGE_VERTEX] = "vertex",
	[HCC_FUNCTION_SHADER_STAGE_FRAGMENT] = "fragment",
	[HCC_FUNCTION_SHADER_STAGE_GEOMETRY] = "geometry",
	[HCC_FUNCTION_SHADER_STAGE_TESSELLATION] = "tessellation",
	[HCC_FUNCTION_SHADER_STAGE_COMPUTE] = "compute",
	[HCC_FUNCTION_SHADER_STAGE_MESHTASK] = "meshtask",
};

HccIntrinsicTypedef hcc_intrinsic_typedefs[HCC_TYPEDEF_IDX_INTRINSIC_END] = {
	[HCC_TYPEDEF_IDX_UINT8] = { .string_id.idx_plus_one = HCC_STRING_ID_UINT8_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_UINT16] = { .string_id.idx_plus_one = HCC_STRING_ID_UINT16_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_UINT32] = { .string_id.idx_plus_one = HCC_STRING_ID_UINT32_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_UINT64] = { .string_id.idx_plus_one = HCC_STRING_ID_UINT64_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_UINTPTR] = { .string_id.idx_plus_one = HCC_STRING_ID_UINTPTR_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_INT8] = { .string_id.idx_plus_one = HCC_STRING_ID_INT8_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_INT16] = { .string_id.idx_plus_one = HCC_STRING_ID_INT16_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_INT32] = { .string_id.idx_plus_one = HCC_STRING_ID_INT32_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_INT64] = { .string_id.idx_plus_one = HCC_STRING_ID_INT64_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_INTPTR] = { .string_id.idx_plus_one = HCC_STRING_ID_INTPTR_T, .aliased_data_type = HCC_DATA_TYPE_VOID },
	[HCC_TYPEDEF_IDX_HALF] = { .string_id.idx_plus_one = HCC_STRING_ID_HALF, .aliased_data_type = HCC_DATA_TYPE_HALF },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2BOOL] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2BOOL, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2BOOL) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2I8] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2I8, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2I8) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2I16] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2I16, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2I16) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2I32] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2I32, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2I32) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2I64] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2I64, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2I64) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2U8] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2U8, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2U8) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2U16] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2U16, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2U16) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2U32] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2U32, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2U32) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2U64] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2U64, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2U64) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2H] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2H, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2H) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2F] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2F, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2F) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC2D] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC2D, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC2D) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3BOOL] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3BOOL, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3BOOL) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3I8] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3I8, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3I8) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3I16] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3I16, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3I16) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3I32] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3I32, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3I32) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3I64] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3I64, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3I64) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3U8] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3U8, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3U8) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3U16] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3U16, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3U16) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3U32] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3U32, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3U32) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3U64] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3U64, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3U64) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3H] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3H, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3H) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3F] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3F, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3F) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC3D] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC3D, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC3D) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4BOOL] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4BOOL, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4BOOL) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4I8] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4I8, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4I8) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4I16] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4I16, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4I16) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4I32] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4I32, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4I32) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4I64] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4I64, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4I64) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4U8] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4U8, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4U8) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4U16] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4U16, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4U16) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4U32] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4U32, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4U32) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4U64] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4U64, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4U64) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4H] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4H, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4H) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4F] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4F, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4F) },
	[HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC4D] = { .string_id.idx_plus_one = HCC_STRING_ID_PVEC_START + HCC_VEC4D, .aliased_data_type = HCC_DATA_TYPE_PVECTOR(HCC_VEC4D) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2BOOL] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2BOOL, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2BOOL) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2I8] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2I8, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2I8) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2I16] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2I16, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2I16) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2I32] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2I32, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2I32) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2I64] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2I64, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2I64) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2U8] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2U8, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2U8) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2U16] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2U16, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2U16) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2U32] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2U32, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2U32) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2U64] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2U64, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2U64) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2H] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2H, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2H) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2F] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2F, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2F) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC2D] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC2D, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC2D) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3BOOL] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3BOOL, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3BOOL) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3I8] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3I8, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3I8) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3I16] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3I16, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3I16) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3I32] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3I32, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3I32) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3I64] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3I64, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3I64) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3U8] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3U8, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3U8) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3U16] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3U16, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3U16) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3U32] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3U32, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3U32) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3U64] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3U64, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3U64) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3H] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3H, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3H) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3F] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3F, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3F) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC3D] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC3D, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC3D) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4BOOL] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4BOOL, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4BOOL) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4I8] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4I8, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4I8) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4I16] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4I16, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4I16) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4I32] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4I32, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4I32) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4I64] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4I64, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4I64) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4U8] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4U8, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4U8) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4U16] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4U16, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4U16) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4U32] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4U32, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4U32) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4U64] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4U64, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4U64) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4H] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4H, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4H) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4F] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4F, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4F) },
	[HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4D] = { .string_id.idx_plus_one = HCC_STRING_ID_VEC_START + HCC_VEC4D, .aliased_data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT2X2F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT2X2F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT2X2F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT2X2D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT2X2D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT2X2D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT2X3F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT2X3F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT2X3F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT2X3D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT2X3D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT2X3D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT2X4F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT2X4F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT2X4F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT2X4D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT2X4D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT2X4D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT3X2F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT3X2F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT3X2F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT3X2D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT3X2D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT3X2D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT3X3F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT3X3F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT3X3F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT3X3D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT3X3D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT3X3D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT3X4F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT3X4F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT3X4F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT3X4D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT3X4D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT3X4D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT4X2F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT4X2F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT4X2F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT4X2D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT4X2D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT4X2D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT4X3F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT4X3F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT4X3F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT4X3D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT4X3D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT4X3D) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT4X4F] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT4X4F, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT4X4F) },
	[HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT4X4D] = { .string_id.idx_plus_one = HCC_STRING_ID_PMAT_START + HCC_MAT4X4D, .aliased_data_type = HCC_DATA_TYPE_PMATRIX(HCC_MAT4X4D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT2X2F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT2X2F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT2X2F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT2X2D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT2X2D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT2X2D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT2X3F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT2X3F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT2X3F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT2X3D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT2X3D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT2X3D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT2X4F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT2X4F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT2X4F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT2X4D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT2X4D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT2X4D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT3X2F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT3X2F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT3X2F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT3X2D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT3X2D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT3X2D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT3X3F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT3X3F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT3X3F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT3X3D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT3X3D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT3X3D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT3X4F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT3X4F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT3X4F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT3X4D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT3X4D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT3X4D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT4X2F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT4X2F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT4X2F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT4X2D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT4X2D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT4X2D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT4X3F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT4X3F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT4X3F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT4X3D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT4X3D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT4X3D) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT4X4F] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT4X4F, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT4X4F) },
	[HCC_TYPEDEF_IDX_MAT_START + HCC_MAT4X4D] = { .string_id.idx_plus_one = HCC_STRING_ID_MAT_START + HCC_MAT4X4D, .aliased_data_type = HCC_DATA_TYPE_MATRIX(HCC_MAT4X4D) },
	[HCC_TYPEDEF_IDX_VERTEX_INPUT] = { .string_id.idx_plus_one = HCC_STRING_ID_HCC_VERTEX_INPUT, .aliased_data_type = HCC_DATA_TYPE_STRUCT(HCC_STRUCT_IDX_VERTEX_INPUT) },
	[HCC_TYPEDEF_IDX_FRAGMENT_INPUT] = { .string_id.idx_plus_one = HCC_STRING_ID_HCC_FRAGMENT_INPUT, .aliased_data_type = HCC_DATA_TYPE_STRUCT(HCC_STRUCT_IDX_FRAGMENT_INPUT) },
};

HccIntrinsicStruct hcc_intrinsic_structs[HCC_STRUCT_IDX_INTRINSIC_END] = {
	[HCC_STRUCT_IDX_VERTEX_INPUT] = {
		.string_id.idx_plus_one = HCC_STRING_ID_HCC_VERTEX_INPUT,
		.fields_count = 2,
		.fields = {
			[HCC_VERTEX_INPUT_VERTEX_INDEX] = { .data_type = HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_INT32), .name = hcc_string_lit("vertex_idx") },
			[HCC_VERTEX_INPUT_INSTANCE_INDEX] = { .data_type = HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_INT32), .name = hcc_string_lit("instance_idx") },
		},
	},
	[HCC_STRUCT_IDX_FRAGMENT_INPUT] = {
		.string_id.idx_plus_one = HCC_STRING_ID_HCC_FRAGMENT_INPUT,
		.fields_count = 1,
		.fields = {
			[HCC_FRAGMENT_INPUT_FRAG_COORD] = { .data_type = HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_VEC_START + HCC_VEC4F), .name = hcc_string_lit("frag_coord") },
		},
	},
	[HCC_STRUCT_IDX_HALF] = {
		.string_id.idx_plus_one = HCC_STRING_ID_HALF,
		.fields_count = 1,
		.fields = {
			[HCC_FRAGMENT_INPUT_FRAG_COORD] = { .data_type = HCC_DATA_TYPE_UINT16, .name = hcc_string_lit("_bits") },
		},
	},
};

HccIntrinsicFunction hcc_intrinsic_functions[HCC_FUNCTION_IDX_INTRINSIC_END] = {
	//
	// libc-gpu/math.h
	//
	[HCC_FUNCTION_IDX_ISINF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ISINF,
		.return_data_type = HCC_DATA_TYPE_BOOL,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_GENERIC_FLOAT },
	},
	[HCC_FUNCTION_IDX_ISNAN] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ISNAN,
		.return_data_type = HCC_DATA_TYPE_BOOL,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_GENERIC_FLOAT },
	},
	[HCC_FUNCTION_IDX_FMODF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FMODF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 2,
		.param_data_types = { HCC_DATA_TYPE_FLOAT, HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_FMOD] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FMOD,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 2,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE, HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_FABSF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FABSF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_FABS] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FABS,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_FLOORF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FLOORF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_FLOOR] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FLOOR,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_CEILF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_CEILF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_CEIL] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_CEIL,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ROUNDF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ROUNDF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ROUND] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ROUND,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_TRUNCF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_TRUNCF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_TRUNC] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_TRUNC,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_SINF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_SINF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_SIN] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_SIN,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_COSF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_COSF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_COS] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_COS,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_TANF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_TANF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_TAN] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_TAN,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ASINF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ASINF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ASIN] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ASIN,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ACOSF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ACOSF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ACOS] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ACOS,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ATANF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ATANF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ATAN] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ATAN,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_SINHF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_SINHF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_SINH] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_SINH,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_COSHF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_COSHF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_COSH] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_COSH,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_TANHF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_TANHF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_TANH] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_TANH,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ASINHF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ASINHF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ASINH] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ASINH,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ACOSHF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ACOSHF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ACOSH] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ACOSH,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ATANHF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ATANHF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ATANH] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ATANH,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_ATAN2F] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ATAN2F,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 2,
		.param_data_types = { HCC_DATA_TYPE_FLOAT, HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_ATAN2] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_ATAN2,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 2,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE, HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_FMAF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FMAF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_FMA] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_FMA,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_POWF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_POWF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_POW] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_POW,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_EXPF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_EXPF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_EXP] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_EXP,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_LOGF] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_LOGF,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_LOG] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_LOG,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_EXP2F] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_EXP2F,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_EXP2] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_EXP2,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},
	[HCC_FUNCTION_IDX_LOG2F] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_LOG2F,
		.return_data_type = HCC_DATA_TYPE_FLOAT,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_FLOAT },
	},
	[HCC_FUNCTION_IDX_LOG2] = {
		.string_id = HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_LOG2,
		.return_data_type = HCC_DATA_TYPE_DOUBLE,
		.params_count = 1,
		.param_data_types = { HCC_DATA_TYPE_DOUBLE },
	},

	//
	// libhccstd/math.h
	//
};

U8 hcc_data_type_basic_type_ranks[HCC_DATA_TYPE_BASIC_COUNT] = {
	[HCC_DATA_TYPE_BOOL] = 1,
	[HCC_DATA_TYPE_CHAR] = 2,
	[HCC_DATA_TYPE_SCHAR] = 2,
	[HCC_DATA_TYPE_UCHAR] = 2,
	[HCC_DATA_TYPE_SSHORT] = 3,
	[HCC_DATA_TYPE_USHORT] = 3,
	[HCC_DATA_TYPE_SINT] = 4,
	[HCC_DATA_TYPE_UINT] = 4,
	[HCC_DATA_TYPE_SLONG] = 5,
	[HCC_DATA_TYPE_ULONG] = 5,
	[HCC_DATA_TYPE_SLONGLONG] = 6,
	[HCC_DATA_TYPE_ULONGLONG] = 6,
	[HCC_DATA_TYPE_FLOAT] = 7,
	[HCC_DATA_TYPE_DOUBLE] = 8,
};

const char* hcc_data_type_basic_type_strings[HCC_DATA_TYPE_BASIC_COUNT] = {
	[HCC_DATA_TYPE_VOID] = "void",
	[HCC_DATA_TYPE_BOOL] = "bool",
	[HCC_DATA_TYPE_CHAR] = "char",
	[HCC_DATA_TYPE_SCHAR] = "signed char",
	[HCC_DATA_TYPE_UCHAR] = "unsigned char",
	[HCC_DATA_TYPE_SSHORT] = "signed short",
	[HCC_DATA_TYPE_USHORT] = "unsigned short",
	[HCC_DATA_TYPE_SINT] = "signed int",
	[HCC_DATA_TYPE_UINT] = "unsigned int",
	[HCC_DATA_TYPE_SLONG] = "signed long",
	[HCC_DATA_TYPE_ULONG] = "unsigned long",
	[HCC_DATA_TYPE_SLONGLONG] = "signed long long",
	[HCC_DATA_TYPE_ULONGLONG] = "unsigned long long",
	[HCC_DATA_TYPE_FLOAT] = "float",
	[HCC_DATA_TYPE_DOUBLE] = "double",
};

HccString hcc_data_type_string(HccCompiler* c, HccDataType data_type) {
	HccStringId string_id;
	bool is_const = HCC_DATA_TYPE_IS_CONST(data_type);
	data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);
	if (HCC_DATA_TYPE_IS_BASIC(data_type)) {
		return hcc_string_c((char*)hcc_data_type_basic_type_strings[data_type]);
	} else {
		char buf[1024];
		switch (data_type & 0xff) {
			case HCC_DATA_TYPE_TYPEDEF: {
				HccTypedef* typedef_ = hcc_typedef_get(c, data_type);
				return hcc_string_table_get(&c->string_table, typedef_->identifier_string_id);
			};
			case HCC_DATA_TYPE_ARRAY: {
				HccArrayDataType* d = hcc_array_data_type_get(c, data_type);
				HccString element_string = hcc_data_type_string(c, d->element_data_type);
				HccConstant constant = hcc_constant_table_get(c, d->size_constant_id);
				U64 size;
				HCC_DEBUG_ASSERT(hcc_constant_as_uint(c, constant, &size), "internal error: array size is not an unsigned integer");
				U32 string_size = snprintf(buf, sizeof(buf), "%.*s[%zu]", (int)element_string.size, element_string.data, size);
				string_id = hcc_string_table_deduplicate(&c->string_table, buf, string_size);
				break;
			};
			case HCC_DATA_TYPE_STRUCT:
			case HCC_DATA_TYPE_UNION:
			{
				char* compound_name = HCC_DATA_TYPE_IS_STRUCT(data_type) ? "struct" : "union";
				HccCompoundDataType* d = hcc_compound_data_type_get(c, data_type);
				HccString identifier = hcc_string_lit("<anonymous>");
				if (d->identifier_string_id.idx_plus_one) {
					identifier = hcc_string_table_get(&c->string_table, d->identifier_string_id);
				}
				U32 string_size = snprintf(buf, sizeof(buf), "%s(#%u) %.*s", compound_name, HCC_DATA_TYPE_IDX(data_type), (int)identifier.size, identifier.data);
				string_id = hcc_string_table_deduplicate(&c->string_table, buf, string_size);
				break;
			};
			case HCC_DATA_TYPE_ENUM:
			{
				HccEnumDataType* d = hcc_enum_data_type_get(c, data_type);
				HccString identifier = hcc_string_lit("<anonymous>");
				if (d->identifier_string_id.idx_plus_one) {
					identifier = hcc_string_table_get(&c->string_table, d->identifier_string_id);
				}
				U32 string_size = snprintf(buf, sizeof(buf), "enum(#%u) %.*s", HCC_DATA_TYPE_IDX(data_type), (int)identifier.size, identifier.data);
				string_id = hcc_string_table_deduplicate(&c->string_table, buf, string_size);
				break;
			};
			case HCC_DATA_TYPE_GENERIC_FLOAT:
				return hcc_string_lit("__hcc_generic_float");
			default:
				HCC_ABORT("unhandled data type '%u'", data_type);
		}
	}

	if (is_const) {
		char buf[1024];
		HccString data_type_string = hcc_string_table_get(&c->string_table, string_id);
		U32 string_size = snprintf(buf, sizeof(buf), "const %.*s", (int)data_type_string.size, data_type_string.data);
		string_id = hcc_string_table_deduplicate(&c->string_table, buf, string_size);
	}

	return hcc_string_table_get(&c->string_table, string_id);
}

void hcc_data_type_size_align(HccCompiler* c, HccDataType data_type, U64* size_out, U64* align_out) {
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);

	if (HCC_DATA_TYPE_IS_BASIC(data_type)) {
		*size_out = c->basic_type_size_and_aligns[data_type];
		*align_out = c->basic_type_size_and_aligns[data_type];
	} else {
		switch (data_type & 0xff) {
			case HCC_DATA_TYPE_STRUCT:
			case HCC_DATA_TYPE_UNION: {
				HccCompoundDataType* d = hcc_compound_data_type_get(c, data_type);
				*size_out = d->size;
				*align_out = d->align;
				break;
			};
			case HCC_DATA_TYPE_ARRAY: {
				HccArrayDataType* d = hcc_array_data_type_get(c, data_type);
				HccConstant constant = hcc_constant_table_get(c, d->size_constant_id);
				U64 count;
				hcc_constant_as_uint(c, constant, &count);

				U64 size;
				U64 align;
				hcc_data_type_size_align(c, d->element_data_type, &size, &align);

				*size_out = size * count;
				*align_out = align;
				break;
			};
			default:
				HCC_ABORT("unhandled data type '%u'", data_type);
		}
	}
}

void hcc_data_type_print_basic(HccCompiler* c, HccDataType data_type, void* data, FILE* f) {
	HCC_DEBUG_ASSERT(data_type < HCC_DATA_TYPE_BASIC_END, "internal error: expected a basic data type but got '%s'", hcc_data_type_string(c, data_type));

	U64 uint;
	S64 sint;
	F64 float_;
	switch (data_type) {
		case HCC_DATA_TYPE_VOID:
			fprintf(f, "void");
			break;
		case HCC_DATA_TYPE_BOOL:
			fprintf(f, *(U8*)data ? "true" : "false");
			break;
		case HCC_DATA_TYPE_CHAR:
		case HCC_DATA_TYPE_SCHAR:
		case HCC_DATA_TYPE_SSHORT:
		case HCC_DATA_TYPE_SINT:
		case HCC_DATA_TYPE_SLONG:
		case HCC_DATA_TYPE_SLONGLONG:
			switch (c->basic_type_size_and_aligns[data_type]) {
				case sizeof(int8_t): uint = *(int8_t*)data; break;
				case sizeof(int16_t): uint = *(int16_t*)data; break;
				case sizeof(int32_t): uint = *(int32_t*)data; break;
				case sizeof(int64_t): uint = *(int64_t*)data; break;
			}
			fprintf(f, "%zd", uint);
			break;
		case HCC_DATA_TYPE_UCHAR:
		case HCC_DATA_TYPE_USHORT:
		case HCC_DATA_TYPE_UINT:
		case HCC_DATA_TYPE_ULONG:
		case HCC_DATA_TYPE_ULONGLONG:
			switch (c->basic_type_size_and_aligns[data_type]) {
				case sizeof(uint8_t): uint = *(uint8_t*)data; break;
				case sizeof(uint16_t): uint = *(uint16_t*)data; break;
				case sizeof(uint32_t): uint = *(uint32_t*)data; break;
				case sizeof(uint64_t): uint = *(uint64_t*)data; break;
			}
			fprintf(f, "%zu", uint);
			break;
		case HCC_DATA_TYPE_FLOAT: float_ = *(float*)data; goto FLOAT;
		case HCC_DATA_TYPE_DOUBLE: float_ = *(double*)data; goto FLOAT;
FLOAT:
			fprintf(f, "%f", float_);
			break;
	}
}

HccDataType hcc_data_type_unsigned_to_signed(HccCompiler* c, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_BASIC(data_type)) {
		HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_UINT(c, data_type), "data_type must be a unsigned integer");
		return data_type + 5;
	} else if (HCC_DATA_TYPE_IS_VECTOR(data_type)) {
		HccDataType comp_data_type = hcc_vector_data_type_scalar(c, data_type);
		HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_UINT(c, data_type), "component data type must be a unsigned integer");
		return data_type + HCC_DATA_TYPE_INIT(0, 5);
	} else {
		HCC_ABORT("data_type must be a basic or vector type");
	}
}

HccDataType hcc_data_type_signed_to_unsigned(HccCompiler* c, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_BASIC(data_type)) {
		HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_SINT(c, data_type), "data_type must be a signed integer");
		return data_type - 5;
	} else if (HCC_DATA_TYPE_IS_VECTOR(data_type)) {
		HccDataType comp_data_type = hcc_vector_data_type_scalar(c, data_type);
		HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_SINT(c, data_type), "component data type must be a signed integer");
		return data_type - HCC_DATA_TYPE_INIT(0, 5);
	} else {
		HCC_ABORT("data_type must be a basic or vector type");
	}
}

bool hcc_data_type_is_condition(HccCompiler* c, HccDataType data_type) {
	HCC_UNUSED(c); // unused param for now, we will have to get pointers working later with this
	data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);
	return HCC_DATA_TYPE_BOOL <= data_type && data_type < HCC_DATA_TYPE_BASIC_END;
}

U32 hcc_data_type_composite_fields_count(HccCompiler* c, HccDataType data_type) {
	HCC_DEBUG_ASSERT(!HCC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a composite type but got '%s'", hcc_data_type_string(c, data_type));

	switch (data_type & 0xff) {
		case HCC_DATA_TYPE_ARRAY: {
			HccArrayDataType* d = hcc_array_data_type_get(c, data_type);
			HccConstant constant = hcc_constant_table_get(c, d->size_constant_id);
			U64 count;
			hcc_constant_as_uint(c, constant, &count);
			return count;
		};
		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION: {
			HccCompoundDataType* d = hcc_compound_data_type_get(c, data_type);
			return d->fields_count;
		};
		default:
			HCC_ABORT("unhandled data type '%u'", data_type);
	}
}

bool hcc_data_type_is_rasterizer_state(HccCompiler* c, HccDataType data_type) {
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
	return HCC_DATA_TYPE_IS_STRUCT(data_type) && (hcc_compound_data_type_get(c, data_type)->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_RASTERIZER_STATE);
}

bool hcc_data_type_is_fragment_state(HccCompiler* c, HccDataType data_type) {
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
	return HCC_DATA_TYPE_IS_STRUCT(data_type) && (hcc_compound_data_type_get(c, data_type)->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_FRAGMENT_STATE);
}

HccDataType hcc_data_type_from_intrinsic_type(HccCompiler* c, HccIntrinsicType intrinsic_type) {
	if (HCC_INTRINSIC_TYPE_VECTOR_START <= intrinsic_type && intrinsic_type < HCC_INTRINSIC_TYPE_VECTOR_END) {
		HccVec vec = intrinsic_type - HCC_INTRINSIC_TYPE_VECTOR_START;
		return HCC_DATA_TYPE_VECTOR(vec);
	} else if (HCC_INTRINSIC_TYPE_MATRIX_START <= intrinsic_type && intrinsic_type < HCC_INTRINSIC_TYPE_MATRIX_END) {
		HccMat mat = intrinsic_type - HCC_INTRINSIC_TYPE_MATRIX_START;
		return HCC_DATA_TYPE_MATRIX(mat);
	}

	U8 size;
	switch (intrinsic_type) {
		case HCC_INTRINSIC_TYPE_VOID: return HCC_DATA_TYPE_VOID;
		case HCC_INTRINSIC_TYPE_BOOL: return HCC_DATA_TYPE_BOOL;
		case HCC_INTRINSIC_TYPE_S8: size = sizeof(U8); goto FIND_SIGNED;
		case HCC_INTRINSIC_TYPE_S16: size = sizeof(U16); goto FIND_SIGNED;
		case HCC_INTRINSIC_TYPE_S32: size = sizeof(U32); goto FIND_SIGNED;
		case HCC_INTRINSIC_TYPE_S64: size = sizeof(U64); goto FIND_SIGNED;
FIND_SIGNED:
			for (HccDataType dt = HCC_DATA_TYPE_SCHAR; dt <= HCC_DATA_TYPE_SLONGLONG; dt += 1) {
				if (c->basic_type_size_and_aligns[dt] == size) {
					return dt;
				}
			}
			break;
		case HCC_INTRINSIC_TYPE_U8: size = sizeof(U8); goto FIND_UNSIGNED;
		case HCC_INTRINSIC_TYPE_U16: size = sizeof(U16); goto FIND_UNSIGNED;
		case HCC_INTRINSIC_TYPE_U32: size = sizeof(U32); goto FIND_UNSIGNED;
		case HCC_INTRINSIC_TYPE_U64: size = sizeof(U64); goto FIND_UNSIGNED;
FIND_UNSIGNED:
			for (HccDataType dt = HCC_DATA_TYPE_UCHAR; dt <= HCC_DATA_TYPE_ULONGLONG; dt += 1) {
				if (c->basic_type_size_and_aligns[dt] == size) {
					return dt;
				}
			}
			break;
		case HCC_INTRINSIC_TYPE_F16: return HCC_DATA_TYPE_STRUCT(HCC_STRUCT_IDX_HALF);
		case HCC_INTRINSIC_TYPE_F32: return HCC_DATA_TYPE_FLOAT;
		case HCC_INTRINSIC_TYPE_F64: return HCC_DATA_TYPE_DOUBLE;
	}

	HCC_ABORT("unhandled intrinsic type '%u'", intrinsic_type);
}

HccIntrinsicBasicTypeMask hcc_data_type_has_intrinsic_basic_types(HccCompiler* c, HccDataType data_type) {
	HccIntrinsicBasicTypeMask mask = 0;
	if (HCC_DATA_TYPE_IS_INTRINSIC_BASIC(data_type)) {
		HCC_INTRINSIC_BASIC_TYPE_MASK_SET(&mask, hcc_intrinsic_type_from_data_type(c, data_type));
		return mask;
	}

	switch (data_type & 0xff) {
		case HCC_DATA_TYPE_ENUM:
			HCC_INTRINSIC_BASIC_TYPE_MASK_SET(&mask, hcc_intrinsic_type_from_data_type(c, HCC_DATA_TYPE_SINT));
			return mask;

		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION: {
			HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, data_type);
			return compound_data_type->has_intrinsic_basic_types;
		};

		case HCC_DATA_TYPE_ARRAY: {
			HccArrayDataType* array_data_type = hcc_array_data_type_get(c, data_type);
			return hcc_data_type_has_intrinsic_basic_types(c, array_data_type->element_data_type);
		};

		case HCC_DATA_TYPE_TYPEDEF: {
			HccTypedef* typedef_ = hcc_typedef_get(c, data_type);
			return hcc_data_type_has_intrinsic_basic_types(c, typedef_->aliased_data_type);
		};

		default:
			return 0;
	}
}

bool hcc_data_type_is_same_underlaying_type(HccCompiler* c, HccDataType a, HccDataType b) {
	a = hcc_typedef_resolve_and_strip_const(c, a);
	b = hcc_typedef_resolve_and_strip_const(c, b);
	if (a == b) {
		return true;
	}

	if (HCC_DATA_TYPE_IS_INTRINSIC_BASIC(a) && HCC_DATA_TYPE_IS_INTRINSIC_BASIC(b)) {
		HccIntrinsicType ia = hcc_intrinsic_type_from_data_type(c, a);
		HccIntrinsicType ib = hcc_intrinsic_type_from_data_type(c, b);
		return ia == ib;
	}

	return false;
}

bool hcc_data_type_is_same_bitwidth_int(HccCompiler* c, HccDataType a, HccDataType b) {
	a = hcc_typedef_resolve_and_strip_const(c, a);
	b = hcc_typedef_resolve_and_strip_const(c, b);
	if (HCC_DATA_TYPE_IS_INTRINSIC_BASIC(a) && HCC_DATA_TYPE_IS_INTRINSIC_BASIC(b)) {
		HccIntrinsicType ia = hcc_intrinsic_type_from_data_type(c, a);
		HccIntrinsicType ib = hcc_intrinsic_type_from_data_type(c, b);
		return ia == ib || ia + 4 == ib || ia == ib + 4;
	}

	return false;
}

U32 hcc_data_type_token_idx(HccCompiler* c, HccDataType data_type) {
	switch (data_type & 0xff) {
		case HCC_DATA_TYPE_TYPEDEF:
			return hcc_typedef_get(c, data_type)->identifier_token_idx;
		default:
			return -1;
	}
}

HccIntrinsicType hcc_intrinsic_type_from_data_type(HccCompiler* c, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_UINT(c, data_type)) {
		switch (c->basic_type_size_and_aligns[data_type]) {
			case sizeof(U8): return HCC_INTRINSIC_TYPE_U8;
			case sizeof(U16): return HCC_INTRINSIC_TYPE_U16;
			case sizeof(U32): return HCC_INTRINSIC_TYPE_U32;
			case sizeof(U64): return HCC_INTRINSIC_TYPE_U64;
		}
	} else if (HCC_DATA_TYPE_IS_SINT(c, data_type)) {
		switch (c->basic_type_size_and_aligns[data_type]) {
			case sizeof(U8): return HCC_INTRINSIC_TYPE_S8;
			case sizeof(U16): return HCC_INTRINSIC_TYPE_S16;
			case sizeof(U32): return HCC_INTRINSIC_TYPE_S32;
			case sizeof(U64): return HCC_INTRINSIC_TYPE_S64;
		}
	} else if (HCC_DATA_TYPE_IS_VECTOR(data_type)) {
		HccVec vec = HCC_DATA_TYPE_VECTOR_VEC(data_type);
		return HCC_INTRINSIC_TYPE_VECTOR_START + vec;
	} else if (HCC_DATA_TYPE_IS_MATRIX(data_type)) {
		HccMat mat = HCC_DATA_TYPE_MATRIX_MAT(data_type);
		return HCC_INTRINSIC_TYPE_MATRIX_START + mat;
	} else {
		switch (data_type) {
			case HCC_DATA_TYPE_VOID: return HCC_INTRINSIC_TYPE_VOID;
			case HCC_DATA_TYPE_BOOL: return HCC_INTRINSIC_TYPE_BOOL;
			case HCC_DATA_TYPE_HALF: return HCC_INTRINSIC_TYPE_F16;
			case HCC_DATA_TYPE_FLOAT: return HCC_INTRINSIC_TYPE_F32;
			case HCC_DATA_TYPE_DOUBLE: return HCC_INTRINSIC_TYPE_F64;
		}
	}

	HCC_ABORT("unhandled data type '%u'", data_type);
}

HccDataType hcc_vector_data_type_scalar(HccCompiler* c, HccDataType data_type) {
	HccVec vec = HCC_DATA_TYPE_VECTOR_VEC(data_type);
	HccIntrinsicType scalar_intrinsic_type = HCC_INTRINSIC_TYPE_FROM_VEC(vec);
	return hcc_data_type_from_intrinsic_type(c, scalar_intrinsic_type);
}

HccDataType hcc_matrix_data_type_scalar(HccCompiler* c, HccDataType data_type) {
	HccMat mat = HCC_DATA_TYPE_MATRIX_MAT(data_type);
	HccIntrinsicType scalar_intrinsic_type = HCC_INTRINSIC_TYPE_FROM_MAT(mat);
	return hcc_data_type_from_intrinsic_type(c, scalar_intrinsic_type);
}

HccDataType hcc_matrix_data_type_column_vector(HccDataType data_type) {
	HccMat mat = HCC_DATA_TYPE_MATRIX_MAT(data_type);
	U32 columns_count_minus_two = mat % 6;
	HccVec vec = (columns_count_minus_two * 12) + (mat % 2);
	return HCC_DATA_TYPE_VECTOR(vec);
}

HccString hcc_intrinsic_basic_type_mask_string(HccIntrinsicBasicTypeMask mask) {
	static char buf[512];
	U32 size = 0;
	while (mask) {
		HccIntrinsicType intrinsic_type = leastsetbitidx32(mask);
		const char* str;
		switch (intrinsic_type) {
			case HCC_INTRINSIC_TYPE_VOID: str = "void"; break;
			case HCC_INTRINSIC_TYPE_BOOL: str = "bool"; break;
			case HCC_INTRINSIC_TYPE_S8: str = "int8_t"; break;
			case HCC_INTRINSIC_TYPE_S16: str = "int16_t"; break;
			case HCC_INTRINSIC_TYPE_S32: str = "int32_t"; break;
			case HCC_INTRINSIC_TYPE_S64: str = "int64_t"; break;
			case HCC_INTRINSIC_TYPE_U8: str = "uint8_t"; break;
			case HCC_INTRINSIC_TYPE_U16: str = "uint16_t"; break;
			case HCC_INTRINSIC_TYPE_U32: str = "uint32_t"; break;
			case HCC_INTRINSIC_TYPE_U64: str = "uint64_t"; break;
			case HCC_INTRINSIC_TYPE_F16: str = "half"; break;
			case HCC_INTRINSIC_TYPE_F32: str = "float"; break;
			case HCC_INTRINSIC_TYPE_F64: str = "double"; break;
		}

		size += snprintf(buf + size, sizeof(buf) - size, size ? " %s" : "%s", str);
		mask &= ~(1 << intrinsic_type);
	}

	return hcc_string(buf, size);
}

HccBasicTypeClass hcc_basic_type_class(HccCompiler* c, HccDataType data_type) {
	switch (data_type) {
		case HCC_DATA_TYPE_BOOL: return HCC_BASIC_TYPE_CLASS_BOOL;
		case HCC_DATA_TYPE_CHAR:
		case HCC_DATA_TYPE_SCHAR:
		case HCC_DATA_TYPE_SSHORT:
		case HCC_DATA_TYPE_SINT:
		case HCC_DATA_TYPE_SLONG:
		case HCC_DATA_TYPE_SLONGLONG: return HCC_BASIC_TYPE_CLASS_SINT;
		case HCC_DATA_TYPE_UCHAR:
		case HCC_DATA_TYPE_USHORT:
		case HCC_DATA_TYPE_UINT:
		case HCC_DATA_TYPE_ULONG:
		case HCC_DATA_TYPE_ULONGLONG: return HCC_BASIC_TYPE_CLASS_UINT;
		case HCC_DATA_TYPE_FLOAT:
		case HCC_DATA_TYPE_DOUBLE: return HCC_BASIC_TYPE_CLASS_FLOAT;
		default: {
			HccString data_type_name = hcc_data_type_string(c, data_type);
			HCC_UNREACHABLE("internal error: expected a basic type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
		};
	}
}

HccBasic hcc_basic_from_sint(HccCompiler* c, HccDataType data_type, S64 value) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_BASIC(data_type), "data type must be a basic type");

	HccBasic basic;
	switch (data_type) {
	case HCC_DATA_TYPE_CHAR:
	case HCC_DATA_TYPE_SCHAR:
	case HCC_DATA_TYPE_SSHORT:
	case HCC_DATA_TYPE_SINT:
	case HCC_DATA_TYPE_SLONG:
	case HCC_DATA_TYPE_SLONGLONG:
		switch (c->basic_type_size_and_aligns[data_type]) {
		case sizeof(int8_t): basic.s8 = value; break;
		case sizeof(int16_t): basic.s16 = value; break;
		case sizeof(int32_t): basic.s32 = value; break;
		case sizeof(int64_t): basic.s64 = value; break;
		}
		break;
	case HCC_DATA_TYPE_BOOL:
	case HCC_DATA_TYPE_UCHAR:
	case HCC_DATA_TYPE_USHORT:
	case HCC_DATA_TYPE_UINT:
	case HCC_DATA_TYPE_ULONG:
	case HCC_DATA_TYPE_ULONGLONG:
		switch (c->basic_type_size_and_aligns[data_type]) {
		case sizeof(uint8_t): basic.u8 = value; break;
		case sizeof(uint16_t): basic.u16 = value; break;
		case sizeof(uint32_t): basic.u32 = value; break;
		case sizeof(uint64_t): basic.u64 = value; break;
		}
		break;
	case HCC_DATA_TYPE_FLOAT: basic.f = value; break;
	case HCC_DATA_TYPE_DOUBLE: basic.d = value; break;
	}

	return basic;
}

HccBasic hcc_basic_from_uint(HccCompiler* c, HccDataType data_type, U64 value) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_BASIC(data_type), "data type must be a basic type");

	HccBasic basic;
	switch (data_type) {
	case HCC_DATA_TYPE_BOOL:
	case HCC_DATA_TYPE_CHAR:
	case HCC_DATA_TYPE_SCHAR:
	case HCC_DATA_TYPE_SSHORT:
	case HCC_DATA_TYPE_SINT:
	case HCC_DATA_TYPE_SLONG:
	case HCC_DATA_TYPE_SLONGLONG:
	case HCC_DATA_TYPE_UCHAR:
	case HCC_DATA_TYPE_USHORT:
	case HCC_DATA_TYPE_UINT:
	case HCC_DATA_TYPE_ULONG:
	case HCC_DATA_TYPE_ULONGLONG:
		switch (c->basic_type_size_and_aligns[data_type]) {
		case sizeof(uint8_t): basic.u8 = value; break;
		case sizeof(uint16_t): basic.u16 = value; break;
		case sizeof(uint32_t): basic.u32 = value; break;
		case sizeof(uint64_t): basic.u64 = value; break;
		}
		break;
	case HCC_DATA_TYPE_FLOAT: basic.f = value; break;
	case HCC_DATA_TYPE_DOUBLE: basic.d = value; break;
	}

	return basic;
}

HccBasic hcc_basic_from_float(HccCompiler* c, HccDataType data_type, double value) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_BASIC(data_type), "data type must be a basic type");

	HccBasic basic;
	switch (data_type) {
	case HCC_DATA_TYPE_CHAR:
	case HCC_DATA_TYPE_SCHAR:
	case HCC_DATA_TYPE_SSHORT:
	case HCC_DATA_TYPE_SINT:
	case HCC_DATA_TYPE_SLONG:
	case HCC_DATA_TYPE_SLONGLONG:
		switch (c->basic_type_size_and_aligns[data_type]) {
		case sizeof(int8_t): basic.s8 = (S8)value; break;
		case sizeof(int16_t): basic.s16 = (S16)value; break;
		case sizeof(int32_t): basic.s32 = (S32)value; break;
		case sizeof(int64_t): basic.s64 = (S64)value; break;
		}
		break;
	case HCC_DATA_TYPE_BOOL:
	case HCC_DATA_TYPE_UCHAR:
	case HCC_DATA_TYPE_USHORT:
	case HCC_DATA_TYPE_UINT:
	case HCC_DATA_TYPE_ULONG:
	case HCC_DATA_TYPE_ULONGLONG:
		switch (c->basic_type_size_and_aligns[data_type]) {
		case sizeof(uint8_t): basic.u8 = (U8)value; break;
		case sizeof(uint16_t): basic.u16 = (U16)value; break;
		case sizeof(uint32_t): basic.u32 = (U32)value; break;
		case sizeof(uint64_t): basic.u64 = (U64)value; break;
		}
		break;
	case HCC_DATA_TYPE_FLOAT: basic.f = value; break;
	case HCC_DATA_TYPE_DOUBLE: basic.d = value; break;
	}

	return basic;
}

HccArrayDataType* hcc_array_data_type_get(HccCompiler* c, HccDataType data_type) {
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_ARRAY(data_type), "internal error: expected array data type");
	return hcc_stack_get(c->astgen.array_data_types, HCC_DATA_TYPE_IDX(data_type));
}

HccEnumDataType* hcc_enum_data_type_get(HccCompiler* c, HccDataType data_type) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_ENUM_TYPE(data_type), "internal error: expected enum data type");
	return hcc_stack_get(c->astgen.enum_data_types, HCC_DATA_TYPE_IDX(data_type));
}

HccCompoundDataType* hcc_compound_data_type_get(HccCompiler* c, HccDataType data_type) {
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_COMPOUND_TYPE(data_type), "internal error: expected compound data type");
	return hcc_stack_get(c->astgen.compound_data_types, HCC_DATA_TYPE_IDX(data_type));
}

HccTypedef* hcc_typedef_get(HccCompiler* c, HccDataType data_type) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_TYPEDEF(data_type), "internal error: expected typedef");
	return hcc_stack_get(c->astgen.typedefs, HCC_DATA_TYPE_IDX(data_type));
}

HccDataType hcc_typedef_resolve_and_keep_const(HccCompiler* c, HccDataType data_type) {
	U32 const_mask = data_type & HCC_DATA_TYPE_CONST_MASK;
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
	return data_type | const_mask;
}

HccDataType hcc_typedef_resolve_and_strip_const(HccCompiler* c, HccDataType data_type) {
	data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);
	while (1) {
		switch (data_type & 0xff) {
			case HCC_DATA_TYPE_ENUM:
				return HCC_DATA_TYPE_SINT;
			case HCC_DATA_TYPE_TYPEDEF: {
				HccTypedef* typedef_ = hcc_typedef_get(c, data_type);
				data_type = typedef_->aliased_data_type;
				break;
			};
			default:
				return data_type;
		}
	}
}

U32 hcc_decl_token_idx(HccCompiler* c, HccDecl decl) {
	switch (decl & 0xff) {
		case HCC_DECL_FUNCTION:
			return hcc_function_get(c, decl)->identifier_token_idx;
		case HCC_DECL_ENUM_VALUE:
			return hcc_enum_value_get(c, decl)->identifier_token_idx;
		default:
			if (HCC_DECL_IS_DATA_TYPE(decl)) {
				return hcc_data_type_token_idx(c, (HccDataType)decl);
			}
			return -1;
	}
}

HccFunction* hcc_function_get(HccCompiler* c, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_FUNCTION(decl), "internal error: expected a function declaration");
	return hcc_stack_get(c->astgen.functions, HCC_DECL_IDX(decl));
}

HccEnumValue* hcc_enum_value_get(HccCompiler* c, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_ENUM_VALUE(decl), "internal error: expected a enum value");
	return hcc_stack_get(c->astgen.enum_values, HCC_DECL_IDX(decl));
}

HccVariable* hcc_global_variable_get(HccCompiler* c, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_GLOBAL_VARIABLE(decl), "internal error: expected a global variable");
	return hcc_stack_get(c->astgen.global_variables, HCC_DECL_IDX(decl));
}

U32 hcc_variable_to_string(HccCompiler* c, HccVariable* variable, char* buf, U32 buf_size, bool color) {
	char* fmt;
	if (color) {
		fmt = "\x1b[1;95m%s\x1b[1;94m%.*s \x1b[97m%.*s\x1b[0m";
	} else {
		fmt = "%s%.*s %.*s";
	}

	char* specifiers;
	if (variable->is_static) {
		specifiers = "static ";
	} else {
		// no need to handle const since hcc_data_type_string does this at a type level
		specifiers = "";
	}
	HccString type_name = hcc_data_type_string(c, variable->data_type);
	HccString variable_name = hcc_string_table_get(&c->string_table, variable->identifier_string_id);
	return snprintf(buf, buf_size, fmt, specifiers, (int)type_name.size, type_name.data, (int)variable_name.size, variable_name.data);
}

U32 hcc_function_to_string(HccCompiler* c, HccFunction* function, char* buf, U32 buf_size, bool color) {
	char* function_fmt;
	if (color) {
		function_fmt = "\x1b[1;94m%.*s \x1b[97m%.*s\x1b[0m";
	} else {
		function_fmt = "%.*s %.*s";
	}

	HccString return_type_name = hcc_data_type_string(c, function->return_data_type);
	HccString name = hcc_string_table_get(&c->string_table, function->identifier_string_id);
	U32 cursor = 0;
	cursor += snprintf(buf + cursor, buf_size - cursor, function_fmt, (int)return_type_name.size, return_type_name.data, (int)name.size, name.data);
	cursor += snprintf(buf + cursor, buf_size - cursor, "(");
	for (U32 param_idx = 0; param_idx < function->params_count; param_idx += 1) {
		HccVariable* param = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + param_idx);
		cursor += hcc_variable_to_string(c, param, buf + cursor, buf_size - cursor, color);
		if (param_idx + 1 < function->params_count) {
			cursor += snprintf(buf + cursor, buf_size - cursor, ", ");
		}
	}
	cursor += snprintf(buf + cursor, buf_size - cursor, ")");
	return cursor;
}

// ===========================================
//
//
// Token Generator
//
//
// ===========================================

void hcc_tokengen_init(HccCompiler* c, HccCompilerSetup* setup) {
	c->tokengen.token_bag.tokens = hcc_stack_init(HccToken, setup->tokengen.tokens_cap, HCC_ALLOC_TAG_TOKENGEN_TOKENS);
	c->tokengen.token_bag.token_location_indices = hcc_stack_init(U32, setup->tokengen.tokens_cap, HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATION_INDICES);
	c->tokengen.token_bag.token_values = hcc_stack_init(HccTokenValue, setup->tokengen.token_values_cap, HCC_ALLOC_TAG_TOKENGEN_TOKEN_VALUES);
	c->tokengen.token_locations = hcc_stack_init(HccLocation, setup->tokengen.token_locations_cap, HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATION_INDICES);
	c->tokengen.paused_file_stack = hcc_stack_init(HccPausedFile, setup->tokengen.paused_file_stack_cap, HCC_ALLOC_TAG_TOKENGEN_PAUSED_FILE_STACK);
	c->tokengen.open_bracket_stack = hcc_stack_init(HccOpenBracket, setup->tokengen.open_bracket_stack_cap, HCC_ALLOC_TAG_TOKENGEN_OPEN_BRACKET_STACK);
}

void hcc_tokengen_advance_column(HccCompiler* c, U32 by) {
	c->tokengen.location.column_end += by;
	c->tokengen.location.code_end_idx += by;
}

void hcc_tokengen_advance_newline(HccCompiler* c) {
	c->tokengen.location.line_end += 1;
	c->tokengen.location.column_start = 1;
	c->tokengen.location.column_end = 1;
	c->tokengen.location.code_end_idx += 1;

	U32* dst = hcc_stack_push(c->tokengen.location.code_file->line_code_start_indices);
	*dst = c->tokengen.location.code_end_idx;
}

U32 hcc_tokengen_display_line(HccCompiler* c) {
	U32 line_num = c->tokengen.location.line_start;
	return c->tokengen.custom_line_dst ? c->tokengen.custom_line_dst + (line_num - c->tokengen.custom_line_src) : line_num;
}

U32 hcc_tokengen_token_add(HccCompiler* c, HccToken token) {
	U32 token_idx = hcc_stack_count(c->tokengen.dst_token_bag->tokens);
	*hcc_stack_push(c->tokengen.dst_token_bag->tokens) = token;

	*hcc_stack_push(c->tokengen.dst_token_bag->token_location_indices) = hcc_stack_count(c->tokengen.token_locations);
	HccLocation* dst_location = hcc_stack_push(c->tokengen.token_locations);
	*dst_location = c->tokengen.location;
	dst_location->display_line = hcc_tokengen_display_line(c);

	return token_idx;
}

U32 hcc_tokengen_token_value_add(HccCompiler* c, HccTokenValue value) {
	U32 token_value_idx = hcc_stack_count(c->tokengen.dst_token_bag->token_values);
	*hcc_stack_push(c->tokengen.dst_token_bag->token_values) = value;

	return token_value_idx;
}

void hcc_tokengen_count_extra_newlines(HccCompiler* c) {
	HccLocation* location = &c->tokengen.location;
	HccString code = location->code_file->code;
	U32 idx = hcc_stack_count(c->tokengen.paused_file_stack);
	while (1) {
		U32 lines_count = 3;
		while (location->code_end_idx < code.size) {
			char byte = code.data[location->code_end_idx];
			location->code_end_idx += 1;
			if (byte == '\n') {
				U32* dst = hcc_stack_push(location->code_file->line_code_start_indices);
				*dst = location->code_end_idx;

				lines_count -= 1;
				if (lines_count == 0) {
					break;
				}
			}
		}

		if (idx == 0) {
			break;
		}

		idx -= 1;
		location = &hcc_stack_get(c->tokengen.paused_file_stack, idx)->location;
		code = location->code_file->code;
	}
}

noreturn void hcc_tokengen_bail_error_1(HccCompiler* c, HccErrorCode error_code, ...) {
	hcc_tokengen_count_extra_newlines(c);

	va_list va_args;
	va_start(va_args, error_code);
	c->tokengen.location.display_line = hcc_tokengen_display_line(c);
	hcc_error_pushv(c, error_code, &c->tokengen.location, NULL, va_args);
	va_end(va_args);

	//
	// TODO see if you can recover from an error at the tokengen stage.
	// I don't think it is worth it as i don't want to pump out any incorrect errors.
	hcc_compiler_bail(c);
}

noreturn void hcc_tokengen_bail_error_2_idx(HccCompiler* c, HccErrorCode error_code, U32 other_token_idx, ...) {
	hcc_tokengen_count_extra_newlines(c);

	HccLocation* other_token_location = other_token_idx == U32_MAX ? NULL : hcc_token_bag_location_get(c, &c->tokengen.token_bag, other_token_idx);

	va_list va_args;
	va_start(va_args, other_token_idx);
	c->tokengen.location.display_line = hcc_tokengen_display_line(c);
	hcc_error_pushv(c, error_code, &c->tokengen.location, other_token_location, va_args);
	va_end(va_args);

	//
	// TODO see if you can recover from an error at the tokengen stage.
	// I don't think it is worth it as i don't want to pump out any incorrect errors.
	hcc_compiler_bail(c);
}

noreturn void hcc_tokengen_bail_error_2_ptr(HccCompiler* c, HccErrorCode error_code, HccLocation* token_location, HccLocation* other_token_location, ...) {
	hcc_tokengen_count_extra_newlines(c);

	va_list va_args;
	va_start(va_args, other_token_location);
	c->tokengen.location.display_line = hcc_tokengen_display_line(c);
	hcc_error_pushv(c, error_code, token_location, other_token_location, va_args);
	va_end(va_args);

	//
	// TODO see if you can recover from an error at the tokengen stage.
	// I don't think it is worth it as i don't want to pump out any incorrect errors.
	hcc_compiler_bail(c);
}

void hcc_tokengen_location_push(HccCompiler* c) {
	HccPausedFile* paused_file = hcc_stack_push(c->tokengen.paused_file_stack);
	paused_file->location = c->tokengen.location;
	paused_file->if_span_stack_count = hcc_stack_count(c->pp.if_span_stack);
}

void hcc_tokengen_location_pop(HccCompiler* c) {
	c->tokengen.location.code_file->flags |= HCC_CODE_FILE_FLAGS_PARSED_ONCE_OR_MORE;
	HccPausedFile* paused_file = hcc_stack_get_last(c->tokengen.paused_file_stack);
	if (paused_file->if_span_stack_count != hcc_stack_count(c->pp.if_span_stack)) {
		HccPPIfSpan* if_span = *hcc_stack_get_last(c->pp.if_span_stack);
		c->tokengen.location = if_span->location;
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_PP_IF_UNTERMINATED, hcc_pp_directive_strings[if_span->directive]);
	}

	hcc_change_working_directory_to_same_as_this_file(paused_file->location.code_file->path_string.data);
	c->tokengen.location = paused_file->location;
	c->tokengen.code = paused_file->location.code_file->code.data;
	c->tokengen.code_size = paused_file->location.code_file->code.size;
	hcc_stack_pop(c->tokengen.paused_file_stack);
}

void hcc_tokengen_location_setup_new_file(HccCompiler* c, HccCodeFile* code_file, bool change_dir) {
	HccLocation* location = &c->tokengen.location;
	location->code_file = code_file;
	location->parent_location = NULL;
	location->macro = NULL;
	location->code_start_idx = 0;
	location->code_end_idx = 0;
	location->line_start = 1;
	location->line_end = 2;
	location->column_start = 1;
	location->column_end = 1;

	c->tokengen.code = code_file->code.data;
	c->tokengen.code_size = code_file->code.size;
	code_file->pp_if_span_id = 0;

	if (change_dir) {
		hcc_change_working_directory_to_same_as_this_file(code_file->path_string.data);
	}
}

bool hcc_tokengen_consume_backslash(HccCompiler* c) {
	hcc_tokengen_advance_column(c, 1);
	U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
	bool found_newline = false;
	if (byte == '\r') {
		hcc_tokengen_advance_column(c, 1);
		found_newline = true;
	}
	if (byte == '\n'){
		hcc_tokengen_advance_column(c, 1);
		found_newline = true;
	}
	if (found_newline) {
		c->tokengen.location.line_end += 1;
		c->tokengen.location.column_start = 1;
		c->tokengen.location.column_end = 1;
		U32* dst = hcc_stack_push(c->tokengen.location.code_file->line_code_start_indices);
		*dst = c->tokengen.location.code_end_idx;
	}
	return found_newline;
}

void hcc_tokengen_consume_whitespace(HccCompiler* c) {
	while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
		U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
		if (byte != ' ' && byte != '\t') {
			U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
			if (byte != '\\' || !hcc_tokengen_consume_backslash(c)) {
				break;
			}
		} else {
			hcc_tokengen_advance_column(c, 1);
		}
	}
}

void hcc_tokengen_consume_whitespace_and_newlines(HccCompiler* c) {
	while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
		U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
		if (byte != ' ' && byte != '\t' && byte != '\r' && byte != '\n' && byte != '\\') {
			break;
		} else {
			if (byte == '\n') {
				hcc_tokengen_advance_newline(c);
			} else {
				hcc_tokengen_advance_column(c, 1);
			}
		}
	}
}


void hcc_tokengen_consume_until_any_byte(HccCompiler* c, char* terminator_bytes) {
	while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
		U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];
		if (byte == '\\') {
			hcc_tokengen_consume_backslash(c);
			continue;
		}

		if (strchr(terminator_bytes, byte)) {
			break;
		}

		if (byte == '\r' || byte == '\n') {
			hcc_tokengen_advance_newline(c);
		} else {
			hcc_tokengen_advance_column(c, 1);
		}
	}
}

HccString hcc_tokengen_parse_ident_from_string(HccCompiler* c, HccString string, HccErrorCode error_code) {
	U8 byte = string.data[0];
	if (
		(byte < 'a' || 'z' < byte) &&
		(byte < 'A' || 'Z' < byte) &&
		byte != '_'
	) {
		HCC_DEBUG_ASSERT(error_code != HCC_ERROR_CODE_NONE, "internal error: expected no error to happen when parsing this identifier");
		c->tokengen.location.column_end += 1;
		hcc_tokengen_bail_error_1(c, error_code, byte);
	}

	HccString ident_string = hcc_string(string.data, 0);
	while (ident_string.size < string.size) {
		U8 ident_byte = ident_string.data[ident_string.size];
		if (
			(ident_byte < 'a' || 'z' < ident_byte) &&
			(ident_byte < 'A' || 'Z' < ident_byte) &&
			(ident_byte < '0' || '9' < ident_byte) &&
			(ident_byte != '_')
		) {
			break;
		}
		ident_string.size += 1;
	}

	return ident_string;
}

HccString hcc_tokengen_parse_ident(HccCompiler* c, HccErrorCode error_code) {
	HccString code = hcc_string((char*)&c->tokengen.code[c->tokengen.location.code_end_idx], c->tokengen.code_size - c->tokengen.location.code_end_idx);
	return hcc_tokengen_parse_ident_from_string(c, code, error_code);
}

U32 hcc_tokengen_parse_num_literals(HccCompiler* c, char* num_string, U32 token_size, U32 remaining_size, HccToken* token_mut) {
	HccToken token = *token_mut;
	token_size -= 1; // move back onto the character
	while (token_size < remaining_size) {
		char ch = num_string[token_size];
		token_size += 1;
		switch (ch) {
			case 'u':
			case 'U':
				switch (token) {
					case HCC_TOKEN_LIT_SINT: token = HCC_TOKEN_LIT_UINT; break;
					case HCC_TOKEN_LIT_SLONG: token = HCC_TOKEN_LIT_ULONG; break;
					case HCC_TOKEN_LIT_SLONGLONG: token = HCC_TOKEN_LIT_ULONGLONG; break;
					case HCC_TOKEN_LIT_UINT:
					case HCC_TOKEN_LIT_ULONG:
					case HCC_TOKEN_LIT_ULONGLONG:
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_U_SUFFIX_ALREADY_USED);
						break;
					case HCC_TOKEN_LIT_FLOAT:
					case HCC_TOKEN_LIT_DOUBLE:
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_U_SUFFIX_ON_FLOAT);
						break;
				}
				break;
			case 'l':
			case 'L':
				switch (token) {
					case HCC_TOKEN_LIT_FLOAT:
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_L_SUFFIX_ON_FLOAT);
					case HCC_TOKEN_LIT_DOUBLE:
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_LONG_DOUBLE_IS_UNSUPPORTED);
					case HCC_TOKEN_LIT_SINT:
						token = HCC_TOKEN_LIT_SLONG;
						break;
					case HCC_TOKEN_LIT_SLONG:
						token = HCC_TOKEN_LIT_SLONGLONG;
						break;
					case HCC_TOKEN_LIT_UINT:
						token = HCC_TOKEN_LIT_ULONG;
						break;
					case HCC_TOKEN_LIT_ULONG:
						token = HCC_TOKEN_LIT_ULONGLONG;
						break;
				}
				break;
			case 'f':
			case 'F':
				if (token != HCC_TOKEN_LIT_DOUBLE) {
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_FLOAT_SUFFIX_MUST_FOLLOW_DECIMAL_PLACE);
				}
				token = HCC_TOKEN_LIT_FLOAT;
				break;
			default:
				token_size -= 1;
				goto END;
		}
	}

END:
	*token_mut = token;
	return token_size;
}

U32 hcc_tokengen_parse_num(HccCompiler* c, HccToken* token_out) {
	char* num_string = (char*)&c->tokengen.code[c->tokengen.location.code_end_idx];
	U32 remaining_size = c->tokengen.code_size - c->tokengen.location.code_end_idx;
	U32 token_size = 0;

	bool is_negative = num_string[0] == '-';
	if (is_negative) {
		token_size += 1; // skip the '-'
	}

	//
	// parse the radix prefix if there is a 0x or 0
	HccToken token = HCC_TOKEN_LIT_SINT;
	uint8_t radix = 10;
	if (num_string[0] == '0') {
		switch (num_string[1]) {
			case 'x':
			case 'X':
				radix = 16;
				token_size += 2;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				radix = 8;
				token_size += 2;
				break;
		}
	}

	U64 u64 = 0;
	S64 s64 = 0;
	F64 f64 = 0.0;
	F64 pow_10 = 10.0;
	while (token_size < remaining_size) {
		U8 digit = num_string[token_size];
		token_size += 1;

		switch (digit) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			{
				if (radix == 8 && digit > 7) {
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_OCTAL_DIGIT);
				}
				U32 int_digit = digit - '0';
				switch (token) {
					case HCC_TOKEN_LIT_SINT:
					case HCC_TOKEN_LIT_SLONG:
					case HCC_TOKEN_LIT_SLONGLONG:
					case HCC_TOKEN_LIT_UINT:
					case HCC_TOKEN_LIT_ULONG:
					case HCC_TOKEN_LIT_ULONGLONG:
						if (
							!hcc_i64_checked_mul(u64, radix, &u64)        ||
							!hcc_u64_checked_add(u64, int_digit, &u64)
						) {
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MAX_UINT_OVERFLOW);
						}
						break;
					case HCC_TOKEN_LIT_FLOAT:
					case HCC_TOKEN_LIT_DOUBLE:
						f64 += (F64)(int_digit) / pow_10;
						if (isinf(f64)) {
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MAX_FLOAT_OVERFLOW);
						}
						pow_10 *= 10.0;
						break;
				}
				break;
			};
			case 'u':
			case 'U':
			case 'l':
			case 'L':
				token_size = hcc_tokengen_parse_num_literals(c, num_string, token_size, remaining_size, &token);
				break;

			case '.':
				if (token == HCC_TOKEN_LIT_DOUBLE) {
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_FLOAT_HAS_DOUBLE_FULL_STOP);
				}
				if (radix != 10) {
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_FLOAT_MUST_BE_DECIMAL);
				}

				token = HCC_TOKEN_LIT_DOUBLE;
				f64 = (F64)u64;
				if ((U64)f64 != u64) {
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MAX_FLOAT_OVERFLOW);
				}
				break;

			default: {
				if (radix == 16 && ((digit >= 'a' && digit <= 'f') || (digit >= 'A' && digit <= 'F'))) {
					U32 int_digit = 10 + (digit >= 'A' ? (digit - 'A') : (digit - 'a'));
					HCC_DEBUG_ASSERT(token != HCC_TOKEN_LIT_FLOAT && token != HCC_TOKEN_LIT_DOUBLE, "internal error: expected to be dealing with integer literals here");
					if (
						!hcc_i64_checked_mul(u64, radix, &u64)        ||
						!hcc_u64_checked_add(u64, int_digit, &u64)
					) {
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MAX_UINT_OVERFLOW);
					}
					break;
				} else if (digit == 'f' || digit == 'F') {
					token_size = hcc_tokengen_parse_num_literals(c, num_string, token_size, remaining_size, &token);
				} else if ((digit >= 'a' && digit <= 'z') || (digit >= 'A' && digit <= 'Z')) {
					switch (token) {
						case HCC_TOKEN_LIT_SINT:
						case HCC_TOKEN_LIT_SLONG:
						case HCC_TOKEN_LIT_SLONGLONG:
						case HCC_TOKEN_LIT_UINT:
						case HCC_TOKEN_LIT_ULONG:
						case HCC_TOKEN_LIT_ULONGLONG:
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_INTEGER_LITERALS);
						case HCC_TOKEN_LIT_FLOAT:
						case HCC_TOKEN_LIT_DOUBLE:
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_FLOAT_LITERALS);
					}
				} else {
					token_size -= 1;
					goto NUM_END;
				}
				break;
			};
		}
	}
NUM_END:

	//
	// apply the negate if it is negative and a signed type
	if (is_negative) {
		switch (token) {
			case HCC_TOKEN_LIT_SINT:
			case HCC_TOKEN_LIT_SLONG:
			case HCC_TOKEN_LIT_SLONGLONG:
			{
				if (u64 > (U64)S64_MAX + 1) {
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MAX_SINT_OVERFLOW);
				}
				s64 = -u64;
				break;
			};
			case HCC_TOKEN_LIT_FLOAT:
			case HCC_TOKEN_LIT_DOUBLE:
				f64 = -f64;
				break;
		}
	}

	//
	// perform literal type upgrades if they exceed their current type
	switch (token) {
		case HCC_TOKEN_LIT_UINT:
			if (u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_UINT]) {
				break;
			}
			token = HCC_TOKEN_LIT_ULONG;
			// fallthrough
		case HCC_TOKEN_LIT_ULONG:
			if (u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_ULONG]) {
				break;
			}
			token = HCC_TOKEN_LIT_ULONGLONG;
			break;
		case HCC_TOKEN_LIT_SINT:
			if (is_negative) {
				if (s64 > (S64)c->basic_type_int_maxes[HCC_DATA_TYPE_SLONG] || s64 < (S64)c->basic_type_int_mins[HCC_DATA_TYPE_SLONG]) {
					token = HCC_TOKEN_LIT_SLONGLONG;
				} else if (s64 > (S64)c->basic_type_int_maxes[HCC_DATA_TYPE_SINT] || s64 < (S64)c->basic_type_int_mins[HCC_DATA_TYPE_SINT]) {
					token = HCC_TOKEN_LIT_SLONG;
				}
			} else {
				if (u64 > c->basic_type_int_maxes[HCC_DATA_TYPE_SINT]) {
					if (radix != 10 && u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_UINT]) {
						token = HCC_TOKEN_LIT_UINT;
					} else if (u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_ULONG]) {
						token = HCC_TOKEN_LIT_SLONG;
					} else if (radix != 10 && u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_ULONG]) {
						token = HCC_TOKEN_LIT_ULONG;
					} else if (u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_SLONGLONG]) {
						token = HCC_TOKEN_LIT_SLONGLONG;
					} else if (radix != 10 && u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_ULONGLONG]) {
						token = HCC_TOKEN_LIT_ULONGLONG;
					} else {
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MAX_SINT_OVERFLOW_DECIMAL);
					}
				}
				s64 = u64;
			}
			break;
		case HCC_TOKEN_LIT_SLONG:
			if (is_negative) {
				if (s64 > (S64)c->basic_type_int_maxes[HCC_DATA_TYPE_ULONG] || s64 < (S64)c->basic_type_int_mins[HCC_DATA_TYPE_ULONG]) {
					token = HCC_TOKEN_LIT_SLONGLONG;
				}
			} else {
				if (u64 > c->basic_type_int_maxes[HCC_DATA_TYPE_ULONG]) {
					if (radix != 10 && u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_ULONG]) {
						token = HCC_TOKEN_LIT_ULONG;
					} else if (u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_SLONGLONG]) {
						token = HCC_TOKEN_LIT_SLONGLONG;
					} else if (radix != 10 && u64 <= c->basic_type_int_maxes[HCC_DATA_TYPE_ULONGLONG]) {
						token = HCC_TOKEN_LIT_ULONGLONG;
					} else {
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MAX_SINT_OVERFLOW_DECIMAL);
					}
				}
				s64 = u64;
			}
			break;
	}

	//
	// apply the negate for if it is an unsigned literal
	if (is_negative) {
		switch (token) {
			case HCC_TOKEN_LIT_UINT:
			case HCC_TOKEN_LIT_ULONG:
			case HCC_TOKEN_LIT_ULONGLONG:
			{
				u64 = -u64;
				break;
			};
		}
	}

	if (token_out == NULL) {
		return token_size;
	}

	HccDataType data_type;
	switch (token) {
		case HCC_TOKEN_LIT_SINT: data_type = HCC_DATA_TYPE_SINT; break;
		case HCC_TOKEN_LIT_SLONG: data_type = HCC_DATA_TYPE_SLONG; break;
		case HCC_TOKEN_LIT_SLONGLONG: data_type = HCC_DATA_TYPE_SLONGLONG; break;
		case HCC_TOKEN_LIT_UINT: data_type = HCC_DATA_TYPE_UINT; break;
		case HCC_TOKEN_LIT_ULONG: data_type = HCC_DATA_TYPE_ULONG; break;
		case HCC_TOKEN_LIT_ULONGLONG: data_type = HCC_DATA_TYPE_ULONGLONG; break;
		case HCC_TOKEN_LIT_FLOAT: data_type = HCC_DATA_TYPE_FLOAT; break;
		case HCC_TOKEN_LIT_DOUBLE: data_type = HCC_DATA_TYPE_DOUBLE; break;
	}

	HccBasic basic;
	switch (token) {
		case HCC_TOKEN_LIT_SINT:
		case HCC_TOKEN_LIT_SLONG:
		case HCC_TOKEN_LIT_SLONGLONG: basic = hcc_basic_from_sint(c, data_type, s64); break;
		case HCC_TOKEN_LIT_UINT:
		case HCC_TOKEN_LIT_ULONG:
		case HCC_TOKEN_LIT_ULONGLONG: basic = hcc_basic_from_uint(c, data_type, u64); break;
		case HCC_TOKEN_LIT_FLOAT:
		case HCC_TOKEN_LIT_DOUBLE: basic = hcc_basic_from_float(c, data_type, f64); break;
	}

	HccTokenValue token_value;
	token_value.constant_id = hcc_constant_table_deduplicate_basic(c, data_type, &basic);
	hcc_tokengen_token_value_add(c, token_value);

	//
	// push on an addition string id of the literial so we can get the exact string representation.
	// this is needed for macro argument stringify and macro concatination.
	// TODO: we should probably just append these to the end of an array so it'll be faster but consume more RAM
	token_value.string_id = hcc_string_table_deduplicate(&c->string_table, num_string, token_size);
	hcc_tokengen_token_value_add(c, token_value);

	*token_out = token;
	return token_size;
}

void hcc_tokengen_parse_string(HccCompiler* c, char terminator_byte, bool ignore_escape_sequences_except_double_quotes) {
	c->tokengen.location.code_end_idx += 1;
	c->tokengen.location.column_end += 1;

	U32 stringify_buffer_start_idx = hcc_stack_count(c->string_buffer);
	if (c->tokengen.run_mode == HCC_TOKENGEN_RUN_MODE_PP_INCLUDE_OPERAND || ignore_escape_sequences_except_double_quotes) {
		bool ended_with_terminator = false;
		while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
			char byte = c->tokengen.code[c->tokengen.location.code_end_idx];
			c->tokengen.location.column_end += 1;
			c->tokengen.location.code_end_idx += 1;

			if (byte == '\\') {
				byte = c->tokengen.code[c->tokengen.location.code_end_idx];
				switch (byte) {
					case '\r':
						c->tokengen.location.column_end += 2;
						c->tokengen.location.code_end_idx += 2;
						break;
					case '\n':
						c->tokengen.location.column_end += 1;
						c->tokengen.location.code_end_idx += 1;
						break;
					case '\"':
						if (ignore_escape_sequences_except_double_quotes) {
							hcc_stack_push_char(c->string_buffer, '\\');
						}
						c->tokengen.location.column_end += 1;
						c->tokengen.location.code_end_idx += 1;
						hcc_stack_push_char(c->string_buffer, '"');
						break;
					default:
						if (ignore_escape_sequences_except_double_quotes) {
							hcc_stack_push_char(c->string_buffer, '\\');
						} else {
							hcc_stack_push_char(c->string_buffer, '/'); // convert \ to / for windows paths
						}
						break;
				}
			} else if (byte == '\r' || byte == '\n') {
				break;
			} else if (byte == terminator_byte) {
				ended_with_terminator = true;
				break;
			} else {
				hcc_stack_push_char(c->string_buffer, byte);
			}
		}

		if (!ended_with_terminator) {
			c->tokengen.location.column_end += 1;
			c->tokengen.location.code_end_idx -= 1;
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_UNCLOSED_STRING_LITERAL, terminator_byte);
		}

		if (ignore_escape_sequences_except_double_quotes) {
			return;
		}
	} else {
		bool ended_with_terminator = false;
		while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
			char byte = c->tokengen.code[c->tokengen.location.code_end_idx];
			c->tokengen.location.column_end += 1;
			c->tokengen.location.code_end_idx += 1;

			if (byte == '\\') {
				byte = c->tokengen.code[c->tokengen.location.code_end_idx];
				switch (byte) {
					case '\\':
					case '\r':
					case '\n':
					case '\"':
					case '\'':
						hcc_stack_push_char(c->string_buffer, byte);
						c->tokengen.location.column_end += 2;
						c->tokengen.location.code_end_idx += 2;
						break;
					default:
						break;
				}
			} else if (byte == '\r' || byte == '\n') {
				break;
			} else if (byte == terminator_byte) {
				ended_with_terminator = true;
				break;
			} else {
				hcc_stack_push_char(c->string_buffer, byte);
			}
		}

		if (!ended_with_terminator) {
			c->tokengen.location.column_end += 1;
			c->tokengen.location.code_end_idx -= 1;
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_UNCLOSED_STRING_LITERAL, terminator_byte);
		}
	}

	hcc_stack_push_char(c->string_buffer, '\0');

	HccTokenValue token_value;
	U32 string_size = hcc_stack_count(c->string_buffer) - stringify_buffer_start_idx;
	token_value.string_id = hcc_string_table_deduplicate(&c->string_table, hcc_stack_get(c->string_buffer, stringify_buffer_start_idx), string_size);
	hcc_stack_resize(c->string_buffer, stringify_buffer_start_idx);

	hcc_tokengen_token_add(c, terminator_byte == '>' ? HCC_TOKEN_INCLUDE_PATH_SYSTEM : HCC_TOKEN_STRING);
	hcc_tokengen_token_value_add(c, token_value);
}

U32 hcc_tokengen_find_macro_param(HccCompiler* c, HccStringId ident_string_id) {
	U32 param_idx;
	for (param_idx = 0; param_idx < c->tokengen.macro_params_count; param_idx += 1) {
		if (c->tokengen.macro_param_string_ids[param_idx].idx_plus_one == ident_string_id.idx_plus_one) {
			break;
		}
	}

	return param_idx;
}

void hcc_tokengen_consume_hash_for_define_replacement_list(HccCompiler* c) {
	if (c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '#') {
		if (c->tokengen.macro_tokens_start_idx == hcc_stack_count(c->pp.macro_token_bag.tokens)) {
			hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_MACRO_STARTS_WITH_CONCAT);
		}

		HccToken temp_token = *hcc_stack_get_last(c->pp.macro_token_bag.tokens);
		U32 temp_token_location_idx = *hcc_stack_get_last(c->pp.macro_token_bag.token_location_indices);
		bool has_left_whitespace = false;
		if (temp_token == HCC_TOKEN_MACRO_WHITESPACE) {
			hcc_stack_pop(c->pp.macro_token_bag.tokens);
			hcc_stack_pop(c->pp.macro_token_bag.token_location_indices);
			temp_token = *hcc_stack_get_last(c->pp.macro_token_bag.tokens);
			temp_token_location_idx = *hcc_stack_get_last(c->pp.macro_token_bag.token_location_indices);
			has_left_whitespace = true;
		}

		hcc_stack_pop(c->pp.macro_token_bag.tokens);
		hcc_stack_pop(c->pp.macro_token_bag.token_location_indices);

		hcc_tokengen_advance_column(c, 2); // skip the '##'
		hcc_tokengen_token_add(c, has_left_whitespace ? HCC_TOKEN_MACRO_CONCAT_WHITESPACE : HCC_TOKEN_MACRO_CONCAT);

		*hcc_stack_push(c->pp.macro_token_bag.tokens) = temp_token;
		*hcc_stack_push(c->pp.macro_token_bag.token_location_indices) = temp_token_location_idx;
	} else {
		if (c->tokengen.macro_is_function) {
			hcc_tokengen_advance_column(c, 1); // skip the '#'

			U32 before_ident_start_idx = c->tokengen.location.code_end_idx;
			hcc_tokengen_consume_whitespace(c);
			bool whitespace_after_hash = before_ident_start_idx != c->tokengen.location.code_end_idx;
			HccString ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_STRINGIFY_MUST_BE_MACRO_PARAM);
			HccStringId ident_string_id = hcc_string_table_deduplicate(&c->string_table, (char*)ident_string.data, ident_string.size);

			U32 param_idx;
			if (ident_string_id.idx_plus_one == HCC_STRING_ID___VA_ARGS__) {
				if (!c->tokengen.macro_has_va_arg) {
					c->tokengen.location.column_end += ident_string.size;
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_USE_OF_VA_ARGS);
				}
				param_idx = c->tokengen.macro_params_count - 1;
			} else {
				param_idx = hcc_tokengen_find_macro_param(c, ident_string_id);
				if (param_idx == c->tokengen.macro_params_count) {
					c->tokengen.location.column_end += ident_string.size;
					hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_STRINGIFY_MUST_BE_MACRO_PARAM);
				}
			}

			hcc_tokengen_advance_column(c, ident_string.size);
			hcc_tokengen_token_add(c, whitespace_after_hash ? HCC_TOKEN_MACRO_STRINGIFY_WHITESPACE : HCC_TOKEN_MACRO_STRINGIFY);
			hcc_tokengen_token_value_add(c, ((HccTokenValue) { .macro_param_idx = param_idx }));
		} else {
			hcc_tokengen_advance_column(c, 1); // skip the '#'
			hcc_tokengen_token_add(c, HCC_TOKEN_HASH);
		}
	}
}

bool hcc_tokengen_is_first_non_whitespace_on_line(HccCompiler* c) {
	for (U32 idx = c->tokengen.location.code_end_idx; idx-- > 0; ) {
		switch (c->tokengen.code[idx]) {
			case '\r':
			case '\n':
				return true;
			case ' ':
			case '\t':
				break;
			default:
				return false;
		}
	}

	return true;
}


void hcc_tokengen_bracket_open(HccCompiler* c, HccToken token, HccLocation* location) {
	HccOpenBracket* ob = hcc_stack_push(c->tokengen.open_bracket_stack);
	HccToken close_token = token + 1; // the close token is defined right away after the open in the HccToken enum
	ob->close_token = close_token;
	ob->open_token_location = location;
}

void hcc_tokengen_bracket_close(HccCompiler* c, HccToken token, HccLocation* location) {
	if (hcc_stack_count(c->tokengen.open_bracket_stack) == 0) {
		hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_NO_BRACKETS_OPEN, hcc_token_strings[token]);
	}

	HccOpenBracket* ob = hcc_stack_get_last(c->tokengen.open_bracket_stack);
	hcc_stack_pop(c->tokengen.open_bracket_stack);
	if (ob->close_token != token) {
		hcc_tokengen_advance_column(c, 1);
		hcc_tokengen_bail_error_2_ptr(c, HCC_ERROR_CODE_INVALID_CLOSE_BRACKET_PAIR, location, ob->open_token_location, hcc_token_strings[ob->close_token], hcc_token_strings[token]);
	}
}

void hcc_tokengen_run(HccCompiler* c, HccTokenBag* dst_token_bag, HccTokenGenRunMode run_mode) {
	HccTokenGenRunMode old_run_mode = c->tokengen.run_mode;
	HccTokenBag* old_dst_token_bag = c->tokengen.dst_token_bag;
	c->tokengen.run_mode = run_mode;
	c->tokengen.dst_token_bag = dst_token_bag;

	U32 macro_args_nested_parenthesis_count = 0;
	while (1) {
		//
		// if we have reached the end of this file, then return
		// to the parent file or end the token generation.
		if (c->tokengen.location.code_end_idx >= c->tokengen.code_size) {
			if (hcc_stack_count(c->tokengen.paused_file_stack) == 0) {
				goto RETURN;
			}

			hcc_tokengen_location_pop(c);
			if (run_mode == HCC_TOKENGEN_RUN_MODE_PP_CONCAT) {
				goto RETURN;
			}
			continue;
		}

		U8 byte = c->tokengen.code[c->tokengen.location.code_end_idx];

		c->tokengen.location.code_start_idx = c->tokengen.location.code_end_idx;
		c->tokengen.location.line_start += c->tokengen.location.line_end - c->tokengen.location.line_start - 1;
		c->tokengen.location.column_start = c->tokengen.location.column_end;

		HccToken token = HCC_TOKEN_COUNT;
		HccToken close_token;
		U32 token_size = 1;
		switch (byte) {
			case ' ':
			case '\t':
				hcc_tokengen_consume_whitespace(c);
				switch (run_mode) {
					case HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST:
					case HCC_TOKENGEN_RUN_MODE_PP_MACRO_ARGS:
					case HCC_TOKENGEN_RUN_MODE_PP_CONCAT:
						if (*hcc_stack_get_last(dst_token_bag->tokens) != HCC_TOKEN_MACRO_WHITESPACE) {
							hcc_tokengen_token_add(c, HCC_TOKEN_MACRO_WHITESPACE);
						}
						break;
				}
				continue;
			case '\r':
				c->tokengen.location.code_start_idx += 1;
				c->tokengen.location.code_end_idx += 1;
				continue;
			case '\n':
				switch (run_mode) {
					case HCC_TOKENGEN_RUN_MODE_PP_INCLUDE_OPERAND:
					case HCC_TOKENGEN_RUN_MODE_PP_IF_OPERAND:
					case HCC_TOKENGEN_RUN_MODE_PP_OPERAND:
					case HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST:
						goto RETURN;
					case HCC_TOKENGEN_RUN_MODE_PP_MACRO_ARGS:
					case HCC_TOKENGEN_RUN_MODE_PP_CONCAT:
						if (*hcc_stack_get_last(dst_token_bag->tokens) != HCC_TOKEN_MACRO_WHITESPACE) {
							hcc_tokengen_token_add(c, HCC_TOKEN_MACRO_WHITESPACE);
						}
						break;
				}

				hcc_tokengen_advance_newline(c);
				continue;
			case '.': token = HCC_TOKEN_FULL_STOP; break;
			case ',': token = HCC_TOKEN_COMMA; break;
			case ';': token = HCC_TOKEN_SEMICOLON; break;
			case ':': token = HCC_TOKEN_COLON; break;
			case '~': token = HCC_TOKEN_TILDE; break;
			case '?': token = HCC_TOKEN_QUESTION_MARK; break;
			case '+': {
				U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HCC_TOKEN_ADD_ASSIGN;
				} else if (next_byte == '+') {
					token_size = 2;
					token = HCC_TOKEN_INCREMENT;
				} else {
					token = HCC_TOKEN_PLUS;
				}
				break;
			};
			case '-': {
				U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
				if (isdigit(next_byte)) {
					token_size = hcc_tokengen_parse_num(c, &token);
				} else if (next_byte == '=') {
					token_size = 2;
					token = HCC_TOKEN_SUBTRACT_ASSIGN;
				} else if (next_byte == '-') {
					token_size = 2;
					token = HCC_TOKEN_DECREMENT;
				} else {
					token = HCC_TOKEN_MINUS;
				}
				break;
			};
			case '/': {
				U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
				if (next_byte == '/') {
					c->tokengen.location.code_end_idx += 2;
					while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
						U8 b = c->tokengen.code[c->tokengen.location.code_end_idx];
						if (b == '\n') {
							break;
						}
						c->tokengen.location.code_end_idx += 1;
					}

					token_size = c->tokengen.location.code_end_idx - c->tokengen.location.code_start_idx;
					c->tokengen.location.column_start += token_size;
					c->tokengen.location.column_end += token_size;
					continue;
				} else if (next_byte == '*') {
					hcc_tokengen_advance_column(c, 2);
					while (c->tokengen.location.code_end_idx < c->tokengen.code_size) {
						U8 b = c->tokengen.code[c->tokengen.location.code_end_idx];
						if (byte == '\n') {
							hcc_tokengen_advance_newline(c);
						} else {
							hcc_tokengen_advance_column(c, 1);
							if (b == '*') {
								b = c->tokengen.code[c->tokengen.location.code_end_idx];
								if (b == '/') { // no need to check in bounds see _HCC_TOKENIZER_LOOK_HEAD_SIZE
									hcc_tokengen_advance_column(c, 1);
									break;
								}
							}
						}
					}

					c->tokengen.location.column_start = c->tokengen.location.column_end;
					continue;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HCC_TOKEN_DIVIDE_ASSIGN;
				} else {
					token = HCC_TOKEN_FORWARD_SLASH;
				}
				break;
			};
			case '*':
				if (c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_TOKEN_MULTIPLY_ASSIGN;
				} else {
					token = HCC_TOKEN_ASTERISK;
				}
				break;
			case '%':
				if (c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_TOKEN_MODULO_ASSIGN;
				} else {
					token = HCC_TOKEN_PERCENT;
				}
				break;
			case '&': {
				U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
				if (next_byte == '&') {
					token_size = 2;
					token = HCC_TOKEN_LOGICAL_AND;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HCC_TOKEN_BIT_AND_ASSIGN;
				} else {
					token = HCC_TOKEN_AMPERSAND;
				}
				break;
			};
			case '|': {
				U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
				if (next_byte == '|') {
					token_size = 2;
					token = HCC_TOKEN_LOGICAL_OR;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HCC_TOKEN_BIT_OR_ASSIGN;
				} else {
					token = HCC_TOKEN_PIPE;
				}
				break;
			};
			case '^':
				if (c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_TOKEN_BIT_XOR_ASSIGN;
				} else {
					token = HCC_TOKEN_CARET;
				}
				break;
			case '!':
				if (c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_TOKEN_LOGICAL_NOT_EQUAL;
				} else {
					token = HCC_TOKEN_EXCLAMATION_MARK;
				}
				break;
			case '=':
				if (c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_TOKEN_LOGICAL_EQUAL;
				} else {
					token = HCC_TOKEN_EQUAL;
				}
				break;
			case '<': {
				if (run_mode == HCC_TOKENGEN_RUN_MODE_PP_INCLUDE_OPERAND) {
					hcc_tokengen_parse_string(c, '>', false);
					continue;
				}
				U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HCC_TOKEN_LESS_THAN_OR_EQUAL;
				} else if (next_byte == '<') {
					if (c->tokengen.code[c->tokengen.location.code_end_idx + 2] == '=') {
						token_size = 3;
						token = HCC_TOKEN_BIT_SHIFT_LEFT_ASSIGN;
					} else {
						token_size = 2;
						token = HCC_TOKEN_BIT_SHIFT_LEFT;
					}
				} else {
					token = HCC_TOKEN_LESS_THAN;
				}
				break;
			};
			case '>': {
				U8 next_byte = c->tokengen.code[c->tokengen.location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HCC_TOKEN_GREATER_THAN_OR_EQUAL;
				} else if (next_byte == '>') {
					if (c->tokengen.code[c->tokengen.location.code_end_idx + 2] == '=') {
						token_size = 3;
						token = HCC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN;
					} else {
						token_size = 2;
						token = HCC_TOKEN_BIT_SHIFT_RIGHT;
					}
				} else {
					token = HCC_TOKEN_GREATER_THAN;
				}
				break;
			};
			case '(':
				macro_args_nested_parenthesis_count += 1;
				token = HCC_TOKEN_PARENTHESIS_OPEN;
				goto OPEN_BRACKET;
			case '{': token = HCC_TOKEN_CURLY_OPEN; goto OPEN_BRACKET;
			case '[': token = HCC_TOKEN_SQUARE_OPEN; goto OPEN_BRACKET;
OPEN_BRACKET: {}
				if (run_mode == HCC_TOKENGEN_RUN_MODE_CODE) {
					HccLocation* location = hcc_stack_get_next_push(c->tokengen.token_locations);
					hcc_tokengen_bracket_open(c, token, location);
				}
				break;

			case ')':
				macro_args_nested_parenthesis_count -= 1;
				token = HCC_TOKEN_PARENTHESIS_CLOSE;
				if (run_mode == HCC_TOKENGEN_RUN_MODE_PP_MACRO_ARGS && macro_args_nested_parenthesis_count == 0) {
					hcc_tokengen_advance_column(c, 1);
					hcc_tokengen_token_add(c, token);
					goto RETURN;
				}
				goto CLOSE_BRACKET;
			case '}': token = HCC_TOKEN_CURLY_CLOSE; goto CLOSE_BRACKET;
			case ']': token = HCC_TOKEN_SQUARE_CLOSE; goto CLOSE_BRACKET;
CLOSE_BRACKET:
				if (run_mode == HCC_TOKENGEN_RUN_MODE_CODE) {
					hcc_tokengen_bracket_close(c, token, &c->tokengen.location);
				}
				break;

			case '"':
				hcc_tokengen_parse_string(c, '"', false);
				continue;

			case '\\':
				if (!hcc_tokengen_consume_backslash(c)) {
					hcc_tokengen_token_add(c, HCC_TOKEN_BACK_SLASH);
				}
				continue;

			case '#':
				switch (run_mode) {
					case HCC_TOKENGEN_RUN_MODE_PP_INCLUDE_OPERAND:
					case HCC_TOKENGEN_RUN_MODE_PP_IF_OPERAND:
					case HCC_TOKENGEN_RUN_MODE_PP_OPERAND:
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_TOKEN_HASH_IN_PP_OPERAND);
					case HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST:
						hcc_tokengen_consume_hash_for_define_replacement_list(c);
						continue;
					case HCC_TOKENGEN_RUN_MODE_PP_CONCAT:
						if (c->tokengen.code[c->tokengen.location.code_end_idx + 1] == '#') {
							hcc_tokengen_advance_column(c, 2); // skip the '##'
							hcc_tokengen_token_add(c, HCC_TOKEN_DOUBLE_HASH);
							continue;
						} else {
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_TOKEN, '#');
						}
						break;
					case HCC_TOKENGEN_RUN_MODE_PP_MACRO_ARGS:
						hcc_tokengen_advance_column(c, 1);
						hcc_tokengen_token_add(c, HCC_TOKEN_HASH);
						continue;
					case HCC_TOKENGEN_RUN_MODE_CODE:
						if (hcc_tokengen_is_first_non_whitespace_on_line(c)) {
							hcc_pp_parse_directive(c);
							continue;
						} else {
							hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_PP_DIRECTIVE_NOT_FIRST_ON_LINE);
						}
						break;
				}
				break;

			default: {
				if ('0' <= byte && byte <= '9') {
					token_size = hcc_tokengen_parse_num(c, &token);
					break;
				}

				HccString ident_string = hcc_tokengen_parse_ident(c, HCC_ERROR_CODE_INVALID_TOKEN);
				hcc_tokengen_advance_column(c, ident_string.size);

				HccStringId ident_string_id = hcc_string_table_deduplicate(&c->string_table, (char*)ident_string.data, ident_string.size);
				if (run_mode == HCC_TOKENGEN_RUN_MODE_PP_IF_OPERAND && ident_string_id.idx_plus_one == HCC_STRING_ID_DEFINED) {
					hcc_pp_parse_defined(c);
					continue;
				}

				if (ident_string_id.idx_plus_one == HCC_STRING_ID___VA_ARGS__) {
					if (run_mode != HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST || !c->tokengen.macro_has_va_arg) {
						c->tokengen.location.column_end += ident_string.size;
						hcc_tokengen_bail_error_1(c, HCC_ERROR_CODE_INVALID_USE_OF_VA_ARGS);
					}

					hcc_tokengen_token_add(c, HCC_TOKEN_MACRO_PARAM);
					hcc_tokengen_token_value_add(c, ((HccTokenValue){ .macro_param_idx = c->tokengen.macro_params_count - 1 }));
					continue;
				}

				if (run_mode == HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST) {
					U32 param_idx = hcc_tokengen_find_macro_param(c, ident_string_id);
					if (param_idx < c->tokengen.macro_params_count) {
						hcc_tokengen_token_add(c, HCC_TOKEN_MACRO_PARAM);
						hcc_tokengen_token_value_add(c, ((HccTokenValue){ .macro_param_idx = param_idx }));
						continue;
					}
				}

				if (run_mode == HCC_TOKENGEN_RUN_MODE_CODE) {
					U32 macro_idx;
					if (hcc_hash_table_find(&c->pp.macro_declarations, ident_string_id.idx_plus_one, &macro_idx)) {
						if (HCC_STRING_ID_PREDEFINED_MACROS_START <= ident_string_id.idx_plus_one && ident_string_id.idx_plus_one < HCC_STRING_ID_PREDEFINED_MACROS_END) {
							HccPPPredefinedMacro m = ident_string_id.idx_plus_one - HCC_STRING_ID_PREDEFINED_MACROS_START;
							hcc_pp_copy_expand_predefined_macro(c, m);
							continue;
						} else {
							HccPPMacro* macro = hcc_pp_macro_get(c, macro_idx);
							bool can_expand = true;

							HccLocation macro_callsite_location = c->tokengen.location;
							if (macro->is_function) {
								HccLocation backup_location = c->tokengen.location;
								U32 backup_lines_count = hcc_stack_count(c->tokengen.location.code_file->line_code_start_indices);

								hcc_tokengen_consume_whitespace_and_newlines(c);
								if (c->tokengen.code[c->tokengen.location.code_end_idx] != '(') {
									//
									// the identifier is a macro function but we don't actually call it.
									// so return back and just process this as a regular identifier.
									c->tokengen.location = backup_location;
									hcc_stack_resize(c->tokengen.location.code_file->line_code_start_indices, backup_lines_count);
									can_expand = false;
								}
							}

							if (can_expand) {
								hcc_pp_copy_expand_macro_begin(c, macro, &macro_callsite_location);
								continue;
							}
						}
					}
				}

				if (HCC_STRING_ID_KEYWORDS_START <= ident_string_id.idx_plus_one && ident_string_id.idx_plus_one < HCC_STRING_ID_KEYWORDS_END) {
					token = HCC_TOKEN_KEYWORDS_START + (ident_string_id.idx_plus_one - HCC_STRING_ID_KEYWORDS_START);
				} else {
					token = HCC_TOKEN_IDENT;
					HccTokenValue token_value;
					token_value.string_id = ident_string_id;
					hcc_tokengen_token_value_add(c, token_value);
				}

				hcc_tokengen_token_add(c, token);
				continue;
			};
		}

		hcc_tokengen_advance_column(c, token_size);
		hcc_tokengen_token_add(c, token);
	}

RETURN: {}
	if (run_mode == HCC_TOKENGEN_RUN_MODE_CODE && hcc_stack_count(c->tokengen.paused_file_stack) == 0) {
		hcc_tokengen_token_add(c, HCC_TOKEN_EOF);
	}
	c->tokengen.run_mode = old_run_mode;
	c->tokengen.dst_token_bag = old_dst_token_bag;
}

void hcc_tokengen_print(HccCompiler* c, FILE* f) {
	uint32_t token_value_idx = 0;
	for (uint32_t i = 0; i < hcc_stack_count(c->tokengen.token_bag.tokens); i += 1) {
		HccToken token = *hcc_stack_get(c->tokengen.token_bag.tokens, i);
		HccTokenValue value;
		HccString string;
		switch (token) {
			case HCC_TOKEN_IDENT:
				value = *hcc_stack_get(c->tokengen.token_bag.token_values, token_value_idx);
				token_value_idx += 1;
				string = hcc_string_table_get(&c->string_table, value.string_id);
				fprintf(f, "%s -> %.*s\n", hcc_token_strings[token], (int)string.size, string.data);
				break;
			case HCC_TOKEN_STRING:
				value = *hcc_stack_get(c->tokengen.token_bag.token_values, token_value_idx);
				token_value_idx += 1;
				string = hcc_string_table_get(&c->string_table, value.string_id);
				fprintf(f, "%s -> \"%.*s\"\n", hcc_token_strings[token], (int)string.size, string.data);
				break;
			case HCC_TOKEN_LIT_SINT:
			case HCC_TOKEN_LIT_SLONG:
			case HCC_TOKEN_LIT_SLONGLONG:
			case HCC_TOKEN_LIT_UINT:
			case HCC_TOKEN_LIT_ULONG:
			case HCC_TOKEN_LIT_ULONGLONG:
			case HCC_TOKEN_LIT_FLOAT:
			case HCC_TOKEN_LIT_DOUBLE:
				value = *hcc_stack_get(c->tokengen.token_bag.token_values, token_value_idx);
				fprintf(f, "%s -> ", hcc_token_strings[token]);
				hcc_constant_print(c, value.constant_id, stdout);
				fprintf(f, "\n");
				token_value_idx += hcc_token_num_values(token);
				break;
			default:
				fprintf(f, "%s\n", hcc_token_strings[token]);
				token_value_idx += hcc_token_num_values(token);
				break;
		}
	}

	for (U32 macros_idx = 0; macros_idx < hcc_stack_count(c->pp.macros); macros_idx += 1) {
		HccPPMacro* macro = hcc_stack_get(c->pp.macros, macros_idx);
		HccString name = hcc_string_table_get(&c->string_table, macro->identifier_string_id);
		HccTokenCursor cursor = macro->token_cursor;
		hcc_stack_clear(c->pp.stringify_buffer);
		HccStringId string_id = hcc_token_bag_stringify_range(c, &c->pp.macro_token_bag, &cursor, macro);
		HccString string = hcc_string_table_get(&c->string_table, string_id);
		fprintf(f, "#define %.*s", (int)name.size, name.data);
		char macro_param_names[1024];
		if (macro->is_function) {
			fprintf(f, "(");
			for (U32 idx = 0; idx < macro->params_count; idx += 1) {
				HccStringId param_string_id = *hcc_stack_get(c->pp.macro_params, macro->params_start_idx + idx);
				HccString param_string = hcc_string_table_get(&c->string_table, param_string_id);
				fprintf(f, "%.*s", (int)param_string.size, param_string.data);
				if (idx + 1 < macro->params_count) {
					fprintf(f, ", ");
				}
			}
			fprintf(f, ")");
		}
		fprintf(f, " %.*s\n", (int)string.size, string.data);
	}
}

// ===========================================
//
//
// Syntax Generator
//
//
// ===========================================

HccToken hcc_specifier_tokens[HCC_SPECIFIER_COUNT] = {
	[HCC_SPECIFIER_STATIC] =           HCC_TOKEN_KEYWORD_STATIC,
	[HCC_SPECIFIER_CONST] =            HCC_TOKEN_KEYWORD_CONST,
	[HCC_SPECIFIER_INLINE] =           HCC_TOKEN_KEYWORD_INLINE,
	[HCC_SPECIFIER_NO_RETURN] =        HCC_TOKEN_KEYWORD_NO_RETURN,
	[HCC_SPECIFIER_INTRINSIC] =        HCC_TOKEN_KEYWORD_INTRINSIC,
	[HCC_SPECIFIER_RASTERIZER_STATE] = HCC_TOKEN_KEYWORD_RASTERIZER_STATE,
	[HCC_SPECIFIER_FRAGMENT_STATE] =   HCC_TOKEN_KEYWORD_FRAGMENT_STATE,
	[HCC_SPECIFIER_POSITION] =         HCC_TOKEN_KEYWORD_POSITION,
	[HCC_SPECIFIER_NOINTERP] =         HCC_TOKEN_KEYWORD_NOINTERP,
	[HCC_SPECIFIER_VERTEX] =           HCC_TOKEN_KEYWORD_VERTEX,
	[HCC_SPECIFIER_FRAGMENT] =         HCC_TOKEN_KEYWORD_FRAGMENT,
};

void hcc_astgen_init(HccCompiler* c, HccCompilerSetup* setup) {
	HccAstGen* astgen = &c->astgen;
	U32 compound_data_types_cap = setup->astgen.struct_data_types_cap + setup->astgen.union_data_types_cap;
	U32 global_declarations_cap = setup->astgen.typedefs_cap + setup->astgen.functions_cap + setup->astgen.enum_values_cap + setup->astgen.global_variables_cap;
	astgen->token_bag = c->tokengen.token_bag;

	astgen->function_params_and_variables = hcc_stack_init(HccVariable, setup->astgen.function_params_and_variables_cap, HCC_ALLOC_TAG_ASTGEN_FUNCTION_PARAMS_AND_VARIABLES);
	astgen->functions = hcc_stack_init(HccFunction, setup->astgen.functions_cap, HCC_ALLOC_TAG_ASTGEN_FUNCTIONS);
	astgen->exprs = hcc_stack_init(HccExpr, setup->astgen.exprs_cap, HCC_ALLOC_TAG_ASTGEN_EXPRS);
	astgen->expr_locations = hcc_stack_init(HccLocation, setup->astgen.expr_locations_cap, HCC_ALLOC_TAG_ASTGEN_EXPR_LOCATIONS);
	astgen->global_variables = hcc_stack_init(HccVariable, setup->astgen.global_variables_cap, HCC_ALLOC_TAG_ASTGEN_GLOBAL_VARIABLES);
	astgen->function_used_function_indices = hcc_stack_init(U32, setup->astgen.function_used_function_indices_cap, HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_FUNCTION_INDICES);
	astgen->function_used_static_variables = hcc_stack_init(HccDecl, setup->astgen.function_used_static_variables_cap, HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_STATIC_VARIABLES);
	astgen->entry_point_function_indices = hcc_stack_init(U32, setup->astgen.entry_point_function_indices_cap, HCC_ALLOC_TAG_ASTGEN_ENTRY_POINT_FUNCTION_INDICES);
	astgen->used_function_indices = hcc_stack_init(U32, setup->astgen.used_function_indices_cap, HCC_ALLOC_TAG_ASTGEN_USED_FUNCTION_INDICES);
	astgen->recursive_function_indices_stack = hcc_stack_init(U32, setup->astgen.recursive_function_indices_stack_cap, HCC_ALLOC_TAG_ASTGEN_RECURSIVE_FUNCTION_INDICES_STACK);
	astgen->array_data_types = hcc_stack_init(HccArrayDataType, setup->astgen.array_data_types_cap, HCC_ALLOC_TAG_ASTGEN_ARRAY_DATA_TYPES);
	astgen->compound_data_types = hcc_stack_init(HccCompoundDataType, compound_data_types_cap, HCC_ALLOC_TAG_ASTGEN_COMPOUND_DATA_TYPES);
	astgen->compound_fields = hcc_stack_init(HccCompoundField, setup->astgen.compound_fields_cap, HCC_ALLOC_TAG_ASTGEN_COMPOUND_FIELDS);
	astgen->typedefs = hcc_stack_init(HccTypedef, setup->astgen.typedefs_cap, HCC_ALLOC_TAG_ASTGEN_TYPEDEFS);
	astgen->enum_data_types = hcc_stack_init(HccEnumDataType, setup->astgen.enum_data_types_cap, HCC_ALLOC_TAG_ASTGEN_ENUM_DATA_TYPES);
	astgen->enum_values = hcc_stack_init(HccEnumValue, setup->astgen.enum_values_cap, HCC_ALLOC_TAG_ASTGEN_ENUM_VALUES);
	astgen->ordered_data_types = hcc_stack_init(HccDataType, setup->astgen.ordered_data_types_cap, HCC_ALLOC_TAG_ASTGEN_ORDERED_DATA_TYPES);
	astgen->compound_type_find_fields = hcc_stack_init(HccFieldAccess, setup->astgen.compound_type_find_fields_cap, HCC_ALLOC_TAG_ASTGEN_COMPOUND_TYPE_FIND_FIELDS);

	hcc_hash_table_init(&astgen->global_declarations, global_declarations_cap, HCC_ALLOC_TAG_ASTGEN_GLOBAL_DECLARATIONS);
	hcc_hash_table_init(&astgen->struct_declarations, setup->astgen.struct_data_types_cap, HCC_ALLOC_TAG_ASTGEN_STRUCT_DECLARATIONS);
	hcc_hash_table_init(&astgen->union_declarations, setup->astgen.union_data_types_cap, HCC_ALLOC_TAG_ASTGEN_UNION_DECLARATIONS);
	hcc_hash_table_init(&astgen->enum_declarations, setup->astgen.enum_data_types_cap, HCC_ALLOC_TAG_ASTGEN_ENUM_DECLARATIONS);

	astgen->curly_initializer.nested = hcc_stack_init(HccAstGenCurlyInitializerNested, setup->astgen.curly_initializer_nested_cap, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_CURLYS);
	astgen->curly_initializer.nested_curlys = hcc_stack_init(HccAstGenCurlyInitializerCurly, setup->astgen.curly_initializer_nested_curlys_cap, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_CURLYS);
	astgen->curly_initializer.nested_elmts = hcc_stack_init(HccAstGenCurlyInitializerElmt, setup->astgen.curly_initializer_nested_elmts_cap, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_ELMTS);
	astgen->curly_initializer.designated_initializers = hcc_stack_init(HccAstGenDesignatorInitializer, setup->astgen.curly_initializer_designator_initializers_cap, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZERS);
	astgen->curly_initializer.designated_initializer_elmt_indices = hcc_stack_init(U64, setup->astgen.curly_initializer_designator_initializer_elmt_indices_cap, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZER_ELMT_INDICES);

	astgen->variable_stack_strings = hcc_stack_init(HccStringId, setup->astgen.variable_stack_cap, HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_STRINGS);
	astgen->variable_stack_var_indices = hcc_stack_init(U32, setup->astgen.variable_stack_cap, HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_VAR_INDICES);

	hcc_stack_resize(c->astgen.typedefs, HCC_TYPEDEF_IDX_USER_START);
	hcc_stack_resize(c->astgen.compound_data_types, HCC_COMPOUND_DATA_TYPE_IDX_USER_START);
	hcc_stack_resize(c->astgen.functions, HCC_FUNCTION_IDX_USER_START);

	hcc_hash_table_init(&astgen->field_name_to_token_idx, setup->astgen.compound_type_fields_cap, HCC_ALLOC_TAG_ASTGEN_FIELD_NAME_TO_TOKEN_IDX);
}

void hcc_astgen_error_1(HccCompiler* c, HccErrorCode error_code, ...) {
	va_list va_args;
	va_start(va_args, error_code);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	hcc_error_pushv(c, error_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_astgen_error_2_idx(HccCompiler* c, HccErrorCode error_code, U32 other_token_idx, ...) {
	va_list va_args;
	va_start(va_args, other_token_idx);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	HccLocation* other_location = other_token_idx == U32_MAX ? NULL : hcc_token_bag_location_get(c, &c->astgen.token_bag, other_token_idx);
	hcc_error_pushv(c, error_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_astgen_error_2_ptr(HccCompiler* c, HccErrorCode error_code, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	hcc_error_pushv(c, error_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_astgen_warn_1(HccCompiler* c, HccWarnCode warn_code, ...) {
	va_list va_args;
	va_start(va_args, warn_code);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	hcc_warn_pushv(c, warn_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_astgen_warn_2_idx(HccCompiler* c, HccWarnCode warn_code, U32 other_token_idx, ...) {
	va_list va_args;
	va_start(va_args, other_token_idx);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	HccLocation* other_location = other_token_idx == U32_MAX ? NULL : hcc_token_bag_location_get(c, &c->astgen.token_bag, other_token_idx);
	hcc_warn_pushv(c, warn_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_astgen_warn_2_ptr(HccCompiler* c, HccWarnCode warn_code, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	hcc_warn_pushv(c, warn_code, location, other_location, va_args);
	va_end(va_args);
}

noreturn void hcc_astgen_bail_error_1(HccCompiler* c, HccErrorCode error_code, ...) {
	va_list va_args;
	va_start(va_args, error_code);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	hcc_error_pushv(c, error_code, location, NULL, va_args);
	va_end(va_args);

	hcc_compiler_bail(c);
}

noreturn void hcc_astgen_bail_error_1_merge_apply(HccCompiler* c, HccErrorCode error_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	hcc_location_merge_apply(location, hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx));
	hcc_error_pushv(c, error_code, location, NULL, va_args);
	va_end(va_args);

	hcc_compiler_bail(c);
}

noreturn void hcc_astgen_bail_error_2_idx(HccCompiler* c, HccErrorCode error_code, U32 other_token_idx, ...) {
	va_list va_args;
	va_start(va_args, other_token_idx);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	HccLocation* other_location = other_token_idx == U32_MAX ? NULL : hcc_token_bag_location_get(c, &c->astgen.token_bag, other_token_idx);
	hcc_error_pushv(c, error_code, location, other_location, va_args);
	va_end(va_args);

	hcc_compiler_bail(c);
}

noreturn void hcc_astgen_bail_error_2_ptr(HccCompiler* c, HccErrorCode error_code, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
	HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
	hcc_error_pushv(c, error_code, location, other_location, va_args);
	va_end(va_args);

	hcc_compiler_bail(c);
}

HccToken hcc_astgen_token_peek(HccCompiler* c) {
	return c->astgen.token_bag.tokens[HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1)];
}

HccToken hcc_astgen_token_peek_ahead(HccCompiler* c, U32 by) {
	return c->astgen.token_bag.tokens[HCC_MIN(c->astgen.token_read_idx + by, hcc_stack_count(c->astgen.token_bag.tokens) - 1)];
}

void hcc_astgen_token_consume(HccCompiler* c, U32 amount) {
	c->astgen.token_read_idx += amount;
}

HccToken hcc_astgen_token_next(HccCompiler* c) {
	c->astgen.token_read_idx += 1;
	return c->astgen.token_bag.tokens[HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1)];
}

void hcc_astgen_token_value_consume(HccCompiler* c, U32 amount) {
	c->astgen.token_value_read_idx += amount;
}

HccTokenValue hcc_astgen_token_value_peek(HccCompiler* c) {
	HccTokenValue value = c->astgen.token_bag.token_values[HCC_MIN(c->astgen.token_value_read_idx, hcc_stack_count(c->astgen.token_bag.token_values) - 1)];
	return value;
}

HccTokenValue hcc_astgen_token_value_next(HccCompiler* c) {
	HccTokenValue value = c->astgen.token_bag.token_values[HCC_MIN(c->astgen.token_value_read_idx, hcc_stack_count(c->astgen.token_bag.token_values) - 1)];
	c->astgen.token_value_read_idx += 1;
	return value;
}

void hcc_astgen_data_type_found(HccCompiler* c, HccDataType data_type) {
	*hcc_stack_push(c->astgen.ordered_data_types) = data_type;
}

HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name(HccCompiler* c, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id) {
	hcc_stack_clear(c->astgen.compound_type_find_fields);
	return hcc_astgen_compound_data_type_find_field_by_name_recursive(c, compound_data_type, identifier_string_id);
}

HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_checked(HccCompiler* c, HccDataType data_type, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id) {
	HccCompoundField* field = hcc_astgen_compound_data_type_find_field_by_name(c, compound_data_type, identifier_string_id);
	if (field == NULL) {
		HccString data_type_name = hcc_data_type_string(c, data_type);
		HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
		hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_CANNOT_FIND_FIELD, compound_data_type->identifier_token_idx, (int)identifier_string.size, identifier_string.data, (int)data_type_name.size, data_type_name.data);
	}
	return field;
}

HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_recursive(HccCompiler* c, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id) {
	HccFieldAccess* access = hcc_stack_push(c->astgen.compound_type_find_fields);
	for (U32 field_idx = 0; field_idx < compound_data_type->fields_count; field_idx += 1) {
		HccCompoundField* field = &c->astgen.compound_fields[compound_data_type->fields_start_idx + field_idx];
		access->data_type = field->data_type;
		access->idx = field_idx;

		if (field->identifier_string_id.idx_plus_one == 0) {
			//
			// we have an anonymous compound data type as a field.
			// so nest down and search for the identifier in there too
			HccCompoundDataType* field_compound_data_type = hcc_compound_data_type_get(c, field->data_type);
			HccCompoundField* nested_field = hcc_astgen_compound_data_type_find_field_by_name_recursive(c, field_compound_data_type, identifier_string_id);
			if (nested_field) {
				return nested_field;
			}
		}

		if (field->identifier_string_id.idx_plus_one == identifier_string_id.idx_plus_one) {
			// found a field so return success
			return field;
		}
	}

	//
	// no luck finding the field, so remove this result and return failure
	hcc_stack_pop(c->astgen.compound_type_find_fields);
	return NULL;
}

void hcc_astgen_static_variable_usage_found(HccCompiler* c, HccDecl decl) {
	bool found = false;
	U32 end_idx = hcc_stack_count(c->astgen.function_used_static_variables);
	for (U32 idx = c->astgen.function->used_static_variables_start_idx; idx < end_idx; idx += 1) {
		if (c->astgen.function_used_static_variables[idx] == decl) {
			found = true;
			break;
		}
	}
	if (!found) {
		*hcc_stack_push(c->astgen.function_used_static_variables) = decl;
	}
}

void hcc_astgen_insert_global_declaration(HccCompiler* c, HccStringId identifier_string_id, HccDecl decl) {
	HccDecl* decl_ptr;
	if (hcc_hash_table_find_or_insert(&c->astgen.global_declarations, identifier_string_id.idx_plus_one, &decl_ptr)) {
		U32 other_token_idx = hcc_decl_token_idx(c, *decl_ptr);
		HccString string = hcc_string_table_get(&c->string_table, identifier_string_id);
		hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_GLOBAL, other_token_idx, (int)string.size, string.data);
	}
	*decl_ptr = decl;
}

void hcc_astgen_eval_cast(HccCompiler* c, HccExpr* expr, HccDataType dst_data_type) {
	HCC_DEBUG_ASSERT(expr->type == HCC_EXPR_TYPE_CONSTANT, "internal error: expected to be evaluating a constant");
	HccConstantId constant_id = { .idx_plus_one = expr->constant.id };
	HccConstant constant = hcc_constant_table_get(c, constant_id);

	HccBasic basic;
	if (HCC_DATA_TYPE_IS_UINT(c, expr->data_type)) {
		U64 uint;
		hcc_constant_as_uint(c, constant, &uint);
		basic = hcc_basic_from_uint(c, dst_data_type, uint);
	} else if (HCC_DATA_TYPE_IS_SINT(c, expr->data_type)) {
		S64 sint;
		hcc_constant_as_sint(c, constant, &sint);
		basic = hcc_basic_from_sint(c, dst_data_type, sint);
	} else if (HCC_DATA_TYPE_IS_FLOAT(expr->data_type)) {
		F64 float_;
		hcc_constant_as_float(c, constant, &float_);
		basic = hcc_basic_from_float(c, dst_data_type, float_);
	} else {
		HCC_ABORT("unhandled data type '%u'", expr->data_type);
	}

	constant_id = hcc_constant_table_deduplicate_basic(c, dst_data_type, &basic);
	expr->constant.id = constant_id.idx_plus_one;
	expr->data_type = dst_data_type;
}

bool hcc_stmt_has_return(HccExpr* stmt) {
	if (stmt->type == HCC_EXPR_TYPE_STMT_RETURN) {
		return true;
	} else if (stmt->type == HCC_EXPR_TYPE_STMT_BLOCK && stmt->stmt_block.has_return_stmt) {
		return true;
	} else if (stmt->type == HCC_EXPR_TYPE_STMT_IF && stmt[stmt->if_.true_stmt_rel_idx].if_aux.true_and_false_stmts_have_return_stmt) {
		return true;
	}
	return false;
}

HccExpr* hcc_astgen_alloc_expr(HccCompiler* c, HccExprType type) {
	HccExpr* expr = hcc_stack_push(c->astgen.exprs);
	expr->type = type;
	expr->is_stmt_block_entry = false;
	return expr;
}

HccExpr* hcc_astgen_alloc_expr_many(HccCompiler* c, U32 amount) {
	return hcc_stack_push_many(c->astgen.exprs, amount);
}

const char* hcc_type_specifier_string(HccTypeSpecfier specifier) {
	switch (specifier) {
		case HCC_TYPE_SPECIFIER_VOID: return "void";
		case HCC_TYPE_SPECIFIER_BOOL: return "_Bool";
		case HCC_TYPE_SPECIFIER_CHAR: return "char";
		case HCC_TYPE_SPECIFIER_SHORT: return "short";
		case HCC_TYPE_SPECIFIER_INT: return "int";
		case HCC_TYPE_SPECIFIER_LONG: return "long";
		case HCC_TYPE_SPECIFIER_LONGLONG: return "long long";
		case HCC_TYPE_SPECIFIER_FLOAT: return "float";
		case HCC_TYPE_SPECIFIER_DOUBLE: return "double";
		case HCC_TYPE_SPECIFIER_GENERIC_FLOAT: return "__hcc_generic_float";
		case HCC_TYPE_SPECIFIER_UNSIGNED: return "unsigned";
		case HCC_TYPE_SPECIFIER_SIGNED: return "signed";
		case HCC_TYPE_SPECIFIER_COMPLEX: return "_Complex";
		case HCC_TYPE_SPECIFIER_ATOMIC: return "_Atomic";
		default: return NULL;
	}
}

void hcc_astgen_data_type_ensure_is_condition(HccCompiler* c, HccDataType data_type) {
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
	if (!hcc_data_type_is_condition(c, data_type)) {
		HccString data_type_name = hcc_data_type_string(c, data_type);
		hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_CONDITION, (int)data_type_name.size, data_type_name.data);
	}
}

void hcc_astgen_compound_data_type_validate_field_names(HccCompiler* c, HccDataType outer_data_type, HccCompoundDataType* compound_data_type) {
	for (U32 field_idx = 0; field_idx < compound_data_type->fields_count; field_idx += 1) {
		HccCompoundField* field = &c->astgen.compound_fields[compound_data_type->fields_start_idx + field_idx];
		if (field->identifier_string_id.idx_plus_one == 0) {
			HccCompoundDataType* field_compound_data_type = hcc_compound_data_type_get(c, field->data_type);
			hcc_astgen_compound_data_type_validate_field_names(c, outer_data_type, field_compound_data_type);
		} else {
			U32* dst_token_idx;
			bool result = hcc_hash_table_find_or_insert(&c->astgen.field_name_to_token_idx, field->identifier_string_id.idx_plus_one, &dst_token_idx);
			if (result) {
				c->astgen.token_read_idx = field->identifier_token_idx;
				HccString field_identifier_string = hcc_string_table_get(&c->string_table, field->identifier_string_id);
				HccString data_type_name = hcc_data_type_string(c, outer_data_type);
				hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_DUPLICATE_FIELD_IDENTIFIER, *dst_token_idx, (int)field_identifier_string.size, field_identifier_string.data, (int)data_type_name.size, data_type_name.data);
			}
			*dst_token_idx = field->identifier_token_idx;
		}
	}
}

void hcc_astgen_validate_specifiers(HccCompiler* c, HccSpecifierFlags non_specifiers, HccErrorCode invalid_specifier_error_code) {
	while (c->astgen.specifier_flags & non_specifiers) {
		HccSpecifier specifier = HCC_LEAST_SET_BIT_IDX_U32(c->astgen.specifier_flags & non_specifiers);
		HccToken token = hcc_specifier_tokens[specifier];
		hcc_astgen_error_1(c, invalid_specifier_error_code, hcc_token_strings[token]);
		c->astgen.specifier_flags = HCC_LEAST_SET_BIT_REMOVE(c->astgen.specifier_flags);
	}
}

void hcc_astgen_ensure_semicolon(HccCompiler* c) {
	HccToken token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_SEMICOLON) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_MISSING_SEMICOLON);
	}
	hcc_astgen_token_next(c);
}

void hcc_astgen_ensure_not_unsupported_basic_type(HccCompiler* c, U32 num_tokens_behind, HccErrorCode error_code, HccDataType data_type) {
	data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);
	HccIntrinsicBasicTypeMask mask = hcc_data_type_has_intrinsic_basic_types(c, data_type) & ~c->spirvgen.available_basic_types;
	if (mask) {
		U32 other_token_idx = hcc_data_type_token_idx(c, data_type);
		HccString data_type_string = hcc_data_type_string(c, data_type);
		HccString mask_string = hcc_intrinsic_basic_type_mask_string(mask);
		c->astgen.token_read_idx -= num_tokens_behind;

		U32 token_idx = HCC_MIN(c->astgen.token_read_idx, hcc_stack_count(c->astgen.token_bag.tokens) - 1);
		HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
		HccLocation* other_location = other_token_idx == U32_MAX ? NULL : hcc_token_bag_location_get(c, &c->astgen.token_bag, other_token_idx);
		c->message_sys.next_is_deferred = true;
		hcc_error_push(c, error_code, location, other_location, (int)data_type_string.size, data_type_string.data, (int)mask_string.size, mask_string.data);

		c->astgen.token_read_idx += num_tokens_behind;
	}
}

bool hcc_data_type_check_compatible_assignment(HccCompiler* c, HccDataType target_data_type, HccExpr** source_expr_mut) {
	HccExpr* source_expr = *source_expr_mut;
	HccDataType source_data_type = source_expr->data_type;
	if (HCC_DATA_TYPE_IS_CONST(target_data_type) && !HCC_DATA_TYPE_IS_CONST(source_data_type)) {
		return false;
	}

	target_data_type = hcc_typedef_resolve_and_strip_const(c, target_data_type);
	source_data_type = hcc_typedef_resolve_and_strip_const(c, source_data_type);
	if (target_data_type == source_data_type) {
		return true;
	}

	if (target_data_type == HCC_DATA_TYPE_GENERIC_FLOAT && (source_data_type == HCC_DATA_TYPE_FLOAT || source_data_type == HCC_DATA_TYPE_DOUBLE)) {
		return true;
	}

	if (HCC_DATA_TYPE_IS_BASIC(target_data_type) && HCC_DATA_TYPE_IS_BASIC(source_data_type)) {
		hcc_astgen_generate_implicit_cast(c, target_data_type, source_expr_mut);
		return true;
	}

	return false;
}

void hcc_data_type_ensure_compatible_assignment(HccCompiler* c, U32 other_token_idx, HccDataType target_data_type, HccExpr** source_expr_mut) {
	if (!hcc_data_type_check_compatible_assignment(c, target_data_type, source_expr_mut)) {
		HccString target_data_type_name = hcc_data_type_string(c, target_data_type);
		HccString source_data_type_name = hcc_data_type_string(c, (*source_expr_mut)->data_type);
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_TYPE_MISMATCH_IMPLICIT_CAST, other_token_idx, (int)source_data_type_name.size, source_data_type_name.data, (int)target_data_type_name.size, target_data_type_name.data);
	}
}

bool hcc_data_type_check_compatible_arithmetic(HccCompiler* c, HccExpr** left_expr_mut, HccExpr** right_expr_mut) {
	HccExpr* left_expr = *left_expr_mut;
	HccExpr* right_expr = *right_expr_mut;

	HccDataType left_data_type = hcc_typedef_resolve_and_strip_const(c, left_expr->data_type);
	HccDataType right_data_type = hcc_typedef_resolve_and_strip_const(c, right_expr->data_type);
	if (left_data_type == right_data_type) {
		return true;
	}

	if (HCC_DATA_TYPE_IS_BASIC(left_data_type) && HCC_DATA_TYPE_IS_BASIC(right_data_type)) {
		bool left_is_float = HCC_DATA_TYPE_IS_FLOAT(left_data_type);
		bool right_is_float = HCC_DATA_TYPE_IS_FLOAT(right_data_type);
		U8 left_rank = hcc_data_type_basic_type_ranks[left_data_type];
		U8 right_rank = hcc_data_type_basic_type_ranks[right_data_type];
		if (left_is_float || right_is_float) {
			//
			// if one of operands is a float, then cast lower ranked operand
			// into the type of the higher ranked operand
			if (left_rank < right_rank) {
				hcc_astgen_generate_implicit_cast(c, right_data_type, left_expr_mut);
			} else if (left_rank > right_rank) {
				hcc_astgen_generate_implicit_cast(c, left_data_type, right_expr_mut);
			}
		} else {
			//
			// both operands are integers
			//

			{
				//
				// promote each operand to an int if it has a lower rank
				//

				U8 int_rank = hcc_data_type_basic_type_ranks[HCC_DATA_TYPE_SINT];
				if (left_rank < int_rank) {
					hcc_astgen_generate_implicit_cast(c, HCC_DATA_TYPE_SINT, left_expr_mut);
					left_data_type = HCC_DATA_TYPE_SINT;
					left_rank = int_rank;
				}

				if (right_rank < int_rank) {
					hcc_astgen_generate_implicit_cast(c, HCC_DATA_TYPE_SINT, right_expr_mut);
					right_data_type = HCC_DATA_TYPE_SINT;
					right_rank = int_rank;
				}
			}

			if (left_data_type != right_data_type) {
				bool left_is_unsigned = HCC_DATA_TYPE_IS_UINT(c, left_data_type);
				bool right_is_unsigned = HCC_DATA_TYPE_IS_UINT(c, right_data_type);
				if (left_is_unsigned == right_is_unsigned) {
					//
					// both types are either unsigned or signed,
					// so cast the type with the lower rank to the higher ranked type.
					//

					if (left_rank < right_rank) {
						hcc_astgen_generate_implicit_cast(c, right_data_type, left_expr_mut);
					} else {
						hcc_astgen_generate_implicit_cast(c, left_data_type, right_expr_mut);
					}

					return true;
				}

				//
				// from here one of the operands is unsigned and the other is signed.
				//

				//
				// if unsigned operand has a higher or equal rank to the signed type,
				// then convert the other operand into the unsigned data type.
				//
				if (right_is_unsigned && left_rank <= right_rank) {
					hcc_astgen_generate_implicit_cast(c, right_data_type, left_expr_mut);
					return true;
				} else if (left_is_unsigned && left_rank >= right_rank) {
					hcc_astgen_generate_implicit_cast(c, left_data_type, right_expr_mut);
					return true;
				}

				//
				// check to see if the unsigned type can fit in the other signed type.
				// if it can, convert it into that.
				//
				if (left_is_unsigned && c->basic_type_size_and_aligns[left_data_type] < c->basic_type_size_and_aligns[right_data_type]) {
					hcc_astgen_generate_implicit_cast(c, right_data_type, left_expr_mut);
					return true;
				} else if (right_is_unsigned && c->basic_type_size_and_aligns[right_data_type] < c->basic_type_size_and_aligns[left_data_type]) {
					hcc_astgen_generate_implicit_cast(c, left_data_type, right_expr_mut);
					return true;
				}

				//
				// all of the other conversion have failed, so convert both expressions to
				// the unsigned version of the signed data type.
				//
				HccDataType unsigned_data_type = hcc_data_type_signed_to_unsigned(c, right_data_type);
				if (left_is_unsigned) {
					unsigned_data_type = hcc_data_type_signed_to_unsigned(c, right_data_type);
				} else {
					unsigned_data_type = hcc_data_type_signed_to_unsigned(c, left_data_type);
				}
				hcc_astgen_generate_implicit_cast(c, unsigned_data_type, left_expr_mut);
				hcc_astgen_generate_implicit_cast(c, unsigned_data_type, right_expr_mut);
			}
		}

		return true;
	}

	return false;
}

void hcc_data_type_ensure_compatible_arithmetic(HccCompiler* c, U32 other_token_idx, HccExpr** left_expr_mut, HccExpr** right_expr_mut, HccToken operator_token) {
	if (!hcc_data_type_check_compatible_arithmetic(c, left_expr_mut, right_expr_mut)) {
		HccString left_data_type_name = hcc_data_type_string(c, (*left_expr_mut)->data_type);
		HccString right_data_type_name = hcc_data_type_string(c, (*right_expr_mut)->data_type);
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_UNSUPPORTED_BINARY_OPERATOR, other_token_idx, hcc_token_strings[operator_token], (int)right_data_type_name.size, right_data_type_name.data, (int)left_data_type_name.size, left_data_type_name.data);
	}
}

void hcc_astgen_ensure_function_args_count(HccCompiler* c, HccFunction* function, U32 args_count) {
	if (args_count < function->params_count) {
		HccString string = hcc_string_table_get(&c->string_table, function->identifier_string_id);
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_NOT_ENOUGH_FUNCTION_ARGS, function->identifier_token_idx, function->params_count, args_count, (int)string.size, string.data);
	} else if (args_count > function->params_count) {
		HccString string = hcc_string_table_get(&c->string_table, function->identifier_string_id);
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_TOO_MANY_FUNCTION_ARGS, function->identifier_token_idx, function->params_count, args_count, (int)string.size, string.data);
	}
}

HccDataType hcc_astgen_deduplicate_array_data_type(HccCompiler* c, HccDataType element_data_type, HccConstantId size_constant_id) {
	element_data_type = hcc_typedef_resolve_and_strip_const(c, element_data_type);
	U32 count = hcc_stack_count(c->astgen.array_data_types);
	for (U32 i = 0; i < count; i += 1) {
		HccArrayDataType* d = hcc_stack_get(c->astgen.array_data_types, i);
		if (d->element_data_type == element_data_type && d->size_constant_id.idx_plus_one == size_constant_id.idx_plus_one) {
			return HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_ARRAY, i);
		}
	}

	U32 array_data_types_idx = count;
	HccArrayDataType* d = hcc_stack_push(c->astgen.array_data_types);
	d->element_data_type = element_data_type;
	d->size_constant_id = size_constant_id;

	HccDataType data_type = HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_ARRAY, array_data_types_idx);
	hcc_astgen_data_type_found(c, data_type);
	return data_type;
}

void _hcc_astgen_ensure_no_unused_specifiers(HccCompiler* c, char* what) {
	if (c->astgen.specifier_flags) {
		HccToken keyword_token;
		if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_STATIC) {
			keyword_token = HCC_TOKEN_KEYWORD_STATIC;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_CONST) {
			keyword_token = HCC_TOKEN_KEYWORD_CONST;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_INLINE) {
			keyword_token = HCC_TOKEN_KEYWORD_INLINE;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_NO_RETURN) {
			keyword_token = HCC_TOKEN_KEYWORD_NO_RETURN;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_INTRINSIC) {
			keyword_token = HCC_TOKEN_KEYWORD_INTRINSIC;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_RASTERIZER_STATE) {
			keyword_token = HCC_TOKEN_KEYWORD_RASTERIZER_STATE;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_FRAGMENT_STATE) {
			keyword_token = HCC_TOKEN_KEYWORD_FRAGMENT_STATE;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_POSITION) {
			keyword_token = HCC_TOKEN_KEYWORD_POSITION;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_NOINTERP) {
			keyword_token = HCC_TOKEN_KEYWORD_NOINTERP;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_VERTEX) {
			keyword_token = HCC_TOKEN_KEYWORD_VERTEX;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_FRAGMENT) {
			keyword_token = HCC_TOKEN_KEYWORD_FRAGMENT;
		}
		hcc_astgen_error_1(c, HCC_ERROR_CODE_UNUSED_SPECIFIER, hcc_token_strings[keyword_token], what, hcc_token_strings[hcc_astgen_token_peek(c)]);
	}
}

void hcc_astgen_ensure_no_unused_specifiers_data_type(HccCompiler* c) {
	_hcc_astgen_ensure_no_unused_specifiers(c, "a data type");
}

void hcc_astgen_ensure_no_unused_specifiers_identifier(HccCompiler* c) {
	_hcc_astgen_ensure_no_unused_specifiers(c, "an identifier");
}

void hcc_astgen_variable_stack_open(HccCompiler* c) {
	if (hcc_stack_count(c->astgen.variable_stack_strings) == 0) {
		c->astgen.next_var_idx = 0;
	}
	hcc_stack_push(c->astgen.variable_stack_strings)->idx_plus_one = 0;
	*hcc_stack_push(c->astgen.variable_stack_var_indices) = U32_MAX;
}

void hcc_astgen_variable_stack_close(HccCompiler* c) {
	while (hcc_stack_count(c->astgen.variable_stack_strings)) {
		bool end = hcc_stack_get_last(c->astgen.variable_stack_strings)->idx_plus_one == 0;
		hcc_stack_pop(c->astgen.variable_stack_strings);
		hcc_stack_pop(c->astgen.variable_stack_var_indices);
		if (end) {
			break;
		}
	}
}

U32 hcc_astgen_variable_stack_add(HccCompiler* c, HccStringId string_id) {
	U32 var_idx = c->astgen.next_var_idx;
	*hcc_stack_push(c->astgen.variable_stack_strings) = string_id;
	*hcc_stack_push(c->astgen.variable_stack_var_indices) = var_idx;
	c->astgen.next_var_idx += 1;
	return var_idx;
}

U32 hcc_astgen_variable_stack_find(HccCompiler* c, HccStringId string_id) {
	HCC_DEBUG_ASSERT(string_id.idx_plus_one, "string id is null");
	for (U32 idx = hcc_stack_count(c->astgen.variable_stack_strings); idx-- > 0;) {
		if (hcc_stack_get(c->astgen.variable_stack_strings, idx)->idx_plus_one == string_id.idx_plus_one) {
			return *hcc_stack_get(c->astgen.variable_stack_var_indices, idx) + 1;
		}
	}
	return 0;
}

HccToken hcc_astgen_curly_initializer_start(HccCompiler* c, HccDataType data_type, HccDataType resolved_data_type, HccExpr* first_expr) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;
	gen->elmt_data_type = data_type;
	gen->resolved_elmt_data_type = resolved_data_type;

	if (gen->first_initializer_expr) {
		HccAstGenCurlyInitializerNested* nested = hcc_stack_push(gen->nested);
		nested->first_initializer_expr = gen->first_initializer_expr;
		nested->prev_initializer_expr = gen->prev_initializer_expr;
		nested->nested_elmts_start_idx = gen->nested_elmts_start_idx;
	}

	//
	// start the initializer expression linked list with
	// a designated initializer to zero the whole initial composite type.
	gen->first_initializer_expr = first_expr;
	gen->prev_initializer_expr = first_expr;
	gen->nested_elmts_start_idx = hcc_stack_count(gen->nested_elmts);
	HccExpr* initializer_expr = hcc_astgen_curly_initializer_generate_designated_initializer(c);
	initializer_expr->designated_initializer.value_expr_rel_idx = 0;

	return hcc_astgen_curly_initializer_open(c);
}

HccToken hcc_astgen_curly_initializer_open(HccCompiler* c) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;
	if (!HCC_DATA_TYPE_IS_COMPOSITE_TYPE(gen->resolved_elmt_data_type)) {
		if (gen->elmt_data_type == HCC_DATA_TYPE_VOID) {
			// this is only happens for the first opening initializer when it is not used for an assignment.
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_CURLY_EXPR);
		} else {
			HccString data_type_name = hcc_data_type_string(c, gen->elmt_data_type);
			hcc_astgen_warn_1(c, HCC_WARN_CODE_CURLY_INITIALIZER_ON_SCALAR, (int)data_type_name.size, data_type_name.data);
		}
	}

	*hcc_stack_push(gen->nested_curlys) = (HccAstGenCurlyInitializerCurly) {
		.nested_elmts_start_idx = hcc_stack_count(gen->nested_elmts) + 1,
		.found_designator = false,
	};

	hcc_astgen_curly_initializer_tunnel_in(c);
	return hcc_astgen_token_next(c);
}

HccToken hcc_astgen_curly_initializer_close(HccCompiler* c, bool is_finished) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;

	//
	// restore the elements back to where the parent curly initializer was inserted.
	// so we start searching from the parent curly initializer.
	U32 new_elmts_count = hcc_stack_get_last(gen->nested_curlys)->nested_elmts_start_idx - 1;
	hcc_stack_resize(gen->nested_elmts, new_elmts_count);
	hcc_stack_pop(gen->nested_curlys);

	if (is_finished) {
		return 0;
	}

	HccAstGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
	hcc_astgen_curly_initializer_set_composite(c, nested_elmt->data_type, nested_elmt->resolved_data_type);

	return hcc_astgen_token_next(c);
}

bool hcc_astgen_curly_initializer_next_elmt(HccCompiler* c, HccDataType resolved_target_data_type) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;

	//
	// tunnel in and out the composite data types until we reach the next non composite data type
	while (1) {
		HccAstGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
		HccAstGenCurlyInitializerCurly* curly = hcc_stack_get_last(gen->nested_curlys);
		bool had_explicit_designator_for_union_field = nested_elmt->had_explicit_designator_for_union_field;

		nested_elmt->elmt_idx += 1;
		nested_elmt->had_explicit_designator_for_union_field = false;

		if (!had_explicit_designator_for_union_field && nested_elmt->elmt_idx >= gen->elmts_end_idx) {
			//
			// the end of this initializer/data_type has been reached.
			// it could be either a composite or non composite data type.
			//

			if (curly->nested_elmts_start_idx == hcc_stack_count(gen->nested_elmts)) {
				//
				// here we have reached the end of the initializers for the explicit curly braces { }.
				// so we have a initializer that is targeting nothing.
				HccString data_type_name = hcc_data_type_string(c, gen->composite_data_type);
				hcc_astgen_warn_1(c, HCC_WARN_CODE_UNUSED_INITIALIZER_REACHED_END, (int)data_type_name.size, data_type_name.data);
				return false;
			} else {
				//
				// otherwise we are tunneled inside a nested composite data type.
				// which means one of it's parents has explicit curly braces.
				// so we just tunnel back out of our nested composite data type.
				hcc_astgen_curly_initializer_tunnel_out(c);
			}
		} else {
			if (HCC_DATA_TYPE_IS_COMPOUND_TYPE(gen->resolved_composite_data_type)) {
				//
				// this is a compound data type so fetch the next element data type
				gen->elmt_data_type = gen->compound_fields[nested_elmt->elmt_idx].data_type;
				gen->resolved_elmt_data_type = hcc_typedef_resolve_and_keep_const(c, gen->elmt_data_type);
			}

			if (resolved_target_data_type == gen->resolved_elmt_data_type) {
				return true;
			}

			if (resolved_target_data_type == HCC_DATA_TYPE_VOID) {
				return true;
			}

			if (HCC_DATA_TYPE_IS_COMPOSITE_TYPE(gen->resolved_elmt_data_type)) {
				hcc_astgen_curly_initializer_tunnel_in(c);
			} else {
				//
				// we found the non composite data type
				return true;
			}
		}
	}
}

HccToken hcc_astgen_curly_initializer_next_elmt_with_designator(HccCompiler* c) {
	HccToken token = hcc_astgen_token_peek(c);
	HCC_DEBUG_ASSERT(token == HCC_TOKEN_FULL_STOP || token == HCC_TOKEN_SQUARE_OPEN, "internal error: expected '.' or '['");
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;
	HccAstGenCurlyInitializerCurly* curly = hcc_stack_get_last(gen->nested_curlys);
	curly->found_designator = true;

	//
	// remove all of the excess nested elements so we are back on
	// the open curly braces that we are in.
	hcc_stack_resize(gen->nested_elmts, curly->nested_elmts_start_idx);
	HccAstGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
	hcc_astgen_curly_initializer_set_composite(c, nested_elmt->data_type, nested_elmt->resolved_data_type);

	//
	// parse and process the chain of designators for the composite types
	// eg: .field[0].another
	while (1) {
		switch (token) {
			case HCC_TOKEN_FULL_STOP:
				if (HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					HccString data_type_name = hcc_data_type_string(c, gen->composite_data_type);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FIELD_DESIGNATOR_ON_ARRAY_TYPE, (int)data_type_name.size, data_type_name.data);
				}

				token = hcc_astgen_token_next(c);
				if (token != HCC_TOKEN_IDENT) {
					HccString data_type_name = hcc_data_type_string(c, gen->composite_data_type);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_DESIGNATOR, (int)data_type_name.size, data_type_name.data);
				}

				HccStringId identifier_string_id = hcc_astgen_token_value_next(c).string_id;
				hcc_astgen_compound_data_type_find_field_by_name_checked(c, gen->composite_data_type, gen->compound_data_type, identifier_string_id);
				for (U32 i = 0; i < hcc_stack_count(c->astgen.compound_type_find_fields); i += 1) {
					HccFieldAccess* field = hcc_stack_get(c->astgen.compound_type_find_fields, i);
					hcc_stack_get_last(gen->nested_elmts)->elmt_idx = field->idx;
					hcc_astgen_curly_initializer_nested_elmt_push(c, field->data_type, hcc_typedef_resolve_and_keep_const(c, field->data_type));
				}

				if (hcc_stack_count(c->astgen.compound_type_find_fields) > 1) {
					gen->composite_data_type = hcc_stack_get_back(c->astgen.compound_type_find_fields, 1)->data_type;
					gen->resolved_composite_data_type = hcc_typedef_resolve_and_keep_const(c, gen->composite_data_type);
					gen->compound_data_type = hcc_compound_data_type_get(c, gen->resolved_composite_data_type);
					gen->compound_fields = hcc_stack_get(c->astgen.compound_fields, gen->compound_data_type->fields_start_idx);
				}

				gen->elmt_data_type = hcc_stack_get_last(c->astgen.compound_type_find_fields)->data_type;
				gen->resolved_elmt_data_type = hcc_typedef_resolve_and_keep_const(c, gen->elmt_data_type);
				token = hcc_astgen_token_next(c);
				break;
			case HCC_TOKEN_SQUARE_OPEN:
				if (!HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					HccString data_type_name = hcc_data_type_string(c, gen->composite_data_type);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ARRAY_DESIGNATOR_ON_COMPOUND_TYPE, (int)data_type_name.size, data_type_name.data);
				}

				token = hcc_astgen_token_next(c);
				HccExpr* expr = hcc_astgen_generate_expr(c, 0);
				if (expr->type != HCC_EXPR_TYPE_CONSTANT || !HCC_DATA_TYPE_IS_INT(expr->data_type)) {
					HccString data_type_name = hcc_data_type_string(c, gen->composite_data_type);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_INTEGER_FOR_ARRAY_IDX, (int)data_type_name.size, data_type_name.data);
				}

				HccConstantId value_constant_id = { .idx_plus_one = expr->constant.id };
				HccConstant constant = hcc_constant_table_get(c, value_constant_id);
				U64 elmt_idx;
				if (!hcc_constant_as_uint(c, constant, &elmt_idx) || elmt_idx >= gen->elmts_end_idx) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ARRAY_INDEX_OUT_OF_BOUNDS, gen->elmts_end_idx);
				}

				token = hcc_astgen_token_peek(c);
				if (token != HCC_TOKEN_SQUARE_CLOSE) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ARRAY_DESIGNATOR_EXPECTED_SQUARE_BRACE_CLOSE);
				}
				token = hcc_astgen_token_next(c);

				hcc_stack_get_last(gen->nested_elmts)->elmt_idx = elmt_idx;
				hcc_astgen_curly_initializer_nested_elmt_push(c, gen->elmt_data_type, gen->resolved_elmt_data_type);
				break;
			case HCC_TOKEN_EQUAL:
				goto END;
			default: {
				//
				// we reach here when we have looped at least once and we do not have a '=', '.' or '[' with a compatible composite data type
				HccErrorCode error_code;
				if (HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					error_code = HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_ARRAY_DESIGNATOR;
				} else if (HCC_DATA_TYPE_IS_COMPOUND_TYPE(gen->resolved_composite_data_type)) {
					error_code = HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_FIELD_DESIGNATOR;
				} else {
					HccString data_type_name = hcc_data_type_string(c, gen->composite_data_type);
					HCC_UNREACHABLE("we only handle array and compound types here right? but we got '%.*s'", (int)data_type_name.size, data_type_name.data);
				}
				hcc_astgen_bail_error_1(c, error_code);
			};
		}

		if (token == HCC_TOKEN_EQUAL) {
			// we reach here after processing a array or field designator that is followed by a '='
			goto END;
		} else if (!HCC_DATA_TYPE_IS_COMPOSITE_TYPE(gen->resolved_elmt_data_type)) {
			HccString data_type_name = hcc_data_type_string(c, gen->elmt_data_type);
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_ASSIGN, (int)data_type_name.size, data_type_name.data);
		}

		hcc_astgen_curly_initializer_set_composite(c, gen->elmt_data_type, gen->resolved_elmt_data_type);
	}
END: {}
	token = hcc_astgen_token_next(c);

	//
	// pop off the last nested element since we don't
	// want to tunnel into the last designator in the chain.
	hcc_stack_pop(gen->nested_elmts);

	if (token != HCC_TOKEN_CURLY_OPEN) {
		// take away one as the next call to hcc_astgen_curly_initializer_next_elmt
		// will increment it past where the designator has specified.
		nested_elmt = hcc_stack_get_last(gen->nested_elmts);
		nested_elmt->elmt_idx -= 1;
		nested_elmt->had_explicit_designator_for_union_field = HCC_DATA_TYPE_IS_UNION(gen->resolved_composite_data_type);
	}
	return token;
}

void hcc_astgen_curly_initializer_nested_elmt_push(HccCompiler* c, HccDataType data_type, HccDataType resolved_data_type) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;
	*hcc_stack_push(gen->nested_elmts) = (HccAstGenCurlyInitializerElmt) {
		.elmt_idx = -1, // start on -1 so that it'll be 0 after the first call to hcc_astgen_curly_initializer_next_elmt
		.data_type =  data_type,
		.resolved_data_type = resolved_data_type,
	};
}

void hcc_astgen_curly_initializer_tunnel_in(HccCompiler* c) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;
	hcc_astgen_curly_initializer_nested_elmt_push(c, gen->elmt_data_type, gen->resolved_elmt_data_type);
	hcc_astgen_curly_initializer_set_composite(c, gen->elmt_data_type, gen->resolved_elmt_data_type);
}

void hcc_astgen_curly_initializer_tunnel_out(HccCompiler* c) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;
	hcc_stack_pop(gen->nested_elmts);
	HccAstGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
	hcc_astgen_curly_initializer_set_composite(c, nested_elmt->data_type, nested_elmt->resolved_data_type);
}

void hcc_astgen_curly_initializer_set_composite(HccCompiler* c, HccDataType data_type, HccDataType resolved_data_type) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;

	gen->composite_data_type = data_type;
	gen->resolved_composite_data_type = resolved_data_type;
	if (HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
		gen->array_data_type = hcc_array_data_type_get(c, gen->resolved_composite_data_type);

		HccConstant constant = hcc_constant_table_get(c, gen->array_data_type->size_constant_id);
		U64 cap;
		hcc_constant_as_uint(c, constant, &cap);

		gen->elmt_data_type = gen->array_data_type->element_data_type;
		gen->resolved_elmt_data_type = hcc_typedef_resolve_and_keep_const(c, gen->elmt_data_type);
		gen->elmts_end_idx = cap;
	} else if (HCC_DATA_TYPE_IS_COMPOUND_TYPE(gen->resolved_composite_data_type)) {
		gen->compound_data_type = hcc_compound_data_type_get(c, gen->resolved_composite_data_type);
		gen->compound_fields = hcc_stack_get(c->astgen.compound_fields, gen->compound_data_type->fields_start_idx);
		gen->elmt_data_type = gen->compound_fields[0].data_type;
		gen->resolved_elmt_data_type = hcc_typedef_resolve_and_keep_const(c, gen->elmt_data_type);

		gen->elmts_end_idx = HCC_DATA_TYPE_IS_UNION(gen->resolved_composite_data_type) ? 1 : gen->compound_data_type->fields_count;
	} else {
		//
		// non composite data type that has explicit curly braces { }.
		gen->elmt_data_type = data_type;
		gen->resolved_elmt_data_type = resolved_data_type;
		gen->elmts_end_idx = 1;
	}
}

HccExpr* hcc_astgen_curly_initializer_generate_designated_initializer(HccCompiler* c) {
	HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;
	U32 elmt_indices_start_idx = gen->nested_elmts_start_idx;
	U32 elmt_indices_end_idx = hcc_stack_count(gen->nested_elmts);
	U32 elmt_indices_count = elmt_indices_end_idx - elmt_indices_start_idx;

	//
	// setup the auxillary data
	U32 designated_initializer_idx = hcc_stack_count(gen->designated_initializers);
	HccAstGenDesignatorInitializer* designated_initializer = hcc_stack_push(gen->designated_initializers);
	designated_initializer->elmt_indices_start_idx = hcc_stack_count(gen->designated_initializer_elmt_indices);
	designated_initializer->elmt_indices_count = elmt_indices_count;

	//
	// copy the element indices out into the persistant array
	U64* elmt_indices = hcc_stack_push_many(gen->designated_initializer_elmt_indices, elmt_indices_count);
	for (U32 idx = 0; idx < elmt_indices_count; idx += 1) {
		elmt_indices[idx] = hcc_stack_get(gen->nested_elmts, elmt_indices_start_idx + idx)->elmt_idx;
	}

	//
	// create the expression node and reference the auxillary data
	HccExpr* initializer_expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_DESIGNATED_INITIALIZER);
	initializer_expr->is_stmt_block_entry = true;
	initializer_expr->next_expr_rel_idx = 0;
	initializer_expr->alt_next_expr_rel_idx = designated_initializer_idx;

	//
	// append to the link list of designated initializers
	if (gen->prev_initializer_expr) {
		gen->prev_initializer_expr->next_expr_rel_idx = initializer_expr - gen->prev_initializer_expr;
	} else {
		gen->first_initializer_expr = initializer_expr;
	}
	gen->prev_initializer_expr = initializer_expr;

	return initializer_expr;
}

HccToken hcc_astgen_generate_specifiers(HccCompiler* c) {
	HccToken token = hcc_astgen_token_peek(c);
	while (1) {
		HccSpecifierFlags flag = 0;
		switch (token) {
			case HCC_TOKEN_KEYWORD_STATIC:           flag = HCC_SPECIFIER_FLAGS_STATIC;           break;
			case HCC_TOKEN_KEYWORD_CONST:            flag = HCC_SPECIFIER_FLAGS_CONST;            break;
			case HCC_TOKEN_KEYWORD_INLINE:           flag = HCC_SPECIFIER_FLAGS_INLINE;           break;
			case HCC_TOKEN_KEYWORD_NO_RETURN:        flag = HCC_SPECIFIER_FLAGS_NO_RETURN;        break;
			case HCC_TOKEN_KEYWORD_INTRINSIC:        flag = HCC_SPECIFIER_FLAGS_INTRINSIC;        break;
			case HCC_TOKEN_KEYWORD_RASTERIZER_STATE: flag = HCC_SPECIFIER_FLAGS_RASTERIZER_STATE; break;
			case HCC_TOKEN_KEYWORD_FRAGMENT_STATE:   flag = HCC_SPECIFIER_FLAGS_FRAGMENT_STATE;   break;
			case HCC_TOKEN_KEYWORD_POSITION:         flag = HCC_SPECIFIER_FLAGS_POSITION;         break;
			case HCC_TOKEN_KEYWORD_NOINTERP:         flag = HCC_SPECIFIER_FLAGS_NOINTERP;         break;
			case HCC_TOKEN_KEYWORD_VERTEX:           flag = HCC_SPECIFIER_FLAGS_VERTEX;           break;
			case HCC_TOKEN_KEYWORD_FRAGMENT:         flag = HCC_SPECIFIER_FLAGS_FRAGMENT;         break;
			case HCC_TOKEN_KEYWORD_AUTO: break;
			case HCC_TOKEN_KEYWORD_VOLATILE:
			case HCC_TOKEN_KEYWORD_EXTERN:
				hcc_astgen_error_1(c, HCC_ERROR_CODE_UNSUPPORTED_SPECIFIER, hcc_token_strings[token]);
			default: return token;
		}

		if (c->astgen.specifier_flags & flag) {
			hcc_astgen_error_1(c, HCC_ERROR_CODE_SPECIFIER_ALREADY_BEEN_USED, hcc_token_strings[token]);
		}
		c->astgen.specifier_flags |= flag;
		token = hcc_astgen_token_next(c);
	}
}

HccDataType hcc_astgen_generate_enum_data_type(HccCompiler* c) {
	HccToken token = hcc_astgen_token_peek(c);
	HCC_DEBUG_ASSERT(token == HCC_TOKEN_KEYWORD_ENUM, "internal error: expected 'enum' but got '%s'", hcc_token_strings[token]);
	token = hcc_astgen_token_next(c);

	HccDataType data_type;
	HccStringId identifier_string_id = {0};
	HccEnumDataType* enum_data_type = NULL;
	if (token == HCC_TOKEN_IDENT) {
		token = hcc_astgen_token_next(c);
		identifier_string_id = hcc_astgen_token_value_next(c).string_id;

		HccDataType* insert_value_ptr;
		if (hcc_hash_table_find_or_insert(&c->astgen.enum_declarations, identifier_string_id.idx_plus_one, &insert_value_ptr)) {
			data_type = *insert_value_ptr;
			enum_data_type = hcc_enum_data_type_get(c, data_type);
		} else {
			*insert_value_ptr = HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_ENUM, hcc_stack_count(c->astgen.enum_data_types));
			goto MAKE_NEW;
		}
	} else {
MAKE_NEW: {}
		U32 enum_data_type_idx = hcc_stack_count(c->astgen.enum_data_types);
		enum_data_type = hcc_stack_push(c->astgen.enum_data_types);
		memset(enum_data_type, 0x0, sizeof(*enum_data_type)); // TODO: maybe enforce the memory allocators to just give zeroed memory
		enum_data_type->identifier_token_idx = c->astgen.token_read_idx;
		enum_data_type->identifier_string_id = identifier_string_id;

		data_type = HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_ENUM, enum_data_type_idx);
	}

	if (token != HCC_TOKEN_CURLY_OPEN) {
		if (identifier_string_id.idx_plus_one) {
			return data_type;
		}
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ENUM);
	}

	if (enum_data_type->values_count) {
		HccString data_type_name = hcc_data_type_string(c, data_type);
		c->astgen.token_read_idx -= 1;
		hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_REIMPLEMENTATION, enum_data_type->identifier_token_idx, (int)data_type_name.size, data_type_name.data);
	}

	token = hcc_astgen_token_next(c);
	enum_data_type->identifier_token_idx = c->astgen.token_read_idx - 2;
	enum_data_type->values_start_idx = hcc_stack_count(c->astgen.enum_values);

	if (token == HCC_TOKEN_CURLY_CLOSE) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EMPTY_ENUM);
	}

	U32 value_idx = enum_data_type->values_start_idx;
	S64 next_value = 0;
	while (token != HCC_TOKEN_CURLY_CLOSE) {
		HccEnumValue* enum_value = hcc_stack_push(c->astgen.enum_values);

		if (token != HCC_TOKEN_IDENT) {
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_ENUM_VALUE);
		}

		if (next_value > S32_MAX) {
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ENUM_VALUE_OVERFLOW);
		}

		HccStringId value_identifier_string_id = hcc_astgen_token_value_next(c).string_id;
		enum_value->identifier_token_idx = c->astgen.token_read_idx;
		enum_value->identifier_string_id = value_identifier_string_id;

		HccDecl decl = HCC_DECL_INIT(HCC_DECL_ENUM_VALUE, value_idx);
		hcc_astgen_insert_global_declaration(c, value_identifier_string_id, decl);

		token = hcc_astgen_token_next(c);
		bool has_explicit_value = token == HCC_TOKEN_EQUAL;
		if (has_explicit_value) {
			token = hcc_astgen_token_next(c);

			HccExpr* expr = hcc_astgen_generate_expr_no_comma_operator(c, 0);
			if (expr->type != HCC_EXPR_TYPE_CONSTANT || !HCC_DATA_TYPE_IS_INT(expr->data_type)) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ENUM_VALUE_INVALID_FORMAT);
			}

			HccConstantId value_constant_id = { .idx_plus_one = expr->constant.id };
			HccConstant constant = hcc_constant_table_get(c, value_constant_id);

			S32 value;
			if (!hcc_constant_as_sint32(c, constant, &value)) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ENUM_VALUE_INVALID_FORMAT);
			}

			next_value = value;
			token = hcc_astgen_token_peek(c);
		}

		//
		// do not deduplicate when adding this constant to the constant table so we can pass in a debug name for the code generation to use for the enum value debug info
		HccBasic v = hcc_basic_from_sint(c, HCC_DATA_TYPE_SINT, next_value);
		HccConstantId value_constant_id = _hcc_constant_table_deduplicate_end(c, HCC_DATA_TYPE_SINT, &v, sizeof(S32), sizeof(S32), enum_value->identifier_string_id);

		enum_value->value_constant_id = value_constant_id;
		next_value += 1;

		if (token == HCC_TOKEN_COMMA) {
			token = hcc_astgen_token_next(c);
		} else if (token != HCC_TOKEN_CURLY_CLOSE) {
			HccErrorCode error_code = has_explicit_value
				? HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR_WITH_EXPLICIT_VALUE
				: HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR;
			hcc_astgen_bail_error_1(c, error_code);
		}

		value_idx += 1;
	}

	enum_data_type->values_count = value_idx - enum_data_type->values_start_idx;

	token = hcc_astgen_token_next(c);
	hcc_astgen_data_type_found(c, data_type);
	return data_type;
}

HccDataType hcc_astgen_generate_compound_data_type(HccCompiler* c) {
	U32 compound_data_type_token_idx = c->astgen.token_read_idx;
	HccToken token = hcc_astgen_token_peek(c);
	bool is_union = false;
	switch (token) {
		case HCC_TOKEN_KEYWORD_STRUCT: break;
		case HCC_TOKEN_KEYWORD_UNION:
			is_union = true;
			break;
		default:
			HCC_UNREACHABLE("internal error: expected 'struct' or 'union' but got '%s'", hcc_token_strings[token]);
	}
	token = hcc_astgen_token_next(c);

	HccDataType data_type;
	HccStringId identifier_string_id = {0};
	HccCompoundDataType* compound_data_type = NULL;
	U32 intrinsic_id = 0;
	if (token == HCC_TOKEN_IDENT) {
		compound_data_type_token_idx = c->astgen.token_read_idx;
		token = hcc_astgen_token_next(c);
		identifier_string_id = hcc_astgen_token_value_next(c).string_id;

		if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_INTRINSIC) {
			if (is_union) {
				if (HCC_STRING_ID_VEC_START <= identifier_string_id.idx_plus_one && identifier_string_id.idx_plus_one < HCC_STRING_ID_VEC_END) {
					HccVec vec = identifier_string_id.idx_plus_one - HCC_STRING_ID_VEC_START;
					intrinsic_id = HCC_UNION_IDX_VEC_START + vec + 1;
				} else if (HCC_STRING_ID_MAT_START <= identifier_string_id.idx_plus_one && identifier_string_id.idx_plus_one < HCC_STRING_ID_MAT_END) {
					HccMat mat = identifier_string_id.idx_plus_one - HCC_STRING_ID_MAT_START;
					intrinsic_id = HCC_UNION_IDX_MAT_START + mat + 1;
				} else {
					HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_STRUCT, (int)identifier_string.size, identifier_string.data);
				}
			} else {
				if (HCC_STRING_ID_PVEC_START <= identifier_string_id.idx_plus_one && identifier_string_id.idx_plus_one < HCC_STRING_ID_PVEC_END) {
					HccVec vec = identifier_string_id.idx_plus_one - HCC_STRING_ID_PVEC_START;
					intrinsic_id = HCC_STRUCT_IDX_PVEC_START + vec + 1;
				} else if (HCC_STRING_ID_PMAT_START <= identifier_string_id.idx_plus_one && identifier_string_id.idx_plus_one < HCC_STRING_ID_MAT_END) {
					HccMat mat = identifier_string_id.idx_plus_one - HCC_STRING_ID_PMAT_START;
					intrinsic_id = HCC_STRUCT_IDX_PMAT_START + mat + 1;
				} else {
					U32 i = 0;
					for (; i < HCC_ARRAY_COUNT(hcc_intrinsic_structs); i += 1) {
						HccIntrinsicStruct* s = &hcc_intrinsic_structs[i];
						if (s->string_id.idx_plus_one == identifier_string_id.idx_plus_one) {
							break;
						}
					}

					if (i == HCC_ARRAY_COUNT(hcc_intrinsic_structs)) {
						HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
						hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_STRUCT, (int)identifier_string.size, identifier_string.data);
					} else {
						intrinsic_id = i + 1;
					}
				}
			}

			c->astgen.specifier_flags &= ~HCC_SPECIFIER_FLAGS_INTRINSIC;
		}

		HccDataType* insert_value_ptr;
		HccHashTable(HccStringId, HccDataType)* declarations;
		if (is_union) {
			declarations = &c->astgen.union_declarations;
		} else {
			declarations = &c->astgen.struct_declarations;
		}

		if (hcc_hash_table_find_or_insert(declarations, identifier_string_id.idx_plus_one, &insert_value_ptr)) {
			data_type = *insert_value_ptr;
			compound_data_type = hcc_compound_data_type_get(c, data_type);
		} else {
			U32 insert_idx = intrinsic_id ? intrinsic_id - 1 : hcc_stack_count(c->astgen.compound_data_types);
			*insert_value_ptr = HCC_DATA_TYPE_INIT(is_union ? HCC_DATA_TYPE_UNION : HCC_DATA_TYPE_STRUCT, insert_idx);
			goto MAKE_NEW;
		}
	} else {
MAKE_NEW: {}
		U32 compound_data_type_idx;
		if (intrinsic_id) {
			compound_data_type_idx = intrinsic_id - 1;
			compound_data_type = hcc_stack_get(c->astgen.compound_data_types, compound_data_type_idx);
		} else {
			compound_data_type_idx = hcc_stack_count(c->astgen.compound_data_types);
			compound_data_type = hcc_stack_push(c->astgen.compound_data_types);
		}

		memset(compound_data_type, 0x0, sizeof(*compound_data_type)); // TODO see if we can just make the allocators give back zeroed memory instead
		compound_data_type->identifier_token_idx = c->astgen.token_read_idx;
		compound_data_type->identifier_string_id = identifier_string_id;
		if (is_union) {
			compound_data_type->flags |= HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION;
			data_type = HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_UNION, compound_data_type_idx);
		} else {
			data_type = HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_STRUCT, compound_data_type_idx);
		}
	}

	if (token != HCC_TOKEN_CURLY_OPEN) {
		if (identifier_string_id.idx_plus_one) {
			return data_type;
		}
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_COMPOUND_TYPE);
	}

	{
		hcc_astgen_validate_specifiers(c, HCC_SPECIFIER_FLAGS_ALL_NON_STRUCT_SPECIFIERS, HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT);
		if (c->astgen.specifier_flags) {
			//
			// ensure only one specifier is enabled
			if (!HCC_IS_POWER_OF_TWO_OR_ZERO(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS)) {
				hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT, hcc_token_strings[HCC_TOKEN_KEYWORD_RASTERIZER_STATE], hcc_token_strings[HCC_TOKEN_KEYWORD_FRAGMENT_STATE]);
			}

			U32 idx = HCC_LEAST_SET_BIT_IDX_U32(c->astgen.specifier_flags);
			if (is_union) {
				hcc_astgen_error_1(c, HCC_ERROR_CODE_NOT_AVAILABLE_FOR_UNION, hcc_token_strings[hcc_specifier_tokens[idx]]);
			}

			switch (idx) {
				case HCC_SPECIFIER_RASTERIZER_STATE:
					compound_data_type->flags |= HCC_COMPOUND_DATA_TYPE_FLAGS_IS_RASTERIZER_STATE;
					break;
				case HCC_SPECIFIER_FRAGMENT_STATE:
					compound_data_type->flags |= HCC_COMPOUND_DATA_TYPE_FLAGS_IS_FRAGMENT_STATE;
					break;
			}
		}

		c->astgen.specifier_flags &= ~HCC_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS;
	}

	if (compound_data_type->fields_count) {
		HccString data_type_name = hcc_data_type_string(c, data_type);
		c->astgen.token_read_idx -= 1;
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_REIMPLEMENTATION, compound_data_type->identifier_token_idx, (int)data_type_name.size, data_type_name.data);
	}

	token = hcc_astgen_token_next(c);
	compound_data_type->identifier_token_idx = c->astgen.token_read_idx - 2;
	compound_data_type->fields_start_idx = hcc_stack_count(c->astgen.compound_fields);

	//
	// scan ahead an count how many fields we are going to have.
	// this is because when generating fields we can recurse down
	// into generating another compound data type.
	U32 ahead_by = 0;
	U32 curly_open = 0; // to avoid counting the comma operator
	while (1) {
		HccToken token = hcc_astgen_token_peek_ahead(c, ahead_by);
		switch (token) {
			case HCC_TOKEN_EOF:
				goto END_FIELDS_COUNT;
			case HCC_TOKEN_SEMICOLON:
				if (curly_open == 0) {
					compound_data_type->fields_count += 1;
				}
				break;
			case HCC_TOKEN_CURLY_OPEN:
				curly_open += 1;
				break;
			case HCC_TOKEN_CURLY_CLOSE:
				if (curly_open == 0) {
					goto END_FIELDS_COUNT;
				}
				curly_open -= 1;
				break;
		}
		ahead_by += 1;
	}
END_FIELDS_COUNT: {}

	HccCompoundField* fields = hcc_stack_push_many(c->astgen.compound_fields, compound_data_type->fields_count);
	U32 field_idx = 0;
	bool found_position = false;
	while (1) { // for each field
		HccCompoundField* compound_field = &fields[field_idx];
		token = hcc_astgen_generate_specifiers(c);

		//
		// process the compound field specifiers
		//
		{
			if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_STRUCT_FIELD_SPECIFIERS) {
				HccSpecifier specifier = HCC_LEAST_SET_BIT_IDX_U32(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_STRUCT_FIELD_SPECIFIERS);
				HccToken token = hcc_specifier_tokens[specifier];
				hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT_FIELD, hcc_token_strings[token]);
			}

			if (c->astgen.specifier_flags) {
				if (!(compound_data_type->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_RASTERIZER_STATE)) {
					c->astgen.token_read_idx -= 1;
					hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_MISSING_RASTERIZER_STATE_SPECIFIER, compound_data_type_token_idx, hcc_token_strings[HCC_TOKEN_KEYWORD_RASTERIZER_STATE], hcc_token_strings[HCC_TOKEN_KEYWORD_POSITION], hcc_token_strings[HCC_TOKEN_KEYWORD_NOINTERP]);
				}

				//
				// ensure only one specifier is enabled
				if (!HCC_IS_POWER_OF_TWO_OR_ZERO(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS)) {
					hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT_FIELD, hcc_token_strings[HCC_TOKEN_KEYWORD_POSITION], hcc_token_strings[HCC_TOKEN_KEYWORD_NOINTERP]);
				}

				switch (HCC_LEAST_SET_BIT_IDX_U32(c->astgen.specifier_flags)) {
					case HCC_SPECIFIER_POSITION:
						if (found_position) {
							hcc_astgen_error_1(c, HCC_ERROR_CODE_POSITION_ALREADY_SPECIFIED, hcc_token_strings[HCC_TOKEN_KEYWORD_POSITION]);
						}
						found_position = true;
						compound_field->rasterizer_state_field_kind = HCC_RASTERIZER_STATE_FIELD_KIND_POSITION;
						break;
					case HCC_SPECIFIER_NOINTERP: compound_field->rasterizer_state_field_kind = HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP; break;
				}

				c->astgen.specifier_flags &= ~HCC_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS;
			}
		}

		U64 alignas_align = 0;
		if (token == HCC_TOKEN_KEYWORD_ALIGNAS) {
			token = hcc_astgen_token_next(c);
			if (token != HCC_TOKEN_PARENTHESIS_OPEN) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_ALIGNAS);
			}
			token = hcc_astgen_token_next(c);

			switch (token) {
				case HCC_TOKEN_LIT_SINT:
				case HCC_TOKEN_LIT_UINT:
				case HCC_TOKEN_LIT_SLONG:
				case HCC_TOKEN_LIT_ULONG: {
					token = hcc_astgen_token_next(c);
					HccConstantId constant_id = hcc_astgen_token_value_next(c).constant_id;
					HccConstant constant = hcc_constant_table_get(c, constant_id);

					//
					// skip the associated HccStringId kept around to turn the
					// literal back into the exact string it was parsed from.
					hcc_astgen_token_value_next(c);
					if (!hcc_constant_as_uint(c, constant, &alignas_align)) {
						hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_ALIGNAS_INT_CONSTANT);
					}
					break;
				};
				default: {
					HccDataType alignas_data_type;
					if (!hcc_astgen_generate_data_type(c, &alignas_data_type)) {
						hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_ALIGNAS_OPERAND);
					}

					U64 unused_size;
					hcc_data_type_size_align(c, alignas_data_type, &unused_size, &alignas_align);
					token = hcc_astgen_token_peek(c);
					break;
				};
			}

			if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_ALIGNAS);
			}
			token = hcc_astgen_token_next(c);
		}

		bool requires_name;
		switch (token) {
			case HCC_TOKEN_CURLY_CLOSE:
				goto END;
			case HCC_TOKEN_KEYWORD_STRUCT:
			case HCC_TOKEN_KEYWORD_UNION: {
				compound_field->data_type = hcc_astgen_generate_compound_data_type(c);
				requires_name = false;
				break;
			};
			case HCC_TOKEN_KEYWORD_ENUM: {
				compound_field->data_type = hcc_astgen_generate_enum_data_type(c);
				requires_name = true;
				break;
			};
			default: {
				if (!hcc_astgen_generate_data_type(c, &compound_field->data_type)) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_COMPOUND_FIELD_INVALID_TERMINATOR);
				}
				requires_name = true;
				break;
			};
		}

		HccDataType data_type = hcc_typedef_resolve_and_strip_const(c, compound_field->data_type);
		if (HCC_DATA_TYPE_IS_INTRINSIC_BASIC(data_type)) {
			HccIntrinsicType intrinsic_type = hcc_intrinsic_type_from_data_type(c, data_type);
			HCC_INTRINSIC_BASIC_TYPE_MASK_SET(&compound_data_type->has_intrinsic_basic_types, intrinsic_type);
		} else if (HCC_DATA_TYPE_IS_COMPOUND_TYPE(data_type)) {
			HccCompoundDataType* field_compound_data_type = hcc_compound_data_type_get(c, data_type);
			compound_data_type->has_intrinsic_basic_types |= field_compound_data_type->has_intrinsic_basic_types;
		}

		if ((compound_data_type->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_RASTERIZER_STATE)) {
			if (
				!HCC_DATA_TYPE_IS_BASIC(data_type) &&
				!HCC_DATA_TYPE_IS_VECTOR(data_type) &&
				!HCC_DATA_TYPE_IS_MATRIX(data_type)
			) {
				HccString data_type_name = hcc_data_type_string(c, compound_field->data_type);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_UNSUPPORTED_RASTERIZER_STATE_DATA_TYPE, (int)data_type_name.size, data_type_name.data);
			}

			if (
				compound_field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_POSITION &&
				data_type != HCC_DATA_TYPE_VECTOR(HCC_VEC4F)
			) {
				HccString expected_data_type_name = hcc_data_type_string(c, HCC_DATA_TYPE_VECTOR(HCC_VEC4F));
				HccString data_type_name = hcc_data_type_string(c, compound_field->data_type);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_POSITION_MUST_BE_VEC4_F32, hcc_token_strings[HCC_TOKEN_KEYWORD_POSITION], (int)expected_data_type_name.size, expected_data_type_name.data, (int)data_type_name.size, data_type_name.data);
			}
		}

		if ((compound_data_type->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_FRAGMENT_STATE)) {
			if (
				!HCC_DATA_TYPE_IS_BASIC(data_type) &&
				!HCC_DATA_TYPE_IS_VECTOR(data_type)
			) {
				HccString data_type_name = hcc_data_type_string(c, compound_field->data_type);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_UNSUPPORTED_FRAGMENT_STATE_DATA_TYPE, (int)data_type_name.size, data_type_name.data);
			}
		}

		token = hcc_astgen_token_peek(c);
		if (token != HCC_TOKEN_IDENT) {
			if (requires_name) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_COMPOUND_FIELD_MISSING_NAME);
			}

			compound_field->identifier_token_idx = 0;
			compound_field->identifier_string_id.idx_plus_one = 0;
		} else {
			HccStringId field_identifier_string_id = hcc_astgen_token_value_next(c).string_id;
			compound_field->identifier_token_idx = c->astgen.token_read_idx;
			compound_field->identifier_string_id = field_identifier_string_id;

			token = hcc_astgen_token_next(c);
			if (token == HCC_TOKEN_SQUARE_OPEN) {
				compound_field->data_type = hcc_astgen_generate_variable_decl_array(c, compound_field->data_type);
			}
		}
		hcc_astgen_ensure_semicolon(c);
		token = hcc_astgen_token_peek(c);

		//
		// TODO: in future this will be a platform specific problem
		U64 size;
		U64 align;
		hcc_data_type_size_align(c, compound_field->data_type, &size, &align);

		if (alignas_align) {
			if (alignas_align < align) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ALIGNAS_REDUCES_ALIGNMENT, alignas_align, align);
			}
			align = alignas_align;
		}

		if (is_union) {
			if (compound_data_type->size < size) {
				compound_data_type->largest_sized_field_idx = field_idx;
				compound_data_type->size = size;
			}
		} else {
			compound_data_type->size = HCC_INT_ROUND_UP_ALIGN(compound_data_type->size, align) + size;
		}
		compound_data_type->align = HCC_MAX(compound_data_type->align, align);

		field_idx += 1;
	}

END:{}
	hcc_hash_table_clear(&c->astgen.field_name_to_token_idx);
	hcc_astgen_compound_data_type_validate_field_names(c, data_type, compound_data_type);

	if (!is_union) {
		compound_data_type->size = HCC_INT_ROUND_UP_ALIGN(compound_data_type->size, compound_data_type->align);
	}

	if ((compound_data_type->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_RASTERIZER_STATE) && !found_position) {
		c->astgen.token_read_idx = compound_data_type_token_idx;
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_POSITION_NOT_SPECIFIED, hcc_token_strings[HCC_TOKEN_KEYWORD_POSITION]);
	}

	if (intrinsic_id) {
		intrinsic_id -= 1;
		if (intrinsic_id < HCC_STRUCT_IDX_INTRINSIC_END) {
			HccIntrinsicStruct* s = &hcc_intrinsic_structs[intrinsic_id];
			if (s->fields_count != compound_data_type->fields_count) {
				c->astgen.token_read_idx = compound_data_type_token_idx;
				HccString name = hcc_string_table_get(&c->string_table, identifier_string_id);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELDS_COUNT, (int)name.size, name.data, s->fields_count, compound_data_type->fields_count);
			}

			for (U32 field_idx = 0; field_idx < s->fields_count; field_idx += 1) {
				HccIntrinsicStructField* isf = &s->fields[field_idx];
				HccCompoundField* f = &fields[field_idx];
				HccString identifier_string = hcc_string_table_get(&c->string_table, f->identifier_string_id);
				if (isf->data_type != f->data_type || !hcc_string_eq(isf->name, identifier_string)) {
					c->astgen.token_read_idx = f->identifier_token_idx;
					HccString data_type_name = hcc_data_type_string(c, isf->data_type);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELD, (int)data_type_name.size, data_type_name.data, (int)isf->name.size, isf->name.data);
				}
			}
		} else if (HCC_STRUCT_IDX_PVEC_START <= intrinsic_id && intrinsic_id < HCC_STRUCT_IDX_PVEC_END) {
			HccVec vec = intrinsic_id - HCC_STRUCT_IDX_PVEC_START;
			U32 size = hcc_packed_vec_sizes[vec];
			U32 align = hcc_packed_vec_aligns[vec];
			if (compound_data_type->size != size || compound_data_type->align != align) {
				HccString data_type_name = hcc_string_table_get(&c->string_table, identifier_string_id);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_VECTOR_INVALID_SIZE_AND_ALIGN, (int)data_type_name.size, data_type_name.data, size, align, compound_data_type->size, compound_data_type->align);
			}
		} else if (HCC_UNION_IDX_VEC_START <= intrinsic_id && intrinsic_id < HCC_UNION_IDX_VEC_END) {
			HccVec vec = intrinsic_id - HCC_UNION_IDX_VEC_START;
			U32 size_and_align = hcc_vec_size_and_aligns[vec];
			if (compound_data_type->size != size_and_align || compound_data_type->align != size_and_align) {
				HccString data_type_name = hcc_string_table_get(&c->string_table, identifier_string_id);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_VECTOR_INVALID_SIZE_AND_ALIGN, (int)data_type_name.size, data_type_name.data, size_and_align, size_and_align, compound_data_type->size, compound_data_type->align);
			}
		} else if (HCC_STRUCT_IDX_PMAT_START <= intrinsic_id && intrinsic_id < HCC_STRUCT_IDX_PMAT_END) {
			HccMat mat = intrinsic_id - HCC_STRUCT_IDX_PMAT_START;
			U32 size = hcc_packed_mat_sizes[mat];
			U32 align = hcc_packed_mat_aligns[mat];
			if (compound_data_type->size != size || compound_data_type->align != align) {
				HccString data_type_name = hcc_string_table_get(&c->string_table, identifier_string_id);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_MATRIX_INVALID_SIZE_AND_ALIGN, (int)data_type_name.size, data_type_name.data, size, align, compound_data_type->size, compound_data_type->align);
			}
		} else if (HCC_UNION_IDX_MAT_START <= intrinsic_id && intrinsic_id < HCC_UNION_IDX_MAT_END) {
			HccMat mat = intrinsic_id - HCC_UNION_IDX_MAT_START;
			U32 size = hcc_mat_sizes[mat];
			U32 align = hcc_mat_aligns[mat];
			if (compound_data_type->size != size || compound_data_type->align != align) {
				HccString data_type_name = hcc_string_table_get(&c->string_table, identifier_string_id);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_MATRIX_INVALID_SIZE_AND_ALIGN, (int)data_type_name.size, data_type_name.data, size, align, compound_data_type->size, compound_data_type->align);
			}
		}
	}

	token = hcc_astgen_token_next(c);
	hcc_astgen_data_type_found(c, data_type);
	return data_type;
}

HccToken hcc_astgen_generate_type_specifiers(HccCompiler* c, HccLocation* location, HccTypeSpecfier* type_specifiers_mut) {
	HccToken token = hcc_astgen_token_peek(c);
	while (1) {
		HccTypeSpecfier found_specifier;
		switch (token) {
			case HCC_TOKEN_KEYWORD_VOID: found_specifier = HCC_TYPE_SPECIFIER_VOID; break;
			case HCC_TOKEN_KEYWORD_BOOL: found_specifier = HCC_TYPE_SPECIFIER_BOOL; break;
			case HCC_TOKEN_KEYWORD_CHAR: found_specifier = HCC_TYPE_SPECIFIER_CHAR; break;
			case HCC_TOKEN_KEYWORD_SHORT: found_specifier = HCC_TYPE_SPECIFIER_SHORT; break;
			case HCC_TOKEN_KEYWORD_INT: found_specifier = HCC_TYPE_SPECIFIER_INT; break;
			case HCC_TOKEN_KEYWORD_LONG: found_specifier = *type_specifiers_mut & HCC_TYPE_SPECIFIER_LONG ? HCC_TYPE_SPECIFIER_LONGLONG : HCC_TYPE_SPECIFIER_LONG; break;
			case HCC_TOKEN_KEYWORD_FLOAT: found_specifier = HCC_TYPE_SPECIFIER_FLOAT; break;
			case HCC_TOKEN_KEYWORD_DOUBLE: found_specifier = HCC_TYPE_SPECIFIER_DOUBLE; break;
			case HCC_TOKEN_KEYWORD_GENERIC_FLOAT: found_specifier = HCC_TYPE_SPECIFIER_GENERIC_FLOAT; break;
			case HCC_TOKEN_KEYWORD_UNSIGNED: found_specifier = HCC_TYPE_SPECIFIER_UNSIGNED; break;
			case HCC_TOKEN_KEYWORD_SIGNED: found_specifier = HCC_TYPE_SPECIFIER_SIGNED; break;
			case HCC_TOKEN_KEYWORD_COMPLEX: found_specifier = HCC_TYPE_SPECIFIER_COMPLEX; break;
			case HCC_TOKEN_KEYWORD_ATOMIC: found_specifier = HCC_TYPE_SPECIFIER_ATOMIC; break;
			default: return token;
		}

		if (*type_specifiers_mut & found_specifier) {
			hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_DUPLICATE_TYPE_SPECIFIER, location, hcc_type_specifier_string(found_specifier));
		}

		*type_specifiers_mut |= found_specifier;
		token = hcc_astgen_token_next(c);
	}
}

bool hcc_astgen_generate_data_type(HccCompiler* c, HccDataType* data_type_out) {
	HccToken token = hcc_astgen_token_peek(c);

	HccLocation location = *hcc_token_bag_location_get(c, &c->astgen.token_bag, c->astgen.token_read_idx);
	HccTypeSpecfier type_specifiers = 0;
	token = hcc_astgen_generate_type_specifiers(c, &location, &type_specifiers);

	HccDataType data_type = 0;
	if (!(type_specifiers & HCC_TYPE_SPECIFIER_TYPES)) {
		switch (token) {
			case HCC_TOKEN_KEYWORD_STRUCT:
			case HCC_TOKEN_KEYWORD_UNION: {
				data_type = hcc_astgen_generate_compound_data_type(c);
				hcc_astgen_generate_type_specifiers(c, &location, &type_specifiers);
				break;
			};
			case HCC_TOKEN_KEYWORD_ENUM:
				data_type = hcc_astgen_generate_enum_data_type(c);
				hcc_astgen_generate_type_specifiers(c, &location, &type_specifiers);
				break;
			case HCC_TOKEN_IDENT: {
				HccDecl decl;
				HccStringId identifier_string_id = hcc_astgen_token_value_peek(c).string_id;
				if (hcc_hash_table_find(&c->astgen.global_declarations, identifier_string_id.idx_plus_one, &decl)) {
					if (HCC_DECL_IS_DATA_TYPE(decl)) {
						data_type = decl;
						hcc_astgen_generate_type_specifiers(c, &location, &type_specifiers);
						hcc_astgen_token_value_next(c);
						hcc_astgen_token_next(c);
					}
				}
				break;
			};
		}
	}

	switch (data_type & 0xff) {
		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION:
		case HCC_DATA_TYPE_TYPEDEF:
		case HCC_DATA_TYPE_ENUM:
			if (type_specifiers & HCC_TYPE_SPECIFIER_ATOMIC) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ATOMIC_UNSUPPORTED_AT_THIS_TIME);
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
				hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, &location);
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_COMPLEX) {
				hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_COMPLEX_ON_NON_FLOAT_TYPE, &location);
			}

			break;

		default: {
			if (type_specifiers == 0) {
				return false;
			}

			U32 num_types = onebitscount32((type_specifiers & HCC_TYPE_SPECIFIER_TYPES) & ~HCC_TYPE_SPECIFIER_INT);
			if (
				num_types > 1 &&
				!(num_types == 2 && type_specifiers & HCC_TYPE_SPECIFIER_LONG_DOUBLE)
			) {
				hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_MULTIPLE_TYPES_SPECIFIED, &location);
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_FLOAT_TYPES) {
				if (type_specifiers & HCC_TYPE_SPECIFIER_COMPLEX) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_COMPLEX_UNSUPPORTED_AT_THIS_TIME);
				}

				if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
					hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, &location);
				}

				if ((type_specifiers & HCC_TYPE_SPECIFIER_LONG_DOUBLE) == HCC_TYPE_SPECIFIER_LONG_DOUBLE) {
					hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_LONG_DOUBLE_IS_UNSUPPORTED, &location);
				}

				if (type_specifiers & HCC_TYPE_SPECIFIER_FLOAT) {
					data_type = HCC_DATA_TYPE_FLOAT;
				} else if (type_specifiers & HCC_TYPE_SPECIFIER_DOUBLE) {
					data_type = HCC_DATA_TYPE_DOUBLE;
				} else if (type_specifiers & HCC_TYPE_SPECIFIER_GENERIC_FLOAT) {
					data_type = HCC_DATA_TYPE_GENERIC_FLOAT;
				} else {
					HCC_ABORT("wat?");
				}
				break;
			} else {
				if (type_specifiers & HCC_TYPE_SPECIFIER_COMPLEX) {
					hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_COMPLEX_ON_NON_FLOAT_TYPE, &location);
				}
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_VOID) {
				if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
					hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, &location);
				}

				data_type = HCC_DATA_TYPE_VOID;
				break;
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_BOOL) {
				if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
					hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, &location);
				}

				data_type = HCC_DATA_TYPE_BOOL;
				break;
			}

			if ((type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED_SIGNED) == HCC_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
				hcc_astgen_bail_error_1_merge_apply(c, HCC_ERROR_CODE_UNSIGNED_AND_SIGNED, &location);
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_CHAR) {
				if (type_specifiers == HCC_TYPE_SPECIFIER_CHAR) {
					data_type = HCC_DATA_TYPE_CHAR;
				} else if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE_UCHAR;
				} else {
					data_type = HCC_DATA_TYPE_SCHAR;
				}

				break;
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_SHORT) {
				if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE_USHORT;
				} else {
					data_type = HCC_DATA_TYPE_SSHORT;
				}

				break;
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_LONG) {
				if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE_ULONG;
				} else {
					data_type = HCC_DATA_TYPE_SLONG;
				}

				break;
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_LONGLONG) {
				if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE_ULONGLONG;
				} else {
					data_type = HCC_DATA_TYPE_SLONGLONG;
				}

				break;
			}

			if (type_specifiers & HCC_TYPE_SPECIFIER_INT) {
				if (type_specifiers & HCC_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE_UINT;
				} else {
					data_type = HCC_DATA_TYPE_SINT;
				}

				break;
			}

			break;
		};
	}

	*data_type_out = data_type;
	return true;
}

HccDataType hcc_astgen_generate_typedef(HccCompiler* c) {
	HCC_DEBUG_ASSERT(hcc_astgen_token_peek(c) == HCC_TOKEN_KEYWORD_TYPEDEF, "internal error: expected a typedef token");
	hcc_astgen_token_consume(c, 1);

	HccSpecifierFlags intrinsic_specifier_flags = c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_INTRINSIC;

	HccDataType aliased_data_type;
	if (!hcc_astgen_generate_data_type(c, &aliased_data_type)) {
		HccToken token = hcc_astgen_token_peek(c);
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_TYPE_NAME, hcc_token_strings[token]);
	}

	c->astgen.specifier_flags |= intrinsic_specifier_flags;
	return hcc_astgen_generate_typedef_with_data_type(c, aliased_data_type);
}

HccDataType hcc_astgen_generate_typedef_with_data_type(HccCompiler* c, HccDataType aliased_data_type) {
	HccToken token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_IDENT) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_TYPEDEF, hcc_token_strings[token]);
	}
	HccStringId identifier_string_id = hcc_astgen_token_value_next(c).string_id;

	hcc_astgen_validate_specifiers(c, HCC_SPECIFIER_FLAGS_ALL_NON_TYPEDEF_SPECIFIERS, HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_TYPEDEF);

	U32 intrinsic_id = 0;
	if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_INTRINSIC) {
		U32 i = 0;
		HccIntrinsicTypedef* it;
		for (; i < HCC_ARRAY_COUNT(hcc_intrinsic_typedefs); i += 1) {
			it = &hcc_intrinsic_typedefs[i];
			if (it->string_id.idx_plus_one == identifier_string_id.idx_plus_one) {
				break;
			}
		}

		if (i == HCC_ARRAY_COUNT(hcc_intrinsic_typedefs)) {
			HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_TYPEDEF, (int)identifier_string.size, identifier_string.data);
		}
		intrinsic_id = i + 1;

		if (it->aliased_data_type != HCC_DATA_TYPE_VOID && it->aliased_data_type != aliased_data_type) {
			HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
			HccString found_data_type_name = hcc_data_type_string(c, aliased_data_type);
			HccString data_type_name = hcc_data_type_string(c, it->aliased_data_type);
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_INVALID_TYPEDEF,
				(int)data_type_name.size, data_type_name.data, (int)identifier_string.size, identifier_string.data,
				(int)found_data_type_name.size, found_data_type_name.data, (int)identifier_string.size, identifier_string.data
			);
		}

		c->astgen.specifier_flags &= ~HCC_SPECIFIER_FLAGS_INTRINSIC;
	}

	HccDataType* insert_value_ptr;
	HccTypedef* typedef_ = NULL;
	HccDataType data_type;
	if (hcc_hash_table_find_or_insert(&c->astgen.global_declarations, identifier_string_id.idx_plus_one, &insert_value_ptr)) {
		data_type = *insert_value_ptr;
		typedef_ = hcc_typedef_get(c, data_type);
		if (typedef_->aliased_data_type != aliased_data_type) {
			HccString data_type_name = hcc_string_table_get(&c->string_table, identifier_string_id);
			U32 other_token_idx = hcc_data_type_token_idx(c, data_type);
			hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_GLOBAL, other_token_idx, (int)data_type_name.size, data_type_name.data);
		}
	} else {
		U32 typedef_idx;
		if (intrinsic_id) {
			typedef_idx = intrinsic_id - 1;
			typedef_ = &c->astgen.typedefs[typedef_idx];
		} else {
			typedef_idx = hcc_stack_count(c->astgen.typedefs);
			typedef_ = hcc_stack_push(c->astgen.typedefs);
		}

		data_type = HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_TYPEDEF, typedef_idx);
		typedef_->identifier_token_idx = c->astgen.token_read_idx;
		typedef_->identifier_string_id = identifier_string_id;
		typedef_->aliased_data_type = aliased_data_type;

		hcc_astgen_data_type_found(c, data_type);
		*insert_value_ptr = data_type;
	}

	hcc_astgen_token_consume(c, 1);
	hcc_astgen_ensure_semicolon(c);
	return data_type;
}

void hcc_astgen_generate_implicit_cast(HccCompiler* c, HccDataType dst_data_type, HccExpr** expr_mut) {
	HccExpr* expr = *expr_mut;
	if (hcc_typedef_resolve_and_strip_const(c, expr->data_type) == hcc_typedef_resolve_and_strip_const(c, dst_data_type)) {
		return;
	}

	if (expr->type == HCC_EXPR_TYPE_CONSTANT) {
		hcc_astgen_eval_cast(c, expr, dst_data_type);
		return;
	}

	HccExpr* cast_expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_CAST);
	cast_expr->unary.expr_rel_idx = cast_expr - expr;
	cast_expr->data_type = dst_data_type;
	*expr_mut = cast_expr;
}

HccExpr* hcc_astgen_generate_unary_op(HccCompiler* c, HccExpr* inner_expr, HccUnaryOp unary_op, HccToken operator_token) {
	HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_UNARY_OP_START + unary_op);
	HccDataType resolved_data_type = hcc_typedef_resolve_and_strip_const(c, inner_expr->data_type);

	if (!HCC_DATA_TYPE_IS_NON_VOID_BASIC(resolved_data_type)) {
		HccString data_type_name = hcc_data_type_string(c, inner_expr->data_type);
		hcc_astgen_error_1(c, HCC_ERROR_CODE_UNARY_OPERATOR_NOT_SUPPORTED, hcc_token_strings[operator_token], (int)data_type_name.size, data_type_name.data);
	}

	if (unary_op != HCC_UNARY_OP_LOGICAL_NOT) {
		if (HCC_DATA_TYPE_IS_INT(resolved_data_type)) {
			U8 rank = hcc_data_type_basic_type_ranks[resolved_data_type];
			U8 int_rank = hcc_data_type_basic_type_ranks[HCC_DATA_TYPE_SINT];
			if (rank < int_rank) {
				hcc_astgen_generate_implicit_cast(c, HCC_DATA_TYPE_SINT, &inner_expr);
			}
		}
	}

	expr->unary.expr_rel_idx = expr - inner_expr;
	expr->data_type = unary_op == HCC_UNARY_OP_LOGICAL_NOT ? HCC_DATA_TYPE_BOOL : inner_expr->data_type;

	return expr;
}

HccExpr* hcc_astgen_generate_unary_expr(HccCompiler* c) {
	HccToken token = hcc_astgen_token_peek(c);
	HccUnaryOp unary_op;
	switch (token) {
		case HCC_TOKEN_KEYWORD_TRUE:
		case HCC_TOKEN_KEYWORD_FALSE:
		case HCC_TOKEN_LIT_SINT:
		case HCC_TOKEN_LIT_SLONG:
		case HCC_TOKEN_LIT_SLONGLONG:
		case HCC_TOKEN_LIT_UINT:
		case HCC_TOKEN_LIT_ULONG:
		case HCC_TOKEN_LIT_ULONGLONG:
		case HCC_TOKEN_LIT_FLOAT:
		case HCC_TOKEN_LIT_DOUBLE: {
			HccDataType data_type;
			HccConstantId constant_id;
			switch (token) {
				case HCC_TOKEN_KEYWORD_TRUE:
					data_type = HCC_DATA_TYPE_BOOL;
					constant_id = c->basic_type_one_constant_ids[HCC_DATA_TYPE_BOOL];
					break;
				case HCC_TOKEN_KEYWORD_FALSE:
					data_type = HCC_DATA_TYPE_BOOL;
					constant_id = c->basic_type_zero_constant_ids[HCC_DATA_TYPE_BOOL];
					break;
				case HCC_TOKEN_LIT_SINT: data_type = HCC_DATA_TYPE_SINT; break;
				case HCC_TOKEN_LIT_SLONG: data_type = HCC_DATA_TYPE_SLONG; break;
				case HCC_TOKEN_LIT_SLONGLONG: data_type = HCC_DATA_TYPE_SLONGLONG; break;
				case HCC_TOKEN_LIT_UINT: data_type = HCC_DATA_TYPE_UINT; break;
				case HCC_TOKEN_LIT_ULONG: data_type = HCC_DATA_TYPE_ULONG; break;
				case HCC_TOKEN_LIT_ULONGLONG: data_type = HCC_DATA_TYPE_ULONGLONG; break;
				case HCC_TOKEN_LIT_FLOAT: data_type = HCC_DATA_TYPE_FLOAT; break;
				case HCC_TOKEN_LIT_DOUBLE: data_type = HCC_DATA_TYPE_DOUBLE; break;
			}
			if (data_type != HCC_DATA_TYPE_BOOL) {
				constant_id = hcc_astgen_token_value_next(c).constant_id;

				//
				// skip the associated HccStringId kept around to turn the
				// literal back into the exact string it was parsed from.
				hcc_astgen_token_value_next(c);
			}

			HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_CONSTANT);
			expr->constant.id = constant_id.idx_plus_one;
			expr->data_type = data_type;
			hcc_astgen_token_consume(c, 1);
			return expr;
		};
		case HCC_TOKEN_IDENT: {
			HccTokenValue identifier_value = hcc_astgen_token_value_next(c);
			hcc_astgen_token_consume(c, 1);

			U32 existing_variable_id = hcc_astgen_variable_stack_find(c, identifier_value.string_id);
			if (existing_variable_id) {
				HccVariable* variable = hcc_stack_get(c->astgen.function_params_and_variables, c->astgen.function->params_start_idx + existing_variable_id - 1);

				HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_LOCAL_VARIABLE);
				expr->variable.idx = existing_variable_id - 1;
				expr->data_type = variable->data_type;

				if (variable->is_static) {
					hcc_astgen_static_variable_usage_found(c, HCC_DECL_INIT(HCC_DECL_LOCAL_VARIABLE, existing_variable_id - 1));
				}
				return expr;
			}

			HccDecl decl;
			if (hcc_hash_table_find(&c->astgen.global_declarations, identifier_value.string_id.idx_plus_one, &decl)) {
				if (HCC_DECL_IS_DATA_TYPE(decl)) {
					HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_DATA_TYPE);
					expr->data_type = decl;
					return expr;
				} else if (HCC_DECL_IS_FUNCTION(decl)) {
					HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_FUNCTION);
					expr->function.idx = HCC_DECL_IDX(decl);
					return expr;
				} else if (HCC_DECL_IS_ENUM_VALUE(decl)) {
					HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_CONSTANT);
					HccEnumValue* enum_value = hcc_enum_value_get(c, decl);
					expr->constant.id = enum_value->value_constant_id.idx_plus_one;
					expr->data_type = HCC_DATA_TYPE_SINT;
					return expr;
				} else if (HCC_DECL_IS_GLOBAL_VARIABLE(decl)) {
					HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_GLOBAL_VARIABLE);
					HccVariable* variable = hcc_global_variable_get(c, decl);
					expr->variable.idx = HCC_DECL_IDX(decl);
					expr->data_type = variable->data_type;

					hcc_astgen_static_variable_usage_found(c, decl);
					return expr;
				} else {
					HCC_UNREACHABLE("unhandled decl type here %u", decl);
				}
			}

			HccString string = hcc_string_table_get(&c->string_table, identifier_value.string_id);
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_UNDECLARED_IDENTIFIER, (int)string.size, string.data);
		};
		case HCC_TOKEN_TILDE: unary_op = HCC_UNARY_OP_BIT_NOT; goto UNARY;
		case HCC_TOKEN_EXCLAMATION_MARK: unary_op = HCC_UNARY_OP_LOGICAL_NOT; goto UNARY;
		case HCC_TOKEN_PLUS: unary_op = HCC_UNARY_OP_PLUS; goto UNARY;
		case HCC_TOKEN_MINUS: unary_op = HCC_UNARY_OP_NEGATE; goto UNARY;
		case HCC_TOKEN_INCREMENT: unary_op = HCC_UNARY_OP_PRE_INCREMENT; goto UNARY;
		case HCC_TOKEN_DECREMENT: unary_op = HCC_UNARY_OP_PRE_DECREMENT; goto UNARY;
UNARY:
		{
			HccToken operator_token = token;
			hcc_astgen_token_consume(c, 1);

			HccExpr* inner_expr = hcc_astgen_generate_unary_expr(c);
			return hcc_astgen_generate_unary_op(c, inner_expr, unary_op, operator_token);
		};
		case HCC_TOKEN_PARENTHESIS_OPEN: {
			hcc_astgen_token_consume(c, 1);
			HccExpr* expr = hcc_astgen_generate_expr(c, 0);
			HccToken token = hcc_astgen_token_peek(c);
			if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_EXPR);
			}
			token = hcc_astgen_token_next(c);

			if (expr->type == HCC_EXPR_TYPE_DATA_TYPE) {
				if (token == HCC_TOKEN_CURLY_OPEN) {
					//
					// found compound literal
					c->astgen.assign_data_type = expr->data_type;
					return hcc_astgen_generate_unary_expr(c);
				} else {
					HccExpr* right_expr = hcc_astgen_generate_expr(c, 2);
					HccDataType resolved_cast_data_type = hcc_typedef_resolve_and_strip_const(c, expr->data_type);
					HccDataType resolved_castee_data_type = hcc_typedef_resolve_and_strip_const(c, right_expr->data_type);
					if (resolved_cast_data_type != resolved_castee_data_type) {
						if (!HCC_DATA_TYPE_IS_BASIC(resolved_cast_data_type) || !HCC_DATA_TYPE_IS_BASIC(resolved_castee_data_type)) {
							HccString target_data_type_name = hcc_data_type_string(c, expr->data_type);
							HccString source_data_type_name = hcc_data_type_string(c, right_expr->data_type);
							hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_CAST, (int)source_data_type_name.size, source_data_type_name.data, (int)target_data_type_name.size, target_data_type_name.data);
						}

						if (right_expr->type == HCC_EXPR_TYPE_CONSTANT) {
							hcc_astgen_eval_cast(c, right_expr, expr->data_type);
							return right_expr;
						}

						HccExpr* cast_expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_CAST);
						cast_expr->unary.expr_rel_idx = cast_expr - right_expr;
						cast_expr->data_type = expr->data_type;
						return cast_expr;
					}
					return right_expr;
				}
			}

			return expr;
		};
		case HCC_TOKEN_CURLY_OPEN: {
			HccDataType assign_data_type = c->astgen.assign_data_type;
			HccDataType resolved_assign_data_type = hcc_typedef_resolve_and_keep_const(c, assign_data_type);
			c->astgen.assign_data_type = HCC_DATA_TYPE_VOID;
			HccAstGenCurlyInitializer* gen = &c->astgen.curly_initializer;

			HccExpr* curly_initializer_expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_CURLY_INITIALIZER);
			curly_initializer_expr->data_type = assign_data_type;

			HccExpr* variable_expr;
			{
				HccVariable* variable = hcc_stack_push(c->astgen.function_params_and_variables);
				variable->identifier_string_id.idx_plus_one = 0;
				variable->identifier_token_idx = 0;
				variable->data_type = assign_data_type;
				c->astgen.stmt_block->stmt_block.variables_count += 1;

				U32 variable_idx = c->astgen.next_var_idx;
				c->astgen.next_var_idx += 1;

				variable_expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_LOCAL_VARIABLE);
				variable_expr->variable.idx = variable_idx;
				variable_expr->next_expr_rel_idx = 0;
			}

			U32 nested_curlys_start_idx = hcc_stack_count(gen->nested_curlys);
			token = hcc_astgen_curly_initializer_start(c, assign_data_type, resolved_assign_data_type, variable_expr);

			while (1) {
				if (token == HCC_TOKEN_CURLY_OPEN) {
					//
					// we have just found a another curly initializer, so nest down into the next element type
					//
					hcc_astgen_curly_initializer_next_elmt(c, HCC_DATA_TYPE_VOID);
					token = hcc_astgen_curly_initializer_open(c);
					continue;
				}

				if (token == HCC_TOKEN_FULL_STOP || token == HCC_TOKEN_SQUARE_OPEN) {
					token = hcc_astgen_curly_initializer_next_elmt_with_designator(c);
					if (token == HCC_TOKEN_CURLY_OPEN) {
						token = hcc_astgen_curly_initializer_open(c);
						continue;
					}
				} else if (hcc_stack_get_last(gen->nested_curlys)->found_designator) {
					hcc_astgen_warn_1(c, HCC_WARN_CODE_NO_DESIGNATOR_AFTER_DESIGNATOR);
				}

				HccExpr* value_expr = hcc_astgen_generate_expr_no_comma_operator(c, 0);
				HccDataType resolved_value_data_type = hcc_typedef_resolve_and_keep_const(c, value_expr->data_type);
				bool emit_elmt_initializer = hcc_astgen_curly_initializer_next_elmt(c, resolved_value_data_type);
				if (emit_elmt_initializer) {
					HccExpr* initializer_expr = hcc_astgen_curly_initializer_generate_designated_initializer(c);
					U32 other_token_idx = U32_MAX;
					hcc_data_type_ensure_compatible_assignment(c, other_token_idx, c->astgen.curly_initializer.elmt_data_type, &value_expr);
					initializer_expr->designated_initializer.value_expr_rel_idx = initializer_expr - value_expr;
				}

				token = hcc_astgen_token_peek(c);

				//
				// loop to close curly braces and/or go to the next element.
				// loop until we find the final curly close or when we find a ',' that is _not_ followed by a '}'
				while (1) {
					bool found_one = false;
					if (token == HCC_TOKEN_CURLY_CLOSE) {
						bool is_finished = hcc_stack_count(gen->nested_curlys) == nested_curlys_start_idx + 1;
						token = hcc_astgen_curly_initializer_close(c, is_finished);
						if (is_finished) {
							goto CURLY_INITIALIZER_FINISH;
						}
						found_one = true;
					}

					if (token == HCC_TOKEN_COMMA) {
						token = hcc_astgen_token_next(c);
						if (token != HCC_TOKEN_CURLY_CLOSE) {
							break;
						}
						found_one = true;
					}

					if (!found_one) {
						hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_CURLY_INITIALIZER_LIST_END);
					}
				}
			}
CURLY_INITIALIZER_FINISH: {}
			token = hcc_astgen_token_next(c);
			curly_initializer_expr->curly_initializer.first_expr_rel_idx = gen->first_initializer_expr - curly_initializer_expr;

			//
			// if we have nested into another curly initializer expression
			// then restore the parent curly initializers state in the HccAstGenCurlyInitializer structure.
			if (nested_curlys_start_idx) {
				HccAstGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
				hcc_astgen_curly_initializer_set_composite(c, nested_elmt->data_type, nested_elmt->resolved_data_type);

				HccAstGenCurlyInitializerNested* nested = hcc_stack_get_last(gen->nested);
				gen->prev_initializer_expr = nested->prev_initializer_expr;
				gen->first_initializer_expr = nested->first_initializer_expr;
				gen->nested_elmts_start_idx = nested->nested_elmts_start_idx;
				hcc_stack_pop(gen->nested);
			}

			return curly_initializer_expr;
		};
		case HCC_TOKEN_KEYWORD_SIZEOF:
		case HCC_TOKEN_KEYWORD_ALIGNOF:
		{
			bool is_sizeof = token == HCC_TOKEN_KEYWORD_SIZEOF;
			token = hcc_astgen_token_next(c);
			bool has_parenthesis = token == HCC_TOKEN_PARENTHESIS_OPEN;
			if (has_parenthesis) {
				token = hcc_astgen_token_next(c);
			}
			HccExpr* expr = hcc_astgen_generate_unary_expr(c);
			if (has_parenthesis) {
				token = hcc_astgen_token_peek(c);
				if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_EXPR);
				}
				token = hcc_astgen_token_next(c);
			} else if (expr->type == HCC_EXPR_TYPE_DATA_TYPE) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_SIZEALIGNOF_TYPE_OPERAND_NOT_WRAPPED, hcc_token_strings[token]);
			}

			U64 size;
			U64 align;
			hcc_data_type_size_align(c, expr->data_type, &size, &align);

			// TODO the actual type of sizeof must be a size_t aka uintptr_t
			HccBasic TODO_intptr_support_plz = hcc_basic_from_uint(c, HCC_DATA_TYPE_UINT, is_sizeof ? size : align);

			expr->type = HCC_EXPR_TYPE_CONSTANT;
			expr->constant.id = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_UINT, &TODO_intptr_support_plz).idx_plus_one;
			expr->data_type = HCC_DATA_TYPE_UINT;
			return expr;
		};

		case HCC_TOKEN_KEYWORD_VOID:
		case HCC_TOKEN_KEYWORD_BOOL:
		case HCC_TOKEN_KEYWORD_CHAR:
		case HCC_TOKEN_KEYWORD_SHORT:
		case HCC_TOKEN_KEYWORD_INT:
		case HCC_TOKEN_KEYWORD_LONG:
		case HCC_TOKEN_KEYWORD_FLOAT:
		case HCC_TOKEN_KEYWORD_DOUBLE:
		case HCC_TOKEN_KEYWORD_UNSIGNED:
		case HCC_TOKEN_KEYWORD_SIGNED:
		case HCC_TOKEN_KEYWORD_STRUCT:
		case HCC_TOKEN_KEYWORD_UNION:
		default: {
			HccDataType data_type;
			if (hcc_astgen_generate_data_type(c, &data_type)) {
				HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_DATA_TYPE);
				expr->data_type = data_type;
				return expr;
			}
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_EXPR, hcc_token_strings[token]);
		};
	}
}

void hcc_astgen_generate_binary_op(HccCompiler* c, HccExprType* binary_op_type_out, U32* precedence_out, bool* is_assignment_out) {
	HccToken token = hcc_astgen_token_peek(c);
	*is_assignment_out = false;
	switch (token) {
		case HCC_TOKEN_INCREMENT:
			*binary_op_type_out = HCC_EXPR_TYPE_UNARY_OP(POST_INCREMENT);
			*precedence_out = 1;
			break;
		case HCC_TOKEN_DECREMENT:
			*binary_op_type_out = HCC_EXPR_TYPE_UNARY_OP(POST_DECREMENT);
			*precedence_out = 1;
			break;
		case HCC_TOKEN_FULL_STOP:
			*binary_op_type_out = HCC_EXPR_TYPE_FIELD_ACCESS;
			*precedence_out = 1;
			break;
		case HCC_TOKEN_PARENTHESIS_OPEN:
			*binary_op_type_out = HCC_EXPR_TYPE_CALL;
			*precedence_out = 1;
			break;
		case HCC_TOKEN_SQUARE_OPEN:
			*binary_op_type_out = HCC_EXPR_TYPE_ARRAY_SUBSCRIPT;
			*precedence_out = 1;
			break;
		case HCC_TOKEN_ASTERISK:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(MULTIPLY);
			*precedence_out = 3;
			break;
		case HCC_TOKEN_FORWARD_SLASH:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(DIVIDE);
			*precedence_out = 3;
			break;
		case HCC_TOKEN_PERCENT:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(MODULO);
			*precedence_out = 3;
			break;
		case HCC_TOKEN_PLUS:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(ADD);
			*precedence_out = 4;
			break;
		case HCC_TOKEN_MINUS:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(SUBTRACT);
			*precedence_out = 4;
			break;
		case HCC_TOKEN_BIT_SHIFT_LEFT:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT);
			*precedence_out = 5;
			break;
		case HCC_TOKEN_BIT_SHIFT_RIGHT:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT);
			*precedence_out = 5;
			break;
		case HCC_TOKEN_LESS_THAN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(LESS_THAN);
			*precedence_out = 6;
			break;
		case HCC_TOKEN_LESS_THAN_OR_EQUAL:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(LESS_THAN_OR_EQUAL);
			*precedence_out = 6;
			break;
		case HCC_TOKEN_GREATER_THAN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(GREATER_THAN);
			*precedence_out = 6;
			break;
		case HCC_TOKEN_GREATER_THAN_OR_EQUAL:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(GREATER_THAN_OR_EQUAL);
			*precedence_out = 6;
			break;
		case HCC_TOKEN_LOGICAL_EQUAL:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(EQUAL);
			*precedence_out = 7;
			break;
		case HCC_TOKEN_LOGICAL_NOT_EQUAL:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(NOT_EQUAL);
			*precedence_out = 7;
			break;
		case HCC_TOKEN_AMPERSAND:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_AND);
			*precedence_out = 8;
			break;
		case HCC_TOKEN_CARET:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_XOR);
			*precedence_out = 9;
			break;
		case HCC_TOKEN_PIPE:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_OR);
			*precedence_out = 10;
			break;
		case HCC_TOKEN_LOGICAL_AND:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(LOGICAL_AND);
			*precedence_out = 11;
			break;
		case HCC_TOKEN_LOGICAL_OR:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(LOGICAL_OR);
			*precedence_out = 12;
			break;
		case HCC_TOKEN_QUESTION_MARK:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(TERNARY);
			*precedence_out = 13;
			break;
		case HCC_TOKEN_EQUAL:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(ASSIGN);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_ADD_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(ADD);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_SUBTRACT_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(SUBTRACT);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_MULTIPLY_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(MULTIPLY);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_DIVIDE_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(DIVIDE);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_MODULO_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(MODULO);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_BIT_SHIFT_LEFT_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_BIT_AND_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_AND);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_BIT_XOR_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_XOR);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_BIT_OR_ASSIGN:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(BIT_OR);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_TOKEN_COMMA:
			*binary_op_type_out = HCC_EXPR_TYPE_BINARY_OP(COMMA);
			*precedence_out = 15;
			*is_assignment_out = true;
			break;
		default:
			*binary_op_type_out = HCC_EXPR_TYPE_NONE;
			*precedence_out = 0;
			break;
	}
}

HccExpr* hcc_astgen_generate_call_expr(HccCompiler* c, HccExpr* function_expr) {
	U32 args_count = 0;
	HccExpr* call_args_expr = NULL;

	U32 function_idx = function_expr->function.idx;
	HccFunction* function = hcc_function_get(c, HCC_DECL_INIT(HCC_DECL_FUNCTION, function_idx));
	if (function->shader_stage != HCC_FUNCTION_SHADER_STAGE_NONE) {
		c->astgen.token_read_idx -= 2;
		hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_CANNOT_CALL_SHADER_FUNCTION, function->identifier_token_idx);
	}

	if (function_idx >= HCC_FUNCTION_IDX_USER_START) {
		*hcc_stack_push(c->astgen.function_used_function_indices) = function_idx;
	}

	HccToken token = hcc_astgen_token_peek(c);
	if (token == HCC_TOKEN_PARENTHESIS_CLOSE) {
		hcc_astgen_ensure_function_args_count(c, function, 0);
		token = hcc_astgen_token_next(c);
		call_args_expr = hcc_astgen_alloc_expr_many(c, 2);
		call_args_expr->type = HCC_EXPR_TYPE_CALL_ARG_LIST;
		call_args_expr->is_stmt_block_entry = true;
		((U8*)call_args_expr)[1] = 0;

		HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_CALL);
		expr->binary.left_expr_rel_idx = expr - function_expr;
		expr->binary.right_expr_rel_idx = expr - call_args_expr;
		expr->data_type = function->return_data_type;
		return expr;
	}

	//
	// scan ahead an count how many arguments we are going to have.
	args_count = 1;
	U32 ahead_by = 0;
	U32 parenthesis_open = 0; // to avoid counting the comma operator
	while (1) {
		HccToken token = hcc_astgen_token_peek_ahead(c, ahead_by);
		switch (token) {
			case HCC_TOKEN_EOF:
				goto END_ARG_COUNT;
			case HCC_TOKEN_COMMA:
				if (parenthesis_open == 0) {
					args_count += 1;
				}
				break;
			case HCC_TOKEN_PARENTHESIS_OPEN:
				parenthesis_open += 1;
				break;
			case HCC_TOKEN_PARENTHESIS_CLOSE:
				if (parenthesis_open == 0) {
					goto END_ARG_COUNT;
				}
				parenthesis_open -= 1;
				break;
		}
		ahead_by += 1;
	}
END_ARG_COUNT: {}

	//
	// preallocating enough room in and after the call_args_expr to store relative indices to the next args for each argument expression
	U32 required_header_expressions = ((args_count + 2) / 8) + 1;
	call_args_expr = hcc_astgen_alloc_expr_many(c, required_header_expressions);
	call_args_expr->type = HCC_EXPR_TYPE_CALL_ARG_LIST;
	call_args_expr->is_stmt_block_entry = true;
	((U8*)call_args_expr)[1] = args_count;
	U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];

	HccExpr* prev_arg_expr = call_args_expr;

	U32 arg_idx = 0;
	token = hcc_astgen_token_peek(c);
	HccVariable* params_array = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx);
	while (1) {
		HccExpr* arg_expr = hcc_astgen_generate_expr_no_comma_operator(c, 0);
		HccVariable* param = &params_array[arg_idx];
		HccDataType param_data_type = HCC_DATA_TYPE_STRIP_CONST(param->data_type);
		hcc_data_type_ensure_compatible_assignment(c, param->identifier_token_idx, param_data_type, &arg_expr);

		next_arg_expr_rel_indices[arg_idx] = arg_expr - prev_arg_expr;
		arg_idx += 1;

		token = hcc_astgen_token_peek(c);
		if (token != HCC_TOKEN_COMMA) {
			if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_FUNCTION_ARG_DELIMITER);
			}
			token = hcc_astgen_token_next(c);
			break;
		}
		token = hcc_astgen_token_next(c);
		prev_arg_expr = arg_expr;
	}

	HCC_DEBUG_ASSERT(arg_idx == args_count, "internal error: the scan ahead arguments count code is out of sync with the parser");

	hcc_astgen_ensure_function_args_count(c, function, arg_idx);

	HccDataType return_data_type = function->return_data_type;

	function_expr->function.idx = function_idx;

	HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_CALL);
	expr->binary.left_expr_rel_idx = expr - function_expr;
	expr->binary.right_expr_rel_idx = expr - call_args_expr;
	expr->data_type = return_data_type;
	return expr;
}

HccExpr* hcc_astgen_generate_array_subscript_expr(HccCompiler* c, HccExpr* array_expr) {
	HccExpr* index_expr = hcc_astgen_generate_expr(c, 0);
	HccToken token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_SQUARE_CLOSE) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ARRAY_SUBSCRIPT_EXPECTED_SQUARE_BRACE_CLOSE);
	}
	hcc_astgen_token_next(c);

	HccArrayDataType* d = hcc_array_data_type_get(c, array_expr->data_type);

	HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_ARRAY_SUBSCRIPT);
	expr->binary.left_expr_rel_idx = expr - array_expr;
	expr->binary.right_expr_rel_idx = expr - index_expr;
	expr->data_type = d->element_data_type;
	if (HCC_DATA_TYPE_IS_CONST(array_expr->data_type)) {
		expr->data_type = HCC_DATA_TYPE_CONST(expr->data_type);
	}
	return expr;
}

HccExpr* hcc_astgen_generate_field_access_expr(HccCompiler* c, HccExpr* left_expr) {
	HccToken token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_IDENT) {
		HccString left_data_type_name = hcc_data_type_string(c, left_expr->data_type);
		U32 token_idx = hcc_data_type_token_idx(c, left_expr->data_type);
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_ACCESS, token_idx, (int)left_data_type_name.size, left_data_type_name.data);
	}

	HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, left_expr->data_type);

	HccStringId identifier_string_id = hcc_astgen_token_value_next(c).string_id;
	hcc_astgen_compound_data_type_find_field_by_name_checked(c, left_expr->data_type, compound_data_type, identifier_string_id);

	hcc_astgen_token_next(c);

	HccDataType const_mask = 0;
	if (HCC_DATA_TYPE_IS_CONST(left_expr->data_type)) {
		const_mask = HCC_DATA_TYPE_CONST_MASK;
	}

	U32 fields_count = hcc_stack_count(c->astgen.compound_type_find_fields);
	for (U32 i = 0; i < fields_count; i += 1) {
		HccFieldAccess* access = hcc_stack_get(c->astgen.compound_type_find_fields, i);
		HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_FIELD_ACCESS);
		expr->binary.left_expr_rel_idx = i == 0 ? (expr - left_expr) : 1; // link to the previous expression
		expr->binary.right_expr_rel_idx = access->idx;
		expr->data_type = access->data_type | const_mask;
	}

	HccExpr* field_access_expr = hcc_stack_get_last(c->astgen.exprs);
	return field_access_expr;
}

HccExpr* hcc_astgen_generate_ternary_expr(HccCompiler* c, HccExpr* cond_expr) {
	hcc_astgen_data_type_ensure_is_condition(c, cond_expr->data_type);

	HccExpr* true_expr = hcc_astgen_generate_expr(c, 0);

	HccToken token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_COLON) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_MISSING_COLON_TERNARY_OP);
	}
	token = hcc_astgen_token_next(c);

	HccExpr* false_expr = hcc_astgen_generate_expr(c, 0);

	U32 other_token_idx = -1;
	if (!hcc_data_type_check_compatible_arithmetic(c, &true_expr, &false_expr)) {
		HccString true_data_type_name = hcc_data_type_string(c, true_expr->data_type);
		HccString false_data_type_name = hcc_data_type_string(c, false_expr->data_type);
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_TYPE_MISMATCH, other_token_idx, (int)false_data_type_name.size, false_data_type_name.data, (int)true_data_type_name.size, true_data_type_name.data);
	}

	HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_BINARY_OP(TERNARY));
	expr->ternary.cond_expr_rel_idx = expr - cond_expr;
	expr->ternary.true_expr_rel_idx = expr - true_expr;
	expr->ternary.false_expr_rel_idx = expr - false_expr;
	expr->data_type = HCC_DATA_TYPE_STRIP_CONST(true_expr->data_type);
	return expr;
}

HccExpr* hcc_astgen_generate_expr_(HccCompiler* c, U32 min_precedence, bool no_comma_operator) {
	U32 callee_token_idx = c->astgen.token_read_idx;
	HccExpr* left_expr = hcc_astgen_generate_unary_expr(c);
	if (left_expr->type == HCC_EXPR_TYPE_DATA_TYPE) {
		hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR, left_expr->data_type);
		return left_expr;
	}

	while (1) {
		HccExprType binary_op_type;
		U32 precedence;
		bool is_assignment;
		HccToken operator_token = hcc_astgen_token_peek(c);
		if (no_comma_operator && operator_token == HCC_TOKEN_COMMA) {
			return left_expr;
		}

		hcc_astgen_generate_binary_op(c, &binary_op_type, &precedence, &is_assignment);
		if (binary_op_type == HCC_EXPR_TYPE_NONE || (min_precedence && min_precedence <= precedence)) {
			return left_expr;
		}
		hcc_astgen_token_next(c);
		HccDataType resolved_left_expr_data_type = hcc_typedef_resolve_and_strip_const(c, left_expr->data_type);
		hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR, left_expr->data_type);

		if (binary_op_type == HCC_EXPR_TYPE_CALL) {
			if (left_expr->type != HCC_EXPR_TYPE_FUNCTION) { // TODO add function pointer support
				hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_PARENTHISES_USED_ON_NON_FUNCTION, callee_token_idx);
			}

			left_expr = hcc_astgen_generate_call_expr(c, left_expr);
			hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR, left_expr->data_type);
		} else if (binary_op_type == HCC_EXPR_TYPE_ARRAY_SUBSCRIPT) {
			if (!HCC_DATA_TYPE_IS_ARRAY(resolved_left_expr_data_type)) { // TODO add pointer support
				HccString left_data_type_name = hcc_data_type_string(c, left_expr->data_type);
				hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_SQUARE_BRACE_USED_ON_NON_ARRAY_DATA_TYPE, callee_token_idx, (int)left_data_type_name.size, left_data_type_name.data);
			}

			left_expr = hcc_astgen_generate_array_subscript_expr(c, left_expr);
			hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR, left_expr->data_type);
		} else if (binary_op_type == HCC_EXPR_TYPE_FIELD_ACCESS) {
			if (!HCC_DATA_TYPE_IS_COMPOUND_TYPE(resolved_left_expr_data_type)) {
				HccString left_data_type_name = hcc_data_type_string(c, left_expr->data_type);
				hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_FULL_STOP_USED_ON_NON_COMPOUND_DATA_TYPE, callee_token_idx, (int)left_data_type_name.size, left_data_type_name.data);
			}

			left_expr = hcc_astgen_generate_field_access_expr(c, left_expr);
			hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR, left_expr->data_type);
		} else if (binary_op_type == HCC_EXPR_TYPE_BINARY_OP(TERNARY)) {
			left_expr = hcc_astgen_generate_ternary_expr(c, left_expr);
			hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR, left_expr->data_type);
		} else if (binary_op_type == HCC_EXPR_TYPE_UNARY_OP(POST_INCREMENT) || binary_op_type == HCC_EXPR_TYPE_UNARY_OP(POST_DECREMENT)) {
			HccUnaryOp unary_op = binary_op_type - HCC_EXPR_TYPE_UNARY_OP_START;
			left_expr = hcc_astgen_generate_unary_op(c, left_expr, unary_op, operator_token);
		} else {
			HccExpr* right_expr = hcc_astgen_generate_expr(c, precedence);
			hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_EXPR, right_expr->data_type);

			U32 other_token_idx = -1;
			if (is_assignment) {
				hcc_data_type_ensure_compatible_assignment(c, other_token_idx, resolved_left_expr_data_type, &right_expr);
			} else {
				hcc_data_type_ensure_compatible_arithmetic(c, other_token_idx, &left_expr, &right_expr, operator_token);
			}

			HccDataType data_type;
			if (HCC_EXPR_TYPE_BINARY_OP(EQUAL) <= binary_op_type && binary_op_type <= HCC_EXPR_TYPE_BINARY_OP(LOGICAL_OR)) {
				data_type = HCC_DATA_TYPE_BOOL;
			} else {
				data_type = left_expr->data_type; // TODO make implicit conversions explicit in the AST and make the error above work correctly
			}
			data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);

			if (
				hcc_options_is_enabled(c, HCC_OPTION_CONSTANT_FOLDING) &&
				left_expr->type == HCC_EXPR_TYPE_CONSTANT &&
				right_expr->type == HCC_EXPR_TYPE_CONSTANT
			) {
				//HccConstantId left_constant_id = { .idx_plus_one = left_expr->constant.id };
				//HccConstantId right_constant_id = { .idx_plus_one = right_expr->constant.id };
				HCC_ABORT("TODO CONSTANT FOLDING");
				//
				// combine left_expr and right_expr and store them in the left_expr
				//
			} else {
				if (is_assignment && HCC_DATA_TYPE_IS_CONST(data_type)) {
					HccString left_data_type_name = hcc_data_type_string(c, data_type);
					hcc_astgen_error_1(c, HCC_ERROR_CODE_CANNOT_ASSIGN_TO_CONST, (int)left_data_type_name.size, left_data_type_name.data);
				}

				HccExpr* expr = hcc_astgen_alloc_expr(c, binary_op_type);
				expr->binary.left_expr_rel_idx = expr - left_expr;
				expr->binary.right_expr_rel_idx = right_expr ? expr - right_expr : 0;
				expr->binary.is_assignment = is_assignment;
				expr->data_type = data_type;
				left_expr = expr;
			}
		}
	}
}

HccExpr* hcc_astgen_generate_expr(HccCompiler* c, U32 min_precedence) {
	return hcc_astgen_generate_expr_(c, min_precedence, false);
}

HccExpr* hcc_astgen_generate_expr_no_comma_operator(HccCompiler* c, U32 min_precedence) {
	return hcc_astgen_generate_expr_(c, min_precedence, true);
}

HccExpr* hcc_astgen_generate_cond_expr(HccCompiler* c) {
	HccToken token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_PARENTHESIS_OPEN) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_CONDITION_EXPR);
	}
	token = hcc_astgen_token_next(c);

	HccExpr* cond_expr = hcc_astgen_generate_expr(c, 0);
	hcc_astgen_data_type_ensure_is_condition(c, cond_expr->data_type);

	token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_CONDITION_EXPR);
	}
	token = hcc_astgen_token_next(c);
	return cond_expr;
}

HccDataType hcc_astgen_generate_variable_decl_array(HccCompiler* c, HccDataType element_data_type) {
	HccToken token = hcc_astgen_token_next(c);
	if (token == HCC_TOKEN_SQUARE_CLOSE) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_ARRAY_SIZE);
	}

	HccExpr* size_expr = hcc_astgen_generate_expr(c, 0);
	if (size_expr->type != HCC_EXPR_TYPE_CONSTANT || !HCC_DATA_TYPE_IS_INT(size_expr->data_type)) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_INTEGER_CONSTANT_ARRAY_SIZE);
	}

	HccConstantId size_constant_id = { .idx_plus_one = size_expr->constant.id };
	HccConstant constant = hcc_constant_table_get(c, size_constant_id);
	U64 size;
	if (!hcc_constant_as_uint(c, constant, &size)) {
		hcc_astgen_error_1(c, HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_NEGATIVE);
	}
	if (size == 0) {
		hcc_astgen_error_1(c, HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_ZERO);
	}

	token = hcc_astgen_token_peek(c);
	if (token != HCC_TOKEN_SQUARE_CLOSE) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_ARRAY_DECL_EXPECTED_SQUARE_BRACE_CLOSE);
	}
	token = hcc_astgen_token_next(c);

	HccDataType data_type = hcc_astgen_deduplicate_array_data_type(c, element_data_type, size_constant_id);
	if (token == HCC_TOKEN_SQUARE_OPEN) {
		data_type = hcc_astgen_generate_variable_decl_array(c, data_type);
	}
	return data_type;
}

U32 hcc_astgen_generate_variable_decl(HccCompiler* c, bool is_global, HccDataType* data_type_mut, HccExpr** init_expr_out) {
	HccToken token = hcc_astgen_token_peek(c);
	HCC_DEBUG_ASSERT(token == HCC_TOKEN_IDENT, "internal error: expected '%s' at the start of generating a variable", hcc_token_strings[HCC_TOKEN_IDENT]);
	HccStringId identifier_string_id = hcc_astgen_token_value_next(c).string_id;

	hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_VARIABLE, *data_type_mut);

	token = hcc_astgen_token_next(c);
	if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_VARIABLE_SPECIFIERS) {
		HccSpecifier specifier = HCC_LEAST_SET_BIT_IDX_U32(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_VARIABLE_SPECIFIERS);
		HccToken token = hcc_specifier_tokens[specifier];
		hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_SPECIFIER_VARIABLE_DECL, hcc_token_strings[token]);
	}

	U32 existing_variable_id = hcc_astgen_variable_stack_find(c, identifier_string_id);
	if (existing_variable_id) { // TODO: support shadowing but also warn or error about it
		U32 other_token_idx = -1;// TODO: location of existing variable
		HccString string = hcc_string_table_get(&c->string_table, identifier_string_id);
		hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_LOCAL, other_token_idx, (int)string.size, string.data);
	}

	U32 variable_idx;
	HccVariable* variable;
	if (is_global) {
		variable_idx = hcc_stack_count(c->astgen.global_variables);
		variable = hcc_stack_push(c->astgen.global_variables);

		HccDecl decl = HCC_DECL_INIT(HCC_DECL_GLOBAL_VARIABLE, variable_idx);
		hcc_astgen_insert_global_declaration(c, identifier_string_id, decl);
	} else {
		variable_idx = hcc_astgen_variable_stack_add(c, identifier_string_id);
		variable = hcc_stack_push(c->astgen.function_params_and_variables);
		c->astgen.stmt_block->stmt_block.variables_count += 1;
	}
	variable->identifier_string_id = identifier_string_id;
	variable->identifier_token_idx = c->astgen.token_read_idx;
	variable->data_type = *data_type_mut;
	variable->is_static = !!(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_STATIC) || is_global;
	variable->is_const = !!(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_CONST);
	variable->initializer_constant_id.idx_plus_one = 0;

	if (token == HCC_TOKEN_SQUARE_OPEN) {
		variable->data_type = hcc_astgen_generate_variable_decl_array(c, variable->data_type);
		*data_type_mut = variable->data_type;
		token = hcc_astgen_token_peek(c);
	}

	if (variable->is_const) {
		variable->data_type = HCC_DATA_TYPE_CONST(variable->data_type);
		*data_type_mut = variable->data_type;
	}

	switch (token) {
		case HCC_TOKEN_SEMICOLON:
			if (init_expr_out) *init_expr_out = NULL;
			break;
		case HCC_TOKEN_EQUAL: {
			hcc_astgen_token_next(c);

			c->astgen.assign_data_type = variable->data_type;
			HccExpr* init_expr = hcc_astgen_generate_expr_no_comma_operator(c, 0);
			U32 other_token_idx = -1;
			HccDataType variable_data_type = HCC_DATA_TYPE_STRIP_CONST(variable->data_type);
			hcc_data_type_ensure_compatible_assignment(c, other_token_idx, variable_data_type, &init_expr);
			c->astgen.assign_data_type = HCC_DATA_TYPE_VOID;

			if (variable->is_static) {
				if (init_expr->type != HCC_EXPR_TYPE_CONSTANT) {
					hcc_astgen_error_1(c, HCC_ERROR_CODE_STATIC_VARIABLE_INITIALIZER_MUST_BE_CONSTANT);
				}
				variable->initializer_constant_id.idx_plus_one = init_expr->constant.id;
				if (init_expr_out) *init_expr_out = NULL;
			} else {
				if (init_expr_out) *init_expr_out = init_expr;
			}
			break;
		};
		case HCC_TOKEN_COMMA: break;
		default:
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_VARIABLE_DECL_TERMINATOR);
	}

	if (variable->is_static && variable->initializer_constant_id.idx_plus_one == 0) {
		variable->initializer_constant_id = hcc_constant_table_deduplicate_zero(c, variable->data_type);
	}

	c->astgen.specifier_flags &= ~HCC_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS;
	return variable_idx;
}

HccExpr* hcc_astgen_generate_variable_decl_expr(HccCompiler* c, HccDataType data_type) {
	HccExpr* prev_expr = NULL;
	HccExpr* init_expr = NULL;
	HccToken token = hcc_astgen_token_peek(c);

	while (1) {
		U32 variable_idx = hcc_astgen_generate_variable_decl(c, false, &data_type, &init_expr);
		if (init_expr) {
			HccExpr* left_expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_LOCAL_VARIABLE);
			left_expr->variable.idx = variable_idx;
			left_expr->data_type = data_type;

			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_BINARY_OP(ASSIGN));
			stmt->binary.is_assignment = true;
			stmt->binary.left_expr_rel_idx = stmt - left_expr;
			stmt->binary.right_expr_rel_idx = stmt - init_expr;

			if (prev_expr) {
				HccExpr* expr = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_BINARY_OP(COMMA));
				expr->binary.is_assignment = false;
				expr->binary.left_expr_rel_idx = expr - prev_expr;
				expr->binary.right_expr_rel_idx = expr - stmt;
				prev_expr = expr;
			} else {
				prev_expr = stmt;
			}
		}

		token = hcc_astgen_token_peek(c);
		if (token != HCC_TOKEN_COMMA) {
			break;
		}
		token = hcc_astgen_token_next(c);
	}

	return prev_expr;
}

HccExpr* hcc_astgen_generate_stmt(HccCompiler* c) {
	HccToken token = hcc_astgen_token_peek(c);
	switch (token) {
		case HCC_TOKEN_CURLY_OPEN: {
			hcc_astgen_variable_stack_open(c);

			HccExpr* prev_stmt = NULL;

			HccExpr* stmt_block = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_BLOCK);
			U32 stmts_count = 0;

			stmt_block->is_stmt_block_entry = true;
			stmt_block->stmt_block.variables_count = 0;
			HccExpr* prev_stmt_block = c->astgen.stmt_block;
			c->astgen.stmt_block = stmt_block;

			token = hcc_astgen_token_next(c);
			while (token != HCC_TOKEN_CURLY_CLOSE) {
				HccExpr* stmt = hcc_astgen_generate_stmt(c);
				if (stmt == NULL) {
					continue;
				}
				stmt->is_stmt_block_entry = true;
				stmt_block->stmt_block.has_return_stmt |= stmt->type == HCC_EXPR_TYPE_STMT_RETURN;

				if (prev_stmt) {
					prev_stmt->next_expr_rel_idx = stmt - prev_stmt;
				} else {
					stmt_block->stmt_block.first_expr_rel_idx = stmt - stmt_block;
				}

				stmts_count += 1;
				token = hcc_astgen_token_peek(c);
				prev_stmt = stmt;
			}

			stmt_block->stmt_block.stmts_count = stmts_count;
			hcc_astgen_variable_stack_close(c);
			token = hcc_astgen_token_next(c);
			c->astgen.stmt_block = prev_stmt_block;
			return stmt_block;
		};
		case HCC_TOKEN_KEYWORD_RETURN: {
			hcc_astgen_token_next(c);
			HccExpr* expr = hcc_astgen_generate_expr(c, 0);

			hcc_data_type_ensure_compatible_assignment(c, c->astgen.function->return_data_type_token_idx, c->astgen.function->return_data_type, &expr);

			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_RETURN);
			stmt->unary.expr_rel_idx = stmt - expr;
			hcc_astgen_ensure_semicolon(c);

			return stmt;
		};
		case HCC_TOKEN_KEYWORD_IF: {
			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_IF);
			hcc_astgen_token_next(c);
			HccExpr* cond_expr = hcc_astgen_generate_cond_expr(c);

			HccExpr* true_stmt = hcc_astgen_generate_stmt(c);
			true_stmt->is_stmt_block_entry = true;

			token = hcc_astgen_token_peek(c);
			HccExpr* false_stmt = NULL;
			if (token == HCC_TOKEN_KEYWORD_ELSE) {
				token = hcc_astgen_token_next(c);
				if (token != HCC_TOKEN_KEYWORD_IF && token != HCC_TOKEN_CURLY_OPEN) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_ELSE);
				}
				false_stmt = hcc_astgen_generate_stmt(c);
				false_stmt->is_stmt_block_entry = true;
			}

			stmt->type = HCC_EXPR_TYPE_STMT_IF;
			stmt->if_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->if_.true_stmt_rel_idx = true_stmt - stmt;

			true_stmt->if_aux.false_stmt_rel_idx = false_stmt ? false_stmt - true_stmt : 0;
			true_stmt->if_aux.true_and_false_stmts_have_return_stmt = false;
			if (false_stmt) {
				true_stmt->if_aux.true_and_false_stmts_have_return_stmt = hcc_stmt_has_return(true_stmt) && hcc_stmt_has_return(false_stmt);
			}
			return stmt;
		};
		case HCC_TOKEN_KEYWORD_SWITCH: {
			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_SWITCH);
			token = hcc_astgen_token_next(c);

			HccExpr* cond_expr;
			{
				if (token != HCC_TOKEN_PARENTHESIS_OPEN) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_CONDITION_EXPR);
				}
				token = hcc_astgen_token_next(c);

				cond_expr = hcc_astgen_generate_expr(c, 0);
				if (!HCC_DATA_TYPE_IS_INT(cond_expr->data_type)) {
					HccString data_type_name = hcc_data_type_string(c, cond_expr->data_type);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INVALID_SWITCH_CONDITION_TYPE, (int)data_type_name.size, data_type_name.data);
				}

				token = hcc_astgen_token_peek(c);
				if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_CONDITION_EXPR);
				}
				token = hcc_astgen_token_next(c);
			}
			stmt->switch_.cond_expr_rel_idx = cond_expr - stmt;

			if (token != HCC_TOKEN_CURLY_OPEN) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_SWITCH_STATEMENT);
			}

			HccSwitchState* switch_state = &c->astgen.switch_state;
			HccSwitchState prev_switch_state = *switch_state;

			switch_state->switch_stmt = stmt;
			switch_state->first_switch_case = NULL;
			switch_state->prev_switch_case = NULL;
			switch_state->default_switch_case = NULL;
			switch_state->switch_condition_type = cond_expr->data_type;
			switch_state->case_stmts_count = 0;

			HccExpr* block_stmt = hcc_astgen_generate_stmt(c);
			block_stmt->is_stmt_block_entry = true;
			block_stmt->switch_aux.case_stmts_count = switch_state->case_stmts_count;
			block_stmt->switch_aux.first_case_expr_rel_idx = switch_state->first_switch_case ? switch_state->first_switch_case - block_stmt : 0;

			stmt->switch_.block_expr_rel_idx = block_stmt - stmt;
			stmt->alt_next_expr_rel_idx = switch_state->default_switch_case ? switch_state->default_switch_case - stmt : 0;

			*switch_state = prev_switch_state;
			return stmt;
		};
		case HCC_TOKEN_KEYWORD_DO: {
			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_WHILE);
			hcc_astgen_token_next(c);

			bool prev_is_in_loop = c->astgen.is_in_loop;
			c->astgen.is_in_loop = true;
			HccExpr* loop_stmt = hcc_astgen_generate_stmt(c);
			loop_stmt->is_stmt_block_entry = true;
			c->astgen.is_in_loop = prev_is_in_loop;

			token = hcc_astgen_token_peek(c);
			if (token != HCC_TOKEN_KEYWORD_WHILE) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_WHILE_CONDITION_FOR_DO_WHILE);
			}
			token = hcc_astgen_token_next(c);

			HccExpr* cond_expr = hcc_astgen_generate_cond_expr(c);

			stmt->while_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->while_.loop_stmt_rel_idx = loop_stmt - stmt;

			hcc_astgen_ensure_semicolon(c);
			return stmt;
		};
		case HCC_TOKEN_KEYWORD_WHILE: {
			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_WHILE);
			hcc_astgen_token_next(c);

			HccExpr* cond_expr = hcc_astgen_generate_cond_expr(c);

			bool prev_is_in_loop = c->astgen.is_in_loop;
			c->astgen.is_in_loop = true;
			HccExpr* loop_stmt = hcc_astgen_generate_stmt(c);
			loop_stmt->is_stmt_block_entry = true;
			c->astgen.is_in_loop = prev_is_in_loop;

			stmt->while_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->while_.loop_stmt_rel_idx = loop_stmt - stmt;

			return stmt;
		};
		case HCC_TOKEN_KEYWORD_FOR: {
			hcc_astgen_variable_stack_open(c);

			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_FOR);
			token = hcc_astgen_token_next(c);

			if (token != HCC_TOKEN_PARENTHESIS_OPEN) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_FOR);
			}
			token = hcc_astgen_token_next(c);

			HccExpr* init_expr = hcc_astgen_generate_expr(c, 0);
			if (init_expr->type == HCC_EXPR_TYPE_DATA_TYPE) {
				token = hcc_astgen_token_peek(c);
				if (token != HCC_TOKEN_IDENT) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FOR_VARIABLE_DECL);
				}
				init_expr = hcc_astgen_generate_variable_decl_expr(c, init_expr->data_type);
			}
			hcc_astgen_ensure_semicolon(c);

			HccExpr* cond_expr = hcc_astgen_generate_expr(c, 0);
			hcc_astgen_data_type_ensure_is_condition(c, cond_expr->data_type);
			hcc_astgen_ensure_semicolon(c);

			HccExpr* inc_expr = hcc_astgen_generate_expr(c, 0);

			token = hcc_astgen_token_peek(c);
			if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_FOR);
			}
			token = hcc_astgen_token_next(c);

			bool prev_is_in_loop = c->astgen.is_in_loop;
			c->astgen.is_in_loop = true;
			HccExpr* loop_stmt = hcc_astgen_generate_stmt(c);
			loop_stmt->is_stmt_block_entry = true;
			c->astgen.is_in_loop = prev_is_in_loop;

			stmt->for_.init_expr_rel_idx = init_expr - stmt;
			stmt->for_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->for_.inc_expr_rel_idx = inc_expr - stmt;
			stmt->for_.loop_stmt_rel_idx = loop_stmt - stmt;

			hcc_astgen_variable_stack_close(c);
			return stmt;
		};
		case HCC_TOKEN_KEYWORD_CASE: {
			HccSwitchState* switch_state = &c->astgen.switch_state;
			if (switch_state->switch_condition_type == HCC_DATA_TYPE_VOID) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_CASE_STATEMENT_OUTSIDE_OF_SWITCH);
			}

			token = hcc_astgen_token_next(c);

			HccExpr* expr = hcc_astgen_generate_expr(c, 0);
			if (expr->type != HCC_EXPR_TYPE_CONSTANT) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_SWITCH_CASE_VALUE_MUST_BE_A_CONSTANT);
			}
			U32 other_token_idx = -1;// TODO: the switch condition expr
			hcc_data_type_ensure_compatible_assignment(c, other_token_idx, switch_state->switch_condition_type, &expr);

			expr->type = HCC_EXPR_TYPE_STMT_CASE;
			expr->is_stmt_block_entry = true;
			expr->next_expr_rel_idx = 0;
			expr->alt_next_expr_rel_idx = 0;

			token = hcc_astgen_token_peek(c);
			if (token != HCC_TOKEN_COLON) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_CASE);
			}
			hcc_astgen_token_next(c);

			//
			// TODO: add this constant to a linear array with a location in a parallel array and check to see
			// if this constant has already been used in the switch case

			if (switch_state->prev_switch_case) {
				switch_state->prev_switch_case->alt_next_expr_rel_idx = expr - switch_state->prev_switch_case;
			} else {
				switch_state->first_switch_case = expr;
			}

			switch_state->case_stmts_count += 1;
			switch_state->prev_switch_case = expr;
			return expr;
		};
		case HCC_TOKEN_KEYWORD_DEFAULT: {
			HccSwitchState* switch_state = &c->astgen.switch_state;
			if (switch_state->switch_condition_type == HCC_DATA_TYPE_VOID) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_DEFAULT_STATMENT_OUTSIDE_OF_SWITCH);
			}
			if (switch_state->default_switch_case) {
				hcc_astgen_error_1(c, HCC_ERROR_CODE_DEFAULT_STATEMENT_ALREADY_DECLARED);
			}

			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_DEFAULT);
			stmt->is_stmt_block_entry = true;

			token = hcc_astgen_token_next(c);
			if (token != HCC_TOKEN_COLON) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_DEFAULT);
			}
			hcc_astgen_token_next(c);

			switch_state->default_switch_case = stmt;
			return stmt;
		};
		case HCC_TOKEN_KEYWORD_BREAK: {
			if (c->astgen.switch_state.switch_condition_type == HCC_DATA_TYPE_VOID && !c->astgen.is_in_loop) {
				hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_BREAK_STATEMENT_USAGE);
			}
			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_BREAK);
			stmt->is_stmt_block_entry = true;
			hcc_astgen_token_next(c);
			hcc_astgen_ensure_semicolon(c);
			return stmt;
		};
		case HCC_TOKEN_KEYWORD_CONTINUE: {
			if (c->astgen.switch_state.switch_condition_type == HCC_DATA_TYPE_VOID && !c->astgen.is_in_loop) {
				hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_CONTINUE_STATEMENT_USAGE);
			}
			HccExpr* stmt = hcc_astgen_alloc_expr(c, HCC_EXPR_TYPE_STMT_CONTINUE);
			stmt->is_stmt_block_entry = true;
			hcc_astgen_token_next(c);
			hcc_astgen_ensure_semicolon(c);
			return stmt;
		};
		case HCC_TOKEN_KEYWORD_TYPEDEF:
			hcc_astgen_generate_typedef(c);
			return NULL;
		case HCC_TOKEN_SEMICOLON:
			hcc_astgen_token_next(c);
			return NULL;
		default: {
			hcc_astgen_generate_specifiers(c);
			HccExpr* expr = hcc_astgen_generate_expr(c, 0);
			if (expr->type == HCC_EXPR_TYPE_DATA_TYPE) {
				token = hcc_astgen_token_peek(c);
				token = hcc_astgen_generate_specifiers(c);

				if (token == HCC_TOKEN_IDENT) {
					expr = hcc_astgen_generate_variable_decl_expr(c, expr->data_type);
				} else if (token == HCC_TOKEN_KEYWORD_TYPEDEF) {
					hcc_astgen_generate_typedef_with_data_type(c, expr->data_type);
				} else {
					hcc_astgen_ensure_no_unused_specifiers_identifier(c);
					expr = NULL;
				}
			} else {
				hcc_astgen_ensure_no_unused_specifiers_data_type(c);
			}

			hcc_astgen_ensure_semicolon(c);
			return expr;
		};
	}
}

void hcc_astgen_generate_function(HccCompiler* c, HccDataType return_data_type, U32 data_type_token_idx) {
	HccToken token = hcc_astgen_token_peek(c);
	HCC_DEBUG_ASSERT(token == HCC_TOKEN_IDENT, "internal error: expected '%s' at the start of generating a function", hcc_token_strings[HCC_TOKEN_IDENT]);
	HccStringId identifier_string_id = hcc_astgen_token_value_next(c).string_id;
	U32 identifier_token_idx = c->astgen.token_read_idx;
	U32 params_token_idx = c->astgen.token_read_idx;

	bool is_intrinsic = false;
	HccFunctionShaderStage shader_stage = HCC_FUNCTION_SHADER_STAGE_NONE;
	HccFunctionFlags flags = 0;
	{
		if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_FUNCTION_SPECIFIERS) {
			HccSpecifier specifier = HCC_LEAST_SET_BIT_IDX_U32(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_FUNCTION_SPECIFIERS);
			HccToken token = hcc_specifier_tokens[specifier];
			hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_SPECIFIER_FUNCTION_DECL, hcc_token_strings[token]);
		}

		if (!HCC_IS_POWER_OF_TWO_OR_ZERO(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_SHADER_STAGES)) {
			hcc_astgen_error_1(c, HCC_ERROR_CODE_MULTIPLE_SHADER_STAGES_ON_FUNCTION);
		}

		if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_VERTEX) {
			shader_stage = HCC_FUNCTION_SHADER_STAGE_VERTEX;
		} else if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_FRAGMENT) {
			shader_stage = HCC_FUNCTION_SHADER_STAGE_FRAGMENT;
		}

		if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_STATIC) {
			flags |= HCC_FUNCTION_FLAGS_STATIC;
		}

		if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_CONST) {
			flags |= HCC_FUNCTION_FLAGS_CONST;
		}

		if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_INLINE) {
			flags |= HCC_FUNCTION_FLAGS_INLINE;
		}

		is_intrinsic = (bool)(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_INTRINSIC);
		c->astgen.specifier_flags &= ~HCC_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS;
	}

	U32 function_idx;
	HccFunction* function;
	bool is_new = true;
	if (is_intrinsic) {
		if (identifier_string_id.idx_plus_one < HCC_STRING_ID_FUNCTION_IDXS_START || identifier_string_id.idx_plus_one >= HCC_STRING_ID_FUNCTION_IDXS_END) {
			HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_FUNCTION, (int)identifier_string.size, identifier_string.data);
		}

		function_idx = identifier_string_id.idx_plus_one - HCC_STRING_ID_FUNCTION_IDXS_START;
	} else {
		HccDecl decl;
		if (hcc_hash_table_find(&c->astgen.global_declarations, identifier_string_id.idx_plus_one, &decl)) {
			if (!HCC_DECL_IS_FUNCTION(decl)) {
				U32 other_token_idx = hcc_decl_token_idx(c, decl);
				HccString string = hcc_string_table_get(&c->string_table, identifier_string_id);
				hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_GLOBAL, other_token_idx, (int)string.size, string.data);
			}

			function_idx = HCC_DECL_IDX(decl);
			is_new = false;
		} else {
			function_idx = hcc_stack_count(c->astgen.functions);
			hcc_stack_push(c->astgen.functions);
		}
	}
	function = hcc_stack_get(c->astgen.functions, function_idx);
	c->astgen.function = function;
	function->identifier_token_idx = identifier_token_idx;
	function->shader_stage = shader_stage;
	function->flags = flags;

	token = hcc_astgen_token_next(c);

	switch (function->shader_stage) {
		case HCC_FUNCTION_SHADER_STAGE_VERTEX:
			if (!hcc_data_type_is_rasterizer_state(c, return_data_type)) {
				c->astgen.token_read_idx = data_type_token_idx;
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_VERTEX_SHADER_MUST_RETURN_RASTERIZER_STATE);
			}
			break;
		case HCC_FUNCTION_SHADER_STAGE_FRAGMENT:
			if (!hcc_data_type_is_fragment_state(c, return_data_type)) {
				c->astgen.token_read_idx = data_type_token_idx;
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FRAGMENT_SHADER_MUST_RETURN_FRAGMENT_STATE);
			}
			break;
	}

	function->identifier_string_id = identifier_string_id;
	function->return_data_type = return_data_type;
	function->return_data_type_token_idx = data_type_token_idx;

	HccDecl decl = HCC_DECL_INIT(HCC_DECL_FUNCTION, function_idx);
	if (is_new) {
		hcc_astgen_insert_global_declaration(c, identifier_string_id, decl);
	}

	hcc_astgen_variable_stack_open(c);

	if (is_new) {
		function->params_start_idx = hcc_stack_count(c->astgen.function_params_and_variables);
		function->params_count = 0;
	}
	token = hcc_astgen_token_next(c);
	U32 param_idx = 0;
	if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
		while (1) {
			HccVariable* param;
			if (is_new) {
				param = hcc_stack_push(c->astgen.function_params_and_variables);
				function->params_count += 1;
			} else {
				param = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + param_idx);
			}

			hcc_astgen_generate_specifiers(c);
			HccDataType param_data_type;
			if (!hcc_astgen_generate_data_type(c, &param_data_type)) {
				token = hcc_astgen_token_peek(c);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_TYPE_NAME, hcc_token_strings[token]);
			}
			hcc_astgen_generate_specifiers(c);

			hcc_astgen_ensure_not_unsupported_basic_type(c, 1, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED_IN_PARAM, param_data_type);

			if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_FUNCTION_PARAM_SPECIFIERS) {
				HccSpecifier specifier = HCC_LEAST_SET_BIT_IDX_U32(c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_ALL_NON_FUNCTION_PARAM_SPECIFIERS);
				HccToken token = hcc_specifier_tokens[specifier];
				hcc_astgen_error_1(c, HCC_ERROR_CODE_INVALID_SPECIFIER_FUNCTION_PARAM, hcc_token_strings[token]);
			}

			if (c->astgen.specifier_flags & HCC_SPECIFIER_FLAGS_CONST) {
				param_data_type = HCC_DATA_TYPE_CONST(param_data_type);
			}
			c->astgen.specifier_flags &= ~HCC_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS;

			token = hcc_astgen_token_peek(c);
			if (token != HCC_TOKEN_IDENT) {
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FUNCTION_PARAM);
			}
			HccStringId param_identifier_string_id = hcc_astgen_token_value_next(c).string_id;
			U32 param_identifier_token_idx = c->astgen.token_read_idx;
			if (function->shader_stage != HCC_FUNCTION_SHADER_STAGE_NONE && !HCC_DATA_TYPE_IS_CONST(param_data_type)) {
				HccString identifier = hcc_string_table_get(&c->string_table, param_identifier_string_id);
				HccString data_type_name = hcc_data_type_string(c, param_data_type);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_EXPECTED_SHADER_PARAM_TO_BE_CONST, (int)data_type_name.size, data_type_name.data, (int)identifier.size, identifier.data);
			}

			U32 existing_variable_id = hcc_astgen_variable_stack_find(c, param_identifier_string_id);
			if (existing_variable_id) {
				U32 other_token_idx = -1;// TODO: location of existing variable
				HccString string = hcc_string_table_get(&c->string_table, param_identifier_string_id);
				hcc_astgen_error_2_idx(c, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_FUNCTION_PARAM, other_token_idx, (int)string.size, string.data);
			}
			hcc_astgen_variable_stack_add(c, param_identifier_string_id);
			token = hcc_astgen_token_next(c);

			if (is_new) {
				param->data_type = param_data_type;
				param->identifier_string_id = param_identifier_string_id;
				param->identifier_token_idx = param_identifier_token_idx;
			} else {
				if (param->data_type != param_data_type) {
					HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
					HccString param_identifier_string = hcc_string_table_get(&c->string_table, param_identifier_string_id);
					HccString expected_data_type_name = hcc_data_type_string(c, param->data_type);
					HccString data_type_name = hcc_data_type_string(c, param_data_type);
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_MISMATCH_PARAM_DATA_TYPE, (int)identifier_string.size, identifier_string.data, (int)param_identifier_string.size, param_identifier_string.data, (int)expected_data_type_name.size, expected_data_type_name.data, (int)data_type_name.size, data_type_name.data);
				}
			}

			if (token != HCC_TOKEN_COMMA) {
				if (token != HCC_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FUNCTION_INVALID_TERMINATOR);
				}
				break;
			}
			token = hcc_astgen_token_next(c);

			param_idx += 1;
			if (!is_new && param_idx == function->params_count) {
				HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_TOO_MANY_PARAMETERS, (int)identifier_string.size, identifier_string.data, function->params_count);
			}
		}
	}
	token = hcc_astgen_token_next(c);

	if (!is_new && param_idx != function->params_count) {
		HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_NOT_ENOUGH_PARAMETERS, (int)identifier_string.size, identifier_string.data, function->params_count, param_idx);
	}

	switch (function->shader_stage) {
		case HCC_FUNCTION_SHADER_STAGE_VERTEX: {
			if (function->params_count != 1) {
				c->astgen.token_read_idx = params_token_idx;
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_VERTEX_INVALID_COUNT);
			}

			HccVariable* vertex_input_var = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + 0);
			if (hcc_typedef_resolve_and_keep_const(c, vertex_input_var->data_type) != HCC_DATA_TYPE_CONST(HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_STRUCT, HCC_STRUCT_IDX_VERTEX_INPUT))) {
				c->astgen.token_read_idx = vertex_input_var->identifier_token_idx;
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_VERTEX_INVALID_INPUT);
			}
			break;
		};
		case HCC_FUNCTION_SHADER_STAGE_FRAGMENT: {
			if (function->params_count != 2) {
				c->astgen.token_read_idx = params_token_idx;
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_COUNT);
			}

			HccVariable* fragment_input_var = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + 0);
			if (hcc_typedef_resolve_and_keep_const(c, fragment_input_var->data_type) != HCC_DATA_TYPE_CONST(HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_STRUCT, HCC_STRUCT_IDX_FRAGMENT_INPUT))) {
				c->astgen.token_read_idx = fragment_input_var->identifier_token_idx;
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_INPUT);
			}

			HccVariable* rasterizer_state_var = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + 1);
			if (!hcc_data_type_is_rasterizer_state(c, rasterizer_state_var->data_type)) {
				c->astgen.token_read_idx = rasterizer_state_var->identifier_token_idx;
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_RASTERIZER_STATE);
			}

			break;
		};
	}

	if (function->shader_stage != HCC_FUNCTION_SHADER_STAGE_NONE) {
		*hcc_stack_push(c->astgen.entry_point_function_indices) = function_idx;
	}

	if (function_idx < HCC_FUNCTION_IDX_INTRINSIC_END) {
		HccIntrinsicFunction* f = &hcc_intrinsic_functions[function_idx];
		if (f->return_data_type != return_data_type) {
			HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
			HccString expected_data_type_name = hcc_data_type_string(c, f->return_data_type);
			HccString data_type_name = hcc_data_type_string(c, return_data_type);
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_RETURN_DATA_TYPE, (int)identifier_string.size, identifier_string.data, (int)expected_data_type_name.size, expected_data_type_name.data, (int)data_type_name.size, data_type_name.data);
		}

		if (f->params_count != function->params_count) {
			HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
			hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_PARAMS_COUNT, (int)identifier_string.size, identifier_string.data, f->params_count, function->params_count);
		}

		for (U32 idx = 0; idx < function->params_count; idx += 1) {
			HccDataType expected_data_type = f->param_data_types[idx];
			HccVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + idx];
			if (variable->data_type != expected_data_type) {
				HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
				HccString expected_data_type_name = hcc_data_type_string(c, expected_data_type);
				HccString data_type_name = hcc_data_type_string(c, variable->data_type);
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_PARAM_DATA_TYPE, (int)identifier_string.size, identifier_string.data, idx, (int)expected_data_type_name.size, expected_data_type_name.data, (int)data_type_name.size, data_type_name.data);
			}
		}
	}

	if (token == HCC_TOKEN_SEMICOLON) {
		hcc_astgen_variable_stack_close(c);
		hcc_astgen_token_next(c);
		c->astgen.function = NULL;
		return;
	} else if (token != HCC_TOKEN_CURLY_OPEN) {
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_UNEXPECTED_TOKEN_FUNCTION_PROTOTYPE_END, hcc_token_strings[token]);
	}

	if (function->block_expr_id.idx_plus_one) {
		HccString identifier_string = hcc_string_table_get(&c->string_table, identifier_string_id);
		hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_BODY_ALREADY_DECLARED, (int)identifier_string.size, identifier_string.data);
	}

	U32 ordered_data_types_start_idx = hcc_stack_count(c->astgen.ordered_data_types);
	function->used_static_variables_start_idx = hcc_stack_count(c->astgen.function_used_static_variables);
	function->used_function_indices_start_idx = hcc_stack_count(c->astgen.function_used_function_indices);
	function->unsupported_basic_types_deferred_messages_start_idx = hcc_stack_count(c->message_sys.deferred_elmts);

	function->block_expr_id.idx_plus_one = 0;
	if (token == HCC_TOKEN_CURLY_OPEN) {
		HccExpr* expr = hcc_astgen_generate_stmt(c);
		function->block_expr_id.idx_plus_one = (expr - c->astgen.exprs) + 1;
	}

	hcc_astgen_variable_stack_close(c);
	function->variables_count = c->astgen.next_var_idx;

	function->used_static_variables_count = hcc_stack_count(c->astgen.function_used_static_variables) - function->used_static_variables_start_idx;
	function->used_function_indices_count = hcc_stack_count(c->astgen.function_used_function_indices) - function->used_function_indices_start_idx;
	function->unsupported_basic_types_deferred_messages_count = hcc_stack_count(c->message_sys.deferred_elmts) - function->unsupported_basic_types_deferred_messages_start_idx;

	//
	// TODO: these silly buggers actually shadow! so we need to store any function local declarations
	// in their own hash tables and simply nuke them at the end of a function.
	for (U32 i = ordered_data_types_start_idx; i < hcc_stack_count(c->astgen.ordered_data_types); i += 1) {
		HccDataType data_type = *hcc_stack_get(c->astgen.ordered_data_types, i);
		switch (data_type & 0xff) {
		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION: {
			HccCompoundDataType* d = hcc_compound_data_type_get(c, data_type);
			if (d->identifier_string_id.idx_plus_one) {
				HccHashTable(HccStringId, HccDataType)* declarations;
				if (HCC_DATA_TYPE_IS_UNION(data_type)) {
					declarations = &c->astgen.union_declarations;
				} else {
					declarations = &c->astgen.struct_declarations;
				}

				HCC_DEBUG_ASSERT(hcc_hash_table_remove(declarations, d->identifier_string_id.idx_plus_one, NULL), "internal error: compound type should have existed");
			}
			break;
		};
		case HCC_DATA_TYPE_ENUM: {
			HccEnumDataType* enum_data_type = hcc_enum_data_type_get(c, data_type);
			HCC_DEBUG_ASSERT(hcc_hash_table_remove(&c->astgen.enum_declarations, enum_data_type->identifier_string_id.idx_plus_one, NULL), "internal error: enum type should have existed");
			break;
		};
		case HCC_DATA_TYPE_TYPEDEF: {
			HccTypedef* typedef_ = hcc_typedef_get(c, data_type);
			HCC_DEBUG_ASSERT(hcc_hash_table_remove(&c->astgen.global_declarations, typedef_->identifier_string_id.idx_plus_one, NULL), "internal error: typedef should have existed");
			break;
		};
		}
	}

	c->astgen.function = NULL;
}

const char* hcc_astgen_make_callstack(HccCompiler* c, U32* function_indices, U32 functions_count) {
	static char callstack[1024];
	U32 size = 0;
	for (U32 idx = 0; idx < functions_count; idx += 1) {
		HccFunction* function = hcc_stack_get(c->astgen.functions, *hcc_stack_get(function_indices, idx));
		HccString identifier_string = hcc_string_table_get(&c->string_table, function->identifier_string_id);
		size += snprintf(callstack + size, sizeof(callstack) - size, "%u: %.*s\n", idx, (int)identifier_string.size, identifier_string.data);
	}
	return callstack;
}

void hcc_astgen_recurse_used_functions(HccCompiler* c, U32 function_idx) {
	HccFunction* function = hcc_stack_get(c->astgen.functions, function_idx);
	if (function_idx >= HCC_FUNCTION_IDX_USER_START && function->block_expr_id.idx_plus_one == 0) {
		hcc_astgen_bail_error_2_idx(c, HCC_ERROR_CODE_CANNOT_CALL_UNIMPLEMENTED_FUNCTION, function->identifier_token_idx);
	}

	for (U32 idx = 0; idx < hcc_stack_count(c->astgen.recursive_function_indices_stack); idx += 1) {
		if (*hcc_stack_get(c->astgen.recursive_function_indices_stack, idx) == function_idx) {
			hcc_stack_resize(c->astgen.recursive_function_indices_stack, idx);
			*hcc_stack_push(c->astgen.recursive_function_indices_stack) = function_idx;
			const char* callstack = hcc_astgen_make_callstack(c, c->astgen.recursive_function_indices_stack, hcc_stack_count(c->astgen.recursive_function_indices_stack));
			HccString identifier_string = hcc_string_table_get(&c->string_table, function->identifier_string_id);
			U32 token_idx = HCC_MIN(c->astgen.token_read_idx, function->identifier_token_idx);
			HccLocation* location = hcc_token_bag_location_get(c, &c->astgen.token_bag, token_idx);
			hcc_error_push(c, HCC_ERROR_CODE_FUNCTION_RECURSION, location, NULL, (int)identifier_string.size, identifier_string.data, callstack);
			return;
		}
	}
	*hcc_stack_push(c->astgen.recursive_function_indices_stack) = function_idx;

	for (U32 idx = 0; idx < hcc_stack_count(c->astgen.used_function_indices); idx += 1) {
		if (*hcc_stack_get(c->astgen.used_function_indices, idx) == function_idx) {
			// function has already been added to the used array and check before
			// so nothing to do for this one!
			return;
		}
	}
	*hcc_stack_push(c->astgen.used_function_indices) = function_idx;

	if (function->unsupported_basic_types_deferred_messages_count) {
		hcc_message_copy_deferred(c, function->unsupported_basic_types_deferred_messages_start_idx, function->unsupported_basic_types_deferred_messages_count);

		//
		// return as this function with the unsupported basics types is built for
		// supporting these basic types so we in turn call many other functions that use this unsupported data type.
		// lets avoid spamming the errors to the user.
		//
		return;
	}

	for (U32 called_function_idx = 0; called_function_idx < function->used_function_indices_count; called_function_idx += 1) {
		U32 used_function_idx = *hcc_stack_get(c->astgen.function_used_function_indices, function->used_function_indices_start_idx + called_function_idx);
		hcc_astgen_recurse_used_functions(c, used_function_idx);
	}

	hcc_stack_pop(c->astgen.recursive_function_indices_stack);
}

void hcc_astgen_generate(HccCompiler* c) {
	while (1) {
		HccToken token = hcc_astgen_token_peek(c);
		token = hcc_astgen_generate_specifiers(c);

		switch (token) {
			case HCC_TOKEN_EOF:
				goto END_OF_FILE;
			case HCC_TOKEN_KEYWORD_TYPEDEF:
				hcc_astgen_generate_typedef(c);
				break;
			default: {
				HccDataType data_type;
				U32 data_type_token_idx = c->astgen.token_read_idx;
				if (hcc_astgen_generate_data_type(c, &data_type)) {
					token = hcc_astgen_generate_specifiers(c);
					bool ensure_semi_colon = true;
					if (token == HCC_TOKEN_IDENT) {
						if (hcc_astgen_token_peek_ahead(c, 1) == HCC_TOKEN_PARENTHESIS_OPEN) {
							hcc_astgen_generate_function(c, data_type, data_type_token_idx);
							ensure_semi_colon = false;
						} else {
							while (1) {
								hcc_astgen_generate_variable_decl(c, true, &data_type, NULL);
								token = hcc_astgen_token_peek(c);
								if (token != HCC_TOKEN_COMMA) {
									break;
								}
								token = hcc_astgen_token_next(c);
							}
						}
					} else if (token == HCC_TOKEN_KEYWORD_TYPEDEF) {
						hcc_astgen_generate_typedef_with_data_type(c, data_type);
					} else {
						hcc_astgen_ensure_no_unused_specifiers_identifier(c);
					}
					if (ensure_semi_colon) {
						hcc_astgen_ensure_semicolon(c);
					}
					break;
				} else {
					hcc_astgen_ensure_no_unused_specifiers_data_type(c);
				}
				hcc_astgen_bail_error_1(c, HCC_ERROR_CODE_UNEXPECTED_TOKEN, hcc_token_strings[token]);
			};
		}
	}
END_OF_FILE:{}

	U32 entry_points_count = hcc_stack_count(c->astgen.entry_point_function_indices);
	for (U32 entry_point_idx = 0; entry_point_idx < entry_points_count; entry_point_idx += 1) {
		U32 entry_point_function_idx = c->astgen.entry_point_function_indices[entry_point_idx];
		hcc_astgen_recurse_used_functions(c, entry_point_function_idx);
	}
}

void hcc_astgen_print_expr(HccCompiler* c, HccExpr* expr, U32 indent, FILE* f) {
	static char* indent_chars = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	fprintf(f, "%.*s", indent, indent_chars);
	if (!expr->is_stmt_block_entry) {
		HccString data_type_name = hcc_data_type_string(c, expr->data_type);
		fprintf(f, "(%.*s)", (int)data_type_name.size, data_type_name.data);
	}

	const char* expr_name;
	switch (expr->type) {
		case HCC_EXPR_TYPE_CONSTANT: expr_name = "EXPR_CONSTANT"; goto CONSTANT;
		case HCC_EXPR_TYPE_STMT_CASE: expr_name = "STMT_CASE"; goto CONSTANT;
CONSTANT: {
			fprintf(f, "%s ", expr_name);
			HccConstantId constant_id = { .idx_plus_one = expr->constant.id };
			hcc_constant_print(c, constant_id, stdout);
			break;
		};
		case HCC_EXPR_TYPE_STMT_BLOCK: {
			U32 stmts_count = expr->stmt_block.stmts_count;
			fprintf(f, "STMT_BLOCK[%u] {\n", stmts_count);
			HccExpr* stmt = &expr[expr->stmt_block.first_expr_rel_idx];
			U32 variables_count = expr->stmt_block.variables_count;
			for (U32 i = 0; i < variables_count; i += 1) {
				char buf[1024] = "<CURLY_INITIALIZER_RESULT>";
				U32 variable_idx = c->astgen.print_variable_base_idx + i;
				HccVariable* variable = hcc_stack_get(c->astgen.function_params_and_variables, c->astgen.function->params_start_idx + variable_idx);
				if (variable->identifier_string_id.idx_plus_one) {
					hcc_variable_to_string(c, variable, buf, sizeof(buf), false);
				}
				fprintf(f, "%.*sLOCAL_VARIABLE(#%u): %s", indent + 1, indent_chars, variable_idx, buf);
				if (variable->initializer_constant_id.idx_plus_one) {
					fprintf(f, " = ");
					hcc_constant_print(c, variable->initializer_constant_id, f);
				}
				fprintf(f, "\n");
			}
			c->astgen.print_variable_base_idx += variables_count;

			for (U32 i = 0; i < stmts_count; i += 1) {
				hcc_astgen_print_expr(c, stmt, indent + 1, f);
				stmt = &stmt[stmt->next_expr_rel_idx];
			}
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_FUNCTION: {
			HccFunction* function = hcc_stack_get(c->astgen.functions, expr->function.idx);
			char buf[1024];
			hcc_function_to_string(c, function, buf, sizeof(buf), false);
			fprintf(f, "EXPR_FUNCTION Function(#%u): %s", expr->function.idx, buf);
			break;
		};
		case HCC_EXPR_TYPE_STMT_RETURN: expr_name = "STMT_RETURN"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(LOGICAL_NOT): expr_name = "EXPR_LOGICAL_NOT"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(BIT_NOT): expr_name = "EXPR_BIT_NOT"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(PLUS): expr_name = "EXPR_PLUS"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(NEGATE): expr_name = "EXPR_NEGATE"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT): expr_name = "EXPR_PRE_INCREMENT"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(PRE_DECREMENT): expr_name = "EXPR_PRE_DECREMENT"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(POST_INCREMENT): expr_name = "EXPR_POST_INCREMENT"; goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(POST_DECREMENT): expr_name = "EXPR_POST_DECREMENT"; goto UNARY;
UNARY:
		{
			fprintf(f, "%s: {\n", expr_name);
			HccExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			hcc_astgen_print_expr(c, unary_expr, indent + 1, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_CAST: {
			fprintf(f, "EXPR_CAST: {\n");
			HccExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			hcc_astgen_print_expr(c, unary_expr, indent + 1, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_IF: {
			fprintf(f, "%s: {\n", "STMT_IF");

			HccExpr* cond_expr = &expr[expr->if_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, cond_expr, indent + 2, f);

			HccExpr* true_stmt = &expr[expr->if_.true_stmt_rel_idx];
			fprintf(f, "%.*sTRUE_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, true_stmt, indent + 2, f);

			if (true_stmt->if_aux.false_stmt_rel_idx) {
				HccExpr* false_stmt = &true_stmt[true_stmt->if_aux.false_stmt_rel_idx];
				fprintf(f, "%.*sFALSE_STMT:\n", indent + 1, indent_chars);
				hcc_astgen_print_expr(c, false_stmt, indent + 2, f);
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_SWITCH: {
			fprintf(f, "%s: {\n", "STMT_SWITCH");

			HccExpr* block_expr = &expr[expr->switch_.block_expr_rel_idx];
			hcc_astgen_print_expr(c, block_expr, indent + 1, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_WHILE: {
			fprintf(f, "%s: {\n", expr->while_.cond_expr_rel_idx > expr->while_.loop_stmt_rel_idx ? "STMT_DO_WHILE" : "STMT_WHILE");

			HccExpr* cond_expr = &expr[expr->while_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, cond_expr, indent + 2, f);

			HccExpr* loop_stmt = &expr[expr->while_.loop_stmt_rel_idx];
			fprintf(f, "%.*sLOOP_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, loop_stmt, indent + 2, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_FOR: {
			fprintf(f, "%s: {\n", "STMT_FOR");

			HccExpr* init_expr = &expr[expr->for_.init_expr_rel_idx];
			fprintf(f, "%.*sINIT_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, init_expr, indent + 2, f);

			HccExpr* cond_expr = &expr[expr->for_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, cond_expr, indent + 2, f);

			HccExpr* inc_expr = &expr[expr->for_.inc_expr_rel_idx];
			fprintf(f, "%.*sINCREMENT_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, inc_expr, indent + 2, f);

			HccExpr* loop_stmt = &expr[expr->for_.loop_stmt_rel_idx];
			fprintf(f, "%.*sLOOP_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, loop_stmt, indent + 2, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_DEFAULT: {
			fprintf(f, "%s:\n", "STMT_DEFAULT");
			break;
		};
		case HCC_EXPR_TYPE_STMT_BREAK: {
			fprintf(f, "%s:\n", "STMT_BREAK");
			break;
		};
		case HCC_EXPR_TYPE_STMT_CONTINUE: {
			fprintf(f, "%s:\n", "STMT_CONTINUE");
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(ASSIGN): expr_name = "ASSIGN"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(ADD): expr_name = "ADD"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(SUBTRACT): expr_name = "SUBTRACT"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(MULTIPLY): expr_name = "MULTIPLY"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(DIVIDE): expr_name = "DIVIDE"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(MODULO): expr_name = "MODULO"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(BIT_AND): expr_name = "BIT_AND"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(BIT_OR): expr_name = "BIT_OR"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(BIT_XOR): expr_name = "BIT_XOR"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT): expr_name = "BIT_SHIFT_LEFT"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT): expr_name = "BIT_SHIFT_RIGHT"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(EQUAL): expr_name = "EQUAL"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(NOT_EQUAL): expr_name = "NOT_EQUAL"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(LESS_THAN): expr_name = "LESS_THAN"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(LESS_THAN_OR_EQUAL): expr_name = "LESS_THAN_OR_EQUAL"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(GREATER_THAN): expr_name = "GREATER_THAN"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(GREATER_THAN_OR_EQUAL): expr_name = "GREATER_THAN_OR_EQUAL"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(LOGICAL_AND): expr_name = "LOGICAL_AND"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(LOGICAL_OR): expr_name = "LOGICAL_OR"; goto BINARY;
		case HCC_EXPR_TYPE_BINARY_OP(COMMA): expr_name = "COMMA"; goto BINARY;
		case HCC_EXPR_TYPE_CALL: expr_name = "CALL"; goto BINARY;
		case HCC_EXPR_TYPE_ARRAY_SUBSCRIPT: expr_name = "ARRAY_SUBSCRIPT"; goto BINARY;
BINARY:
		{
			char* prefix = expr->binary.is_assignment ? "STMT_ASSIGN_" : "EXPR_";
			fprintf(f, "%s%s: {\n", prefix, expr_name);
			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HccExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			hcc_astgen_print_expr(c, left_expr, indent + 1, f);
			hcc_astgen_print_expr(c, right_expr, indent + 1, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_CURLY_INITIALIZER: {
			fprintf(f, "%s: {\n", "EXPR_CURLY_INITIALIZER");

			////////////////////////////////////////////////////////////////////////////
			// skip the internal variable expression that sits at the start of the initializer_expr list
			HccExpr* initializer_expr = &expr[expr->curly_initializer.first_expr_rel_idx];
			U32 expr_rel_idx;
			////////////////////////////////////////////////////////////////////////////

			while (1) {
				expr_rel_idx = initializer_expr->next_expr_rel_idx;
				if (expr_rel_idx == 0) {
					break;
				}
				initializer_expr = &initializer_expr[expr_rel_idx];

				HccAstGenDesignatorInitializer* di = &c->astgen.curly_initializer.designated_initializers[initializer_expr->alt_next_expr_rel_idx];
				U64* elmt_indices = hcc_stack_get(c->astgen.curly_initializer.designated_initializer_elmt_indices, di->elmt_indices_start_idx);
				fprintf(f, "%.*s", indent + 1, indent_chars);
				HccDataType data_type = expr->data_type;
				if (di->elmt_indices_count) {
					for (U32 idx = 0; idx < di->elmt_indices_count; idx += 1) {
						data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
						U64 entry_idx = elmt_indices[idx];
						if (HCC_DATA_TYPE_IS_ARRAY(data_type)) {
							HccArrayDataType* array_data_type = hcc_array_data_type_get(c, data_type);
							fprintf(f, "[%zu]", entry_idx);
							data_type = array_data_type->element_data_type;
						} else if (HCC_DATA_TYPE_IS_COMPOUND_TYPE(data_type)) {
							HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, data_type);
							HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, compound_data_type->fields_start_idx + entry_idx);
							if (field->identifier_string_id.idx_plus_one) {
								HccString identifier_string = hcc_string_table_get(&c->string_table, field->identifier_string_id);
								fprintf(f, ".%.*s", (int)identifier_string.size, identifier_string.data);
							}
							data_type = field->data_type;
						}
					}
				} else {
				fprintf(f, "...");
				}
				fprintf(f, " = ");

				if (initializer_expr->designated_initializer.value_expr_rel_idx) {
					HccExpr* value_expr = initializer_expr - initializer_expr->designated_initializer.value_expr_rel_idx;
					if (value_expr->type == HCC_EXPR_TYPE_CURLY_INITIALIZER) {
						fprintf(f, "\n");
					}
					hcc_astgen_print_expr(c, value_expr, value_expr->type == HCC_EXPR_TYPE_CURLY_INITIALIZER ? indent + 2 : 0, f);
				} else {
					fprintf(f, "<ZERO>\n");
				}
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_FIELD_ACCESS: {
			fprintf(f, "%s: {\n", "EXPR_FIELD_ACCESS");

			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			U32 field_idx = expr->binary.right_expr_rel_idx;
			hcc_astgen_print_expr(c, left_expr, indent + 1, f);

			HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, left_expr->data_type);
			HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, compound_data_type->fields_start_idx + field_idx);

			HccString field_data_type_name = hcc_data_type_string(c, field->data_type);
			if (field->identifier_string_id.idx_plus_one) {
				HccString identifier_string = hcc_string_table_get(&c->string_table, field->identifier_string_id);
				fprintf(f, "%.*sfield_idx(%u): %.*s %.*s\n", indent + 1, indent_chars, field_idx, (int)field_data_type_name.size, field_data_type_name.data, (int)identifier_string.size, identifier_string.data);
			} else {
				fprintf(f, "%.*sfield_idx(%u): %.*s\n", indent + 1, indent_chars, field_idx, (int)field_data_type_name.size, field_data_type_name.data);
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_CALL_ARG_LIST: {
			fprintf(f, "EXPR_CALL_ARG_LIST: {\n");
			HccExpr* arg_expr = expr;
			U32 args_count = ((U8*)expr)[1];
			U8* next_arg_expr_rel_indices = &((U8*)expr)[2];
			for (U32 i = 0; i < args_count; i += 1) {
				arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
				hcc_astgen_print_expr(c, arg_expr, indent + 1, f);
			}
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_LOCAL_VARIABLE: {
			char buf[1024];
			HccVariable* variable = hcc_stack_get(c->astgen.function_params_and_variables, c->astgen.function->params_start_idx + expr->variable.idx);
			hcc_variable_to_string(c, variable, buf, sizeof(buf), false);
			fprintf(f, "LOCAL_VARIABLE(#%u): %s", expr->variable.idx, buf);
			break;
		};
		case HCC_EXPR_TYPE_GLOBAL_VARIABLE: {
			char buf[1024];
			HccVariable* variable = hcc_stack_get(c->astgen.global_variables, expr->variable.idx);
			hcc_variable_to_string(c, variable, buf, sizeof(buf), false);
			fprintf(f, "GLOBAL_VARIABLE(#%u): %s", expr->variable.idx, buf);
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(TERNARY): {
			fprintf(f, "%s: {\n", "STMT_TERNARY");

			HccExpr* cond_expr = expr - expr->ternary.cond_expr_rel_idx;
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, cond_expr, indent + 2, f);

			HccExpr* true_stmt = expr - expr->ternary.true_expr_rel_idx;
			fprintf(f, "%.*sTRUE_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, true_stmt, indent + 2, f);

			HccExpr* false_stmt = expr - expr->ternary.false_expr_rel_idx;
			fprintf(f, "%.*sFALSE_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(c, false_stmt, indent + 2, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		default:
			HCC_ABORT("unhandled expr type %u\n", expr->type);
	}
	fprintf(f, "\n");
}

void hcc_astgen_print(HccCompiler* c, FILE* f) {
	for (U32 enum_type_idx = 0; enum_type_idx < hcc_stack_count(c->astgen.enum_data_types); enum_type_idx += 1) {
		HccEnumDataType* d = hcc_stack_get(c->astgen.enum_data_types, enum_type_idx);
		HccString name = hcc_string_lit("<anonymous>");
		if (d->identifier_string_id.idx_plus_one) {
			name = hcc_string_table_get(&c->string_table, d->identifier_string_id);
		}
		fprintf(f, "ENUM(#%u): %.*s {\n", enum_type_idx, (int)name.size, name.data);
		for (U32 value_idx = 0; value_idx < d->values_count; value_idx += 1) {
			HccEnumValue* value = hcc_stack_get(c->astgen.enum_values, d->values_start_idx + value_idx);
			HccString identifier = hcc_string_table_get_or_empty(&c->string_table, value->identifier_string_id);

			HccConstant constant = hcc_constant_table_get(c, value->value_constant_id);

			S64 v;
			HCC_DEBUG_ASSERT(hcc_constant_as_sint(c, constant, &v), "internal error: expected to be a signed int");
			fprintf(f, "\t%.*s = %ld\n", (int)identifier.size, identifier.data, v);
		}
		fprintf(f, "}\n");
	}

	for (U32 compound_type_idx = 0; compound_type_idx < hcc_stack_count(c->astgen.compound_data_types); compound_type_idx += 1) {
		HccCompoundDataType* d = hcc_stack_get(c->astgen.compound_data_types, compound_type_idx);
		HccString name = hcc_string_lit("<anonymous>");
		if (d->identifier_string_id.idx_plus_one) {
			name = hcc_string_table_get(&c->string_table, d->identifier_string_id);
		}
		char* compound_name = d->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION ? "UNION" : "STRUCT";
		fprintf(f, "%s(#%u): %.*s {\n", compound_name, compound_type_idx, (int)name.size, name.data);
		fprintf(f, "\tsize: %zu\n", d->size);
		fprintf(f, "\talign: %zu\n", d->align);
		fprintf(f, "\tfields: {\n");
		for (U32 field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
			HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, d->fields_start_idx + field_idx);
			HccString data_type_name = hcc_data_type_string(c, field->data_type);
			fprintf(f, "\t\t%.*s ", (int)data_type_name.size, data_type_name.data);
			if (field->identifier_string_id.idx_plus_one) {
				HccString identifier = hcc_string_table_get_or_empty(&c->string_table, field->identifier_string_id);
				fprintf(f, "%.*s\n", (int)identifier.size, identifier.data);
			} else {
				fprintf(f, "\n");
			}
		}
		fprintf(f, "\t}\n");
		fprintf(f, "}\n");
	}

	for (U32 array_type_idx = 0; array_type_idx < hcc_stack_count(c->astgen.array_data_types); array_type_idx += 1) {
		HccArrayDataType* d = hcc_stack_get(c->astgen.array_data_types, array_type_idx);
		HccString data_type_name = hcc_data_type_string(c, d->element_data_type);

		HccConstant constant = hcc_constant_table_get(c, d->size_constant_id);

		U64 count;
		HCC_DEBUG_ASSERT(hcc_constant_as_uint(c, constant, &count), "internal error: expected to be a unsigned int");

		fprintf(f, "ARRAY(#%u): %.*s[%zu]\n", array_type_idx, (int)data_type_name.size, data_type_name.data, count);
	}

	for (U32 typedefs_idx = 0; typedefs_idx < hcc_stack_count(c->astgen.typedefs); typedefs_idx += 1) {
		HccTypedef* d = hcc_stack_get(c->astgen.typedefs, typedefs_idx);
		HccString name = hcc_string_table_get_or_empty(&c->string_table, d->identifier_string_id);
		HccString aliased_data_type_name = hcc_data_type_string(c, d->aliased_data_type);
		fprintf(f, "typedef(#%u) %.*s %.*s\n", typedefs_idx, (int)aliased_data_type_name.size, aliased_data_type_name.data, (int)name.size, name.data);
	}

	for (U32 variable_idx = 0; variable_idx < hcc_stack_count(c->astgen.global_variables); variable_idx += 1) {
		HccVariable* variable = hcc_stack_get(c->astgen.global_variables, variable_idx);

		char buf[1024];
		hcc_variable_to_string(c, variable, buf, sizeof(buf), false);
		fprintf(f, "GLOBAL_VARIABLE(#%u): %s", variable_idx, buf);
		fprintf(f, " = ");
		hcc_constant_print(c, variable->initializer_constant_id, f);
		fprintf(f, "\n");
	}

	for (U32 function_idx = 0; function_idx < hcc_stack_count(c->astgen.functions); function_idx += 1) {
		HccFunction* function = hcc_stack_get(c->astgen.functions, function_idx);
		if (function->identifier_string_id.idx_plus_one == 0) {
			continue;
		}
		HccString name = hcc_string_table_get_or_empty(&c->string_table, function->identifier_string_id);
		HccString return_data_type_name = hcc_data_type_string(c, function->return_data_type);
		fprintf(f, "Function(#%u): %.*s {\n", function_idx, (int)name.size, name.data);
		fprintf(f, "\treturn_type: %.*s\n", (int)return_data_type_name.size, return_data_type_name.data);
		fprintf(f, "\tshader_stage: %s\n", hcc_function_shader_stage_strings[function->shader_stage]);
		fprintf(f, "\tstatic: %s\n", function->flags & HCC_FUNCTION_FLAGS_STATIC ? "true" : "false");
		fprintf(f, "\tconst: %s\n", function->flags & HCC_FUNCTION_FLAGS_CONST ? "true" : "false");
		fprintf(f, "\tinline: %s\n", function->flags & HCC_FUNCTION_FLAGS_INLINE ? "true" : "false");
		if (function->params_count) {
			fprintf(f, "\tparams[%u]: {\n", function->params_count);
			for (U32 param_idx = 0; param_idx < function->params_count; param_idx += 1) {
				HccVariable* param = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + param_idx);
				HccString type_name = hcc_data_type_string(c, param->data_type);
				HccString param_name = hcc_string_table_get(&c->string_table, param->identifier_string_id);
				fprintf(f, "\t\t%.*s %.*s\n", (int)type_name.size, type_name.data, (int)param_name.size, param_name.data);
			}
			fprintf(f, "\t}\n");
		}
		if (function->block_expr_id.idx_plus_one) {
			c->astgen.function = function;
			c->astgen.print_variable_base_idx = function->params_count;
			HccExpr* expr = &c->astgen.exprs[function->block_expr_id.idx_plus_one - 1];
			hcc_astgen_print_expr(c, expr, 1, f);
		}
		fprintf(f, "}\n");
	}
}

// ===========================================
//
//
// IR Generation
//
//
// ===========================================

void hcc_irgen_init(HccCompiler* c, HccCompilerSetup* setup) {
	c->irgen.functions = hcc_stack_init(HccIRFunction, setup->irgen.functions_cap, HCC_ALLOC_TAG_IRGEN_FUNCTIONS);
	c->irgen.basic_blocks = hcc_stack_init(HccIRBasicBlock, setup->irgen.basic_blocks_cap, HCC_ALLOC_TAG_IRGEN_BASIC_BLOCKS);
	c->irgen.values = hcc_stack_init(HccIRValue, setup->irgen.values_cap, HCC_ALLOC_TAG_IRGEN_VALUES);
	c->irgen.instructions = hcc_stack_init(HccIRInstr, setup->irgen.instructions_cap, HCC_ALLOC_TAG_IRGEN_INSTRUCTIONS);
	c->irgen.operands = hcc_stack_init(HccIROperand, setup->irgen.operands_cap, HCC_ALLOC_TAG_IRGEN_OPERANDS);
	c->irgen.function_call_param_data_types = hcc_stack_init(HccDataType, setup->irgen.function_call_param_data_types_cap, HCC_ALLOC_TAG_IRGEN_FUNCTION_CALL_PARAM_DATA_TYPES);

	hcc_stack_resize(c->irgen.functions, HCC_FUNCTION_IDX_USER_START);
}

HccIRBasicBlock* hcc_irgen_current_basic_block(HccCompiler* c) {
	return hcc_stack_get_last(c->irgen.basic_blocks);
}

HccIRBasicBlock* hcc_irgen_add_basic_block(HccCompiler* c) {
	HccIRFunction* ir_function = c->irgen.ir_function;

	HccIRBasicBlock* basic_block = hcc_stack_push(c->irgen.basic_blocks);
	ir_function->basic_blocks_count += 1;
	basic_block->instructions_start_idx = ir_function->instructions_count;
	return basic_block;
}

U16 hcc_irgen_add_value(HccCompiler* c, HccDataType data_type) {
	HccIRFunction* ir_function = c->irgen.ir_function;

	HccIRValue* value = hcc_stack_push(c->irgen.values);
	value->data_type = data_type;
	value->defined_instruction_idx = ir_function->instructions_count - 1;
	value->last_used_instruction_idx = ir_function->instructions_count - 1;

	U16 value_idx = ir_function->values_count;
	ir_function->values_count += 1;
	return value_idx;
}

void hcc_irgen_add_instruction(HccCompiler* c, HccIROpCode op_code, HccIROperand* operands, U32 operands_count) {
	HccIRFunction* ir_function = c->irgen.ir_function;
	HccIRBasicBlock* basic_block = hcc_irgen_current_basic_block(c);

	HccIRInstr* instruction = hcc_stack_push(c->irgen.instructions);
	instruction->op_code = op_code;
	instruction->operands_start_idx = (operands - c->irgen.operands) - ir_function->operands_start_idx;
	instruction->operands_count = operands_count;

	basic_block->instructions_count += 1;
	ir_function->instructions_count += 1;
#if HCC_DEBUG_ASSERTIONS
	//
	// TODO validation
	switch (instruction->op_code) {
		default:break;
	}
#endif // HCC_DEBUG_ASSERTIONS
}

void hcc_irgen_remove_last_instruction(HccCompiler* c) {
	HccIRFunction* ir_function = c->irgen.ir_function;
	HccIRBasicBlock* basic_block = hcc_irgen_current_basic_block(c);

	U16 operands_count = hcc_stack_get_last(c->irgen.instructions)->operands_count;
	hcc_stack_pop_many(c->irgen.operands, operands_count);
	hcc_stack_pop(c->irgen.instructions);

	ir_function->instructions_count -= 1;
	ir_function->operands_count -= operands_count;
	basic_block->instructions_count -= 1;
}

HccIROperand* hcc_irgen_add_operands_many(HccCompiler* c, U32 amount) {
	HccIRFunction* ir_function = c->irgen.ir_function;
	ir_function->operands_count += amount;

	return hcc_stack_push_many(c->irgen.operands, amount);
}

void hcc_irgen_shrink_last_operands_count(HccCompiler* c, U32 new_amount) {
	HccIRFunction* ir_function = c->irgen.ir_function;

	HccIRInstr* instruction = hcc_stack_get_last(c->irgen.instructions);
	U32 amount = instruction->operands_count;
	HCC_DEBUG_ASSERT(amount >= new_amount, "internal error: new amount is larger than the original");

	U32 shrink_by = amount - new_amount;
	hcc_stack_pop_many(c->irgen.operands, shrink_by);
	ir_function->operands_count -= shrink_by;
	instruction->operands_count -= shrink_by;
}

U16 hcc_irgen_basic_block_idx(HccCompiler* c, HccIRBasicBlock* basic_block) {
	HccIRFunction* ir_function = c->irgen.ir_function;
	return (basic_block - c->irgen.basic_blocks) - ir_function->basic_blocks_start_idx;
}

HccDataType hcc_irgen_operand_data_type(HccCompiler* c, HccIRFunction* ir_function, HccIROperand ir_operand) {
	switch (ir_operand & 0xff) {
		case HCC_IR_OPERAND_VALUE: {
			HccIRValue* value = hcc_stack_get(c->irgen.values, ir_function->values_start_idx + HCC_IR_OPERAND_VALUE_IDX(ir_operand));
			return value->data_type;
		};
		case HCC_IR_OPERAND_CONSTANT: {
			HccConstant constant = hcc_constant_table_get(c, HCC_IR_OPERAND_CONSTANT_ID(ir_operand));
			return constant.data_type;
		};
		case HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24:
			HCC_UNREACHABLE("not sure if we need to get the data type for an immediate constant");
		case HCC_IR_OPERAND_BASIC_BLOCK:
			HCC_UNREACHABLE("cannot get the type of a basic block");
		case HCC_IR_OPERAND_LOCAL_VARIABLE: {
			U32 function_idx = ir_function - c->irgen.functions;
			HccFunction* function = hcc_stack_get(c->astgen.functions, function_idx);
			HccVariable* variable = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + HCC_IR_OPERAND_VARIABLE_IDX(ir_operand));
			return variable->data_type;
		};
		case HCC_IR_OPERAND_GLOBAL_VARIABLE: {
			HccVariable* variable = hcc_stack_get(c->astgen.global_variables, HCC_IR_OPERAND_VARIABLE_IDX(ir_operand));
			return variable->data_type;
		};
		default:
			return (HccDataType)ir_operand;
	}
}

void hcc_irgen_generate_convert_to_bool(HccCompiler* c, HccIROperand cond_operand) {
	HccIRFunction* ir_function = c->irgen.ir_function;
	HccDataType cond_data_type = hcc_typedef_resolve_and_strip_const(c, hcc_irgen_operand_data_type(c, ir_function, cond_operand));
	HCC_DEBUG_ASSERT(
		!HCC_DATA_TYPE_IS_COMPOSITE_TYPE(cond_data_type),
		"a condition expression must be a non-composite type"
	);

	HccIROperand* operands = hcc_irgen_add_operands_many(c, 3);
	hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BINARY_OP(NOT_EQUAL), operands, 3);

	U16 return_value_idx = hcc_irgen_add_value(c, HCC_DATA_TYPE_BOOL);
	operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = cond_operand;
	operands[2] = HCC_IR_OPERAND_CONSTANT_INIT(hcc_constant_table_deduplicate_zero(c, cond_data_type).idx_plus_one);
	c->irgen.last_operand = operands[0];
}

void hcc_irgen_generate_condition_expr(HccCompiler* c, HccExpr* cond_expr) {
	hcc_irgen_generate_instructions(c, cond_expr);
	HccIROperand cond_operand = c->irgen.last_operand;
	HccIRFunction* ir_function = c->irgen.ir_function;
	HccDataType cond_data_type = hcc_irgen_operand_data_type(c, ir_function, cond_operand);
	if (cond_data_type != HCC_DATA_TYPE_BOOL) {
		hcc_irgen_generate_convert_to_bool(c, cond_operand);
	}
}

void hcc_irgen_generate_case_instructions(HccCompiler* c, HccExpr* first_stmt) {
	HccExpr* stmt = first_stmt;
	while (1) {
		hcc_irgen_generate_instructions(c, stmt);
		if (
			stmt->type == HCC_EXPR_TYPE_STMT_CASE ||
			stmt->type == HCC_EXPR_TYPE_STMT_DEFAULT
		) {
			break;
		}
		stmt = &stmt[stmt->next_expr_rel_idx];
	}
	c->irgen.branch_state.all_cases_return &= hcc_stmt_has_return(stmt);
}

void hcc_irgen_generate_load(HccCompiler* c, HccDataType data_type, HccIROperand src_operand) {
	HccIROperand* operands = hcc_irgen_add_operands_many(c, 2);
	U16 return_value_idx = hcc_irgen_add_value(c, data_type);
	operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = src_operand;
	hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_LOAD, operands, 2);

	c->irgen.last_operand = operands[0];
}

void hcc_irgen_generate_store(HccCompiler* c, HccIROperand dst_operand, HccIROperand src_operand) {
	HccIROperand* operands = hcc_irgen_add_operands_many(c, 2);
	operands[0] = dst_operand;
	operands[1] = src_operand;
	hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_STORE, operands, 2);
}

void hcc_irgen_generate_bitcast(HccCompiler* c, HccDataType dst_data_type, HccIROperand src_operand, bool is_ptr) {
	HccIROperand* operands = hcc_irgen_add_operands_many(c, 3);
	hcc_irgen_add_instruction(c, is_ptr ? HCC_IR_OP_CODE_BITCAST_PTR : HCC_IR_OP_CODE_BITCAST, operands, 3);

	U16 return_value_idx = hcc_irgen_add_value(c, dst_data_type);
	operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = dst_data_type;
	operands[2] = src_operand;

	c->irgen.last_operand = operands[0];
}

void hcc_irgen_generate_bitcast_union_field(HccCompiler* c, HccDataType union_data_type, U32 field_idx, HccIROperand src_operand) {
	HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, union_data_type);
	HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, compound_data_type->fields_start_idx + field_idx);

	hcc_irgen_generate_bitcast(c, field->data_type, src_operand, true);
}

HccIROperand* hcc_irgen_generate_access_chain_start(HccCompiler* c, U32 count) {
	HccIROperand* operands = hcc_irgen_add_operands_many(c, count + 3);
	hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_ACCESS_CHAIN, operands, count + 3);

	U16 return_value_idx = hcc_irgen_add_value(c, HCC_DATA_TYPE_VOID);
	operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = c->irgen.last_operand;

	c->irgen.last_operand = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
	return operands;
}

void hcc_irgen_generate_access_chain_end(HccCompiler* c, HccDataType data_type) {
	U16 operands_count = hcc_stack_get_last(c->irgen.instructions)->operands_count;
	HccIROperand* operands = hcc_stack_get_back(c->irgen.operands, operands_count - 1);
	if (operands_count <= 3) {
		//
		// no accesses where generated so remove the access chain instruction and
		// return the original source of the access chain.
		hcc_irgen_remove_last_instruction(c);
		c->irgen.last_operand = operands[1];
		return;
	}

	HccIRFunction* ir_function = c->irgen.ir_function;
	HccIRValue* value = hcc_stack_get(c->irgen.values, ir_function->values_start_idx + HCC_IR_OPERAND_VALUE_IDX(operands[0]));
	value->data_type = data_type;
	operands[2] = data_type;
}

void hcc_irgen_generate_access_chain_instruction(HccCompiler* c, HccExpr* expr, U32 count) {
	switch (expr->type) {
		case HCC_EXPR_TYPE_FIELD_ACCESS: {
			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			U32 child_count = count + 1;
			HccDataType resolved_left_expr_data_type = hcc_typedef_resolve_and_strip_const(c, left_expr->data_type);
			if (HCC_DATA_TYPE_IS_UNION(resolved_left_expr_data_type)) {
				child_count = 0;
			}
			hcc_irgen_generate_access_chain_instruction(c, left_expr, child_count);
			S32 field_idx = expr->binary.right_expr_rel_idx;

			if (HCC_DATA_TYPE_IS_UNION(resolved_left_expr_data_type)) {
				hcc_irgen_generate_access_chain_end(c, left_expr->data_type);
				hcc_irgen_generate_bitcast_union_field(c, left_expr->data_type, field_idx, c->irgen.last_operand);
				if (count != 0) {
					hcc_irgen_generate_access_chain_start(c, count);
				}
			} else {
				HccBasic basic = hcc_basic_from_sint(c, HCC_DATA_TYPE_SINT, field_idx);
				HccConstantId constant_id = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SINT, &basic);
				*hcc_stack_get_back(c->irgen.operands, count) = HCC_IR_OPERAND_CONSTANT_INIT(constant_id.idx_plus_one);
			}
			break;
		};
		case HCC_EXPR_TYPE_ARRAY_SUBSCRIPT: {
			HccExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			hcc_irgen_generate_instructions(c, right_expr);
			HccIROperand right_operand = c->irgen.last_operand;

			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			hcc_irgen_generate_access_chain_instruction(c, left_expr, count + 1);

			*hcc_stack_get_back(c->irgen.operands, count) = right_operand;
			break;
		};
		default: {
			c->irgen.do_not_load_variable = true;
			hcc_irgen_generate_instructions(c, expr);
			hcc_irgen_generate_access_chain_start(c, count);
			break;
		};
	}
}

void hcc_irgen_add_shader_param_load(HccCompiler* c, HccIROpCode op_code, HccDataType field_data_type, U32 field_idx) {
	HccIROperand* operands = hcc_irgen_add_operands_many(c, 2);
	U16 return_value_idx = hcc_irgen_add_value(c, field_data_type);
	operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24_INIT(field_idx);
	hcc_irgen_add_instruction(c, op_code, operands, 2);

	c->irgen.last_operand = operands[0];
}

void hcc_irgen_generate_instructions(HccCompiler* c, HccExpr* expr) {
	HccIROpCode op_code;
	switch (expr->type) {
		case HCC_EXPR_TYPE_STMT_BLOCK: {
			U32 stmts_count = expr->stmt_block.stmts_count;
			HccExpr* stmt = &expr[expr->stmt_block.first_expr_rel_idx];
			for (U32 i = 0; i < stmts_count; i += 1) {
				hcc_irgen_generate_instructions(c, stmt);
				stmt = &stmt[stmt->next_expr_rel_idx];
			}

			break;
		};
		case HCC_EXPR_TYPE_STMT_RETURN: {
			HccExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			c->irgen.do_not_load_variable = c->irgen.function->shader_stage == HCC_FUNCTION_SHADER_STAGE_VERTEX || c->irgen.function->shader_stage == HCC_FUNCTION_SHADER_STAGE_FRAGMENT;
			hcc_irgen_generate_instructions(c, unary_expr);

			HccIROperand* operands = hcc_irgen_add_operands_many(c, 1);
			operands[0] = c->irgen.last_operand;
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_FUNCTION_RETURN, operands, 1);
			break;
		};
		case HCC_EXPR_TYPE_UNARY_OP(PLUS): {
			HccExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			hcc_irgen_generate_instructions(c, unary_expr);
			break;
		};
		case HCC_EXPR_TYPE_UNARY_OP(LOGICAL_NOT): op_code = HCC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT); goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(BIT_NOT): op_code = HCC_IR_OP_CODE_UNARY_OP(BIT_NOT); goto UNARY;
		case HCC_EXPR_TYPE_UNARY_OP(NEGATE): op_code = HCC_IR_OP_CODE_UNARY_OP(NEGATE); goto UNARY;
UNARY:
		{
			HccExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			if (op_code == HCC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT) && unary_expr->data_type != HCC_DATA_TYPE_BOOL) {
				hcc_irgen_generate_condition_expr(c, unary_expr);
			} else {
				hcc_irgen_generate_instructions(c, unary_expr);
			}

			U16 return_value_idx = hcc_irgen_add_value(c, expr->data_type);
			HccIROperand* operands = hcc_irgen_add_operands_many(c, 2);
			operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
			operands[1] = c->irgen.last_operand;
			hcc_irgen_add_instruction(c, op_code, operands, 2);

			c->irgen.last_operand = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
			break;
		};
		case HCC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT):
		case HCC_EXPR_TYPE_UNARY_OP(PRE_DECREMENT):
		case HCC_EXPR_TYPE_UNARY_OP(POST_INCREMENT):
		case HCC_EXPR_TYPE_UNARY_OP(POST_DECREMENT):
		{
			HccExpr* unary_expr = expr - expr->unary.expr_rel_idx;

			c->irgen.do_not_load_variable = true;
			hcc_irgen_generate_instructions(c, unary_expr);
			HccIROperand target_operand = c->irgen.last_operand;

			hcc_irgen_generate_instructions(c, unary_expr);
			HccIROperand value_operand = c->irgen.last_operand;

			HccIROpCode op_code;
			if (expr->type == HCC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT) || expr->type == HCC_EXPR_TYPE_UNARY_OP(POST_INCREMENT)) {
				op_code = HCC_IR_OP_CODE_BINARY_OP(ADD);
			} else {
				op_code = HCC_IR_OP_CODE_BINARY_OP(SUBTRACT);
			}

			HccDataType resolved_data_type = hcc_typedef_resolve_and_strip_const(c, expr->data_type);
			HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_BASIC(resolved_data_type), "internal error: expected basic type");

			HccIROperand* operands = hcc_irgen_add_operands_many(c, 3);
			U16 result_value_idx = hcc_irgen_add_value(c, resolved_data_type);
			operands[0] = HCC_IR_OPERAND_VALUE_INIT(result_value_idx);
			operands[1] = value_operand;
			operands[2] = HCC_IR_OPERAND_CONSTANT_INIT(c->basic_type_one_constant_ids[resolved_data_type].idx_plus_one);
			hcc_irgen_add_instruction(c, op_code, operands, 3);
			hcc_irgen_generate_store(c, target_operand, operands[0]);

			if (expr->type == HCC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT) || expr->type == HCC_EXPR_TYPE_UNARY_OP(PRE_DECREMENT)) {
				c->irgen.last_operand = operands[0];
			} else {
				c->irgen.last_operand = value_operand;
			}

			break;
		};
		case HCC_EXPR_TYPE_STMT_IF: {
			HccExpr* cond_expr = &expr[expr->if_.cond_expr_rel_idx];
			hcc_irgen_generate_condition_expr(c, cond_expr);
			HccIROperand cond_operand = c->irgen.last_operand;

			HccIROperand* selection_merge_operands = hcc_irgen_add_operands_many(c, 1);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_SELECTION_MERGE, selection_merge_operands, 1);

			HccIROperand* cond_branch_operands = hcc_irgen_add_operands_many(c, 3);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);
			cond_branch_operands[0] = cond_operand;

			HccExpr* true_stmt = &expr[expr->if_.true_stmt_rel_idx];
			bool true_needs_branch = !hcc_stmt_has_return(true_stmt);
			HccIRBasicBlock* true_basic_block = hcc_irgen_add_basic_block(c);
			hcc_irgen_generate_instructions(c, true_stmt);
			cond_branch_operands[1] = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, true_basic_block));

			HccIROperand* true_branch_operands;
			if (true_needs_branch) {
				true_branch_operands = hcc_irgen_add_operands_many(c, 1);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, true_branch_operands, 1);
			}

			HccIROperand* false_branch_operands;
			bool false_needs_branch;
			if (true_stmt->if_aux.false_stmt_rel_idx) {
				HccExpr* false_stmt = &true_stmt[true_stmt->if_aux.false_stmt_rel_idx];
				false_needs_branch = !hcc_stmt_has_return(false_stmt);
				HccIRBasicBlock* false_basic_block = hcc_irgen_add_basic_block(c);
				hcc_irgen_generate_instructions(c, false_stmt);
				cond_branch_operands[2] = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, false_basic_block));

				if (false_needs_branch) {
					false_branch_operands = hcc_irgen_add_operands_many(c, 1);
					hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, false_branch_operands, 1);
				}
			}

			HccIRBasicBlock* basic_block = hcc_irgen_add_basic_block(c);
			HccIROperand converging_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));
			selection_merge_operands[0] = converging_basic_block_operand;
			if (true_needs_branch) {
				true_branch_operands[0] = converging_basic_block_operand;
			}

			if (true_stmt->if_aux.false_stmt_rel_idx) {
				if (false_needs_branch) {
					false_branch_operands[0] = converging_basic_block_operand;
				}
			} else {
				cond_branch_operands[2] = converging_basic_block_operand;
			}

			if (!true_needs_branch && !false_needs_branch) {
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_UNREACHABLE, NULL, 0);
			}
			break;
		};
		case HCC_EXPR_TYPE_STMT_SWITCH: {
			HccExpr* block_stmt = &expr[expr->switch_.block_expr_rel_idx];
			if (block_stmt->switch_aux.first_case_expr_rel_idx == 0) {
				break;
			}

			HccIROperand* selection_merge_operands = hcc_irgen_add_operands_many(c, 1);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_SELECTION_MERGE, selection_merge_operands, 1);

			U32 operands_count = 2 + block_stmt->switch_aux.case_stmts_count * 2;
			HccIROperand* operands = hcc_irgen_add_operands_many(c, operands_count);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_SWITCH, operands, operands_count);

			HccExpr* cond_expr = &expr[expr->switch_.cond_expr_rel_idx];
			hcc_irgen_generate_instructions(c, cond_expr);
			operands[0] = c->irgen.last_operand;

			HccExpr* case_expr = &block_stmt[block_stmt->switch_aux.first_case_expr_rel_idx];
			U32 case_idx = 0;

			HccIRBranchState prev_branch_state = c->irgen.branch_state;
			c->irgen.branch_state.all_cases_return = true;
			c->irgen.branch_state.break_branch_linked_list_head = -1;
			c->irgen.branch_state.break_branch_linked_list_tail = -1;
			c->irgen.branch_state.continue_branch_linked_list_head = -1;
			c->irgen.branch_state.continue_branch_linked_list_tail = -1;
			while (1) {
				HccIRBasicBlock* basic_block = hcc_irgen_add_basic_block(c);
				operands[2 + (case_idx * 2) + 0] = HCC_IR_OPERAND_CONSTANT_INIT(case_expr->constant.id);
				operands[2 + (case_idx * 2) + 1] = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));
				if (case_expr->next_expr_rel_idx == 0) {
					break;
				}

				HccExpr* first_stmt = &case_expr[case_expr->next_expr_rel_idx];
				hcc_irgen_generate_case_instructions(c, first_stmt);

				if (case_expr->alt_next_expr_rel_idx == 0) {
					break;
				}
				case_expr = &case_expr[case_expr->alt_next_expr_rel_idx];
				case_idx += 1;
			}

			//HccIROperand* default_branch_operands;
			if (expr->alt_next_expr_rel_idx) {
				HccExpr* default_case_expr = &expr[expr->alt_next_expr_rel_idx];

				HccIRBasicBlock* default_basic_block = hcc_irgen_add_basic_block(c);
				HccExpr* first_stmt = &default_case_expr[1];
				hcc_irgen_generate_case_instructions(c, first_stmt);

				/*
				default_branch_operands = hcc_irgen_add_operands_many(c, 1);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, default_branch_operands, 1);
				*/

				operands[1] = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, default_basic_block));
			}

			HccIRBasicBlock* converging_basic_block = hcc_irgen_add_basic_block(c);
			HccIROperand converging_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, converging_basic_block));

			selection_merge_operands[0] = converging_basic_block_operand;
			if (expr->alt_next_expr_rel_idx) {
				//default_branch_operands[1] = converging_basic_block_operand;
			} else {
				operands[1] = converging_basic_block_operand;
			}

			if (c->irgen.branch_state.all_cases_return) {
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_UNREACHABLE, NULL, 0);
			}

			HccIRFunction* ir_function = c->irgen.ir_function;
			while (c->irgen.branch_state.break_branch_linked_list_head != (U32)-1) {
				U32 next = *hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.break_branch_linked_list_head);
				*hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.break_branch_linked_list_head) = converging_basic_block_operand;
				c->irgen.branch_state.break_branch_linked_list_head = next;
			}

			c->irgen.branch_state = prev_branch_state;

			break;
		};
		case HCC_EXPR_TYPE_STMT_WHILE:
		case HCC_EXPR_TYPE_STMT_FOR: {
			HccExpr* init_expr;
			HccExpr* cond_expr;
			HccExpr* inc_expr;
			HccExpr* loop_stmt;
			bool is_do_while;
			if (expr->type == HCC_EXPR_TYPE_STMT_FOR) {
				is_do_while = false;
				init_expr = &expr[expr->for_.init_expr_rel_idx];
				cond_expr = &expr[expr->for_.cond_expr_rel_idx];
				inc_expr = &expr[expr->for_.inc_expr_rel_idx];
				loop_stmt = &expr[expr->for_.loop_stmt_rel_idx];
			} else {
				is_do_while = expr->while_.cond_expr_rel_idx > expr->while_.loop_stmt_rel_idx;
				init_expr = NULL;
				cond_expr = &expr[expr->while_.cond_expr_rel_idx];
				inc_expr = NULL;
				loop_stmt = &expr[expr->while_.loop_stmt_rel_idx];
			}

			if (init_expr) {
				hcc_irgen_generate_instructions(c, init_expr);
			}

			HccIROperand* operands = hcc_irgen_add_operands_many(c, 1);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, operands, 1);

			HccIRBasicBlock* basic_block = hcc_irgen_add_basic_block(c);
			HccIROperand starting_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));
			operands[0] = starting_basic_block_operand;

			HccIROperand cond_operand;
			if (!is_do_while) {
				hcc_irgen_generate_condition_expr(c, cond_expr);
				cond_operand = c->irgen.last_operand;
			}

			HccIROperand* loop_merge_operands = hcc_irgen_add_operands_many(c, 2);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_LOOP_MERGE, loop_merge_operands, 2);

			HccIROperand* cond_branch_operands;
			if (is_do_while) {
				cond_branch_operands = hcc_irgen_add_operands_many(c, 1);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, cond_branch_operands, 1);
			} else {
				cond_branch_operands = hcc_irgen_add_operands_many(c, 3);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);
				cond_branch_operands[0] = cond_operand;
			}

			HccIRBranchState prev_branch_state = c->irgen.branch_state;
			c->irgen.branch_state.break_branch_linked_list_head = -1;
			c->irgen.branch_state.break_branch_linked_list_tail = -1;
			c->irgen.branch_state.continue_branch_linked_list_head = -1;
			c->irgen.branch_state.continue_branch_linked_list_tail = -1;

			HccIRBasicBlock* loop_basic_block = hcc_irgen_add_basic_block(c);
			hcc_irgen_generate_instructions(c, loop_stmt);
			HccIROperand loop_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, loop_basic_block));
			cond_branch_operands[is_do_while ? 0 : 1] = loop_basic_block_operand;

			HccIROpCode last_op_code = hcc_stack_get_last(c->irgen.instructions)->op_code;
			HccIROperand* loop_branch_operands = NULL;
			if (last_op_code != HCC_IR_OP_CODE_BRANCH && last_op_code != HCC_IR_OP_CODE_FUNCTION_RETURN) {
				loop_branch_operands = hcc_irgen_add_operands_many(c, 1);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, loop_branch_operands, 1);
			}

			basic_block = hcc_irgen_add_basic_block(c);
			HccIROperand continue_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));
			if (loop_branch_operands) {
				loop_branch_operands[0] = continue_basic_block_operand;
			}
			loop_merge_operands[1] = continue_basic_block_operand;
			if (inc_expr) {
				hcc_irgen_generate_instructions(c, inc_expr);
			}

			if (is_do_while) {
				hcc_irgen_generate_condition_expr(c, cond_expr);
				cond_operand = c->irgen.last_operand;

				cond_branch_operands = hcc_irgen_add_operands_many(c, 3);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);

				cond_branch_operands[0] = cond_operand;
				cond_branch_operands[1] = starting_basic_block_operand;
			} else {
				operands = hcc_irgen_add_operands_many(c, 1);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, operands, 1);
				operands[0] = starting_basic_block_operand;
			}

			basic_block = hcc_irgen_add_basic_block(c);
			HccIROperand converging_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));
			cond_branch_operands[2] = converging_basic_block_operand;
			loop_merge_operands[0] = converging_basic_block_operand;

			HccIRFunction* ir_function = c->irgen.ir_function;
			while (c->irgen.branch_state.break_branch_linked_list_head != (U32)-1) {
				U32 next = *hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.break_branch_linked_list_head);
				*hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.break_branch_linked_list_head) = converging_basic_block_operand;
				c->irgen.branch_state.break_branch_linked_list_head = next;
			}

			while (c->irgen.branch_state.continue_branch_linked_list_head != (U32)-1) {
				U32 next = *hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.continue_branch_linked_list_head);
				*hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.continue_branch_linked_list_head) = continue_basic_block_operand;
				c->irgen.branch_state.continue_branch_linked_list_head = next;
			}

			c->irgen.branch_state = prev_branch_state;
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(ASSIGN):
		{
			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HccExpr* right_expr = expr - expr->binary.right_expr_rel_idx;

			c->irgen.do_not_load_variable = true;
			hcc_irgen_generate_instructions(c, left_expr);
			HccIROperand left_operand = c->irgen.last_operand;

			HccIRFunction* ir_function = c->irgen.ir_function;
			c->irgen.assign_data_type = hcc_irgen_operand_data_type(c, ir_function, left_operand);
			hcc_irgen_generate_instructions(c, right_expr);
			HccIROperand right_operand = c->irgen.last_operand;

			hcc_irgen_generate_store(c, left_operand, right_operand);
			c->irgen.last_operand = left_operand;
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(ADD):
		case HCC_EXPR_TYPE_BINARY_OP(SUBTRACT):
		case HCC_EXPR_TYPE_BINARY_OP(MULTIPLY):
		case HCC_EXPR_TYPE_BINARY_OP(DIVIDE):
		case HCC_EXPR_TYPE_BINARY_OP(MODULO):
		case HCC_EXPR_TYPE_BINARY_OP(BIT_AND):
		case HCC_EXPR_TYPE_BINARY_OP(BIT_OR):
		case HCC_EXPR_TYPE_BINARY_OP(BIT_XOR):
		case HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT):
		case HCC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT):
		case HCC_EXPR_TYPE_BINARY_OP(EQUAL):
		case HCC_EXPR_TYPE_BINARY_OP(NOT_EQUAL):
		case HCC_EXPR_TYPE_BINARY_OP(LESS_THAN):
		case HCC_EXPR_TYPE_BINARY_OP(LESS_THAN_OR_EQUAL):
		case HCC_EXPR_TYPE_BINARY_OP(GREATER_THAN):
		case HCC_EXPR_TYPE_BINARY_OP(GREATER_THAN_OR_EQUAL):
		{
			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HccExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			HccIROpCode op_code = HCC_IR_OP_CODE_BINARY_OP_START + (expr->type - HCC_EXPR_TYPE_BINARY_OP_START);

			HccDataType data_type;
			if (expr->binary.is_assignment) {
				data_type = left_expr->data_type;
			} else {
				data_type = expr->data_type;
			}

			HccIROperand* operands = hcc_irgen_add_operands_many(c, 3);

			hcc_irgen_generate_instructions(c, left_expr);
			operands[1] = c->irgen.last_operand;

			hcc_irgen_generate_instructions(c, right_expr);
			operands[2] = c->irgen.last_operand;

			U16 return_value_idx = hcc_irgen_add_value(c, data_type);
			operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
			hcc_irgen_add_instruction(c, op_code, operands, 3);

			if (expr->binary.is_assignment) {
				HccIROperand dst_operand;
				if (left_expr->type == HCC_EXPR_TYPE_LOCAL_VARIABLE) {
					dst_operand = HCC_IR_OPERAND_LOCAL_VARIABLE_INIT(left_expr->variable.idx);
				} else if (left_expr->type == HCC_EXPR_TYPE_GLOBAL_VARIABLE) {
					dst_operand = HCC_IR_OPERAND_GLOBAL_VARIABLE_INIT(left_expr->variable.idx);
				} else {
					dst_operand = operands[1];
				}

				hcc_irgen_generate_store(c, dst_operand, operands[0]);
			}

			c->irgen.last_operand = operands[0];
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(LOGICAL_AND):
		case HCC_EXPR_TYPE_BINARY_OP(LOGICAL_OR):
		{
			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HccExpr* right_expr = expr - expr->binary.right_expr_rel_idx;

			HccIRBasicBlock* basic_block = hcc_irgen_current_basic_block(c);
			HccIROperand starting_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));

			hcc_irgen_generate_condition_expr(c, left_expr);
			HccIROperand cond_operand = c->irgen.last_operand;

			HccIROperand* selection_merge_operands = hcc_irgen_add_operands_many(c, 1);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_SELECTION_MERGE, selection_merge_operands, 1);

			HccIROperand* cond_branch_operands = hcc_irgen_add_operands_many(c, 3);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);
			cond_branch_operands[0] = cond_operand;
			U32 success_idx = expr->type != HCC_EXPR_TYPE_BINARY_OP(LOGICAL_AND);
			U32 converging_idx = expr->type == HCC_EXPR_TYPE_BINARY_OP(LOGICAL_AND);

			basic_block = hcc_irgen_add_basic_block(c);
			HccIROperand success_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));
			cond_branch_operands[1 + success_idx] = success_basic_block_operand;

			hcc_irgen_generate_condition_expr(c, right_expr);
			HccIROperand success_cond_operand = c->irgen.last_operand;

			HccIROperand* success_branch_operands = hcc_irgen_add_operands_many(c, 1);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, success_branch_operands, 1);

			basic_block = hcc_irgen_add_basic_block(c);
			HccIROperand converging_basic_block_operand = HCC_IR_OPERAND_BASIC_BLOCK_INIT(hcc_irgen_basic_block_idx(c, basic_block));
			selection_merge_operands[0] = converging_basic_block_operand;
			success_branch_operands[0] = converging_basic_block_operand;
			cond_branch_operands[1 + converging_idx] = converging_basic_block_operand;

			HccIRFunction* ir_function = c->irgen.ir_function;
			HccIROperand* phi_operands = hcc_irgen_add_operands_many(c, 5);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_PHI, phi_operands, 5);
			U16 return_value_idx = hcc_irgen_add_value(c, hcc_irgen_operand_data_type(c, ir_function, cond_operand));
			phi_operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
			phi_operands[1] = cond_operand;
			phi_operands[2] = starting_basic_block_operand;
			phi_operands[3] = success_cond_operand;
			phi_operands[4] = success_basic_block_operand;

			c->irgen.last_operand = phi_operands[0];
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(COMMA): {
			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HccExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			hcc_irgen_generate_condition_expr(c, left_expr);
			hcc_irgen_generate_condition_expr(c, right_expr);
			break;
		};
		case HCC_EXPR_TYPE_CALL: {
			HccExpr* function_expr = expr - expr->binary.left_expr_rel_idx;
			HccExpr* call_args_expr = expr - expr->binary.right_expr_rel_idx;
			HCC_DEBUG_ASSERT(function_expr->type == HCC_EXPR_TYPE_FUNCTION, "expected an function expression");
			HCC_DEBUG_ASSERT(call_args_expr->type == HCC_EXPR_TYPE_CALL_ARG_LIST, "expected call argument list expression");

			HccFunction* function = hcc_stack_get(c->astgen.functions, function_expr->function.idx);
			HccExpr* arg_expr = call_args_expr;
			U32 args_count = ((U8*)call_args_expr)[1];
			U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];

			HccIROperand* operands = hcc_irgen_add_operands_many(c, args_count + 2);
			U16 return_value_idx = hcc_irgen_add_value(c, function->return_data_type);
			operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
			operands[1] = HCC_IR_OPERAND_FUNCTION_INIT(function_expr->function.idx); // TODO function pointer support

			if (function_expr->function.idx >= HCC_FUNCTION_IDX_USER_START) {
				HccDataType* param_data_types = hcc_stack_push_many(c->irgen.function_call_param_data_types, args_count);
				HccVariable* variables = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx);

				HccIRFunction* ir_function = c->irgen.ir_function;
				ir_function->call_param_data_types_count += args_count;

				for (U32 i = 0; i < args_count; i += 1) {
					param_data_types[i] = hcc_typedef_resolve_and_strip_const(c, variables[i].data_type);
				}
			}

			for (U32 i = 0; i < args_count; i += 1) {
				arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
				hcc_irgen_generate_instructions(c, arg_expr);
				operands[i + 2] = c->irgen.last_operand;
			}

			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_FUNCTION_CALL, operands, args_count + 2);
			c->irgen.last_operand = operands[0];

			break;
		};
		case HCC_EXPR_TYPE_FIELD_ACCESS: {
			U32 function_idx = hcc_stack_count(c->irgen.functions) - 1;
			HccExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			if (
				c->irgen.function->shader_stage != HCC_FUNCTION_SHADER_STAGE_NONE &&
				left_expr->type == HCC_EXPR_TYPE_LOCAL_VARIABLE &&
				left_expr->variable.idx < c->irgen.function->params_count
			) {
#if HCC_DEBUG_ASSERTIONS
				if (c->irgen.do_not_load_variable) {
					HccString identifier_string = hcc_string_table_get(&c->string_table, c->irgen.function->identifier_string_id);
					HCC_ABORT(
						"internal error: the compiler should have stopped shader paramters from being assigned too in '%.*s'",
						(int)identifier_string.size, identifier_string.data
					);
				}
#endif // HCC_DEBUG_ASSERTIONS

				HccDataType data_type = hcc_typedef_resolve_and_strip_const(c, left_expr->data_type);
				switch (left_expr->variable.idx) {
					case 0: { // HccVertexInput, HccFragmentInput or HccComputeInput
						HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, data_type);
						U32 field_idx = expr->binary.right_expr_rel_idx;
						HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, compound_data_type->fields_start_idx + field_idx);
						hcc_irgen_add_shader_param_load(c, HCC_IR_OP_CODE_LOAD_SHADER_STAGE_INPUT, field->data_type, field_idx);
						break;
					};
					case 1: {
						switch (c->irgen.function->shader_stage) {
							case HCC_FUNCTION_SHADER_STAGE_FRAGMENT: {
								HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, data_type);
								U32 field_idx = expr->binary.right_expr_rel_idx;
								HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, compound_data_type->fields_start_idx + field_idx);

								HccIROperand* operands = hcc_irgen_add_operands_many(c, 2);
								U16 return_value_idx = hcc_irgen_add_value(c, field->data_type);
								operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
								operands[1] = HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24_INIT(field_idx);
								hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_LOAD_RASTERIZER_STATE, operands, 2);
								c->irgen.last_operand = operands[0];
								break;
							};
							default: HCC_UNREACHABLE();
						}
						break;
					};
					default: HCC_UNREACHABLE();
				}
				return;
			}

			// fallthrough
		};

		case HCC_EXPR_TYPE_ARRAY_SUBSCRIPT:
		{
			bool do_load = !c->irgen.do_not_load_variable;
			c->irgen.do_not_load_variable = false;

			hcc_irgen_generate_access_chain_instruction(c, expr, 0);
			if (hcc_stack_get_last(c->irgen.instructions)->op_code == HCC_IR_OP_CODE_ACCESS_CHAIN) {
				hcc_irgen_generate_access_chain_end(c, expr->data_type);
			}

			if (do_load) {
				hcc_irgen_generate_load(c, expr->data_type, c->irgen.last_operand);
			}
			break;
		};
		case HCC_EXPR_TYPE_CURLY_INITIALIZER: {
			HccExpr* variable_expr = &expr[expr->curly_initializer.first_expr_rel_idx];
			HCC_DEBUG_ASSERT(variable_expr->type == HCC_EXPR_TYPE_LOCAL_VARIABLE, "internal error: expected the first node of the compound literal to be the hidden variable expression that we can mutate");
			HccIROperand variable_operand = HCC_IR_OPERAND_LOCAL_VARIABLE_INIT(variable_expr->variable.idx);

			//
			// store a zeroed value in the hidden local variable where the compound data type gets constructed
			HccConstantId zeroed_constant_id = hcc_constant_table_deduplicate_zero(c, expr->data_type);
			hcc_irgen_generate_store(c, variable_operand, HCC_IR_OPERAND_CONSTANT_INIT(zeroed_constant_id.idx_plus_one));

			HccExpr* initializer_expr = variable_expr;
			while (1) {
				U32 expr_rel_idx = initializer_expr->next_expr_rel_idx;
				if (expr_rel_idx == 0) {
					break;
				}
				initializer_expr = &initializer_expr[expr_rel_idx];

				HCC_DEBUG_ASSERT(initializer_expr->type == HCC_EXPR_TYPE_DESIGNATED_INITIALIZER, "internal error: expected a designated initializer");

				c->irgen.last_operand = variable_operand;

				HccIROperand dst_operand;
				HccDataType data_type = expr->data_type;
				{
					HccAstGenDesignatorInitializer* di = &c->astgen.curly_initializer.designated_initializers[initializer_expr->alt_next_expr_rel_idx];
					U64* elmt_indices = hcc_stack_get(c->astgen.curly_initializer.designated_initializer_elmt_indices, di->elmt_indices_start_idx);

					HccIROperand* operands = hcc_irgen_generate_access_chain_start(c, di->elmt_indices_count);
					U32 operand_idx = 3;
					for (U32 elmt_indices_idx = 0; elmt_indices_idx < di->elmt_indices_count; elmt_indices_idx += 1) {
						data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
						U64 entry_idx = elmt_indices[elmt_indices_idx];

						if (HCC_DATA_TYPE_IS_UNION(data_type)) {
							hcc_irgen_shrink_last_operands_count(c, operand_idx);
							hcc_irgen_generate_access_chain_end(c, data_type);
							hcc_irgen_generate_bitcast_union_field(c, data_type, entry_idx, c->irgen.last_operand);
							if (elmt_indices_idx + 1 < di->elmt_indices_count) {
								operands = hcc_irgen_generate_access_chain_start(c, di->elmt_indices_count - elmt_indices_idx);
								operand_idx = 3;
							}
						} else {
							HccBasic TODO_int_64_support_plz = hcc_basic_from_uint(c, HCC_DATA_TYPE_UINT, entry_idx);
							HccConstantId entry_constant_id = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_UINT, &TODO_int_64_support_plz);
							operands[operand_idx] = HCC_IR_OPERAND_CONSTANT_INIT(entry_constant_id.idx_plus_one);
							operand_idx += 1;
						}

						if (HCC_DATA_TYPE_IS_ARRAY(data_type)) {
							HccArrayDataType* array_data_type = hcc_array_data_type_get(c, data_type);
							data_type = array_data_type->element_data_type;
						} else if (HCC_DATA_TYPE_IS_COMPOUND_TYPE(data_type)) {
							HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, data_type);
							HccCompoundField* field = &c->astgen.compound_fields[compound_data_type->fields_start_idx + entry_idx];
							data_type = field->data_type;
						}
					}

					if (hcc_stack_get_last(c->irgen.instructions)->op_code == HCC_IR_OP_CODE_ACCESS_CHAIN) {
						hcc_irgen_shrink_last_operands_count(c, operand_idx);
						hcc_irgen_generate_access_chain_end(c, data_type);
					}

					dst_operand = c->irgen.last_operand;
				}


				HccIROperand value_operand;
				if (initializer_expr->designated_initializer.value_expr_rel_idx) {
					HccExpr* value_expr = initializer_expr - initializer_expr->designated_initializer.value_expr_rel_idx;
					hcc_irgen_generate_instructions(c, value_expr);
					value_operand = c->irgen.last_operand;
				} else {
					HccConstantId zeroed_constant_id = hcc_constant_table_deduplicate_zero(c, data_type);
					value_operand = HCC_IR_OPERAND_CONSTANT_INIT(zeroed_constant_id.idx_plus_one);
				}

				hcc_irgen_generate_store(c, dst_operand, value_operand);
			}

			hcc_irgen_generate_load(c, expr->data_type, variable_operand);
			break;
		};
		case HCC_EXPR_TYPE_CAST: {
			HccExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			hcc_irgen_generate_instructions(c, unary_expr);

			if (hcc_data_type_is_same_underlaying_type(c, expr->data_type, unary_expr->data_type)) {
				break;
			}

			if (hcc_data_type_is_same_bitwidth_int(c, expr->data_type, unary_expr->data_type)) {
				hcc_irgen_generate_bitcast(c, expr->data_type, c->irgen.last_operand, false);
				break;
			}

			HccBasicTypeClass dst_type_class = hcc_basic_type_class(c, expr->data_type);
			if (dst_type_class == HCC_BASIC_TYPE_CLASS_BOOL) {
				hcc_irgen_generate_convert_to_bool(c, c->irgen.last_operand);
			} else {
				HccIROperand* operands = hcc_irgen_add_operands_many(c, 3);
				hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_CONVERT, operands, 3);

				U16 return_value_idx = hcc_irgen_add_value(c, expr->data_type);
				operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
				operands[1] = expr->data_type;
				operands[2] = c->irgen.last_operand;

				c->irgen.last_operand = operands[0];
			}
			break;
		};
		case HCC_EXPR_TYPE_CONSTANT: {
			c->irgen.last_operand = HCC_IR_OPERAND_CONSTANT_INIT(expr->constant.id);
			break;
		};
		case HCC_EXPR_TYPE_STMT_BREAK: {
			HccIROperand* operands = hcc_irgen_add_operands_many(c, 1);
			operands[0] = -1; // the operand is initialized later by the callee
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, operands, 1);

			HccIRFunction* ir_function = c->irgen.ir_function;
			if (c->irgen.branch_state.break_branch_linked_list_tail == (U32)-1) {
				c->irgen.branch_state.break_branch_linked_list_head = ir_function->operands_count - 1;
			} else {
				*hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.break_branch_linked_list_tail) = ir_function->operands_count - 1;
			}
			c->irgen.branch_state.break_branch_linked_list_tail = ir_function->operands_count - 1;

			if (expr->next_expr_rel_idx) {
				hcc_irgen_add_basic_block(c);
			}

			break;
		};
		case HCC_EXPR_TYPE_STMT_CONTINUE: {
			HccIROperand* operands = hcc_irgen_add_operands_many(c, 1);
			operands[0] = -1; // the operand is initialized later by the callee
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, operands, 1);

			HccIRFunction* ir_function = c->irgen.ir_function;
			if (c->irgen.branch_state.continue_branch_linked_list_tail == (U32)-1) {
				c->irgen.branch_state.continue_branch_linked_list_head = ir_function->operands_count - 1;
			} else {
				*hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + c->irgen.branch_state.continue_branch_linked_list_tail) = ir_function->operands_count - 1;
			}
			c->irgen.branch_state.continue_branch_linked_list_tail = ir_function->operands_count - 1;

			if (expr->next_expr_rel_idx) {
				hcc_irgen_add_basic_block(c);
			}

			break;
		};
		case HCC_EXPR_TYPE_STMT_CASE:
		case HCC_EXPR_TYPE_STMT_DEFAULT: {
			//
			// 'case' and 'default' will get found when hcc_irgen_generate_case_instructions
			// is processing the statements of a 'case' and 'default' block.
			// they will implicitly fallthrough to this next 'case' and 'default' block
			// so make the next operand reference the next basic block index that will get made.
			HccIROperand* operands = hcc_irgen_add_operands_many(c, 1);
			HccIRFunction* ir_function = c->irgen.ir_function;
			operands[0] = HCC_IR_OPERAND_BASIC_BLOCK_INIT(ir_function->basic_blocks_count);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_BRANCH, operands, 1);
			break;
		};
		case HCC_EXPR_TYPE_LOCAL_VARIABLE: {
			if (c->irgen.do_not_load_variable) {
				c->irgen.last_operand = HCC_IR_OPERAND_LOCAL_VARIABLE_INIT(expr->variable.idx);
				c->irgen.do_not_load_variable = false;
			} else {
				U32 function_idx = hcc_stack_count(c->irgen.functions) - 1;
				HccVariable* variable = hcc_stack_get(c->astgen.function_params_and_variables, c->irgen.function->params_start_idx + expr->variable.idx);
				if (
					c->irgen.function->shader_stage != HCC_FUNCTION_SHADER_STAGE_NONE &&
					expr->variable.idx < c->irgen.function->params_count
				) {
					HccIROpCode op_code;
					switch (expr->variable.idx) {
						case 0: op_code = HCC_IR_OP_CODE_LOAD_SHADER_STAGE_INPUT; break;
						case 1: {
							switch (c->irgen.function->shader_stage) {
								case HCC_FUNCTION_SHADER_STAGE_FRAGMENT:
									op_code = HCC_IR_OP_CODE_LOAD_RASTERIZER_STATE;
									break;
								default: HCC_UNREACHABLE();
							}

							break;
						};
						default: HCC_UNREACHABLE();
					}

					HccDataType data_type = hcc_typedef_resolve_and_strip_const(c, HCC_DATA_TYPE_STRIP_CONST(variable->data_type));
					HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(c, data_type);
					HccIROperand* operands = hcc_irgen_add_operands_many(c, compound_data_type->fields_count + 1);
					U16 return_value_idx = hcc_irgen_add_value(c, data_type);
					operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);

					for (U32 field_idx = 0; field_idx < compound_data_type->fields_count; field_idx += 1) {
						HccCompoundField* field = &c->astgen.compound_fields[compound_data_type->fields_start_idx + field_idx];
						hcc_irgen_add_shader_param_load(c, op_code, field->data_type, field_idx);
						operands[field_idx + 1] = c->irgen.last_operand;
					}

					hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_COMPOSITE_INIT, operands, compound_data_type->fields_count + 1);
					c->irgen.last_operand = operands[0];
				} else {
					hcc_irgen_generate_load(c, variable->data_type, HCC_IR_OPERAND_LOCAL_VARIABLE_INIT(expr->variable.idx));
				}
			}
			break;
		};
		case HCC_EXPR_TYPE_GLOBAL_VARIABLE: {
			if (c->irgen.do_not_load_variable) {
				c->irgen.last_operand = HCC_IR_OPERAND_GLOBAL_VARIABLE_INIT(expr->variable.idx);
				c->irgen.do_not_load_variable = false;
			} else {
				HccVariable* variable = &c->astgen.global_variables[expr->variable.idx];
				hcc_irgen_generate_load(c, variable->data_type, HCC_IR_OPERAND_GLOBAL_VARIABLE_INIT(expr->variable.idx));
			}
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(TERNARY): {
			HccExpr* cond_expr = expr - expr->ternary.cond_expr_rel_idx;
			hcc_irgen_generate_condition_expr(c, cond_expr);
			HccIROperand cond_operand = c->irgen.last_operand;

			HccExpr* true_expr = expr - expr->ternary.true_expr_rel_idx;
			hcc_irgen_generate_instructions(c, true_expr);
			HccIROperand true_operand = c->irgen.last_operand;

			HccExpr* false_expr = expr - expr->ternary.false_expr_rel_idx;
			hcc_irgen_generate_instructions(c, false_expr);
			HccIROperand false_operand = c->irgen.last_operand;

			HccIROperand* operands = hcc_irgen_add_operands_many(c, 4);
			hcc_irgen_add_instruction(c, HCC_IR_OP_CODE_SELECT, operands, 4);

			U16 return_value_idx = hcc_irgen_add_value(c, expr->data_type);
			operands[0] = HCC_IR_OPERAND_VALUE_INIT(return_value_idx);
			operands[1] = cond_operand;
			operands[2] = true_operand;
			operands[3] = false_operand;

			c->irgen.last_operand = operands[0];
			break;
		};
		default:
			HCC_ABORT("unhandled expr type %u\n", expr->type);
	}
}

void hcc_irgen_generate_function(HccCompiler* c, U32 function_idx) {
	HccFunction* function = hcc_stack_get(c->astgen.functions, function_idx);
	HccIRFunction* ir_function = hcc_stack_get(c->irgen.functions, function_idx);
	ir_function->basic_blocks_start_idx = hcc_stack_count(c->irgen.basic_blocks);
	ir_function->basic_blocks_count = 0;
	ir_function->instructions_start_idx = hcc_stack_count(c->irgen.instructions);
	ir_function->instructions_count = 0;
	ir_function->values_start_idx = hcc_stack_count(c->irgen.values);
	ir_function->values_count = 0;
	ir_function->operands_start_idx = hcc_stack_count(c->irgen.operands);
	ir_function->operands_count = 0;
	ir_function->call_param_data_types_start_idx = hcc_stack_count(c->irgen.function_call_param_data_types);
	ir_function->call_param_data_types_count = 0;

	HCC_DEBUG_ASSERT(function->block_expr_id.idx_plus_one, "expected to have a function body");

	c->irgen.function = function;
	c->irgen.ir_function = ir_function;
	HccExpr* expr = hcc_stack_get(c->astgen.exprs, function->block_expr_id.idx_plus_one - 1);
	hcc_irgen_add_basic_block(c);
	hcc_irgen_generate_instructions(c, expr);
}

void hcc_irgen_generate(HccCompiler* c) {
	hcc_stack_resize(c->irgen.functions, hcc_stack_count(c->astgen.functions));
	for (U32 used_function_idx = 0; used_function_idx < hcc_stack_count(c->astgen.used_function_indices); used_function_idx += 1) {
		U32 function_idx = c->astgen.used_function_indices[used_function_idx];
		hcc_irgen_generate_function(c, function_idx);
	}
}

void hcc_irgen_print_operand(HccCompiler* c, HccIROperand operand, FILE* f) {
	switch (operand & 0xff) {
		case HCC_IR_OPERAND_VALUE:
			fprintf(f, "v%u", HCC_IR_OPERAND_VALUE_IDX(operand));
			break;
		case HCC_IR_OPERAND_CONSTANT:
			fprintf(f, "c%u", HCC_IR_OPERAND_CONSTANT_ID(operand).idx_plus_one - 1);
			break;
		case HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24:
			fprintf(f, "immc(%u)", HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24(operand));
			break;
		case HCC_IR_OPERAND_BASIC_BLOCK:
			fprintf(f, "b%u", HCC_IR_OPERAND_BASIC_BLOCK_IDX(operand));
			break;
		case HCC_IR_OPERAND_LOCAL_VARIABLE:
			fprintf(f, "var%u", HCC_IR_OPERAND_VARIABLE_IDX(operand));
			break;
		case HCC_IR_OPERAND_GLOBAL_VARIABLE:
			fprintf(f, "global_var%u", HCC_IR_OPERAND_VARIABLE_IDX(operand));
			break;
		case HCC_IR_OPERAND_FUNCTION: {
			HccFunction* function = hcc_stack_get(c->astgen.functions, HCC_IR_OPERAND_FUNCTION_IDX(operand));
			HccString ident = hcc_string_table_get(&c->string_table, function->identifier_string_id);
			fprintf(f, "function%u(%.*s)", HCC_IR_OPERAND_FUNCTION_IDX(operand), (int)ident.size, ident.data);
			break;
		};
		default: {
			HccString data_type_name = hcc_data_type_string(c, operand);
			fprintf(f, "%.*s", (int)data_type_name.size, data_type_name.data);
			break;
		};
	}
}

void hcc_irgen_print(HccCompiler* c, FILE* f) {
	HccConstantTable* constant_table = &c->constant_table;
	for (U32 idx = 0; idx < constant_table->entries_count; idx += 1) {
		fprintf(f, "Constant(c%u): ", idx);
		HccConstantId constant_id = { .idx_plus_one = idx + 1 };
		hcc_constant_print(c, constant_id, stdout);
		fprintf(f, "\n");
	}

	for (U32 used_function_idx = 0; used_function_idx < hcc_stack_count(c->astgen.used_function_indices); used_function_idx += 1) {
		U32 function_idx = c->astgen.used_function_indices[used_function_idx];
		HccFunction* function = hcc_stack_get(c->astgen.functions, function_idx);
		HccIRFunction* ir_function = hcc_stack_get(c->irgen.functions, function_idx);
		char buf[1024];
		hcc_function_to_string(c, function, buf, sizeof(buf), false);
		fprintf(f, "Function(#%u): %s\n", function_idx, buf);
		for (U32 basic_block_idx = ir_function->basic_blocks_start_idx; basic_block_idx < ir_function->basic_blocks_start_idx + (U32)ir_function->basic_blocks_count; basic_block_idx += 1) {
			HccIRBasicBlock* basic_block = hcc_stack_get(c->irgen.basic_blocks, basic_block_idx);
			fprintf(f, "\tBASIC_BLOCK(#%u):\n", basic_block_idx - ir_function->basic_blocks_start_idx);
			for (U32 instruction_idx = basic_block->instructions_start_idx; instruction_idx < basic_block->instructions_start_idx + (U32)basic_block->instructions_count; instruction_idx += 1) {
				HccIRInstr* instruction = hcc_stack_get(c->irgen.instructions, ir_function->instructions_start_idx + instruction_idx);
				char* op_name;
				switch (instruction->op_code) {
					case HCC_IR_OP_CODE_LOAD:
					{
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_LOAD: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_STORE:
					{
						fprintf(f, "\t\tOP_STORE: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_LOAD_SHADER_STAGE_INPUT:
					{
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = HCC_IR_OP_CODE_LOAD_SHADER_STAGE_INPUT: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_LOAD_RASTERIZER_STATE:
					{
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = HCC_IR_OP_CODE_LOAD_RASTERIZER_STATE: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_COMPOSITE_INIT: {
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_COMPOSITE_INIT: ");
						for (U32 idx = 1; idx < instruction->operands_count; idx += 1) {
							hcc_irgen_print_operand(c, operands[idx], f);
							if (idx + 1 < instruction->operands_count) {
								fprintf(f, ", ");
							}
						}
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_ACCESS_CHAIN: {
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_ACCESS_CHAIN: ");
						for (U32 idx = 1; idx < instruction->operands_count; idx += 1) {
							hcc_irgen_print_operand(c, operands[idx], f);
							if (idx + 1 < instruction->operands_count) {
								fprintf(f, ", ");
							}
						}
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_FUNCTION_RETURN:
						fprintf(f, "\t\tOP_FUNCTION_RETURN: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, "\n");
						break;
					case HCC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT): op_name = "LOGICAL_NOT"; goto UNARY;
					case HCC_IR_OP_CODE_UNARY_OP(BIT_NOT): op_name = "BIT_NOT"; goto UNARY;
					case HCC_IR_OP_CODE_UNARY_OP(NEGATE): op_name = "NEGATE"; goto UNARY;
UNARY:				{
						fprintf(f, "\t\tOP_%s: ", op_name);
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_SELECTION_MERGE: {
						fprintf(f, "\t\tOP_SELECTION_MERGE: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_LOOP_MERGE: {
						fprintf(f, "\t\tOP_LOOP_MERGE: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_BINARY_OP(ADD): op_name = "OP_ADD"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(SUBTRACT): op_name = "OP_SUBTRACT"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(MULTIPLY): op_name = "OP_MULTIPLY"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(DIVIDE): op_name = "OP_DIVIDE"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(MODULO): op_name = "OP_MODULO"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(BIT_AND): op_name = "OP_BIT_AND"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(BIT_OR): op_name = "OP_BIT_OR"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(BIT_XOR): op_name = "OP_BIT_XOR"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_LEFT): op_name = "OP_BIT_SHIFT_LEFT"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_RIGHT): op_name = "OP_BIT_SHIFT_RIGHT"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(EQUAL): op_name = "OP_EQUAL"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(NOT_EQUAL): op_name = "OP_NOT_EQUAL"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(LESS_THAN): op_name = "OP_LESS_THAN"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(LESS_THAN_OR_EQUAL): op_name = "OP_LESS_THAN_OR_EQUAL"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(GREATER_THAN): op_name = "OP_GREATER_THAN"; goto BINARY_OP;
					case HCC_IR_OP_CODE_BINARY_OP(GREATER_THAN_OR_EQUAL): op_name = "OP_GREATER_THAN_OR_EQUAL"; goto BINARY_OP;
BINARY_OP:
					{
						fprintf(f, "\t\t%s: ", op_name);
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_BRANCH: {
						fprintf(f, "\t\tOP_BRANCH: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_BRANCH_CONDITIONAL: {
						fprintf(f, "\t\tOP_BRANCH_CONDITIONAL: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_PHI: {
						fprintf(f, "\t\tOP_PHI: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						for (U32 idx = 1; idx < instruction->operands_count; idx += 2) {
							fprintf(f, "(");
							hcc_irgen_print_operand(c, operands[idx + 1], f);
							fprintf(f, ": ");
							hcc_irgen_print_operand(c, operands[idx + 0], f);
							fprintf(f, ")");
						}
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_SWITCH: {
						fprintf(f, "\t\tOP_SWITCH: ");
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, ", ");
						for (U32 idx = 2; idx < instruction->operands_count; idx += 2) {
							fprintf(f, "(");
							hcc_irgen_print_operand(c, operands[idx + 0], f);
							fprintf(f, ": ");
							hcc_irgen_print_operand(c, operands[idx + 1], f);
							fprintf(f, ")");
							if (idx + 1 < instruction->operands_count) {
								fprintf(f, ", ");
							}
						}
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_CONVERT: {
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_CONVERT: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_BITCAST: {
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_BITCAST: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_BITCAST_PTR: {
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_BITCAST_PTR: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_UNREACHABLE: {
						fprintf(f, "\t\tOP_UNREACHABLE:\n");
						break;
					};
					case HCC_IR_OP_CODE_SELECT: {
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_SELECT: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[2], f);
						fprintf(f, ", ");
						hcc_irgen_print_operand(c, operands[3], f);
						fprintf(f, "\n");
						break;
					};
					case HCC_IR_OP_CODE_FUNCTION_CALL: {
						HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
						fprintf(f, "\t\t");
						hcc_irgen_print_operand(c, operands[0], f);
						fprintf(f, " = OP_FUNCTION_CALL: ");
						hcc_irgen_print_operand(c, operands[1], f);
						fprintf(f, "(");
						for (U32 idx = 2; idx < instruction->operands_count; idx += 1) {
							hcc_irgen_print_operand(c, operands[idx], f);
							if (idx + 1 < instruction->operands_count) {
								fprintf(f, ", ");
							}
						}
						fprintf(f, ")");
						fprintf(f, "\n");
						break;
					};
					default:
						HCC_ABORT("unhandled instruction '%u'", instruction->op_code);
				}
			}
		}
	}
}

// ===========================================
//
//
// SPIR-V Generation
//
//
// ===========================================

HccSpirvOp hcc_spirv_binary_ops[HCC_BINARY_OP_COUNT][HCC_BASIC_TYPE_CLASS_COUNT] = {
	[HCC_BINARY_OP_ADD] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_I_ADD,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_I_ADD,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_ADD,
	},
	[HCC_BINARY_OP_SUBTRACT] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_I_SUB,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_I_SUB,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_SUB,
	},
	[HCC_BINARY_OP_MULTIPLY] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_I_MUL,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_I_MUL,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_MUL,
	},
	[HCC_BINARY_OP_DIVIDE] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_U_DIV,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_S_DIV,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_DIV,
	},
	[HCC_BINARY_OP_MODULO] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_U_MOD,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_S_MOD,
	},
	[HCC_BINARY_OP_BIT_AND] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_BITWISE_AND,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_BITWISE_AND,
	},
	[HCC_BINARY_OP_BIT_OR] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_BITWISE_OR,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_BITWISE_OR,
	},
	[HCC_BINARY_OP_BIT_XOR] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_BITWISE_XOR,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_BITWISE_XOR,
	},
	[HCC_BINARY_OP_BIT_SHIFT_LEFT] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL,
	},
	[HCC_BINARY_OP_BIT_SHIFT_RIGHT] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_LOGICAL,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_ARITHMETIC,
	},
	[HCC_BINARY_OP_EQUAL] = {
		[HCC_BASIC_TYPE_CLASS_BOOL] = HCC_SPIRV_OP_LOGICAL_EQUAL,
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_I_EQUAL,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_I_EQUAL,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_UNORD_EQUAL,
	},
	[HCC_BINARY_OP_NOT_EQUAL] = {
		[HCC_BASIC_TYPE_CLASS_BOOL] = HCC_SPIRV_OP_LOGICAL_NOT_EQUAL,
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_I_NOT_EQUAL,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_I_NOT_EQUAL,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_UNORD_NOT_EQUAL,
	},
	[HCC_BINARY_OP_LESS_THAN] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_U_LESS_THAN,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_S_LESS_THAN,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_UNORD_LESS_THAN,
	},
	[HCC_BINARY_OP_LESS_THAN_OR_EQUAL] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_U_LESS_THAN_EQUAL,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_S_LESS_THAN_EQUAL,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_UNORD_LESS_THAN_EQUAL,
	},
	[HCC_BINARY_OP_GREATER_THAN] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_U_GREATER_THAN,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_S_GREATER_THAN,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_UNORD_GREATER_THAN,
	},
	[HCC_BINARY_OP_GREATER_THAN_OR_EQUAL] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_U_GREATER_THAN_EQUAL,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_S_GREATER_THAN_EQUAL,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_UNORD_GREATER_THAN_EQUAL,
	},
};

HccSpirvOp hcc_spirv_unary_ops[HCC_UNARY_OP_COUNT][HCC_BASIC_TYPE_CLASS_COUNT] = {
	[HCC_UNARY_OP_LOGICAL_NOT] = {
		[HCC_BASIC_TYPE_CLASS_BOOL] = HCC_SPIRV_OP_LOGICAL_NOT,
	},
	[HCC_UNARY_OP_BIT_NOT] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_BITWISE_NOT,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_BITWISE_NOT,
	},
	[HCC_UNARY_OP_NEGATE] = {
		[HCC_BASIC_TYPE_CLASS_UINT] = HCC_SPIRV_OP_S_NEGATE,
		[HCC_BASIC_TYPE_CLASS_SINT] = HCC_SPIRV_OP_S_NEGATE,
		[HCC_BASIC_TYPE_CLASS_FLOAT] = HCC_SPIRV_OP_F_NEGATE,
	},
};

U32 hcc_spirv_type_table_deduplicate_function(HccCompiler* c, HccFunction* function) {
	HCC_DEBUG_ASSERT(function->shader_stage == HCC_FUNCTION_SHADER_STAGE_NONE, "internal error: shader stage functions do not belong in the function type table");
	HccSpirvTypeTable* table = &c->spirvgen.type_table;

	//
	// TODO make this a hash table look for speeeds
	for (U32 i = 0; i < hcc_stack_count(table->entries); i += 1) {
		HccSpirvTypeEntry* entry = hcc_stack_get(table->entries, i);
		U32 function_data_types_count = function->params_count + 1;
		if (entry->kind != HCC_SPIRV_TYPE_KIND_FUNCTION) {
			continue;
		}
		if (entry->data_types_count != function_data_types_count) {
			continue;
		}

		HccDataType* data_types = hcc_stack_get(table->data_types, entry->data_types_start_idx);
		if (data_types[0] != hcc_typedef_resolve_and_strip_const(c, function->return_data_type)) {
			continue;
		}

		bool is_match = true;
		HccVariable* params = &c->astgen.function_params_and_variables[function->params_start_idx];
		for (U32 j = 0; j < function->params_count; j += 1) {
			if (data_types[j + 1] != hcc_typedef_resolve_and_strip_const(c, params[j].data_type)) {
				is_match = false;
				break;
			}
		}

		if (is_match) {
			return entry->spirv_id;
		}
	}

	HccSpirvTypeEntry* entry = hcc_stack_push(table->entries);
	entry->data_types_start_idx = hcc_stack_count(table->data_types);
	entry->data_types_count = function->params_count + 1;
	entry->spirv_id = c->spirvgen.next_id;
	entry->kind = HCC_SPIRV_TYPE_KIND_FUNCTION;

	HccDataType* data_types = hcc_stack_push_many(table->data_types, entry->data_types_count);
	data_types[0] = hcc_typedef_resolve_and_strip_const(c, function->return_data_type);
	HccVariable* params = &c->astgen.function_params_and_variables[function->params_start_idx];
	for (U32 j = 0; j < entry->data_types_count; j += 1) {
		data_types[j + 1] = hcc_typedef_resolve_and_strip_const(c, params[j].data_type);
	}

	return entry->spirv_id;
}

U32 hcc_spirv_type_table_deduplicate_variable(HccCompiler* c, HccDataType data_type, HccSpirvTypeKind kind) {
	HccSpirvTypeTable* table = &c->spirvgen.type_table;
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);

	//
	// TODO make this a hash table look for speeeds
	for (U32 i = 0; i < hcc_stack_count(table->entries); i += 1) {
		HccSpirvTypeEntry* entry = hcc_stack_get(table->entries, i);
		if (entry->kind != kind) {
			continue;
		}

		if (*hcc_stack_get(table->data_types, entry->data_types_start_idx) == data_type) {
			return entry->spirv_id;
		}
	}

	HccSpirvTypeEntry* entry = hcc_stack_push(table->entries);
	entry->data_types_start_idx = hcc_stack_count(table->data_types);
	entry->data_types_count = 1;
	entry->kind = kind;
	switch (kind) {
		case HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_INPUT:
			entry->spirv_id = hcc_spirvgen_pointer_type_input_id(c, data_type);
			break;
		case HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_OUTPUT:
			entry->spirv_id = hcc_spirvgen_pointer_type_output_id(c, data_type);
			break;
		default:
			entry->spirv_id = c->spirvgen.next_id;
			break;
	}

	*hcc_stack_push(table->data_types) = data_type;
	return c->spirvgen.next_id;
}

void hcc_spirvgen_init(HccCompiler* c, HccCompilerSetup* setup) {
	U32 total_data_types_count
		= HCC_DATA_TYPE_COUNT
		+ setup->astgen.array_data_types_cap
		+ setup->astgen.struct_data_types_cap
		+ setup->astgen.union_data_types_cap;

	c->spirvgen.type_table.data_types = hcc_stack_init(HccDataType, total_data_types_count, HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_DATA_TYPES);
	c->spirvgen.type_table.entries = hcc_stack_init(HccSpirvTypeEntry, total_data_types_count, HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_ENTRIES);

	// zero is an invalid spirv id so start on 1
	c->spirvgen.next_id = 1;

	c->spirvgen.available_basic_types = 0xffff;
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_U8);
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_S8);
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_U16);
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_S16);
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_F16);
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_U64);
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_S64);
	HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(&c->spirvgen.available_basic_types, HCC_INTRINSIC_TYPE_F64);

	c->spirvgen.out_capabilities = hcc_stack_init(U32, setup->spirvgen.out_capabilities_cap, HCC_ALLOC_TAG_SPIRVGEN_OUT_CAPABILITIES);
	c->spirvgen.out_entry_points = hcc_stack_init(U32, setup->spirvgen.out_entry_points_cap, HCC_ALLOC_TAG_SPIRVGEN_OUT_ENTRY_POINTS);
	c->spirvgen.out_debug_info = hcc_stack_init(U32, setup->spirvgen.out_debug_info_cap, HCC_ALLOC_TAG_SPIRVGEN_OUT_DEBUG_INFO);
	c->spirvgen.out_annotations = hcc_stack_init(U32, setup->spirvgen.out_annotations_cap, HCC_ALLOC_TAG_SPIRVGEN_OUT_ANNOTATIONS);
	c->spirvgen.out_types_variables_constants = hcc_stack_init(U32, setup->spirvgen.out_types_variables_constants_cap, HCC_ALLOC_TAG_SPIRVGEN_OUT_TYPES_VARIABLES_CONSTANTS);
	c->spirvgen.out_functions = hcc_stack_init(U32, setup->spirvgen.out_functions_cap, HCC_ALLOC_TAG_SPIRVGEN_OUT_FUNCTIONS);
}

U32 hcc_spirvgen_resolve_type_id(HccCompiler* c, HccDataType data_type) {
	data_type = hcc_typedef_resolve_and_strip_const(c, data_type);
	U32 spirv_id;
	if (HCC_DATA_TYPE_IS_INTRINSIC(data_type)) {
		HccIntrinsicType intrinsic_type = hcc_intrinsic_type_from_data_type(c, data_type);
		spirv_id = HCC_SPIRV_ID_FROM_INTRINSIC_TYPE(intrinsic_type);
	} else {
		switch (data_type & 0xff) {
			case HCC_DATA_TYPE_STRUCT:
			case HCC_DATA_TYPE_UNION:
				spirv_id = c->spirvgen.compound_type_base_id + HCC_DATA_TYPE_IDX(data_type);
				break;
			case HCC_DATA_TYPE_ARRAY:
				spirv_id = c->spirvgen.array_type_base_id + HCC_DATA_TYPE_IDX(data_type);
				break;
			default:
				HCC_ABORT("unhandled data type '%u'", data_type);
		}
	}

	return spirv_id;
}

U32 hcc_spirvgen_convert_operand(HccCompiler* c, HccIROperand ir_operand) {
	switch (ir_operand & 0xff) {
		case HCC_IR_OPERAND_VALUE: return c->spirvgen.value_base_id + HCC_IR_OPERAND_VALUE_IDX(ir_operand);
		case HCC_IR_OPERAND_CONSTANT: return c->spirvgen.constant_base_id + HCC_IR_OPERAND_CONSTANT_ID(ir_operand).idx_plus_one - 1;
		case HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24:
			HCC_UNREACHABLE("not sure if we need to convert the operand for the immediate constant");
		case HCC_IR_OPERAND_BASIC_BLOCK: return c->spirvgen.basic_block_base_spirv_id + HCC_IR_OPERAND_BASIC_BLOCK_IDX(ir_operand);
		case HCC_IR_OPERAND_LOCAL_VARIABLE: return c->spirvgen.local_variable_base_spirv_id + HCC_IR_OPERAND_VARIABLE_IDX(ir_operand);
		case HCC_IR_OPERAND_GLOBAL_VARIABLE: return c->spirvgen.global_variable_base_spirv_id + HCC_IR_OPERAND_VARIABLE_IDX(ir_operand);
		case HCC_IR_OPERAND_FUNCTION: return c->spirvgen.function_base_spirv_id + HCC_IR_OPERAND_FUNCTION_IDX(ir_operand) - HCC_FUNCTION_IDX_USER_START;
		default: return hcc_spirvgen_resolve_type_id(c, ir_operand);
	}
}

U32 hcc_spirvgen_pointer_type_input_id(HccCompiler* c, HccDataType data_type) {
	U32 data_type_spirv_id = hcc_spirvgen_resolve_type_id(c, data_type);
	HCC_DEBUG_ASSERT(HCC_SPIRV_ID_IS_INTRINSIC_TYPE(data_type_spirv_id), "spirv pointer type output can only be for basic, vector and matrix types");

	return c->spirvgen.pointer_type_inputs_base_id + data_type_spirv_id;
}

U32 hcc_spirvgen_pointer_type_output_id(HccCompiler* c, HccDataType data_type) {
	U32 data_type_spirv_id = hcc_spirvgen_resolve_type_id(c, data_type);
	HCC_DEBUG_ASSERT(HCC_SPIRV_ID_IS_INTRINSIC_TYPE(data_type_spirv_id), "spirv pointer type output can only be for basic, vector and matrix types");

	return c->spirvgen.pointer_type_outputs_base_id + data_type_spirv_id;
}

bool hcc_spirvgen_basic_type_is_supported(HccCompiler* c, HccIntrinsicType intrinsic_type) {
	return HCC_INTRINSIC_BASIC_TYPE_MASK_IS_SET(c->spirvgen.available_basic_types, intrinsic_type);
}

void hcc_spirvgen_instr_start(HccCompiler* c, HccSpirvOp op) {
	HCC_DEBUG_ASSERT(c->spirvgen.instr_op == HCC_SPIRV_OP_NO_OP, "internal error: hcc_spirvgen_instr_end has not be called before a new instruction was started");
	c->spirvgen.instr_op = op;
	c->spirvgen.instr_operands_count = 0;
}

void hcc_spirvgen_instr_add_operand(HccCompiler* c, U32 word) {
	HCC_DEBUG_ASSERT(c->spirvgen.instr_op != HCC_SPIRV_OP_NO_OP, "internal error: hcc_spirvgen_instr_start has not been called when making an instruction");
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(c->spirvgen.instr_operands_count, HCC_SPIRV_INSTR_OPERANDS_CAP);
	c->spirvgen.instr_operands[c->spirvgen.instr_operands_count] = word;
	c->spirvgen.instr_operands_count += 1;
}

void hcc_spirvgen_instr_add_converted_operand(HccCompiler* c, HccIROperand ir_operand) {
	U32 word = hcc_spirvgen_convert_operand(c, ir_operand);
	hcc_spirvgen_instr_add_operand(c, word);
}

void hcc_spirvgen_instr_add_result_operand(HccCompiler* c) {
	hcc_spirvgen_instr_add_operand(c, c->spirvgen.next_id);
	c->spirvgen.next_id += 1;
}

void hcc_spirvgen_instr_add_operands_string(HccCompiler* c, char* string, U32 string_size) {
	for (U32 i = 0; i < string_size; i += 4) {
		U32 word = 0;
		word |= string[i] << 0;
		if (i + 1 < string_size) word |= string[i + 1] << 8;
		if (i + 2 < string_size) word |= string[i + 2] << 16;
		if (i + 3 < string_size) word |= string[i + 3] << 24;
		hcc_spirvgen_instr_add_operand(c, word);
	}
	if (string_size % 4 == 0) {
		hcc_spirvgen_instr_add_operand(c, 0);
	}
}

void hcc_spirvgen_instr_end(HccCompiler* c) {
	HCC_DEBUG_ASSERT(c->spirvgen.instr_op != HCC_SPIRV_OP_NO_OP, "internal error: hcc_spirvgen_instr_start has not been called when making an instruction");

	U32* out;
	c->spirvgen.instr_operands_count += 1; // for the first operand that contains the op code and words count
	switch (c->spirvgen.instr_op) {
		case HCC_SPIRV_OP_CAPABILITY:
		case HCC_SPIRV_OP_EXTENSION:
			out = hcc_stack_push_many(c->spirvgen.out_capabilities, c->spirvgen.instr_operands_count);
			break;
		case HCC_SPIRV_OP_EXT_INST_IMPORT:
		case HCC_SPIRV_OP_MEMORY_MODEL:
		case HCC_SPIRV_OP_ENTRY_POINT:
		case HCC_SPIRV_OP_EXECUTION_MODE:
			out = hcc_stack_push_many(c->spirvgen.out_entry_points, c->spirvgen.instr_operands_count);
			break;
		case HCC_SPIRV_OP_DECORATE:
			out = hcc_stack_push_many(c->spirvgen.out_annotations, c->spirvgen.instr_operands_count);
			break;
		case HCC_SPIRV_OP_TYPE_VOID:
		case HCC_SPIRV_OP_TYPE_BOOL:
		case HCC_SPIRV_OP_TYPE_INT:
		case HCC_SPIRV_OP_TYPE_FLOAT:
		case HCC_SPIRV_OP_TYPE_VECTOR:
		case HCC_SPIRV_OP_TYPE_MATRIX:
		case HCC_SPIRV_OP_TYPE_ARRAY:
		case HCC_SPIRV_OP_TYPE_STRUCT:
		case HCC_SPIRV_OP_TYPE_POINTER:
		case HCC_SPIRV_OP_TYPE_FUNCTION:
		case HCC_SPIRV_OP_CONSTANT_TRUE:
		case HCC_SPIRV_OP_CONSTANT_FALSE:
		case HCC_SPIRV_OP_CONSTANT:
		case HCC_SPIRV_OP_CONSTANT_COMPOSITE:
		case HCC_SPIRV_OP_CONSTANT_NULL:
TYPES_VARIABLES_CONSTANTS:
			out = hcc_stack_push_many(c->spirvgen.out_types_variables_constants, c->spirvgen.instr_operands_count);
			break;
		case HCC_SPIRV_OP_FUNCTION:
		case HCC_SPIRV_OP_FUNCTION_PARAMETER:
		case HCC_SPIRV_OP_FUNCTION_END:
		case HCC_SPIRV_OP_FUNCTION_CALL:
		case HCC_SPIRV_OP_COMPOSITE_CONSTRUCT:
		case HCC_SPIRV_OP_ACCESS_CHAIN:
		case HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN:
		case HCC_SPIRV_OP_CONVERT_F_TO_U:
		case HCC_SPIRV_OP_CONVERT_F_TO_S:
		case HCC_SPIRV_OP_CONVERT_S_TO_F:
		case HCC_SPIRV_OP_CONVERT_U_TO_F:
		case HCC_SPIRV_OP_U_CONVERT:
		case HCC_SPIRV_OP_S_CONVERT:
		case HCC_SPIRV_OP_F_CONVERT:
		case HCC_SPIRV_OP_BITCAST:
		case HCC_SPIRV_OP_S_NEGATE:
		case HCC_SPIRV_OP_F_NEGATE:
		case HCC_SPIRV_OP_I_ADD:
		case HCC_SPIRV_OP_F_ADD:
		case HCC_SPIRV_OP_I_SUB:
		case HCC_SPIRV_OP_F_SUB:
		case HCC_SPIRV_OP_I_MUL:
		case HCC_SPIRV_OP_F_MUL:
		case HCC_SPIRV_OP_U_DIV:
		case HCC_SPIRV_OP_S_DIV:
		case HCC_SPIRV_OP_F_DIV:
		case HCC_SPIRV_OP_U_MOD:
		case HCC_SPIRV_OP_S_MOD:
		case HCC_SPIRV_OP_F_MOD:
		case HCC_SPIRV_OP_ISNAN:
		case HCC_SPIRV_OP_ISINF:
		case HCC_SPIRV_OP_LOGICAL_EQUAL:
		case HCC_SPIRV_OP_LOGICAL_NOT_EQUAL:
		case HCC_SPIRV_OP_LOGICAL_OR:
		case HCC_SPIRV_OP_LOGICAL_AND:
		case HCC_SPIRV_OP_LOGICAL_NOT:
		case HCC_SPIRV_OP_SELECT:
		case HCC_SPIRV_OP_I_EQUAL:
		case HCC_SPIRV_OP_I_NOT_EQUAL:
		case HCC_SPIRV_OP_U_GREATER_THAN:
		case HCC_SPIRV_OP_S_GREATER_THAN:
		case HCC_SPIRV_OP_U_GREATER_THAN_EQUAL:
		case HCC_SPIRV_OP_S_GREATER_THAN_EQUAL:
		case HCC_SPIRV_OP_U_LESS_THAN:
		case HCC_SPIRV_OP_S_LESS_THAN:
		case HCC_SPIRV_OP_U_LESS_THAN_EQUAL:
		case HCC_SPIRV_OP_S_LESS_THAN_EQUAL:
		case HCC_SPIRV_OP_F_UNORD_EQUAL:
		case HCC_SPIRV_OP_F_UNORD_NOT_EQUAL:
		case HCC_SPIRV_OP_F_UNORD_LESS_THAN:
		case HCC_SPIRV_OP_F_UNORD_GREATER_THAN:
		case HCC_SPIRV_OP_F_UNORD_LESS_THAN_EQUAL:
		case HCC_SPIRV_OP_F_UNORD_GREATER_THAN_EQUAL:
		case HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_LOGICAL:
		case HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_ARITHMETIC:
		case HCC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL:
		case HCC_SPIRV_OP_BITWISE_OR:
		case HCC_SPIRV_OP_BITWISE_XOR:
		case HCC_SPIRV_OP_BITWISE_AND:
		case HCC_SPIRV_OP_BITWISE_NOT:
		case HCC_SPIRV_OP_PHI:
		case HCC_SPIRV_OP_LOOP_MERGE:
		case HCC_SPIRV_OP_SELECTION_MERGE:
		case HCC_SPIRV_OP_LABEL:
		case HCC_SPIRV_OP_BRANCH:
		case HCC_SPIRV_OP_BRANCH_CONDITIONAL:
		case HCC_SPIRV_OP_SWITCH:
		case HCC_SPIRV_OP_RETURN:
		case HCC_SPIRV_OP_RETURN_VALUE:
		case HCC_SPIRV_OP_UNREACHABLE:
		case HCC_SPIRV_OP_LOAD:
		case HCC_SPIRV_OP_STORE:
		case HCC_SPIRV_OP_EXT_INST:
FUNCTIONS:
			out = hcc_stack_push_many(c->spirvgen.out_functions, c->spirvgen.instr_operands_count);
			break;
		case HCC_SPIRV_OP_VARIABLE: {
			U32 storage_class = c->spirvgen.instr_operands[2];
			if (storage_class == HCC_SPIRV_STORAGE_CLASS_FUNCTION) {
				goto FUNCTIONS;
			} else {
				goto TYPES_VARIABLES_CONSTANTS;
			}
			break;
		};
		default:
			HCC_ABORT("unhandled spirv instruction op");
	}

	out[0] = (((c->spirvgen.instr_operands_count) & 0xffff) << 16) | (c->spirvgen.instr_op & 0xffff);
	for (U32 i = 0; i < c->spirvgen.instr_operands_count - 1; i += 1) {
		out[i + 1] = c->spirvgen.instr_operands[i];
	}

	printf("INSTRUCTION(%u): ", c->spirvgen.instr_op);
	for (U32 i = 0; i < c->spirvgen.instr_operands_count; i += 1) {
		printf("%u, ", out[i]);
	}
	printf("\n");

	c->spirvgen.instr_op = HCC_SPIRV_OP_NO_OP;
}

U32 hcc_spirvgen_generate_variable_type(HccCompiler* c, HccDataType data_type, HccSpirvTypeKind type_kind) {
	data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);
	U32 type_id = hcc_spirv_type_table_deduplicate_variable(c, data_type, type_kind);
	if (type_id == c->spirvgen.next_id) {
		U32 storage_class;
		switch (type_kind) {
			case HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_INPUT:
				storage_class = HCC_SPIRV_STORAGE_CLASS_INPUT;
				type_id = hcc_spirvgen_pointer_type_input_id(c, data_type);
				break;
			case HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_OUTPUT:
				storage_class = HCC_SPIRV_STORAGE_CLASS_OUTPUT;
				type_id = hcc_spirvgen_pointer_type_output_id(c, data_type);
				break;
			case HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE:
				storage_class = HCC_SPIRV_STORAGE_CLASS_FUNCTION;
				c->spirvgen.next_id += 1;
				break;
			case HCC_SPIRV_TYPE_KIND_STATIC_VARIABLE:
				storage_class = HCC_SPIRV_STORAGE_CLASS_PRIVATE;
				c->spirvgen.next_id += 1;
				break;
		}
		hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_POINTER);
		hcc_spirvgen_instr_add_operand(c, type_id);
		hcc_spirvgen_instr_add_operand(c, storage_class);
		hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, data_type));
		hcc_spirvgen_instr_end(c);
	}

	return type_id;
}

U32 hcc_spirvgen_generate_function_type(HccCompiler* c, HccFunction* function) {
	if (function->shader_stage != HCC_FUNCTION_SHADER_STAGE_NONE) {
		static HccFunction void_function = {0};
		function = &void_function;
	}

	U32 function_type_id = hcc_spirv_type_table_deduplicate_function(c, function);

	if (function_type_id == c->spirvgen.next_id) {
		c->spirvgen.next_id += 1;

		//
		// prebuild any function variable types
		HccVariable* params = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx);
		for (U32 i = 0; i < function->params_count; i += 1) {
			hcc_spirvgen_generate_variable_type(c, params[i].data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
		}

		hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_FUNCTION);
		hcc_spirvgen_instr_add_operand(c, function_type_id);
		hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, function->return_data_type));
		for (U32 i = 0; i < function->params_count; i += 1) {
			U32 type_id = hcc_spirvgen_generate_variable_type(c, params[i].data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
			hcc_spirvgen_instr_add_operand(c, type_id);
		}
		hcc_spirvgen_instr_end(c);
	}

	return function_type_id;
}

void hcc_spirvgen_generate_select(HccCompiler* c, U32 result_spirv_operand, HccDataType dst_type, U32 cond_value_spirv_operand, U32 a_spirv_operand, U32 b_spirv_operand) {
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_SELECT);
	hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, dst_type));
	hcc_spirvgen_instr_add_operand(c, result_spirv_operand);
	hcc_spirvgen_instr_add_operand(c, cond_value_spirv_operand);
	hcc_spirvgen_instr_add_operand(c, a_spirv_operand);
	hcc_spirvgen_instr_add_operand(c, b_spirv_operand);
	hcc_spirvgen_instr_end(c);
}

void hcc_spirvgen_generate_convert(HccCompiler* c, HccSpirvOp spirv_convert_op, U32 result_spirv_operand, HccDataType dst_type, U32 value_spirv_operand) {
	hcc_spirvgen_instr_start(c, spirv_convert_op);
	hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, dst_type));
	hcc_spirvgen_instr_add_operand(c, result_spirv_operand);
	hcc_spirvgen_instr_add_operand(c, value_spirv_operand);
	hcc_spirvgen_instr_end(c);
}

void hcc_spirvgen_generate_entry_point_used_global_variable_spirv_ids(HccCompiler* c, HccFunction* function) {
	for (U32 idx = function->used_static_variables_start_idx; idx < function->used_static_variables_start_idx + function->used_static_variables_count; idx += 1) {
		HccDecl decl = c->astgen.function_used_static_variables[idx];
		U32 spirv_base_id = HCC_DECL_IS_LOCAL_VARIABLE(decl)
			? c->spirvgen.local_variable_base_spirv_id
			: c->spirvgen.global_variable_base_spirv_id;
		hcc_spirvgen_instr_add_operand(c, spirv_base_id + HCC_DECL_IDX(decl));
	}
}

void hcc_spirvgen_generate_instr(HccCompiler* c, HccSpirvOp op, HccDataType return_data_type, HccIROperand* operands, U32 operands_count) {
	hcc_spirvgen_instr_start(c, op);
	hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, return_data_type));
	hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
	for (U32 operand_idx = 2; operand_idx < operands_count; operand_idx += 1) {
		hcc_spirvgen_instr_add_converted_operand(c, operands[operand_idx]);
	}
	hcc_spirvgen_instr_end(c);
}

void hcc_spirvgen_generate_ext_instr(HccCompiler* c, HccSpirvGLSLSTD450Op op, HccDataType return_data_type, HccIROperand* operands, U32 operands_count) {
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_EXT_INST);
	hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, return_data_type));
	hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
	hcc_spirvgen_instr_add_operand(c, c->spirvgen.ext_inst_glsl_std_450_spirv_id);
	hcc_spirvgen_instr_add_operand(c, op);
	for (U32 operand_idx = 2; operand_idx < operands_count; operand_idx += 1) {
		hcc_spirvgen_instr_add_converted_operand(c, operands[operand_idx]);
	}
	hcc_spirvgen_instr_end(c);
}

void hcc_spirvgen_generate_intrinsic_function(HccCompiler* c, U32 function_idx, HccDataType return_data_type, HccIROperand* operands, U32 operands_count) {
	switch (function_idx) {
		//
		// libc-gpu/math.h
		//
		case HCC_FUNCTION_IDX_ISINF:
			hcc_spirvgen_generate_instr(c, HCC_SPIRV_OP_ISINF, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ISNAN:
			hcc_spirvgen_generate_instr(c, HCC_SPIRV_OP_ISNAN, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_FMODF:
		case HCC_FUNCTION_IDX_FMOD:
			hcc_spirvgen_generate_instr(c, HCC_SPIRV_OP_F_MOD, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_FABSF:
		case HCC_FUNCTION_IDX_FABS:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_FABS, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_FLOORF:
		case HCC_FUNCTION_IDX_FLOOR:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_FLOOR, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_CEILF:
		case HCC_FUNCTION_IDX_CEIL:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_CEIL, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ROUNDF:
		case HCC_FUNCTION_IDX_ROUND:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ROUND, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_TRUNCF:
		case HCC_FUNCTION_IDX_TRUNC:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_TRUNC, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_SINF:
		case HCC_FUNCTION_IDX_SIN:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_SIN, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_COSF:
		case HCC_FUNCTION_IDX_COS:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_COS, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_TANF:
		case HCC_FUNCTION_IDX_TAN:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_TAN, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ASINF:
		case HCC_FUNCTION_IDX_ASIN:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ASIN, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ACOSF:
		case HCC_FUNCTION_IDX_ACOS:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ACOS, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ATANF:
		case HCC_FUNCTION_IDX_ATAN:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ATAN, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_SINHF:
		case HCC_FUNCTION_IDX_SINH:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_SINH, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_COSHF:
		case HCC_FUNCTION_IDX_COSH:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_COSH, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_TANHF:
		case HCC_FUNCTION_IDX_TANH:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_TANH, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ASINHF:
		case HCC_FUNCTION_IDX_ASINH:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ASINH, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ACOSHF:
		case HCC_FUNCTION_IDX_ACOSH:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ACOSH, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ATANHF:
		case HCC_FUNCTION_IDX_ATANH:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ATANH, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_ATAN2F:
		case HCC_FUNCTION_IDX_ATAN2:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_ATAN2, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_FMAF:
		case HCC_FUNCTION_IDX_FMA:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_FMA, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_POWF:
		case HCC_FUNCTION_IDX_POW:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_POW, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_EXPF:
		case HCC_FUNCTION_IDX_EXP:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_EXP, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_LOGF:
		case HCC_FUNCTION_IDX_LOG:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_LOG, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_EXP2F:
		case HCC_FUNCTION_IDX_EXP2:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_EXP2, return_data_type, operands, operands_count);
			break;
		case HCC_FUNCTION_IDX_LOG2F:
		case HCC_FUNCTION_IDX_LOG2:
			hcc_spirvgen_generate_ext_instr(c, HCC_SPIRV_GLSL_STD_450_OP_LOG2, return_data_type, operands, operands_count);
			break;
		default:
			HCC_ABORT("internal error: unhandled intrinsic function idx '%u' in spirvgen", function_idx);
	}
}

void hcc_spirvgen_generate_function(HccCompiler* c, U32 function_idx) {
	HccFunction* function = hcc_stack_get(c->astgen.functions, function_idx);
	HccIRFunction* ir_function = hcc_stack_get(c->irgen.functions, function_idx);

	HccDataType return_data_type = function->return_data_type;
	switch (function->shader_stage) {
		case HCC_FUNCTION_SHADER_STAGE_VERTEX:
			return_data_type = HCC_DATA_TYPE_VOID;
			break;
		case HCC_FUNCTION_SHADER_STAGE_FRAGMENT:
			return_data_type = HCC_DATA_TYPE_VOID;
			break;
		case HCC_FUNCTION_SHADER_STAGE_NONE:
			break;
		default: HCC_ABORT("unhandled shader stage");
	}

	U32 function_type_id = hcc_spirvgen_generate_function_type(c, function);
	HccString ident = hcc_string_table_get(&c->string_table, function->identifier_string_id);

	U32 function_ctrl = HCC_SPIRV_FUNCTION_CTRL_NONE;
	if (function->flags & HCC_FUNCTION_FLAGS_INLINE) {
		function_ctrl |= HCC_SPIRV_FUNCTION_CTRL_INLINE;
	}

	U32 function_spirv_id = c->spirvgen.function_base_spirv_id + function_idx - HCC_FUNCTION_IDX_USER_START;
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_FUNCTION);
	hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, return_data_type));
	hcc_spirvgen_instr_add_operand(c, function_spirv_id);
	hcc_spirvgen_instr_add_operand(c, function_ctrl);
	hcc_spirvgen_instr_add_operand(c, function_type_id);
	hcc_spirvgen_instr_end(c);

	c->spirvgen.local_variable_base_spirv_id = c->spirvgen.next_id;
	c->spirvgen.next_id += function->params_count + function->variables_count;

	U32 position_spirv_id;
	U32 vertex_index_spirv_id;
	U32 instance_index_spirv_id;
	U32 frag_coord_spirv_id;
	HccCompoundDataType* rasterizer_state_compound_data_type;
	U32 rasterizer_state_variable_start_spirv_id;
	HccCompoundDataType* fragment_state_compound_data_type;
	U32 fragment_state_variable_start_spirv_id;
	switch (function->shader_stage) {
		case HCC_FUNCTION_SHADER_STAGE_VERTEX: {
			{
				vertex_index_spirv_id = c->spirvgen.next_id;
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_pointer_type_input_id(c, HCC_DATA_TYPE_INT32));
				hcc_spirvgen_instr_add_result_operand(c);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_INPUT);
				hcc_spirvgen_instr_end(c);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
				hcc_spirvgen_instr_add_operand(c, vertex_index_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_BUILTIN);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_BUILTIN_VERTEX_INDEX);
				hcc_spirvgen_instr_end(c);
			}

			{
				instance_index_spirv_id = c->spirvgen.next_id;
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_pointer_type_input_id(c, HCC_DATA_TYPE_INT32));
				hcc_spirvgen_instr_add_result_operand(c);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_INPUT);
				hcc_spirvgen_instr_end(c);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
				hcc_spirvgen_instr_add_operand(c, instance_index_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_BUILTIN);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_BUILTIN_INSTANCE_INDEX);
				hcc_spirvgen_instr_end(c);
			}

			{
				position_spirv_id = c->spirvgen.next_id;
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_pointer_type_output_id(c, HCC_DATA_TYPE_VECTOR(HCC_VEC4F)));
				hcc_spirvgen_instr_add_result_operand(c);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_OUTPUT);
				hcc_spirvgen_instr_end(c);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
				hcc_spirvgen_instr_add_operand(c, position_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_BUILTIN);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_BUILTIN_POSITION);
				hcc_spirvgen_instr_end(c);
			}

			rasterizer_state_compound_data_type = hcc_compound_data_type_get(c, function->return_data_type);
			rasterizer_state_variable_start_spirv_id = c->spirvgen.next_id;
			for (U32 field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, rasterizer_state_compound_data_type->fields_start_idx + field_idx);
				HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);

				U32 variable_spirv_id = c->spirvgen.next_id;
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_pointer_type_output_id(c, field_data_type));
				hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_OUTPUT);
				hcc_spirvgen_instr_end(c);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
				hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_LOCATION);
				hcc_spirvgen_instr_add_operand(c, field_idx);
				hcc_spirvgen_instr_end(c);

				if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP) {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
					hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
					hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_FLAT);
					hcc_spirvgen_instr_end(c);
				}

				c->spirvgen.next_id += 1;
			}

			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_ENTRY_POINT);
			hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_EXECUTION_MODEL_VERTEX);
			hcc_spirvgen_instr_add_operand(c, function_spirv_id);
			HccString name = hcc_string_table_get(&c->string_table, function->identifier_string_id);
			hcc_spirvgen_instr_add_operands_string(c, (char*)name.data, name.size);
			hcc_spirvgen_instr_add_operand(c, vertex_index_spirv_id);
			hcc_spirvgen_instr_add_operand(c, instance_index_spirv_id);
			hcc_spirvgen_instr_add_operand(c, position_spirv_id);
			for (U32 field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, rasterizer_state_compound_data_type->fields_start_idx + field_idx);
				hcc_spirvgen_instr_add_operand(c, rasterizer_state_variable_start_spirv_id + field_idx);
			}

			hcc_spirvgen_generate_entry_point_used_global_variable_spirv_ids(c, function);

			hcc_spirvgen_instr_end(c);
			break;
		};
		case HCC_FUNCTION_SHADER_STAGE_FRAGMENT:
			{
				frag_coord_spirv_id = c->spirvgen.next_id;
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_pointer_type_input_id(c, HCC_DATA_TYPE_VECTOR(HCC_VEC4F)));
				hcc_spirvgen_instr_add_result_operand(c);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_INPUT);
				hcc_spirvgen_instr_end(c);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
				hcc_spirvgen_instr_add_operand(c, frag_coord_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_BUILTIN);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_BUILTIN_FRAG_COORD);
				hcc_spirvgen_instr_end(c);
			}

			fragment_state_compound_data_type = hcc_compound_data_type_get(c, function->return_data_type);
			fragment_state_variable_start_spirv_id = c->spirvgen.next_id;
			for (U32 field_idx = 0; field_idx < fragment_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, fragment_state_compound_data_type->fields_start_idx + field_idx);
				HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);

				U32 variable_spirv_id = c->spirvgen.next_id;
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_pointer_type_output_id(c, field_data_type));
				hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_OUTPUT);
				hcc_spirvgen_instr_end(c);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
				hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_LOCATION);
				hcc_spirvgen_instr_add_operand(c, field_idx);
				hcc_spirvgen_instr_end(c);

				c->spirvgen.next_id += 1;
			}

			HccVariable* v = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + 1);
			rasterizer_state_compound_data_type = hcc_compound_data_type_get(c, hcc_typedef_resolve_and_strip_const(c, v->data_type));
			rasterizer_state_variable_start_spirv_id = c->spirvgen.next_id;
			for (U32 field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, rasterizer_state_compound_data_type->fields_start_idx + field_idx);
				HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);

				U32 variable_spirv_id = c->spirvgen.next_id;
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_pointer_type_input_id(c, field_data_type));
				hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_INPUT);
				hcc_spirvgen_instr_end(c);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
				hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
				hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_LOCATION);
				hcc_spirvgen_instr_add_operand(c, field_idx);
				hcc_spirvgen_instr_end(c);

				if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP) {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_DECORATE);
					hcc_spirvgen_instr_add_operand(c, variable_spirv_id);
					hcc_spirvgen_instr_add_operand(c, HCC_SPRIV_DECORATION_FLAT);
					hcc_spirvgen_instr_end(c);
				}

				c->spirvgen.next_id += 1;
			}

			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_ENTRY_POINT);
			hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_EXECUTION_MODEL_FRAGMENT);
			hcc_spirvgen_instr_add_operand(c, function_spirv_id);
			HccString name = hcc_string_table_get(&c->string_table, function->identifier_string_id);
			hcc_spirvgen_instr_add_operands_string(c, (char*)name.data, name.size);
			hcc_spirvgen_instr_add_operand(c, frag_coord_spirv_id);
			for (U32 field_idx = 0; field_idx < fragment_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, fragment_state_compound_data_type->fields_start_idx + field_idx);
				hcc_spirvgen_instr_add_operand(c, fragment_state_variable_start_spirv_id + field_idx);
			}
			for (U32 field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, rasterizer_state_compound_data_type->fields_start_idx + field_idx);
				hcc_spirvgen_instr_add_operand(c, rasterizer_state_variable_start_spirv_id + field_idx);
			}

			hcc_spirvgen_generate_entry_point_used_global_variable_spirv_ids(c, function);

			hcc_spirvgen_instr_end(c);

			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_EXECUTION_MODE);
			hcc_spirvgen_instr_add_operand(c, function_spirv_id);
			hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_EXECUTION_MODE_ORIGIN_UPPER_LEFT);
			hcc_spirvgen_instr_end(c);

			break;
		case HCC_FUNCTION_SHADER_STAGE_NONE:
			break;
		default: HCC_ABORT("unhandled shader stage");
	}

	c->spirvgen.basic_block_base_spirv_id = c->spirvgen.next_id;
	c->spirvgen.next_id += ir_function->basic_blocks_count;

	c->spirvgen.value_base_id = c->spirvgen.next_id;
	c->spirvgen.next_id += ir_function->values_count;

	U32 call_params_base_spirv_id = c->spirvgen.next_id;
	c->spirvgen.next_id += ir_function->call_param_data_types_count;

	//
	// generate the local variable types before we make the local variables and the global variables have a linear spirv id range
	{
		if (function->shader_stage == HCC_FUNCTION_SHADER_STAGE_NONE) {
			//
			// function parameters
			for (U32 variable_idx = 0; variable_idx < function->params_count; variable_idx += 1) {
				HccVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + variable_idx];
				hcc_spirvgen_generate_variable_type(c, variable->data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
			}
		}

		//
		// local variables
		for (U32 variable_idx = function->params_count; variable_idx < function->variables_count; variable_idx += 1) {
			HccVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + variable_idx];
			hcc_spirvgen_generate_variable_type(c, variable->data_type, variable->is_static ? HCC_SPIRV_TYPE_KIND_STATIC_VARIABLE : HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
		}

		//
		// local variables for every function call argument.
		HccDataType* function_call_param_data_types = hcc_stack_get_or_null(c->irgen.function_call_param_data_types, ir_function->call_param_data_types_start_idx);
		for (U32 variable_idx = 0; variable_idx < ir_function->call_param_data_types_count; variable_idx += 1) {
			hcc_spirvgen_generate_variable_type(c, function_call_param_data_types[variable_idx], HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
		}
	}

	for (U32 basic_block_idx = ir_function->basic_blocks_start_idx; basic_block_idx < ir_function->basic_blocks_start_idx + (U32)ir_function->basic_blocks_count; basic_block_idx += 1) {
		HccIRBasicBlock* basic_block = hcc_stack_get(c->irgen.basic_blocks, basic_block_idx);

		if (basic_block_idx == ir_function->basic_blocks_start_idx) {
			//
			// function params
			for (U32 variable_idx = 0; variable_idx < function->params_count; variable_idx += 1) {
				if (function->shader_stage == HCC_FUNCTION_SHADER_STAGE_NONE) {
					HccVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + variable_idx];
					U32 type_spirv_id = hcc_spirvgen_generate_variable_type(c, variable->data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_FUNCTION_PARAMETER);
					hcc_spirvgen_instr_add_operand(c, type_spirv_id);
					hcc_spirvgen_instr_add_operand(c, c->spirvgen.local_variable_base_spirv_id + variable_idx);
					hcc_spirvgen_instr_end(c);
				}
			}
		}

		hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_LABEL);
		hcc_spirvgen_instr_add_operand(c, c->spirvgen.basic_block_base_spirv_id + (basic_block_idx - ir_function->basic_blocks_start_idx));
		hcc_spirvgen_instr_end(c);

		if (basic_block_idx == ir_function->basic_blocks_start_idx) {
			//
			// local variables
			for (U32 variable_idx = function->params_count; variable_idx < function->variables_count; variable_idx += 1) {
				HccVariable* variable = hcc_stack_get(c->astgen.function_params_and_variables, function->params_start_idx + variable_idx);
				U32 type_spirv_id = hcc_spirvgen_generate_variable_type(c, variable->data_type, variable->is_static ? HCC_SPIRV_TYPE_KIND_STATIC_VARIABLE : HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, type_spirv_id);
				hcc_spirvgen_instr_add_operand(c, c->spirvgen.local_variable_base_spirv_id + variable_idx);
				if (variable->is_static) {
					hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_PRIVATE);
					hcc_spirvgen_instr_add_converted_operand(c, HCC_IR_OPERAND_CONSTANT_INIT(variable->initializer_constant_id.idx_plus_one));
				} else {
					hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_FUNCTION);
				}
				hcc_spirvgen_instr_end(c);
			}

			//
			// these are the local variables for every function call argument so that the value can be stored in here
			// before the call to the function. it has to be done this way so that the argument has the same data type
			// as the function param which is a 'pointer' to the actual data type.
			HccDataType* function_call_param_data_types = hcc_stack_get_or_null(c->irgen.function_call_param_data_types, ir_function->call_param_data_types_start_idx);
			for (U32 idx = 0; idx < ir_function->call_param_data_types_count; idx += 1) {
				U32 type_spirv_id = hcc_spirvgen_generate_variable_type(c, function_call_param_data_types[idx], HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
				hcc_spirvgen_instr_add_operand(c, type_spirv_id);
				hcc_spirvgen_instr_add_operand(c, call_params_base_spirv_id + idx);
				hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_FUNCTION);
				hcc_spirvgen_instr_end(c);
			}
		}

		for (U32 instruction_idx = basic_block->instructions_start_idx; instruction_idx < basic_block->instructions_start_idx + (U32)basic_block->instructions_count; instruction_idx += 1) {
			HccIRInstr* instruction = hcc_stack_get(c->irgen.instructions, ir_function->instructions_start_idx + instruction_idx);
			HccIROperand* operands = hcc_stack_get(c->irgen.operands, ir_function->operands_start_idx + (U32)instruction->operands_start_idx);
			switch (instruction->op_code) {
				case HCC_IR_OP_CODE_LOAD: {
					U32 type_spirv_id = hcc_spirvgen_resolve_type_id(c, hcc_irgen_operand_data_type(c, ir_function, operands[0]));
					U32 src_operand_spirv_id = hcc_spirvgen_convert_operand(c, operands[1]);
					hcc_spirvgen_generate_load(c, type_spirv_id, hcc_spirvgen_convert_operand(c, operands[0]), src_operand_spirv_id);
					break;
				};
				case HCC_IR_OP_CODE_STORE: {
					U32 dst_spirv_id = hcc_spirvgen_convert_operand(c, operands[0]);
					U32 src_spirv_id = hcc_spirvgen_convert_operand(c, operands[1]);
					hcc_spirvgen_generate_store(c, dst_spirv_id, src_spirv_id);
					break;
				};
				case HCC_IR_OP_CODE_LOAD_SHADER_STAGE_INPUT: {
					U32 src_operand_spirv_id = 0;
					HccDataType data_type = HCC_DATA_TYPE_VOID;
					switch (function->shader_stage) {
						case HCC_FUNCTION_SHADER_STAGE_VERTEX: {
							switch (HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24(operands[1])) {
								case HCC_VERTEX_INPUT_VERTEX_INDEX:
									data_type = HCC_DATA_TYPE_INT32;
									src_operand_spirv_id = vertex_index_spirv_id;
									break;
								case HCC_VERTEX_INPUT_INSTANCE_INDEX:
									data_type = HCC_DATA_TYPE_INT32;
									src_operand_spirv_id = instance_index_spirv_id;
									break;
							}

							break;
						};
						case HCC_FUNCTION_SHADER_STAGE_FRAGMENT: {
							switch (HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24(operands[1])) {
								case HCC_FRAGMENT_INPUT_FRAG_COORD:
									data_type = HCC_DATA_TYPE_VECTOR(HCC_VEC4F);
									src_operand_spirv_id = frag_coord_spirv_id;
									break;
							}

							break;
						};
					}

					U32 type_spirv_id = hcc_spirvgen_resolve_type_id(c, data_type);
					hcc_spirvgen_generate_load(c, type_spirv_id, hcc_spirvgen_convert_operand(c, operands[0]), src_operand_spirv_id);
					break;
				};
				case HCC_IR_OP_CODE_LOAD_RASTERIZER_STATE: {
					U32 field_idx = HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24(operands[1]);
					HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, rasterizer_state_compound_data_type->fields_start_idx + field_idx);
					HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);

					U32 dst_spirv_id = hcc_spirvgen_convert_operand(c, operands[0]);
					hcc_spirvgen_generate_load(c, hcc_spirvgen_resolve_type_id(c, field_data_type), dst_spirv_id, rasterizer_state_variable_start_spirv_id + field_idx);
					break;
				};
				case HCC_IR_OP_CODE_COMPOSITE_INIT: {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_COMPOSITE_CONSTRUCT);
					U32 return_value_idx = HCC_IR_OPERAND_VALUE_IDX(operands[0]);
					HccIRValue* return_value = hcc_stack_get(c->irgen.values, ir_function->values_start_idx + return_value_idx);

					hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, return_value->data_type));

					for (U32 i = 0; i < instruction->operands_count; i += 1) {
						hcc_spirvgen_instr_add_converted_operand(c, operands[i]);
					}

					U32 fields_count = hcc_data_type_composite_fields_count(c, return_value->data_type);
					for (U32 i = instruction->operands_count; i < fields_count + 1; i += 1) {
						hcc_spirvgen_instr_add_converted_operand(c, operands[instruction->operands_count - 1]);
					}

					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_ACCESS_CHAIN: {
					U32 data_type_spirv_id = hcc_spirvgen_generate_variable_type(c, operands[2], HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);

					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN);
					hcc_spirvgen_instr_add_operand(c, data_type_spirv_id);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);
					for (U32 i = 3; i < instruction->operands_count; i += 1) {
						hcc_spirvgen_instr_add_converted_operand(c, operands[i]);
					}
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_FUNCTION_RETURN: {
					switch (function->shader_stage) {
						case HCC_FUNCTION_SHADER_STAGE_VERTEX: {
							HccDataType data_type = hcc_irgen_operand_data_type(c, ir_function, operands[0]);
							data_type = hcc_typedef_resolve_and_strip_const(c, data_type);

							U32 var_spirv_id = hcc_spirvgen_convert_operand(c, operands[0]);
							for (U32 field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
								HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, rasterizer_state_compound_data_type->fields_start_idx + field_idx);
								HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);
								U32 field_spirv_id = hcc_spirvgen_generate_access_chain_single_field(c, var_spirv_id, field_data_type, field_idx);
								U32 src_spirv_id = c->spirvgen.next_id;
								c->spirvgen.next_id += 1;
								hcc_spirvgen_generate_load(c, hcc_spirvgen_resolve_type_id(c, field_data_type), src_spirv_id, field_spirv_id);
								if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_POSITION) {
									hcc_spirvgen_generate_store(c, position_spirv_id, src_spirv_id);
								}

								hcc_spirvgen_generate_store(c, rasterizer_state_variable_start_spirv_id + field_idx, src_spirv_id);
							}
							break;
						};
						case HCC_FUNCTION_SHADER_STAGE_FRAGMENT: {
							HccDataType data_type = hcc_irgen_operand_data_type(c, ir_function, operands[0]);
							data_type = hcc_typedef_resolve_and_strip_const(c, data_type);

							U32 var_spirv_id = hcc_spirvgen_convert_operand(c, operands[0]);
							for (U32 field_idx = 0; field_idx < fragment_state_compound_data_type->fields_count; field_idx += 1) {
								HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, fragment_state_compound_data_type->fields_start_idx + field_idx);
								HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);
								U32 field_spirv_id = hcc_spirvgen_generate_access_chain_single_field(c, var_spirv_id, field_data_type, field_idx);
								U32 src_spirv_id = c->spirvgen.next_id;
								c->spirvgen.next_id += 1;
								hcc_spirvgen_generate_load(c, hcc_spirvgen_resolve_type_id(c, field_data_type), src_spirv_id, field_spirv_id);
								hcc_spirvgen_generate_store(c, fragment_state_variable_start_spirv_id + field_idx, src_spirv_id);
							}
							break;
						};
					}

					if (return_data_type == HCC_DATA_TYPE_VOID) {
						hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_RETURN);
						hcc_spirvgen_instr_end(c);
					} else {
						hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_RETURN_VALUE);
						hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
						hcc_spirvgen_instr_end(c);
					}

					break;
				};
				case HCC_IR_OP_CODE_LOOP_MERGE: {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_LOOP_MERGE);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);
					hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_LOOP_CONTROL_NONE);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_SELECTION_MERGE: {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_SELECTION_MERGE);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_SELECTION_CONTROL_NONE);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_BINARY_OP(ADD):
				case HCC_IR_OP_CODE_BINARY_OP(SUBTRACT):
				case HCC_IR_OP_CODE_BINARY_OP(MULTIPLY):
				case HCC_IR_OP_CODE_BINARY_OP(DIVIDE):
				case HCC_IR_OP_CODE_BINARY_OP(MODULO):
				case HCC_IR_OP_CODE_BINARY_OP(BIT_AND):
				case HCC_IR_OP_CODE_BINARY_OP(BIT_OR):
				case HCC_IR_OP_CODE_BINARY_OP(BIT_XOR):
				case HCC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_LEFT):
				case HCC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_RIGHT):
				case HCC_IR_OP_CODE_BINARY_OP(EQUAL):
				case HCC_IR_OP_CODE_BINARY_OP(NOT_EQUAL):
				case HCC_IR_OP_CODE_BINARY_OP(LESS_THAN):
				case HCC_IR_OP_CODE_BINARY_OP(LESS_THAN_OR_EQUAL):
				case HCC_IR_OP_CODE_BINARY_OP(GREATER_THAN):
				case HCC_IR_OP_CODE_BINARY_OP(GREATER_THAN_OR_EQUAL):
				{
					U32 return_value_idx = HCC_IR_OPERAND_VALUE_IDX(operands[0]);
					HccIRValue* return_value = hcc_stack_get(c->irgen.values, ir_function->values_start_idx + return_value_idx);

					HccBinaryOp binary_op = instruction->op_code - HCC_IR_OP_CODE_BINARY_OP_START;
					HccDataType resolved_data_type = hcc_irgen_operand_data_type(c, ir_function, operands[1]);
					resolved_data_type = hcc_typedef_resolve_and_strip_const(c, resolved_data_type);
					HccBasicTypeClass type_class = hcc_basic_type_class(c, resolved_data_type);
					HccSpirvOp spirv_op = hcc_spirv_binary_ops[binary_op][type_class];
					HCC_DEBUG_ASSERT(spirv_op != HCC_SPIRV_OP_NO_OP, "internal error: invalid configuration for a binary op");

					hcc_spirvgen_instr_start(c, spirv_op);
					hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, return_value->data_type));
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[2]);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT):
				case HCC_IR_OP_CODE_UNARY_OP(BIT_NOT):
				case HCC_IR_OP_CODE_UNARY_OP(NEGATE):
				{
					U32 return_value_idx = HCC_IR_OPERAND_VALUE_IDX(operands[0]);
					HccIRValue* return_value = hcc_stack_get(c->irgen.values, ir_function->values_start_idx + return_value_idx);

					HccDataType data_type = hcc_irgen_operand_data_type(c, ir_function, operands[1]);
					HccBasicTypeClass type_class = hcc_basic_type_class(c, data_type);

					HccUnaryOp unary_op = instruction->op_code - HCC_IR_OP_CODE_UNARY_OP_START;
					HccSpirvOp spirv_op = hcc_spirv_unary_ops[unary_op][type_class];
					HCC_DEBUG_ASSERT(spirv_op != HCC_SPIRV_OP_NO_OP, "internal error: invalid configuration for a unary op");

					hcc_spirvgen_instr_start(c, spirv_op);
					hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, return_value->data_type));
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_BRANCH: {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_BRANCH);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_BRANCH_CONDITIONAL: {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_BRANCH_CONDITIONAL);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[2]);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_SWITCH: {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_SWITCH);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);
					for (U32 idx = 2; idx < instruction->operands_count; idx += 2) {
						HccConstant constant = hcc_constant_table_get(c, HCC_IR_OPERAND_CONSTANT_ID(operands[idx + 0]));

						switch (c->basic_type_size_and_aligns[constant.data_type]) {
							case sizeof(U8):
								hcc_spirvgen_instr_add_operand(c, *(U8*)constant.data);
								break;
							case sizeof(U16):
								hcc_spirvgen_instr_add_operand(c, *(U16*)constant.data);
								break;
							case sizeof(U32):
								hcc_spirvgen_instr_add_operand(c, *(U32*)constant.data);
								break;
							case sizeof(U64):
								hcc_spirvgen_instr_add_operand(c, ((U32*)constant.data)[0]);
								hcc_spirvgen_instr_add_operand(c, ((U32*)constant.data)[1]);
								break;
							default:
								HCC_UNREACHABLE("internal error: unhandled data type %u", constant.data_type);
						}

						hcc_spirvgen_instr_add_converted_operand(c, operands[idx + 1]);
					}
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_CONVERT: {
					HccDataType dst_type = operands[1];
					HccDataType src_type = hcc_irgen_operand_data_type(c, ir_function, operands[2]);
					dst_type = hcc_typedef_resolve_and_strip_const(c, dst_type);
					src_type = hcc_typedef_resolve_and_strip_const(c, src_type);
					HccBasicTypeClass dst_type_class = hcc_basic_type_class(c, dst_type);
					HccBasicTypeClass src_type_class = hcc_basic_type_class(c, src_type);

					U32 result_spirv_operand = hcc_spirvgen_convert_operand(c, operands[0]);
					U32 src_spirv_operand = hcc_spirvgen_convert_operand(c, operands[2]);
					switch (dst_type_class) {
						///////////////////////////////////////
						// case HCC_BASIC_TYPE_CLASS_BOOL:
						// ^^ this is handled in the HccIRGen, see hcc_irgen_generate_convert_to_bool
						///////////////////////////////////////

						case HCC_BASIC_TYPE_CLASS_UINT:
							switch (src_type_class) {
								case HCC_BASIC_TYPE_CLASS_BOOL: {
									U32 true_spirv_operand = c->spirvgen.constant_base_id + c->basic_type_one_constant_ids[dst_type].idx_plus_one - 1;
									U32 false_spirv_operand = c->spirvgen.constant_base_id + c->basic_type_zero_constant_ids[dst_type].idx_plus_one - 1;
									hcc_spirvgen_generate_select(c, result_spirv_operand, dst_type, src_spirv_operand, true_spirv_operand, false_spirv_operand);
									break;
								};
								case HCC_BASIC_TYPE_CLASS_UINT:
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_U_CONVERT, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								case HCC_BASIC_TYPE_CLASS_SINT: {
									HccDataType signed_dst_type = hcc_data_type_unsigned_to_signed(c, dst_type);
									if (signed_dst_type != src_type) {
										hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_S_CONVERT, c->spirvgen.next_id, signed_dst_type, src_spirv_operand);
										src_spirv_operand = c->spirvgen.next_id;
										c->spirvgen.next_id += 1;
									}
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_BITCAST, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HCC_BASIC_TYPE_CLASS_FLOAT:
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_CONVERT_F_TO_U, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								default:
									HCC_UNREACHABLE();
							}
							break;
						case HCC_BASIC_TYPE_CLASS_SINT:
							switch (src_type_class) {
								case HCC_BASIC_TYPE_CLASS_BOOL: {
									U32 true_spirv_operand = c->spirvgen.constant_base_id + c->basic_type_one_constant_ids[dst_type].idx_plus_one - 1;
									U32 false_spirv_operand = c->spirvgen.constant_base_id + c->basic_type_zero_constant_ids[dst_type].idx_plus_one - 1;
									hcc_spirvgen_generate_select(c, result_spirv_operand, dst_type, src_spirv_operand, true_spirv_operand, false_spirv_operand);
									break;
								};
								case HCC_BASIC_TYPE_CLASS_UINT: {
									HccDataType unsigned_dst_type = hcc_data_type_signed_to_unsigned(c, dst_type);
									if (unsigned_dst_type != src_type) {
										hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_U_CONVERT, c->spirvgen.next_id, unsigned_dst_type, src_spirv_operand);
										src_spirv_operand = c->spirvgen.next_id;
										c->spirvgen.next_id += 1;
									}
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_BITCAST, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HCC_BASIC_TYPE_CLASS_SINT: {
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_S_CONVERT, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HCC_BASIC_TYPE_CLASS_FLOAT:
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_CONVERT_F_TO_S, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								default:
									HCC_UNREACHABLE();
							}
							break;
						case HCC_BASIC_TYPE_CLASS_FLOAT:
							switch (src_type_class) {
								case HCC_BASIC_TYPE_CLASS_BOOL: {
									U32 true_spirv_operand = c->spirvgen.constant_base_id + c->basic_type_one_constant_ids[dst_type].idx_plus_one - 1;
									U32 false_spirv_operand = c->spirvgen.constant_base_id + c->basic_type_zero_constant_ids[dst_type].idx_plus_one - 1;
									hcc_spirvgen_generate_select(c, result_spirv_operand, dst_type, src_spirv_operand, true_spirv_operand, false_spirv_operand);
									break;
								};
								case HCC_BASIC_TYPE_CLASS_UINT:
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_CONVERT_U_TO_F, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								case HCC_BASIC_TYPE_CLASS_SINT: {
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_CONVERT_S_TO_F, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HCC_BASIC_TYPE_CLASS_FLOAT:
									hcc_spirvgen_generate_convert(c, HCC_SPIRV_OP_F_CONVERT, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								default:
									HCC_UNREACHABLE();
							}
							break;
						default:
							HCC_UNREACHABLE();
					}

					break;
				};
				case HCC_IR_OP_CODE_BITCAST: {
					U32 type_spirv_id = hcc_spirvgen_resolve_type_id(c, operands[1]);
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_BITCAST);
					hcc_spirvgen_instr_add_operand(c, type_spirv_id);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[2]);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_BITCAST_PTR: {
					U32 type_spirv_id = hcc_spirvgen_generate_variable_type(c, operands[1], HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_BITCAST);
					hcc_spirvgen_instr_add_operand(c, type_spirv_id);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[2]);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_PHI: {
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_PHI);
					HccDataType data_type = hcc_irgen_operand_data_type(c, ir_function, operands[1]);
					hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, data_type));
					for (U32 idx = 0; idx < instruction->operands_count; idx += 1) {
						hcc_spirvgen_instr_add_converted_operand(c, operands[idx]);
					}
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_UNREACHABLE:
					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_UNREACHABLE);
					hcc_spirvgen_instr_end(c);
					break;
				case HCC_IR_OP_CODE_SELECT: {
					U32 return_value_idx = HCC_IR_OPERAND_VALUE_IDX(operands[0]);
					HccIRValue* return_value = hcc_stack_get(c->irgen.values, ir_function->values_start_idx + return_value_idx);

					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_SELECT);
					hcc_spirvgen_instr_add_converted_operand(c, return_value->data_type);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[2]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[3]);
					hcc_spirvgen_instr_end(c);
					break;
				};
				case HCC_IR_OP_CODE_FUNCTION_CALL: {
					U32 return_value_idx = HCC_IR_OPERAND_VALUE_IDX(operands[0]);
					HccIRValue* return_value = hcc_stack_get(c->irgen.values, ir_function->values_start_idx + return_value_idx);

					U32 function_idx = HCC_IR_OPERAND_FUNCTION_IDX(operands[1]);
					if (function_idx < HCC_FUNCTION_IDX_USER_START) {
						hcc_spirvgen_generate_intrinsic_function(c, function_idx, return_value->data_type, operands, instruction->operands_count);
						break;
					}

					//
					// store the arguments inside the local variabes that were made at the
					// beginning of the function.
					for (U32 idx = 2; idx < instruction->operands_count; idx += 1) {
						U32 src_spirv_id = hcc_spirvgen_convert_operand(c, operands[idx]);
						hcc_spirvgen_generate_store(c, call_params_base_spirv_id + idx - 2, src_spirv_id);
					}

					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_FUNCTION_CALL);
					hcc_spirvgen_instr_add_converted_operand(c, return_value->data_type);
					hcc_spirvgen_instr_add_converted_operand(c, operands[0]);
					hcc_spirvgen_instr_add_converted_operand(c, operands[1]);

					for (U32 idx = 2; idx < instruction->operands_count; idx += 1) {
						hcc_spirvgen_instr_add_operand(c, call_params_base_spirv_id + idx - 2);
					}

					hcc_spirvgen_instr_end(c);

					call_params_base_spirv_id += instruction->operands_count - 2;
					break;
				};
				default:
					HCC_ABORT("unhandled instruction '%u'", instruction->op_code);
			}
		}
	}

	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_FUNCTION_END);
	hcc_spirvgen_instr_end(c);
}

void hcc_spirvgen_generate_intrinsic_types(HccCompiler* c) {
	//
	// HCC_INTRINSIC_TYPE_VOID
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_VOID);
	hcc_spirvgen_instr_add_result_operand(c);
	hcc_spirvgen_instr_end(c);

	//
	// HCC_INTRINSIC_TYPE_BOOL
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_BOOL);
	hcc_spirvgen_instr_add_result_operand(c);
	hcc_spirvgen_instr_end(c);

	//
	// HCC_INTRINSIC_TYPE_S8 - HCC_INTRINSIC_TYPE_S64
	for (U32 i = 3; i < 7; i += 1) {
		HccIntrinsicType intrinsic_type = HCC_INTRINSIC_TYPE_S8 + i - 3;
		if (!hcc_spirvgen_basic_type_is_supported(c, intrinsic_type)) {
			c->spirvgen.next_id += 1;
			continue;
		}

		hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_INT);
		hcc_spirvgen_instr_add_result_operand(c);
		hcc_spirvgen_instr_add_operand(c, 1 << i);
		hcc_spirvgen_instr_add_operand(c, 1);
		hcc_spirvgen_instr_end(c);
	}

	//
	// HCC_INTRINSIC_TYPE_U8 - HCC_INTRINSIC_TYPE_U64
	for (U32 i = 3; i < 7; i += 1) {
		HccDataType data_type = c->spirvgen.next_id - 1;
		HccIntrinsicType intrinsic_type = HCC_INTRINSIC_TYPE_U8 + i - 3;
		if (!hcc_spirvgen_basic_type_is_supported(c, intrinsic_type)) {
			c->spirvgen.next_id += 1;
			continue;
		}

		hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_INT);
		hcc_spirvgen_instr_add_result_operand(c);
		hcc_spirvgen_instr_add_operand(c, 1 << i);
		hcc_spirvgen_instr_add_operand(c, 0);
		hcc_spirvgen_instr_end(c);
	}

	//
	// HCC_INTRINSIC_TYPE_F16 - HCC_INTRINSIC_TYPE_F64
	for (U32 i = 4; i < 7; i += 1) {
		HccDataType data_type = c->spirvgen.next_id - 1;
		HccIntrinsicType intrinsic_type = HCC_INTRINSIC_TYPE_F16 + i - 4;
		if (!hcc_spirvgen_basic_type_is_supported(c, intrinsic_type)) {
			c->spirvgen.next_id += 1;
			continue;
		}

		hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_FLOAT);
		hcc_spirvgen_instr_add_result_operand(c);
		hcc_spirvgen_instr_add_operand(c, 1 << i);
		hcc_spirvgen_instr_end(c);
	}

	//
	// HCC_INTRINSIC_TYPE_VECTOR_START - HCC_INTRINSIC_TYPE_VECTOR_END
	for (U32 num_comps = 2; num_comps < 5; num_comps += 1) {
		for (HccIntrinsicType scalar_intrinsic_type = HCC_INTRINSIC_TYPE_BOOL; scalar_intrinsic_type < HCC_INTRINSIC_TYPE_VECTOR_START; scalar_intrinsic_type += 1) {
			if (!hcc_spirvgen_basic_type_is_supported(c, scalar_intrinsic_type)) {
				c->spirvgen.next_id += 1;
				continue;
			}

			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_VECTOR);
			hcc_spirvgen_instr_add_result_operand(c);
			hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_ID_FROM_INTRINSIC_TYPE(scalar_intrinsic_type));
			hcc_spirvgen_instr_add_operand(c, num_comps);
			hcc_spirvgen_instr_end(c);
		}
	}

	//
	// HCC_INTRINSIC_TYPE_MATRIX_START - HCC_INTRINSIC_TYPE_MATRIX_END
	for (U32 columns_count = 2; columns_count < 5; columns_count += 1) {
		for (U32 rows_count = 2; rows_count < 5; rows_count += 1) {
			for (HccIntrinsicType scalar_intrinsic_type = HCC_INTRINSIC_TYPE_F32; scalar_intrinsic_type <= HCC_INTRINSIC_TYPE_F64; scalar_intrinsic_type += 1) {
				if (!hcc_spirvgen_basic_type_is_supported(c, scalar_intrinsic_type)) {
					c->spirvgen.next_id += 1;
					continue;
				}

				HccVec column_vec = ((rows_count - 2) * 12) + scalar_intrinsic_type - 1;
				HccDataType column_vector_data_type = HCC_DATA_TYPE_VECTOR(column_vec);
				U32 column_vector_spirv_id = hcc_spirvgen_resolve_type_id(c, column_vector_data_type);

				hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_MATRIX);
				hcc_spirvgen_instr_add_result_operand(c);
				hcc_spirvgen_instr_add_operand(c, column_vector_spirv_id);
				hcc_spirvgen_instr_add_operand(c, columns_count);
				hcc_spirvgen_instr_end(c);
			}
		}
	}

	c->spirvgen.pointer_type_inputs_base_id = c->spirvgen.next_id;
	c->spirvgen.next_id += HCC_INTRINSIC_TYPE_COUNT;

	c->spirvgen.pointer_type_outputs_base_id = c->spirvgen.next_id;
	c->spirvgen.next_id += HCC_INTRINSIC_TYPE_COUNT;
}

void hcc_spirvgen_generate_basic_type_constants(HccCompiler* c) {
	HccConstantTable* constant_table = &c->constant_table;
	for (U32 idx = 0; idx < constant_table->entries_count; idx += 1) {
		HccConstantEntry* entry = &constant_table->entries[idx];
		if (!HCC_DATA_TYPE_IS_BASIC(entry->data_type)) {
			continue;
		}

		U32 data_type_spirv_id = hcc_spirvgen_resolve_type_id(c, entry->data_type);
		if (
			HCC_SPIRV_ID_IS_INTRINSIC_TYPE(data_type_spirv_id) &&
			!hcc_spirvgen_basic_type_is_supported(c, HCC_SPIRV_ID_TO_INTRINSIC_TYPE(data_type_spirv_id))
		) {
			continue;
		}

		if (entry->size == 0) {
			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_CONSTANT_NULL);
			hcc_spirvgen_instr_add_operand(c, data_type_spirv_id);
			hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + idx);
			hcc_spirvgen_instr_end(c);
		} else if (entry->data_type == HCC_DATA_TYPE_BOOL) {
			bool is_true = c->basic_type_one_constant_ids[HCC_DATA_TYPE_BOOL].idx_plus_one == idx + 1;
			hcc_spirvgen_instr_start(c, is_true ? HCC_SPIRV_OP_CONSTANT_TRUE : HCC_SPIRV_OP_CONSTANT_FALSE);
			hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, HCC_DATA_TYPE_BOOL));
			hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + idx);
			hcc_spirvgen_instr_end(c);
		} else if (HCC_DATA_TYPE_IS_BASIC(entry->data_type)) {
			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_CONSTANT);
			hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, entry->data_type));
			hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + idx);

			void* data = HCC_PTR_ADD(constant_table->data, entry->start_idx);
			switch (c->basic_type_size_and_aligns[entry->data_type]) {
				case sizeof(U64):
					// TODO: big endian might need a manual swap here?
					hcc_spirvgen_instr_add_operand(c, ((U32*)data)[0]);
					hcc_spirvgen_instr_add_operand(c, ((U32*)data)[1]);
					break;
				case sizeof(U32): hcc_spirvgen_instr_add_operand(c, *(U32*)data); break;
				case sizeof(U16): hcc_spirvgen_instr_add_operand(c, *(U16*)data); break;
				case sizeof(U8): hcc_spirvgen_instr_add_operand(c, *(U8*)data); break;
			}

			hcc_spirvgen_instr_end(c);
		}
	}
}

void hcc_spirvgen_generate_non_basic_type_constants(HccCompiler* c) {
	HccConstantTable* constant_table = &c->constant_table;
	for (U32 idx = 0; idx < constant_table->entries_count; idx += 1) {
		HccConstantEntry* entry = &constant_table->entries[idx];
		if (HCC_DATA_TYPE_IS_BASIC(entry->data_type)) {
			continue;
		}

		if (entry->size == 0) {
			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_CONSTANT_NULL);
			hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, entry->data_type));
			hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + idx);
			hcc_spirvgen_instr_end(c);
		} else {
			hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_CONSTANT_COMPOSITE);
			hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, entry->data_type));
			hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + idx);

			HccConstantId* constants = HCC_PTR_ADD(constant_table->data, entry->start_idx);
			for (U32 i = 0; i < entry->size / sizeof(HccConstantId); i += 1) {
				hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + (constants[i].idx_plus_one - 1));
			}

			hcc_spirvgen_instr_end(c);
		}
	}
}

void hcc_spirvgen_generate_load(HccCompiler* c, U32 type_spirv_id, U32 result_id, U32 src_spirv_id) {
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_LOAD);
	hcc_spirvgen_instr_add_operand(c, type_spirv_id);
	hcc_spirvgen_instr_add_operand(c, result_id);
	hcc_spirvgen_instr_add_operand(c, src_spirv_id);
	hcc_spirvgen_instr_end(c);
}

void hcc_spirvgen_generate_store(HccCompiler* c, U32 dst_spirv_id, U32 src_spirv_id) {
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_STORE);
	hcc_spirvgen_instr_add_operand(c, dst_spirv_id);
	hcc_spirvgen_instr_add_operand(c, src_spirv_id);
	hcc_spirvgen_instr_end(c);
}

U32 hcc_spirvgen_generate_access_chain_single_field(HccCompiler* c, U32 base_spirv_id, HccDataType field_data_type, U32 field_idx) {
	U32 data_type_spirv_id = hcc_spirvgen_generate_variable_type(c, field_data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE);

	U32 spirv_id = c->spirvgen.next_id;
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN);
	hcc_spirvgen_instr_add_operand(c, data_type_spirv_id);
	hcc_spirvgen_instr_add_result_operand(c);
	hcc_spirvgen_instr_add_operand(c, base_spirv_id);
	HccBasic basic = hcc_basic_from_uint(c, HCC_DATA_TYPE_UINT, field_idx);
	HccConstantId constant_id = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_UINT, &basic);
	hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + constant_id.idx_plus_one - 1);
	hcc_spirvgen_instr_end(c);
	return spirv_id;
}

void hcc_spirvgen_write_word_many(FILE* f, U32* words, U32 words_count, char* path) {
	U32 size = words_count * sizeof(U32);
	U32 written_size = fwrite(words, 1, size, f);
	HCC_ASSERT(size == written_size, "error writing file '%s'", path);
}

void hcc_spirvgen_write_word(FILE* f, U32 word, char* path) {
	U32 written_size = fwrite(&word, 1, sizeof(U32), f);
	HCC_ASSERT(sizeof(U32) == written_size, "error writing file '%s'", path);
}

void hcc_spirvgen_generate_binary(HccCompiler* c) {
	char* path = "test.spv";
	FILE* f = fopen(path, "wb");
	HCC_ASSERT(f, "error opening file for write '%s'");

	U32 magic_number = 0x07230203;
	hcc_spirvgen_write_word(f, magic_number, path);

	U32 major_version = 1;
	U32 minor_version = 5;
	U32 version = (major_version << 16) | (minor_version << 8);
	hcc_spirvgen_write_word(f, version, path);

	U32 generator_number = 0; // TODO: when we are feeling ballsy enough, register with the khronos folks and get a number for the lang.
	hcc_spirvgen_write_word(f, generator_number, path);

	hcc_spirvgen_write_word(f, c->spirvgen.next_id, path);

	U32 reserved_instruction_schema = 0;
	hcc_spirvgen_write_word(f, reserved_instruction_schema, path);

	hcc_spirvgen_write_word_many(f, c->spirvgen.out_capabilities, hcc_stack_count(c->spirvgen.out_capabilities), path);
	hcc_spirvgen_write_word_many(f, c->spirvgen.out_entry_points, hcc_stack_count(c->spirvgen.out_entry_points), path);
	hcc_spirvgen_write_word_many(f, c->spirvgen.out_debug_info, hcc_stack_count(c->spirvgen.out_debug_info), path);
	hcc_spirvgen_write_word_many(f, c->spirvgen.out_annotations, hcc_stack_count(c->spirvgen.out_annotations), path);
	hcc_spirvgen_write_word_many(f, c->spirvgen.out_types_variables_constants, hcc_stack_count(c->spirvgen.out_types_variables_constants), path);
	hcc_spirvgen_write_word_many(f, c->spirvgen.out_functions, hcc_stack_count(c->spirvgen.out_functions), path);

	fclose(f);
}

void hcc_spirvgen_generate_intrinsic_input_variable(HccCompiler* c, U32 intrinsic_idx) {
	HccCompoundDataType* d = hcc_stack_get(c->astgen.compound_data_types, intrinsic_idx);
	for (U32 field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
		HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, d->fields_start_idx + field_idx);
		HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);
		hcc_spirvgen_generate_variable_type(c, field_data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_INPUT);
	}
}

void hcc_spirvgen_generate(HccCompiler* c) {
	hcc_spirvgen_generate_intrinsic_types(c);

	//
	// make sure we have constants for rasterizer state field indices
	// when we make access chains to them.
	for (U32 field_idx = 0; field_idx < 16; field_idx += 1) {
		HccBasic basic = hcc_basic_from_uint(c, HCC_DATA_TYPE_UINT, field_idx);
		hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_UINT, (HccBasic*)&field_idx);
	}

	// generates the basic type constant before we make the array types (that use the constants)
	c->spirvgen.constant_base_id = c->spirvgen.next_id;
	c->spirvgen.next_id += c->constant_table.entries_count;
	hcc_spirvgen_generate_basic_type_constants(c);

	{
		c->spirvgen.array_type_base_id = c->spirvgen.next_id;
		c->spirvgen.next_id += hcc_stack_count(c->astgen.array_data_types);

		c->spirvgen.compound_type_base_id = c->spirvgen.next_id;
		c->spirvgen.next_id += hcc_stack_count(c->astgen.compound_data_types);

		for (U32 i = 0; i < hcc_stack_count(c->astgen.ordered_data_types); i += 1) {
			HccDataType data_type = c->astgen.ordered_data_types[i];
			if (data_type == HCC_DATA_TYPE_HALF && !hcc_spirvgen_basic_type_is_supported(c, HCC_INTRINSIC_TYPE_U16)) {
				// skip emitting the userland structure when U16 support is not available
				continue;
			}

			if (hcc_data_type_has_intrinsic_basic_types(c, data_type) & ~c->spirvgen.available_basic_types) {
				// skip emitting the data types when they contain intrinsic basic data types that are not supported
				continue;
			}

			switch (data_type & 0xff) {
				case HCC_DATA_TYPE_STRUCT:
				case HCC_DATA_TYPE_UNION: {
					HccCompoundDataType* d = hcc_compound_data_type_get(c, data_type);

					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_STRUCT);
					hcc_spirvgen_instr_add_operand(c, c->spirvgen.compound_type_base_id + HCC_DATA_TYPE_IDX(data_type));
					if (HCC_DATA_TYPE_IS_UNION(data_type)) {
						HccCompoundField* largest_sized_field = hcc_stack_get(c->astgen.compound_fields, d->fields_start_idx + d->largest_sized_field_idx);
						hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, largest_sized_field->data_type));
					} else {
						for (U32 field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
							HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, d->fields_start_idx + field_idx);
							hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, field->data_type));
						}
					}

					hcc_spirvgen_instr_end(c);

					if (d->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_RASTERIZER_STATE) {
						for (U32 field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
							HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, d->fields_start_idx + field_idx);
							HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);
							hcc_spirvgen_generate_variable_type(c, field_data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_INPUT);
							hcc_spirvgen_generate_variable_type(c, field_data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_OUTPUT);
						}
					} else if (d->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_FRAGMENT_STATE) {
						for (U32 field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
							HccCompoundField* field = hcc_stack_get(c->astgen.compound_fields, d->fields_start_idx + field_idx);
							HccDataType field_data_type = hcc_typedef_resolve_and_strip_const(c, field->data_type);
							hcc_spirvgen_generate_variable_type(c, field_data_type, HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_OUTPUT);
						}
					}
					break;
				};
				case HCC_DATA_TYPE_ARRAY: {
					HccArrayDataType* d = hcc_array_data_type_get(c, data_type);

					hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_TYPE_ARRAY);
					hcc_spirvgen_instr_add_operand(c, c->spirvgen.array_type_base_id + HCC_DATA_TYPE_IDX(data_type));
					hcc_spirvgen_instr_add_operand(c, hcc_spirvgen_resolve_type_id(c, d->element_data_type));
					hcc_spirvgen_instr_add_operand(c, c->spirvgen.constant_base_id + d->size_constant_id.idx_plus_one - 1);
					hcc_spirvgen_instr_end(c);
					break;
				};
			}
		}
	}

	hcc_spirvgen_generate_intrinsic_input_variable(c, HCC_STRUCT_IDX_VERTEX_INPUT);
	hcc_spirvgen_generate_intrinsic_input_variable(c, HCC_STRUCT_IDX_FRAGMENT_INPUT);

	hcc_spirvgen_generate_non_basic_type_constants(c);

	c->spirvgen.ext_inst_glsl_std_450_spirv_id = c->spirvgen.next_id;
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_EXT_INST_IMPORT);
	hcc_spirvgen_instr_add_result_operand(c);
	hcc_spirvgen_instr_add_operands_string_lit(c, "GLSL.std.450");
	hcc_spirvgen_instr_end(c);

	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_MEMORY_MODEL);
	hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_ADDRESS_MODEL_PHYSICAL_STORAGE_BUFFER_64);
	hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_MEMORY_MODEL_VULKAN);
	hcc_spirvgen_instr_end(c);

	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_CAPABILITY);
	hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_CAPABILITY_SHADER);
	hcc_spirvgen_instr_end(c);

	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_CAPABILITY);
	hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_CAPABILITY_VULKAN_MEMORY_MODEL);
	hcc_spirvgen_instr_end(c);

	// TODO: i don't know if we can support unions if we don't use this.
	// this allows us to bitcast a pointer, so we can bitcast a union to it's field type.
	// this is the SPIRV side to VK_KHR_buffer_device_address.
	hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_CAPABILITY);
	hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_CAPABILITY_PHYSICAL_STORAGE_BUFFER);
	hcc_spirvgen_instr_end(c);

	//
	// generate the global variable types before we make the global variables as the global variables have a linear spirv id range
	for (U32 global_variable_idx = 0; global_variable_idx < hcc_stack_count(c->astgen.global_variables); global_variable_idx += 1) {
		HccVariable* variable = hcc_stack_get(c->astgen.global_variables, global_variable_idx);
		hcc_spirvgen_generate_variable_type(c, variable->data_type, HCC_SPIRV_TYPE_KIND_STATIC_VARIABLE);
	}

	c->spirvgen.global_variable_base_spirv_id = c->spirvgen.next_id;
	c->spirvgen.next_id += hcc_stack_count(c->astgen.global_variables);
	for (U32 global_variable_idx = 0; global_variable_idx < hcc_stack_count(c->astgen.global_variables); global_variable_idx += 1) {
		HccVariable* variable = hcc_stack_get(c->astgen.global_variables, global_variable_idx);
		U32 type_spirv_id = hcc_spirvgen_generate_variable_type(c, variable->data_type, HCC_SPIRV_TYPE_KIND_STATIC_VARIABLE);
		hcc_spirvgen_instr_start(c, HCC_SPIRV_OP_VARIABLE);
		hcc_spirvgen_instr_add_operand(c, type_spirv_id);
		hcc_spirvgen_instr_add_operand(c, c->spirvgen.global_variable_base_spirv_id + global_variable_idx);
		hcc_spirvgen_instr_add_operand(c, HCC_SPIRV_STORAGE_CLASS_PRIVATE);
		hcc_spirvgen_instr_add_converted_operand(c, HCC_IR_OPERAND_CONSTANT_INIT(variable->initializer_constant_id.idx_plus_one));
		hcc_spirvgen_instr_end(c);
	}

	c->spirvgen.function_base_spirv_id = c->spirvgen.next_id;
	c->spirvgen.next_id += hcc_stack_count(c->astgen.functions) - HCC_FUNCTION_IDX_USER_START;
	for (U32 used_function_idx = 0; used_function_idx < hcc_stack_count(c->astgen.used_function_indices); used_function_idx += 1) {
		U32 function_idx = c->astgen.used_function_indices[used_function_idx];
		hcc_spirvgen_generate_function(c, function_idx);
	}

	hcc_spirvgen_generate_binary(c);
}

// ===========================================
//
//
// Constant Table
//
//
// ===========================================

void hcc_constant_table_init(HccCompiler* c, uint32_t data_cap, uint32_t entries_cap) {
	c->constant_table.data = HCC_ALLOC_ARRAY(char, HCC_ALLOC_TAG_CONSTANT_TABLE_DATA, data_cap);
	if (c->constant_table.data == NULL) {
		hcc_compiler_bail_allocation_failure(_hcc_compiler_ptr, HCC_ALLOC_TAG_CONSTANT_TABLE_DATA);
	}

	c->constant_table.entries = HCC_ALLOC_ARRAY(HccConstantEntry, HCC_ALLOC_TAG_CONSTANT_TABLE_ENTRIES, entries_cap);
	if (c->constant_table.entries == NULL) {
		hcc_compiler_bail_allocation_failure(_hcc_compiler_ptr, HCC_ALLOC_TAG_CONSTANT_TABLE_ENTRIES);
	}

	HCC_ASSERT(c->constant_table.entries, "out of memory");
	c->constant_table.data_cap = data_cap;
	c->constant_table.entries_cap = entries_cap;
}

HccConstantId hcc_constant_table_deduplicate_basic(HccCompiler* c, HccDataType data_type, HccBasic* basic) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a basic type but got '%s'", hcc_data_type_string(c, data_type));
	HCC_DEBUG_ASSERT(c->constant_table.fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");

	U64 size;
	U64 align;
	hcc_data_type_size_align(c, data_type, &size, &align);

	c->constant_table.data_write_ptr = NULL;
	HccStringId debug_string_id = {0};
	return _hcc_constant_table_deduplicate_end(c, data_type, basic, size, align, debug_string_id);
}

void hcc_constant_table_deduplicate_composite_start(HccCompiler* c, HccDataType data_type) {
	HCC_DEBUG_ASSERT(!HCC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a non basic type but got '%s'", hcc_data_type_string(c, data_type));
	HCC_DEBUG_ASSERT(c->constant_table.fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");

	U32 fields_count;
	fields_count = hcc_data_type_composite_fields_count(c, data_type);

	c->constant_table.data_type = data_type;
	c->constant_table.fields_count = 0;
	c->constant_table.fields_cap = fields_count;
	c->constant_table.data_write_ptr = HCC_PTR_ROUND_UP_ALIGN(HCC_PTR_ADD(c->constant_table.data, c->constant_table.data_used_size), alignof(HccConstantId));
}

void hcc_constant_table_deduplicate_composite_add(HccCompiler* c, HccConstantId constant_id) {
	HCC_DEBUG_ASSERT(c->constant_table.fields_cap, "internal error: cannot add data when deduplication of constant has not started");
	HCC_DEBUG_ASSERT(c->constant_table.fields_count < c->constant_table.fields_cap, "internal error: the expected constant with '%u' fields has been exceeded", c->constant_table.fields_cap);

	c->constant_table.data_write_ptr[c->constant_table.fields_count] = constant_id;
	c->constant_table.fields_count += 1;
}

HccConstantId hcc_constant_table_deduplicate_composite_end(HccCompiler* c) {
	HCC_DEBUG_ASSERT(c->constant_table.fields_count == c->constant_table.fields_cap, "internal error: the composite constant for deduplication is incomplete, expected to be '%u' fields but got '%u'", c->constant_table.fields_count, c->constant_table.fields_cap);
	c->constant_table.fields_cap = 0;

	HccStringId debug_string_id = {0};
	return _hcc_constant_table_deduplicate_end(c, c->constant_table.data_type, c->constant_table.data_write_ptr, c->constant_table.fields_count * sizeof(HccConstantId), alignof(HccConstantId), debug_string_id);
}

HccConstantId hcc_constant_table_deduplicate_zero(HccCompiler* c, HccDataType data_type) {
	if (HCC_DATA_TYPE_IS_BASIC(data_type)) {
		//
		// basic type's need to store their zero data into the consant table. this is so that
		// when the spirv code is generated it will generate OpConstant instructions for the consants instead of OpConstantNull.
		// this will allow them to be used as indices in OpAccessChain.
		HccBasic zero = {0};
		return hcc_constant_table_deduplicate_basic(c, data_type, &zero);
	} else {
		HCC_DEBUG_ASSERT(c->constant_table.fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");
		HccStringId debug_string_id = {0};
		return _hcc_constant_table_deduplicate_end(c, data_type, NULL, 0, 0, debug_string_id);
	}
}

HccConstantId _hcc_constant_table_deduplicate_end(HccCompiler* c, HccDataType data_type, void* data, U32 data_size, U32 data_align, HccStringId debug_string_id) {
	//
	// TODO: make this a hash table look up
	for (uint32_t entry_idx = 0; entry_idx < c->constant_table.entries_count; entry_idx += 1) {
		HccConstantEntry* entry = &c->constant_table.entries[entry_idx];
		if (entry->data_type == data_type && data_size == entry->size && memcmp(HCC_PTR_ADD(c->constant_table.data, entry->start_idx), data, data_size) == 0) {
			return (HccConstantId) { .idx_plus_one = entry_idx + 1 };
		}
	}

	if (c->constant_table.entries_count >= c->constant_table.entries_cap) {
		HCC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	if (c->constant_table.data_used_size + data_size > c->constant_table.data_cap) {
		HCC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	uint32_t new_entry_idx = c->constant_table.entries_count;
	c->constant_table.entries_count += 1;
	HccConstantEntry* entry = &c->constant_table.entries[new_entry_idx];
	entry->size = data_size;
	entry->data_type = data_type;
	entry->debug_string_id = debug_string_id;

	if (data_align) {
		c->constant_table.data_used_size = HCC_INT_ROUND_UP_ALIGN(c->constant_table.data_used_size, data_align);
		entry->start_idx = c->constant_table.data_used_size;
		c->constant_table.data_used_size += data_size;
	}

	if (c->constant_table.data_write_ptr != data && data_size) {
		memcpy(HCC_PTR_ADD(c->constant_table.data, entry->start_idx), data, data_size);
	}

	return (HccConstantId) { .idx_plus_one = new_entry_idx + 1 };
}

HccConstant hcc_constant_table_get(HccCompiler* c, HccConstantId id) {
	HCC_DEBUG_ASSERT(id.idx_plus_one, "constant id is null");

	HccConstantEntry* entry = &c->constant_table.entries[id.idx_plus_one - 1];

	HccConstant constant;
	constant.data_type = entry->data_type;
	constant.data = HCC_PTR_ADD(c->constant_table.data, entry->start_idx);
	constant.size = entry->size;
	return constant;
}

void hcc_constant_print(HccCompiler* c, HccConstantId constant_id, FILE* f) {
	HccConstant constant = hcc_constant_table_get(c, constant_id);
	if (constant.size == 0) {
		HccString data_type_name = hcc_data_type_string(c, constant.data_type);
		fprintf(f, "%.*s: <ZERO>", (int)data_type_name.size, data_type_name.data);
		return;
	}

	if (constant.data_type < HCC_DATA_TYPE_BASIC_END) {
		hcc_data_type_print_basic(c, constant.data_type, constant.data, f);
	} else {
		HCC_ABORT("unhandled type '%u'", constant.data_type);
	}
}

bool hcc_constant_as_uint(HccCompiler* c, HccConstant constant, U64* out) {
	if (!HCC_DATA_TYPE_IS_BASIC(constant.data_type)) {
		return false;
	}

	switch (c->basic_type_size_and_aligns[constant.data_type]) {
		case sizeof(U8): *out = *(U8*)constant.data; break;
		case sizeof(U16): *out = *(U16*)constant.data; break;
		case sizeof(U32): *out = *(U32*)constant.data; break;
		case sizeof(U64): *out = *(U64*)constant.data; break;
	}

	if (HCC_DATA_TYPE_IS_SINT(c, constant.data_type)) {
		if ((S64)*out < 0) {
			return false;
		}
	}

	return true;
}

bool hcc_constant_as_sint(HccCompiler* c, HccConstant constant, S64* out) {
	if (!HCC_DATA_TYPE_IS_BASIC(constant.data_type)) {
		return false;
	}

	switch (c->basic_type_size_and_aligns[constant.data_type]) {
		case sizeof(U8): *out = *(U8*)constant.data; break;
		case sizeof(U16): *out = *(U16*)constant.data; break;
		case sizeof(U32): *out = *(U32*)constant.data; break;
		case sizeof(U64): *out = *(U64*)constant.data; break;
	}

	if (
		HCC_DATA_TYPE_IS_UINT(c, constant.data_type) &&
		c->basic_type_size_and_aligns[constant.data_type] == sizeof(U64)
	) {
		if ((U64)*out > INT64_MAX) {
			return false;
		}
	}

	return true;
}

bool hcc_constant_as_sint32(HccCompiler* c, HccConstant constant, S32* out) {
	if (!HCC_DATA_TYPE_IS_BASIC(constant.data_type)) {
		return false;
	}

	S64 value;
	switch (c->basic_type_size_and_aligns[constant.data_type]) {
		case sizeof(U8): value = *(S8*)constant.data; break;
		case sizeof(U16): value = *(S16*)constant.data; break;
		case sizeof(U32): value = *(S32*)constant.data; break;
		case sizeof(U64): value = *(S64*)constant.data; break;
	}

	if (value < INT32_MIN || value > INT32_MAX) {
		return false;
	}

	*out = value;
	return true;
}

bool hcc_constant_as_float(HccCompiler* c, HccConstant constant, F64* out) {
	if (!HCC_DATA_TYPE_IS_BASIC(constant.data_type)) {
		return false;
	}

	switch (constant.data_type) {
		case HCC_DATA_TYPE_FLOAT: *out = *(F32*)constant.data; return true;
		case HCC_DATA_TYPE_DOUBLE: *out = *(F64*)constant.data; return true;
		default:
			if (HCC_DATA_TYPE_IS_UINT(c, constant.data_type)) {
				switch (c->basic_type_size_and_aligns[constant.data_type]) {
					case sizeof(U8): *out = *(U8*)constant.data; return true;
					case sizeof(U16): *out = *(U16*)constant.data; return true;
					case sizeof(U32): *out = *(U32*)constant.data; return true;
					case sizeof(U64): *out = *(U64*)constant.data; return true;
				}
			} else if (HCC_DATA_TYPE_IS_SINT(c, constant.data_type)) {
				switch (c->basic_type_size_and_aligns[constant.data_type]) {
					case sizeof(U8): *out = *(S8*)constant.data; return true;
					case sizeof(U16): *out = *(S16*)constant.data; return true;
					case sizeof(U32): *out = *(S32*)constant.data; return true;
					case sizeof(U64): *out = *(S64*)constant.data; return true;
				}
			}
			break;
	}

	return false;
}

// ===========================================
//
//
// String Table
//
//
// ===========================================

void hcc_string_table_init(HccStringTable* string_table, U32 data_cap, U32 entries_cap) {
	string_table->data = HCC_ALLOC_ARRAY(char, HCC_ALLOC_TAG_STRING_TABLE_DATA, data_cap);
	if (string_table->data == NULL) {
		hcc_compiler_bail_allocation_failure(_hcc_compiler_ptr, HCC_ALLOC_TAG_STRING_TABLE_DATA);
	}

	string_table->entries = HCC_ALLOC_ARRAY(HccStringEntry, HCC_ALLOC_TAG_STRING_TABLE_ENTRIES, entries_cap);
	if (string_table->entries == NULL) {
		hcc_compiler_bail_allocation_failure(_hcc_compiler_ptr, HCC_ALLOC_TAG_STRING_TABLE_ENTRIES);
	}

	string_table->data_cap = data_cap;
	string_table->entries_cap = entries_cap;
}

HccStringId hcc_string_table_deduplicate(HccStringTable* string_table, const char* string, U32 string_size) {
	//
	// TODO: make this a hash table look up
	for (U32 entry_idx = 0; entry_idx < string_table->entries_count; entry_idx += 1) {
		HccStringEntry* entry = &string_table->entries[entry_idx];

		if (string_size == entry->size && memcmp(string_table->data + entry->start_idx, string, string_size) == 0) {
			return (HccStringId) { .idx_plus_one = entry_idx + 1 };
		}
	}

	if (string_table->entries_count >= string_table->entries_cap) {
		HCC_ABORT("string tables entries capacity exceeded TODO make this error message proper");
	}

	if (string_table->data_used_size + string_size >= string_table->data_cap) {
		HCC_ABORT("string tables entries capacity exceeded TODO make this error message proper");
	}


	U32 new_entry_idx = string_table->entries_count;
	string_table->entries_count += 1;
	HccStringEntry* entry = &string_table->entries[new_entry_idx];
	entry->start_idx = string_table->data_used_size;
	entry->size = string_size;

	memcpy(string_table->data + string_table->data_used_size, string, string_size);
	string_table->data_used_size += string_size;

	return (HccStringId) { .idx_plus_one = new_entry_idx + 1 };
}

HccString hcc_string_table_get(HccStringTable* string_table, HccStringId id) {
	HCC_DEBUG_ASSERT(id.idx_plus_one, "string id is null");
	U32 idx = id.idx_plus_one - 1;
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(idx, string_table->entries_count);

	HccStringEntry* entry = &string_table->entries[idx];

	HccString string;
	string.data = string_table->data + entry->start_idx;
	string.size = entry->size;
	return string;
}

HccString hcc_string_table_get_or_empty(HccStringTable* string_table, HccStringId id) {
	if (id.idx_plus_one - 1 < string_table->entries_count) {
		return hcc_string_table_get(string_table, id);
	}

	return hcc_string_lit("");
}

char* hcc_string_intrinsic_param_names[HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END] = {
	[HCC_STRING_ID_UINT8_T] = "uint8_t",
	[HCC_STRING_ID_UINT16_T] = "uint16_t",
	[HCC_STRING_ID_UINT32_T] = "uint32_t",
	[HCC_STRING_ID_UINT64_T] = "uint64_t",
	[HCC_STRING_ID_UINTPTR_T] = "uintptr_t",
	[HCC_STRING_ID_INT8_T] = "int8_t",
	[HCC_STRING_ID_INT16_T] = "int16_t",
	[HCC_STRING_ID_INT32_T] = "int32_t",
	[HCC_STRING_ID_INT64_T] = "int64_t",
	[HCC_STRING_ID_INTPTR_T] = "intptr_t",
	[HCC_STRING_ID_HCC_VERTEX_INPUT] = "HccVertexInput",
	[HCC_STRING_ID_HCC_FRAGMENT_INPUT] = "HccFragmentInput",
	[HCC_STRING_ID_HALF] = "half",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2BOOL] = "pvec2bool",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2U8] = "pvec2u8",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2U16] = "pvec2u16",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2U32] = "pvec2u32",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2U64] = "pvec2u64",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2I8] = "pvec2i8",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2I16] = "pvec2i16",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2I32] = "pvec2i32",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2I64] = "pvec2i64",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2H] = "pvec2h",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2F] = "pvec2f",
	[HCC_STRING_ID_PVEC_START + HCC_VEC2D] = "pvec2d",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3BOOL] = "pvec3bool",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3U8] = "pvec3u8",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3U16] = "pvec3u16",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3U32] = "pvec3u32",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3U64] = "pvec3u64",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3I8] = "pvec3i8",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3I16] = "pvec3i16",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3I32] = "pvec3i32",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3I64] = "pvec3i64",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3H] = "pvec3h",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3F] = "pvec3f",
	[HCC_STRING_ID_PVEC_START + HCC_VEC3D] = "pvec3d",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4BOOL] = "pvec4bool",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4U8] = "pvec4u8",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4U16] = "pvec4u16",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4U32] = "pvec4u32",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4U64] = "pvec4u64",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4I8] = "pvec4i8",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4I16] = "pvec4i16",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4I32] = "pvec4i32",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4I64] = "pvec4i64",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4H] = "pvec4h",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4F] = "pvec4f",
	[HCC_STRING_ID_PVEC_START + HCC_VEC4D] = "pvec4d",
	[HCC_STRING_ID_VEC_START + HCC_VEC2BOOL] = "vec2bool",
	[HCC_STRING_ID_VEC_START + HCC_VEC2U8] = "vec2u8",
	[HCC_STRING_ID_VEC_START + HCC_VEC2U16] = "vec2u16",
	[HCC_STRING_ID_VEC_START + HCC_VEC2U32] = "vec2u32",
	[HCC_STRING_ID_VEC_START + HCC_VEC2U64] = "vec2u64",
	[HCC_STRING_ID_VEC_START + HCC_VEC2I8] = "vec2i8",
	[HCC_STRING_ID_VEC_START + HCC_VEC2I16] = "vec2i16",
	[HCC_STRING_ID_VEC_START + HCC_VEC2I32] = "vec2i32",
	[HCC_STRING_ID_VEC_START + HCC_VEC2I64] = "vec2i64",
	[HCC_STRING_ID_VEC_START + HCC_VEC2H] = "vec2h",
	[HCC_STRING_ID_VEC_START + HCC_VEC2F] = "vec2f",
	[HCC_STRING_ID_VEC_START + HCC_VEC2D] = "vec2d",
	[HCC_STRING_ID_VEC_START + HCC_VEC3BOOL] = "vec3bool",
	[HCC_STRING_ID_VEC_START + HCC_VEC3U8] = "vec3u8",
	[HCC_STRING_ID_VEC_START + HCC_VEC3U16] = "vec3u16",
	[HCC_STRING_ID_VEC_START + HCC_VEC3U32] = "vec3u32",
	[HCC_STRING_ID_VEC_START + HCC_VEC3U64] = "vec3u64",
	[HCC_STRING_ID_VEC_START + HCC_VEC3I8] = "vec3i8",
	[HCC_STRING_ID_VEC_START + HCC_VEC3I16] = "vec3i16",
	[HCC_STRING_ID_VEC_START + HCC_VEC3I32] = "vec3i32",
	[HCC_STRING_ID_VEC_START + HCC_VEC3I64] = "vec3i64",
	[HCC_STRING_ID_VEC_START + HCC_VEC3H] = "vec3h",
	[HCC_STRING_ID_VEC_START + HCC_VEC3F] = "vec3f",
	[HCC_STRING_ID_VEC_START + HCC_VEC3D] = "vec3d",
	[HCC_STRING_ID_VEC_START + HCC_VEC4BOOL] = "vec4bool",
	[HCC_STRING_ID_VEC_START + HCC_VEC4U8] = "vec4u8",
	[HCC_STRING_ID_VEC_START + HCC_VEC4U16] = "vec4u16",
	[HCC_STRING_ID_VEC_START + HCC_VEC4U32] = "vec4u32",
	[HCC_STRING_ID_VEC_START + HCC_VEC4U64] = "vec4u64",
	[HCC_STRING_ID_VEC_START + HCC_VEC4I8] = "vec4i8",
	[HCC_STRING_ID_VEC_START + HCC_VEC4I16] = "vec4i16",
	[HCC_STRING_ID_VEC_START + HCC_VEC4I32] = "vec4i32",
	[HCC_STRING_ID_VEC_START + HCC_VEC4I64] = "vec4i64",
	[HCC_STRING_ID_VEC_START + HCC_VEC4H] = "vec4h",
	[HCC_STRING_ID_VEC_START + HCC_VEC4F] = "vec4f",
	[HCC_STRING_ID_VEC_START + HCC_VEC4D] = "vec4d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT2X2F] = "pmat2x2f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT2X2D] = "pmat2x2d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT2X3F] = "pmat2x3f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT2X3D] = "pmat2x3d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT2X4F] = "pmat2x4f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT2X4D] = "pmat2x4d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT3X2F] = "pmat3x2f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT3X2D] = "pmat3x2d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT3X3F] = "pmat3x3f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT3X3D] = "pmat3x3d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT3X4F] = "pmat3x4f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT3X4D] = "pmat3x4d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT4X2F] = "pmat4x2f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT4X2D] = "pmat4x2d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT4X3F] = "pmat4x3f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT4X3D] = "pmat4x3d",
	[HCC_STRING_ID_PMAT_START + HCC_MAT4X4F] = "pmat4x4f",
	[HCC_STRING_ID_PMAT_START + HCC_MAT4X4D] = "pmat4x4d",
	[HCC_STRING_ID_MAT_START + HCC_MAT2X2F] = "mat2x2f",
	[HCC_STRING_ID_MAT_START + HCC_MAT2X2D] = "mat2x2d",
	[HCC_STRING_ID_MAT_START + HCC_MAT2X3F] = "mat2x3f",
	[HCC_STRING_ID_MAT_START + HCC_MAT2X3D] = "mat2x3d",
	[HCC_STRING_ID_MAT_START + HCC_MAT2X4F] = "mat2x4f",
	[HCC_STRING_ID_MAT_START + HCC_MAT2X4D] = "mat2x4d",
	[HCC_STRING_ID_MAT_START + HCC_MAT3X2F] = "mat3x2f",
	[HCC_STRING_ID_MAT_START + HCC_MAT3X2D] = "mat3x2d",
	[HCC_STRING_ID_MAT_START + HCC_MAT3X3F] = "mat3x3f",
	[HCC_STRING_ID_MAT_START + HCC_MAT3X3D] = "mat3x3d",
	[HCC_STRING_ID_MAT_START + HCC_MAT3X4F] = "mat3x4f",
	[HCC_STRING_ID_MAT_START + HCC_MAT3X4D] = "mat3x4d",
	[HCC_STRING_ID_MAT_START + HCC_MAT4X2F] = "mat4x2f",
	[HCC_STRING_ID_MAT_START + HCC_MAT4X2D] = "mat4x2d",
	[HCC_STRING_ID_MAT_START + HCC_MAT4X3F] = "mat4x3f",
	[HCC_STRING_ID_MAT_START + HCC_MAT4X3D] = "mat4x3d",
	[HCC_STRING_ID_MAT_START + HCC_MAT4X4F] = "mat4x4f",
	[HCC_STRING_ID_MAT_START + HCC_MAT4X4D] = "mat4x4d",
};

// ===========================================
//
//
// Compiler
//
//
// ===========================================

HccCompilerSetup hcc_compiler_setup_default = {
	.pp = {
		.macro_tokens_cap = 64 * 1024,
		.macro_token_values_cap = 64 * 1024,
		.macros_cap = 64 * 1024,
		.macro_params_cap = 64 * 1024,
		.macro_args_stack_cap = 64 * 1024,
		.expand_stack_cap = 64 * 1024,
		.stringify_buffer_cap = 64 * 1024,
		.if_stack_cap = 64 * 1024,
	},
	.tokengen = {
		.tokens_cap = 64 * 1024,
		.token_values_cap = 64 * 1024,
		.token_locations_cap = 64 * 1024,
		.paused_file_stack_cap = 64 * 1024,
		.open_bracket_stack_cap = 64 * 1024,
	},
	.astgen = {
		.function_params_and_variables_cap = 64 * 1024,
		.functions_cap = 64 * 1024,
		.exprs_cap = 64 * 1024,
		.expr_locations_cap = 64 * 1024,
		.global_variables_cap = 64 * 1024,
		.function_used_function_indices_cap = 64 * 1024,
		.function_used_static_variables_cap = 64 * 1024,
		.entry_point_function_indices_cap = 64 * 1024,
		.used_function_indices_cap = 64 * 1024,
		.recursive_function_indices_stack_cap = 64 * 1024,
		.array_data_types_cap = 64 * 1024,
		.struct_data_types_cap = 64 * 1024,
		.union_data_types_cap = 64 * 1024,
		.compound_fields_cap = 64 * 1024,
		.compound_type_fields_cap = 64 * 1024,
		.typedefs_cap = 64 * 1024,
		.enum_data_types_cap = 64 * 1024,
		.enum_values_cap = 64 * 1024,
		.ordered_data_types_cap = 64 * 1024,
		.compound_type_find_fields_cap = 64 * 1024,
		.curly_initializer_nested_cap = 64 * 1024,
		.curly_initializer_nested_curlys_cap = 64 * 1024,
		.curly_initializer_nested_elmts_cap = 64 * 1024,
		.curly_initializer_designator_initializers_cap = 64 * 1024,
		.curly_initializer_designator_initializer_elmt_indices_cap = 64 * 1024,
		.variable_stack_cap = 64 * 1024,
	},
	.irgen = {
		.functions_cap = 64 * 1024,
		.basic_blocks_cap = 64 * 1024,
		.values_cap = 64 * 1024,
		.instructions_cap = 64 * 1024,
		.operands_cap = 64 * 1024,
		.function_call_param_data_types_cap = 64 * 1024,
	},
	.spirvgen = {
		.out_capabilities_cap = 64 * 1024,
		.out_entry_points_cap = 64 * 1024,
		.out_debug_info_cap = 64 * 1024,
		.out_annotations_cap = 64 * 1024,
		.out_types_variables_constants_cap = 64 * 1024,
		.out_functions_cap = 64 * 1024,
	},

	.messages_cap = 64 * 1024,
	.message_strings_cap = 64 * 1024,
	.string_table_data_cap = 64 * 1024,
	.string_table_entries_cap = 64 * 1024,
	.string_buffer_cap = 64 * 1024,
	.include_paths_cap = 64 * 1024,
	.code_files_cap = 64 * 1024,
	.code_file_lines_cap = 64 * 1024,
	.code_file_pp_if_spans_cap = 64 * 1024,
};

void hcc_string_table_intrinsic_add(HccCompiler* c, U32 expected_string_id, const char* string) {
	HccStringId id = hcc_string_table_deduplicate(&c->string_table, string, strlen(string));
	HCC_DEBUG_ASSERT(id.idx_plus_one == expected_string_id, "intrinsic string id for '%s' does not match! expected '%u' but got '%u'", string, expected_string_id, id.idx_plus_one);
}

bool hcc_compiler_init(HccCompiler* c, HccCompilerSetup* setup) {
	if (setjmp(c->compile_entry_jmp_loc)) {
		return false;
	}
	c->flags |= HCC_COMPILER_FLAGS_SET_LONG_JMP;
	c->code_file_lines_cap = setup->code_file_lines_cap;
	c->code_file_pp_if_spans_cap = setup->code_file_pp_if_spans_cap;
	hcc_options_set_enabled(c, HCC_OPTION_PRINT_COLOR);
	c->arch = HCC_ARCH_X86_64;
	c->os = HCC_OS_LINUX;

	switch (c->arch) {
		case HCC_ARCH_X86_64:
			switch (c->os) {
				case HCC_OS_LINUX:
					c->basic_type_size_and_aligns = hcc_basic_type_size_and_aligns_x86_64_linux;
					c->basic_type_int_mins = hcc_basic_type_int_mins_x86_64_linux;
					c->basic_type_int_maxes = hcc_basic_type_int_maxes_x86_64_linux;
					break;
				default: HCC_ABORT("internal error: unhandled OS for the X86_64 architecture");
			}
			break;
		default: HCC_ABORT("internal error: unhandled architecture");
	}

	c->message_sys.elmts = hcc_stack_init(HccMessage, setup->messages_cap, HCC_ALLOC_TAG_MESSAGES);
	c->message_sys.deferred_elmts = hcc_stack_init(HccMessage, setup->messages_cap, HCC_ALLOC_TAG_MESSAGES);
	c->message_sys.locations = hcc_stack_init(HccLocation, setup->messages_cap * 2, HCC_ALLOC_TAG_MESSAGES);
	c->message_sys.strings = hcc_stack_init(char, setup->message_strings_cap, HCC_ALLOC_TAG_MESSAGE_STRINGS);
	c->string_buffer = hcc_stack_init(char, setup->string_buffer_cap, HCC_ALLOC_TAG_STRING_BUFFER);
	c->include_paths = hcc_stack_init(HccString, setup->include_paths_cap, HCC_ALLOC_TAG_INCLUDE_PATHS);
	c->code_files = hcc_stack_init(HccCodeFile, setup->code_files_cap, HCC_ALLOC_TAG_CODE_FILES);
	hcc_hash_table_init(&c->path_to_code_file_id_map, setup->code_files_cap, HCC_ALLOC_TAG_PATH_TO_CODE_FILE_ID_MAP);

	hcc_constant_table_init(c, setup->string_table_data_cap, setup->string_table_entries_cap);
	{
		for (HccDataType data_type = HCC_DATA_TYPE_BOOL; data_type < HCC_DATA_TYPE_BASIC_COUNT; data_type += 1) {
			c->basic_type_zero_constant_ids[data_type] = hcc_constant_table_deduplicate_zero(c, data_type);
		}

		HccBasic one;
		one.u8 = 1;
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_BOOL] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_BOOL, &one);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_CHAR] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_CHAR, &one);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_SCHAR] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SCHAR, &one);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_UCHAR] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_UCHAR, &one);
		one = hcc_basic_from_sint(c, HCC_DATA_TYPE_SSHORT, 1);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_SSHORT] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SSHORT, &one);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_USHORT] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_USHORT, &one);
		one = hcc_basic_from_sint(c, HCC_DATA_TYPE_SINT, 1);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_SINT] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SINT, &one);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_UINT] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_UINT, &one);
		one = hcc_basic_from_sint(c, HCC_DATA_TYPE_SLONG, 1);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_SLONG] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SLONG, &one);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_ULONG] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_ULONG, &one);
		one = hcc_basic_from_sint(c, HCC_DATA_TYPE_SLONGLONG, 1);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_SLONGLONG] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_SLONGLONG, &one);
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_ULONGLONG] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_ULONGLONG, &one);
		one.f = 1.f;
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_FLOAT] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_FLOAT, &one);
		one.d = 1.f;
		c->basic_type_one_constant_ids[HCC_DATA_TYPE_DOUBLE] = hcc_constant_table_deduplicate_basic(c, HCC_DATA_TYPE_DOUBLE, &one);
	}

	hcc_string_table_init(&c->string_table, setup->string_table_data_cap, setup->string_table_entries_cap);
	{
		for (U32 expected_string_id = HCC_STRING_ID_INTRINSIC_PARAM_NAMES_START; expected_string_id < HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END; expected_string_id += 1) {
			char* string = hcc_string_intrinsic_param_names[expected_string_id];
			hcc_string_table_intrinsic_add(c, expected_string_id, string);
		}

		for (HccToken t = HCC_TOKEN_KEYWORDS_START; t < HCC_TOKEN_KEYWORDS_END; t += 1) {
			char* string = hcc_token_strings[t];
			U32 expected_string_id = HCC_STRING_ID_KEYWORDS_START + (t - HCC_TOKEN_KEYWORDS_START);
			hcc_string_table_intrinsic_add(c, expected_string_id, string);
		}

		for (U32 f = 0; f < HCC_FUNCTION_IDX_INTRINSIC_END; f += 1) {
			const char* string = hcc_intrinsic_function_strings[f];
			U32 expected_string_id = HCC_STRING_ID_FUNCTION_IDXS_START + f;
			hcc_string_table_intrinsic_add(c, expected_string_id, string);
		}

		for (HccPPPredefinedMacro m = 0; m < HCC_PP_PREDEFINED_MACRO_COUNT; m += 1) {
			char* string = hcc_pp_predefined_macro_identifier_strings[m];
			U32 expected_string_id = HCC_STRING_ID_PREDEFINED_MACROS_START + m;
			hcc_string_table_intrinsic_add(c, expected_string_id, string);
		}

		hcc_string_table_intrinsic_add(c, HCC_STRING_ID_ONCE, "once");
		hcc_string_table_intrinsic_add(c, HCC_STRING_ID_DEFINED, "defined");
		hcc_string_table_intrinsic_add(c, HCC_STRING_ID___VA_ARGS__, "__VA_ARGS__");
	}

	c->concat_buffer_code_file = hcc_code_file_init(c, hcc_string_table_deduplicate_lit(&c->string_table, "<concat buffer>\0"));

	//hcc_opt_set_enabled(&c->astgen.opts, HCC_OPTION_CONSTANT_FOLDING);

	hcc_pp_init(c, setup);
	hcc_tokengen_init(c, setup);
	hcc_astgen_init(c, setup);
	hcc_irgen_init(c, setup);
	hcc_spirvgen_init(c, setup);

	return true;
}

bool hcc_compiler_compile(HccCompiler* c, char* file_path) {
	if (setjmp(c->compile_entry_jmp_loc)) {
		return false;
	}
	_hcc_compiler_ptr = c;
	c->flags |= HCC_COMPILER_FLAGS_SET_LONG_JMP;

	c->allocation_failure_alloc_tag = HCC_ALLOC_TAG_NONE;
	c->collection_is_full_alloc_tag = HCC_ALLOC_TAG_NONE;

	U32 file_path_size = strlen(file_path) + 1;

	HccCodeFileId code_file_id;
	HccCodeFile* code_file;
	bool found_file = hcc_code_file_find_or_insert(c, hcc_string_c(file_path), &code_file_id, &code_file);
	HCC_DEBUG_ASSERT(!found_file, "internal error: root file should be able to be inserted into the hash table no problem");

	U64 code_size;
	char* code = hcc_file_read_all_the_codes(file_path, &code_size);
	if (code == NULL) {
		char buf[512];
		hcc_get_last_system_error_string(buf, sizeof(buf));
		printf("failed to read file '%s': %s\n", file_path, buf);
	}

	code_file->flags |= HCC_CODE_FILE_FLAGS_COMPILATION_UNIT;
	code_file->code = hcc_string(code, code_size);

	hcc_tokengen_location_setup_new_file(c, code_file, true);
	hcc_tokengen_run(c, &c->tokengen.token_bag, HCC_TOKENGEN_RUN_MODE_CODE);

	hcc_astgen_generate(c);

	if (c->message_sys.used_type_flags & HCC_MESSAGE_TYPE_ERROR) {
		return false;
	}

	hcc_irgen_generate(c);
	hcc_spirvgen_generate(c);

	hcc_tokengen_print(c, stdout);
	hcc_astgen_print(c, stdout);
	hcc_irgen_print(c, stdout);

	c->flags &= ~HCC_COMPILER_FLAGS_SET_LONG_JMP;
	HCC_ZERO_ELMT(&c->compile_entry_jmp_loc);

	return !hcc_stack_count(c->message_sys.elmts);
}

noreturn void hcc_compiler_bail(HccCompiler* c) {
	HCC_DEBUG_ASSERT(c->flags & HCC_COMPILER_FLAGS_SET_LONG_JMP, "error: the long jump has not been set");
	longjmp(c->compile_entry_jmp_loc, 1);
}

noreturn void hcc_compiler_bail_allocation_failure(HccCompiler* c, HccAllocTag tag) {
	c->allocation_failure_alloc_tag = tag;
	hcc_compiler_bail(c);
}

noreturn void hcc_compiler_bail_collection_is_full(HccCompiler* c, HccAllocTag tag) {
	c->collection_is_full_alloc_tag = tag;
	hcc_compiler_bail(c);
}

bool hcc_options_is_enabled(HccCompiler* c, HccOption opt) {
	U64 bit = ((U64)1 << (opt % 64));
	return (c->options.bitset[opt / 64] & bit) == bit;
}

void hcc_options_set_enabled(HccCompiler* c, HccOption opt) {
	U64 bit = (U64)1 << (opt % 64);
	c->options.bitset[opt / 64] |= bit;
}

void hcc_options_set_disabled(HccCompiler* c, HccOption opt) {
	U64 bit = (U64)1 << (opt % 64);
	c->options.bitset[opt / 64] &= ~bit;
}

