#pragma once

#include <setjmp.h> // jmp_buf
#include <stdatomic.h>
#include <immintrin.h>
#include <signal.h>

#include "hcc.h"

typedef struct HccWorker HccWorker;
HCC_DEFINE_ID(HccASTExprId);

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

uint32_t onebitscount32(uint32_t bits);
uint32_t leastsetbitidx32(uint32_t bits);
void hcc_get_last_system_error_string(char* buf_out, uint32_t buf_out_size);
bool hcc_file_exist(const char* path);
bool hcc_change_working_directory(const char* path);
uint32_t hcc_path_canonicalize_internal(const char* path, char* out_buf);
HccString hcc_path_canonicalize(const char* path);
bool hcc_path_is_absolute(const char* path);
char* hcc_file_read_all_the_codes(const char* path, uint64_t* size_out);
void hcc_stacktrace(uint32_t ignore_levels_count, char* buf, uint32_t buf_size);
bool hcc_file_open_read(const char* path, HccIIO* out);
bool hcc_path_is_relative(const char* path);
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

#define hcc_stack_push_thread_safe(stack) (&(stack)[_hcc_stack_push_many(stack, 1, sizeof(*(stack)))])
#define hcc_stack_push_many_thread_safe(stack, amount) (&(stack)[_hcc_stack_push_many(stack, amount, sizeof(*(stack)))])
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

typedef struct HccHashTableHeader HccHashTableHeader;
struct HccHashTableHeader {
	alignas(hcc_max_align_t)
	HccAtomic(uintptr_t) count;
	uintptr_t            cap;
	HccHashTableKeyCmpFn key_cmp_fn;
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

#define hcc_hash_table_init(KVEntry, tag, key_cmp_fn, cap) _hcc_hash_table_init(tag, key_cmp_fn, cap, sizeof(KVEntry))
HccHashTable(void) _hcc_hash_table_init(HccAllocTag tag, HccHashTableKeyCmpFn key_cmp_fn, uintptr_t cap, uintptr_t elmt_size);

#define hcc_hash_table_deinit(table) _hcc_hash_table_deinit(table, sizeof(*(table)))
void _hcc_hash_table_deinit(HccHashTable(void) table, uintptr_t elmt_size);

#define hcc_hash_table_clear(table) _hcc_hash_table_clear(table, sizeof(*(table)))
void _hcc_hash_table_clear(HccHashTable(void) table, uintptr_t elmt_size);

#define hcc_hash_table_find_idx(table, key) _hcc_hash_table_find_idx(table, key, sizeof(*(key)), sizeof(*(table)))
#define hcc_hash_table_find_idx_string(table, key) _hcc_hash_table_find_idx(table, key, 0, sizeof(*(table)))
uintptr_t _hcc_hash_table_find_idx(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size);

#define hcc_hash_table_find_insert_idx(table, key) _hcc_hash_table_find_insert_idx(table, key, sizeof(*(key)), sizeof(*(table)))
#define hcc_hash_table_find_insert_idx_string(table, key) _hcc_hash_table_find_insert_idx(table, key, 0, sizeof(*(table)))
HccHashTableInsert _hcc_hash_table_find_insert_idx(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size);

#define hcc_hash_table_remove(table, key) _hcc_hash_table_remove(table, key, sizeof(*(key)), sizeof(*(table)))
#define hcc_hash_table_remove_string(table, key) _hcc_hash_table_remove(table, key, 0, sizeof(*(table)))
bool _hcc_hash_table_remove(HccHashTable(void) table, void* key, uintptr_t key_size, uintptr_t elmt_size);

bool hcc_u32_key_cmp(void* a, void* b, uintptr_t size);

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
	HccIntrinsicBasicTypeMask has_intrinsic_basic_types;
	HccCompoundDataTypeFlags  flags;
	HccCompoundDataTypeKind   kind;
	uint8_t                   resource_set_slot;
};

typedef struct HccCompoundField HccCompoundField;
struct HccCompoundField {
	HccLocation*             identifier_location;
	HccStringId              identifier_string_id;
	HccDataType              data_type;
	HccRasterizerStateFieldKind rasterizer_state_field_kind;
};

typedef struct HccEnumDataType HccEnumDataType;
struct HccEnumDataType {
	HccLocation*  identifier_location;
	HccEnumValue* values;
	uint32_t      values_count;
	HccStringId   identifier_string_id;
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

typedef struct HccDataTypeTable HccDataTypeTable;
struct HccDataTypeTable {
	uint8_t*                      basic_type_size_and_aligns; // [HCC_AST_BASIC_DATA_TYPE_COUNT]
	uint64_t*                     basic_type_int_mins; // [HCC_AST_BASIC_DATA_TYPE_COUNT]
	uint64_t*                     basic_type_int_maxes; // [HCC_AST_BASIC_DATA_TYPE_COUNT]
	HccConstantId                 basic_type_zero_constant_ids[HCC_AST_BASIC_DATA_TYPE_COUNT];
	HccConstantId                 basic_type_one_constant_ids[HCC_AST_BASIC_DATA_TYPE_COUNT];
	HccConstantId                 basic_type_minus_one_constant_ids[HCC_AST_BASIC_DATA_TYPE_COUNT];
	HccStack(HccArrayDataType)    arrays;
	HccStack(HccCompoundDataType) compounds;
	HccStack(HccCompoundField)    compound_fields;
	HccStack(HccTypedef)          typedefs;
	HccStack(HccEnumDataType)     enums;
	HccStack(HccEnumValue)        enum_values;
	HccStack(HccBufferDataType)   buffers;
	HccStack(HccPointerDataType)  pointers;
};

#define HCC_MATRIX_SCALAR_TYPES_COUNT 3 // F16, F32, F64

enum {
	HCC_TYPEDEF_IDX_UINT8,
	HCC_TYPEDEF_IDX_UINT16,
	HCC_TYPEDEF_IDX_UINT32,
	HCC_TYPEDEF_IDX_UINT64,
	HCC_TYPEDEF_IDX_UINTPTR,
	HCC_TYPEDEF_IDX_INT8,
	HCC_TYPEDEF_IDX_INT16,
	HCC_TYPEDEF_IDX_INT32,
	HCC_TYPEDEF_IDX_INT64,
	HCC_TYPEDEF_IDX_INTPTR,
	HCC_TYPEDEF_IDX_HALF,

	HCC_TYPEDEF_IDX_SCALARX_START,
#define HCC_TYPEDEF_IDX_SCALARX_END (HCC_TYPEDEF_IDX_SCALARX_START + (HCC_AML_INTRINSIC_DATA_TYPE_COUNT))

	HCC_TYPEDEF_IDX_PSCALARX_START = HCC_TYPEDEF_IDX_SCALARX_END,
#define HCC_TYPEDEF_IDX_PSCALARX_END (HCC_TYPEDEF_IDX_PSCALARX_START + (HCC_AML_INTRINSIC_DATA_TYPE_COUNT))

	HCC_TYPEDEF_IDX_VERTEX_INPUT = HCC_TYPEDEF_IDX_PSCALARX_END,
	HCC_TYPEDEF_IDX_FRAGMENT_INPUT,

#define HCC_TYPEDEF_IDX_INTRINSIC_END HCC_TYPEDEF_IDX_USER_START
	HCC_TYPEDEF_IDX_USER_START,
};

enum {
	HCC_COMPOUND_DATA_TYPE_IDX_VERTEX_INPUT,
	HCC_COMPOUND_DATA_TYPE_IDX_FRAGMENT_INPUT,
	HCC_COMPOUND_DATA_TYPE_IDX_HALF,

#define HCC_COMPOUND_DATA_TYPE_IDX_INTRINSIC_END HCC_COMPOUND_DATA_TYPE_IDX_SCALARX_START

	HCC_COMPOUND_DATA_TYPE_IDX_SCALARX_START,
#define HCC_COMPOUND_DATA_TYPE_IDX_SCALARX_END (HCC_COMPOUND_DATA_TYPE_IDX_SCALARX_START + (HCC_AML_INTRINSIC_DATA_TYPE_COUNT))

	HCC_COMPOUND_DATA_TYPE_IDX_PSCALARX_START,
#define HCC_COMPOUND_DATA_TYPE_IDX_PSCALARX_END (HCC_COMPOUND_DATA_TYPE_IDX_PSCALARX_START + (HCC_AML_INTRINSIC_DATA_TYPE_COUNT))

	HCC_COMPOUND_DATA_TYPE_IDX_USER_START = HCC_COMPOUND_DATA_TYPE_IDX_PSCALARX_END,
};

void hcc_data_type_table_init(HccCU* cu, HccDataTypeTableSetup* setup);
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
	void*                     data;
	uint32_t                  size;
	HccAtomic(HccDataType) data_type;
	HccStringId               debug_string_id;
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
HccConstantId _hcc_constant_table_deduplicate_end(HccCU* cu, HccDataType data_type, void* data, uint32_t data_size, uint32_t data_align, HccStringId debug_string_id);

// ===========================================
//
//
// AST Declarations
//
//
// ===========================================

typedef struct HccASTVariable HccASTVariable;
struct HccASTVariable {
	HccLocation*  identifier_location;
	HccStringId   identifier_string_id;
	HccDataType   data_type;
	HccConstantId initializer_constant_id; // if is_static
	uint32_t      is_static: 1;
};

typedef uint8_t HccASTFunctionFlags;
enum HccASTFunctionFlags {
	HCC_AST_FUNCTION_FLAGS_STATIC = 0x1,
	HCC_AST_FUNCTION_FLAGS_INLINE = 0x2,
};

#define HCC_FUNCTION_MAX_PARAMS_COUNT 32

typedef struct HccASTFunction HccASTFunction;
struct HccASTFunction {
	HccLocation*              identifier_location;
	HccLocation*              return_data_type_location;
	HccASTVariable*           params_and_variables;
	HccStringId               identifier_string_id;
	HccDataType               return_data_type;
	uint16_t                  variables_count;
	uint8_t                   params_count;
	HccASTFunctionFlags       flags;
	HccASTFunctionShaderStage shader_stage;
	HccASTExprId              block_expr_id;
	uint32_t                  used_function_indices_start_idx;
	uint32_t                  used_function_indices_count;
	uint32_t                  used_static_variables_start_idx;
	uint32_t                  used_static_variables_count;
	uint32_t                  unsupported_basic_types_deferred_messages_start_idx;
	uint32_t                  unsupported_basic_types_deferred_messages_count;
	uint32_t                  blocks_start_idx;
};

enum {
	HCC_FUNCTION_IDX_FMODF,
	HCC_FUNCTION_IDX_FMOD,
	HCC_FUNCTION_IDX_F16TOF32,
	HCC_FUNCTION_IDX_F16TOF64,
	HCC_FUNCTION_IDX_F32TOF16,
	HCC_FUNCTION_IDX_F64TOF16,
	HCC_FUNCTION_IDX_ADDF16,
	HCC_FUNCTION_IDX_SUBF16,
	HCC_FUNCTION_IDX_MULF16,
	HCC_FUNCTION_IDX_DIVF16,
	HCC_FUNCTION_IDX_MODF16,
	HCC_FUNCTION_IDX_EQF16,
	HCC_FUNCTION_IDX_NEQF16,
	HCC_FUNCTION_IDX_LTF16,
	HCC_FUNCTION_IDX_LTEQF16,
	HCC_FUNCTION_IDX_GTF16,
	HCC_FUNCTION_IDX_GTEQF16,
	HCC_FUNCTION_IDX_NOTF16,
	HCC_FUNCTION_IDX_NEGF16,

#define HCC_FUNCTION_IDX_IS_VECTOR_SWIZZLE(function_idx) \
	(HCC_FUNCTION_IDX_VECTOR_SWIZZLE_START <= (function_idx) && function_idx < HCC_FUNCTION_IDX_VECTOR_SWIZZLE_END)

#define HCC_FUNCTION_IDX_VECTOR_SWIZZLE_START HCC_FUNCTION_IDX_SWIZZLEV2F16

	//
	// libhccstd/math.h - vector
	//
	HCC_FUNCTION_IDX_SWIZZLEV2F16,
	HCC_FUNCTION_IDX_SWIZZLEV2F32,
	HCC_FUNCTION_IDX_SWIZZLEV2F64,
	HCC_FUNCTION_IDX_SWIZZLEV2I8,
	HCC_FUNCTION_IDX_SWIZZLEV2I16,
	HCC_FUNCTION_IDX_SWIZZLEV2I32,
	HCC_FUNCTION_IDX_SWIZZLEV2I64,
	HCC_FUNCTION_IDX_SWIZZLEV2U8,
	HCC_FUNCTION_IDX_SWIZZLEV2U16,
	HCC_FUNCTION_IDX_SWIZZLEV2U32,
	HCC_FUNCTION_IDX_SWIZZLEV2U64,
	HCC_FUNCTION_IDX_SWIZZLEV3F16,
	HCC_FUNCTION_IDX_SWIZZLEV3F32,
	HCC_FUNCTION_IDX_SWIZZLEV3F64,
	HCC_FUNCTION_IDX_SWIZZLEV3I8,
	HCC_FUNCTION_IDX_SWIZZLEV3I16,
	HCC_FUNCTION_IDX_SWIZZLEV3I32,
	HCC_FUNCTION_IDX_SWIZZLEV3I64,
	HCC_FUNCTION_IDX_SWIZZLEV3U8,
	HCC_FUNCTION_IDX_SWIZZLEV3U16,
	HCC_FUNCTION_IDX_SWIZZLEV3U32,
	HCC_FUNCTION_IDX_SWIZZLEV3U64,
	HCC_FUNCTION_IDX_SWIZZLEV4F16,
	HCC_FUNCTION_IDX_SWIZZLEV4F32,
	HCC_FUNCTION_IDX_SWIZZLEV4F64,
	HCC_FUNCTION_IDX_SWIZZLEV4I8,
	HCC_FUNCTION_IDX_SWIZZLEV4I16,
	HCC_FUNCTION_IDX_SWIZZLEV4I32,
	HCC_FUNCTION_IDX_SWIZZLEV4I64,
	HCC_FUNCTION_IDX_SWIZZLEV4U8,
	HCC_FUNCTION_IDX_SWIZZLEV4U16,
	HCC_FUNCTION_IDX_SWIZZLEV4U32,
	HCC_FUNCTION_IDX_SWIZZLEV4U64,

#define HCC_FUNCTION_IDX_VECTOR_SWIZZLE_END (HCC_FUNCTION_IDX_SWIZZLEV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_PACK(function_idx) \
	(HCC_FUNCTION_IDX_VECTOR_PACK_START <= (function_idx) && function_idx < HCC_FUNCTION_IDX_VECTOR_PACK_END)
#define HCC_FUNCTION_IDX_VECTOR_PACK_START HCC_FUNCTION_IDX_PACKV2BOOL

	HCC_FUNCTION_IDX_PACKV2BOOL,
	HCC_FUNCTION_IDX_PACKV2F16,
	HCC_FUNCTION_IDX_PACKV2F32,
	HCC_FUNCTION_IDX_PACKV2F64,
	HCC_FUNCTION_IDX_PACKV2I8,
	HCC_FUNCTION_IDX_PACKV2I16,
	HCC_FUNCTION_IDX_PACKV2I32,
	HCC_FUNCTION_IDX_PACKV2I64,
	HCC_FUNCTION_IDX_PACKV2U8,
	HCC_FUNCTION_IDX_PACKV2U16,
	HCC_FUNCTION_IDX_PACKV2U32,
	HCC_FUNCTION_IDX_PACKV2U64,
	HCC_FUNCTION_IDX_PACKV3BOOL,
	HCC_FUNCTION_IDX_PACKV3F16,
	HCC_FUNCTION_IDX_PACKV3F32,
	HCC_FUNCTION_IDX_PACKV3F64,
	HCC_FUNCTION_IDX_PACKV3I8,
	HCC_FUNCTION_IDX_PACKV3I16,
	HCC_FUNCTION_IDX_PACKV3I32,
	HCC_FUNCTION_IDX_PACKV3I64,
	HCC_FUNCTION_IDX_PACKV3U8,
	HCC_FUNCTION_IDX_PACKV3U16,
	HCC_FUNCTION_IDX_PACKV3U32,
	HCC_FUNCTION_IDX_PACKV3U64,
	HCC_FUNCTION_IDX_PACKV4BOOL,
	HCC_FUNCTION_IDX_PACKV4F16,
	HCC_FUNCTION_IDX_PACKV4F32,
	HCC_FUNCTION_IDX_PACKV4F64,
	HCC_FUNCTION_IDX_PACKV4I8,
	HCC_FUNCTION_IDX_PACKV4I16,
	HCC_FUNCTION_IDX_PACKV4I32,
	HCC_FUNCTION_IDX_PACKV4I64,
	HCC_FUNCTION_IDX_PACKV4U8,
	HCC_FUNCTION_IDX_PACKV4U16,
	HCC_FUNCTION_IDX_PACKV4U32,
	HCC_FUNCTION_IDX_PACKV4U64,

#define HCC_FUNCTION_IDX_VECTOR_PACK_END (HCC_FUNCTION_IDX_PACKV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_UNPACK(function_idx) \
	(HCC_FUNCTION_IDX_VECTOR_UNPACK_START <= (function_idx) && function_idx < HCC_FUNCTION_IDX_VECTOR_UNPACK_END)
#define HCC_FUNCTION_IDX_VECTOR_UNPACK_START HCC_FUNCTION_IDX_UNPACKV2BOOL

