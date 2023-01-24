#pragma once

#include <setjmp.h> // jmp_buf
#include <stdatomic.h>
#include <immintrin.h>
#include <signal.h>

#include "hcc.h"

typedef struct HccWorker HccWorker;

// ===========================================
//
//
// Config
//
//
// ===========================================

#ifndef HCC_ENABLE_DEBUG_ASSERTIONS
#define HCC_ENABLE_DEBUG_ASSERTIONS 1
#endif

#ifndef HCC_ENABLE_WORKER_LOGGING
#define HCC_ENABLE_WORKER_LOGGING 1
#endif

// ===========================================
//
//
// Misc Macros
//
//
// ===========================================

#ifdef __linux__
#define HCC_OS_LINUX
#endif

#ifdef _WIN32
#define HCC_OS_WINDOWS
#endif

#ifdef __x86_64__
#define HCC_ARCH_X86_64
#endif

#define HCC_LITTLE_ENDIAN 0
#define HCC_BIG_ENDIAN    1
#ifdef HCC_OS_LINUX

#ifdef HCC_ARCH_X86_64
#define HCC_BYTE_ORDER HCC_LITTLE_ENDIAN
#else
#error "unsupported platform"
#endif

#else
#error "unsupported platform"
#endif

#define HCC_STRINGIFY(v) #v
#define HCC_CONCAT_0(a, b) a##b
#define HCC_CONCAT(a, b) HCC_CONCAT_0(a, b)

#ifdef __GNUC__
#define HCC_LIKELY(expr) __builtin_expect((expr), 1)
#define HCC_UNLIKELY(expr) __builtin_expect((expr), 0)
#else
#define HCC_LIKELY(expr) expr
#define HCC_UNLIKELY(expr) expr
#endif

#define hcc_fallthrough __attribute__ ((fallthrough))

// trailing _Atomic so it works with pointer types HccAtomic(T)* is different from HccAtomic(T*)
#define HccAtomic(T) T _Atomic

#define HCC_PRAGMA(x) _Pragma(#x)
#define HCC_PRAGMA_MESSAGE(x) HCC_PRAGMA(message #x)
#ifdef HCC_ENABLE_CONTRIBUTOR_TASK_LOG
#define HCC_CONTRIBUTOR_TASK(msg) HCC_PRAGMA_MESSAGE([contributor task][msg])
#else
#define HCC_CONTRIBUTOR_TASK(msg)
#endif

#define HCC_CPU_RELAX() _mm_pause()

#define HCC_LEAST_SET_BIT_REMOVE(bitset) ((bitset) & ((bitset) - 1))

// ===========================================
//
//
// Abort & Assertions
//
//
// ===========================================

#ifndef HCC_ABORT
#define HCC_ABORT(...) _hcc_abort(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef HCC_ASSERT
#define HCC_ASSERT(cond, ...) if (HCC_UNLIKELY(!(cond))) _hcc_assert_failed(#cond, __FILE__, __LINE__, __VA_ARGS__)
#endif

#if HCC_ENABLE_DEBUG_ASSERTIONS
#define HCC_DEBUG_ASSERT HCC_ASSERT
#else
#define HCC_DEBUG_ASSERT(cond, ...) (void)(cond)
#endif

#define HCC_DEBUG_ASSERT_NON_ZERO(value) HCC_DEBUG_ASSERT(value, "'%s' must be a non-zero value", #value)
#define HCC_DEBUG_ASSERT_COMMIT_RESERVE_SIZE(commit_size, reserve_size) HCC_DEBUG_ASSERT((commit_size) <= (reserve_size), #commit_size " '%zu' must be less than or equal to " #reserve_size " '%zu'", (commit_size), (reserve_size))
#define HCC_DEBUG_ASSERT_ARRAY_BOUNDS(idx, count) (((idx) < (count)) ? (idx) : HCC_ABORT("idx '%zu' is out of bounds for an array of count '%zu'", (idx), (count)))
#define HCC_DEBUG_ASSERT_ARRAY_RESIZE(count, cap) HCC_DEBUG_ASSERT((count) <= (cap), "cannot resize array to count '%zu' when it has a capacity of '%zu'", (count), (cap))
#define HCC_DEBUG_ASSERT_POWER_OF_TWO(v) HCC_DEBUG_ASSERT(HCC_IS_POWER_OF_TWO(v), #v "must be a power of two but got '%zu'", v)

#if HCC_ENABLE_DEBUG_ASSERTIONS
#define HCC_UNREACHABLE(...) HCC_ABORT("unreachable code: " __VA_ARGS__);
#else
#define HCC_UNREACHABLE(...) __builtin_unreachable()
#endif

#define HCC_STACKTRACE_CAP 4096

#define HCC_SET_BAIL_JMP_LOC_GLOBAL() \
	_hcc_tls.jmp_loc_recursive_set_count += 1; \
	if (_hcc_tls.jmp_result_data == NULL) { \
		if (setjmp(_hcc_tls.jmp_loc)) { \
			hcc_clear_bail_jmp_loc(); \
			return _hcc_tls.result_data.result; \
		} \
		_hcc_tls.jmp_result_data = &_hcc_tls.result_data; \
	} \

#define HCC_SET_BAIL_JMP_LOC_COMPILER() \
	_hcc_tls.jmp_loc_recursive_set_count += 1; \
	if (_hcc_tls.jmp_result_data == NULL) { \
		if (setjmp(_hcc_tls.jmp_loc)) { \
			hcc_clear_bail_jmp_loc(); \
			return c->result_data.result; \
		} \
		_hcc_tls.c = c; \
		_hcc_tls.jmp_result_data = &c->result_data; \
	} \

#define HCC_SET_BAIL_JMP_LOC_WORKER() \
	_hcc_tls.jmp_loc_recursive_set_count += 1; \
	if (_hcc_tls.jmp_result_data == NULL) { \
		if (setjmp(_hcc_tls.jmp_loc)) { \
			hcc_clear_bail_jmp_loc(); \
		} else { \
			_hcc_tls.w = w; \
			_hcc_tls.c = w->c; \
			_hcc_tls.jmp_result_data = &w->c->result_data; \
		} \
	} \

void _hcc_assert_failed(const char* cond, const char* file, int line, const char* message, ...);
noreturn uintptr_t _hcc_abort(const char* file, int line, const char* message, ...);

HccResult hcc_get_last_global_result();

void hcc_clear_bail_jmp_loc();
noreturn void hcc_bail(HccResultCode code, int32_t value);

// ===========================================
//
//
// Platform Abstraction
//
//
// ===========================================

uint32_t hcc_onebitscount32(uint32_t bits);
uint32_t hcc_leastsetbitidx32(uint32_t bits);
uint32_t hcc_mostsetbitidx32(uint32_t bits);
void hcc_get_last_system_error_string(char* buf_out, uint32_t buf_out_size);
bool hcc_change_working_directory(const char* path);
uint32_t hcc_path_canonicalize_internal(const char* path, char* out_buf);
HccString hcc_path_canonicalize(const char* path);
bool hcc_path_is_absolute(const char* path);
char* hcc_file_read_all_the_codes(const char* path, uint64_t* size_out);
void hcc_stacktrace(uint32_t ignore_levels_count, char* buf, uint32_t buf_size);
bool hcc_file_open_read(const char* path, HccIIO* out);
bool hcc_file_open_write(const char* path, HccIIO* out);
bool hcc_path_is_relative(const char* path);
bool hcc_path_exists(const char* path);
bool hcc_path_is_file(const char* path);
bool hcc_path_is_directory(const char* path);
HccString hcc_path_replace_file_name(HccString parent, HccString file_name);
uint32_t hcc_logical_cores_count();

// ===========================================
//
//
// Pointer & Number Utilities
//
//
// ===========================================

#define HCC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HCC_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define HCC_CLAMP(v, min, max) (((v) > (max)) ? (max) : ((v) < (min)) ? (min) : (v))
#define HCC_UNUSED(expr) ((void)(expr))

// align must be a power of 2
#define HCC_INT_ROUND_UP_ALIGN(i, align) (((i) + ((align) - 1)) & ~((align) - 1))
// align must be a power of 2
#define HCC_INT_ROUND_DOWN_ALIGN(i, align) ((i) & ~((align) - 1))

#define HCC_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
#define HCC_IS_POWER_OF_TWO_OR_ZERO(v) ((((v) & ((v) - 1)) == 0))
#define HCC_IS_POWER_OF_TWO(v) (((v) != 0) && (((v) & ((v) - 1)) == 0))
#define HCC_PTR_ADD(ptr, by) (void*)((uintptr_t)(ptr) + (uintptr_t)(by))
#define HCC_PTR_SUB(ptr, by) (void*)((uintptr_t)(ptr) - (uintptr_t)(by))
#define HCC_PTR_DIFF(to, from) ((char*)(to) - (char*)(from))
// align must be a power of 2
#define HCC_PTR_ROUND_UP_ALIGN(ptr, align) ((void*)HCC_INT_ROUND_UP_ALIGN((uintptr_t)(ptr), (uintptr_t)(align)))
// align must be a power of 2
#define HCC_PTR_ROUND_DOWN_ALIGN(ptr, align) ((void*)HCC_INT_ROUND_DOWN_ALIGN((uintptr_t)(ptr), (uintptr_t)(align)))
#define HCC_ZERO_ELMT(ptr) memset(ptr, 0, sizeof(*(ptr)))
#define HCC_ONE_ELMT(ptr) memset(ptr, 0xff, sizeof(*(ptr)))
#define HCC_ZERO_ELMT_MANY(ptr, elmts_count) memset(ptr, 0, sizeof(*(ptr)) * (elmts_count))
#define HCC_ONE_ELMT_MANY(ptr, elmts_count) memset(ptr, 0xff, sizeof(*(ptr)) * (elmts_count))
#define HCC_ZERO_ARRAY(array) memset(array, 0, sizeof(array))
#define HCC_ONE_ARRAY(array) memset(array, 0xff, sizeof(array))
#define HCC_COPY_ARRAY(dst, src) memcpy(dst, src, sizeof(dst))
#define HCC_COPY_ELMT_MANY(dst, src, elmts_count) memcpy(dst, src, elmts_count * sizeof(*(dst)))
#define HCC_COPY_OVERLAP_ELMT_MANY(dst, src, elmts_count) memmove(dst, src, elmts_count * sizeof(*(dst)))
#define HCC_CMP_ARRAY(a, b) (memcmp(a, b, sizeof(a)) == 0)
#define HCC_CMP_ELMT(a, b) (memcmp(a, b, sizeof(*(a))) == 0)
#define HCC_CMP_ELMT_MANY(a, b, elmts_count) (memcmp(a, b, elmts_count * sizeof(*(a))) == 0)

// TODO: on other architectures this could be different, like the Apple M1
#define HCC_CACHE_LINE_SIZE 64
#define HCC_CACHE_LINE_ALIGN 64

#define HCC_DIV_ROUND_UP(a, b) (((a) / (b)) + ((a) % (b) != 0))

uint32_t hcc_uint32_round_to_multiple(uint32_t v, uint32_t multiple);
uint32_t hcc_uint32_round_up_to_multiple(uint32_t v, uint32_t multiple);
uint32_t hcc_uint32_round_down_to_multiple(uint32_t v, uint32_t multiple);
uint64_t hcc_uint64_round_to_multiple(uint64_t v, uint64_t multiple);
uint64_t hcc_uint64_round_up_to_multiple(uint64_t v, uint64_t multiple);
uint64_t hcc_uint64_round_down_to_multiple(uint64_t v, uint64_t multiple);
float hcc_float_round_to_multiple(float v, float multiple);
float hcc_float_round_up_to_multiple(float v, float multiple);
float hcc_float_round_down_to_multiple(float v, float multiple);

typedef struct half half;
struct half { uint16_t _bits; };

float f16tof32(half v);
half f32tof16(float v);

static inline bool hcc_u64_checked_add(uint64_t a, uint64_t b, uint64_t* out) {
	if (b > (UINT64_MAX - a)) { return false; }
	*out = a + b;
	return true;
}

static inline bool hcc_s64_checked_add(int64_t a, int64_t b, int64_t* out) {
	if (a >= 0) {
		if (b > (INT64_MAX - a)) { return false; }
	} else {
		if (b < (INT64_MIN - a)) { return false; }
	}

	*out = a + b;
	return true;
}

static inline bool hcc_i64_checked_mul(uint64_t a, uint64_t b, uint64_t* out) {
	uint64_t r = a * b;
	if (a != 0 && b != 0 && a != r / b) {
		return false;
	}
	*out = r;
	return true;
}

static inline bool hcc_ascii_is_alpha(uint32_t byte) {
	return ((byte | 32u) - 97u) < 26u;
}

static inline bool hcc_ascii_is_digit(uint32_t byte) {
	return (byte - '0') < 10;
}

static inline bool hcc_ascii_is_a_to_f(uint32_t byte) {
	return ((byte | 32u) - 97u) < 6u;
}

// ===========================================
//
//
// Result
//
//
// ===========================================

typedef struct HccResultData HccResultData;
struct HccResultData {
	HccResult result;
	char      result_stacktrace[HCC_STACKTRACE_CAP];
};

// ===========================================
//
//
// Thread
//
//
// ===========================================

typedef struct HccThread HccThread;
struct HccThread {
#ifdef __linux__
	pthread_t handle;
#endif
};

typedef void (*HccThreadMainFn)(void* arg);

typedef struct HccThreadSetup HccThreadSetup;
struct HccThreadSetup {
	HccThreadMainFn thread_main_fn;
	void*           arg;
	void*           call_stack;
	uintptr_t       call_stack_size;
};

void hcc_thread_start(HccThread* thread, HccThreadSetup* setup);
void hcc_thread_wait_for_termination(HccThread* thread);

// ===========================================
//
//
// Semaphore
//
//
// ===========================================

typedef struct HccSemaphore HccSemaphore;
struct HccSemaphore {
	HccAtomic(uint32_t) value;
	HccAtomic(uint32_t) waiters_count;
};

void hcc_semaphore_init(HccSemaphore* semaphore, uint32_t initial_value);
void hcc_semaphore_set(HccSemaphore* semaphore, uint32_t value);
void hcc_semaphore_give(HccSemaphore* semaphore, uint32_t count);
void hcc_semaphore_take_or_wait_then_take(HccSemaphore* semaphore);
uint32_t hcc_semaphore_get_waiters_count(HccSemaphore* semaphore);

// ===========================================
//
//
// Spin Mutex
//
//
// ===========================================

typedef struct HccSpinMutex HccSpinMutex;
struct HccSpinMutex {
	HccAtomic(uint8_t) is_locked;
};

void hcc_spin_mutex_init(HccSpinMutex* mutex);
bool hcc_spin_mutex_is_locked(HccSpinMutex* mutex);
void hcc_spin_mutex_lock(HccSpinMutex* mutex);
void hcc_spin_mutex_unlock(HccSpinMutex* mutex);

// ===========================================
//
//
// Mutex
//
//
// ===========================================

typedef struct HccMutex HccMutex;
struct HccMutex {
	HccAtomic(uint32_t) is_locked;
};

void hcc_mutex_init(HccMutex* mutex);
bool hcc_mutex_is_locked(HccMutex* mutex);
void hcc_mutex_lock(HccMutex* mutex);
void hcc_mutex_unlock(HccMutex* mutex);

// ===========================================
//
//
// Allocation Tagging
//
//
// ===========================================

void hcc_mem_tracker_init();
void hcc_mem_tracker_deinit();
void hcc_mem_tracker_update(HccAllocMode mode, HccAllocTag tag, void* addr, uintptr_t size);

// ===========================================
//
//
// Arena Alctor
//
//
// ===========================================

typedef struct HccArenaHeader HccArenaHeader;
struct HccArenaHeader {
	HccArenaHeader*     prev;
	HccAtomic(uint32_t) pos;
	uint32_t            size;
};

typedef struct HccArenaAlctor HccArenaAlctor;
struct HccArenaAlctor {
	HccAtomic(HccArenaHeader*) arena;
	HccAtomic(bool) alloc_arena_sync_point;
	HccAllocTag     tag;
};

HccArenaHeader* hcc_arena_alloc(HccAllocTag tag, uint32_t arena_size);
void hcc_arena_alctor_init(HccArenaAlctor* alctor, HccAllocTag tag, uint32_t arena_size);
void hcc_arena_alctor_deinit(HccArenaAlctor* alctor);
void* hcc_arena_alctor_alloc(HccArenaAlctor* alctor, uint32_t size, uint32_t align);
void* hcc_arena_alctor_alloc_thread_safe(HccArenaAlctor* alctor, uint32_t size, uint32_t align);
void hcc_arena_alctor_reset(HccArenaAlctor* alctor);
#define HCC_ARENA_ALCTOR_ALLOC_ELMT(T, alctor) hcc_arena_alctor_alloc(alctor, sizeof(T), alignof(T))
#define HCC_ARENA_ALCTOR_ALLOC_ARRAY(T, alctor, count) hcc_arena_alctor_alloc(alctor, sizeof(T) * count, alignof(T))
#define HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(T, alctor) hcc_arena_alctor_alloc_thread_safe(alctor, sizeof(T), alignof(T))
#define HCC_ARENA_ALCTOR_ALLOC_ARRAY_THREAD_SAFE(T, alctor, count) hcc_arena_alctor_alloc_thread_safe(alctor, sizeof(T) * count, alignof(T))

// ===========================================
//
//
// Virtual Memory Abstraction
//
//
// ===========================================

typedef uint8_t HccVirtMemProtection;
enum {
	HCC_VIRT_MEM_PROTECTION_NO_ACCESS,
	HCC_VIRT_MEM_PROTECTION_READ,
	HCC_VIRT_MEM_PROTECTION_READ_WRITE,
	HCC_VIRT_MEM_PROTECTION_EXEC_READ,
	HCC_VIRT_MEM_PROTECTION_EXEC_READ_WRITE,
};

void hcc_virt_mem_update_page_size_reserve_align();

//
// @return:
//     the page size of the OS.
//     used to align the parameters of the virtual memory functions to a page.
uintptr_t hcc_virt_mem_page_size();

//
// @return:
//     the alignment of the address of a virtual memory allocation.
//     this is used as the alignment the requested_addr parameter of hcc_virt_mem_alloc.
//     this is guaranteed to the be the same as page size or a multiple of it.
//     On Unix: this is just the page_size
//     On Windows: this is what known as the page granularity.
uintptr_t hcc_virt_mem_reserve_align();