	HCC_FUNCTION_IDX_UNPACKV2BOOL,
	HCC_FUNCTION_IDX_UNPACKV2F16,
	HCC_FUNCTION_IDX_UNPACKV2F32,
	HCC_FUNCTION_IDX_UNPACKV2F64,
	HCC_FUNCTION_IDX_UNPACKV2I8,
	HCC_FUNCTION_IDX_UNPACKV2I16,
	HCC_FUNCTION_IDX_UNPACKV2I32,
	HCC_FUNCTION_IDX_UNPACKV2I64,
	HCC_FUNCTION_IDX_UNPACKV2U8,
	HCC_FUNCTION_IDX_UNPACKV2U16,
	HCC_FUNCTION_IDX_UNPACKV2U32,
	HCC_FUNCTION_IDX_UNPACKV2U64,
	HCC_FUNCTION_IDX_UNPACKV3BOOL,
	HCC_FUNCTION_IDX_UNPACKV3F16,
	HCC_FUNCTION_IDX_UNPACKV3F32,
	HCC_FUNCTION_IDX_UNPACKV3F64,
	HCC_FUNCTION_IDX_UNPACKV3I8,
	HCC_FUNCTION_IDX_UNPACKV3I16,
	HCC_FUNCTION_IDX_UNPACKV3I32,
	HCC_FUNCTION_IDX_UNPACKV3I64,
	HCC_FUNCTION_IDX_UNPACKV3U8,
	HCC_FUNCTION_IDX_UNPACKV3U16,
	HCC_FUNCTION_IDX_UNPACKV3U32,
	HCC_FUNCTION_IDX_UNPACKV3U64,
	HCC_FUNCTION_IDX_UNPACKV4BOOL,
	HCC_FUNCTION_IDX_UNPACKV4F16,
	HCC_FUNCTION_IDX_UNPACKV4F32,
	HCC_FUNCTION_IDX_UNPACKV4F64,
	HCC_FUNCTION_IDX_UNPACKV4I8,
	HCC_FUNCTION_IDX_UNPACKV4I16,
	HCC_FUNCTION_IDX_UNPACKV4I32,
	HCC_FUNCTION_IDX_UNPACKV4I64,
	HCC_FUNCTION_IDX_UNPACKV4U8,
	HCC_FUNCTION_IDX_UNPACKV4U16,
	HCC_FUNCTION_IDX_UNPACKV4U32,
	HCC_FUNCTION_IDX_UNPACKV4U64,

#define HCC_FUNCTION_IDX_VECTOR_UNPACK_END (HCC_FUNCTION_IDX_UNPACKV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_ANY_OR_ALL(function_idx) \
	(HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_START <= (function_idx) && function_idx < HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_END)

#define HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_START HCC_FUNCTION_IDX_ANYV2F16

	HCC_FUNCTION_IDX_ANYV2F16,
	HCC_FUNCTION_IDX_ANYV2F32,
	HCC_FUNCTION_IDX_ANYV2F64,
	HCC_FUNCTION_IDX_ANYV2I8,
	HCC_FUNCTION_IDX_ANYV2I16,
	HCC_FUNCTION_IDX_ANYV2I32,
	HCC_FUNCTION_IDX_ANYV2I64,
	HCC_FUNCTION_IDX_ANYV2U8,
	HCC_FUNCTION_IDX_ANYV2U16,
	HCC_FUNCTION_IDX_ANYV2U32,
	HCC_FUNCTION_IDX_ANYV2U64,
	HCC_FUNCTION_IDX_ANYV3F16,
	HCC_FUNCTION_IDX_ANYV3F32,
	HCC_FUNCTION_IDX_ANYV3F64,
	HCC_FUNCTION_IDX_ANYV3I8,
	HCC_FUNCTION_IDX_ANYV3I16,
	HCC_FUNCTION_IDX_ANYV3I32,
	HCC_FUNCTION_IDX_ANYV3I64,
	HCC_FUNCTION_IDX_ANYV3U8,
	HCC_FUNCTION_IDX_ANYV3U16,
	HCC_FUNCTION_IDX_ANYV3U32,
	HCC_FUNCTION_IDX_ANYV3U64,
	HCC_FUNCTION_IDX_ANYV4F16,
	HCC_FUNCTION_IDX_ANYV4F32,
	HCC_FUNCTION_IDX_ANYV4F64,
	HCC_FUNCTION_IDX_ANYV4I8,
	HCC_FUNCTION_IDX_ANYV4I16,
	HCC_FUNCTION_IDX_ANYV4I32,
	HCC_FUNCTION_IDX_ANYV4I64,
	HCC_FUNCTION_IDX_ANYV4U8,
	HCC_FUNCTION_IDX_ANYV4U16,
	HCC_FUNCTION_IDX_ANYV4U32,
	HCC_FUNCTION_IDX_ANYV4U64,
	HCC_FUNCTION_IDX_ALLV2F16,
	HCC_FUNCTION_IDX_ALLV2F32,
	HCC_FUNCTION_IDX_ALLV2F64,
	HCC_FUNCTION_IDX_ALLV2I8,
	HCC_FUNCTION_IDX_ALLV2I16,
	HCC_FUNCTION_IDX_ALLV2I32,
	HCC_FUNCTION_IDX_ALLV2I64,
	HCC_FUNCTION_IDX_ALLV2U8,
	HCC_FUNCTION_IDX_ALLV2U16,
	HCC_FUNCTION_IDX_ALLV2U32,
	HCC_FUNCTION_IDX_ALLV2U64,
	HCC_FUNCTION_IDX_ALLV3F16,
	HCC_FUNCTION_IDX_ALLV3F32,
	HCC_FUNCTION_IDX_ALLV3F64,
	HCC_FUNCTION_IDX_ALLV3I8,
	HCC_FUNCTION_IDX_ALLV3I16,
	HCC_FUNCTION_IDX_ALLV3I32,
	HCC_FUNCTION_IDX_ALLV3I64,
	HCC_FUNCTION_IDX_ALLV3U8,
	HCC_FUNCTION_IDX_ALLV3U16,
	HCC_FUNCTION_IDX_ALLV3U32,
	HCC_FUNCTION_IDX_ALLV3U64,
	HCC_FUNCTION_IDX_ALLV4F16,
	HCC_FUNCTION_IDX_ALLV4F32,
	HCC_FUNCTION_IDX_ALLV4F64,
	HCC_FUNCTION_IDX_ALLV4I8,
	HCC_FUNCTION_IDX_ALLV4I16,
	HCC_FUNCTION_IDX_ALLV4I32,
	HCC_FUNCTION_IDX_ALLV4I64,
	HCC_FUNCTION_IDX_ALLV4U8,
	HCC_FUNCTION_IDX_ALLV4U16,
	HCC_FUNCTION_IDX_ALLV4U32,
	HCC_FUNCTION_IDX_ALLV4U64,

#define HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_END (HCC_FUNCTION_IDX_ALLV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_ANY_OR_ALL_BOOL(function_idx) \
	(HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_BOOL_START <= (function_idx) && function_idx < HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_BOOL_END)
#define HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_BOOL_START HCC_FUNCTION_IDX_ANYV2BOOL

	HCC_FUNCTION_IDX_ANYV2BOOL,
	HCC_FUNCTION_IDX_ANYV3BOOL,
	HCC_FUNCTION_IDX_ANYV4BOOL,
	HCC_FUNCTION_IDX_ALLV2BOOL,
	HCC_FUNCTION_IDX_ALLV3BOOL,
	HCC_FUNCTION_IDX_ALLV4BOOL,

#define HCC_FUNCTION_IDX_VECTOR_ANY_OR_ALL_BOOL_END (HCC_FUNCTION_IDX_ALLV4BOOL + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_NOT(function_idx) \
	(HCC_FUNCTION_IDX_VECTOR_NOT_START <= (function_idx) && function_idx < HCC_FUNCTION_IDX_VECTOR_NOT_END)
#define HCC_FUNCTION_IDX_VECTOR_NOT_START HCC_FUNCTION_IDX_NOTV2F16
	HCC_FUNCTION_IDX_NOTV2F16,
	HCC_FUNCTION_IDX_NOTV2F32,
	HCC_FUNCTION_IDX_NOTV2F64,
	HCC_FUNCTION_IDX_NOTV2I8,
	HCC_FUNCTION_IDX_NOTV2I16,
	HCC_FUNCTION_IDX_NOTV2I32,
	HCC_FUNCTION_IDX_NOTV2I64,
	HCC_FUNCTION_IDX_NOTV2U8,
	HCC_FUNCTION_IDX_NOTV2U16,
	HCC_FUNCTION_IDX_NOTV2U32,
	HCC_FUNCTION_IDX_NOTV2U64,
	HCC_FUNCTION_IDX_NOTV3F16,
	HCC_FUNCTION_IDX_NOTV3F32,
	HCC_FUNCTION_IDX_NOTV3F64,
	HCC_FUNCTION_IDX_NOTV3I8,
	HCC_FUNCTION_IDX_NOTV3I16,
	HCC_FUNCTION_IDX_NOTV3I32,
	HCC_FUNCTION_IDX_NOTV3I64,
	HCC_FUNCTION_IDX_NOTV3U8,
	HCC_FUNCTION_IDX_NOTV3U16,
	HCC_FUNCTION_IDX_NOTV3U32,
	HCC_FUNCTION_IDX_NOTV3U64,
	HCC_FUNCTION_IDX_NOTV4F16,
	HCC_FUNCTION_IDX_NOTV4F32,
	HCC_FUNCTION_IDX_NOTV4F64,
	HCC_FUNCTION_IDX_NOTV4I8,
	HCC_FUNCTION_IDX_NOTV4I16,
	HCC_FUNCTION_IDX_NOTV4I32,
	HCC_FUNCTION_IDX_NOTV4I64,
	HCC_FUNCTION_IDX_NOTV4U8,
	HCC_FUNCTION_IDX_NOTV4U16,
	HCC_FUNCTION_IDX_NOTV4U32,
	HCC_FUNCTION_IDX_NOTV4U64,

#define HCC_FUNCTION_IDX_VECTOR_NOT_END (HCC_FUNCTION_IDX_NOTV4U64 + 1)

#define HCC_FUNCTION_IDX_VECTOR_BINARY_OP_TO_BINARY_OP(function_idx) \
	HCC_BINARY_OP_ADD + (((function_idx) - HCC_FUNCTION_IDX_VECTOR_BINARY_OP_START) / 33)
#define HCC_FUNCTION_IDX_IS_VECTOR_BINARY_OP(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_BINARY_OP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_BINARY_OP_END
#define HCC_FUNCTION_IDX_VECTOR_BINARY_OP_START HCC_FUNCTION_IDX_ADDV2F16

	HCC_FUNCTION_IDX_ADDV2F16,
	HCC_FUNCTION_IDX_ADDV2F32,
	HCC_FUNCTION_IDX_ADDV2F64,
	HCC_FUNCTION_IDX_ADDV2I8,
	HCC_FUNCTION_IDX_ADDV2I16,
	HCC_FUNCTION_IDX_ADDV2I32,
	HCC_FUNCTION_IDX_ADDV2I64,
	HCC_FUNCTION_IDX_ADDV2U8,
	HCC_FUNCTION_IDX_ADDV2U16,
	HCC_FUNCTION_IDX_ADDV2U32,
	HCC_FUNCTION_IDX_ADDV2U64,
	HCC_FUNCTION_IDX_ADDV3F16,
	HCC_FUNCTION_IDX_ADDV3F32,
	HCC_FUNCTION_IDX_ADDV3F64,
	HCC_FUNCTION_IDX_ADDV3I8,
	HCC_FUNCTION_IDX_ADDV3I16,
	HCC_FUNCTION_IDX_ADDV3I32,
	HCC_FUNCTION_IDX_ADDV3I64,
	HCC_FUNCTION_IDX_ADDV3U8,
	HCC_FUNCTION_IDX_ADDV3U16,
	HCC_FUNCTION_IDX_ADDV3U32,
	HCC_FUNCTION_IDX_ADDV3U64,
	HCC_FUNCTION_IDX_ADDV4F16,
	HCC_FUNCTION_IDX_ADDV4F32,
	HCC_FUNCTION_IDX_ADDV4F64,
	HCC_FUNCTION_IDX_ADDV4I8,
	HCC_FUNCTION_IDX_ADDV4I16,
	HCC_FUNCTION_IDX_ADDV4I32,
	HCC_FUNCTION_IDX_ADDV4I64,
	HCC_FUNCTION_IDX_ADDV4U8,
	HCC_FUNCTION_IDX_ADDV4U16,
	HCC_FUNCTION_IDX_ADDV4U32,
	HCC_FUNCTION_IDX_ADDV4U64,
	HCC_FUNCTION_IDX_SUBV2F16,
	HCC_FUNCTION_IDX_SUBV2F32,
	HCC_FUNCTION_IDX_SUBV2F64,
	HCC_FUNCTION_IDX_SUBV2I8,
	HCC_FUNCTION_IDX_SUBV2I16,
	HCC_FUNCTION_IDX_SUBV2I32,
	HCC_FUNCTION_IDX_SUBV2I64,
	HCC_FUNCTION_IDX_SUBV2U8,
	HCC_FUNCTION_IDX_SUBV2U16,
	HCC_FUNCTION_IDX_SUBV2U32,
	HCC_FUNCTION_IDX_SUBV2U64,
	HCC_FUNCTION_IDX_SUBV3F16,
	HCC_FUNCTION_IDX_SUBV3F32,
	HCC_FUNCTION_IDX_SUBV3F64,
	HCC_FUNCTION_IDX_SUBV3I8,
	HCC_FUNCTION_IDX_SUBV3I16,
	HCC_FUNCTION_IDX_SUBV3I32,
	HCC_FUNCTION_IDX_SUBV3I64,
	HCC_FUNCTION_IDX_SUBV3U8,
	HCC_FUNCTION_IDX_SUBV3U16,
	HCC_FUNCTION_IDX_SUBV3U32,
	HCC_FUNCTION_IDX_SUBV3U64,
	HCC_FUNCTION_IDX_SUBV4F16,
	HCC_FUNCTION_IDX_SUBV4F32,
	HCC_FUNCTION_IDX_SUBV4F64,
	HCC_FUNCTION_IDX_SUBV4I8,
	HCC_FUNCTION_IDX_SUBV4I16,
	HCC_FUNCTION_IDX_SUBV4I32,
	HCC_FUNCTION_IDX_SUBV4I64,
	HCC_FUNCTION_IDX_SUBV4U8,
	HCC_FUNCTION_IDX_SUBV4U16,
	HCC_FUNCTION_IDX_SUBV4U32,
	HCC_FUNCTION_IDX_SUBV4U64,
	HCC_FUNCTION_IDX_MULV2F16,
	HCC_FUNCTION_IDX_MULV2F32,
	HCC_FUNCTION_IDX_MULV2F64,
	HCC_FUNCTION_IDX_MULV2I8,
	HCC_FUNCTION_IDX_MULV2I16,
	HCC_FUNCTION_IDX_MULV2I32,
	HCC_FUNCTION_IDX_MULV2I64,
	HCC_FUNCTION_IDX_MULV2U8,
	HCC_FUNCTION_IDX_MULV2U16,
	HCC_FUNCTION_IDX_MULV2U32,
	HCC_FUNCTION_IDX_MULV2U64,
	HCC_FUNCTION_IDX_MULV3F16,
	HCC_FUNCTION_IDX_MULV3F32,
	HCC_FUNCTION_IDX_MULV3F64,
	HCC_FUNCTION_IDX_MULV3I8,
	HCC_FUNCTION_IDX_MULV3I16,
	HCC_FUNCTION_IDX_MULV3I32,
	HCC_FUNCTION_IDX_MULV3I64,
	HCC_FUNCTION_IDX_MULV3U8,
	HCC_FUNCTION_IDX_MULV3U16,
	HCC_FUNCTION_IDX_MULV3U32,
	HCC_FUNCTION_IDX_MULV3U64,
	HCC_FUNCTION_IDX_MULV4F16,
	HCC_FUNCTION_IDX_MULV4F32,
	HCC_FUNCTION_IDX_MULV4F64,
	HCC_FUNCTION_IDX_MULV4I8,
	HCC_FUNCTION_IDX_MULV4I16,
	HCC_FUNCTION_IDX_MULV4I32,
	HCC_FUNCTION_IDX_MULV4I64,
	HCC_FUNCTION_IDX_MULV4U8,
	HCC_FUNCTION_IDX_MULV4U16,
	HCC_FUNCTION_IDX_MULV4U32,
	HCC_FUNCTION_IDX_MULV4U64,
	HCC_FUNCTION_IDX_DIVV2F16,
	HCC_FUNCTION_IDX_DIVV2F32,
	HCC_FUNCTION_IDX_DIVV2F64,
	HCC_FUNCTION_IDX_DIVV2I8,
	HCC_FUNCTION_IDX_DIVV2I16,
	HCC_FUNCTION_IDX_DIVV2I32,
	HCC_FUNCTION_IDX_DIVV2I64,
	HCC_FUNCTION_IDX_DIVV2U8,
	HCC_FUNCTION_IDX_DIVV2U16,
	HCC_FUNCTION_IDX_DIVV2U32,
	HCC_FUNCTION_IDX_DIVV2U64,
	HCC_FUNCTION_IDX_DIVV3F16,
	HCC_FUNCTION_IDX_DIVV3F32,
	HCC_FUNCTION_IDX_DIVV3F64,
	HCC_FUNCTION_IDX_DIVV3I8,
	HCC_FUNCTION_IDX_DIVV3I16,
	HCC_FUNCTION_IDX_DIVV3I32,
	HCC_FUNCTION_IDX_DIVV3I64,
	HCC_FUNCTION_IDX_DIVV3U8,
	HCC_FUNCTION_IDX_DIVV3U16,
	HCC_FUNCTION_IDX_DIVV3U32,
	HCC_FUNCTION_IDX_DIVV3U64,
	HCC_FUNCTION_IDX_DIVV4F16,
	HCC_FUNCTION_IDX_DIVV4F32,
	HCC_FUNCTION_IDX_DIVV4F64,
	HCC_FUNCTION_IDX_DIVV4I8,
	HCC_FUNCTION_IDX_DIVV4I16,
	HCC_FUNCTION_IDX_DIVV4I32,
	HCC_FUNCTION_IDX_DIVV4I64,
	HCC_FUNCTION_IDX_DIVV4U8,
	HCC_FUNCTION_IDX_DIVV4U16,
	HCC_FUNCTION_IDX_DIVV4U32,
	HCC_FUNCTION_IDX_DIVV4U64,
	HCC_FUNCTION_IDX_MODV2F16,
	HCC_FUNCTION_IDX_MODV2F32,
	HCC_FUNCTION_IDX_MODV2F64,
	HCC_FUNCTION_IDX_MODV2I8,
	HCC_FUNCTION_IDX_MODV2I16,
	HCC_FUNCTION_IDX_MODV2I32,
	HCC_FUNCTION_IDX_MODV2I64,
	HCC_FUNCTION_IDX_MODV2U8,
	HCC_FUNCTION_IDX_MODV2U16,
	HCC_FUNCTION_IDX_MODV2U32,
	HCC_FUNCTION_IDX_MODV2U64,
	HCC_FUNCTION_IDX_MODV3F16,
	HCC_FUNCTION_IDX_MODV3F32,
	HCC_FUNCTION_IDX_MODV3F64,
	HCC_FUNCTION_IDX_MODV3I8,
	HCC_FUNCTION_IDX_MODV3I16,
	HCC_FUNCTION_IDX_MODV3I32,
	HCC_FUNCTION_IDX_MODV3I64,
	HCC_FUNCTION_IDX_MODV3U8,
	HCC_FUNCTION_IDX_MODV3U16,
	HCC_FUNCTION_IDX_MODV3U32,
	HCC_FUNCTION_IDX_MODV3U64,
	HCC_FUNCTION_IDX_MODV4F16,
	HCC_FUNCTION_IDX_MODV4F32,
	HCC_FUNCTION_IDX_MODV4F64,
	HCC_FUNCTION_IDX_MODV4I8,
	HCC_FUNCTION_IDX_MODV4I16,
	HCC_FUNCTION_IDX_MODV4I32,
	HCC_FUNCTION_IDX_MODV4I64,
	HCC_FUNCTION_IDX_MODV4U8,
	HCC_FUNCTION_IDX_MODV4U16,
	HCC_FUNCTION_IDX_MODV4U32,
	HCC_FUNCTION_IDX_MODV4U64,

#define HCC_FUNCTION_IDX_VECTOR_BINARY_OP_END (HCC_FUNCTION_IDX_MODV4U64 + 1)

#define HCC_FUNCTION_IDX_VECTOR_BINARY_CMP_OP_TO_BINARY_OP(function_idx) \
	HCC_BINARY_OP_EQUAL + (((function_idx) - HCC_FUNCTION_IDX_VECTOR_BINARY_CMP_OP_START) / 33)
#define HCC_FUNCTION_IDX_IS_VECTOR_BINARY_CMP_OP(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_BINARY_CMP_OP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_BINARY_CMP_OP_END
#define HCC_FUNCTION_IDX_VECTOR_BINARY_CMP_OP_START HCC_FUNCTION_IDX_EQV2F16