//
// reserve a range of the virtual address space that has the physical pages of memory committed.
//
// WARNING: on Windows, you cannot release sub sections of the address space.
//          you can only release the full reserved address space that is issued by this function call.
//          there are also restriction on protection, see hcc_virt_mem_protection_set.
//
// @param(requested_addr): the requested start address you wish to reserve.
//     be a aligned to the reserve_align that is retrieved from hcc_virt_mem_page_size function.
//     this is not guaranteed and is only used as hint.
//     NULL will not be used as a hint, instead the OS will choose an address for you.
//
// @param(size): the size in bytes you wish to reserve from the @param(requested_addr)
//     must be a multiple of the reserve_align that is retrieved from hcc_virt_mem_page_size function.
//
// @param(addr_out) a pointer to a value that is set to the start of the reserved block of memory
//     when this function returns successfully.
//
void hcc_virt_mem_reserve_commit(HccAllocTag tag, void* requested_addr, uintptr_t size, HccVirtMemProtection protection, void** addr_out);

//
// reserve a range of the virtual address space but does not commit any physical pages of memory.
// none of this memory cannot be used until hcc_virt_mem_commit is called.
//
// WARNING: on Windows, you cannot release sub sections of the address space.
//          you can only release the full reserved address space that is issued by this function call.
//          there are also restriction on protection, see hcc_virt_mem_protection_set.
//
// @param(requested_addr): the requested start address you wish to reserve.
//     be a aligned to the reserve_align that is retrieved from hcc_virt_mem_page_size function.
//     this is not guaranteed and is only used as hint.
//     NULL will not be used as a hint, instead the OS will choose an address for you.
//
// @param(size): the size in bytes you wish to reserve from the @param(requested_addr)
//     must be a multiple of the reserve_align that is retrieved from hcc_virt_mem_page_size function.
//
// @param(addr_out) a pointer to a value that is set to the start of the reserved block of memory
//     when this function returns successfully.
//
void hcc_virt_mem_reserve(HccAllocTag tag, void* requested_addr, uintptr_t size, void** addr_out);

//
// requests the OS to commit physical pages of memory to the the address space.
// this address space must be a full or subsection of the reserved address space with hcc_virt_mem_reserve.
// the memory in the commited address space will be zeroed after calling this function.
//
// @param(addr): the start address of the memory you wish to commit.
//     must be a aligned to the page size hcc_virt_mem_page_size returns.
//     this is not guaranteed and is only used as hint.
//     NULL will not be used as a hint.
//
// @param(size): the size in bytes you wish to reserve from the @param(addr)
//     must be a aligned to the page size hcc_virt_mem_page_size returns.
//
// @param(protection): what the memory is allowed to be used for
//
void hcc_virt_mem_commit(HccAllocTag tag, void* addr, uintptr_t size, HccVirtMemProtection protection);

//
// change the protection of a range of memory.
// this memory must have been reserved with hcc_virt_mem_reserve.
//
// WARNING: on Windows, you can change protection of any number pages
//          but that they all must come from the same call they where reserved with.
//
// @param(addr): the start of the pages you wish to change the protection for.
//             must be a aligned to the page size hcc_virt_mem_page_size returns.
//
// @param(size): the size in bytes of the memory you wish to change protection for.
//             must be a aligned to the page size hcc_virt_mem_page_size returns.
//
// @param(protection): what the memory is allowed to be used for
//
void hcc_virt_mem_protection_set(HccAllocTag tag, void* addr, uintptr_t size, HccVirtMemProtection protection);

//
// gives the memory back to the OS but will keep the address space reserved
//
// @param(addr): the start of the pages you wish to decommit.
//             must be a aligned to the page size hcc_virt_mem_page_size returns.
//
// @param(size): the size in bytes of the memory you wish to release.
//             must be a aligned to the page size hcc_virt_mem_page_size returns.
//
void hcc_virt_mem_decommit(HccAllocTag tag, void* addr, uintptr_t size);

//
// gives the reserved pages back to the OS. the address range must have be reserved with hcc_virt_mem_reserve.
// all commit pages in the released address space are automatically decommit when you release.
//
// on non Windows systems only:
//     you can target sub pages of the original allocation but just make sure the parameters are aligned.
//
// WARNING: on Windows, you cannot release sub sections of the address space.
//          you can only release the full reserved address space that is issued by hcc_virt_mem_reserve.
//          so @param(size) is ignored on Windows.
//
// @param(addr): the start of the pages you wish to release.
//             must be a aligned to the page size hcc_virt_mem_page_size returns.
//
// @param(size): the size in bytes of the memory you wish to release.
//             must be a aligned to the page size hcc_virt_mem_page_size returns.
//
void hcc_virt_mem_release(HccAllocTag tag, void* addr, uintptr_t size);

//
// gives the physical memory pages back to the OS but will keep the address space allocated.
// this will give you zeroed memory when the target pages in this address space are next accessed.
//
// @param(addr): the start of the pages you wish to reset.
//               must be a aligned to the return value of the hcc_virt_mem_page_size function.
//
// @param(size): the size in bytes of the memory you wish to reset.
//               must be a aligned to the return value of the hcc_virt_mem_page_size function.
//
void hcc_virt_mem_reset(HccAllocTag tag, void* addr, uintptr_t size);

//
// after hcc_virt_mem_alloc or hcc_virt_mem_reset, the target address space will not yet have
// any physical pages assignd to the virtual pages. these will get assigned when
// they are first accessed.
//
// this function accesses all pages in the specified range so that the OS assigns physical pages
// all at once. this will help prevent the system switching back to the kernel when you first
// access these virtual pages when the CPU is right in the middle of performing a task.
//
// @param(addr): the start of the pages you wish to be physically allocated.
//               must be a aligned to the return value of the hcc_virt_mem_page_size function.
//
// @param(size): the size in bytes of the memory you wish to be physically allocated.
//               must be a aligned to the return value of the hcc_virt_mem_page_size function.
//
void hcc_virt_mem_force_physical_page_allocation(void* addr, uintptr_t size);

//
// creates a magic ring buffer using virtual memory. the ring buffer will be of @param(size)
// in bytes in physical memory but take up twice the address space. we use virtual memory to map/mirror
// the same physical memory again directly after it finishes in the address space.
// this allows you to memcpy without worrying if you reach the end of the buffer. when you move
// the read/write cursor off the buffer, just use index % buffer_size to wrap the index back around.
// that way the index will always remain in the original virtual pages and the copying can then
// spill over into the mirrored virtual pages.
//
// @param(requested_addr): the requested start address you wish to use for the ring buffer.
//     must be a aligned to the reserve_align that is retrieved from hcc_virt_mem_alloc_align function.
//     this is not guaranteed and is only used as hint.
//     NULL will not be used as a hint, instead the OS will choose an address for you.
//
// @param(size): the size in bytes you wish to the ring buffer to be.
//     address space from @param(addr) to @param(addr) + @param(size) * 2 will be used.
//     must be a aligned to the page size hcc_virt_mem_page_size returns.
//
// @param(addr_out) a pointer to a value that is set to the start of the magic ring buffer
//     when this function returns successfully.
//
void hcc_virt_mem_magic_ring_buffer_alloc(HccAllocTag tag, void* requested_addr, uintptr_t size, void** addr_out);

//
// deallocates the magic ring buffer.
//
// @param(addr): the address that was returned when allocating the magic ring buffer
//
// @param(size): the size that was used when allocating the magic ring buffer.
//
void hcc_virt_mem_magic_ring_buffer_dealloc(HccAllocTag tag, void* addr, uintptr_t size);

// ===========================================
//
//
// String
//
//
// ===========================================

#define hcc_string_eq(a, b) ((a).size == (b).size && memcmp((a).data, (b).data, (a).size) == 0)
#define hcc_string_eq_c(a, c_string) ((a).size == strlen(c_string) && memcmp((a).data, c_string, (a).size) == 0)
#define hcc_string_eq_lit(a, lit) ((a).size == sizeof(lit) - 1 && memcmp((a).data, lit, (a).size) == 0)
static inline HccString hcc_string_slice_start(HccString string, uintptr_t start) {
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(start, string.size + 1);
	return hcc_string(string.data + start, string.size - start);
}
static inline HccString hcc_string_slice_end(HccString string, uintptr_t end) {
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(end, string.size + 1);
	return hcc_string(string.data, end);
}
static inline HccString hcc_string_slice(HccString string, uintptr_t start, uintptr_t end) {
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(start, string.size + 1);
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(end, string.size + 1);
	HCC_DEBUG_ASSERT(start <= end, "start of '%zu' must be less than end of '%zu'", start, end);
	return hcc_string(string.data + start, end - start);
}

bool hcc_string_key_cmp(void* a, void* b, uintptr_t size);

// ===========================================
//
//
// Hashing
//
//
// ===========================================

typedef uint32_t HccHash32;
typedef uint64_t HccHash64;
typedef uintptr_t HccHash;

#define HCC_HASH_FNV_32_INIT 0x811c9dc5
#define HCC_HASH_FNV_64_INIT 0xcbf29ce484222325

typedef HccHash32 (*HccHash32Fn)(void* data, HccHash32 hash);
typedef HccHash64 (*HccHash64Fn)(void* data, HccHash64 hash);

#if UINTPTR_MAX == 0xFFFFFFFF
#define HccHashFn HccHash32Fn
#define hcc_hash_fnv hcc_hash_fnv_32
#define HCC_HASH_FNV_INIT HCC_HASH_FNV_32_INIT
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define HccHashFn HccHash64Fn
#define hcc_hash_fnv hcc_hash_fnv_64
#define HCC_HASH_FNV_INIT HCC_HASH_FNV_64_INIT
#else
#error "unsupported platform"
#endif

HccHash32 hcc_hash_fnv_32(const void* data, uintptr_t size, HccHash32 hash);
HccHash64 hcc_hash_fnv_64(const void* data, uintptr_t size, HccHash64 hash);

void hcc_generate_enum_hashes(char* array_name, char** strings, char** enum_strings, uint32_t enums_count);
void hcc_generate_hashes();
uint32_t hcc_string_to_enum_hashed_find(HccString string, HccHash32* enum_hashes, uint32_t enums_count);

// ===========================================
//
//
// Stack
//
//
// ===========================================

#define HCC_STACK_MAGIC_NUMBER 0x57ac4c71

typedef struct HccStackHeader HccStackHeader;
struct HccStackHeader {
	HccAtomic(uintptr_t) count;
	HccAtomic(uintptr_t) cap;
	uintptr_t            grow_count;
	uintptr_t            reserve_cap;
	HccAllocTag          tag;
	HccSpinMutex         push_thread_safe_mutex;
#if HCC_ENABLE_DEBUG_ASSERTIONS
	uint32_t             magic_number;
	uintptr_t            elmt_size;
#endif
};

#define HccStack(T) T*

#define hcc_stack_header(stack)      ((stack) ? (((HccStackHeader*)(stack)) - 1)     : NULL)
#define hcc_stack_count(stack)       ((stack) ? atomic_load(&hcc_stack_header(stack)->count) : 0)
#define hcc_stack_cap(stack)         ((stack) ? atomic_load(&hcc_stack_header(stack)->cap)   : 0)
#define hcc_stack_grow_count(stack)  ((stack) ? hcc_stack_header(stack)->grow_count  : 0)
#define hcc_stack_reserve_cap(stack) ((stack) ? hcc_stack_header(stack)->reserve_cap : 0)
#define hcc_stack_clear(stack)       ((stack) ? (atomic_store(&hcc_stack_header(stack)->count, 0), 0) : 0)
#define hcc_stack_is_full(stack)     (hcc_stack_count(stack) == hcc_stack_cap(stack))

#define hcc_stack_init(T, tag, grow_count, reserve_cap) ((HccStack(T))_hcc_stack_init(tag, grow_count, reserve_cap, sizeof(T)))
HccStack(void) _hcc_stack_init(HccAllocTag tag, uintptr_t grow_count, uintptr_t reserve_cap, uintptr_t elmt_size);

#define hcc_stack_deinit(stack) _hcc_stack_deinit(stack, sizeof(*(stack))); (stack) = NULL
void _hcc_stack_deinit(HccStack(void) stack, uintptr_t elmt_size);

#if HCC_ENABLE_DEBUG_ASSERTIONS
#define hcc_stack_get(stack, idx) (&(stack)[HCC_DEBUG_ASSERT_ARRAY_BOUNDS(idx, hcc_stack_count(stack))])
#else
#define hcc_stack_get(stack, idx) (&(stack)[idx])
#endif
#define hcc_stack_get_first(stack) hcc_stack_get(stack, 0)
#define hcc_stack_get_last(stack) hcc_stack_get(stack, hcc_stack_count(stack) - 1)
#define hcc_stack_get_back(stack, back_idx) hcc_stack_get(stack, hcc_stack_count(stack) - (back_idx) - 1)
#define hcc_stack_get_or_null(stack, idx) ((idx) < hcc_stack_count(stack) ? &(stack)[idx] : NULL)
#define hcc_stack_get_or_value(stack, idx, value) ((idx) < hcc_stack_count(stack) ? (stack)[idx] : (value))
#define hcc_stack_get_next_push(stack) (&(stack)[hcc_stack_count(stack)])

#define hcc_stack_resize(stack, new_count) _hcc_stack_resize(stack, new_count, sizeof(*(stack)))
uintptr_t _hcc_stack_resize(HccStack(void) stack, uintptr_t new_count, uintptr_t elmt_size);

#define hcc_stack_insert(stack, idx) _hcc_stack_insert_many(stack, idx, 1, sizeof(*(stack)))
#define hcc_stack_insert_many(stack, idx, amount) _hcc_stack_insert_many(stack, idx, amount, sizeof(*(stack)))
void* _hcc_stack_insert_many(HccStack(void) stack, uintptr_t idx, uintptr_t amount, uintptr_t elmt_size);

#define hcc_stack_push(stack) (&(stack)[_hcc_stack_push_many(stack, 1, sizeof(*(stack)))])
#define hcc_stack_push_many(stack, amount) (&(stack)[_hcc_stack_push_many(stack, amount, sizeof(*(stack)))])
uintptr_t _hcc_stack_push_many(HccStack(void) stack, uintptr_t amount, uintptr_t elmt_size);

#define hcc_stack_push_thread_safe(stack) (&(stack)[_hcc_stack_push_many_thread_safe(stack, 1, sizeof(*(stack)))])
#define hcc_stack_push_many_thread_safe(stack, amount) (&(stack)[_hcc_stack_push_many_thread_safe(stack, amount, sizeof(*(stack)))])
uintptr_t _hcc_stack_push_many_thread_safe(HccStack(void) stack, uintptr_t amount, uintptr_t elmt_size);

#define hcc_stack_pop(stack) _hcc_stack_pop_many(stack, 1, sizeof(*(stack)))
#define hcc_stack_pop_many(stack, amount) _hcc_stack_pop_many(stack, amount, sizeof(*(stack)))
void _hcc_stack_pop_many(HccStack(void) stack, uintptr_t amount, uintptr_t elmt_size);

#define hcc_stack_remove_swap(stack, idx) _hcc_stack_remove_swap_many(stack, idx, 1, sizeof(*(stack)))
#define hcc_stack_remove_swap_many(stack, idx, count) _hcc_stack_remove_swap_many(stack, idx, count, sizeof(*(stack)))
void _hcc_stack_remove_swap_many(HccStack(void) stack, uintptr_t idx, uintptr_t amount, uintptr_t elmt_size);

#define hcc_stack_remove_shift(stack, idx) _hcc_stack_remove_shift_many(stack, idx, 1, sizeof(*(stack)))
#define hcc_stack_remove_shift_many(stack, idx, count) _hcc_stack_remove_shift_many(stack, idx, count, sizeof(*(stack)))
void _hcc_stack_remove_shift_many(HccStack(void) stack, uintptr_t idx, uintptr_t amount, uintptr_t elmt_size);

void hcc_stack_push_char(HccStack(char) stack, char ch);
HccString hcc_stack_push_string(HccStack(char) stack, HccString string);
HccString hcc_stack_push_string_fmtv(HccStack(char) stack, char* fmt, va_list args);
#ifdef __GNUC__
HccString hcc_stack_push_string_fmt(HccStack(char) stack, char* fmt, ...) __attribute__ ((format (printf, 2, 3)));
#else
HccString hcc_stack_push_string_fmt(HccStack(char) stack, char* fmt, ...);
#endif

// ===========================================
//
//
// Deque
//
//
// ===========================================

#define HCC_DEQUE_MAGIC_NUMBER 0x4ec31f5c

#define HCC_WRAPPING_ADD(a, b, cap_that_is_power_of_two) (((a) + (b)) & ((cap_that_is_power_of_two) - 1))
#define HCC_WRAPPING_SUB(a, b, cap_that_is_power_of_two) (((a) - (b)) & ((cap_that_is_power_of_two) - 1))

#define HccDeque(T) T*

typedef struct HccDequeHeader HccDequeHeader;
struct HccDequeHeader {
	alignas(hcc_max_align_t)
	uintptr_t   cap;
	uintptr_t   front_idx;
	uintptr_t   back_idx;
	HccAllocTag tag;
#if HCC_ENABLE_DEBUG_ASSERTIONS
	uint32_t    magic_number;
	uintptr_t   elmt_size;
#endif
};

#define hcc_deque_header(deque)     ((deque) ? (((HccDequeHeader*)(deque)) - 1)   : NULL)
#define hcc_deque_front_idx(deque)  ((deque) ? hcc_deque_header(deque)->front_idx : 0)
#define hcc_deque_back_idx(deque)   ((deque) ? hcc_deque_header(deque)->back_idx  : 0)
#define hcc_deque_is_empty(deque)   ((deque) ? hcc_deque_header(deque)->front_idx == hcc_deque_header(deque)->back_idx : true)
#define hcc_deque_count(deque) \
	(deque \
		? hcc_deque_header(deque)->back_idx >= hcc_deque_header(deque)->front_idx \
			? hcc_deque_header(deque)->back_idx - hcc_deque_header(deque)->front_idx \
			: hcc_deque_header(deque)->back_idx + (hcc_deque_header(deque)->cap - hcc_deque_header(deque)->front_idx) \
		: 0 \
	)
#define hcc_deque_cap(deque)    ((deque) ? hcc_deque_header(deque)->cap       : 0)
#define hcc_deque_clear(deque)  ((deque) ? hcc_deque_header(deque)->front_idx = hcc_deque_header(deque)->back_idx : 0);

#define hcc_deque_init(T, cap, tag) _hcc_deque_init(tag, cap, sizeof(T))
HccDeque(void) _hcc_deque_init(HccAllocTag tag, uintptr_t cap, uintptr_t elmt_size);

#define hcc_deque_deinit(deque) _hcc_deque_deinit(deque, sizeof(*(deque))); (deque) = NULL
void _hcc_deque_deinit(HccDeque(void) deque, uintptr_t elmt_size);

#if HCC_ENABLE_DEBUG_ASSERTIONS
#define hcc_deque_get(deque, idx) (&(deque)[HCC_WRAPPING_ADD(hcc_deque_front_idx(deque), _HCC_ASSERT_ARRAY_BOUNDS(idx, hcc_deque_count(deque)), hcc_deque_cap(deque))])
#else
#define hcc_deque_get(deque, idx) (&(deque)[HCC_WRAPPING_ADD(hcc_deque_front_idx(deque), idx, hcc_deque_cap(deque))])
#endif
#define hcc_deque_get_first(deque) hcc_deque_get(deque, 0)
#define hcc_deque_get_last(deque) hcc_deque_get(deque, hcc_deque_count(deque) - 1)

#define hcc_deque_read(deque, idx, count, elmts_out) _hcc_deque_read(deque, idx, count, elmts_out, sizeof(*(deque)))
void _hcc_deque_read(HccDeque(void) deque, uintptr_t idx, uintptr_t amount, void* elmts_out, uintptr_t elmt_size);

#define hcc_deque_write(deque, idx, elmts, count) _hcc_deque_write(deque, idx, elmts, count, sizeof(*(deque)))
void _hcc_deque_write(HccDeque(void) deque, uintptr_t idx, void* elmts, uintptr_t amount, uintptr_t elmt_size);

#define hcc_deque_push_front(deque) _hcc_deque_push_front_many(deque, 1, sizeof(*(deque)))
#define hcc_deque_push_front_many(deque, count) _hcc_deque_push_front_many(deque, count, sizeof(*(deque)))
uintptr_t _hcc_deque_push_front_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size);

#define hcc_deque_push_back(deque) _hcc_deque_push_back_many(deque, 1, sizeof(*(deque)))
#define hcc_deque_push_back_many(deque, count) _hcc_deque_push_back_many(deque, count, sizeof(*(deque)))
uintptr_t _hcc_deque_push_back_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size);

#define hcc_deque_pop_front(deque) _hcc_deque_pop_front_many(deque, 1, sizeof(*(deque)))
#define hcc_deque_pop_front_many(deque, count) _hcc_deque_pop_front_many(deque, count, sizeof(*(deque)))
void _hcc_deque_pop_front_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size);

#define hcc_deque_pop_back(deque) _hcc_deque_pop_back_many(deque, 1, sizeof(*(deque)))
#define hcc_deque_pop_back_many(deque, count) _hcc_deque_pop_back_many(deque, count, sizeof(*(deque)))
void _hcc_deque_pop_back_many(HccDeque(void) deque, uintptr_t amount, uintptr_t elmt_size);

// ===========================================
//
//
// Hash Table
//
//
// ===========================================

#define HCC_HASH_TABLE_MAGIC_NUMBER 0x0b7ec7b7

typedef bool (*HccHashTableKeyCmpFn)(void* a, void* b, uintptr_t size);
typedef HccHash (*HccHashTableKeyHashFn)(void* key, uintptr_t size);

typedef struct HccHashTableHeader HccHashTableHeader;
struct HccHashTableHeader {
	alignas(hcc_max_align_t)
	HccAtomic(uintptr_t) count;
	uintptr_t            cap;
	HccHashTableKeyCmpFn  key_cmp_fn;
	HccHashTableKeyHashFn key_hash_fn;
	HccAtomic(HccHash)*  hashes;
	HccAllocTag          tag;
#if HCC_ENABLE_DEBUG_ASSERTIONS
	uint32_t             magic_number;
	uintptr_t            elmt_size;
#endif
};

typedef struct HccHashTableInsert HccHashTableInsert;
struct HccHashTableInsert {
	uintptr_t idx;
	bool is_new;
};

typedef struct HccNameToIdxEntry HccNameToIdxEntry;
struct HccNameToIdxEntry {
	HccString key;
	uintptr_t value;
};

#define HccHashTable(KVEntry) KVEntry*

#define hcc_hash_table_header(table) ((table) ? (((HccHashTableHeader*)(table)) - 1) : NULL)
#define hcc_hash_table_count(table)  ((table) ? hcc_hash_table_header(table)->count  : 0)
#define hcc_hash_table_cap(table)    ((table) ? hcc_hash_table_header(table)->cap    : 0)

#if HCC_ENABLE_DEBUG_ASSERTIONS
#define hcc_hash_table_get(table, idx) (&(table)[_HCC_ASSERT_ARRAY_BOUNDS(idx, hcc_hash_table_cap(table))])
#else
#define hcc_hash_table_get(table, idx) (&(table)[idx])
#endif

#define hcc_hash_table_init(KVEntry, tag, key_cmp_fn, key_hash_fn, cap) _hcc_hash_table_init(tag, key_cmp_fn, key_hash_fn, cap, sizeof(KVEntry))
HccHashTable(void) _hcc_hash_table_init(HccAllocTag tag, HccHashTableKeyCmpFn key_cmp_fn, HccHashTableKeyHashFn key_hash_fn, uintptr_t cap, uintptr_t elmt_size);

#define hcc_hash_table_deinit(table) _hcc_hash_table_deinit(table, sizeof(*(table)))
void _hcc_hash_table_deinit(HccHashTable(void) table, uintptr_t elmt_size);

#define hcc_hash_table_clear(table) _hcc_hash_table_clear(table, sizeof(*(table)))
void _hcc_hash_table_clear(HccHashTable(void) table, uintptr_t elmt_size);

#define hcc_hash_table_find_idx(table, key) _hcc_hash_table_find_idx(table, key, sizeof(*(key)), sizeof(*(table)))
uintptr_t _hcc_hash_table_find_idx(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size);

#define hcc_hash_table_find_insert_idx(table, key) _hcc_hash_table_find_insert_idx(table, key, sizeof(*(key)), sizeof(*(table)))
HccHashTableInsert _hcc_hash_table_find_insert_idx(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size);

#define hcc_hash_table_remove(table, key) _hcc_hash_table_remove(table, key, sizeof(*(key)), sizeof(*(table)))
bool _hcc_hash_table_remove(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size);

bool hcc_u32_key_cmp(void* a, void* b, uintptr_t size);
bool hcc_u64_key_cmp(void* a, void* b, uintptr_t size);

HccHash hcc_data_key_hash(void* key, uintptr_t size);
HccHash hcc_string_key_hash(void* key, uintptr_t size);
HccHash hcc_u32_key_hash(void* key, uintptr_t size);
HccHash hcc_u64_key_hash(void* key, uintptr_t size);

// ===========================================
//
//
// Data Type Table
//
//
// ===========================================

typedef struct HccArrayDataType HccArrayDataType;
struct HccArrayDataType {
	HccDataType element_data_type;
	HccConstantId element_count_constant_id;
	uint32_t has_pointer: 1;
	uint32_t has_resource: 1;
};

//
// this is either:
// - HCC_RESOURCE_DATA_TYPE_CONSTBUFFER
// - HCC_RESOURCE_DATA_TYPE_BUFFER
typedef struct HccBufferDataType HccBufferDataType;
struct HccBufferDataType {
	HccDataType element_data_type;
};

typedef struct HccPointerDataType HccPointerDataType;
struct HccPointerDataType {
	HccDataType element_data_type;
};

typedef struct HccFunctionDataType HccFunctionDataType;
struct HccFunctionDataType {
	HccDataType  return_data_type;
	uint32_t     params_count;
	HccDataType* params;
};

typedef uint16_t HccCompoundDataTypeFlags;
enum HccCompoundDataTypeFlags {
	HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION =     0x1,
	HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_RESOURCE = 0x2,
	HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_POINTER =  0x4,
	HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_UNION =    0x8,
};

typedef uint8_t HccCompoundDataTypeKind;
enum HccCompoundDataTypeKind {
	HCC_COMPOUND_DATA_TYPE_KIND_DEFAULT,
	HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE,
	HCC_COMPOUND_DATA_TYPE_KIND_FRAGMENT_STATE,
	HCC_COMPOUND_DATA_TYPE_KIND_BUFFER_ELEMENT,
	HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_SET,
	HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_TABLE,
	HCC_COMPOUND_DATA_TYPE_KIND_RESOURCES,
};

typedef uint16_t HccRasterizerStateFieldKind;
enum HccRasterizerStateFieldKind {
	HCC_RASTERIZER_STATE_FIELD_KIND_INTERP,
	HCC_RASTERIZER_STATE_FIELD_KIND_POSITION,
	HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP,
};

typedef struct HccCompoundDataType HccCompoundDataType;
struct HccCompoundDataType {
	uint64_t                  size;
	uint64_t                  align;
	HccLocation*              identifier_location;
	HccStringId               identifier_string_id;
	HccCompoundField*         fields;
	uint16_t                  fields_count;
	uint16_t                  largest_sized_field_idx;
	HccAMLScalarDataTypeMask  scalar_data_types_mask;
	HccCompoundDataTypeFlags  flags;
	HccCompoundDataTypeKind   kind;
	uint8_t                   resource_set_slot;
	HccHash64                 field_data_types_hash;
	HccHash64                 field_identifiers_hash;

};

typedef struct HccCompoundField HccCompoundField;
struct HccCompoundField {
	HccLocation*                identifier_location;
	HccStringId                 identifier_string_id;
	HccDataType                 data_type;
	HccRasterizerStateFieldKind rasterizer_state_field_kind;
};

typedef struct HccEnumDataType HccEnumDataType;
struct HccEnumDataType {
	HccLocation*  identifier_location;
	HccEnumValue* values;
	uint32_t      values_count;
	HccStringId   identifier_string_id;

	uint64_t      value_identifiers_hash;
	uint64_t      values_hash;
};

typedef struct HccEnumValue HccEnumValue;
struct HccEnumValue {
	HccLocation*  identifier_location;
	HccStringId   identifier_string_id;
	HccConstantId constant_id;
};

typedef struct HccTypedef HccTypedef;
struct HccTypedef {
	HccLocation* identifier_location;
	HccStringId  identifier_string_id;
	HccDataType  aliased_data_type;
};

typedef struct HccDataTypeDedupEntry HccDataTypeDedupEntry;
struct HccDataTypeDedupEntry {
	uint64_t            key;
	HccAtomic(uint32_t) id;
};

typedef struct HccDataTypeTable HccDataTypeTable;
struct HccDataTypeTable {
	uint8_t*                      basic_type_size_and_aligns; // [HCC_AST_BASIC_DATA_TYPE_COUNT]
	uint64_t*                     basic_type_int_mins; // [HCC_AST_BASIC_DATA_TYPE_COUNT]
	uint64_t*                     basic_type_int_maxes; // [HCC_AST_BASIC_DATA_TYPE_COUNT]
	HccStack(HccArrayDataType)    arrays;
	HccStack(HccCompoundDataType) compounds;
	HccStack(HccCompoundField)    compound_fields;
	HccStack(HccTypedef)          typedefs;
	HccStack(HccEnumDataType)     enums;
	HccStack(HccEnumValue)        enum_values;
	HccStack(HccPointerDataType)  pointers;
	HccStack(HccFunctionDataType) functions;
	HccStack(HccDataType)         function_params;
	HccStack(HccBufferDataType)   buffers;
	HccHashTable(HccDataTypeDedupEntry) arrays_dedup_hash_table;
	HccHashTable(HccDataTypeDedupEntry) pointers_dedup_hash_table;
	HccHashTable(HccDataTypeDedupEntry) functions_dedup_hash_table;
	HccHashTable(HccDataTypeDedupEntry) buffers_dedup_hash_table;
};

void hcc_data_type_table_init(HccCU* cu, HccCUSetup* setup);
void hcc_data_type_table_deinit(HccCU* cu);

// ===========================================
//
//
// Constant
//
//
// ===========================================

uint8_t hcc_constant_read_8(HccConstant constant);
uint16_t hcc_constant_read_16(HccConstant constant);
uint32_t hcc_constant_read_32(HccConstant constant);
uint64_t hcc_constant_read_64(HccConstant constant);
bool hcc_constant_read_int_extend_64(HccCU* cu, HccConstant constant, uint64_t* out);

// ===========================================
//
//
// Constant Table
//
//
// ===========================================
//
// used to hold a unique set of constants so the user
// can reduce the constant down to a uint32_t identifier.
// this identifier can be used to uniquely identify the constant.
// so a comparision with the two identifier integers can be used to compare constants.
//

typedef struct HccConstantEntry HccConstantEntry;
struct HccConstantEntry {
	void*                  data;
	uint32_t               size;
	HccAtomic(HccDataType) data_type;
};

typedef struct HccConstantTable HccConstantTable;
struct HccConstantTable {
	HccHashTable(HccConstantEntry) entries_hash_table;
	HccStack(uint8_t)                 data;
	HccStack(HccConstantId)        composite_fields_buffer;
};

bool hcc_constant_entry_key_cmp(void* a, void* b, uintptr_t size);

void hcc_constant_table_init(HccCU* cu, HccConstantTableSetup* setup);
void hcc_constant_table_deinit(HccCU* cu);
HccConstantId _hcc_constant_table_deduplicate_end(HccCU* cu, HccDataType data_type, void* data, uint32_t data_size, uint32_t data_align);

// ===========================================
//
//
// AST Declarations
//
//
// ===========================================

typedef struct HccASTVariable HccASTVariable;
struct HccASTVariable {
	HccASTFile*           ast_file;
	HccLocation*          identifier_location;
	HccStringId           identifier_string_id;
	HccDataType           data_type;
	HccConstantId         initializer_constant_id;
	HccASTLinkage         linkage;
	HccASTStorageDuration storage_duration;
	uint32_t              linkage_has_been_explicitly_declared: 1;
};

typedef uint8_t HccASTFunctionFlags;
enum HccASTFunctionFlags {
	HCC_AST_FUNCTION_FLAGS_INLINE = 0x1,
};

#define HCC_FUNCTION_MAX_PARAMS_COUNT 32
#define HCC_FUNCTION_CALL_STACK_CAP 256
#define HCC_FUNCTION_UNIQUE_GLOBALS_CAP 256

typedef struct HccASTFunction HccASTFunction;
struct HccASTFunction {
	HccLocation*        identifier_location;
	HccLocation*        return_data_type_location;
	HccASTVariable*     params_and_variables;
	HccStringId         identifier_string_id;
	HccDataType         function_data_type;
	HccDataType         return_data_type;
	uint16_t            variables_count;
	uint8_t             params_count;
	HccASTLinkage       linkage;
	HccASTFunctionFlags flags;
	HccShaderStage      shader_stage;
	HccOptLevel         opt_level;
	HccASTExpr*         block_expr;
	uint32_t            max_instrs_count;
};

enum {
	HCC_VERTEX_SHADER_PARAM_VERTEX_SV,
	HCC_VERTEX_SHADER_PARAM_RASTERIZER_STATE,
};

enum {
	HCC_FRAGMENT_SHADER_PARAM_FRAGMENT_SV,
	HCC_FRAGMENT_SHADER_PARAM_RASTERIZER_STATE,
	HCC_FRAGMENT_SHADER_PARAM_FRAGMENT_STATE,
};

enum {
	HCC_VERTEX_SV_VERTEX_IDX,
	HCC_VERTEX_SV_INSTANCE_IDX,
};

enum {
	HCC_FRAGMENT_SV_FRAG_COORD,
};

// ===========================================
//
//
// ATA Token Cursor
//
//
// ===========================================

typedef struct HccATATokenCursor HccATATokenCursor;
struct HccATATokenCursor {
	uint32_t tokens_start_idx;
	uint32_t tokens_end_idx;
	uint32_t token_idx;
	uint32_t token_value_idx;
};

uint32_t hcc_ata_token_cursor_tokens_count(HccATATokenCursor* cursor);

// ===========================================
//
//
// ATA Token Bag
//
//
// ===========================================

#define HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG(location) (((uintptr_t)location) & 0x1)
#define HCC_PP_TOKEN_SET_PREEXPANDED_MACRO_ARG(location) ((HccLocation*)(((uintptr_t)location) | 0x1))
#define HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(location) ((HccLocation*)(((uintptr_t)location) & ~(uintptr_t)0x1))

typedef struct HccATATokenBag HccATATokenBag;
struct HccATATokenBag {
	HccStack(HccATAToken)  tokens;
	HccStack(HccLocation*) locations;
	HccStack(HccATAValue)  values;
};

void hcc_ata_token_bag_init(HccATATokenBag* bag, uint32_t tokens_grow_count, uint32_t tokens_reserve_cap, uint32_t values_grow_count, uint32_t values_reserve_cap);
void hcc_ata_token_bag_deinit(HccATATokenBag* bag);
void hcc_ata_token_bag_reset(HccATATokenBag* bag);
void hcc_ata_token_bag_push_token(HccATATokenBag* bag, HccATAToken token, HccLocation* location);
void hcc_ata_token_bag_push_value(HccATATokenBag* bag, HccATAValue value);
bool hcc_ata_token_bag_pop_token(HccATATokenBag* bag);
uint32_t hcc_ata_token_bag_stringify_single(HccATATokenBag* bag, HccATATokenCursor* cursor, HccPPMacro* macro, char* outbuf, uint32_t outbufsize);
HccATAToken hcc_ata_token_bag_stringify_single_or_macro_param(HccWorker* w, HccATATokenBag* bag, HccATATokenCursor* cursor, uint32_t args_start_idx, HccATATokenBag* args_src_bag, bool false_before_true_after, char* outbuf, uint32_t outbufsize, uint32_t* outbufidx_out);
HccStringId hcc_ata_token_bag_stringify_range(HccATATokenBag* bag, HccATATokenCursor* cursor, HccPPMacro* macro, char* outbuf, uint32_t outbufsize, uint32_t* outbufidx_out);

// ===========================================
//
//
// Preprocessor
//
//
// ===========================================

typedef struct HccPPMacro HccPPMacro;
struct HccPPMacro {
	HccStringId       identifier_string_id;
	HccString         identifier_string;
	HccLocation*      location;
	HccStringId*      params;
	HccATATokenCursor token_cursor;
	uint32_t          params_count: 8;
	uint32_t          is_function: 1;
	uint32_t          has_va_args: 1;
};

typedef uint32_t HccPPPredefinedMacro;
enum HccPPPredefinedMacro {
	HCC_PP_PREDEFINED_MACRO___FILE__,
	HCC_PP_PREDEFINED_MACRO___LINE__,
	HCC_PP_PREDEFINED_MACRO___COUNTER__,
	HCC_PP_PREDEFINED_MACRO___HCC__,
	HCC_PP_PREDEFINED_MACRO___HCC_GPU__,
	HCC_PP_PREDEFINED_MACRO___HCC_X86_64__,
	HCC_PP_PREDEFINED_MACRO___HCC_LINUX__,
	HCC_PP_PREDEFINED_MACRO___HCC_WINDOWS__,

	HCC_PP_PREDEFINED_MACRO_COUNT,
};