	HCC_FUNCTION_IDX_EQV2F16,
	HCC_FUNCTION_IDX_EQV2F32,
	HCC_FUNCTION_IDX_EQV2F64,
	HCC_FUNCTION_IDX_EQV2I8,
	HCC_FUNCTION_IDX_EQV2I16,
	HCC_FUNCTION_IDX_EQV2I32,
	HCC_FUNCTION_IDX_EQV2I64,
	HCC_FUNCTION_IDX_EQV2U8,
	HCC_FUNCTION_IDX_EQV2U16,
	HCC_FUNCTION_IDX_EQV2U32,
	HCC_FUNCTION_IDX_EQV2U64,
	HCC_FUNCTION_IDX_EQV3F16,
	HCC_FUNCTION_IDX_EQV3F32,
	HCC_FUNCTION_IDX_EQV3F64,
	HCC_FUNCTION_IDX_EQV3I8,
	HCC_FUNCTION_IDX_EQV3I16,
	HCC_FUNCTION_IDX_EQV3I32,
	HCC_FUNCTION_IDX_EQV3I64,
	HCC_FUNCTION_IDX_EQV3U8,
	HCC_FUNCTION_IDX_EQV3U16,
	HCC_FUNCTION_IDX_EQV3U32,
	HCC_FUNCTION_IDX_EQV3U64,
	HCC_FUNCTION_IDX_EQV4F16,
	HCC_FUNCTION_IDX_EQV4F32,
	HCC_FUNCTION_IDX_EQV4F64,
	HCC_FUNCTION_IDX_EQV4I8,
	HCC_FUNCTION_IDX_EQV4I16,
	HCC_FUNCTION_IDX_EQV4I32,
	HCC_FUNCTION_IDX_EQV4I64,
	HCC_FUNCTION_IDX_EQV4U8,
	HCC_FUNCTION_IDX_EQV4U16,
	HCC_FUNCTION_IDX_EQV4U32,
	HCC_FUNCTION_IDX_EQV4U64,
	HCC_FUNCTION_IDX_NEQV2F16,
	HCC_FUNCTION_IDX_NEQV2F32,
	HCC_FUNCTION_IDX_NEQV2F64,
	HCC_FUNCTION_IDX_NEQV2I8,
	HCC_FUNCTION_IDX_NEQV2I16,
	HCC_FUNCTION_IDX_NEQV2I32,
	HCC_FUNCTION_IDX_NEQV2I64,
	HCC_FUNCTION_IDX_NEQV2U8,
	HCC_FUNCTION_IDX_NEQV2U16,
	HCC_FUNCTION_IDX_NEQV2U32,
	HCC_FUNCTION_IDX_NEQV2U64,
	HCC_FUNCTION_IDX_NEQV3F16,
	HCC_FUNCTION_IDX_NEQV3F32,
	HCC_FUNCTION_IDX_NEQV3F64,
	HCC_FUNCTION_IDX_NEQV3I8,
	HCC_FUNCTION_IDX_NEQV3I16,
	HCC_FUNCTION_IDX_NEQV3I32,
	HCC_FUNCTION_IDX_NEQV3I64,
	HCC_FUNCTION_IDX_NEQV3U8,
	HCC_FUNCTION_IDX_NEQV3U16,
	HCC_FUNCTION_IDX_NEQV3U32,
	HCC_FUNCTION_IDX_NEQV3U64,
	HCC_FUNCTION_IDX_NEQV4F16,
	HCC_FUNCTION_IDX_NEQV4F32,
	HCC_FUNCTION_IDX_NEQV4F64,
	HCC_FUNCTION_IDX_NEQV4I8,
	HCC_FUNCTION_IDX_NEQV4I16,
	HCC_FUNCTION_IDX_NEQV4I32,
	HCC_FUNCTION_IDX_NEQV4I64,
	HCC_FUNCTION_IDX_NEQV4U8,
	HCC_FUNCTION_IDX_NEQV4U16,
	HCC_FUNCTION_IDX_NEQV4U32,
	HCC_FUNCTION_IDX_NEQV4U64,
	HCC_FUNCTION_IDX_LTV2F16,
	HCC_FUNCTION_IDX_LTV2F32,
	HCC_FUNCTION_IDX_LTV2F64,
	HCC_FUNCTION_IDX_LTV2I8,
	HCC_FUNCTION_IDX_LTV2I16,
	HCC_FUNCTION_IDX_LTV2I32,
	HCC_FUNCTION_IDX_LTV2I64,
	HCC_FUNCTION_IDX_LTV2U8,
	HCC_FUNCTION_IDX_LTV2U16,
	HCC_FUNCTION_IDX_LTV2U32,
	HCC_FUNCTION_IDX_LTV2U64,
	HCC_FUNCTION_IDX_LTV3F16,
	HCC_FUNCTION_IDX_LTV3F32,
	HCC_FUNCTION_IDX_LTV3F64,
	HCC_FUNCTION_IDX_LTV3I8,
	HCC_FUNCTION_IDX_LTV3I16,
	HCC_FUNCTION_IDX_LTV3I32,
	HCC_FUNCTION_IDX_LTV3I64,
	HCC_FUNCTION_IDX_LTV3U8,
	HCC_FUNCTION_IDX_LTV3U16,
	HCC_FUNCTION_IDX_LTV3U32,
	HCC_FUNCTION_IDX_LTV3U64,
	HCC_FUNCTION_IDX_LTV4F16,
	HCC_FUNCTION_IDX_LTV4F32,
	HCC_FUNCTION_IDX_LTV4F64,
	HCC_FUNCTION_IDX_LTV4I8,
	HCC_FUNCTION_IDX_LTV4I16,
	HCC_FUNCTION_IDX_LTV4I32,
	HCC_FUNCTION_IDX_LTV4I64,
	HCC_FUNCTION_IDX_LTV4U8,
	HCC_FUNCTION_IDX_LTV4U16,
	HCC_FUNCTION_IDX_LTV4U32,
	HCC_FUNCTION_IDX_LTV4U64,
	HCC_FUNCTION_IDX_LTEQV2F16,
	HCC_FUNCTION_IDX_LTEQV2F32,
	HCC_FUNCTION_IDX_LTEQV2F64,
	HCC_FUNCTION_IDX_LTEQV2I8,
	HCC_FUNCTION_IDX_LTEQV2I16,
	HCC_FUNCTION_IDX_LTEQV2I32,
	HCC_FUNCTION_IDX_LTEQV2I64,
	HCC_FUNCTION_IDX_LTEQV2U8,
	HCC_FUNCTION_IDX_LTEQV2U16,
	HCC_FUNCTION_IDX_LTEQV2U32,
	HCC_FUNCTION_IDX_LTEQV2U64,
	HCC_FUNCTION_IDX_LTEQV3F16,
	HCC_FUNCTION_IDX_LTEQV3F32,
	HCC_FUNCTION_IDX_LTEQV3F64,
	HCC_FUNCTION_IDX_LTEQV3I8,
	HCC_FUNCTION_IDX_LTEQV3I16,
	HCC_FUNCTION_IDX_LTEQV3I32,
	HCC_FUNCTION_IDX_LTEQV3I64,
	HCC_FUNCTION_IDX_LTEQV3U8,
	HCC_FUNCTION_IDX_LTEQV3U16,
	HCC_FUNCTION_IDX_LTEQV3U32,
	HCC_FUNCTION_IDX_LTEQV3U64,
	HCC_FUNCTION_IDX_LTEQV4F16,
	HCC_FUNCTION_IDX_LTEQV4F32,
	HCC_FUNCTION_IDX_LTEQV4F64,
	HCC_FUNCTION_IDX_LTEQV4I8,
	HCC_FUNCTION_IDX_LTEQV4I16,
	HCC_FUNCTION_IDX_LTEQV4I32,
	HCC_FUNCTION_IDX_LTEQV4I64,
	HCC_FUNCTION_IDX_LTEQV4U8,
	HCC_FUNCTION_IDX_LTEQV4U16,
	HCC_FUNCTION_IDX_LTEQV4U32,
	HCC_FUNCTION_IDX_LTEQV4U64,
	HCC_FUNCTION_IDX_GTV2F16,
	HCC_FUNCTION_IDX_GTV2F32,
	HCC_FUNCTION_IDX_GTV2F64,
	HCC_FUNCTION_IDX_GTV2I8,
	HCC_FUNCTION_IDX_GTV2I16,
	HCC_FUNCTION_IDX_GTV2I32,
	HCC_FUNCTION_IDX_GTV2I64,
	HCC_FUNCTION_IDX_GTV2U8,
	HCC_FUNCTION_IDX_GTV2U16,
	HCC_FUNCTION_IDX_GTV2U32,
	HCC_FUNCTION_IDX_GTV2U64,
	HCC_FUNCTION_IDX_GTV3F16,
	HCC_FUNCTION_IDX_GTV3F32,
	HCC_FUNCTION_IDX_GTV3F64,
	HCC_FUNCTION_IDX_GTV3I8,
	HCC_FUNCTION_IDX_GTV3I16,
	HCC_FUNCTION_IDX_GTV3I32,
	HCC_FUNCTION_IDX_GTV3I64,
	HCC_FUNCTION_IDX_GTV3U8,
	HCC_FUNCTION_IDX_GTV3U16,
	HCC_FUNCTION_IDX_GTV3U32,
	HCC_FUNCTION_IDX_GTV3U64,
	HCC_FUNCTION_IDX_GTV4F16,
	HCC_FUNCTION_IDX_GTV4F32,
	HCC_FUNCTION_IDX_GTV4F64,
	HCC_FUNCTION_IDX_GTV4I8,
	HCC_FUNCTION_IDX_GTV4I16,
	HCC_FUNCTION_IDX_GTV4I32,
	HCC_FUNCTION_IDX_GTV4I64,
	HCC_FUNCTION_IDX_GTV4U8,
	HCC_FUNCTION_IDX_GTV4U16,
	HCC_FUNCTION_IDX_GTV4U32,
	HCC_FUNCTION_IDX_GTV4U64,
	HCC_FUNCTION_IDX_GTEQV2F16,
	HCC_FUNCTION_IDX_GTEQV2F32,
	HCC_FUNCTION_IDX_GTEQV2F64,
	HCC_FUNCTION_IDX_GTEQV2I8,
	HCC_FUNCTION_IDX_GTEQV2I16,
	HCC_FUNCTION_IDX_GTEQV2I32,
	HCC_FUNCTION_IDX_GTEQV2I64,
	HCC_FUNCTION_IDX_GTEQV2U8,
	HCC_FUNCTION_IDX_GTEQV2U16,
	HCC_FUNCTION_IDX_GTEQV2U32,
	HCC_FUNCTION_IDX_GTEQV2U64,
	HCC_FUNCTION_IDX_GTEQV3F16,
	HCC_FUNCTION_IDX_GTEQV3F32,
	HCC_FUNCTION_IDX_GTEQV3F64,
	HCC_FUNCTION_IDX_GTEQV3I8,
	HCC_FUNCTION_IDX_GTEQV3I16,
	HCC_FUNCTION_IDX_GTEQV3I32,
	HCC_FUNCTION_IDX_GTEQV3I64,
	HCC_FUNCTION_IDX_GTEQV3U8,
	HCC_FUNCTION_IDX_GTEQV3U16,
	HCC_FUNCTION_IDX_GTEQV3U32,
	HCC_FUNCTION_IDX_GTEQV3U64,
	HCC_FUNCTION_IDX_GTEQV4F16,
	HCC_FUNCTION_IDX_GTEQV4F32,
	HCC_FUNCTION_IDX_GTEQV4F64,
	HCC_FUNCTION_IDX_GTEQV4I8,
	HCC_FUNCTION_IDX_GTEQV4I16,
	HCC_FUNCTION_IDX_GTEQV4I32,
	HCC_FUNCTION_IDX_GTEQV4I64,
	HCC_FUNCTION_IDX_GTEQV4U8,
	HCC_FUNCTION_IDX_GTEQV4U16,
	HCC_FUNCTION_IDX_GTEQV4U32,
	HCC_FUNCTION_IDX_GTEQV4U64,

#define HCC_FUNCTION_IDX_VECTOR_BINARY_CMP_OP_END (HCC_FUNCTION_IDX_GTEQV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_UNARY_OP(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_UNARY_OP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_UNARY_OP_END
#define HCC_FUNCTION_IDX_VECTOR_UNARY_OP_START HCC_FUNCTION_IDX_NOTV2BOOL

	HCC_FUNCTION_IDX_NOTV2BOOL,
	HCC_FUNCTION_IDX_NOTV3BOOL,
	HCC_FUNCTION_IDX_NOTV4BOOL,
	HCC_FUNCTION_IDX_NEGV2F16,
	HCC_FUNCTION_IDX_NEGV2F32,
	HCC_FUNCTION_IDX_NEGV2F64,
	HCC_FUNCTION_IDX_NEGV2I8,
	HCC_FUNCTION_IDX_NEGV2I16,
	HCC_FUNCTION_IDX_NEGV2I32,
	HCC_FUNCTION_IDX_NEGV2I64,
	HCC_FUNCTION_IDX_NEGV2U8,
	HCC_FUNCTION_IDX_NEGV2U16,
	HCC_FUNCTION_IDX_NEGV2U32,
	HCC_FUNCTION_IDX_NEGV2U64,
	HCC_FUNCTION_IDX_NEGV3F16,
	HCC_FUNCTION_IDX_NEGV3F32,
	HCC_FUNCTION_IDX_NEGV3F64,
	HCC_FUNCTION_IDX_NEGV3I8,
	HCC_FUNCTION_IDX_NEGV3I16,
	HCC_FUNCTION_IDX_NEGV3I32,
	HCC_FUNCTION_IDX_NEGV3I64,
	HCC_FUNCTION_IDX_NEGV3U8,
	HCC_FUNCTION_IDX_NEGV3U16,
	HCC_FUNCTION_IDX_NEGV3U32,
	HCC_FUNCTION_IDX_NEGV3U64,
	HCC_FUNCTION_IDX_NEGV4F16,
	HCC_FUNCTION_IDX_NEGV4F32,
	HCC_FUNCTION_IDX_NEGV4F64,
	HCC_FUNCTION_IDX_NEGV4I8,
	HCC_FUNCTION_IDX_NEGV4I16,
	HCC_FUNCTION_IDX_NEGV4I32,
	HCC_FUNCTION_IDX_NEGV4I64,
	HCC_FUNCTION_IDX_NEGV4U8,
	HCC_FUNCTION_IDX_NEGV4U16,
	HCC_FUNCTION_IDX_NEGV4U32,
	HCC_FUNCTION_IDX_NEGV4U64,
	HCC_FUNCTION_IDX_BITNOTV2I8,
	HCC_FUNCTION_IDX_BITNOTV2I16,
	HCC_FUNCTION_IDX_BITNOTV2I32,
	HCC_FUNCTION_IDX_BITNOTV2I64,
	HCC_FUNCTION_IDX_BITNOTV2U8,
	HCC_FUNCTION_IDX_BITNOTV2U16,
	HCC_FUNCTION_IDX_BITNOTV2U32,
	HCC_FUNCTION_IDX_BITNOTV2U64,
	HCC_FUNCTION_IDX_BITNOTV3I8,
	HCC_FUNCTION_IDX_BITNOTV3I16,
	HCC_FUNCTION_IDX_BITNOTV3I32,
	HCC_FUNCTION_IDX_BITNOTV3I64,
	HCC_FUNCTION_IDX_BITNOTV3U8,
	HCC_FUNCTION_IDX_BITNOTV3U16,
	HCC_FUNCTION_IDX_BITNOTV3U32,
	HCC_FUNCTION_IDX_BITNOTV3U64,
	HCC_FUNCTION_IDX_BITNOTV4I8,
	HCC_FUNCTION_IDX_BITNOTV4I16,
	HCC_FUNCTION_IDX_BITNOTV4I32,
	HCC_FUNCTION_IDX_BITNOTV4I64,
	HCC_FUNCTION_IDX_BITNOTV4U8,
	HCC_FUNCTION_IDX_BITNOTV4U16,
	HCC_FUNCTION_IDX_BITNOTV4U32,
	HCC_FUNCTION_IDX_BITNOTV4U64,

#define HCC_FUNCTION_IDX_VECTOR_UNARY_OP_END (HCC_FUNCTION_IDX_BITNOTV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_MIN(function_idx) \
	HCC_FUNCTION_IDX_MIN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MIN_END
#define HCC_FUNCTION_IDX_MIN_START HCC_FUNCTION_IDX_MINF16

	HCC_FUNCTION_IDX_MINF16,
	HCC_FUNCTION_IDX_MINF32,
	HCC_FUNCTION_IDX_MINF64,
	HCC_FUNCTION_IDX_MINV2F16,
	HCC_FUNCTION_IDX_MINV2F32,
	HCC_FUNCTION_IDX_MINV2F64,
	HCC_FUNCTION_IDX_MINV3F16,
	HCC_FUNCTION_IDX_MINV3F32,
	HCC_FUNCTION_IDX_MINV3F64,
	HCC_FUNCTION_IDX_MINV4F16,
	HCC_FUNCTION_IDX_MINV4F32,
	HCC_FUNCTION_IDX_MINV4F64,
	HCC_FUNCTION_IDX_MINI8,
	HCC_FUNCTION_IDX_MINI16,
	HCC_FUNCTION_IDX_MINI32,
	HCC_FUNCTION_IDX_MINI64,
	HCC_FUNCTION_IDX_MINV2I8,
	HCC_FUNCTION_IDX_MINV2I16,
	HCC_FUNCTION_IDX_MINV2I32,
	HCC_FUNCTION_IDX_MINV2I64,
	HCC_FUNCTION_IDX_MINV3I8,
	HCC_FUNCTION_IDX_MINV3I16,
	HCC_FUNCTION_IDX_MINV3I32,
	HCC_FUNCTION_IDX_MINV3I64,
	HCC_FUNCTION_IDX_MINV4I8,
	HCC_FUNCTION_IDX_MINV4I16,
	HCC_FUNCTION_IDX_MINV4I32,
	HCC_FUNCTION_IDX_MINV4I64,
	HCC_FUNCTION_IDX_MINU8,
	HCC_FUNCTION_IDX_MINU16,
	HCC_FUNCTION_IDX_MINU32,
	HCC_FUNCTION_IDX_MINU64,
	HCC_FUNCTION_IDX_MINV2U8,
	HCC_FUNCTION_IDX_MINV2U16,
	HCC_FUNCTION_IDX_MINV2U32,
	HCC_FUNCTION_IDX_MINV2U64,
	HCC_FUNCTION_IDX_MINV3U8,
	HCC_FUNCTION_IDX_MINV3U16,
	HCC_FUNCTION_IDX_MINV3U32,
	HCC_FUNCTION_IDX_MINV3U64,
	HCC_FUNCTION_IDX_MINV4U8,
	HCC_FUNCTION_IDX_MINV4U16,
	HCC_FUNCTION_IDX_MINV4U32,
	HCC_FUNCTION_IDX_MINV4U64,

#define HCC_FUNCTION_IDX_MIN_END (HCC_FUNCTION_IDX_MINV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_MAX(function_idx) \
	HCC_FUNCTION_IDX_MAX_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MAX_END
#define HCC_FUNCTION_IDX_MAX_START HCC_FUNCTION_IDX_MAXF16

	HCC_FUNCTION_IDX_MAXF16,
	HCC_FUNCTION_IDX_MAXF32,
	HCC_FUNCTION_IDX_MAXF64,
	HCC_FUNCTION_IDX_MAXV2F16,
	HCC_FUNCTION_IDX_MAXV2F32,
	HCC_FUNCTION_IDX_MAXV2F64,
	HCC_FUNCTION_IDX_MAXV3F16,
	HCC_FUNCTION_IDX_MAXV3F32,
	HCC_FUNCTION_IDX_MAXV3F64,
	HCC_FUNCTION_IDX_MAXV4F16,
	HCC_FUNCTION_IDX_MAXV4F32,
	HCC_FUNCTION_IDX_MAXV4F64,
	HCC_FUNCTION_IDX_MAXI8,
	HCC_FUNCTION_IDX_MAXI16,
	HCC_FUNCTION_IDX_MAXI32,
	HCC_FUNCTION_IDX_MAXI64,
	HCC_FUNCTION_IDX_MAXV2I8,
	HCC_FUNCTION_IDX_MAXV2I16,
	HCC_FUNCTION_IDX_MAXV2I32,
	HCC_FUNCTION_IDX_MAXV2I64,
	HCC_FUNCTION_IDX_MAXV3I8,
	HCC_FUNCTION_IDX_MAXV3I16,
	HCC_FUNCTION_IDX_MAXV3I32,
	HCC_FUNCTION_IDX_MAXV3I64,
	HCC_FUNCTION_IDX_MAXV4I8,
	HCC_FUNCTION_IDX_MAXV4I16,
	HCC_FUNCTION_IDX_MAXV4I32,
	HCC_FUNCTION_IDX_MAXV4I64,
	HCC_FUNCTION_IDX_MAXU8,
	HCC_FUNCTION_IDX_MAXU16,
	HCC_FUNCTION_IDX_MAXU32,
	HCC_FUNCTION_IDX_MAXU64,
	HCC_FUNCTION_IDX_MAXV2U8,
	HCC_FUNCTION_IDX_MAXV2U16,
	HCC_FUNCTION_IDX_MAXV2U32,
	HCC_FUNCTION_IDX_MAXV2U64,
	HCC_FUNCTION_IDX_MAXV3U8,
	HCC_FUNCTION_IDX_MAXV3U16,
	HCC_FUNCTION_IDX_MAXV3U32,
	HCC_FUNCTION_IDX_MAXV3U64,
	HCC_FUNCTION_IDX_MAXV4U8,
	HCC_FUNCTION_IDX_MAXV4U16,
	HCC_FUNCTION_IDX_MAXV4U32,
	HCC_FUNCTION_IDX_MAXV4U64,

#define HCC_FUNCTION_IDX_MAX_END (HCC_FUNCTION_IDX_MAXV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_CLAMP(function_idx) \
	HCC_FUNCTION_IDX_CLAMP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_CLAMP_END
#define HCC_FUNCTION_IDX_CLAMP_START HCC_FUNCTION_IDX_CLAMPF16

	HCC_FUNCTION_IDX_CLAMPF16,
	HCC_FUNCTION_IDX_CLAMPF32,
	HCC_FUNCTION_IDX_CLAMPF64,
	HCC_FUNCTION_IDX_CLAMPV2F16,
	HCC_FUNCTION_IDX_CLAMPV2F32,
	HCC_FUNCTION_IDX_CLAMPV2F64,
	HCC_FUNCTION_IDX_CLAMPV3F16,
	HCC_FUNCTION_IDX_CLAMPV3F32,
	HCC_FUNCTION_IDX_CLAMPV3F64,
	HCC_FUNCTION_IDX_CLAMPV4F16,
	HCC_FUNCTION_IDX_CLAMPV4F32,
	HCC_FUNCTION_IDX_CLAMPV4F64,
	HCC_FUNCTION_IDX_CLAMPI8,
	HCC_FUNCTION_IDX_CLAMPI16,
	HCC_FUNCTION_IDX_CLAMPI32,
	HCC_FUNCTION_IDX_CLAMPI64,
	HCC_FUNCTION_IDX_CLAMPV2I8,
	HCC_FUNCTION_IDX_CLAMPV2I16,
	HCC_FUNCTION_IDX_CLAMPV2I32,
	HCC_FUNCTION_IDX_CLAMPV2I64,
	HCC_FUNCTION_IDX_CLAMPV3I8,
	HCC_FUNCTION_IDX_CLAMPV3I16,
	HCC_FUNCTION_IDX_CLAMPV3I32,
	HCC_FUNCTION_IDX_CLAMPV3I64,
	HCC_FUNCTION_IDX_CLAMPV4I8,
	HCC_FUNCTION_IDX_CLAMPV4I16,
	HCC_FUNCTION_IDX_CLAMPV4I32,
	HCC_FUNCTION_IDX_CLAMPV4I64,
	HCC_FUNCTION_IDX_CLAMPU8,
	HCC_FUNCTION_IDX_CLAMPU16,
	HCC_FUNCTION_IDX_CLAMPU32,
	HCC_FUNCTION_IDX_CLAMPU64,
	HCC_FUNCTION_IDX_CLAMPV2U8,
	HCC_FUNCTION_IDX_CLAMPV2U16,
	HCC_FUNCTION_IDX_CLAMPV2U32,
	HCC_FUNCTION_IDX_CLAMPV2U64,
	HCC_FUNCTION_IDX_CLAMPV3U8,
	HCC_FUNCTION_IDX_CLAMPV3U16,
	HCC_FUNCTION_IDX_CLAMPV3U32,
	HCC_FUNCTION_IDX_CLAMPV3U64,
	HCC_FUNCTION_IDX_CLAMPV4U8,
	HCC_FUNCTION_IDX_CLAMPV4U16,
	HCC_FUNCTION_IDX_CLAMPV4U32,
	HCC_FUNCTION_IDX_CLAMPV4U64,

#define HCC_FUNCTION_IDX_CLAMP_END (HCC_FUNCTION_IDX_CLAMPV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_SIGN(function_idx) \
	HCC_FUNCTION_IDX_SIGN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_SIGN_END
#define HCC_FUNCTION_IDX_SIGN_START HCC_FUNCTION_IDX_SIGNF16

	HCC_FUNCTION_IDX_SIGNF16,
	HCC_FUNCTION_IDX_SIGNF32,
	HCC_FUNCTION_IDX_SIGNF64,
	HCC_FUNCTION_IDX_SIGNV2F16,
	HCC_FUNCTION_IDX_SIGNV2F32,
	HCC_FUNCTION_IDX_SIGNV2F64,
	HCC_FUNCTION_IDX_SIGNV3F16,
	HCC_FUNCTION_IDX_SIGNV3F32,
	HCC_FUNCTION_IDX_SIGNV3F64,
	HCC_FUNCTION_IDX_SIGNV4F16,
	HCC_FUNCTION_IDX_SIGNV4F32,
	HCC_FUNCTION_IDX_SIGNV4F64,
	HCC_FUNCTION_IDX_SIGNI8,
	HCC_FUNCTION_IDX_SIGNI16,
	HCC_FUNCTION_IDX_SIGNI32,
	HCC_FUNCTION_IDX_SIGNI64,
	HCC_FUNCTION_IDX_SIGNV2I8,
	HCC_FUNCTION_IDX_SIGNV2I16,
	HCC_FUNCTION_IDX_SIGNV2I32,
	HCC_FUNCTION_IDX_SIGNV2I64,
	HCC_FUNCTION_IDX_SIGNV3I8,
	HCC_FUNCTION_IDX_SIGNV3I16,
	HCC_FUNCTION_IDX_SIGNV3I32,
	HCC_FUNCTION_IDX_SIGNV3I64,
	HCC_FUNCTION_IDX_SIGNV4I8,
	HCC_FUNCTION_IDX_SIGNV4I16,
	HCC_FUNCTION_IDX_SIGNV4I32,
	HCC_FUNCTION_IDX_SIGNV4I64,

#define HCC_FUNCTION_IDX_SIGN_END (HCC_FUNCTION_IDX_SIGNV4I64 + 1)

#define HCC_FUNCTION_IDX_IS_ABS(function_idx) \
	HCC_FUNCTION_IDX_ABS_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ABS_END
#define HCC_FUNCTION_IDX_ABS_START HCC_FUNCTION_IDX_FABSF

	HCC_FUNCTION_IDX_FABSF,
	HCC_FUNCTION_IDX_FABS,
	HCC_FUNCTION_IDX_ABSF16,
	HCC_FUNCTION_IDX_ABSV2F16,
	HCC_FUNCTION_IDX_ABSV2F32,
	HCC_FUNCTION_IDX_ABSV2F64,
	HCC_FUNCTION_IDX_ABSV3F16,
	HCC_FUNCTION_IDX_ABSV3F32,
	HCC_FUNCTION_IDX_ABSV3F64,
	HCC_FUNCTION_IDX_ABSV4F16,
	HCC_FUNCTION_IDX_ABSV4F32,
	HCC_FUNCTION_IDX_ABSV4F64,
	HCC_FUNCTION_IDX_ABSI8,
	HCC_FUNCTION_IDX_ABSI16,
	HCC_FUNCTION_IDX_ABSI32,
	HCC_FUNCTION_IDX_ABSI64,
	HCC_FUNCTION_IDX_ABSV2I8,
	HCC_FUNCTION_IDX_ABSV2I16,
	HCC_FUNCTION_IDX_ABSV2I32,
	HCC_FUNCTION_IDX_ABSV2I64,
	HCC_FUNCTION_IDX_ABSV3I8,
	HCC_FUNCTION_IDX_ABSV3I16,
	HCC_FUNCTION_IDX_ABSV3I32,
	HCC_FUNCTION_IDX_ABSV3I64,
	HCC_FUNCTION_IDX_ABSV4I8,
	HCC_FUNCTION_IDX_ABSV4I16,
	HCC_FUNCTION_IDX_ABSV4I32,
	HCC_FUNCTION_IDX_ABSV4I64,

#define HCC_FUNCTION_IDX_ABS_END (HCC_FUNCTION_IDX_ABSV4I64 + 1)

#define HCC_FUNCTION_IDX_VECTOR_BINARY_BITWISE_OP_TO_BINARY_OP(function_idx) \
	HCC_BINARY_OP_BIT_AND + (((function_idx) - HCC_FUNCTION_IDX_VECTOR_BINARY_BITWISE_OP_START) / 33)
#define HCC_FUNCTION_IDX_IS_VECTOR_BINARY_BITWISE_OP(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_BINARY_BITWISE_OP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_BINARY_BITWISE_OP_END
#define HCC_FUNCTION_IDX_VECTOR_BINARY_BITWISE_OP_START HCC_FUNCTION_IDX_BITANDV2I8