typedef uint8_t HccPPDirective;
enum HccPPDirective {
	HCC_PP_DIRECTIVE_DEFINE,
	HCC_PP_DIRECTIVE_UNDEF,
	HCC_PP_DIRECTIVE_INCLUDE,
	HCC_PP_DIRECTIVE_IF,
	HCC_PP_DIRECTIVE_IFDEF,
	HCC_PP_DIRECTIVE_IFNDEF,
	HCC_PP_DIRECTIVE_ELSE,
	HCC_PP_DIRECTIVE_ELIF,
	HCC_PP_DIRECTIVE_ELIFDEF,
	HCC_PP_DIRECTIVE_ELIFNDEF,
	HCC_PP_DIRECTIVE_ENDIF,
	HCC_PP_DIRECTIVE_LINE,
	HCC_PP_DIRECTIVE_ERROR,
	HCC_PP_DIRECTIVE_WARNING,
	HCC_PP_DIRECTIVE_PRAGMA,

	HCC_PP_DIRECTIVE_COUNT,
};

//
// this represents the span from any preprocessor conditional (#if, #elif, #else, #endif etc)
// to the next preprocessor conditional.
typedef struct HccPPIfSpan HccPPIfSpan;
struct HccPPIfSpan {
	HccPPDirective directive;
	HccLocation    location;
	uint32_t       parent_id;
	uint32_t       first_id: 31; // a link to the span that is the original #if/n/def
	uint32_t       has_else: 1;
	uint32_t       prev_id;
	uint32_t       next_id;
	uint32_t       last_id; // set when this is the original #if/n/def to link to the matching #endif
};

typedef struct HccPPMacroArg HccPPMacroArg;
struct HccPPMacroArg {
	HccATATokenCursor cursor;
	HccLocation*   callsite_location;
};

typedef struct HccPPExpand HccPPExpand;
struct HccPPExpand {
	HccPPMacro*    macro;
	HccATATokenCursor cursor;
};

extern const char* hcc_pp_predefined_macro_identifier_strings[HCC_PP_PREDEFINED_MACRO_COUNT];
extern const char* hcc_pp_directive_enum_strings[HCC_PP_DIRECTIVE_COUNT];
extern const char* hcc_pp_directive_strings[HCC_PP_DIRECTIVE_COUNT];
extern uint32_t hcc_pp_directive_hashes[HCC_PP_DIRECTIVE_COUNT];

// ===========================================
//
//
// ATA Iterator
//
//
// ===========================================

typedef struct HccATAIter HccATAIter;
struct HccATAIter {
	HccATAToken*  tokens;
	HccLocation** locations;
	HccATAValue*  values;
	uint32_t      token_idx;
	uint32_t      value_idx;
	uint32_t      tokens_count;
	uint32_t      values_count;
};

// ===========================================
//
//
// Code File
//
//
// ===========================================

typedef uint16_t HccCodeFileFlags;
enum HccCodeFileFlags {
	HCC_CODE_FILE_FLAGS_IS_LOADED =                0x1,
	HCC_CODE_FILE_FLAGS_HAS_STARTED_BEING_PARSED = 0x2,
	HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS =   0x4,
	HCC_CODE_FILE_FLAGS_IS_MACRO_PASTE_BUFFER =    0x8,
};

typedef struct HccCodeFile HccCodeFile;
struct HccCodeFile {
	HccAtomic(HccCodeFileFlags) flags;
	HccString                   path_string;
	HccString                   code;
	HccStack(uint32_t)          line_code_start_indices;
	HccStack(HccPPIfSpan)       pp_if_spans;
};

HccResult hcc_code_file_init(HccCodeFile* code_file, HccString path_string, bool do_not_open_file);
void hcc_code_file_deinit(HccCodeFile* code_file);

// ===========================================
//
//
// Preprocessor Generator
//
//
// ===========================================

typedef struct HccPPGenMacroDeclEntry HccPPGenMacroDeclEntry;
struct HccPPGenMacroDeclEntry {
	HccStringId identifier_string_id;
	uint32_t    macro_idx;
};

//
// this represents the the whole preprocessor if chaing (#if, #ifdef or #ifndef) to #endif
typedef struct HccPPGenIf HccPPGenIf;
struct HccPPGenIf {
	HccLocation    location;
	HccPPDirective directive;
	uint32_t       start_span_id: 31; // a link to the HccPPIfSpan that is the original #if/n/def, used when HccATAGen.we_are_mutator_of_code_file == true
	uint32_t       has_else: 1;
};

typedef struct HccPPGen HccPPGen;
struct HccPPGen {
	HccCodeFile                          concat_buffer_code_file;
	HccStack(HccPPExpand)                expand_stack;
	HccStack(uint32_t)                   expand_macro_idx_stack;
	HccStack(char)                       stringify_buffer;
	HccStack(HccPPGenIf)                 if_stack;
	HccHashTable(HccPPGenMacroDeclEntry) macro_declarations;
	HccStack(HccPPMacroArg)              macro_args_stack;
};

typedef uint8_t HccPPExpandFlags;
enum HccPPExpandFlags {
	HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION = 0x1,
	HCC_PP_EXPAND_FLAGS_IS_ARGS                   = 0x2,
	HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS              = 0x4,
};

typedef struct HccPPEval HccPPEval;
struct HccPPEval {
	HccBasic basic;
	HccDataType data_type;
};

void hcc_ppgen_init(HccWorker* w, HccPPGenSetup* setup);
void hcc_ppgen_deinit(HccWorker* w);
void hcc_ppgen_reset(HccWorker* w);

HccPPIfSpan* hcc_ppgen_if_span_get(HccWorker* w, uint32_t if_span_id);
uint32_t hcc_ppgen_if_span_id(HccWorker* w, HccPPIfSpan* if_span);
HccPPIfSpan* hcc_ppgen_if_span_push(HccWorker* w, HccPPDirective directive);
void hcc_ppgen_if_found_if(HccWorker* w, HccPPDirective directive);
HccPPIfSpan* hcc_ppgen_if_found_if_counterpart(HccWorker* w, HccPPDirective directive);
void hcc_ppgen_if_found_endif(HccWorker* w);
void hcc_ppgen_if_found_else(HccWorker* w, HccPPDirective directive);
void hcc_ppgen_if_ensure_first_else(HccWorker* w, HccPPDirective directive);
void hcc_ppgen_if_ensure_one_is_open(HccWorker* w, HccPPDirective directive);

void hcc_ppgen_eval_binary_op(HccWorker* w, uint32_t* token_idx_mut, HccASTBinaryOp* binary_op_type_out, uint32_t* precedence_out);
HccPPEval hcc_ppgen_eval_unary_expr(HccWorker* w, uint32_t* token_idx_mut, uint32_t* token_value_idx_mut);
HccPPEval hcc_ppgen_eval_expr(HccWorker* w, uint32_t min_precedence, uint32_t* token_idx_mut, uint32_t* token_value_idx_mut);
void hcc_ppgen_ensure_end_of_directive(HccWorker* w, HccErrorCode error_code, HccPPDirective directive);

void hcc_ppgen_parse_define(HccWorker* w);
void hcc_ppgen_parse_undef(HccWorker* w);
void hcc_ppgen_parse_include(HccWorker* w);
bool hcc_ppgen_parse_if(HccWorker* w);
void hcc_ppgen_parse_defined(HccWorker* w);
bool hcc_ppgen_parse_ifdef(HccWorker* w, HccPPDirective directive);
void hcc_ppgen_parse_line(HccWorker* w);
void hcc_ppgen_parse_error(HccWorker* w);
void hcc_ppgen_parse_pragma(HccWorker* w);
HccPPDirective hcc_ppgen_parse_directive_header(HccWorker* w);
void hcc_ppgen_parse_directive(HccWorker* w);
void hcc_ppgen_skip_false_conditional(HccWorker* w, bool is_skipping_until_endif);
void hcc_ppgen_copy_expand_predefined_macro(HccWorker* w, HccPPPredefinedMacro predefined_macro);
void hcc_ppgen_copy_expand_macro_begin(HccWorker* w, HccPPMacro* macro, HccLocation* macro_callsite_location);
bool hcc_ppgen_is_callable_macro(HccWorker* w, HccStringId ident_string_id, uint32_t* macro_idx_out);
HccPPExpand* hcc_ppgen_expand_push_macro(HccWorker* w, HccPPMacro* macro);
HccPPExpand* hcc_ppgen_expand_push_macro_arg(HccWorker* w, uint32_t param_idx, uint32_t args_start_idx, HccLocation** callsite_location_out);
void hcc_ppgen_expand_pop(HccWorker* w, HccPPExpand* expected_expand);
void hcc_ppgen_copy_expand_range(HccWorker* w, HccPPExpand* expand, HccATATokenBag* dst_bag, HccATATokenBag* src_bag, HccATATokenBag* alt_dst_bag, HccLocation* parent_or_child_location, HccLocation* grandparent_location, HccPPExpandFlags flags, HccPPMacro* expand_macro);
void hcc_ppgen_copy_expand_macro(HccWorker* w, HccPPMacro* macro, HccLocation* macro_callsite_location, HccLocation* parent_location, HccPPExpand* arg_expand, HccATATokenBag* args_src_bag, HccATATokenBag* dst_bag, HccATATokenBag* alt_dst_bag, HccPPExpandFlags flags);
uint32_t hcc_ppgen_process_macro_args(HccWorker* w, HccPPMacro* macro, HccPPExpand* expand, HccATATokenBag* src_bag, HccLocation* parent_location);
HccPPMacroArg* hcc_ppgen_push_macro_arg(HccWorker* w, HccPPExpand* expand, HccATATokenBag* src_bag, HccLocation* parent_location);
void hcc_ppgen_finalize_macro_arg(HccPPMacroArg* arg, HccPPExpand* expand, HccATATokenBag* src_bag);
void hcc_ppgen_attach_to_most_parent(HccWorker* w, HccLocation* location, HccLocation* parent_location);

// ===========================================
//
//
// ATA Generator
//
//
// ===========================================

typedef struct HccATAOpenBracket HccATAOpenBracket;
struct HccATAOpenBracket {
	HccATAToken close_token;
	HccLocation* open_token_location;
};

typedef uint8_t HccATAGenRunMode;
enum HccATAGenRunMode {
	HCC_ATAGEN_RUN_MODE_CODE,
	HCC_ATAGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST,
	HCC_ATAGEN_RUN_MODE_PP_INCLUDE_OPERAND,
	HCC_ATAGEN_RUN_MODE_PP_IF_OPERAND,
	HCC_ATAGEN_RUN_MODE_PP_OPERAND,
	HCC_ATAGEN_RUN_MODE_PP_MACRO_ARGS,
	HCC_ATAGEN_RUN_MODE_PP_CONCAT,
};

typedef struct HccATAPausedFile HccATAPausedFile;
struct HccATAPausedFile {
	bool        we_are_mutator_of_code_file;
	uint32_t    pp_if_span_id;
	uint32_t    if_stack_count;
	HccLocation location;
};

typedef struct HccATAGen HccATAGen;
struct HccATAGen {
	HccPPGen                 ppgen;
	HccASTFile*              ast_file;

	HccATAGenRunMode            run_mode;
	HccATATokenBag*             dst_token_bag;
	HccStack(HccATAPausedFile)  paused_file_stack;
	HccStack(HccATAOpenBracket) open_bracket_stack;
	bool                        we_are_mutator_of_code_file; // this is true when this is the first thread to start parsing the code file.

	//
	// data used when run_mode == HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST
	bool                     macro_is_function;
	bool                     macro_has_va_arg;
	HccStringId*             macro_param_string_ids;
	uint32_t                 macro_params_count;
	uint32_t                 macro_tokens_start_idx;

	HccLocation              location;
	char*                    code;      // is a local copy of location.code_file->code.data
	uint32_t                 code_size; // is a local copy of location.code_file->code.size
	uint32_t                 pp_if_span_id;
	uint32_t                 custom_line_dst;
	uint32_t                 custom_line_src;

	int32_t                  __counter__;
};

void hcc_atagen_init(HccWorker* w, HccATAGenSetup* setup);
void hcc_atagen_deinit(HccWorker* w);
void hcc_atagen_reset(HccWorker* w);
void hcc_atagen_generate(HccWorker* w);

HccLocation* hcc_atagen_make_location(HccWorker* w);
void hcc_atagen_advance_column(HccWorker* w, uint32_t by);
void hcc_atagen_advance_newline(HccWorker* w);
uint32_t hcc_atagen_display_line(HccWorker* w);
void hcc_atagen_token_add(HccWorker* w, HccATAToken token);
void hcc_atagen_token_value_add(HccWorker* w, HccATAValue value);
void hcc_atagen_count_extra_newlines(HccWorker* w);
noreturn void hcc_atagen_bail_error_1(HccWorker* w, HccErrorCode error_code, ...);
noreturn void hcc_atagen_bail_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* token_location, HccLocation* other_token_location, ...);
void hcc_atagen_paused_file_push(HccWorker* w);
void hcc_atagen_paused_file_pop(HccWorker* w);
void hcc_atagen_location_setup_new_file(HccWorker* w, HccCodeFile* code_file);

bool hcc_atagen_consume_backslash(HccWorker* w);
void hcc_atagen_consume_whitespace(HccWorker* w);
void hcc_atagen_consume_whitespace_and_newlines(HccWorker* w);
void hcc_atagen_consume_until_any_byte(HccWorker* w, char* terminator_bytes);

HccString hcc_atagen_parse_ident_from_string(HccWorker* w, HccString string, HccErrorCode error_code);
HccString hcc_atagen_parse_ident(HccWorker* w, HccErrorCode error_code);

uint32_t hcc_atagen_parse_num(HccWorker* w, HccATAToken* token_out);
void hcc_atagen_parse_string(HccWorker* w, char terminator_byte, bool ignore_escape_sequences_except_double_quotes);
uint32_t hcc_atagen_find_macro_param(HccWorker* w, HccStringId ident_string_id);
void hcc_atagen_consume_hash_for_define_replacement_list(HccWorker* w);
bool hcc_atagen_is_first_non_whitespace_on_line(HccWorker* w);
void hcc_atagen_bracket_open(HccWorker* w, HccATAToken token, HccLocation* location);
void hcc_atagen_bracket_close(HccWorker* w, HccATAToken token, HccLocation* location);
void hcc_atagen_run(HccWorker* w, HccATATokenBag* dst_token_bag, HccATAGenRunMode run_mode);

// ===========================================
//
//
// AST File
//
//
// ===========================================

typedef struct HccDeclEntry HccDeclEntry;
struct HccDeclEntry {
	HccStringId       string_id;
	HccDecl           decl;
	HccLocation*      location;
};

typedef struct HccASTFile HccASTFile;
struct HccASTFile {
	HccString                path;
	HccATAIter               iter;
	HccStack(HccPPMacro)     macros;
	HccStack(HccStringId)    macro_params;
	HccStack(HccStringId)    pragma_onced_files;
	HccStack(HccStringId)    unique_included_files;
	HccStack(HccDecl)        forward_declarations_to_link;
	HccATATokenBag           token_bag;
	HccATATokenBag           macro_token_bag;

	//
	// used for when we want to find an identifier local to the file.
	// HccCU has these hash tables that are a combination all of them from all files.
	HccHashTable(HccDeclEntry) global_declarations;
	HccHashTable(HccDeclEntry) struct_declarations; // struct T
	HccHashTable(HccDeclEntry) union_declarations;  // union T
	HccHashTable(HccDeclEntry) enum_declarations;   // enum T
};

void hcc_ast_file_init(HccASTFile* file, HccCU* cu, HccASTFileSetup* setup, HccString path);
void hcc_ast_file_deinit(HccASTFile* file);
bool hcc_ast_file_has_been_pragma_onced(HccASTFile* file, HccStringId path_string_id);
void hcc_ast_file_set_pragma_onced(HccASTFile* file, HccStringId path_string_id);
void hcc_ast_file_found_included_file(HccASTFile* file, HccStringId path_string_id);

// ===========================================
//
//
// AST - Abstract Syntax Tree
//
//
// ===========================================

typedef struct HccASTFileEntry HccASTFileEntry;
struct HccASTFileEntry {
	HccString  path;
	HccASTFile file;
};

typedef struct HccAST HccAST;
struct HccAST {
	HccASTFileSetup               file_setup;
	HccHashTable(HccASTFileEntry) files_hash_table;
	HccStack(HccASTFile*)         files;
	HccStack(HccASTVariable)      function_params_and_variables;
	HccStack(HccASTFunction)      functions;
	HccStack(HccASTExpr)          exprs;
	HccStack(HccLocation)         expr_locations;
	HccStack(HccASTVariable)      global_variables;
	HccStack(HccASTForwardDecl)   forward_declarations;
	HccStack(uint64_t)            designated_initializer_elmt_indices; // referenced by HCC_AST_EXPR_TYPE_DESIGNATED_INITIALIZER
};

void hcc_ast_init(HccCU* cu, HccCUSetup* setup);
void hcc_ast_deinit(HccCU* cu);
void hcc_ast_add_file(HccCU* cu, HccString file_path, HccASTFile** out);
HccASTFile* hcc_ast_find_file(HccCU* cu, HccString file_path);
void hcc_ast_print_expr(HccCU* cu, HccASTFunction* function, HccASTExpr* expr, uint32_t indent, HccIIO* iio);
void hcc_ast_print(HccCU* cu, HccIIO* iio);

// ===========================================
//
//
// AST Generator
//
//
// ===========================================

typedef struct HccASTGenSwitchState HccASTGenSwitchState;
struct HccASTGenSwitchState {
	HccASTExpr*    switch_stmt;
	HccASTExpr*    first_switch_case;
	HccASTExpr*    prev_switch_case;
	HccASTExpr*    default_switch_case;
	HccDataType switch_condition_type;
	uint32_t    case_stmts_count;
};

typedef struct HccFieldAccess HccFieldAccess;
struct HccFieldAccess {
	HccDataType data_type;
	uint32_t    idx;
};

typedef struct HccASTGenCurlyInitializerCurly HccASTGenCurlyInitializerCurly;
struct HccASTGenCurlyInitializerCurly {
	uint32_t nested_elmts_start_idx;
	bool     found_designator;
};

typedef struct HccASTGenCurlyInitializerElmt HccASTGenCurlyInitializerElmt;
struct HccASTGenCurlyInitializerElmt {
	HccDataType data_type; // this is the outer data type that we are initializing at this nested layer
	HccDataType resolved_data_type;
	uint64_t    elmt_idx: 63; // this is the element index into the outer data type that we are initializing
	uint64_t    had_explicit_designator_for_union_field: 1;
};

typedef struct HccASTGenDesignatorInitializer HccASTGenDesignatorInitializer;
struct HccASTGenDesignatorInitializer {
	uint32_t elmt_indices_start_idx;
	uint32_t elmt_indices_count;
};

typedef struct HccASTGenCurlyInitializerNested HccASTGenCurlyInitializerNested;
struct HccASTGenCurlyInitializerNested {
	HccASTExpr* prev_initializer_expr;
	HccASTExpr* first_initializer_expr;
	uint32_t nested_elmts_start_idx;
};