	HCC_FUNCTION_IDX_BITANDV2I8,
	HCC_FUNCTION_IDX_BITANDV2I16,
	HCC_FUNCTION_IDX_BITANDV2I32,
	HCC_FUNCTION_IDX_BITANDV2I64,
	HCC_FUNCTION_IDX_BITANDV2U8,
	HCC_FUNCTION_IDX_BITANDV2U16,
	HCC_FUNCTION_IDX_BITANDV2U32,
	HCC_FUNCTION_IDX_BITANDV2U64,
	HCC_FUNCTION_IDX_BITANDV3I8,
	HCC_FUNCTION_IDX_BITANDV3I16,
	HCC_FUNCTION_IDX_BITANDV3I32,
	HCC_FUNCTION_IDX_BITANDV3I64,
	HCC_FUNCTION_IDX_BITANDV3U8,
	HCC_FUNCTION_IDX_BITANDV3U16,
	HCC_FUNCTION_IDX_BITANDV3U32,
	HCC_FUNCTION_IDX_BITANDV3U64,
	HCC_FUNCTION_IDX_BITANDV4I8,
	HCC_FUNCTION_IDX_BITANDV4I16,
	HCC_FUNCTION_IDX_BITANDV4I32,
	HCC_FUNCTION_IDX_BITANDV4I64,
	HCC_FUNCTION_IDX_BITANDV4U8,
	HCC_FUNCTION_IDX_BITANDV4U16,
	HCC_FUNCTION_IDX_BITANDV4U32,
	HCC_FUNCTION_IDX_BITANDV4U64,
	HCC_FUNCTION_IDX_BITORV2I8,
	HCC_FUNCTION_IDX_BITORV2I16,
	HCC_FUNCTION_IDX_BITORV2I32,
	HCC_FUNCTION_IDX_BITORV2I64,
	HCC_FUNCTION_IDX_BITORV2U8,
	HCC_FUNCTION_IDX_BITORV2U16,
	HCC_FUNCTION_IDX_BITORV2U32,
	HCC_FUNCTION_IDX_BITORV2U64,
	HCC_FUNCTION_IDX_BITORV3I8,
	HCC_FUNCTION_IDX_BITORV3I16,
	HCC_FUNCTION_IDX_BITORV3I32,
	HCC_FUNCTION_IDX_BITORV3I64,
	HCC_FUNCTION_IDX_BITORV3U8,
	HCC_FUNCTION_IDX_BITORV3U16,
	HCC_FUNCTION_IDX_BITORV3U32,
	HCC_FUNCTION_IDX_BITORV3U64,
	HCC_FUNCTION_IDX_BITORV4I8,
	HCC_FUNCTION_IDX_BITORV4I16,
	HCC_FUNCTION_IDX_BITORV4I32,
	HCC_FUNCTION_IDX_BITORV4I64,
	HCC_FUNCTION_IDX_BITORV4U8,
	HCC_FUNCTION_IDX_BITORV4U16,
	HCC_FUNCTION_IDX_BITORV4U32,
	HCC_FUNCTION_IDX_BITORV4U64,
	HCC_FUNCTION_IDX_BITXORV2I8,
	HCC_FUNCTION_IDX_BITXORV2I16,
	HCC_FUNCTION_IDX_BITXORV2I32,
	HCC_FUNCTION_IDX_BITXORV2I64,
	HCC_FUNCTION_IDX_BITXORV2U8,
	HCC_FUNCTION_IDX_BITXORV2U16,
	HCC_FUNCTION_IDX_BITXORV2U32,
	HCC_FUNCTION_IDX_BITXORV2U64,
	HCC_FUNCTION_IDX_BITXORV3I8,
	HCC_FUNCTION_IDX_BITXORV3I16,
	HCC_FUNCTION_IDX_BITXORV3I32,
	HCC_FUNCTION_IDX_BITXORV3I64,
	HCC_FUNCTION_IDX_BITXORV3U8,
	HCC_FUNCTION_IDX_BITXORV3U16,
	HCC_FUNCTION_IDX_BITXORV3U32,
	HCC_FUNCTION_IDX_BITXORV3U64,
	HCC_FUNCTION_IDX_BITXORV4I8,
	HCC_FUNCTION_IDX_BITXORV4I16,
	HCC_FUNCTION_IDX_BITXORV4I32,
	HCC_FUNCTION_IDX_BITXORV4I64,
	HCC_FUNCTION_IDX_BITXORV4U8,
	HCC_FUNCTION_IDX_BITXORV4U16,
	HCC_FUNCTION_IDX_BITXORV4U32,
	HCC_FUNCTION_IDX_BITXORV4U64,
	HCC_FUNCTION_IDX_BITSHLV2I8,
	HCC_FUNCTION_IDX_BITSHLV2I16,
	HCC_FUNCTION_IDX_BITSHLV2I32,
	HCC_FUNCTION_IDX_BITSHLV2I64,
	HCC_FUNCTION_IDX_BITSHLV2U8,
	HCC_FUNCTION_IDX_BITSHLV2U16,
	HCC_FUNCTION_IDX_BITSHLV2U32,
	HCC_FUNCTION_IDX_BITSHLV2U64,
	HCC_FUNCTION_IDX_BITSHLV3I8,
	HCC_FUNCTION_IDX_BITSHLV3I16,
	HCC_FUNCTION_IDX_BITSHLV3I32,
	HCC_FUNCTION_IDX_BITSHLV3I64,
	HCC_FUNCTION_IDX_BITSHLV3U8,
	HCC_FUNCTION_IDX_BITSHLV3U16,
	HCC_FUNCTION_IDX_BITSHLV3U32,
	HCC_FUNCTION_IDX_BITSHLV3U64,
	HCC_FUNCTION_IDX_BITSHLV4I8,
	HCC_FUNCTION_IDX_BITSHLV4I16,
	HCC_FUNCTION_IDX_BITSHLV4I32,
	HCC_FUNCTION_IDX_BITSHLV4I64,
	HCC_FUNCTION_IDX_BITSHLV4U8,
	HCC_FUNCTION_IDX_BITSHLV4U16,
	HCC_FUNCTION_IDX_BITSHLV4U32,
	HCC_FUNCTION_IDX_BITSHLV4U64,
	HCC_FUNCTION_IDX_BITSHRV2I8,
	HCC_FUNCTION_IDX_BITSHRV2I16,
	HCC_FUNCTION_IDX_BITSHRV2I32,
	HCC_FUNCTION_IDX_BITSHRV2I64,
	HCC_FUNCTION_IDX_BITSHRV2U8,
	HCC_FUNCTION_IDX_BITSHRV2U16,
	HCC_FUNCTION_IDX_BITSHRV2U32,
	HCC_FUNCTION_IDX_BITSHRV2U64,
	HCC_FUNCTION_IDX_BITSHRV3I8,
	HCC_FUNCTION_IDX_BITSHRV3I16,
	HCC_FUNCTION_IDX_BITSHRV3I32,
	HCC_FUNCTION_IDX_BITSHRV3I64,
	HCC_FUNCTION_IDX_BITSHRV3U8,
	HCC_FUNCTION_IDX_BITSHRV3U16,
	HCC_FUNCTION_IDX_BITSHRV3U32,
	HCC_FUNCTION_IDX_BITSHRV3U64,
	HCC_FUNCTION_IDX_BITSHRV4I8,
	HCC_FUNCTION_IDX_BITSHRV4I16,
	HCC_FUNCTION_IDX_BITSHRV4I32,
	HCC_FUNCTION_IDX_BITSHRV4I64,
	HCC_FUNCTION_IDX_BITSHRV4U8,
	HCC_FUNCTION_IDX_BITSHRV4U16,
	HCC_FUNCTION_IDX_BITSHRV4U32,
	HCC_FUNCTION_IDX_BITSHRV4U64,

#define HCC_FUNCTION_IDX_VECTOR_BINARY_BITWISE_OP_END (HCC_FUNCTION_IDX_BITSHRV4U64 + 1)

#define HCC_FUNCTION_IDX_IS_FMA(function_idx) \
	HCC_FUNCTION_IDX_FMA_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_FMA_END
#define HCC_FUNCTION_IDX_FMA_START HCC_FUNCTION_IDX_FMAF16

	HCC_FUNCTION_IDX_FMAF16,
	HCC_FUNCTION_IDX_FMAF,
	HCC_FUNCTION_IDX_FMA,
	HCC_FUNCTION_IDX_FMAV2F16,
	HCC_FUNCTION_IDX_FMAV2F32,
	HCC_FUNCTION_IDX_FMAV2F64,
	HCC_FUNCTION_IDX_FMAV3F16,
	HCC_FUNCTION_IDX_FMAV3F32,
	HCC_FUNCTION_IDX_FMAV3F64,
	HCC_FUNCTION_IDX_FMAV4F16,
	HCC_FUNCTION_IDX_FMAV4F32,
	HCC_FUNCTION_IDX_FMAV4F64,

#define HCC_FUNCTION_IDX_FMA_END (HCC_FUNCTION_IDX_FMAV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_FLOOR(function_idx) \
	HCC_FUNCTION_IDX_FLOOR_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_FLOOR_END
#define HCC_FUNCTION_IDX_FLOOR_START HCC_FUNCTION_IDX_FLOORF16


	HCC_FUNCTION_IDX_FLOORF16,
	HCC_FUNCTION_IDX_FLOORF,
	HCC_FUNCTION_IDX_FLOOR,
	HCC_FUNCTION_IDX_FLOORV2F16,
	HCC_FUNCTION_IDX_FLOORV2F32,
	HCC_FUNCTION_IDX_FLOORV2F64,
	HCC_FUNCTION_IDX_FLOORV3F16,
	HCC_FUNCTION_IDX_FLOORV3F32,
	HCC_FUNCTION_IDX_FLOORV3F64,
	HCC_FUNCTION_IDX_FLOORV4F16,
	HCC_FUNCTION_IDX_FLOORV4F32,
	HCC_FUNCTION_IDX_FLOORV4F64,

#define HCC_FUNCTION_IDX_FLOOR_END (HCC_FUNCTION_IDX_FLOORV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_CEIL(function_idx) \
	HCC_FUNCTION_IDX_CEIL_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_CEIL_END
#define HCC_FUNCTION_IDX_CEIL_START HCC_FUNCTION_IDX_CEILF16

	HCC_FUNCTION_IDX_CEILF16,
	HCC_FUNCTION_IDX_CEILF,
	HCC_FUNCTION_IDX_CEIL,
	HCC_FUNCTION_IDX_CEILV2F16,
	HCC_FUNCTION_IDX_CEILV2F32,
	HCC_FUNCTION_IDX_CEILV2F64,
	HCC_FUNCTION_IDX_CEILV3F16,
	HCC_FUNCTION_IDX_CEILV3F32,
	HCC_FUNCTION_IDX_CEILV3F64,
	HCC_FUNCTION_IDX_CEILV4F16,
	HCC_FUNCTION_IDX_CEILV4F32,
	HCC_FUNCTION_IDX_CEILV4F64,

#define HCC_FUNCTION_IDX_CEIL_END (HCC_FUNCTION_IDX_CEILV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ROUND(function_idx) \
	HCC_FUNCTION_IDX_ROUND_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ROUND_END
#define HCC_FUNCTION_IDX_ROUND_START HCC_FUNCTION_IDX_ROUNDF16

	HCC_FUNCTION_IDX_ROUNDF16,
	HCC_FUNCTION_IDX_ROUNDF,
	HCC_FUNCTION_IDX_ROUND,
	HCC_FUNCTION_IDX_ROUNDV2F16,
	HCC_FUNCTION_IDX_ROUNDV2F32,
	HCC_FUNCTION_IDX_ROUNDV2F64,
	HCC_FUNCTION_IDX_ROUNDV3F16,
	HCC_FUNCTION_IDX_ROUNDV3F32,
	HCC_FUNCTION_IDX_ROUNDV3F64,
	HCC_FUNCTION_IDX_ROUNDV4F16,
	HCC_FUNCTION_IDX_ROUNDV4F32,
	HCC_FUNCTION_IDX_ROUNDV4F64,

#define HCC_FUNCTION_IDX_ROUND_END (HCC_FUNCTION_IDX_ROUNDV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_TRUNC(function_idx) \
	HCC_FUNCTION_IDX_TRUNC_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_TRUNC_END
#define HCC_FUNCTION_IDX_TRUNC_START HCC_FUNCTION_IDX_TRUNCF16

	HCC_FUNCTION_IDX_TRUNCF16,
	HCC_FUNCTION_IDX_TRUNCF,
	HCC_FUNCTION_IDX_TRUNC,
	HCC_FUNCTION_IDX_TRUNCV2F16,
	HCC_FUNCTION_IDX_TRUNCV2F32,
	HCC_FUNCTION_IDX_TRUNCV2F64,
	HCC_FUNCTION_IDX_TRUNCV3F16,
	HCC_FUNCTION_IDX_TRUNCV3F32,
	HCC_FUNCTION_IDX_TRUNCV3F64,
	HCC_FUNCTION_IDX_TRUNCV4F16,
	HCC_FUNCTION_IDX_TRUNCV4F32,
	HCC_FUNCTION_IDX_TRUNCV4F64,

#define HCC_FUNCTION_IDX_TRUNC_END (HCC_FUNCTION_IDX_TRUNCV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_FRACT(function_idx) \
	HCC_FUNCTION_IDX_FRACT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_FRACT_END
#define HCC_FUNCTION_IDX_FRACT_START HCC_FUNCTION_IDX_FRACTF16

	HCC_FUNCTION_IDX_FRACTF16,
	HCC_FUNCTION_IDX_FRACTF32,
	HCC_FUNCTION_IDX_FRACTF64,
	HCC_FUNCTION_IDX_FRACTV2F16,
	HCC_FUNCTION_IDX_FRACTV2F32,
	HCC_FUNCTION_IDX_FRACTV2F64,
	HCC_FUNCTION_IDX_FRACTV3F16,
	HCC_FUNCTION_IDX_FRACTV3F32,
	HCC_FUNCTION_IDX_FRACTV3F64,
	HCC_FUNCTION_IDX_FRACTV4F16,
	HCC_FUNCTION_IDX_FRACTV4F32,
	HCC_FUNCTION_IDX_FRACTV4F64,

#define HCC_FUNCTION_IDX_FRACT_END (HCC_FUNCTION_IDX_FRACTV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_RADIANS(function_idx) \
	HCC_FUNCTION_IDX_RADIANS_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_RADIANS_END
#define HCC_FUNCTION_IDX_RADIANS_START HCC_FUNCTION_IDX_RADIANSF16

	HCC_FUNCTION_IDX_RADIANSF16,
	HCC_FUNCTION_IDX_RADIANSF32,
	HCC_FUNCTION_IDX_RADIANSF64,
	HCC_FUNCTION_IDX_RADIANSV2F16,
	HCC_FUNCTION_IDX_RADIANSV2F32,
	HCC_FUNCTION_IDX_RADIANSV2F64,
	HCC_FUNCTION_IDX_RADIANSV3F16,
	HCC_FUNCTION_IDX_RADIANSV3F32,
	HCC_FUNCTION_IDX_RADIANSV3F64,
	HCC_FUNCTION_IDX_RADIANSV4F16,
	HCC_FUNCTION_IDX_RADIANSV4F32,
	HCC_FUNCTION_IDX_RADIANSV4F64,

#define HCC_FUNCTION_IDX_RADIANS_END (HCC_FUNCTION_IDX_RADIANSV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_DEGREES(function_idx) \
	HCC_FUNCTION_IDX_DEGREES_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_DEGREES_END
#define HCC_FUNCTION_IDX_DEGREES_START HCC_FUNCTION_IDX_DEGREESF16

	HCC_FUNCTION_IDX_DEGREESF16,
	HCC_FUNCTION_IDX_DEGREESF32,
	HCC_FUNCTION_IDX_DEGREESF64,
	HCC_FUNCTION_IDX_DEGREESV2F16,
	HCC_FUNCTION_IDX_DEGREESV2F32,
	HCC_FUNCTION_IDX_DEGREESV2F64,
	HCC_FUNCTION_IDX_DEGREESV3F16,
	HCC_FUNCTION_IDX_DEGREESV3F32,
	HCC_FUNCTION_IDX_DEGREESV3F64,
	HCC_FUNCTION_IDX_DEGREESV4F16,
	HCC_FUNCTION_IDX_DEGREESV4F32,
	HCC_FUNCTION_IDX_DEGREESV4F64,

#define HCC_FUNCTION_IDX_DEGREES_END (HCC_FUNCTION_IDX_DEGREESV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_STEP(function_idx) \
	HCC_FUNCTION_IDX_STEP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_STEP_END
#define HCC_FUNCTION_IDX_STEP_START HCC_FUNCTION_IDX_STEPF16

	HCC_FUNCTION_IDX_STEPF16,
	HCC_FUNCTION_IDX_STEPF32,
	HCC_FUNCTION_IDX_STEPF64,
	HCC_FUNCTION_IDX_STEPV2F16,
	HCC_FUNCTION_IDX_STEPV2F32,
	HCC_FUNCTION_IDX_STEPV2F64,
	HCC_FUNCTION_IDX_STEPV3F16,
	HCC_FUNCTION_IDX_STEPV3F32,
	HCC_FUNCTION_IDX_STEPV3F64,
	HCC_FUNCTION_IDX_STEPV4F16,
	HCC_FUNCTION_IDX_STEPV4F32,
	HCC_FUNCTION_IDX_STEPV4F64,

#define HCC_FUNCTION_IDX_STEP_END (HCC_FUNCTION_IDX_STEPV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_SMOOTHSTEP(function_idx) \
	HCC_FUNCTION_IDX_SMOOTHSTEP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_SMOOTHSTEP_END
#define HCC_FUNCTION_IDX_SMOOTHSTEP_START HCC_FUNCTION_IDX_SMOOTHSTEPF16

	HCC_FUNCTION_IDX_SMOOTHSTEPF16,
	HCC_FUNCTION_IDX_SMOOTHSTEPF32,
	HCC_FUNCTION_IDX_SMOOTHSTEPF64,
	HCC_FUNCTION_IDX_SMOOTHSTEPV2F16,
	HCC_FUNCTION_IDX_SMOOTHSTEPV2F32,
	HCC_FUNCTION_IDX_SMOOTHSTEPV2F64,
	HCC_FUNCTION_IDX_SMOOTHSTEPV3F16,
	HCC_FUNCTION_IDX_SMOOTHSTEPV3F32,
	HCC_FUNCTION_IDX_SMOOTHSTEPV3F64,
	HCC_FUNCTION_IDX_SMOOTHSTEPV4F16,
	HCC_FUNCTION_IDX_SMOOTHSTEPV4F32,
	HCC_FUNCTION_IDX_SMOOTHSTEPV4F64,

#define HCC_FUNCTION_IDX_SMOOTHSTEP_END (HCC_FUNCTION_IDX_SMOOTHSTEPV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_BITSTOFROM(function_idx) \
	HCC_FUNCTION_IDX_BITSTOFROM_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_BITSTOFROM_END
#define HCC_FUNCTION_IDX_BITSTOFROM_START HCC_FUNCTION_IDX_BITSTOF16