typedef struct HccASTGenCurlyInitializer HccASTGenCurlyInitializer;
struct HccASTGenCurlyInitializer {
	union {
		HccCompoundDataType* compound_data_type;
		HccArrayDataType*    array_data_type;
	};
	HccCompoundField* compound_fields;
	HccDataType       composite_data_type;
	HccDataType       elmt_data_type;
	HccDataType       resolved_composite_data_type;
	HccDataType       resolved_elmt_data_type;
	uint64_t          elmts_end_idx;

	//
	// a stack to keep track the nested curly initializer expressions.
	// these can happen in variable declaration or for compound literals.
	HccStack(HccASTGenCurlyInitializerNested) nested;

	//
	// a stack to keep track of when we open a new set of curly braces
	// and how to return back to the parent pair of curly braces
	HccStack(HccASTGenCurlyInitializerCurly) nested_curlys;

	//
	// a stack to keep track of each nested elements when we tunnel into nested data types
	// so we can tunnel out and resume from where we were
	HccStack(HccASTGenCurlyInitializerElmt) nested_elmts;

	HccASTExpr* prev_initializer_expr;
	HccASTExpr* first_initializer_expr;
	uint32_t nested_elmts_start_idx;
};

typedef uint8_t HccASTGenSpecifier;
enum {
	HCC_ASTGEN_SPECIFIER_STATIC,
	HCC_ASTGEN_SPECIFIER_EXTERN,
	HCC_ASTGEN_SPECIFIER_THREAD_LOCAL,
	HCC_ASTGEN_SPECIFIER_INLINE,
	HCC_ASTGEN_SPECIFIER_NO_RETURN,

	HCC_ASTGEN_SPECIFIER_RASTERIZER_STATE,
	HCC_ASTGEN_SPECIFIER_FRAGMENT_STATE,
	HCC_ASTGEN_SPECIFIER_BUFFER_ELEMENT,
	HCC_ASTGEN_SPECIFIER_RESOURCE_SET,
	HCC_ASTGEN_SPECIFIER_RESOURCE_TABLE,
	HCC_ASTGEN_SPECIFIER_RESOURCES,
	HCC_ASTGEN_SPECIFIER_POSITION,
	HCC_ASTGEN_SPECIFIER_NOINTERP,

	HCC_ASTGEN_SPECIFIER_VERTEX,
	HCC_ASTGEN_SPECIFIER_FRAGMENT,

	HCC_ASTGEN_SPECIFIER_COUNT,
};

typedef uint16_t HccASTGenSpecifierFlags;
enum {
	HCC_ASTGEN_SPECIFIER_FLAGS_STATIC =              1 << HCC_ASTGEN_SPECIFIER_STATIC,
	HCC_ASTGEN_SPECIFIER_FLAGS_EXTERN =              1 << HCC_ASTGEN_SPECIFIER_EXTERN,
	HCC_ASTGEN_SPECIFIER_FLAGS_THREAD_LOCAL =        1 << HCC_ASTGEN_SPECIFIER_THREAD_LOCAL,
	HCC_ASTGEN_SPECIFIER_FLAGS_INLINE =              1 << HCC_ASTGEN_SPECIFIER_INLINE,
	HCC_ASTGEN_SPECIFIER_FLAGS_NO_RETURN =           1 << HCC_ASTGEN_SPECIFIER_NO_RETURN,

	HCC_ASTGEN_SPECIFIER_FLAGS_RASTERIZER_STATE =    1 << HCC_ASTGEN_SPECIFIER_RASTERIZER_STATE,
	HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT_STATE =      1 << HCC_ASTGEN_SPECIFIER_FRAGMENT_STATE,
	HCC_ASTGEN_SPECIFIER_FLAGS_BUFFER_ELEMENT =      1 << HCC_ASTGEN_SPECIFIER_BUFFER_ELEMENT,
	HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_SET =        1 << HCC_ASTGEN_SPECIFIER_RESOURCE_SET,
	HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_TABLE =      1 << HCC_ASTGEN_SPECIFIER_RESOURCE_TABLE,
	HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCES =           1 << HCC_ASTGEN_SPECIFIER_RESOURCES,
	HCC_ASTGEN_SPECIFIER_FLAGS_POSITION =            1 << HCC_ASTGEN_SPECIFIER_POSITION,
	HCC_ASTGEN_SPECIFIER_FLAGS_NOINTERP =            1 << HCC_ASTGEN_SPECIFIER_NOINTERP,

	HCC_ASTGEN_SPECIFIER_FLAGS_VERTEX =              1 << HCC_ASTGEN_SPECIFIER_VERTEX,
	HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT =            1 << HCC_ASTGEN_SPECIFIER_FRAGMENT,

	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_SHADER_STAGES = HCC_ASTGEN_SPECIFIER_FLAGS_VERTEX | HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT,

	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS =
		HCC_ASTGEN_SPECIFIER_FLAGS_STATIC      |
		HCC_ASTGEN_SPECIFIER_FLAGS_EXTERN      |
		HCC_ASTGEN_SPECIFIER_FLAGS_THREAD_LOCAL,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS =
		HCC_ASTGEN_SPECIFIER_FLAGS_STATIC    |
		HCC_ASTGEN_SPECIFIER_FLAGS_EXTERN    |
		HCC_ASTGEN_SPECIFIER_FLAGS_INLINE    |
		HCC_ASTGEN_SPECIFIER_FLAGS_NO_RETURN |
		HCC_ASTGEN_SPECIFIER_FLAGS_ALL_SHADER_STAGES,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS =
		HCC_ASTGEN_SPECIFIER_FLAGS_RASTERIZER_STATE    |
		HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT_STATE      |
		HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT_STATE      |
		HCC_ASTGEN_SPECIFIER_FLAGS_BUFFER_ELEMENT      |
		HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_SET        |
		HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_TABLE      ,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS = HCC_ASTGEN_SPECIFIER_FLAGS_POSITION | HCC_ASTGEN_SPECIFIER_FLAGS_NOINTERP,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_TYPEDEF_SPECIFIER = 0,

	HCC_ASTGEN_SPECIFIER_FLAGS_ALL =
		HCC_ASTGEN_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS     |
		HCC_ASTGEN_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS     |
		HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS       |
		HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS |
		HCC_ASTGEN_SPECIFIER_FLAGS_ALL_TYPEDEF_SPECIFIER       ,

	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_VARIABLE_SPECIFIERS =      HCC_ASTGEN_SPECIFIER_FLAGS_ALL & ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_FUNCTION_SPECIFIERS =      HCC_ASTGEN_SPECIFIER_FLAGS_ALL & ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_STRUCT_SPECIFIERS =        HCC_ASTGEN_SPECIFIER_FLAGS_ALL & ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_STRUCT_FIELD_SPECIFIERS =  HCC_ASTGEN_SPECIFIER_FLAGS_ALL & ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS,
	HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_TYPEDEF_SPECIFIERS =       HCC_ASTGEN_SPECIFIER_FLAGS_ALL & ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_TYPEDEF_SPECIFIER,
};

typedef uint16_t HccASTGenTypeSpecifier;
enum {
	HCC_ASTGEN_TYPE_SPECIFIER_VOID =          0x1,
	HCC_ASTGEN_TYPE_SPECIFIER_BOOL =          0x2,
	HCC_ASTGEN_TYPE_SPECIFIER_CHAR =          0x4,
	HCC_ASTGEN_TYPE_SPECIFIER_SHORT =         0x8,
	HCC_ASTGEN_TYPE_SPECIFIER_INT =           0x10,
	HCC_ASTGEN_TYPE_SPECIFIER_LONG =          0x20,
	HCC_ASTGEN_TYPE_SPECIFIER_LONGLONG =      0x40,
	HCC_ASTGEN_TYPE_SPECIFIER_FLOAT =         0x80,
	HCC_ASTGEN_TYPE_SPECIFIER_DOUBLE =        0x100,
	HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED =      0x200,
	HCC_ASTGEN_TYPE_SPECIFIER_SIGNED =        0x400,
	HCC_ASTGEN_TYPE_SPECIFIER_COMPLEX =       0x800,
	HCC_ASTGEN_TYPE_SPECIFIER_ATOMIC =        0x1000,
	HCC_ASTGEN_TYPE_SPECIFIER_CONST =         0x2000,
	HCC_ASTGEN_TYPE_SPECIFIER_VOLATILE =      0x4000,

	HCC_ASTGEN_TYPE_SPECIFIER_TYPES     =
		HCC_ASTGEN_TYPE_SPECIFIER_VOID  |
		HCC_ASTGEN_TYPE_SPECIFIER_BOOL  |
		HCC_ASTGEN_TYPE_SPECIFIER_CHAR  |
		HCC_ASTGEN_TYPE_SPECIFIER_SHORT |
		HCC_ASTGEN_TYPE_SPECIFIER_INT   |
		HCC_ASTGEN_TYPE_SPECIFIER_LONG  |
		HCC_ASTGEN_TYPE_SPECIFIER_FLOAT |
		HCC_ASTGEN_TYPE_SPECIFIER_DOUBLE,

	HCC_ASTGEN_TYPE_SPECIFIER_FLOAT_TYPES =
		HCC_ASTGEN_TYPE_SPECIFIER_FLOAT   |
		HCC_ASTGEN_TYPE_SPECIFIER_DOUBLE  ,

	HCC_ASTGEN_TYPE_SPECIFIER_LONG_DOUBLE = HCC_ASTGEN_TYPE_SPECIFIER_LONG | HCC_ASTGEN_TYPE_SPECIFIER_DOUBLE,
	HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED = HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED | HCC_ASTGEN_TYPE_SPECIFIER_SIGNED,
};

typedef struct HccASTGen HccASTGen;
struct HccASTGen {
	HccASTGenSpecifierFlags specifier_flags;
	uint8_t resource_set_slot;

	HccASTFile* ast_file;
	HccATAIter* token_iter;

	HccASTGenCurlyInitializer curly_initializer;
	HccDataType assign_data_type;

	HccASTExpr*     stmt_block;
	HccASTFunction* function;

	HccASTGenSwitchState switch_state;
	bool is_in_loop;

	HccStack(HccCompoundField) compound_fields;
	HccStack(HccEnumValue)     enum_values;
	HccStack(HccASTVariable)   function_params_and_variables;
	HccStack(HccStringId)      variable_stack_strings;
	HccStack(HccDecl)          variable_stack;
	uint32_t                   next_var_idx;

	HccStack(HccFieldAccess) compound_type_find_fields;

	//
	// used to find identical compound field names
	HccStack(HccStringId)  compound_field_names;
	HccStack(HccLocation*) compound_field_locations;
};

extern HccATAToken hcc_astgen_specifier_tokens[HCC_ASTGEN_SPECIFIER_COUNT];

void hcc_astgen_init(HccWorker* w, HccASTGenSetup* setup);
void hcc_astgen_deinit(HccWorker* w);
void hcc_astgen_reset(HccWorker* w);

void hcc_astgen_error_1(HccWorker* w, HccErrorCode error_code, ...);
void hcc_astgen_error_1_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...);
void hcc_astgen_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* other_location, ...);
void hcc_astgen_error_2_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...);
void hcc_astgen_warn_1(HccWorker* w, HccWarnCode warn_code, ...);
void hcc_astgen_warn_1_manual(HccWorker* w, HccWarnCode warn_code, HccLocation* location, ...);
void hcc_astgen_warn_2(HccWorker* w, HccWarnCode warn_code, HccLocation* other_location, ...);
void hcc_astgen_warn_2_manual(HccWorker* w, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, ...);
noreturn void hcc_astgen_bail_error_1(HccWorker* w, HccErrorCode error_code, ...);
noreturn void hcc_astgen_bail_error_1_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...);
noreturn void hcc_astgen_bail_error_1_merge_apply(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...);
noreturn void hcc_astgen_bail_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* other_location, ...);
noreturn void hcc_astgen_bail_error_2_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...);

void hcc_astgen_data_type_found(HccWorker* w, HccDataType data_type);
void hcc_astgen_data_type_ensure_compound_type_default_kind(HccWorker* w, HccDataType data_type, HccErrorCode error_code);
void hcc_astgen_data_type_ensure_valid_variable(HccWorker* w, HccDataType data_type, HccErrorCode error_code);
void hcc_astgen_data_type_ensure_compound_type_has_no_resources(HccWorker* w, HccDataType data_type, HccErrorCode error_code);
void hcc_astgen_data_type_ensure_has_no_resources(HccWorker* w, HccDataType data_type, HccErrorCode error_code);
void hcc_astgen_data_type_ensure_has_no_pointers(HccWorker* w, HccDataType data_type, HccErrorCode error_code);
HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name(HccWorker* w, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id);
HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_checked(HccWorker* w, HccDataType data_type, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id);
HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_recursive(HccWorker* w, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id);
void hcc_astgen_insert_global_declaration(HccWorker* w, HccStringId identifier_string_id, HccDecl decl, HccLocation* location);
void hcc_astgen_eval_cast(HccWorker* w, HccASTExpr* expr, HccDataType dst_data_type);
HccASTExpr* hcc_astgen_alloc_expr(HccWorker* w, HccASTExprType type);
HccHash64 hcc_astgen_hash_compound_data_type_field(HccCU* cu, HccDataType data_type, HccHash64 hash);

const char* hcc_astgen_type_specifier_string(HccASTGenTypeSpecifier specifier);
void hcc_astgen_data_type_ensure_is_condition(HccWorker* w, HccDataType data_type);
void hcc_astgen_compound_data_type_validate_field_names(HccWorker* w, HccDataType outer_data_type, HccCompoundDataType* compound_data_type);
void hcc_astgen_validate_specifiers(HccWorker* w, HccASTGenSpecifierFlags non_specifiers, HccErrorCode invalid_specifier_error_code);
HccATAToken hcc_astgen_ensure_semicolon(HccWorker* w);
bool hcc_astgen_data_type_check_compatible_assignment(HccWorker* w, HccDataType target_data_type, HccASTExpr** source_expr_mut);
void hcc_astgen_data_type_ensure_compatible_assignment(HccWorker* w, HccLocation* other_location, HccDataType target_data_type, HccASTExpr** source_expr_mut);
bool hcc_astgen_data_type_check_compatible_arithmetic(HccWorker* w, HccASTExpr** left_expr_mut, HccASTExpr** right_expr_mut);
void hcc_astgen_data_type_ensure_compatible_arithmetic(HccWorker* w, HccLocation* other_location, HccASTExpr** left_expr_mut, HccASTExpr** right_expr_mut, HccATAToken operator_token);
void hcc_astgen_ensure_function_args_count(HccWorker* w, HccDecl function_decl, uint32_t args_count);
HccDataType hcc_astgen_deduplicate_constbuffer_data_type(HccWorker* w, HccDataType element_data_type);
HccDataType hcc_astgen_deduplicate_buffer_data_type(HccWorker* w, HccDataType element_data_type);
void _hcc_astgen_ensure_no_unused_specifiers(HccWorker* w, char* what);
void hcc_astgen_ensure_no_unused_specifiers_data_type(HccWorker* w);
void hcc_astgen_ensure_no_unused_specifiers_identifier(HccWorker* w);

void hcc_astgen_variable_stack_open(HccWorker* w);
void hcc_astgen_variable_stack_close(HccWorker* w);
HccDecl hcc_astgen_variable_stack_add_local(HccWorker* w, HccStringId string_id);
void hcc_astgen_variable_stack_add_global(HccWorker* w, HccStringId string_id, HccDecl decl);
HccDecl hcc_astgen_variable_stack_find(HccWorker* w, HccStringId string_id);

HccATAToken hcc_astgen_curly_initializer_start(HccWorker* w, HccDataType data_type, HccDataType resolved_data_type);
HccATAToken hcc_astgen_curly_initializer_open(HccWorker* w);
HccATAToken hcc_astgen_curly_initializer_close(HccWorker* w, bool is_finished);
bool hcc_astgen_curly_initializer_next_elmt(HccWorker* w, HccDataType resolved_target_data_type);
HccATAToken hcc_astgen_curly_initializer_next_elmt_with_designator(HccWorker* w);
void hcc_astgen_curly_initializer_nested_elmt_push(HccWorker* w, HccDataType data_type, HccDataType resolved_data_type);
void hcc_astgen_curly_initializer_tunnel_in(HccWorker* w);
void hcc_astgen_curly_initializer_tunnel_out(HccWorker* w);
void hcc_astgen_curly_initializer_set_composite(HccWorker* w, HccDataType data_type, HccDataType resolved_data_type);
HccASTExpr* hcc_astgen_curly_initializer_generate_designated_initializer(HccWorker* w, HccLocation* location);

HccATAToken hcc_astgen_generate_specifiers(HccWorker* w);
HccDataType hcc_astgen_generate_enum_data_type(HccWorker* w);
HccDataType hcc_astgen_generate_compound_data_type(HccWorker* w);
HccATAToken hcc_astgen_generate_type_specifiers(HccWorker* w, HccLocation* location, HccASTGenTypeSpecifier* type_specifiers_mut);
HccATAToken hcc_astgen_generate_type_specifiers_post_qualifiers(HccWorker* w, HccLocation* location, HccASTGenTypeSpecifier* type_specifiers_mut);
HccDataType hcc_astgen_generate_data_type(HccWorker* w, HccErrorCode error_code, bool want_concrete_type);
HccDataType hcc_astgen_generate_pointer_data_type_if_exists(HccWorker* w, HccDataType element_data_type);
HccDataType hcc_astgen_generate_array_data_type_if_exists(HccWorker* w, HccDataType element_data_type);
HccDataType hcc_astgen_generate_typedef(HccWorker* w);
HccDataType hcc_astgen_generate_typedef_with_data_type(HccWorker* w, HccDataType aliased_data_type);
void hcc_astgen_generate_implicit_cast(HccWorker* w, HccDataType dst_data_type, HccASTExpr** expr_mut);
HccASTExpr* hcc_astgen_generate_unary_op(HccWorker* w, HccASTExpr* inner_expr, HccASTUnaryOp unary_op, HccATAToken operator_token, HccLocation* location);
HccASTExpr* hcc_astgen_generate_unary_expr(HccWorker* w);
void hcc_astgen_generate_binary_op(HccWorker* w, HccASTBinaryOp* binary_op_out, uint32_t* precedence_out, bool* is_assignment_out);
HccASTExpr* hcc_astgen_generate_call_expr(HccWorker* w, HccASTExpr* function_expr);
HccASTExpr* hcc_astgen_generate_array_subscript_expr(HccWorker* w, HccASTExpr* array_expr);
HccASTExpr* hcc_astgen_generate_field_access_expr(HccWorker* w, HccASTExpr* left_expr, bool is_indirect);
HccASTExpr* hcc_astgen_generate_ternary_expr(HccWorker* w, HccASTExpr* cond_expr);
HccASTExpr* hcc_astgen_generate_expr_(HccWorker* w, uint32_t min_precedence, bool no_comma_operator);
HccASTExpr* hcc_astgen_generate_expr(HccWorker* w, uint32_t min_precedence);
HccASTExpr* hcc_astgen_generate_expr_no_comma_operator(HccWorker* w, uint32_t min_precedence);
HccASTExpr* hcc_astgen_generate_cond_expr(HccWorker* w);
HccDataType hcc_astgen_generate_variable_decl_array(HccWorker* w, HccDataType element_data_type);
HccDecl hcc_astgen_generate_variable_decl(HccWorker* w, bool is_global, HccDataType element_data_type, HccDataType* data_type_mut, HccASTExpr** init_expr_out);
HccASTExpr* hcc_astgen_generate_variable_decl_stmt(HccWorker* w, HccDataType data_type);
HccASTExpr* hcc_astgen_generate_stmt(HccWorker* w);
void hcc_astgen_generate_function(HccWorker* w, HccDataType return_data_type, HccLocation* return_data_type_location);
void hcc_astgen_generate(HccWorker* w);