	HCC_FUNCTION_IDX_BITSTOF16,
	HCC_FUNCTION_IDX_BITSTOF32,
	HCC_FUNCTION_IDX_BITSTOF64,
	HCC_FUNCTION_IDX_BITSFROMF16,
	HCC_FUNCTION_IDX_BITSFROMF32,
	HCC_FUNCTION_IDX_BITSFROMF64,
	HCC_FUNCTION_IDX_BITSTOV2F16,
	HCC_FUNCTION_IDX_BITSTOV2F32,
	HCC_FUNCTION_IDX_BITSTOV2F64,
	HCC_FUNCTION_IDX_BITSTOV3F16,
	HCC_FUNCTION_IDX_BITSTOV3F32,
	HCC_FUNCTION_IDX_BITSTOV3F64,
	HCC_FUNCTION_IDX_BITSTOV4F16,
	HCC_FUNCTION_IDX_BITSTOV4F32,
	HCC_FUNCTION_IDX_BITSTOV4F64,
	HCC_FUNCTION_IDX_BITSFROMV2F16,
	HCC_FUNCTION_IDX_BITSFROMV2F32,
	HCC_FUNCTION_IDX_BITSFROMV2F64,
	HCC_FUNCTION_IDX_BITSFROMV3F16,
	HCC_FUNCTION_IDX_BITSFROMV3F32,
	HCC_FUNCTION_IDX_BITSFROMV3F64,
	HCC_FUNCTION_IDX_BITSFROMV4F16,
	HCC_FUNCTION_IDX_BITSFROMV4F32,
	HCC_FUNCTION_IDX_BITSFROMV4F64,

#define HCC_FUNCTION_IDX_BITSTOFROM_END (HCC_FUNCTION_IDX_BITSFROMV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_SIN(function_idx) \
	HCC_FUNCTION_IDX_SIN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_SIN_END
#define HCC_FUNCTION_IDX_SIN_START HCC_FUNCTION_IDX_SINF16

	HCC_FUNCTION_IDX_SINF16,
	HCC_FUNCTION_IDX_SINF,
	HCC_FUNCTION_IDX_SIN,
	HCC_FUNCTION_IDX_SINV2F16,
	HCC_FUNCTION_IDX_SINV2F32,
	HCC_FUNCTION_IDX_SINV2F64,
	HCC_FUNCTION_IDX_SINV3F16,
	HCC_FUNCTION_IDX_SINV3F32,
	HCC_FUNCTION_IDX_SINV3F64,
	HCC_FUNCTION_IDX_SINV4F16,
	HCC_FUNCTION_IDX_SINV4F32,
	HCC_FUNCTION_IDX_SINV4F64,

#define HCC_FUNCTION_IDX_SIN_END (HCC_FUNCTION_IDX_SINV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_COS(function_idx) \
	HCC_FUNCTION_IDX_COS_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_COS_END
#define HCC_FUNCTION_IDX_COS_START HCC_FUNCTION_IDX_COSF16

	HCC_FUNCTION_IDX_COSF16,
	HCC_FUNCTION_IDX_COSF,
	HCC_FUNCTION_IDX_COS,
	HCC_FUNCTION_IDX_COSV2F16,
	HCC_FUNCTION_IDX_COSV2F32,
	HCC_FUNCTION_IDX_COSV2F64,
	HCC_FUNCTION_IDX_COSV3F16,
	HCC_FUNCTION_IDX_COSV3F32,
	HCC_FUNCTION_IDX_COSV3F64,
	HCC_FUNCTION_IDX_COSV4F16,
	HCC_FUNCTION_IDX_COSV4F32,
	HCC_FUNCTION_IDX_COSV4F64,

#define HCC_FUNCTION_IDX_COS_END (HCC_FUNCTION_IDX_COSV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_TAN(function_idx) \
	HCC_FUNCTION_IDX_TAN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_TAN_END
#define HCC_FUNCTION_IDX_TAN_START HCC_FUNCTION_IDX_TANF16

	HCC_FUNCTION_IDX_TANF16,
	HCC_FUNCTION_IDX_TANF,
	HCC_FUNCTION_IDX_TAN,
	HCC_FUNCTION_IDX_TANV2F16,
	HCC_FUNCTION_IDX_TANV2F32,
	HCC_FUNCTION_IDX_TANV2F64,
	HCC_FUNCTION_IDX_TANV3F16,
	HCC_FUNCTION_IDX_TANV3F32,
	HCC_FUNCTION_IDX_TANV3F64,
	HCC_FUNCTION_IDX_TANV4F16,
	HCC_FUNCTION_IDX_TANV4F32,
	HCC_FUNCTION_IDX_TANV4F64,

#define HCC_FUNCTION_IDX_TAN_END (HCC_FUNCTION_IDX_TANV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ASIN(function_idx) \
	HCC_FUNCTION_IDX_ASIN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ASIN_END
#define HCC_FUNCTION_IDX_ASIN_START HCC_FUNCTION_IDX_ASINF16

	HCC_FUNCTION_IDX_ASINF16,
	HCC_FUNCTION_IDX_ASINF,
	HCC_FUNCTION_IDX_ASIN,
	HCC_FUNCTION_IDX_ASINV2F16,
	HCC_FUNCTION_IDX_ASINV2F32,
	HCC_FUNCTION_IDX_ASINV2F64,
	HCC_FUNCTION_IDX_ASINV3F16,
	HCC_FUNCTION_IDX_ASINV3F32,
	HCC_FUNCTION_IDX_ASINV3F64,
	HCC_FUNCTION_IDX_ASINV4F16,
	HCC_FUNCTION_IDX_ASINV4F32,
	HCC_FUNCTION_IDX_ASINV4F64,

#define HCC_FUNCTION_IDX_ASIN_END (HCC_FUNCTION_IDX_ASINV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ACOS(function_idx) \
	HCC_FUNCTION_IDX_ACOS_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ACOS_END
#define HCC_FUNCTION_IDX_ACOS_START HCC_FUNCTION_IDX_ACOSF16

	HCC_FUNCTION_IDX_ACOSF16,
	HCC_FUNCTION_IDX_ACOSF,
	HCC_FUNCTION_IDX_ACOS,
	HCC_FUNCTION_IDX_ACOSV2F16,
	HCC_FUNCTION_IDX_ACOSV2F32,
	HCC_FUNCTION_IDX_ACOSV2F64,
	HCC_FUNCTION_IDX_ACOSV3F16,
	HCC_FUNCTION_IDX_ACOSV3F32,
	HCC_FUNCTION_IDX_ACOSV3F64,
	HCC_FUNCTION_IDX_ACOSV4F16,
	HCC_FUNCTION_IDX_ACOSV4F32,
	HCC_FUNCTION_IDX_ACOSV4F64,

#define HCC_FUNCTION_IDX_ACOS_END (HCC_FUNCTION_IDX_ACOSV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ATAN(function_idx) \
	HCC_FUNCTION_IDX_ATAN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ATAN_END
#define HCC_FUNCTION_IDX_ATAN_START HCC_FUNCTION_IDX_ATANF16

	HCC_FUNCTION_IDX_ATANF16,
	HCC_FUNCTION_IDX_ATANF,
	HCC_FUNCTION_IDX_ATAN,
	HCC_FUNCTION_IDX_ATANV2F16,
	HCC_FUNCTION_IDX_ATANV2F32,
	HCC_FUNCTION_IDX_ATANV2F64,
	HCC_FUNCTION_IDX_ATANV3F16,
	HCC_FUNCTION_IDX_ATANV3F32,
	HCC_FUNCTION_IDX_ATANV3F64,
	HCC_FUNCTION_IDX_ATANV4F16,
	HCC_FUNCTION_IDX_ATANV4F32,
	HCC_FUNCTION_IDX_ATANV4F64,

#define HCC_FUNCTION_IDX_ATAN_END (HCC_FUNCTION_IDX_ATANV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_SINH(function_idx) \
	HCC_FUNCTION_IDX_SINH_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_SINH_END
#define HCC_FUNCTION_IDX_SINH_START HCC_FUNCTION_IDX_SINHF16

	HCC_FUNCTION_IDX_SINHF16,
	HCC_FUNCTION_IDX_SINHF,
	HCC_FUNCTION_IDX_SINH,
	HCC_FUNCTION_IDX_SINHV2F16,
	HCC_FUNCTION_IDX_SINHV2F32,
	HCC_FUNCTION_IDX_SINHV2F64,
	HCC_FUNCTION_IDX_SINHV3F16,
	HCC_FUNCTION_IDX_SINHV3F32,
	HCC_FUNCTION_IDX_SINHV3F64,
	HCC_FUNCTION_IDX_SINHV4F16,
	HCC_FUNCTION_IDX_SINHV4F32,
	HCC_FUNCTION_IDX_SINHV4F64,

#define HCC_FUNCTION_IDX_SINH_END (HCC_FUNCTION_IDX_SINHV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_COSH(function_idx) \
	HCC_FUNCTION_IDX_COSH_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_COSH_END
#define HCC_FUNCTION_IDX_COSH_START HCC_FUNCTION_IDX_COSHF16

	HCC_FUNCTION_IDX_COSHF16,
	HCC_FUNCTION_IDX_COSHF,
	HCC_FUNCTION_IDX_COSH,
	HCC_FUNCTION_IDX_COSHV2F16,
	HCC_FUNCTION_IDX_COSHV2F32,
	HCC_FUNCTION_IDX_COSHV2F64,
	HCC_FUNCTION_IDX_COSHV3F16,
	HCC_FUNCTION_IDX_COSHV3F32,
	HCC_FUNCTION_IDX_COSHV3F64,
	HCC_FUNCTION_IDX_COSHV4F16,
	HCC_FUNCTION_IDX_COSHV4F32,
	HCC_FUNCTION_IDX_COSHV4F64,

#define HCC_FUNCTION_IDX_COSH_END (HCC_FUNCTION_IDX_COSHV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_TANH(function_idx) \
	HCC_FUNCTION_IDX_TANH_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_TANH_END
#define HCC_FUNCTION_IDX_TANH_START HCC_FUNCTION_IDX_TANHF16

	HCC_FUNCTION_IDX_TANHF16,
	HCC_FUNCTION_IDX_TANHF,
	HCC_FUNCTION_IDX_TANH,
	HCC_FUNCTION_IDX_TANHV2F16,
	HCC_FUNCTION_IDX_TANHV2F32,
	HCC_FUNCTION_IDX_TANHV2F64,
	HCC_FUNCTION_IDX_TANHV3F16,
	HCC_FUNCTION_IDX_TANHV3F32,
	HCC_FUNCTION_IDX_TANHV3F64,
	HCC_FUNCTION_IDX_TANHV4F16,
	HCC_FUNCTION_IDX_TANHV4F32,
	HCC_FUNCTION_IDX_TANHV4F64,

#define HCC_FUNCTION_IDX_TANH_END (HCC_FUNCTION_IDX_TANHV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ASINH(function_idx) \
	HCC_FUNCTION_IDX_ASINH_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ASINH_END
#define HCC_FUNCTION_IDX_ASINH_START HCC_FUNCTION_IDX_ASINHF16

	HCC_FUNCTION_IDX_ASINHF16,
	HCC_FUNCTION_IDX_ASINHF,
	HCC_FUNCTION_IDX_ASINH,
	HCC_FUNCTION_IDX_ASINHV2F16,
	HCC_FUNCTION_IDX_ASINHV2F32,
	HCC_FUNCTION_IDX_ASINHV2F64,
	HCC_FUNCTION_IDX_ASINHV3F16,
	HCC_FUNCTION_IDX_ASINHV3F32,
	HCC_FUNCTION_IDX_ASINHV3F64,
	HCC_FUNCTION_IDX_ASINHV4F16,
	HCC_FUNCTION_IDX_ASINHV4F32,
	HCC_FUNCTION_IDX_ASINHV4F64,

#define HCC_FUNCTION_IDX_ASINH_END (HCC_FUNCTION_IDX_ASINHV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ACOSH(function_idx) \
	HCC_FUNCTION_IDX_ACOSH_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ACOSH_END
#define HCC_FUNCTION_IDX_ACOSH_START HCC_FUNCTION_IDX_ACOSHF16

	HCC_FUNCTION_IDX_ACOSHF16,
	HCC_FUNCTION_IDX_ACOSHF,
	HCC_FUNCTION_IDX_ACOSH,
	HCC_FUNCTION_IDX_ACOSHV2F16,
	HCC_FUNCTION_IDX_ACOSHV2F32,
	HCC_FUNCTION_IDX_ACOSHV2F64,
	HCC_FUNCTION_IDX_ACOSHV3F16,
	HCC_FUNCTION_IDX_ACOSHV3F32,
	HCC_FUNCTION_IDX_ACOSHV3F64,
	HCC_FUNCTION_IDX_ACOSHV4F16,
	HCC_FUNCTION_IDX_ACOSHV4F32,
	HCC_FUNCTION_IDX_ACOSHV4F64,

#define HCC_FUNCTION_IDX_ACOSH_END (HCC_FUNCTION_IDX_ACOSHV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ATANH(function_idx) \
	HCC_FUNCTION_IDX_ATANH_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ATANH_END
#define HCC_FUNCTION_IDX_ATANH_START HCC_FUNCTION_IDX_ATANHF16

	HCC_FUNCTION_IDX_ATANHF16,
	HCC_FUNCTION_IDX_ATANHF,
	HCC_FUNCTION_IDX_ATANH,
	HCC_FUNCTION_IDX_ATANHV2F16,
	HCC_FUNCTION_IDX_ATANHV2F32,
	HCC_FUNCTION_IDX_ATANHV2F64,
	HCC_FUNCTION_IDX_ATANHV3F16,
	HCC_FUNCTION_IDX_ATANHV3F32,
	HCC_FUNCTION_IDX_ATANHV3F64,
	HCC_FUNCTION_IDX_ATANHV4F16,
	HCC_FUNCTION_IDX_ATANHV4F32,
	HCC_FUNCTION_IDX_ATANHV4F64,

#define HCC_FUNCTION_IDX_ATANH_END (HCC_FUNCTION_IDX_ATANHV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ATAN2(function_idx) \
	HCC_FUNCTION_IDX_ATAN2_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ATAN2_END
#define HCC_FUNCTION_IDX_ATAN2_START HCC_FUNCTION_IDX_ATAN2F16

	HCC_FUNCTION_IDX_ATAN2F16,
	HCC_FUNCTION_IDX_ATAN2F,
	HCC_FUNCTION_IDX_ATAN2,
	HCC_FUNCTION_IDX_ATAN2V2F16,
	HCC_FUNCTION_IDX_ATAN2V2F32,
	HCC_FUNCTION_IDX_ATAN2V2F64,
	HCC_FUNCTION_IDX_ATAN2V3F16,
	HCC_FUNCTION_IDX_ATAN2V3F32,
	HCC_FUNCTION_IDX_ATAN2V3F64,
	HCC_FUNCTION_IDX_ATAN2V4F16,
	HCC_FUNCTION_IDX_ATAN2V4F32,
	HCC_FUNCTION_IDX_ATAN2V4F64,

#define HCC_FUNCTION_IDX_ATAN2_END (HCC_FUNCTION_IDX_ATAN2V4F64 + 1)

#define HCC_FUNCTION_IDX_IS_POW(function_idx) \
	HCC_FUNCTION_IDX_POW_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_POW_END
#define HCC_FUNCTION_IDX_POW_START HCC_FUNCTION_IDX_POWF16

	HCC_FUNCTION_IDX_POWF16,
	HCC_FUNCTION_IDX_POWF,
	HCC_FUNCTION_IDX_POW,
	HCC_FUNCTION_IDX_POWV2F16,
	HCC_FUNCTION_IDX_POWV2F32,
	HCC_FUNCTION_IDX_POWV2F64,
	HCC_FUNCTION_IDX_POWV3F16,
	HCC_FUNCTION_IDX_POWV3F32,
	HCC_FUNCTION_IDX_POWV3F64,
	HCC_FUNCTION_IDX_POWV4F16,
	HCC_FUNCTION_IDX_POWV4F32,
	HCC_FUNCTION_IDX_POWV4F64,

#define HCC_FUNCTION_IDX_POW_END (HCC_FUNCTION_IDX_POWV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_EXP(function_idx) \
	HCC_FUNCTION_IDX_EXP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_EXP_END
#define HCC_FUNCTION_IDX_EXP_START HCC_FUNCTION_IDX_EXPF16

	HCC_FUNCTION_IDX_EXPF16,
	HCC_FUNCTION_IDX_EXPF,
	HCC_FUNCTION_IDX_EXP,
	HCC_FUNCTION_IDX_EXPV2F16,
	HCC_FUNCTION_IDX_EXPV2F32,
	HCC_FUNCTION_IDX_EXPV2F64,
	HCC_FUNCTION_IDX_EXPV3F16,
	HCC_FUNCTION_IDX_EXPV3F32,
	HCC_FUNCTION_IDX_EXPV3F64,
	HCC_FUNCTION_IDX_EXPV4F16,
	HCC_FUNCTION_IDX_EXPV4F32,
	HCC_FUNCTION_IDX_EXPV4F64,

#define HCC_FUNCTION_IDX_EXP_END (HCC_FUNCTION_IDX_EXPV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_LOG(function_idx) \
	HCC_FUNCTION_IDX_LOG_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_LOG_END
#define HCC_FUNCTION_IDX_LOG_START HCC_FUNCTION_IDX_LOGF16

	HCC_FUNCTION_IDX_LOGF16,
	HCC_FUNCTION_IDX_LOGF,
	HCC_FUNCTION_IDX_LOG,
	HCC_FUNCTION_IDX_LOGV2F16,
	HCC_FUNCTION_IDX_LOGV2F32,
	HCC_FUNCTION_IDX_LOGV2F64,
	HCC_FUNCTION_IDX_LOGV3F16,
	HCC_FUNCTION_IDX_LOGV3F32,
	HCC_FUNCTION_IDX_LOGV3F64,
	HCC_FUNCTION_IDX_LOGV4F16,
	HCC_FUNCTION_IDX_LOGV4F32,
	HCC_FUNCTION_IDX_LOGV4F64,

#define HCC_FUNCTION_IDX_LOG_END (HCC_FUNCTION_IDX_LOGV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_EXP2(function_idx) \
	HCC_FUNCTION_IDX_EXP2_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_EXP2_END
#define HCC_FUNCTION_IDX_EXP2_START HCC_FUNCTION_IDX_EXP2F16

	HCC_FUNCTION_IDX_EXP2F16,
	HCC_FUNCTION_IDX_EXP2F,
	HCC_FUNCTION_IDX_EXP2,
	HCC_FUNCTION_IDX_EXP2V2F16,
	HCC_FUNCTION_IDX_EXP2V2F32,
	HCC_FUNCTION_IDX_EXP2V2F64,
	HCC_FUNCTION_IDX_EXP2V3F16,
	HCC_FUNCTION_IDX_EXP2V3F32,
	HCC_FUNCTION_IDX_EXP2V3F64,
	HCC_FUNCTION_IDX_EXP2V4F16,
	HCC_FUNCTION_IDX_EXP2V4F32,
	HCC_FUNCTION_IDX_EXP2V4F64,

#define HCC_FUNCTION_IDX_EXP2_END (HCC_FUNCTION_IDX_EXP2V4F64 + 1)

#define HCC_FUNCTION_IDX_IS_LOG2(function_idx) \
	HCC_FUNCTION_IDX_LOG2_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_LOG2_END
#define HCC_FUNCTION_IDX_LOG2_START HCC_FUNCTION_IDX_LOG2F16

	HCC_FUNCTION_IDX_LOG2F16,
	HCC_FUNCTION_IDX_LOG2F,
	HCC_FUNCTION_IDX_LOG2,
	HCC_FUNCTION_IDX_LOG2V2F16,
	HCC_FUNCTION_IDX_LOG2V2F32,
	HCC_FUNCTION_IDX_LOG2V2F64,
	HCC_FUNCTION_IDX_LOG2V3F16,
	HCC_FUNCTION_IDX_LOG2V3F32,
	HCC_FUNCTION_IDX_LOG2V3F64,
	HCC_FUNCTION_IDX_LOG2V4F16,
	HCC_FUNCTION_IDX_LOG2V4F32,
	HCC_FUNCTION_IDX_LOG2V4F64,

#define HCC_FUNCTION_IDX_LOG2_END (HCC_FUNCTION_IDX_LOG2V4F64 + 1)

#define HCC_FUNCTION_IDX_IS_SQRT(function_idx) \
	HCC_FUNCTION_IDX_SQRT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_SQRT_END
#define HCC_FUNCTION_IDX_SQRT_START HCC_FUNCTION_IDX_SQRTF16

	HCC_FUNCTION_IDX_SQRTF16,
	HCC_FUNCTION_IDX_SQRTF,
	HCC_FUNCTION_IDX_SQRT,
	HCC_FUNCTION_IDX_SQRTV2F16,
	HCC_FUNCTION_IDX_SQRTV2F32,
	HCC_FUNCTION_IDX_SQRTV2F64,
	HCC_FUNCTION_IDX_SQRTV3F16,
	HCC_FUNCTION_IDX_SQRTV3F32,
	HCC_FUNCTION_IDX_SQRTV3F64,
	HCC_FUNCTION_IDX_SQRTV4F16,
	HCC_FUNCTION_IDX_SQRTV4F32,
	HCC_FUNCTION_IDX_SQRTV4F64,

#define HCC_FUNCTION_IDX_SQRT_END (HCC_FUNCTION_IDX_SQRTV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_RSQRT(function_idx) \
	HCC_FUNCTION_IDX_RSQRT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_RSQRT_END
#define HCC_FUNCTION_IDX_RSQRT_START HCC_FUNCTION_IDX_RSQRTF16

	HCC_FUNCTION_IDX_RSQRTF16,
	HCC_FUNCTION_IDX_RSQRTF32,
	HCC_FUNCTION_IDX_RSQRTF64,
	HCC_FUNCTION_IDX_RSQRTV2F16,
	HCC_FUNCTION_IDX_RSQRTV2F32,
	HCC_FUNCTION_IDX_RSQRTV2F64,
	HCC_FUNCTION_IDX_RSQRTV3F16,
	HCC_FUNCTION_IDX_RSQRTV3F32,
	HCC_FUNCTION_IDX_RSQRTV3F64,
	HCC_FUNCTION_IDX_RSQRTV4F16,
	HCC_FUNCTION_IDX_RSQRTV4F32,
	HCC_FUNCTION_IDX_RSQRTV4F64,

#define HCC_FUNCTION_IDX_RSQRT_END (HCC_FUNCTION_IDX_RSQRTV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ISINF(function_idx) \
	HCC_FUNCTION_IDX_ISINF_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ISINF_END
#define HCC_FUNCTION_IDX_ISINF_START HCC_FUNCTION_IDX_ISINFF16

	HCC_FUNCTION_IDX_ISINFF16,
	HCC_FUNCTION_IDX_ISINF,
	HCC_FUNCTION_IDX_ISINFV2F16,
	HCC_FUNCTION_IDX_ISINFV2F32,
	HCC_FUNCTION_IDX_ISINFV2F64,
	HCC_FUNCTION_IDX_ISINFV3F16,
	HCC_FUNCTION_IDX_ISINFV3F32,
	HCC_FUNCTION_IDX_ISINFV3F64,
	HCC_FUNCTION_IDX_ISINFV4F16,
	HCC_FUNCTION_IDX_ISINFV4F32,
	HCC_FUNCTION_IDX_ISINFV4F64,

#define HCC_FUNCTION_IDX_ISINF_END (HCC_FUNCTION_IDX_ISINFV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_ISNAN(function_idx) \
	HCC_FUNCTION_IDX_ISNAN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_ISNAN_END
#define HCC_FUNCTION_IDX_ISNAN_START HCC_FUNCTION_IDX_ISNANF16

	HCC_FUNCTION_IDX_ISNANF16,
	HCC_FUNCTION_IDX_ISNAN,
	HCC_FUNCTION_IDX_ISNANV2F16,
	HCC_FUNCTION_IDX_ISNANV2F32,
	HCC_FUNCTION_IDX_ISNANV2F64,
	HCC_FUNCTION_IDX_ISNANV3F16,
	HCC_FUNCTION_IDX_ISNANV3F32,
	HCC_FUNCTION_IDX_ISNANV3F64,
	HCC_FUNCTION_IDX_ISNANV4F16,
	HCC_FUNCTION_IDX_ISNANV4F32,
	HCC_FUNCTION_IDX_ISNANV4F64,

#define HCC_FUNCTION_IDX_ISNAN_END (HCC_FUNCTION_IDX_ISNANV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_LERP(function_idx) \
	HCC_FUNCTION_IDX_LERP_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_LERP_END
#define HCC_FUNCTION_IDX_LERP_START HCC_FUNCTION_IDX_LERPF16

	HCC_FUNCTION_IDX_LERPF16,
	HCC_FUNCTION_IDX_LERPF32,
	HCC_FUNCTION_IDX_LERPF64,
	HCC_FUNCTION_IDX_LERPV2F16,
	HCC_FUNCTION_IDX_LERPV2F32,
	HCC_FUNCTION_IDX_LERPV2F64,
	HCC_FUNCTION_IDX_LERPV3F16,
	HCC_FUNCTION_IDX_LERPV3F32,
	HCC_FUNCTION_IDX_LERPV3F64,
	HCC_FUNCTION_IDX_LERPV4F16,
	HCC_FUNCTION_IDX_LERPV4F32,
	HCC_FUNCTION_IDX_LERPV4F64,

#define HCC_FUNCTION_IDX_LERP_END (HCC_FUNCTION_IDX_LERPV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_DOT(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_DOT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_DOT_END
#define HCC_FUNCTION_IDX_VECTOR_DOT_START HCC_FUNCTION_IDX_DOTV2F16

	HCC_FUNCTION_IDX_DOTV2F16,
	HCC_FUNCTION_IDX_DOTV2F32,
	HCC_FUNCTION_IDX_DOTV2F64,
	HCC_FUNCTION_IDX_DOTV3F16,
	HCC_FUNCTION_IDX_DOTV3F32,
	HCC_FUNCTION_IDX_DOTV3F64,
	HCC_FUNCTION_IDX_DOTV4F16,
	HCC_FUNCTION_IDX_DOTV4F32,
	HCC_FUNCTION_IDX_DOTV4F64,

#define HCC_FUNCTION_IDX_VECTOR_DOT_END (HCC_FUNCTION_IDX_DOTV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_LEN(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_LEN_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_LEN_END
#define HCC_FUNCTION_IDX_VECTOR_LEN_START HCC_FUNCTION_IDX_LENV2F16

	HCC_FUNCTION_IDX_LENV2F16,
	HCC_FUNCTION_IDX_LENV2F32,
	HCC_FUNCTION_IDX_LENV2F64,
	HCC_FUNCTION_IDX_LENV3F16,
	HCC_FUNCTION_IDX_LENV3F32,
	HCC_FUNCTION_IDX_LENV3F64,
	HCC_FUNCTION_IDX_LENV4F16,
	HCC_FUNCTION_IDX_LENV4F32,
	HCC_FUNCTION_IDX_LENV4F64,

#define HCC_FUNCTION_IDX_VECTOR_LEN_END (HCC_FUNCTION_IDX_LENV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_NORM(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_NORM_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_NORM_END
#define HCC_FUNCTION_IDX_VECTOR_NORM_START HCC_FUNCTION_IDX_NORMV2F16

	HCC_FUNCTION_IDX_NORMV2F16,
	HCC_FUNCTION_IDX_NORMV2F32,
	HCC_FUNCTION_IDX_NORMV2F64,
	HCC_FUNCTION_IDX_NORMV3F16,
	HCC_FUNCTION_IDX_NORMV3F32,
	HCC_FUNCTION_IDX_NORMV3F64,
	HCC_FUNCTION_IDX_NORMV4F16,
	HCC_FUNCTION_IDX_NORMV4F32,
	HCC_FUNCTION_IDX_NORMV4F64,

#define HCC_FUNCTION_IDX_VECTOR_NORM_END (HCC_FUNCTION_IDX_NORMV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_REFLECT(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_REFLECT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_REFLECT_END
#define HCC_FUNCTION_IDX_VECTOR_REFLECT_START HCC_FUNCTION_IDX_REFLECTV2F16

	HCC_FUNCTION_IDX_REFLECTV2F16,
	HCC_FUNCTION_IDX_REFLECTV2F32,
	HCC_FUNCTION_IDX_REFLECTV2F64,
	HCC_FUNCTION_IDX_REFLECTV3F16,
	HCC_FUNCTION_IDX_REFLECTV3F32,
	HCC_FUNCTION_IDX_REFLECTV3F64,
	HCC_FUNCTION_IDX_REFLECTV4F16,
	HCC_FUNCTION_IDX_REFLECTV4F32,
	HCC_FUNCTION_IDX_REFLECTV4F64,

#define HCC_FUNCTION_IDX_VECTOR_REFLECT_END (HCC_FUNCTION_IDX_REFLECTV4F64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_REFRACT(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_REFRACT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_REFRACT_END
#define HCC_FUNCTION_IDX_VECTOR_REFRACT_START HCC_FUNCTION_IDX_REFRACTV2F16

	HCC_FUNCTION_IDX_REFRACTV2F16,
	HCC_FUNCTION_IDX_REFRACTV2F32,
	HCC_FUNCTION_IDX_REFRACTV2F64,
	HCC_FUNCTION_IDX_REFRACTV3F16,
	HCC_FUNCTION_IDX_REFRACTV3F32,
	HCC_FUNCTION_IDX_REFRACTV3F64,
	HCC_FUNCTION_IDX_REFRACTV4F16,
	HCC_FUNCTION_IDX_REFRACTV4F32,
	HCC_FUNCTION_IDX_REFRACTV4F64,

#define HCC_FUNCTION_IDX_VECTOR_REFRACT_END (HCC_FUNCTION_IDX_REFRACTV4F64 + 1)

	HCC_FUNCTION_IDX_PACKF16X2V2F32,
	HCC_FUNCTION_IDX_UNPACKF16X2V2F32,
	HCC_FUNCTION_IDX_PACKU16X2V2F32,
	HCC_FUNCTION_IDX_UNPACKU16X2V2F32,
	HCC_FUNCTION_IDX_PACKS16X2V2F32,
	HCC_FUNCTION_IDX_UNPACKS16X2V2F32,
	HCC_FUNCTION_IDX_PACKU8X4V4F32,
	HCC_FUNCTION_IDX_UNPACKU8X4V4F32,
	HCC_FUNCTION_IDX_PACKS8X4V4F32,
	HCC_FUNCTION_IDX_UNPACKS8X4V4F32,

#define HCC_FUNCTION_IDX_IS_MATRIX_MUL(function_idx) \
	HCC_FUNCTION_IDX_MATRIX_MUL_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MATRIX_MUL_END
#define HCC_FUNCTION_IDX_MATRIX_MUL_START HCC_FUNCTION_IDX_MULM22M22F32

	HCC_FUNCTION_IDX_MULM22M22F32,
	HCC_FUNCTION_IDX_MULM22M22F64,
	HCC_FUNCTION_IDX_MULM23M32F32,
	HCC_FUNCTION_IDX_MULM23M32F64,
	HCC_FUNCTION_IDX_MULM24M42F32,
	HCC_FUNCTION_IDX_MULM24M42F64,
	HCC_FUNCTION_IDX_MULM32M23F32,
	HCC_FUNCTION_IDX_MULM32M23F64,
	HCC_FUNCTION_IDX_MULM33M33F32,
	HCC_FUNCTION_IDX_MULM33M33F64,
	HCC_FUNCTION_IDX_MULM34M43F32,
	HCC_FUNCTION_IDX_MULM34M43F64,
	HCC_FUNCTION_IDX_MULM42M24F32,
	HCC_FUNCTION_IDX_MULM42M24F64,
	HCC_FUNCTION_IDX_MULM43M34F32,
	HCC_FUNCTION_IDX_MULM43M34F64,
	HCC_FUNCTION_IDX_MULM44M44F32,
	HCC_FUNCTION_IDX_MULM44M44F64,

#define HCC_FUNCTION_IDX_MATRIX_MUL_END (HCC_FUNCTION_IDX_MULM44M44F64 + 1)

#define HCC_FUNCTION_IDX_IS_MATRIX_MUL_SCALAR(function_idx) \
	HCC_FUNCTION_IDX_MATRIX_MUL_SCALAR_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MATRIX_MUL_SCALAR_END
#define HCC_FUNCTION_IDX_MATRIX_MUL_SCALAR_START HCC_FUNCTION_IDX_MULSM22F32

	HCC_FUNCTION_IDX_MULSM22F32,
	HCC_FUNCTION_IDX_MULSM22F64,
	HCC_FUNCTION_IDX_MULSM23F32,
	HCC_FUNCTION_IDX_MULSM23F64,
	HCC_FUNCTION_IDX_MULSM24F32,
	HCC_FUNCTION_IDX_MULSM24F64,
	HCC_FUNCTION_IDX_MULSM32F32,
	HCC_FUNCTION_IDX_MULSM32F64,
	HCC_FUNCTION_IDX_MULSM33F32,
	HCC_FUNCTION_IDX_MULSM33F64,
	HCC_FUNCTION_IDX_MULSM34F32,
	HCC_FUNCTION_IDX_MULSM34F64,
	HCC_FUNCTION_IDX_MULSM42F32,
	HCC_FUNCTION_IDX_MULSM42F64,
	HCC_FUNCTION_IDX_MULSM43F32,
	HCC_FUNCTION_IDX_MULSM43F64,
	HCC_FUNCTION_IDX_MULSM44F32,
	HCC_FUNCTION_IDX_MULSM44F64,

#define HCC_FUNCTION_IDX_MATRIX_MUL_SCALAR_END (HCC_FUNCTION_IDX_MULSM44F64 + 1)

#define HCC_FUNCTION_IDX_IS_MATRIX_MUL_VECTOR(function_idx) \
	HCC_FUNCTION_IDX_MATRIX_MUL_VECTOR_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MATRIX_MUL_VECTOR_END
#define HCC_FUNCTION_IDX_MATRIX_MUL_VECTOR_START HCC_FUNCTION_IDX_MULM22V2F32