// ===========================================
//
//
// ASTLink
//
//
// ===========================================

typedef struct HccASTLink HccASTLink;
struct HccASTLink {
	HccASTFile* ast_file;
};

void hcc_astlink_init(HccWorker* w, HccASTLinkSetup* setup);
void hcc_astlink_deinit(HccWorker* w);
void hcc_astlink_reset(HccWorker* w);

void hcc_astlink_error_1_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...);
void hcc_astlink_error_2_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...);

void hcc_astlink_link_file(HccWorker* w);

// ===========================================
//
//
// AML Function
//
//
// ===========================================

typedef struct HccAMLFunction HccAMLFunction;
struct HccAMLFunction {
	HccLocation*              identifier_location;
	HccStringId               identifier_string_id;
	HccDataType               function_data_type;
	HccDataType               return_data_type;
	HccShaderStage            shader_stage;
	HccOptLevel               opt_level;
	uint8_t                   params_count;

	HccAMLWord*               words;
	HccAMLValue*              values;
	HccAMLBasicBlock*         basic_blocks;
	HccAMLBasicBlockParam*    basic_block_params;
	HccAMLBasicBlockParamSrc* basic_block_param_srcs;
	uint32_t                  words_count;
	uint32_t                  words_cap;
	uint32_t                  values_count;
	uint32_t                  values_cap;
	uint32_t                  basic_blocks_count;
	uint32_t                  basic_blocks_cap;
	uint32_t                  basic_block_params_count;
	uint32_t                  basic_block_params_cap;
	uint32_t                  basic_block_param_srcs_count;
	uint32_t                  basic_block_param_srcs_cap;

	HccAtomic(uint32_t)       ref_count;
	HccAtomic(bool)           can_free;
	HccAMLFunction*           next_free;
};

HccAMLFunction* hcc_aml_function_take_ref(HccCU* cu, HccDecl decl);
void hcc_aml_function_return_ref(HccCU* cu, HccAMLFunction* function);

HccAMLOperand hcc_aml_function_value_add(HccAMLFunction* function, HccDataType data_type);
HccAMLOperand hcc_aml_function_basic_block_add(HccAMLFunction* function, uint32_t location_idx);
HccAMLOperand hcc_aml_function_basic_block_param_add(HccAMLFunction* function, HccDataType data_type);
void hcc_aml_function_basic_block_param_src_add(HccAMLFunction* function, HccAMLOperand basic_block_operand, HccAMLOperand operand);
HccAMLOperand* hcc_aml_function_instr_add(HccAMLFunction* function, uint32_t location_idx, HccAMLOp op, uint16_t operands_count);

// ===========================================
//
//
// AML Function Allocator
//
//
// ===========================================

#define HCC_AML_INSTR_AVERAGE_WORDS              4.f
#define HCC_AML_INSTR_AVERAGE_VALUES             0.75f
#define HCC_AML_INSTR_AVERAGE_BASIC_BLOCKS       0.25f
#define HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAMS 0.25f
#define HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAM_SRCS 0.25f

#define HCC_AML_FUNCTION_ALLOCATOR_INTSR_MIN_LOG2 10
#define HCC_AML_FUNCTION_ALLOCATOR_INTSR_MAX_LOG2 16
#define HCC_AML_FUNCTION_ALLOCATOR_INTSR_LOG2_COUNT (HCC_AML_FUNCTION_ALLOCATOR_INTSR_MAX_LOG2 - HCC_AML_FUNCTION_ALLOCATOR_INTSR_MIN_LOG2)

#define HCC_AML_FUNCTION_SENTINAL ((HccAMLFunction*)UINTPTR_MAX)

typedef struct HccAMLFunctionAlctor HccAMLFunctionAlctor;
struct HccAMLFunctionAlctor {
	HccStack(HccAMLFunction)           functions_pool;
	HccStack(HccAMLWord)               words_pool;
	HccStack(HccAMLValue)              values_pool;
	HccStack(HccAMLBasicBlock)         basic_blocks_pool;
	HccStack(HccAMLBasicBlockParam)    basic_block_params_pool;
	HccStack(HccAMLBasicBlockParamSrc) basic_block_param_srcs_pool;
	HccAtomic(HccAMLFunction*) free_functions_by_instr_log2[HCC_AML_FUNCTION_ALLOCATOR_INTSR_LOG2_COUNT];
};

void hcc_aml_function_alctor_init(HccCU* cu, HccAMLFunctionAlctorSetup* setup);
void hcc_aml_function_alctor_deinit(HccCU* cu);
uint32_t hcc_aml_function_alctor_instr_count_round_up_log2(HccCU* cu, uint32_t max_instrs_count);
HccAMLFunction* hcc_aml_function_alctor_alloc(HccCU* cu, uint32_t max_instrs_count);
void hcc_aml_function_alctor_dealloc(HccCU* cu, HccAMLFunction* function);

// ===========================================
//
//
// AML
//
//
// ===========================================

typedef uint8_t HccAMLOptPhase;
enum HccAMLOptPhase {
	HCC_AML_OPT_PHASE_0,
	HCC_AML_OPT_PHASE_1,
	HCC_AML_OPT_PHASE_2,

	HCC_AML_OPT_PHASE_COUNT,
};

typedef struct HccAMLCallNode HccAMLCallNode;
struct HccAMLCallNode {
	HccDecl function_decl;
	uint32_t next_call_node_idx;
};

typedef struct HccAML HccAML;
struct HccAML {
	HccAMLFunctionAlctor      function_alctor;
	HccStack(HccAtomic(HccAMLFunction*)) functions; // use index of HccDecl(Function) to access this array
	HccStack(HccLocation*)    locations; // all the HccAMLInstr have a location index into this array
	HccAMLOptPhase opt_phase;
	HccStack(HccAMLCallNode)  call_graph_nodes;
	HccStack(HccAMLCallNode*) function_call_node_lists;
	HccStack(HccDecl)         optimize_functions[2];
	uint32_t                  optimize_functions_idx;
	HccSpinMutex              optimize_functions_mutex; // used to lock and deduplicate optimize functions when needed
};

void hcc_aml_init(HccCU* cu, HccCUSetup* setup);
void hcc_aml_deinit(HccCU* cu);
void hcc_aml_print_operand(HccCU* cu, const HccAMLFunction* function, HccAMLOperand operand, HccIIO* iio, bool is_definition);
void hcc_aml_print(HccCU* cu, HccIIO* iio);
HccLocation* hcc_aml_instr_location(HccCU* cu, HccAMLInstr* instr);
HccStack(HccDecl) hcc_aml_optimize_functions(HccCU* cu);
void hcc_aml_next_optimize_functions_array(HccCU* cu);
HccAMLOperand hcc_aml_basic_block_next(const HccAMLFunction* function, HccAMLOperand basic_block_operand);
HccAMLOperand hcc_aml_instr_switch_merge_basic_block_operand(const HccAMLFunction* function, HccAMLInstr* instr);

// ===========================================
//
//
// AML Generator
//
//
// ===========================================

typedef struct HccAMLGen HccAMLGen;
struct HccAMLGen {
	HccASTFunction*         ast_function;
	HccAMLFunction*         function;
	HccDecl                 function_decl;
	HccStack(HccAMLOperand) temp_operands;
	HccAMLOperand*          access_chain_operands;
	HccAMLOp                access_chain_op;
	uint32_t                access_chain_operand_idx;
	HccAMLOperand*          switch_case_operands;
	uint32_t                switch_case_idx;
	uint32_t                continue_stmt_list_head_id;
	uint32_t                continue_stmt_list_prev_id;
	uint32_t                break_stmt_list_head_id;
	uint32_t                break_stmt_list_prev_id;
	HccAMLOp                last_op;
	HccLocation*            last_location;
};

void hcc_amlgen_init(HccWorker* w, HccCompilerSetup* setup);
void hcc_amlgen_deinit(HccWorker* w);
void hcc_amlgen_reset(HccWorker* w);

void hcc_amlgen_error_1(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...);

HccAMLOperand hcc_amlgen_value_add(HccWorker* w, HccDataType data_type);
HccAMLOperand hcc_amlgen_basic_block_add(HccWorker* w, HccLocation* location);
HccAMLOperand hcc_amlgen_basic_block_param_add(HccWorker* w, HccDataType data_type);
void hcc_amlgen_basic_block_param_src_add(HccWorker* w, HccAMLOperand basic_block_operand, HccAMLOperand operand);
HccAMLOperand* hcc_amlgen_instr_add(HccWorker* w, HccLocation* location, HccAMLOp op, uint16_t operands_count);
HccAMLOperand hcc_amlgen_instr_add_1(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0);
HccAMLOperand hcc_amlgen_instr_add_2(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0, HccAMLOperand operand_1);
HccAMLOperand hcc_amlgen_instr_add_3(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0, HccAMLOperand operand_1, HccAMLOperand operand_2);
HccAMLOperand hcc_amlgen_instr_add_4(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0, HccAMLOperand operand_1, HccAMLOperand operand_2, HccAMLOperand operand_3);
HccAMLOperand hcc_amlgen_local_variable_operand(HccWorker* w, uint32_t local_variable_idx);
HccAMLOperand hcc_amlgen_current_basic_block(HccWorker* w);

HccAMLOperand hcc_amlgen_generate_convert_to_bool(HccWorker* w, HccLocation* location, HccAMLOperand src_operand, HccDataType src_data_type, bool flip_bool_result);
HccAMLOperand hcc_amlgen_generate_instrs(HccWorker* w, HccASTExpr* expr, bool want_variable_ref);
HccAMLOperand hcc_amlgen_generate_instrs_condition(HccWorker* w, HccASTExpr* cond_expr);
HccAMLOperand hcc_amlgen_generate_instr_shader_param(HccWorker* w, HccASTExpr* expr, uint32_t recursion_count);
HccAMLOperand hcc_amlgen_generate_instr_access_chain(HccWorker* w, HccASTExpr* expr, uint32_t count);
HccAMLOperand hcc_amlgen_generate_instr_access_chain_start(HccWorker* w, HccLocation* location, HccAMLOperand base_ptr_operand, uint32_t count, bool is_in_bounds);
void hcc_amlgen_generate_instr_access_chain_set_next_operand(HccWorker* w, HccAMLOperand operand);
void hcc_amlgen_generate_instr_access_chain_end(HccWorker* w, HccDataType dst_data_type);
HccAMLOperand hcc_amlgen_generate_bitcast_union_field(HccWorker* w, HccLocation* location, HccDataType union_data_type, uint32_t field_idx, HccAMLOperand union_ptr_operand);
void hcc_amlgen_generate(HccWorker* w);

// ===========================================
//
//
// AML Optimizer
//
//
// ===========================================

typedef struct HccAMLOpt HccAMLOpt;
struct HccAMLOpt {
	uint16_t function_recursion_call_stack_count;
	HccDecl  function_recursion_call_stack[HCC_FUNCTION_CALL_STACK_CAP];
};

typedef const HccAMLFunction* (*HccAMLOptFn)(HccWorker* w, HccDecl function_decl, const HccAMLFunction* aml_function);

extern HccAMLOptFn hcc_aml_opts_phase_0_level_0[];
extern HccAMLOptFn hcc_aml_opts_phase_1_level_0[];
extern HccAMLOptFn hcc_aml_opts_phase_2_level_0[];
extern HccAMLOptFn hcc_aml_opts_phase_0_level_1[];
extern HccAMLOptFn hcc_aml_opts_phase_1_level_1[];
extern HccAMLOptFn hcc_aml_opts_phase_2_level_1[];
extern HccAMLOptFn hcc_aml_opts_phase_0_level_2[];
extern HccAMLOptFn hcc_aml_opts_phase_1_level_2[];
extern HccAMLOptFn hcc_aml_opts_phase_2_level_2[];
extern HccAMLOptFn hcc_aml_opts_phase_0_level_3[];
extern HccAMLOptFn hcc_aml_opts_phase_1_level_3[];
extern HccAMLOptFn hcc_aml_opts_phase_2_level_3[];
extern HccAMLOptFn hcc_aml_opts_phase_0_level_s[];
extern HccAMLOptFn hcc_aml_opts_phase_1_level_s[];
extern HccAMLOptFn hcc_aml_opts_phase_2_level_s[];
extern HccAMLOptFn hcc_aml_opts_phase_0_level_g[];
extern HccAMLOptFn hcc_aml_opts_phase_1_level_g[];
extern HccAMLOptFn hcc_aml_opts_phase_2_level_g[];
extern HccAMLOptFn* hcc_aml_opts[HCC_AML_OPT_PHASE_COUNT][HCC_OPT_LEVEL_COUNT];
extern uint32_t hcc_aml_opts_count[HCC_AML_OPT_PHASE_COUNT][HCC_OPT_LEVEL_COUNT];

void hcc_amlopt_init(HccWorker* w, HccCompilerSetup* setup);
void hcc_amlopt_deinit(HccWorker* w);
void hcc_amlopt_reset(HccWorker* w);

void hcc_amlopt_error_1(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...);
void hcc_amlopt_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...);

bool hcc_amlopt_check_for_recursion_and_make_ordered_function_list_(HccWorker* w, HccDecl function_decl);
bool hcc_amlopt_ensure_supported_type(HccWorker* w, HccDataType data_type, HccLocation* location);

const HccAMLFunction* hcc_amlopt_make_call_graph(HccWorker* w, HccDecl function_decl, const HccAMLFunction* aml_function);
const HccAMLFunction* hcc_amlopt_check_for_recursion_and_make_ordered_function_list(HccWorker* w, HccDecl function_decl, const HccAMLFunction* aml_function);
const HccAMLFunction* hcc_amlopt_check_for_unsupported_types(HccWorker* w, HccDecl function_decl, const HccAMLFunction* aml_function);

void hcc_amlopt_optimize(HccWorker* w);

// ===========================================
//
//
// SPIR-V
//
//
// ===========================================

typedef uint32_t HccSPIRVWord;
typedef HccSPIRVWord HccSPIRVId;
typedef HccSPIRVWord HccSPIRVOperand;
typedef HccSPIRVWord HccSPIRVInstr;

typedef uint16_t HccSPIRVOp;
enum {
	HCC_SPIRV_OP_NO_OP = 0,
	HCC_SPIRV_OP_EXTENSION = 10,
	HCC_SPIRV_OP_EXT_INST_IMPORT = 11,
	HCC_SPIRV_OP_EXT_INST = 12,
	HCC_SPIRV_OP_MEMORY_MODEL = 14,
	HCC_SPIRV_OP_ENTRY_POINT = 15,
	HCC_SPIRV_OP_EXECUTION_MODE = 16,
	HCC_SPIRV_OP_CAPABILITY = 17,
	HCC_SPIRV_OP_TYPE_VOID = 19,
	HCC_SPIRV_OP_TYPE_BOOL = 20,
	HCC_SPIRV_OP_TYPE_INT = 21,
	HCC_SPIRV_OP_TYPE_FLOAT = 22,
	HCC_SPIRV_OP_TYPE_VECTOR = 23,
	HCC_SPIRV_OP_TYPE_MATRIX = 24,
	HCC_SPIRV_OP_TYPE_IMAGE = 25,
	HCC_SPIRV_OP_TYPE_SAMPLER = 26,
	HCC_SPIRV_OP_TYPE_ARRAY = 28,
	HCC_SPIRV_OP_TYPE_STRUCT = 30,
	HCC_SPIRV_OP_TYPE_POINTER = 32,
	HCC_SPIRV_OP_TYPE_FUNCTION = 33,

	HCC_SPIRV_OP_CONSTANT_TRUE = 41,
	HCC_SPIRV_OP_CONSTANT_FALSE = 42,
	HCC_SPIRV_OP_CONSTANT = 43,
	HCC_SPIRV_OP_CONSTANT_COMPOSITE = 44,
	HCC_SPIRV_OP_CONSTANT_NULL = 46,

	HCC_SPIRV_OP_FUNCTION = 54,
	HCC_SPIRV_OP_FUNCTION_PARAMETER = 55,
	HCC_SPIRV_OP_FUNCTION_END = 56,
	HCC_SPIRV_OP_FUNCTION_CALL = 57,
	HCC_SPIRV_OP_VARIABLE = 59,
	HCC_SPIRV_OP_LOAD = 61,
	HCC_SPIRV_OP_STORE = 62,
	HCC_SPIRV_OP_ACCESS_CHAIN = 65,
	HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN = 66,
	HCC_SPIRV_OP_DECORATE = 71,
	HCC_SPIRV_OP_COMPOSITE_CONSTRUCT = 80,
	HCC_SPIRV_OP_TRANSPOSE = 84,
	HCC_SPIRV_OP_CONVERT_F_TO_U = 109,
	HCC_SPIRV_OP_CONVERT_F_TO_S = 110,
	HCC_SPIRV_OP_CONVERT_S_TO_F = 111,
	HCC_SPIRV_OP_CONVERT_U_TO_F = 112,
	HCC_SPIRV_OP_U_CONVERT = 113,
	HCC_SPIRV_OP_S_CONVERT = 114,
	HCC_SPIRV_OP_F_CONVERT = 115,
	HCC_SPIRV_OP_BITCAST = 124,
	HCC_SPIRV_OP_S_NEGATE = 126,
	HCC_SPIRV_OP_F_NEGATE = 127,
	HCC_SPIRV_OP_I_ADD = 128,
	HCC_SPIRV_OP_F_ADD = 129,
	HCC_SPIRV_OP_I_SUB = 130,
	HCC_SPIRV_OP_F_SUB = 131,
	HCC_SPIRV_OP_I_MUL = 132,
	HCC_SPIRV_OP_F_MUL = 133,
	HCC_SPIRV_OP_U_DIV = 134,
	HCC_SPIRV_OP_S_DIV = 135,
	HCC_SPIRV_OP_F_DIV = 136,
	HCC_SPIRV_OP_U_MOD = 137,
	HCC_SPIRV_OP_S_MOD = 139,
	HCC_SPIRV_OP_F_MOD = 141,
	HCC_SPIRV_OP_MATRIX_TIMES_SCALAR = 143,
	HCC_SPIRV_OP_VECTOR_TIMES_MATRIX = 144,
	HCC_SPIRV_OP_MATRIX_TIMES_VECTOR = 145,
	HCC_SPIRV_OP_MATRIX_TIMES_MATRIX = 146,
	HCC_SPIRV_OP_OUTER_PRODUCT = 147,
	HCC_SPIRV_OP_DOT = 148,
	HCC_SPIRV_OP_ANY = 154,
	HCC_SPIRV_OP_ALL = 155,
	HCC_SPIRV_OP_ISNAN = 156,
	HCC_SPIRV_OP_ISINF = 157,
	HCC_SPIRV_OP_LOGICAL_EQUAL = 164,
	HCC_SPIRV_OP_LOGICAL_NOT_EQUAL = 165,
	HCC_SPIRV_OP_LOGICAL_OR = 166,
	HCC_SPIRV_OP_LOGICAL_AND = 167,
	HCC_SPIRV_OP_LOGICAL_NOT = 168,
	HCC_SPIRV_OP_SELECT = 169,
	HCC_SPIRV_OP_I_EQUAL = 170,
	HCC_SPIRV_OP_I_NOT_EQUAL = 171,
	HCC_SPIRV_OP_U_GREATER_THAN = 172,
	HCC_SPIRV_OP_S_GREATER_THAN = 173,
	HCC_SPIRV_OP_U_GREATER_THAN_EQUAL = 174,
	HCC_SPIRV_OP_S_GREATER_THAN_EQUAL = 175,
	HCC_SPIRV_OP_U_LESS_THAN = 176,
	HCC_SPIRV_OP_S_LESS_THAN = 177,
	HCC_SPIRV_OP_U_LESS_THAN_EQUAL = 178,
	HCC_SPIRV_OP_S_LESS_THAN_EQUAL = 179,
	HCC_SPIRV_OP_F_UNORD_EQUAL = 181,
	HCC_SPIRV_OP_F_UNORD_NOT_EQUAL = 183,
	HCC_SPIRV_OP_F_UNORD_LESS_THAN = 185,
	HCC_SPIRV_OP_F_UNORD_GREATER_THAN = 187,
	HCC_SPIRV_OP_F_UNORD_LESS_THAN_EQUAL = 189,
	HCC_SPIRV_OP_F_UNORD_GREATER_THAN_EQUAL = 191,
	HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_LOGICAL = 194,
	HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_ARITHMETIC = 195,
	HCC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL = 196,
	HCC_SPIRV_OP_BITWISE_OR = 197,
	HCC_SPIRV_OP_BITWISE_XOR = 198,
	HCC_SPIRV_OP_BITWISE_AND = 199,
	HCC_SPIRV_OP_BITWISE_NOT = 200,
	HCC_SPIRV_OP_PHI = 245,
	HCC_SPIRV_OP_LOOP_MERGE = 246,
	HCC_SPIRV_OP_SELECTION_MERGE = 247,
	HCC_SPIRV_OP_LABEL = 248,
	HCC_SPIRV_OP_BRANCH = 249,
	HCC_SPIRV_OP_BRANCH_CONDITIONAL = 250,
	HCC_SPIRV_OP_SWITCH = 251,
	HCC_SPIRV_OP_RETURN = 253,
	HCC_SPIRV_OP_RETURN_VALUE = 254,
	HCC_SPIRV_OP_UNREACHABLE = 255,
};

typedef uint16_t HccSPIRVGLSLSTD450Op;
enum {
	HCC_SPIRV_GLSL_STD_450_OP_NO_OP = 0,
	HCC_SPIRV_GLSL_STD_450_OP_ROUND = 1,
	HCC_SPIRV_GLSL_STD_450_OP_TRUNC = 3,
	HCC_SPIRV_GLSL_STD_450_OP_F_ABS = 4,
	HCC_SPIRV_GLSL_STD_450_OP_S_ABS = 5,
	HCC_SPIRV_GLSL_STD_450_OP_F_SIGN = 6,
	HCC_SPIRV_GLSL_STD_450_OP_S_SIGN = 7,
	HCC_SPIRV_GLSL_STD_450_OP_FLOOR = 8,
	HCC_SPIRV_GLSL_STD_450_OP_CEIL = 9,
	HCC_SPIRV_GLSL_STD_450_OP_FRACT = 10,
	HCC_SPIRV_GLSL_STD_450_OP_RADIANS = 11,
	HCC_SPIRV_GLSL_STD_450_OP_DEGREES = 12,
	HCC_SPIRV_GLSL_STD_450_OP_SIN = 13,
	HCC_SPIRV_GLSL_STD_450_OP_COS = 14,
	HCC_SPIRV_GLSL_STD_450_OP_TAN = 15,
	HCC_SPIRV_GLSL_STD_450_OP_ASIN = 16,
	HCC_SPIRV_GLSL_STD_450_OP_ACOS = 17,
	HCC_SPIRV_GLSL_STD_450_OP_ATAN = 18,
	HCC_SPIRV_GLSL_STD_450_OP_SINH = 19,
	HCC_SPIRV_GLSL_STD_450_OP_COSH = 20,
	HCC_SPIRV_GLSL_STD_450_OP_TANH = 21,
	HCC_SPIRV_GLSL_STD_450_OP_ASINH = 22,
	HCC_SPIRV_GLSL_STD_450_OP_ACOSH = 23,
	HCC_SPIRV_GLSL_STD_450_OP_ATANH = 24,
	HCC_SPIRV_GLSL_STD_450_OP_ATAN2 = 25,
	HCC_SPIRV_GLSL_STD_450_OP_POW = 26,
	HCC_SPIRV_GLSL_STD_450_OP_EXP = 27,
	HCC_SPIRV_GLSL_STD_450_OP_LOG = 28,
	HCC_SPIRV_GLSL_STD_450_OP_EXP2 = 29,
	HCC_SPIRV_GLSL_STD_450_OP_LOG2 = 30,
	HCC_SPIRV_GLSL_STD_450_OP_SQRT = 31,
	HCC_SPIRV_GLSL_STD_450_OP_INVERSE_SQRT = 32,
	HCC_SPIRV_GLSL_STD_450_OP_DETERMINANT = 33,
	HCC_SPIRV_GLSL_STD_450_OP_MATRIX_INVERSE = 34,
	HCC_SPIRV_GLSL_STD_450_OP_F_MIN = 37,
	HCC_SPIRV_GLSL_STD_450_OP_U_MIN = 38,
	HCC_SPIRV_GLSL_STD_450_OP_S_MIN = 39,
	HCC_SPIRV_GLSL_STD_450_OP_F_MAX = 40,
	HCC_SPIRV_GLSL_STD_450_OP_U_MAX = 41,
	HCC_SPIRV_GLSL_STD_450_OP_S_MAX = 42,
	HCC_SPIRV_GLSL_STD_450_OP_F_CLAMP = 43,
	HCC_SPIRV_GLSL_STD_450_OP_U_CLAMP = 44,
	HCC_SPIRV_GLSL_STD_450_OP_S_CLAMP = 45,
	HCC_SPIRV_GLSL_STD_450_OP_F_MIX = 46,
	HCC_SPIRV_GLSL_STD_450_OP_STEP = 48,
	HCC_SPIRV_GLSL_STD_450_OP_SMOOTHSTEP = 49,
	HCC_SPIRV_GLSL_STD_450_OP_FMA = 50,
	HCC_SPIRV_GLSL_STD_450_OP_PACK_SNORM4X8 = 54,
	HCC_SPIRV_GLSL_STD_450_OP_PACK_UNORM4X8 = 55,
	HCC_SPIRV_GLSL_STD_450_OP_PACK_SNORM2X16 = 56,
	HCC_SPIRV_GLSL_STD_450_OP_PACK_UNORM2X16 = 57,
	HCC_SPIRV_GLSL_STD_450_OP_PACK_HALF2X16 = 58,
	HCC_SPIRV_GLSL_STD_450_OP_UNPACK_SNORM2X16 = 60,
	HCC_SPIRV_GLSL_STD_450_OP_UNPACK_UNORM2X16 = 61,
	HCC_SPIRV_GLSL_STD_450_OP_UNPACK_HALF2X16 = 62,
	HCC_SPIRV_GLSL_STD_450_OP_UNPACK_SNORM4X8 = 63,
	HCC_SPIRV_GLSL_STD_450_OP_UNPACK_UNORM4X8 = 64,
	HCC_SPIRV_GLSL_STD_450_OP_LENGTH = 66,
	HCC_SPIRV_GLSL_STD_450_OP_CROSS = 68,
	HCC_SPIRV_GLSL_STD_450_OP_NORMALIZE = 69,
	HCC_SPIRV_GLSL_STD_450_OP_REFLECT = 71,
	HCC_SPIRV_GLSL_STD_450_OP_REFRACT = 72,
};

enum {
	HCC_SPIRV_MEMORY_OPERANDS_NONE = 0x0,
	HCC_SPIRV_MEMORY_OPERANDS_VOLATILE = 0x1,
	HCC_SPIRV_MEMORY_OPERANDS_ALIGNED = 0x2,
};

enum {
	HCC_SPIRV_ADDRESS_MODEL_LOGICAL = 0,
	HCC_SPIRV_ADDRESS_MODEL_PHYSICAL_STORAGE_BUFFER_64 = 5348,
};

enum {
	HCC_SPIRV_MEMORY_MODEL_GLSL450 = 1,
	HCC_SPIRV_MEMORY_MODEL_VULKAN = 3,
};

enum {
	HCC_SPIRV_EXECUTION_MODE_ORIGIN_UPPER_LEFT = 7,
	HCC_SPIRV_EXECUTION_MODE_ORIGIN_LOWER_LEFT = 8,
};

enum {
	HCC_SPIRV_CAPABILITY_SHADER = 1,
	HCC_SPIRV_CAPABILITY_VULKAN_MEMORY_MODEL = 5345,
	HCC_SPIRV_CAPABILITY_PHYSICAL_STORAGE_BUFFER = 5347,
};

typedef int32_t HccSPIRVStorageClass;
enum {
	HCC_SPIRV_STORAGE_CLASS_UNIFORM_CONSTANT = 0,
	HCC_SPIRV_STORAGE_CLASS_INPUT = 1,
	HCC_SPIRV_STORAGE_CLASS_UNIFORM = 2,
	HCC_SPIRV_STORAGE_CLASS_OUTPUT = 3,
	HCC_SPIRV_STORAGE_CLASS_WORKGROUP = 4,
	HCC_SPIRV_STORAGE_CLASS_PRIVATE = 6,
	HCC_SPIRV_STORAGE_CLASS_FUNCTION = 7,
	HCC_SPIRV_STORAGE_CLASS_PUSH_CONSTANT = 9,
	HCC_SPIRV_STORAGE_CLASS_IMAGE = 11,
	HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER = 12,
	HCC_SPIRV_STORAGE_CLASS_PHYSICAL_STORAGE_BUFFER = 5349,

	HCC_SPIRV_STORAGE_CLASS_INVALID = -1,
};

enum {
	HCC_SPIRV_IMAGE_FORMAT_UNKNOWN = 0,
	HCC_SPIRV_IMAGE_FORMAT_RGBA32F = 1,
	HCC_SPIRV_IMAGE_FORMAT_RGBA16F = 2,
	HCC_SPIRV_IMAGE_FORMAT_R32F = 3,
	HCC_SPIRV_IMAGE_FORMAT_RGBA8 = 4,
	HCC_SPIRV_IMAGE_FORMAT_RGBA8SNORM = 5,
	HCC_SPIRV_IMAGE_FORMAT_RG32F = 6,
	HCC_SPIRV_IMAGE_FORMAT_RG16F = 7,
	HCC_SPIRV_IMAGE_FORMAT_R11FG11FB10F = 8,
	HCC_SPIRV_IMAGE_FORMAT_R16F = 9,
	HCC_SPIRV_IMAGE_FORMAT_RGBA16 = 10,
	HCC_SPIRV_IMAGE_FORMAT_RGB10A2 = 11,
	HCC_SPIRV_IMAGE_FORMAT_RG16 = 12,
	HCC_SPIRV_IMAGE_FORMAT_RG8 = 13,
	HCC_SPIRV_IMAGE_FORMAT_R16 = 14,
	HCC_SPIRV_IMAGE_FORMAT_R8 = 15,
	HCC_SPIRV_IMAGE_FORMAT_RGBA16SNORM = 16,
	HCC_SPIRV_IMAGE_FORMAT_RG16SNORM = 17,
	HCC_SPIRV_IMAGE_FORMAT_RG8SNORM = 18,
	HCC_SPIRV_IMAGE_FORMAT_R16SNORM = 19,
	HCC_SPIRV_IMAGE_FORMAT_R8SNORM = 20,
	HCC_SPIRV_IMAGE_FORMAT_RGBA32I = 21,
	HCC_SPIRV_IMAGE_FORMAT_RGBA16I = 22,
	HCC_SPIRV_IMAGE_FORMAT_RGBA8I = 23,
	HCC_SPIRV_IMAGE_FORMAT_R32I = 24,
	HCC_SPIRV_IMAGE_FORMAT_RG32I = 25,
	HCC_SPIRV_IMAGE_FORMAT_RG16I = 26,
	HCC_SPIRV_IMAGE_FORMAT_RG8I = 27,
	HCC_SPIRV_IMAGE_FORMAT_R16I = 28,
	HCC_SPIRV_IMAGE_FORMAT_R8I = 29,
	HCC_SPIRV_IMAGE_FORMAT_RGBA32UI = 30,
	HCC_SPIRV_IMAGE_FORMAT_RGBA16UI = 31,
	HCC_SPIRV_IMAGE_FORMAT_RGBA8UI = 32,
	HCC_SPIRV_IMAGE_FORMAT_R32UI = 33,
	HCC_SPIRV_IMAGE_FORMAT_RGB10A2UI = 34,
	HCC_SPIRV_IMAGE_FORMAT_RG32UI = 35,
	HCC_SPIRV_IMAGE_FORMAT_RG16UI = 36,
	HCC_SPIRV_IMAGE_FORMAT_RG8UI = 37,
	HCC_SPIRV_IMAGE_FORMAT_R16UI = 38,
	HCC_SPIRV_IMAGE_FORMAT_R8UI = 39,
	HCC_SPIRV_IMAGE_FORMAT_R64UI = 40,
	HCC_SPIRV_IMAGE_FORMAT_R64I = 41,
};

enum {
	HCC_SPIRV_DIM_1D = 0,
	HCC_SPIRV_DIM_2D = 1,
	HCC_SPIRV_DIM_3D = 2,
	HCC_SPIRV_DIM_CUBE = 3,
	HCC_SPIRV_DIM_RECT = 4,
	HCC_SPIRV_DIM_BUFFER = 5,
	HCC_SPIRV_DIM_SUBPASS_DATA = 6,
};

enum {
	HCC_SPIRV_EXECUTION_MODEL_VERTEX                  = 0,
	HCC_SPIRV_EXECUTION_MODEL_TESSELLATION_CONTROL    = 1,
	HCC_SPIRV_EXECUTION_MODEL_TESSELLATION_EVALUATION = 2,
	HCC_SPIRV_EXECUTION_MODEL_GEOMETRY                = 3,
	HCC_SPIRV_EXECUTION_MODEL_FRAGMENT                = 4,
	HCC_SPIRV_EXECUTION_MODEL_GL_COMPUTE              = 5,
};

enum {
	HCC_SPIRV_SELECTION_CONTROL_NONE          = 0,
	HCC_SPIRV_SELECTION_CONTROL_FLATTERN      = 1,
	HCC_SPIRV_SELECTION_CONTROL_DONT_FLATTERN = 2,
};

enum {
	HCC_SPIRV_LOOP_CONTROL_NONE = 0,
};

typedef uint32_t HccSPIRVDecoration;
enum {
	HCC_SPIRV_DECORATION_BUILTIN =  11,
	HCC_SPIRV_DECORATION_FLAT = 14,
	HCC_SPIRV_DECORATION_LOCATION = 30,
};

enum {
	HCC_SPIRV_BUILTIN_POSITION =               0,
	HCC_SPIRV_BUILTIN_POINT_SIZE =             1,
	HCC_SPIRV_BUILTIN_CLIP_DISTANCE =          3,
	HCC_SPIRV_BUILTIN_CULL_DISTANCE =          4,
	HCC_SPIRV_BUILTIN_VIEWPORT_INDEX =         10,
	HCC_SPIRV_BUILTIN_FRAG_COORD =             15,
	HCC_SPIRV_BUILTIN_POINT_COORD =            16,
	HCC_SPIRV_BUILTIN_LOCAL_INVOCATION_INDEX = 29,
	HCC_SPIRV_BUILTIN_VERTEX_INDEX =           42,
	HCC_SPIRV_BUILTIN_INSTANCE_INDEX =         43,
};

enum {
	HCC_SPIRV_FUNCTION_CTRL_NONE         = 0x0,
	HCC_SPIRV_FUNCTION_CTRL_INLINE       = 0x1,
	HCC_SPIRV_FUNCTION_CTRL_DONT_INLINE  = 0x2,
	HCC_SPIRV_FUNCTION_CTRL_PURE         = 0x4,
	HCC_SPIRV_FUNCTION_CTRL_CONST        = 0x8,
};

enum { // some hardcoded SPIR-V ids
	HCC_SPIRV_ID_INVALID,

	HCC_SPIRV_ID_GLSL_STD_450,
	HCC_SPIRV_ID_VARIABLE_INPUT_VERTEX_INDEX,
	HCC_SPIRV_ID_VARIABLE_INPUT_INSTANCE_INDEX,
	HCC_SPIRV_ID_VARIABLE_INPUT_FRAG_COORD,
	HCC_SPIRV_ID_VARIABLE_OUTPUT_POSITION,

	HCC_SPIRV_ID_USER_START,
};

typedef struct HccSPIRVFunction HccSPIRVFunction;
struct HccSPIRVFunction {
	HccSPIRVWord* words;
	uint32_t      words_count;
	uint32_t      words_cap;
	HccSPIRVId*   global_variable_ids;
	uint32_t      global_variables_count;
};

typedef struct HccSPIRVTypeKey HccSPIRVTypeKey;
struct HccSPIRVTypeKey {
	HccSPIRVStorageClass storage_class;
	HccDataType          data_type;
};

typedef struct HccSPIRVTypeEntry HccSPIRVTypeEntry;
struct HccSPIRVTypeEntry {
	HccSPIRVTypeKey       key;
	HccAtomic(HccSPIRVId) spirv_id;
};

typedef struct HccSPIRVTypeOrConstant HccSPIRVTypeOrConstant;
struct HccSPIRVTypeOrConstant {
	HccSPIRVOp       op;
	uint16_t         operands_count;
	HccSPIRVOperand* operands;
};