	HCC_FUNCTION_IDX_MULM22V2F32,
	HCC_FUNCTION_IDX_MULM22V2F64,
	HCC_FUNCTION_IDX_MULM23V2F32,
	HCC_FUNCTION_IDX_MULM23V2F64,
	HCC_FUNCTION_IDX_MULM24V2F32,
	HCC_FUNCTION_IDX_MULM24V2F64,
	HCC_FUNCTION_IDX_MULM32V3F32,
	HCC_FUNCTION_IDX_MULM32V3F64,
	HCC_FUNCTION_IDX_MULM33V3F32,
	HCC_FUNCTION_IDX_MULM33V3F64,
	HCC_FUNCTION_IDX_MULM34V3F32,
	HCC_FUNCTION_IDX_MULM34V3F64,
	HCC_FUNCTION_IDX_MULM42V4F32,
	HCC_FUNCTION_IDX_MULM42V4F64,
	HCC_FUNCTION_IDX_MULM43V4F32,
	HCC_FUNCTION_IDX_MULM43V4F64,
	HCC_FUNCTION_IDX_MULM44V4F32,
	HCC_FUNCTION_IDX_MULM44V4F64,

#define HCC_FUNCTION_IDX_MATRIX_MUL_VECTOR_END (HCC_FUNCTION_IDX_MULM44V4F64 + 1)

#define HCC_FUNCTION_IDX_IS_VECTOR_MUL_MATRIX(function_idx) \
	HCC_FUNCTION_IDX_VECTOR_MUL_MATRIX_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_VECTOR_MUL_MATRIX_END
#define HCC_FUNCTION_IDX_VECTOR_MUL_MATRIX_START HCC_FUNCTION_IDX_MULV2F32M22

	HCC_FUNCTION_IDX_MULV2F32M22,
	HCC_FUNCTION_IDX_MULV2F64M22,
	HCC_FUNCTION_IDX_MULV3F32M23,
	HCC_FUNCTION_IDX_MULV3F64M23,
	HCC_FUNCTION_IDX_MULV4F32M24,
	HCC_FUNCTION_IDX_MULV4F64M24,
	HCC_FUNCTION_IDX_MULV2F32M32,
	HCC_FUNCTION_IDX_MULV2F64M32,
	HCC_FUNCTION_IDX_MULV3F32M33,
	HCC_FUNCTION_IDX_MULV3F64M33,
	HCC_FUNCTION_IDX_MULV4F32M34,
	HCC_FUNCTION_IDX_MULV4F64M34,
	HCC_FUNCTION_IDX_MULV2F32M42,
	HCC_FUNCTION_IDX_MULV2F64M42,
	HCC_FUNCTION_IDX_MULV3F32M43,
	HCC_FUNCTION_IDX_MULV3F64M43,
	HCC_FUNCTION_IDX_MULV4F32M44,
	HCC_FUNCTION_IDX_MULV4F64M44,

#define HCC_FUNCTION_IDX_VECTOR_MUL_MATRIX_END (HCC_FUNCTION_IDX_MULV4F64M44 + 1)

#define HCC_FUNCTION_IDX_IS_MATRIX_TRANSPOSE(function_idx) \
	HCC_FUNCTION_IDX_MATRIX_TRANSPOSE_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MATRIX_TRANSPOSE_END
#define HCC_FUNCTION_IDX_MATRIX_TRANSPOSE_START HCC_FUNCTION_IDX_TRANSPOSEM22F32

	HCC_FUNCTION_IDX_TRANSPOSEM22F32,
	HCC_FUNCTION_IDX_TRANSPOSEM22F64,
	HCC_FUNCTION_IDX_TRANSPOSEM23F32,
	HCC_FUNCTION_IDX_TRANSPOSEM23F64,
	HCC_FUNCTION_IDX_TRANSPOSEM24F32,
	HCC_FUNCTION_IDX_TRANSPOSEM24F64,
	HCC_FUNCTION_IDX_TRANSPOSEM32F32,
	HCC_FUNCTION_IDX_TRANSPOSEM32F64,
	HCC_FUNCTION_IDX_TRANSPOSEM33F32,
	HCC_FUNCTION_IDX_TRANSPOSEM33F64,
	HCC_FUNCTION_IDX_TRANSPOSEM34F32,
	HCC_FUNCTION_IDX_TRANSPOSEM34F64,
	HCC_FUNCTION_IDX_TRANSPOSEM42F32,
	HCC_FUNCTION_IDX_TRANSPOSEM42F64,
	HCC_FUNCTION_IDX_TRANSPOSEM43F32,
	HCC_FUNCTION_IDX_TRANSPOSEM43F64,
	HCC_FUNCTION_IDX_TRANSPOSEM44F32,
	HCC_FUNCTION_IDX_TRANSPOSEM44F64,

#define HCC_FUNCTION_IDX_MATRIX_TRANSPOSE_END (HCC_FUNCTION_IDX_TRANSPOSEM44F64 + 1)

#define HCC_FUNCTION_IDX_IS_MATRIX_OUTER_PRODUCT(function_idx) \
	HCC_FUNCTION_IDX_MATRIX_OUTER_PRODUCT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MATRIX_OUTER_PRODUCT_END
#define HCC_FUNCTION_IDX_MATRIX_OUTER_PRODUCT_START HCC_FUNCTION_IDX_OUTERPRODUCTV2V2F32

	HCC_FUNCTION_IDX_OUTERPRODUCTV2V2F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV2V2F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV2V3F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV2V3F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV2V4F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV2V4F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV3V2F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV3V2F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV3V3F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV3V3F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV3V4F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV3V4F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV4V2F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV4V2F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV4V3F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV4V3F64,
	HCC_FUNCTION_IDX_OUTERPRODUCTV4V4F32,
	HCC_FUNCTION_IDX_OUTERPRODUCTV4V4F64,

#define HCC_FUNCTION_IDX_MATRIX_OUTER_PRODUCT_END (HCC_FUNCTION_IDX_OUTERPRODUCTV4V4F64 + 1)

#define HCC_FUNCTION_IDX_IS_MATRIX_DETERMINANT(function_idx) \
	HCC_FUNCTION_IDX_MATRIX_DETERMINANT_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MATRIX_DETERMINANT_END
#define HCC_FUNCTION_IDX_MATRIX_DETERMINANT_START HCC_FUNCTION_IDX_DETERMINANTM22F32

	HCC_FUNCTION_IDX_DETERMINANTM22F32,
	HCC_FUNCTION_IDX_DETERMINANTM22F64,
	HCC_FUNCTION_IDX_DETERMINANTM33F32,
	HCC_FUNCTION_IDX_DETERMINANTM33F64,
	HCC_FUNCTION_IDX_DETERMINANTM44F32,
	HCC_FUNCTION_IDX_DETERMINANTM44F64,

#define HCC_FUNCTION_IDX_MATRIX_DETERMINANT_END (HCC_FUNCTION_IDX_DETERMINANTM44F64 + 1)

#define HCC_FUNCTION_IDX_IS_MATRIX_INVERSE(function_idx) \
	HCC_FUNCTION_IDX_MATRIX_INVERSE_START <= (function_idx) && (function_idx) < HCC_FUNCTION_IDX_MATRIX_INVERSE_END
#define HCC_FUNCTION_IDX_MATRIX_INVERSE_START HCC_FUNCTION_IDX_INVERSEM22F32

	HCC_FUNCTION_IDX_INVERSEM22F32,
	HCC_FUNCTION_IDX_INVERSEM22F64,
	HCC_FUNCTION_IDX_INVERSEM33F32,
	HCC_FUNCTION_IDX_INVERSEM33F64,
	HCC_FUNCTION_IDX_INVERSEM44F32,
	HCC_FUNCTION_IDX_INVERSEM44F64,

#define HCC_FUNCTION_IDX_MATRIX_INVERSE_END (HCC_FUNCTION_IDX_INVERSEM44F64 + 1)

#define HCC_FUNCTION_IDX_INTRINSIC_END HCC_FUNCTION_IDX_USER_START
	HCC_FUNCTION_IDX_USER_START,
};

enum {
	HCC_VERTEX_INPUT_VERTEX_INDEX,
	HCC_VERTEX_INPUT_INSTANCE_INDEX,
};

enum {
	HCC_FRAGMENT_INPUT_FRAG_COORD,
};

typedef struct HccIntrinsicTypedef HccIntrinsicTypedef;
struct HccIntrinsicTypedef {
	HccStringId string_id;
	HccDataType aliased_data_type;
};

typedef struct HccIntrinsicStructField HccIntrinsicStructField;
struct HccIntrinsicStructField {
	HccDataType data_type;
	HccStringId string_id;
};

#define HCC_INTRINSIC_STRUCT_FIELDS_CAP 16

typedef struct HccIntrinsicStruct HccIntrinsicStruct;
struct HccIntrinsicStruct {
	HccStringId string_id;
	uint32_t fields_count;
	HccIntrinsicStructField fields[HCC_INTRINSIC_STRUCT_FIELDS_CAP];
};

typedef struct HccIntrinsicFunction HccIntrinsicFunction;
struct HccIntrinsicFunction {
	HccDataType return_data_type;
	uint32_t    params_count;
	HccDataType param_data_types[6];
};

extern HccIntrinsicTypedef hcc_intrinsic_typedefs[HCC_TYPEDEF_IDX_INTRINSIC_END];
extern HccIntrinsicStruct hcc_intrinsic_structs[HCC_COMPOUND_DATA_TYPE_IDX_INTRINSIC_END];
extern HccIntrinsicFunction hcc_intrinsic_functions[HCC_FUNCTION_IDX_INTRINSIC_END];
extern const char* hcc_intrinsic_function_strings[HCC_FUNCTION_IDX_INTRINSIC_END];

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

typedef struct HccPPEval HccPPEval;
struct HccPPEval {
	union {
		uint64_t u64;
		int64_t s64;
	};
	bool is_signed;
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
	HccCU*                   cu;
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
// AST Expr
//
//
// ===========================================

typedef struct HccASTExpr HccASTExpr;
struct HccASTExpr {
	uint32_t placeholder;
};

// ===========================================
//
//
// AST File
//
//
// ===========================================

typedef struct HccASTFile HccASTFile;
struct HccASTFile {
	HccString             path;
	HccATAIter            iter;
	HccStack(HccPPMacro)  macros;
	HccStack(HccStringId) macro_params;
	HccStack(HccStringId) pragma_onced_files;
	HccStack(HccStringId) unique_included_files;
	HccATATokenBag        token_bag;
	HccATATokenBag        macro_token_bag;
};

void hcc_ast_file_init(HccASTFile* file, HccASTFileSetup* setup, HccString path);
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
	HccStack(HccASTVariable)      function_params_and_variables;
	HccStack(HccASTFunction)      functions;
	HccStack(HccASTExpr)          exprs;
	HccStack(HccLocation)         expr_locations;
	HccStack(HccASTVariable)      global_variables;
};

void hcc_ast_init(HccCU* cu, HccASTSetup* setup);
void hcc_ast_deinit(HccCU* cu);
void hcc_ast_add_file(HccCU* cu, HccString file_path, HccASTFile** out);
HccASTFile* hcc_ast_find_file(HccCU* cu, HccString file_path);
void hcc_ast_print(HccCU* cu, HccIIO* iio);

// ===========================================
//
//
// AML
//
//
// ===========================================

typedef struct HccAML HccAML;
struct HccAML {
	uint32_t placeholder;
};

void hcc_aml_init(HccCU* cu, HccAMLSetup* setup);
void hcc_aml_deinit(HccCU* cu);
void hcc_aml_print(HccCU* cu, HccIIO* iio);

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

typedef struct HccCU HccCU;
struct HccCU {
	HccTarget        target;
	HccConstantTable constant_table;
	HccDataTypeTable dtt;
	HccAST           ast;
	HccAML           aml;
};

void hcc_cu_init(HccCU* cu, HccCUSetup* setup, HccTarget* target);
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
	HccStack(HccMessage)  deferred_elmts;
	HccStack(HccLocation) locations;
	HccStack(char)        strings;
	HccMessageType        used_type_flags;
	bool                  next_is_deferred;
};

extern const char* hcc_message_type_lang_strings[HCC_LANG_COUNT][HCC_MESSAGE_TYPE_COUNT];
extern const char* hcc_message_type_ascii_color_code[HCC_MESSAGE_TYPE_COUNT];
extern const char* hcc_error_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_ERROR_CODE_COUNT];
extern const char* hcc_warn_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_WARN_CODE_COUNT];

void hcc_message_print_file_line(HccIIO* iio, HccLocation* location);
void hcc_message_print_pasted_buffer(HccIIO* iio, uint32_t line, uint32_t column);
void hcc_message_print_code_line(HccIIO* iio, HccLocation* location, uint32_t display_line_num_size, uint32_t line, uint32_t display_line);
void hcc_message_print_code(HccIIO* iio, HccLocation* location);
void hcc_message_copy_deferred(HccTask* t, uint32_t start_idx, uint32_t count);

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
	HccStage            stage;
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
	HccCUSetup             cu_setup;
	HccCompiler*            c;
	HccTarget               target;
	HccOptions*             options;
	HccResult               result;
	HccPhase                phase;
	HccWorkerJobType        final_worker_job_type;
	HccTaskInputLocation*   input_locations;
	HccTaskOutputLocation   output_stage_locations[HCC_STAGE_COUNT];
	HccMessageSys           message_sys;
	HccStack(HccString)     include_path_strings;
	HccMutex                is_running_mutex;
	HccAtomic(uint32_t)     running_jobs_count;
	HccTime                 phase_start_times[HCC_PHASE_COUNT];
	HccDuration             worker_job_type_durations[HCC_WORKER_JOB_TYPE_COUNT];
	HccDuration             phase_durations[HCC_PHASE_COUNT];
	HccDuration             duration;
	HccTime                 start_time;

	HccCU*                  cu;
};

HccTaskInputLocation* hcc_task_input_location_init(HccTask* t, HccOptions* options);
HccResult hcc_task_add_output(HccTask* t, HccStage stage, HccEncoding encoding, void* arg);
void hcc_task_output_stage(HccTask* t, HccStage stage);
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
	HccWorkerJob   job;
	HccThread      thread;
	HccTime        job_start_time;
	uint8_t        initialized_generators_bitset;
	HccStack(char) string_buffer;
	HccArenaAlctor arena_alctor;

	HccATAGen      atagen;
#if 0
	HccASTGen      astgen;
	HccAMLGen      amlgen;
	HccAMLOpt      amlopt;
#endif
};

extern HccPhase hcc_worker_job_type_phases[HCC_WORKER_JOB_TYPE_COUNT];

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
	HccATAGenSetup                 atagen_setup;
	uint32_t                       workers_count;
	HccWorker*                     workers;
	void*                          worker_call_stacks_addr;
	uintptr_t                      worker_call_stacks_size;
	HccAtomic(uint32_t)            tasks_running_count;
	HccMutex                       wait_for_all_mutex;
	HccResultData                  result_data;
	HccDuration                    worker_job_type_durations[HCC_WORKER_JOB_TYPE_COUNT];
	HccDuration                    phase_durations[HCC_PHASE_COUNT];
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

#define HCC_STRING_ID_INTRINSIC_PARAM_NAMES_START HCC_STRING_ID_UINT8_T
	HCC_STRING_ID_UINT8_T,
	HCC_STRING_ID_UINT16_T,
	HCC_STRING_ID_UINT32_T,
	HCC_STRING_ID_UINT64_T,
	HCC_STRING_ID_UINTPTR_T,
	HCC_STRING_ID_INT8_T,
	HCC_STRING_ID_INT16_T,
	HCC_STRING_ID_INT32_T,
	HCC_STRING_ID_INT64_T,
	HCC_STRING_ID_INTPTR_T,
	HCC_STRING_ID_HCC_VERTEX_INPUT,
	HCC_STRING_ID_VERTEX_IDX,
	HCC_STRING_ID_INSTANCE_IDX,
	HCC_STRING_ID_HCC_FRAGMENT_INPUT,
	HCC_STRING_ID_FRAG_COORD,
	HCC_STRING_ID_HALF,
	HCC_STRING_ID__BITS,

	HCC_STRING_ID_SCALARX_START,
#define HCC_STRING_ID_SCALARX_END (HCC_STRING_ID_SCALARX_START + (HCC_AML_INTRINSIC_DATA_TYPE_COUNT))

	HCC_STRING_ID_PSCALARX_START = HCC_STRING_ID_SCALARX_END,
#define HCC_STRING_ID_PSCALARX_END (HCC_STRING_ID_PSCALARX_START + (HCC_AML_INTRINSIC_DATA_TYPE_COUNT))

#define HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END HCC_STRING_ID_KEYWORDS_START

	HCC_STRING_ID_KEYWORDS_START = HCC_STRING_ID_PSCALARX_END,
#define HCC_STRING_ID_KEYWORDS_END (HCC_STRING_ID_KEYWORDS_START + HCC_ATA_TOKEN_KEYWORDS_COUNT)

	HCC_STRING_ID_PREDEFINED_MACROS_START = HCC_STRING_ID_KEYWORDS_END,
#define HCC_STRING_ID_PREDEFINED_MACROS_END (HCC_STRING_ID_PREDEFINED_MACROS_START + HCC_PP_PREDEFINED_MACRO_COUNT)

	//HCC_STRING_ID_FUNCTION_IDXS_START = HCC_STRING_ID_PREDEFINED_MACROS_END,
//#define HCC_STRING_ID_FUNCTION_IDXS_END (HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_INTRINSIC_END)
	//HCC_STRING_ID_ONCE = HCC_STRING_ID_FUNCTION_IDXS_END,
	HCC_STRING_ID_ONCE = HCC_STRING_ID_PREDEFINED_MACROS_END,
	HCC_STRING_ID_DEFINED,
	HCC_STRING_ID___VA_ARGS__,
};

extern const char* hcc_string_intrinsic_param_names[HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END];

void hcc_string_table_init(HccStringTable* string_table, uint32_t data_grow_count, uint32_t data_reserve_cap, uint32_t entries_cap);
void hcc_string_table_deinit(HccStringTable* string_table);
HccStringId hcc_string_table_alloc_next_id(HccStringTable* string_table);

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