typedef struct HccSPIRVDeclEntry HccSPIRVDeclEntry;
struct HccSPIRVDeclEntry {
	HccDecl               decl;
	HccAtomic(HccSPIRVId) spirv_id;
};

typedef struct HccSPIRVConstantEntry HccSPIRVConstantEntry;
struct HccSPIRVConstantEntry {
	HccConstantId         constant_id;
	HccAtomic(HccSPIRVId) spirv_id;
};

typedef struct HccSPIRVEntryPoint HccSPIRVEntryPoint;
struct HccSPIRVEntryPoint {
	HccShaderStage shader_stage;
	HccStringId    identifier_string_id;
	HccSPIRVId     spirv_id;
	HccDecl        function_decl;
};

typedef struct HccSPIRV HccSPIRV;
struct HccSPIRV {
	HccStack(HccSPIRVFunction)          functions;
	HccStack(HccSPIRVWord)              function_words;
	HccAtomic(HccSPIRVId)               next_spirv_id;
	HccHashTable(HccSPIRVTypeEntry)     type_table;
	HccHashTable(HccSPIRVDeclEntry)     decl_table;
	HccHashTable(HccSPIRVConstantEntry) constant_table;
	HccStack(HccSPIRVTypeOrConstant)    types_and_constants;
	HccStack(HccSPIRVId)                type_elmt_ids;
	HccStack(HccSPIRVEntryPoint)        entry_points;
	HccStack(HccSPIRVId)                entry_point_global_variable_ids;
	HccStack(HccSPIRVWord)              global_variable_words;
	HccStack(HccSPIRVWord)              decorate_words;

	HccSPIRVWord*                       final_binary_words;
	uint32_t                            final_binary_words_count;
};

void hcc_spirv_init(HccCU* cu, HccCUSetup* setup);
HccSPIRVId hcc_spirv_next_id(HccCU* cu);
HccSPIRVId hcc_spirv_next_id_many(HccCU* cu, uint32_t amount);
HccSPIRVId hcc_spirv_type_deduplicate(HccCU* cu, HccSPIRVStorageClass storage_class, HccDataType data_type);
HccSPIRVId hcc_spirv_decl_deduplicate(HccCU* cu, HccDecl decl);
HccSPIRVId hcc_spirv_constant_deduplicate(HccCU* cu, HccConstantId constant_id);
HccSPIRVStorageClass hcc_spirv_storage_class_from_aml_operand(HccCU* cu, HccAMLOperand aml_operand);
HccSPIRVOperand* hcc_spirv_add_global_variable(HccCU* cu, uint32_t operands_count);
HccSPIRVOperand* hcc_spirv_add_decorate(HccCU* cu, uint32_t operands_count);
HccSPIRVOperand* hcc_spirv_function_add_instr(HccSPIRVFunction* function, HccSPIRVOp op, uint32_t operands_count);
bool hcc_spirv_type_key_cmp(void* a, void* b, uintptr_t size);
HccHash hcc_spirv_type_key_hash(void* key, uintptr_t size);

// ===========================================
//
//
// SPIR-V Generator
//
//
// ===========================================

typedef struct HccSPIRVGen HccSPIRVGen;
struct HccSPIRVGen {
	HccSPIRVFunction*     function;
	const HccAMLFunction* aml_function;
	HccSPIRVId            value_base_id;
	HccSPIRVId            basic_block_base_id;
	HccSPIRVId            basic_block_param_base_id;

	uint16_t              function_unique_globals_count;
	HccSPIRVId            function_unique_globals[HCC_FUNCTION_UNIQUE_GLOBALS_CAP];
};

void hcc_spirvgen_init(HccWorker* w, HccCompilerSetup* setup);
void hcc_spirvgen_deinit(HccWorker* w);
void hcc_spirvgen_reset(HccWorker* w);

HccSPIRVId hcc_spirvgen_convert_operand(HccWorker* w, HccAMLOperand aml_operand);
void hcc_spirvgen_found_global(HccWorker* w, HccSPIRVId spirv_id);

void hcc_spirvgen_generate(HccWorker* w);

// ===========================================
//
//
// SPIR-V Link
//
//
// ===========================================

#define HCC_SPIRVLINK_INSTR_OPERANDS_CAP 256

typedef struct HccSPIRVLink HccSPIRVLink;
struct HccSPIRVLink {
	HccStack(HccSPIRVWord) words;

	HccSPIRVOp             instr_op;
	uint16_t               instr_operands_count;
	HccSPIRVOperand        instr_operands[HCC_SPIRVLINK_INSTR_OPERANDS_CAP];

	uint32_t               function_unique_globals_count;
	HccSPIRVId             function_unique_globals[HCC_FUNCTION_UNIQUE_GLOBALS_CAP];
};

void hcc_spirvlink_init(HccWorker* w, HccCompilerSetup* setup);
void hcc_spirvlink_deinit(HccWorker* w);
void hcc_spirvlink_reset(HccWorker* w);

HccSPIRVWord* hcc_spirvlink_add_word(HccWorker* w);
HccSPIRVWord* hcc_spirvlink_add_word_many(HccWorker* w, uint32_t amount);
HccSPIRVOperand* hcc_spirvlink_add_instr(HccWorker* w, HccSPIRVOp op, uint32_t operands_count);
void hcc_spirvlink_instr_start(HccWorker* w, HccSPIRVOp op);
void hcc_spirvlink_instr_add_operand(HccWorker* w, HccSPIRVWord word);
void hcc_spirvlink_instr_add_operands_string(HccWorker* w, char* string, uint32_t string_size);
#define hcc_spirvlink_instr_add_operands_string_lit(w, string) hcc_spirvlink_instr_add_operands_string(w, string, sizeof(string) - 1)
void hcc_spirvlink_instr_end(HccWorker* w);
bool hcc_spirvlink_add_used_global_variable_ids(HccWorker* w, HccDecl function_decl);

void hcc_spirvlink_link(HccWorker* w);

// ===========================================
//
//
// Compilation Unit
//
//
// ===========================================
//
// this is not a traditional compiliation unit from the sense
// of a single C source file and all of it's includes.
// we actually allow multiple C source files to be compiled
// into a single AST and therefore a single binary.
//

typedef struct HccDeclEntryAtomicLink HccDeclEntryAtomicLink;
struct HccDeclEntryAtomicLink {
	HccDecl decl;
	HccAtomic(HccDeclEntryAtomicLink*) next;
};

#define HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL ((HccDeclEntryAtomicLink*)-1)

typedef struct HccDeclEntryAtomic HccDeclEntryAtomic;
struct HccDeclEntryAtomic {
	HccStringId                        string_id;
	HccAtomic(HccDeclEntryAtomicLink*) link;
};

typedef struct HccCU HccCU;
struct HccCU {
	HccConstantTable         constant_table;
	HccDataTypeTable         dtt;
	HccAST                   ast;
	HccAML                   aml;
	HccSPIRV                 spirv;
	HccAMLScalarDataTypeMask supported_scalar_data_types_mask;
	HccOptions*              options;
	uint32_t                 global_declarations_cap;
	uint32_t                 compounds_cap;
	uint32_t                 enums_cap;

	//
	// these hash tables are all of the declarations from all of the files.
	HccHashTable(HccDeclEntryAtomic) global_declarations;
	HccHashTable(HccDeclEntryAtomic) struct_declarations; // struct T
	HccHashTable(HccDeclEntryAtomic) union_declarations;  // union T
	HccHashTable(HccDeclEntryAtomic) enum_declarations;   // enum T
};

void hcc_cu_init(HccCU* cu, HccCUSetup* setup, HccOptions* options);
void hcc_cu_deinit(HccCU* cu);

// ===========================================
//
//
// Message: Error & Warn
//
//
// ===========================================

typedef uint16_t HccLang;
enum HccLang {
	HCC_LANG_ENG,

	HCC_LANG_COUNT,
};

typedef struct HccMessageSys HccMessageSys;
struct HccMessageSys {
	HccStack(HccMessage)  elmts;
	HccStack(HccLocation) locations;
	HccStack(char)        strings;
	HccMessageType        used_type_flags;
};

extern const char* hcc_message_type_lang_strings[HCC_LANG_COUNT][HCC_MESSAGE_TYPE_COUNT];
extern const char* hcc_message_type_ascii_color_code[HCC_MESSAGE_TYPE_COUNT];
extern const char* hcc_error_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_ERROR_CODE_COUNT];
extern const char* hcc_warn_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_WARN_CODE_COUNT];

void hcc_message_print_file_line(HccIIO* iio, HccLocation* location);
void hcc_message_print_pasted_buffer(HccIIO* iio, uint32_t line, uint32_t column);
void hcc_message_print_code_line(HccIIO* iio, HccLocation* location, uint32_t display_line_num_size, uint32_t line, uint32_t display_line);
void hcc_message_print_code(HccIIO* iio, HccLocation* location);

// ===========================================
//
//
// Compiler Options
//
//
// ===========================================

typedef struct HccOptionDefine HccOptionDefine;
struct HccOptionDefine {
	HccString name;
	HccString value;
};

typedef struct HccOptions HccOptions;
struct HccOptions {
	HccOptionValue            key_to_value_map[HCC_OPTION_KEY_COUNT];
	uint64_t                  is_set_bitset[HCC_DIV_ROUND_UP(HCC_OPTION_KEY_COUNT, 64)];
	HccStack(HccOptionDefine) defines;
};

// ===========================================
//
//
// Compiler Task
//
//
// ===========================================

typedef uint8_t HccEncoding;
enum HccEncoding {
	HCC_ENCODING_TEXT,
	HCC_ENCODING_BINARY,
	HCC_ENCODING_RUNTIME_BINARY,
};

typedef struct HccTaskInputLocation HccTaskInputLocation;
struct HccTaskInputLocation {
	HccTaskInputLocation* next;
	HccOptions*           options;
	HccString             file_path; // canonicalized
};

typedef struct HccTaskOutputLocation HccTaskOutputLocation;
struct HccTaskOutputLocation {
	HccEncoding         encoding;
	HccWorkerJobType    worker_job_type;
	void*               arg;
};

typedef uint8_t HccTaskFlags;
enum HccTaskFlags {
	HCC_TASK_FLAGS_NONE =          0x0,
	HCC_TASK_FLAGS_IS_RESULT_SET = 0x1,
};

typedef struct HccTask HccTask;
struct HccTask {
	HccAtomic(HccTaskFlags) flags;
	HccCUSetup              cu_setup;
	HccCompiler*            c;
	HccOptions*             options;
	HccResult               result;
	HccWorkerJobType        worker_job_type;
	HccWorkerJobType        final_worker_job_type;
	HccTaskInputLocation*   input_locations;
	HccTaskOutputLocation   output_job_locations[HCC_WORKER_JOB_TYPE_COUNT];
	HccMessageSys           message_sys;
	HccStack(HccString)     include_path_strings;
	HccMutex                is_running_mutex;
	HccAtomic(uint32_t)     queued_jobs_count;
	HccTime                 worker_job_type_start_times[HCC_WORKER_JOB_TYPE_COUNT];
	HccDuration             worker_job_type_durations[HCC_WORKER_JOB_TYPE_COUNT];
	HccDuration             duration;
	HccTime                 start_time;

	HccCU*                  cu;
};

HccTaskInputLocation* hcc_task_input_location_init(HccTask* t, HccOptions* options);
HccResult hcc_task_add_output(HccTask* t, HccWorkerJobType job_type, HccEncoding encoding, void* arg);
void hcc_task_output_job(HccTask* t, HccWorkerJobType job_type);
void hcc_task_finish(HccTask* t, bool was_successful);

// ===========================================
//
//
// Worker
//
//
// ===========================================

typedef struct HccWorkerJob HccWorkerJob;
struct HccWorkerJob {
	HccWorkerJobType type;
	HccTask*         task;
	void*            arg;
};

typedef struct HccWorker HccWorker;
struct HccWorker {
	HccCompiler*   c;
	HccCU*         cu;
	HccWorkerJob   job;
	HccThread      thread;
	HccTime        job_start_time;
	uint8_t        initialized_generators_bitset;
	HccStack(char) string_buffer;
	HccArenaAlctor arena_alctor;

	HccATAGen      atagen;
	HccASTGen      astgen;
	HccASTLink     astlink;
	HccAMLGen      amlgen;
	HccAMLOpt      amlopt;
	HccSPIRVGen    spirvgen;
	HccSPIRVLink   spirvlink;
};

void hcc_worker_init(HccWorker* w, HccCompiler* c, void* call_stack, uintptr_t call_stack_size, HccCompilerSetup* setup);
void hcc_worker_deinit(HccWorker* w);
HccLocation* hcc_worker_alloc_location(HccWorker* w);
HccTask* hcc_worker_task(HccWorker* w);
HccCU* hcc_worker_cu(HccWorker* w);
void hcc_worker_start_job(HccWorker* w);
void hcc_worker_end_job(HccWorker* w);
void hcc_worker_main(void* arg);

// ===========================================
//
//
// Compiler
//
//
// ===========================================


typedef uint32_t HccCompilerFlags;
enum HccCompilerFlags {
	HCC_COMPILER_FLAGS_NONE =             0x0,
	HCC_COMPILER_FLAGS_CHAR_IS_UNSIGNED = 0x1,
	HCC_COMPILER_FLAGS_IS_RESULT_SET =    0x2,
	HCC_COMPILER_FLAGS_IS_STOPPING =      0x4,
};

typedef struct HccCompiler HccCompiler;
struct HccCompiler {
	HccAtomic(HccCompilerFlags)    flags;
	HccCompilerSetup               setup;
	uint32_t                       workers_count;
	HccWorker*                     workers;
	void*                          worker_call_stacks_addr;
	uintptr_t                      worker_call_stacks_size;
	HccAtomic(uint32_t)            tasks_running_count;
	HccMutex                       wait_for_all_mutex;
	HccResultData                  result_data;
	HccDuration                    worker_job_type_durations[HCC_WORKER_JOB_TYPE_COUNT];
	HccDuration                    duration;
	HccTime                        start_time;
	struct {
		HccWorkerJob*              data;
		uint32_t                   cap;
		HccAtomic(uint32_t)        head_idx;
		HccAtomic(uint32_t)        tail_idx;
		HccSemaphore               semaphore;
	} worker_job_queue;
};

void hcc_compiler_give_worker_job(HccCompiler* c, HccTask* t, HccWorkerJobType job_type, void* arg);
bool hcc_compiler_take_or_wait_then_take_worker_job(HccCompiler* c, HccWorkerJob* job_out);

// ===========================================
//
//
// String Table
//
//
// ===========================================
//
// used to hold a unique set of strings so the user
// can reduce the string down to a uint32_t identifier.
// this identifier can be used to uniquely identify the string.
// so a comparision with the two identifier integers can be used to compare strings.
//

typedef struct HccStringEntry HccStringEntry;
struct HccStringEntry {
	HccString           string;
	HccAtomic(uint32_t) id;
};

typedef struct HccStringTable HccStringTable;
struct HccStringTable {
	HccHashTable(HccStringEntry) entries_hash_table;
	HccStack(uint32_t)           id_to_entry_map;
	HccStack(char)               data;
	HccAtomic(uint32_t)          next_id;
};

enum {
	HCC_STRING_ID_NULL = 0,

	HCC_STRING_ID_KEYWORDS_START,
#define HCC_STRING_ID_KEYWORDS_END (HCC_STRING_ID_KEYWORDS_START + HCC_ATA_TOKEN_KEYWORDS_COUNT)

	HCC_STRING_ID_PREDEFINED_MACROS_START = HCC_STRING_ID_KEYWORDS_END,
#define HCC_STRING_ID_PREDEFINED_MACROS_END (HCC_STRING_ID_PREDEFINED_MACROS_START + HCC_PP_PREDEFINED_MACRO_COUNT)

	HCC_STRING_ID_ONCE = HCC_STRING_ID_PREDEFINED_MACROS_END,
	HCC_STRING_ID_DEFINED,
	HCC_STRING_ID___VA_ARGS__,
	HCC_STRING_ID_X,
	HCC_STRING_ID_Y,
	HCC_STRING_ID_Z,
	HCC_STRING_ID_W,
	HCC_STRING_ID_R,
	HCC_STRING_ID_G,
	HCC_STRING_ID_B,
	HCC_STRING_ID_A,

#define HCC_STRING_ID_INTRINSIC_START HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_START

	HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_START,
#define HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_END (HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_START + HCC_COMPOUND_DATA_TYPE_IDX_INTRINSICS_COUNT)

	HCC_STRING_ID_INTRINSIC_FUNCTIONS_START = HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_END,
#define HCC_STRING_ID_INTRINSIC_FUNCTIONS_END (HCC_STRING_ID_INTRINSIC_FUNCTIONS_START + HCC_FUNCTION_IDX_INTRINSICS_COUNT)

	HCC_STRING_ID_USER_START = HCC_STRING_ID_INTRINSIC_FUNCTIONS_END,
#define HCC_STRING_ID_INTRINSIC_END HCC_STRING_ID_USER_START
};

void hcc_string_table_init(HccStringTable* string_table, uint32_t data_grow_count, uint32_t data_reserve_cap, uint32_t entries_cap);
void hcc_string_table_deinit(HccStringTable* string_table);
HccStringId hcc_string_table_alloc_next_id(HccStringTable* string_table);
void hcc_string_table_skip_next_ids(HccStringTable* string_table, uint32_t num);

// ===========================================
//
//
// Globals
//
//
// ===========================================

typedef struct HccCodeFileEntry HccCodeFileEntry;
struct HccCodeFileEntry {
	HccString   path_string;
	HccCodeFile file;
};

typedef struct HccGS HccGS;
struct HccGS {
	HccFlags                       flags;
	uintptr_t                      virt_mem_page_size;
	uintptr_t                      virt_mem_reserve_align;
	HccAllocEventFn                alloc_event_fn;
	HccPathCanonicalizeFn          path_canonicalize_fn;
	HccFileOpenReadFn              file_open_read_fn;
	HccArenaAlctor                 arena_alctor;
	void*                          alloc_event_userdata;
	HccStringTable                 string_table;
	HccHashTable(HccCodeFileEntry) path_to_code_file_map;
	uint32_t                       code_file_lines_grow_count;
	uint32_t                       code_file_lines_reserve_cap;
	uint32_t                       code_file_pp_if_spans_grow_count;
	uint32_t                       code_file_pp_if_spans_reserve_cap;
};

extern HccGS _hcc_gs;

typedef struct HccTLS HccTLS;
struct HccTLS {
	HccCompiler*    c;
	HccWorker*      w;
	HccResultData   result_data;
	HccResultData*  jmp_result_data;
	jmp_buf         jmp_loc;
	uint32_t        jmp_loc_recursive_set_count;
};

extern thread_local HccTLS _hcc_tls;

