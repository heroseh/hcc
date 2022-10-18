#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <setjmp.h>

typedef struct HccCompiler HccCompiler;
typedef struct HccCompilerSetup HccCompilerSetup;
typedef struct HccCodeFile HccCodeFile;
typedef struct HccPPIfSpan HccPPIfSpan;
typedef struct HccPPMacro HccPPMacro;

// ===========================================
//
//
// General
//
//
// ===========================================

typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef uint64_t  U64;
typedef uintptr_t Uptr;
typedef int8_t    S8;
typedef int16_t   S16;
typedef int32_t   S32;
typedef int64_t   S64;
typedef intptr_t  Sptr;
typedef float     F32;
typedef double    F64;

#define U8_MAX  UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX
#define S8_MIN  INT8_MIN
#define S8_MAX  INT8_MAX
#define S16_MIN INT16_MIN
#define S16_MAX INT16_MAX
#define S32_MIN INT32_MIN
#define S32_MAX INT32_MAX
#define S64_MIN INT64_MIN
#define S64_MAX INT64_MAX

typedef U16 HccAllocTag;
enum {
	HCC_ALLOC_TAG_NONE,

	HCC_ALLOC_TAG_MESSAGES,
	HCC_ALLOC_TAG_MESSAGE_STRINGS,
	HCC_ALLOC_TAG_STRING_TABLE_DATA,
	HCC_ALLOC_TAG_STRING_TABLE_ENTRIES,
	HCC_ALLOC_TAG_CONSTANT_TABLE_DATA,
	HCC_ALLOC_TAG_CONSTANT_TABLE_ENTRIES,
	HCC_ALLOC_TAG_STRING_BUFFER,
	HCC_ALLOC_TAG_INCLUDE_PATHS,
	HCC_ALLOC_TAG_CODE_FILES,
	HCC_ALLOC_TAG_PATH_TO_CODE_FILE_ID_MAP,
	HCC_ALLOC_TAG_CODE_FILE_LINE_CODE_START_INDICES,
	HCC_ALLOC_TAG_CODE_FILE_PP_IF_SPANS,
	HCC_ALLOC_TAG_CODE,

	HCC_ALLOC_TAG_PP_TOKENS,
	HCC_ALLOC_TAG_PP_TOKEN_LOCATION_INDICES,
	HCC_ALLOC_TAG_PP_TOKEN_VALUES,
	HCC_ALLOC_TAG_PP_MACROS,
	HCC_ALLOC_TAG_PP_MACRO_PARAMS,
	HCC_ALLOC_TAG_PP_MACRO_ARGS_STACK,
	HCC_ALLOC_TAG_PP_EXPAND_STACK,
	HCC_ALLOC_TAG_PP_STRINGIFY_BUFFER,
	HCC_ALLOC_TAG_PP_IF_SPAN_STACK,
	HCC_ALLOC_TAG_PP_MACRO_DECLARATIONS,

	HCC_ALLOC_TAG_TOKENGEN_TOKENS,
	HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATION_INDICES,
	HCC_ALLOC_TAG_TOKENGEN_TOKEN_VALUES,
	HCC_ALLOC_TAG_TOKENGEN_TOKEN_LOCATIONS,
	HCC_ALLOC_TAG_TOKENGEN_PAUSED_FILE_STACK,
	HCC_ALLOC_TAG_TOKENGEN_OPEN_BRACKET_STACK,

	HCC_ALLOC_TAG_ASTGEN_FUNCTION_PARAMS_AND_VARIABLES,
	HCC_ALLOC_TAG_ASTGEN_FUNCTIONS,
	HCC_ALLOC_TAG_ASTGEN_EXPRS,
	HCC_ALLOC_TAG_ASTGEN_EXPR_LOCATIONS,
	HCC_ALLOC_TAG_ASTGEN_GLOBAL_VARIABLES,
	HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_FUNCTION_INDICES,
	HCC_ALLOC_TAG_ASTGEN_FUNCTION_USED_STATIC_VARIABLES,
	HCC_ALLOC_TAG_ASTGEN_ENTRY_POINT_FUNCTION_INDICES,
	HCC_ALLOC_TAG_ASTGEN_USED_FUNCTION_INDICES,
	HCC_ALLOC_TAG_ASTGEN_RECURSIVE_FUNCTION_INDICES_STACK,
	HCC_ALLOC_TAG_ASTGEN_ARRAY_DATA_TYPES,
	HCC_ALLOC_TAG_ASTGEN_COMPOUND_DATA_TYPES,
	HCC_ALLOC_TAG_ASTGEN_COMPOUND_FIELDS,
	HCC_ALLOC_TAG_ASTGEN_TYPEDEFS,
	HCC_ALLOC_TAG_ASTGEN_ENUM_DATA_TYPES,
	HCC_ALLOC_TAG_ASTGEN_ENUM_VALUES,
	HCC_ALLOC_TAG_ASTGEN_ORDERED_DATA_TYPES,
	HCC_ALLOC_TAG_ASTGEN_GLOBAL_DECLARATIONS,
	HCC_ALLOC_TAG_ASTGEN_STRUCT_DECLARATIONS,
	HCC_ALLOC_TAG_ASTGEN_UNION_DECLARATIONS,
	HCC_ALLOC_TAG_ASTGEN_ENUM_DECLARATIONS,
	HCC_ALLOC_TAG_ASTGEN_COMPOUND_TYPE_FIND_FIELDS,
	HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_CURLYS,
	HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_ELMTS,
	HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZERS,
	HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_DESIGNATOR_INITIALIZER_ELMT_INDICES,
	HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_STRINGS,
	HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_VAR_INDICES,
	HCC_ALLOC_TAG_ASTGEN_FIELD_NAME_TO_TOKEN_IDX,

	HCC_ALLOC_TAG_IRGEN_FUNCTIONS,
	HCC_ALLOC_TAG_IRGEN_BASIC_BLOCKS,
	HCC_ALLOC_TAG_IRGEN_VALUES,
	HCC_ALLOC_TAG_IRGEN_WORDS,
	HCC_ALLOC_TAG_IRGEN_FUNCTION_CALL_PARAM_DATA_TYPES,

	HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_DATA_TYPES,
	HCC_ALLOC_TAG_SPIRVGEN_TYPE_TABLE_ENTRIES,
	HCC_ALLOC_TAG_SPIRVGEN_OUT_CAPABILITIES,
	HCC_ALLOC_TAG_SPIRVGEN_OUT_ENTRY_POINTS,
	HCC_ALLOC_TAG_SPIRVGEN_OUT_DEBUG_INFO,
	HCC_ALLOC_TAG_SPIRVGEN_OUT_ANNOTATIONS,
	HCC_ALLOC_TAG_SPIRVGEN_OUT_TYPES_VARIABLES_CONSTANTS,
	HCC_ALLOC_TAG_SPIRVGEN_OUT_FUNCTIONS,

	HCC_ALLOC_TAG_COUNT,
};

const char* hcc_alloc_tag_strings[HCC_ALLOC_TAG_COUNT];

#ifndef HCC_ALLOC
#define HCC_ALLOC(tag, size, align) malloc(size)
#endif

#ifndef HCC_DEALLOC
#define HCC_DEALLOC(tag, ptr, size, align) (void)tag, free(ptr)
#endif

#define HCC_ALLOC_ELMT(T, tag) HCC_ALLOC(tag, sizeof(T), alignof(T))
#define HCC_DEALLOC_ELMT(T, tag, ptr) HCC_DEALLOC(tag, ptr, sizeof(T), alignof(T))
#define HCC_ALLOC_ARRAY(T, tag, count) HCC_ALLOC(tag, (count) * sizeof(T), alignof(T))
#define HCC_DEALLOC_ARRAY(T, tag, ptr, count) HCC_DEALLOC(tag, ptr, (count) * sizeof(T), alignof(T))

#ifndef static_assert
#define static_assert _Static_assert
#endif

#ifndef noreturn
#define noreturn _Noreturn
#endif

#ifndef HCC_ABORT
#define HCC_ABORT(...) _hcc_abort(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef HCC_ASSERT
#define HCC_ASSERT(cond, ...) if (HCC_UNLIKELY(!(cond))) _hcc_assert_failed(#cond, __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef HCC_DEBUG_ASSERTIONS
#define HCC_DEBUG_ASSERTIONS 1
#endif

#define HCC_STRINGIFY(v) #v
#define HCC_CONCAT_0(a, b) a##b
#define HCC_CONCAT(a, b) HCC_CONCAT_0(a, b)

#if HCC_DEBUG_ASSERTIONS
#define HCC_DEBUG_ASSERT HCC_ASSERT
#else
#define HCC_DEBUG_ASSERT(cond, ...) (void)(cond)
#endif

#define HCC_DEBUG_ASSERT_NON_ZERO(value) HCC_DEBUG_ASSERT(value, "'%s' must be a non-zero value", #value)
#define HCC_DEBUG_ASSERT_ARRAY_BOUNDS(idx, count) (((idx) < (count)) ? (idx) : HCC_ABORT("idx '%zu' is out of bounds for an array of count '%zu'", (idx), (count)))
#define HCC_DEBUG_ASSERT_ARRAY_RESIZE(count, cap) HCC_DEBUG_ASSERT((count) <= (cap), "cannot resize array to count '%zu' when it has a capacity of '%zu'", (count), (cap))

#ifdef __GNUC__
#define HCC_LIKELY(expr) __builtin_expect((expr), 1)
#define HCC_UNLIKELY(expr) __builtin_expect((expr), 0)
#else
#define HCC_LIKELY(expr) expr
#define HCC_UNLIKELY(expr) expr
#endif

#if HCC_DEBUG_ASSERTIONS
#define HCC_UNREACHABLE(...) HCC_ABORT("unreachable code: " __VA_ARGS__);
#else
#define HCC_UNREACHABLE(...) __builtin_unreachable()
#endif

#define HCC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HCC_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define HCC_UNUSED(expr) ((void)(expr))

#ifndef alignof
#define alignof _Alignof
#endif

void _hcc_assert_failed(const char* cond, const char* file, int line, const char* message, ...);
noreturn Uptr _hcc_abort(const char* file, int line, const char* message, ...);

// align must be a power of 2
#define HCC_INT_ROUND_UP_ALIGN(i, align) (((i) + ((align) - 1)) & ~((align) - 1))
// align must be a power of 2
#define HCC_INT_ROUND_DOWN_ALIGN(i, align) ((i) & ~((align) - 1))

#define HCC_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
#define HCC_IS_POWER_OF_TWO_OR_ZERO(v) ((((v) & ((v) - 1)) == 0))
#define HCC_IS_POWER_OF_TWO(v) (((v) != 0) && (((v) & ((v) - 1)) == 0))
#define HCC_PTR_ADD(ptr, by) (void*)((Uptr)(ptr) + (Uptr)(by))
#define HCC_PTR_SUB(ptr, by) (void*)((Uptr)(ptr) - (Uptr)(by))
#define HCC_PTR_DIFF(to, from) ((char*)(to) - (char*)(from))
// align must be a power of 2
#define HCC_PTR_ROUND_UP_ALIGN(ptr, align) ((void*)HCC_INT_ROUND_UP_ALIGN((Uptr)(ptr), align))
// align must be a power of 2
#define HCC_PTR_ROUND_DOWN_ALIGN(ptr, align) ((void*)HCC_INT_ROUND_DOWN_ALIGN((Uptr)(ptr), align))
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

#define HCC_DIV_ROUND_UP(a, b) (((a) / (b)) + ((a) % (b) != 0))

#define HCC_LEAST_SET_BIT_IDX_U32(bitset) __builtin_ctz(bitset)
#define HCC_LEAST_SET_BIT_REMOVE(bitset) ((bitset) & ((bitset) - 1))

#define HCC_DEFINE_ID(Name) typedef struct Name { uint32_t idx_plus_one; } Name
HCC_DEFINE_ID(HccCodeFileId);
HCC_DEFINE_ID(HccStringId);
HCC_DEFINE_ID(HccConstantId);
HCC_DEFINE_ID(HccFunctionTypeId);
HCC_DEFINE_ID(HccExprId);

static inline bool hcc_bitset64_is_set(U64* bitset, U32 idx) {
	U64 bit = (U64)1 << (idx & 63);
	return (bitset[idx >> 6] & bit) == bit;
}

static inline void hcc_bitset64_set(U64* bitset, U32 idx) {
	U64 bit = (U64)1 << (idx & 63);
	bitset[idx >> 6] |= bit;
}

static inline void hcc_bitset64_unset(U64* bitset, U32 idx) {
	bitset[idx >> 6] &= ~((U64)1 << (idx & 63));
}

#define HCC_COMPOUND_TYPE_NESTED_FIELD_CAP 16

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

static inline bool hcc_ascii_is_alpha(U32 byte) {
	return ((byte | 32u) - 97u) < 26u;
}

static inline bool hcc_ascii_is_digit(U32 byte) {
	return (byte - 16) <= 10;
}

static inline bool hcc_ascii_is_a_to_f(U32 byte) {
	return ((byte | 32u) - 97u) < 6u;
}

// ===========================================
//
//
// Platform Abstraction
//
//
// ===========================================

typedef U8 HccArch;
enum {
	HCC_ARCH_X86_64,

	HCC_ARCH_COUNT,
};

typedef U8 HccOS;
enum {
	HCC_OS_LINUX,
	HCC_OS_WINDOWS,

	HCC_OS_COUNT,
};

typedef U8 HccGfxApi;
enum {
	HCC_GFX_API_OPENGL,
	HCC_GFX_API_VULKAN,
};

typedef U8 HccResourceModel;
enum {
	HCC_RESOURCE_MODEL_BINDING,
	HCC_RESOURCE_MODEL_BINDING_AND_BINDLESS,
};


U32 onebitscount32(U32 bits);
U32 leastsetbitidx32(U32 bits);
void hcc_get_last_system_error_string(char* buf_out, U32 buf_out_size);
bool hcc_file_exist(char* path);
bool hcc_change_working_directory(char* path);
bool hcc_change_working_directory_to_same_as_this_file(char* path);
char* hcc_path_canonicalize(char* path);
bool hcc_path_is_absolute(char* path);
char* hcc_file_read_all_the_codes(char* path, U64* size_out);

// ===========================================
//
//
// String
//
//
// ===========================================

typedef struct HccString HccString;
struct HccString {
	char* data;
	uintptr_t size;
};

#define hcc_string(data, size) ((HccString) { data, size })
#define hcc_string_lit(lit) ((HccString) { lit, sizeof(lit) - 1 })
#define hcc_string_c(string) ((HccString) { string, strlen(string) })
#define hcc_string_c_path(string) ((HccString) { string, strlen(string) + 1 })
#define hcc_string_eq(a, b) ((a).size == (b).size && memcmp((a).data, (b).data, (a).size) == 0)
#define hcc_string_eq_c(a, c_string) ((a).size == strlen(c_string) && memcmp((a).data, c_string, (a).size) == 0)
#define hcc_string_eq_lit(a, lit) ((a).size == sizeof(lit) - 1 && memcmp((a).data, lit, (a).size) == 0)
static inline HccString hcc_string_slice_start(HccString string, Uptr start) {
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(start, string.size + 1);
	return hcc_string(string.data + start, string.size - start);
}
static inline HccString hcc_string_slice_end(HccString string, Uptr end) {
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(end, string.size + 1);
	return hcc_string(string.data, end);
}
static inline HccString hcc_string_slice(HccString string, Uptr start, Uptr end) {
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(start, string.size + 1);
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(end, string.size + 1);
	HCC_DEBUG_ASSERT(start <= end, "start of '%zu' must be less than end of '%zu'", start, end);
	return hcc_string(string.data + start, end - start);
}
#define hcc_string_find_lit(string, lit) hcc_string_find(string, lit, sizeof(lit) - 1)
static inline U32 hcc_string_find(HccString haystack, char* needle, U32 needle_size) {
	if (needle_size == 0 || haystack.size < needle_size) {
		return -1;
	}

	for (U32 idx = 0; idx <= haystack.size - needle_size;) {
		if (haystack.data[idx] == needle[0]) {
			bool is_match = true;
			U32 cmp_idx = 0;
			for (; cmp_idx < needle_size; cmp_idx += 1) {
				if (haystack.data[idx + cmp_idx] != needle[cmp_idx]) {
					is_match = false;
					break;
				}
			}
			if (is_match) {
				return idx;
			}
			idx += cmp_idx;
		} else {
			idx += 1;
		}
	}

	return -1;
}

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
	Uptr count;
	Uptr cap;
	HccAllocTag tag;
#if HCC_DEBUG_ASSERTIONS
	U32  magic_number;
	Uptr elmt_size;
#endif
};

#define HccStack(T) T*

#define hcc_stack_header(stack)  ((stack) ? (((HccStackHeader*)(stack)) - 1)   : NULL)
#define hcc_stack_count(stack)   ((stack) ? hcc_stack_header(stack)->count     : 0)
#define hcc_stack_cap(stack)     ((stack) ? hcc_stack_header(stack)->cap       : 0)
#define hcc_stack_clear(stack)   ((stack)  ? hcc_stack_header(stack)->count = 0 : 0)
#define hcc_stack_is_full(stack) (hcc_stack_count(stack) == hcc_stack_cap(stack))

#define hcc_stack_init(T, cap, tag) ((HccStack(T))_hcc_stack_init(cap, tag, sizeof(T)))
HccStack(void) _hcc_stack_init(Uptr cap, HccAllocTag tag, Uptr elmt_size);

#define hcc_stack_deinit(stack, tag) _hcc_stack_deinit(stack, tag, sizeof(*(stack))); (stack) = NULL
void _hcc_stack_deinit(HccStack(void) stack, HccAllocTag tag, Uptr elmt_size);

#if HCC_DEBUG_ASSERTIONS
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
Uptr _hcc_stack_resize(HccStack(void) stack, Uptr new_count, Uptr elmt_size);

#define hcc_stack_insert(stack, idx) _hcc_stack_insert_many(stack, idx, 1, sizeof(*(stack)))
#define hcc_stack_insert_many(stack, idx, amount) _hcc_stack_insert_many(stack, idx, amount, sizeof(*(stack)))
void* _hcc_stack_insert_many(HccStack(void) stack, Uptr idx, Uptr amount, Uptr elmt_size);

#define hcc_stack_push(stack) (&(stack)[_hcc_stack_push_many(stack, 1, sizeof(*(stack)))])
#define hcc_stack_push_many(stack, amount) (&(stack)[_hcc_stack_push_many(stack, amount, sizeof(*(stack)))])
Uptr _hcc_stack_push_many(HccStack(void) stack, Uptr amount, Uptr elmt_size);

#define hcc_stack_pop(stack) _hcc_stack_pop_many(stack, 1, sizeof(*(stack)))
#define hcc_stack_pop_many(stack, amount) _hcc_stack_pop_many(stack, amount, sizeof(*(stack)))
void _hcc_stack_pop_many(HccStack(void) stack, Uptr amount, Uptr elmt_size);

#define hcc_stack_remove_swap(stack, idx) _hcc_stack_remove_swap_many(stack, idx, 1, sizeof(*(stack)))
#define hcc_stack_remove_swap_many(stack, idx, count) _hcc_stack_remove_swap_many(stack, idx, count, sizeof(*(stack)))
void _hcc_stack_remove_swap_many(HccStack(void) stack, Uptr idx, Uptr amount, Uptr elmt_size);

#define hcc_stack_remove_shift(stack, idx) _hcc_stack_remove_shift_many(stack, idx, 1, sizeof(*(stack)))
#define hcc_stack_remove_shift_many(stack, idx, count) _hcc_stack_remove_shift_many(stack, idx, count, sizeof(*(stack)))
void _hcc_stack_remove_shift_many(HccStack(void) stack, Uptr idx, Uptr amount, Uptr elmt_size);

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
// Hashing
//
//
// ===========================================

#define HCC_FNV_HASH_32_INITIAL 0x811c9dc5
U32 hcc_fnv_hash_32(char* bytes, U32 byte_count, U32 hash);
void hcc_generate_enum_hashes(char* array_name, char** strings, char** enum_strings, U32 enums_count);
void hcc_generate_hashes();
U32 hcc_string_to_enum_hashed_find(HccString string, U32* enum_hashes, U32 enums_count);

// ===========================================
//
//
// Hash Table
//
//
// ===========================================

#define HccHashTable(Key, Value) HccHashTable

typedef struct HccHashTable HccHashTable;
struct HccHashTable {
	U32* keys;
	U32* values;
	U32 count;
	U32 cap;
};

void hcc_hash_table_init(HccHashTable* hash_table, U32 cap, HccAllocTag tag);
bool hcc_hash_table_find(HccHashTable* hash_table, U32 key, U32* value_out);
bool hcc_hash_table_find_or_insert(HccHashTable* hash_table, U32 key, U32** value_ptr_out);
bool hcc_hash_table_remove(HccHashTable* hash_table, U32 key, U32* value_out);
void hcc_hash_table_clear(HccHashTable* hash_table);

// ===========================================
//
//
// Message: Error & Warn
//
//
// ===========================================

typedef U8 HccLang;
enum {
	HCC_LANG_ENG,

	HCC_LANG_COUNT,
};

typedef U32 HccMessageCode;
typedef U8 HccMessageType;
enum {
	HCC_MESSAGE_TYPE_ERROR = 0x1,
	HCC_MESSAGE_TYPE_WARN = 0x2,

	HCC_MESSAGE_TYPE_COUNT,
};

typedef HccMessageCode HccErrorCode;
enum {
	HCC_ERROR_CODE_NONE,

	//
	// tokengen
	HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER,
	HCC_ERROR_CODE_INVALID_TOKEN_MACRO_PARAM_IDENTIFIER,
	HCC_ERROR_CODE_DUPLICATE_MACRO_PARAM_IDENTIFIER,
	HCC_ERROR_CODE_INVALID_MACRO_PARAM_DELIMITER,
	HCC_ERROR_CODE_MACRO_PARAM_VA_ARG_NOT_LAST,
	HCC_ERROR_CODE_MACRO_ALREADY_DEFINED,
	HCC_ERROR_CODE_INVALID_INCLUDE_OPERAND,
	HCC_ERROR_CODE_TOO_MANY_INCLUDE_OPERANDS,
	HCC_ERROR_CODE_INCLUDE_PATH_IS_EMPTY,
	HCC_ERROR_CODE_INCLUDE_PATH_DOES_NOT_EXIST,
	HCC_ERROR_CODE_FAILED_TO_OPEN_FILE_FOR_READ,
	HCC_ERROR_CODE_CONDITION_HAS_NO_PP_TOKENS,
	HCC_ERROR_CODE_INVALID_PP_BINARY_OP,
	HCC_ERROR_CODE_INVALID_PP_UNARY_EXPR,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE,
	HCC_ERROR_CODE_UNDEFINED_IDENTIFIER_IN_PP_EXPR,
	HCC_ERROR_CODE_EXPECTED_COLON_FOR_TERNARY_OP,
	HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS,
	HCC_ERROR_CODE_PP_LINE_MUST_BE_MORE_THAN_ZERO,
	HCC_ERROR_CODE_TOO_MANY_PP_LINE_OPERANDS,
	HCC_ERROR_CODE_PP_ERROR,
	HCC_ERROR_CODE_INVALID_PP_PRAGMA_OPERAND,
	HCC_ERROR_CODE_PP_PRAGMA_OPERAND_USED_IN_MAIN_FILE,
	HCC_ERROR_CODE_TOO_MANY_PP_PRAGMA_OPERANDS,
	HCC_ERROR_CODE_INVALID_TOKEN_PREPROCESSOR_DIRECTIVE,
	HCC_ERROR_CODE_INVALID_PREPROCESSOR_DIRECTIVE,
	HCC_ERROR_CODE_PP_ENDIF_BEFORE_IF,
	HCC_ERROR_CODE_PP_ELSEIF_CANNOT_FOLLOW_ELSE,
	HCC_ERROR_CODE_PP_IF_UNTERMINATED,
	HCC_ERROR_CODE_INVALID_PP_CONCAT_OPERANDS,
	HCC_ERROR_CODE_TOO_MANY_UNDEF_OPERANDS,
	HCC_ERROR_CODE_TOO_MANY_IFDEF_OPERANDS,
	HCC_ERROR_CODE_PP_DIRECTIVE_NOT_FIRST_ON_LINE,
	HCC_ERROR_CODE_INVALID_OCTAL_DIGIT,
	HCC_ERROR_CODE_MAX_UINT_OVERFLOW,
	HCC_ERROR_CODE_MAX_SINT_OVERFLOW,
	HCC_ERROR_CODE_MAX_SINT_OVERFLOW_DECIMAL,
	HCC_ERROR_CODE_MAX_FLOAT_OVERFLOW,
	HCC_ERROR_CODE_U_SUFFIX_ALREADY_USED,
	HCC_ERROR_CODE_U_SUFFIX_ON_FLOAT,
	HCC_ERROR_CODE_L_SUFFIX_ON_FLOAT,
	HCC_ERROR_CODE_LONG_DOUBLE_IS_UNSUPPORTED,
	HCC_ERROR_CODE_FLOAT_HAS_DOUBLE_FULL_STOP,
	HCC_ERROR_CODE_FLOAT_MUST_BE_DECIMAL,
	HCC_ERROR_CODE_FLOAT_SUFFIX_MUST_FOLLOW_DECIMAL_PLACE,
	HCC_ERROR_CODE_INVALID_INTEGER_LITERALS,
	HCC_ERROR_CODE_INVALID_FLOAT_LITERALS,
	HCC_ERROR_CODE_NO_BRACKETS_OPEN,
	HCC_ERROR_CODE_INVALID_CLOSE_BRACKET_PAIR,
	HCC_ERROR_CODE_UNCLOSED_STRING_LITERAL,
	HCC_ERROR_CODE_MACRO_STARTS_WITH_CONCAT,
	HCC_ERROR_CODE_STRINGIFY_MUST_BE_MACRO_PARAM,
	HCC_ERROR_CODE_INVALID_TOKEN,
	HCC_ERROR_CODE_INVALID_TOKEN_HASH_IN_PP_OPERAND,
	HCC_ERROR_CODE_EXPECTED_IDENTIFIER_PP_IF_DEFINED,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_DEFINED,
	HCC_ERROR_CODE_INVALID_USE_OF_VA_ARGS,
	HCC_ERROR_CODE_VA_ARGS_IN_MACRO_PARAMETER,
	HCC_ERROR_CODE_NOT_ENOUGH_MACRO_ARGUMENTS,
	HCC_ERROR_CODE_TOO_MANY_MACRO_ARGUMENTS,

	//
	// astgen
	HCC_ERROR_CODE_CANNOT_FIND_FIELD,
	HCC_ERROR_CODE_DUPLICATE_FIELD_IDENTIFIER,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_CONDITION,
	HCC_ERROR_CODE_MISSING_SEMICOLON,
	HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_GLOBAL,
	HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ENUM,
	HCC_ERROR_CODE_REIMPLEMENTATION,
	HCC_ERROR_CODE_EMPTY_ENUM,
	HCC_ERROR_CODE_EXPECTED_IDENTIFIER_ENUM_VALUE,
	HCC_ERROR_CODE_ENUM_VALUE_OVERFLOW,
	HCC_ERROR_CODE_ENUM_VALUE_INVALID_FORMAT,
	HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR_WITH_EXPLICIT_VALUE,
	HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR,
	HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_FUNCTION,
	HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_STRUCT,
	HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_UNION,
	HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT,
	HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT,
	HCC_ERROR_CODE_NOT_AVAILABLE_FOR_UNION,
	HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_COMPOUND_TYPE,
	HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT_FIELD,
	HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT_FIELD,
	HCC_ERROR_CODE_COMPOUND_FIELD_INVALID_TERMINATOR,
	HCC_ERROR_CODE_COMPOUND_FIELD_MISSING_NAME,
	HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELDS_COUNT,
	HCC_ERROR_CODE_INTRINSIC_INVALID_COMPOUND_STRUCT_FIELD,
	HCC_ERROR_CODE_INTRINSIC_VECTOR_INVALID_SIZE_AND_ALIGN,
	HCC_ERROR_CODE_INTRINSIC_MATRIX_INVALID_SIZE_AND_ALIGN,
	HCC_ERROR_CODE_MISSING_RASTERIZER_STATE_SPECIFIER,
	HCC_ERROR_CODE_POSITION_ALREADY_SPECIFIED,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_ALIGNAS,
	HCC_ERROR_CODE_ALIGNAS_ON_SPECIAL_COMPOUND_DATA_TYPE,
	HCC_ERROR_CODE_INVALID_ALIGNAS_INT_CONSTANT,
	HCC_ERROR_CODE_INVALID_ALIGNAS_OPERAND,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_ALIGNAS,
	HCC_ERROR_CODE_ALIGNAS_REDUCES_ALIGNMENT,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_RESOURCE_SET_SLOT,
	HCC_ERROR_CODE_RESOURCE_SET_SLOT_OUT_OF_BOUNDS,
	HCC_ERROR_CODE_RESOURCE_SET_MUST_BE_A_UINT,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_RESOURCE_SET_SLOT,
	HCC_ERROR_CODE_CONST_BUFFER_IN_RESOURCES_USING_BINDLESS,
	HCC_ERROR_CODE_MATCHING_RESOURCE_SLOTS_IN_RESOURCES,
	HCC_ERROR_CODE_OVERFLOW_RESOURCE_CONSTANTS_SIZE,
	HCC_ERROR_CODE_POSITION_MUST_BE_VEC4_F32,
	HCC_ERROR_CODE_POSITION_NOT_SPECIFIED,
	HCC_ERROR_CODE_EXPECTED_TYPE_NAME,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_RESOURCE_TYPE_GENERIC,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_RESOURCE_TYPE_GENERIC,
	HCC_ERROR_CODE_EXPECTED_POD_DATA_TYPE_FOR_BUFFER,
	HCC_ERROR_CODE_INVALID_TEXEL_TYPE,
	HCC_ERROR_CODE_INVALID_BUFFER_ELEMENT_TYPE,
	HCC_ERROR_CODE_INVALID_RESOURCE_TABLE_RESOURCE_MODEL,
	HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE,
	HCC_ERROR_CODE_COMPLEX_ON_NON_FLOAT_TYPE,
	HCC_ERROR_CODE_MULTIPLE_TYPES_SPECIFIED,
	HCC_ERROR_CODE_COMPLEX_UNSUPPORTED_AT_THIS_TIME,
	HCC_ERROR_CODE_UNSIGNED_AND_SIGNED,
	HCC_ERROR_CODE_ATOMIC_UNSUPPORTED_AT_THIS_TIME,
	HCC_ERROR_CODE_DUPLICATE_TYPE_SPECIFIER,
	HCC_ERROR_CODE_EXPECTED_IDENTIFIER_TYPEDEF,
	HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_TYPEDEF,
	HCC_ERROR_CODE_INTRINSIC_NOT_FOUND_TYPEDEF,
	HCC_ERROR_CODE_INTRINSIC_INVALID_TYPEDEF,
	HCC_ERROR_CODE_TYPE_MISMATCH_IMPLICIT_CAST,
	HCC_ERROR_CODE_TYPE_MISMATCH,
	HCC_ERROR_CODE_UNSUPPORTED_BINARY_OPERATOR,
	HCC_ERROR_CODE_INVALID_CURLY_EXPR,
	HCC_ERROR_CODE_FIELD_DESIGNATOR_ON_ARRAY_TYPE,
	HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_DESIGNATOR,
	HCC_ERROR_CODE_ARRAY_DESIGNATOR_ON_COMPOUND_TYPE,
	HCC_ERROR_CODE_EXPECTED_INTEGER_FOR_ARRAY_IDX,
	HCC_ERROR_CODE_ARRAY_INDEX_OUT_OF_BOUNDS,
	HCC_ERROR_CODE_ARRAY_DESIGNATOR_EXPECTED_SQUARE_BRACE_CLOSE,
	HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_ARRAY_DESIGNATOR,
	HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_FIELD_DESIGNATOR,
	HCC_ERROR_CODE_EXPECTED_ASSIGN,
	HCC_ERROR_CODE_UNARY_OPERATOR_NOT_SUPPORTED,
	HCC_ERROR_CODE_UNDECLARED_IDENTIFIER,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_EXPR,
	HCC_ERROR_CODE_INVALID_CAST,
	HCC_ERROR_CODE_INVALID_CURLY_INITIALIZER_LIST_END,
	HCC_ERROR_CODE_SIZEALIGNOF_TYPE_OPERAND_NOT_WRAPPED,
	HCC_ERROR_CODE_EXPECTED_EXPR,
	HCC_ERROR_CODE_NOT_ENOUGH_FUNCTION_ARGS,
	HCC_ERROR_CODE_TOO_MANY_FUNCTION_ARGS,
	HCC_ERROR_CODE_INVALID_FUNCTION_ARG_DELIMITER,
	HCC_ERROR_CODE_ARRAY_SUBSCRIPT_EXPECTED_SQUARE_BRACE_CLOSE,
	HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_ACCESS,
	HCC_ERROR_CODE_MISSING_COLON_TERNARY_OP,
	HCC_ERROR_CODE_PARENTHISES_USED_ON_NON_FUNCTION,
	HCC_ERROR_CODE_SQUARE_BRACE_USED_ON_NON_ARRAY_DATA_TYPE,
	HCC_ERROR_CODE_FULL_STOP_USED_ON_NON_COMPOUND_DATA_TYPE,
	HCC_ERROR_CODE_CANNOT_ASSIGN_TO_CONST,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_CONDITION_EXPR,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_CONDITION_EXPR,
	HCC_ERROR_CODE_EXPECTED_ARRAY_SIZE,
	HCC_ERROR_CODE_EXPECTED_INTEGER_CONSTANT_ARRAY_SIZE,
	HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_NEGATIVE,
	HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_ZERO,
	HCC_ERROR_CODE_ARRAY_DECL_EXPECTED_SQUARE_BRACE_CLOSE,
	HCC_ERROR_CODE_UNSUPPORTED_SPECIFIER,
	HCC_ERROR_CODE_SPECIFIER_ALREADY_BEEN_USED,
	HCC_ERROR_CODE_UNUSED_SPECIFIER,
	HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED,
	HCC_ERROR_CODE_INVALID_SPECIFIER_VARIABLE_DECL,
	HCC_ERROR_CODE_INVALID_SPECIFIER_FUNCTION_DECL,
	HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_LOCAL,
	HCC_ERROR_CODE_STATIC_VARIABLE_INITIALIZER_MUST_BE_CONSTANT,
	HCC_ERROR_CODE_INVALID_VARIABLE_DECL_TERMINATOR,
	HCC_ERROR_CODE_INVALID_ELSE,
	HCC_ERROR_CODE_INVALID_SWITCH_CONDITION_TYPE,
	HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_SWITCH_STATEMENT,
	HCC_ERROR_CODE_EXPECTED_WHILE_CONDITION_FOR_DO_WHILE,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_FOR,
	HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FOR_VARIABLE_DECL,
	HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_FOR,
	HCC_ERROR_CODE_CASE_STATEMENT_OUTSIDE_OF_SWITCH,
	HCC_ERROR_CODE_SWITCH_CASE_VALUE_MUST_BE_A_CONSTANT,
	HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_CASE,
	HCC_ERROR_CODE_DEFAULT_STATMENT_OUTSIDE_OF_SWITCH,
	HCC_ERROR_CODE_DEFAULT_STATEMENT_ALREADY_DECLARED,
	HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_DEFAULT,
	HCC_ERROR_CODE_INVALID_BREAK_STATEMENT_USAGE,
	HCC_ERROR_CODE_INVALID_CONTINUE_STATEMENT_USAGE,
	HCC_ERROR_CODE_MULTIPLE_SHADER_STAGES_ON_FUNCTION,
	HCC_ERROR_CODE_VERTEX_SHADER_MUST_RETURN_RASTERIZER_STATE,
	HCC_ERROR_CODE_FRAGMENT_SHADER_MUST_RETURN_FRAGMENT_STATE,
	HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FUNCTION_PARAM,
	HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_FUNCTION_PARAM,
	HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_VERTEX_INVALID_COUNT,
	HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_VERTEX_INVALID_INPUT,
	HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_COUNT,
	HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_INPUT,
	HCC_ERROR_CODE_FUNCTION_SHADER_PROTOTYPE_FRAGMENT_INVALID_RASTERIZER_STATE,
	HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_RETURN_DATA_TYPE,
	HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_PARAMS_COUNT,
	HCC_ERROR_CODE_INTRINSIC_INVALID_FUNCTION_PARAM_DATA_TYPE,
	HCC_ERROR_CODE_FUNCTION_INVALID_TERMINATOR,
	HCC_ERROR_CODE_EXPECTED_SHADER_PARAM_TO_BE_CONST,
	HCC_ERROR_CODE_CANNOT_CALL_SHADER_FUNCTION,
	HCC_ERROR_CODE_CANNOT_CALL_UNIMPLEMENTED_FUNCTION,
	HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_MISMATCH_PARAM_DATA_TYPE,
	HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_TOO_MANY_PARAMETERS,
	HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_NOT_ENOUGH_PARAMETERS,
	HCC_ERROR_CODE_REDEFINITION_OF_FUNCTION_BODY_ALREADY_DECLARED,
	HCC_ERROR_CODE_FUNCTION_RECURSION,
	HCC_ERROR_CODE_UNEXPECTED_TOKEN_FUNCTION_PROTOTYPE_END,
	HCC_ERROR_CODE_UNEXPECTED_TOKEN,
	HCC_ERROR_CODE_RESOURCE_IN_UNION,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_RASTERIZER_STATE,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FRAGMENT_STATE,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_COMPOUND_DATA_TYPE,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCE_SET,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCE_TABLE,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCES_BINDING,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCES_BINDING_AND_BINDLESS,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_FUNCTION_PARAM,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_FUNCTION_PARAM_INLINE,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_VARIABLE,
	HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_POINTER_DATA_TYPE,
	HCC_ERROR_CODE_ONLY_SINGLE_POINTERS_ARE_SUPPORTED,
	HCC_ERROR_CODE_POINTER_TO_RESOURCE_SET_TABLE_MUST_BE_CONST,
	HCC_ERROR_CODE_LOGICAL_ADDRESSED_VAR_USED_BEFORE_ASSIGNED,
	HCC_ERROR_CODE_LOGICAL_ADDRESSED_CONDITIONALLY_ASSIGNED_BEFORE_USE,
	HCC_ERROR_CODE_NON_CONST_STATIC_VARIABLE_CANNOT_BE_LOGICALLY_ADDRESSED,

	//
	// spirv

	HCC_ERROR_CODE_COUNT,
};

typedef HccMessageCode HccWarnCode;
enum {
	HCC_WARN_CODE_NONE,

	//
	// tokengen
	HCC_WARN_CODE_PP_WARNING,

	//
	// astgen
	HCC_WARN_CODE_CURLY_INITIALIZER_ON_SCALAR,
	HCC_WARN_CODE_UNUSED_INITIALIZER_REACHED_END,
	HCC_WARN_CODE_NO_DESIGNATOR_AFTER_DESIGNATOR,

	//
	// spirv

	HCC_WARN_CODE_COUNT,
};

typedef struct HccLocation HccLocation;
struct HccLocation {
	HccCodeFile* code_file;
	HccLocation* parent_location;
	HccPPMacro*  macro;
	U32          code_start_idx;
	U32          code_end_idx;
	U32          line_start;
	U32          line_end;
	U32          column_start;
	U32          column_end;

	HccString    display_path;
	U32          display_line;
};

typedef struct HccMessage HccMessage;
struct HccMessage {
	HccString      string; // points to HccMessageSys.message_strings
	HccMessageType type;
	HccMessageCode code;
	HccLocation*   location;
	HccLocation*   other_location;
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

const char* hcc_message_type_lang_strings[HCC_LANG_COUNT][HCC_MESSAGE_TYPE_COUNT];
const char* hcc_message_type_ascii_color_code[HCC_MESSAGE_TYPE_COUNT];
const char* hcc_error_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_ERROR_CODE_COUNT];
const char* hcc_warn_code_lang_fmt_strings[HCC_LANG_COUNT][HCC_WARN_CODE_COUNT];

void hcc_location_merge_apply(HccLocation* before, HccLocation* after);

void hcc_message_print_file_line(HccCompiler* c, HccLocation* location);
void hcc_message_print_pasted_buffer(HccCompiler* c, U32 line, U32 column);
void hcc_message_print_code_line(HccCompiler* c, HccLocation* location, U32 display_line_num_size, U32 line, U32 display_line);
void hcc_message_print_code(HccCompiler* c, HccLocation* location);
void hcc_message_print(HccCompiler* c, HccMessage* message);

void hcc_message_pushv(HccCompiler* c, HccMessageType type, HccMessageCode code, HccLocation* location, HccLocation* other_location, va_list va_args);
void hcc_message_push(HccCompiler* c, HccMessageType type, HccMessageCode code, HccLocation* location, HccLocation* other_location, ...);
void hcc_error_pushv(HccCompiler* c, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, va_list va_args);
void hcc_error_push(HccCompiler* c, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...);
void hcc_warn_pushv(HccCompiler* c, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, va_list va_args);
void hcc_warn_push(HccCompiler* c, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, ...);

void hcc_message_copy_deferred(HccCompiler* c, U32 start_idx, U32 count);

// ===========================================
//
//
// Code File
//
//
// ===========================================

typedef U32 HccCodeFileFlags;
enum {
	HCC_CODE_FILE_FLAGS_COMPILATION_UNIT =      0x1,
	HCC_CODE_FILE_FLAGS_PRAGMA_ONCE =           0x2,
	HCC_CODE_FILE_FLAGS_PARSED_ONCE_OR_MORE =   0x4,
	HCC_CODE_FILE_FLAGS_IS_MACRO_PASTE_BUFFER = 0x8,
};

typedef struct HccCodeFile HccCodeFile;
struct HccCodeFile {
	HccCodeFileFlags      flags;
	HccStringId           path_string_id;
	HccString             path_string;
	HccString             code;
	HccStack(U32)         line_code_start_indices;
	HccStack(HccPPIfSpan) pp_if_spans;
	U32                   pp_if_span_id;
};

HccCodeFile* hcc_code_file_init(HccCompiler* c, HccStringId path_string_id);
bool hcc_code_file_find_or_insert(HccCompiler* c, HccString path_string, HccCodeFileId* code_file_id_out, HccCodeFile** code_file_out);
U32 hcc_code_file_line_size(HccCodeFile* code_file, U32 line);
U32 hcc_code_file_lines_count(HccCodeFile* code_file);

// ===========================================
//
//
// Declarations
//
//
// ===========================================

typedef U8 HccResourceType;
enum {
#define HCC_RESOURCE_TYPE_BUFFER_START HCC_RESOURCE_TYPE_CONSTBUFFER
#define HCC_RESOURCE_TYPE_SIMPLE_BUFFER_START HCC_RESOURCE_TYPE_CONSTBUFFER
	HCC_RESOURCE_TYPE_CONSTBUFFER,
	HCC_RESOURCE_TYPE_ROELEMENTBUFFER,
	HCC_RESOURCE_TYPE_RWELEMENTBUFFER,
	HCC_RESOURCE_TYPE_ROBYTEBUFFER,
	HCC_RESOURCE_TYPE_RWBYTEBUFFER,
#define HCC_RESOURCE_TYPE_SIMPLE_BUFFER_END (HCC_RESOURCE_TYPE_RWBYTEBUFFER + 1)

#define HCC_RESOURCE_TYPE_HAS_TEXEL_TYPE_START HCC_RESOURCE_TYPE_ROTEXELBUFFER
	HCC_RESOURCE_TYPE_ROTEXELBUFFER,
	HCC_RESOURCE_TYPE_RWTEXELBUFFER,
#define HCC_RESOURCE_TYPE_BUFFER_END (HCC_RESOURCE_TYPE_RWTEXELBUFFER + 1)

#define HCC_RESOURCE_TYPE_TEXTURE_START HCC_RESOURCE_TYPE_ROTEXTURE1D
	HCC_RESOURCE_TYPE_ROTEXTURE1D,
	HCC_RESOURCE_TYPE_RWTEXTURE1D,
	HCC_RESOURCE_TYPE_ROTEXTURE1DARRAY,
	HCC_RESOURCE_TYPE_RWTEXTURE1DARRAY,
	HCC_RESOURCE_TYPE_ROTEXTURE2D,
	HCC_RESOURCE_TYPE_RWTEXTURE2D,
	HCC_RESOURCE_TYPE_ROTEXTURE2DARRAY,
	HCC_RESOURCE_TYPE_RWTEXTURE2DARRAY,
	HCC_RESOURCE_TYPE_ROTEXTURE2DMS,
	HCC_RESOURCE_TYPE_RWTEXTURE2DMS,
	HCC_RESOURCE_TYPE_ROTEXTURE2DMSARRAY,
	HCC_RESOURCE_TYPE_RWTEXTURE2DMSARRAY,
	HCC_RESOURCE_TYPE_ROTEXTURE3D,
	HCC_RESOURCE_TYPE_RWTEXTURE3D,
#define HCC_RESOURCE_TYPE_TEXTURE_END (HCC_RESOURCE_TYPE_RWTEXTURE3D + 1)
#define HCC_RESOURCE_TYPE_HAS_TEXEL_TYPE_END HCC_RESOURCE_TYPE_TEXTURE_END
#define HCC_RESOURCE_TYPE_HAS_TEXEL_TYPE(type) (HCC_RESOURCE_TYPE_HAS_TEXEL_TYPE_START <= (type) && (type) < HCC_RESOURCE_TYPE_HAS_TEXEL_TYPE_END)

	HCC_RESOURCE_TYPE_SAMPLER_STATE,

	HCC_RESOURCE_TYPE_COUNT,
};
#define HCC_RESOURCE_TYPE_IS_SIMPLE_BUFFER(type) (HCC_RESOURCE_TYPE_SIMPLE_BUFFER_START <= (type) && (type) < HCC_RESOURCE_TYPE_SIMPLE_BUFFER_END)

typedef U32 HccDataType;
enum {
#define HCC_DATA_TYPE_BASIC_START HCC_DATA_TYPE_VOID
	HCC_DATA_TYPE_VOID,
	HCC_DATA_TYPE_BOOL,
	HCC_DATA_TYPE_CHAR,
	HCC_DATA_TYPE_SCHAR,
	HCC_DATA_TYPE_SSHORT,
	HCC_DATA_TYPE_SINT,
	HCC_DATA_TYPE_SLONG,
	HCC_DATA_TYPE_SLONGLONG,
	HCC_DATA_TYPE_UCHAR,
	HCC_DATA_TYPE_USHORT,
	HCC_DATA_TYPE_UINT,
	HCC_DATA_TYPE_ULONG,
	HCC_DATA_TYPE_ULONGLONG,
	HCC_DATA_TYPE_FLOAT,
	HCC_DATA_TYPE_DOUBLE,
#define HCC_DATA_TYPE_BASIC_END (HCC_DATA_TYPE_DOUBLE + 1)
#define HCC_DATA_TYPE_BASIC_COUNT (HCC_DATA_TYPE_BASIC_END - HCC_DATA_TYPE_BASIC_START)

	HCC_DATA_TYPE_ENUM,
	HCC_DATA_TYPE_STRUCT,
	HCC_DATA_TYPE_UNION,
	HCC_DATA_TYPE_ARRAY,
	HCC_DATA_TYPE_POINTER,
	HCC_DATA_TYPE_TYPEDEF,

	HCC_DATA_TYPE_GENERIC_FLOAT,

	HCC_DATA_TYPE_RESOURCE_START,
#define HCC_DATA_TYPE_RESOURCE_END (HCC_DATA_TYPE_RESOURCE_START + HCC_RESOURCE_TYPE_COUNT)
#define HCC_DATA_TYPE_RESOURCE(Name) (HCC_DATA_TYPE_RESOURCE_START + HCC_RESOURCE_TYPE_##Name)

	HCC_DATA_TYPE_COUNT = HCC_DATA_TYPE_RESOURCE_END,
#define HCC_DATA_TYPE_INVALID HCC_DATA_TYPE_COUNT
};

#define HCC_DATA_TYPE_IS_BASIC(type)              ((type) < HCC_DATA_TYPE_BASIC_END)
#define HCC_DATA_TYPE_IS_NON_VOID_BASIC(type)     ((type) > HCC_DATA_TYPE_VOID && (type) < HCC_DATA_TYPE_BASIC_END)
#define HCC_DATA_TYPE_IS_INT(type)                ((type) >= HCC_DATA_TYPE_CHAR && (type) <= HCC_DATA_TYPE_ULONGLONG)
#define HCC_DATA_TYPE_IS_SINT(c, type)            ( \
		((type) >= HCC_DATA_TYPE_SCHAR && (type) <= HCC_DATA_TYPE_SLONGLONG) || \
		((type) == HCC_DATA_TYPE_CHAR && !(c->flags & HCC_COMPILER_FLAGS_CHAR_IS_UNSIGNED)) \
	)

#define HCC_DATA_TYPE_IS_UINT(c, type)            ( \
		((type) >= HCC_DATA_TYPE_UCHAR && (type) <= HCC_DATA_TYPE_ULONGLONG) || \
		((type) == HCC_DATA_TYPE_CHAR && (c->flags & HCC_COMPILER_FLAGS_CHAR_IS_UNSIGNED)) \
	)

#define HCC_DATA_TYPE_IS_FLOAT(type)                     ((type) >= HCC_DATA_TYPE_FLOAT && (type) <= HCC_DATA_TYPE_DOUBLE)
#define HCC_DATA_TYPE_IS_STRUCT(type)                    (((type) & 0xff) == HCC_DATA_TYPE_STRUCT)
#define HCC_DATA_TYPE_IS_UNION(type)                     (((type) & 0xff) == HCC_DATA_TYPE_UNION)
#define HCC_DATA_TYPE_IS_COMPOUND_TYPE(type)             (HCC_DATA_TYPE_IS_STRUCT(type) || HCC_DATA_TYPE_IS_UNION(type))
#define HCC_DATA_TYPE_IS_ARRAY(type)                     (((type) & 0xff) == HCC_DATA_TYPE_ARRAY)
#define HCC_DATA_TYPE_IS_POINTER(type)                   (((type) & 0xff) == HCC_DATA_TYPE_POINTER)
#define HCC_DATA_TYPE_IS_COMPOSITE_TYPE(type)            (HCC_DATA_TYPE_IS_STRUCT(type) || HCC_DATA_TYPE_IS_UNION(type) || HCC_DATA_TYPE_IS_ARRAY(type))
#define HCC_DATA_TYPE_IS_PVECTOR(type)                   (HCC_DATA_TYPE_IS_STRUCT(type) && HCC_STRUCT_IDX_PVEC_START <= HCC_DATA_TYPE_IDX(type) && HCC_DATA_TYPE_IDX(type) < HCC_STRUCT_IDX_PVEC_END)
#define HCC_DATA_TYPE_IS_VECTOR(type)                    (HCC_DATA_TYPE_IS_UNION(type) && HCC_UNION_IDX_VEC_START <= HCC_DATA_TYPE_IDX(type) && HCC_DATA_TYPE_IDX(type) < HCC_UNION_IDX_VEC_END)
#define HCC_DATA_TYPE_IS_PMATRIX(type)                   (HCC_DATA_TYPE_IS_STRUCT(type) && HCC_STRUCT_IDX_PMAT_START <= HCC_DATA_TYPE_IDX(type) && HCC_DATA_TYPE_IDX(type) < HCC_STRUCT_IDX_PMAT_END)
#define HCC_DATA_TYPE_IS_MATRIX(type)                    (HCC_DATA_TYPE_IS_UNION(type) && HCC_UNION_IDX_MAT_START <= HCC_DATA_TYPE_IDX(type) && HCC_DATA_TYPE_IDX(type) < HCC_UNION_IDX_MAT_END)
#define HCC_DATA_TYPE_IS_INTRINSIC(type)                 (HCC_DATA_TYPE_IS_BASIC(type) || HCC_DATA_TYPE_IS_VECTOR(type) || HCC_DATA_TYPE_IS_MATRIX(type) || (type) == HCC_DATA_TYPE_HALF)
#define HCC_DATA_TYPE_IS_INTRINSIC_BASIC(type)           (HCC_DATA_TYPE_IS_BASIC(type) || (type) == HCC_DATA_TYPE_HALF)
#define HCC_DATA_TYPE_IS_TYPEDEF(type)                   (((type) & 0xff) == HCC_DATA_TYPE_TYPEDEF)
#define HCC_DATA_TYPE_IS_CONSTBUFFER(type)               (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(CONSTBUFFER))
#define HCC_DATA_TYPE_IS_ELEMENTBUFFER(type)             (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROELEMENTBUFFER) || ((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWELEMENTBUFFER))
#define HCC_DATA_TYPE_IS_BYTEBUFFER(type)                (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROBYTEBUFFER) || ((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWBYTEBUFFER))
#define HCC_DATA_TYPE_IS_SIMPLE_BUFFER(type)             (HCC_DATA_TYPE_RESOURCE(SIMPLE_BUFFER_START) <= ((type) & 0xff) && ((type) & 0xff) < HCC_DATA_TYPE_RESOURCE(SIMPLE_BUFFER_END))
#define HCC_DATA_TYPE_IS_BUFFER_RESOURCE(type)           (HCC_DATA_TYPE_RESOURCE(BUFFER_START) <= ((type) & 0xff) && ((type) & 0xff) < HCC_DATA_TYPE_RESOURCE(BUFFER_END))
#define HCC_DATA_TYPE_IS_TEXEL_BUFFER(type)              (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXELBUFFER) || ((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXELBUFFER))
#define HCC_DATA_TYPE_IS_ROTEXTURE1D(type)               (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXTURE1D))
#define HCC_DATA_TYPE_IS_RWTEXTURE1D(type)               (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXTURE1D))
#define HCC_DATA_TYPE_IS_ROTEXTURE1DARRAY(type)          (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXTURE1DARRAY))
#define HCC_DATA_TYPE_IS_RWTEXTURE1DARRAY(type)          (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXTURE1DARRAY))
#define HCC_DATA_TYPE_IS_ROTEXTURE2D(type)               (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXTURE2D))
#define HCC_DATA_TYPE_IS_RWTEXTURE2D(type)               (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXTURE2D))
#define HCC_DATA_TYPE_IS_ROTEXTURE2DARRAY(type)          (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXTURE2DARRAY))
#define HCC_DATA_TYPE_IS_RWTEXTURE2DARRAY(type)          (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXTURE2DARRAY))
#define HCC_DATA_TYPE_IS_ROTEXTURE2DMS(type)             (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXTURE2DMS))
#define HCC_DATA_TYPE_IS_RWTEXTURE2DMS(type)             (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXTURE2DMS))
#define HCC_DATA_TYPE_IS_ROTEXTURE2DMSARRAY(type)        (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXTURE2DMSARRAY))
#define HCC_DATA_TYPE_IS_RWTEXTURE2DMSARRAY(type)        (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXTURE2DMSARRAY))
#define HCC_DATA_TYPE_IS_ROTEXTURE3D(type)               (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(ROTEXTURE3D))
#define HCC_DATA_TYPE_IS_RWTEXTURE3D(type)               (((type) & 0xff) == HCC_DATA_TYPE_RESOURCE(RWTEXTURE3D))
#define HCC_DATA_TYPE_IS_TEXTURE_RESOURCE(type)          (HCC_DATA_TYPE_RESOURCE(TEXTURE_START) <= ((type) & 0xff) && ((type) & 0xff) < HCC_DATA_TYPE_RESOURCE(TEXTURE_END))
#define HCC_DATA_TYPE_IS_RESOURCE(type)                  (HCC_DATA_TYPE_RESOURCE_START <= ((type) & 0xff) && ((type) & 0xff) < HCC_DATA_TYPE_RESOURCE_END)
#define HCC_DATA_TYPE_IS_ENUM_TYPE(type)                 (((type) & 0xff) == HCC_DATA_TYPE_ENUM)
#define HCC_DATA_TYPE_INIT(type, idx)                    (((idx) << 8) | (type))
#define HCC_DATA_TYPE_IS_CONST(type)                     (!!((type) & HCC_DATA_TYPE_CONST_MASK))
#define HCC_DATA_TYPE_IS_VOLATILE(type)                  (!!((type) & HCC_DATA_TYPE_VOLATILE_MASK))
#define HCC_DATA_TYPE_IDX(type)                          (((type) & HCC_DATA_TYPE_IDX_MASK) >> 8)
#define HCC_DATA_TYPE_CONST(type)                        ((type) | HCC_DATA_TYPE_CONST_MASK)
#define HCC_DATA_TYPE_VOLATILE(type)                     ((type) | HCC_DATA_TYPE_VOLATILE_MASK)
#define HCC_DATA_TYPE_STRIP_CONST(type)                  ((type) & ~HCC_DATA_TYPE_CONST_MASK)
#define HCC_DATA_TYPE_STRIP_VOLATILE(type)               ((type) & ~HCC_DATA_TYPE_VOLATILE_MASK)
#define HCC_DATA_TYPE_STRIP_CONST_AND_VOLATILE(type)     ((type) & ~HCC_DATA_TYPE_CONST_AND_VOLATILE_MASK)
#define HCC_DATA_TYPE_ENUM(idx)                          HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_ENUM, idx)
#define HCC_DATA_TYPE_STRUCT(idx)                        HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_STRUCT, idx)
#define HCC_DATA_TYPE_UNION(idx)                         HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_UNION, idx)
#define HCC_DATA_TYPE_ARRAY(idx)                         HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_ARRAY, idx)
#define HCC_DATA_TYPE_POINTER(idx)                       HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_POINTER, idx)
#define HCC_DATA_TYPE_TYPEDEF(idx)                       HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_TYPEDEF, idx)
#define HCC_DATA_TYPE_PVECTOR(vec)                       (HCC_DATA_TYPE_STRUCT | ((HCC_STRUCT_IDX_PVEC_START + (vec)) << 8))
#define HCC_DATA_TYPE_VECTOR(vec)                        (HCC_DATA_TYPE_UNION | ((HCC_UNION_IDX_VEC_START + (vec)) << 8))
#define HCC_DATA_TYPE_PMATRIX(mat)                       (HCC_DATA_TYPE_STRUCT | ((HCC_STRUCT_IDX_PMAT_START + (mat)) << 8))
#define HCC_DATA_TYPE_MATRIX(mat)                        (HCC_DATA_TYPE_UNION | ((HCC_UNION_IDX_MAT_START + (mat)) << 8))
#define HCC_DATA_TYPE_TYPEDEF_PVECTOR(vec)               (HCC_DATA_TYPE_TYPEDEF | ((HCC_TYPEDEF_IDX_PVEC_START + (vec)) << 8))
#define HCC_DATA_TYPE_TYPEDEF_VECTOR(vec)                (HCC_DATA_TYPE_TYPEDEF | ((HCC_TYPEDEF_IDX_VEC_START + (vec)) << 8))
#define HCC_DATA_TYPE_TYPEDEF_PMATRIX(mat)               (HCC_DATA_TYPE_TYPEDEF | ((HCC_TYPEDEF_IDX_PMAT_START + (mat)) << 8))
#define HCC_DATA_TYPE_TYPEDEF_MATRIX(mat)                (HCC_DATA_TYPE_TYPEDEF | ((HCC_TYPEDEF_IDX_MAT_START + (mat)) << 8))
#define HCC_DATA_TYPE_PVECTOR_VEC(type)                  (HCC_DATA_TYPE_IDX(type) - HCC_STRUCT_IDX_PVEC_START)
#define HCC_DATA_TYPE_VECTOR_VEC(type)                   (HCC_DATA_TYPE_IDX(type) - HCC_UNION_IDX_VEC_START)
#define HCC_DATA_TYPE_PMATRIX_MAT(type)                  (HCC_DATA_TYPE_IDX(type) - HCC_STRUCT_IDX_PMAT_START)
#define HCC_DATA_TYPE_MATRIX_MAT(type)                   (HCC_DATA_TYPE_IDX(type) - HCC_UNION_IDX_MAT_START)
#define HCC_DATA_TYPE_CONSTBUFFER(idx)                   HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(CONSTBUFFER), idx)
#define HCC_DATA_TYPE_ROELEMENTBUFFER(idx)               HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROELEMENTBUFFER), idx)
#define HCC_DATA_TYPE_RWELEMENTBUFFER(idx)               HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWELEMENTBUFFER), idx)
#define HCC_DATA_TYPE_ROTEXELBUFFER(intrinsic_type)      HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXELBUFFER), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXELBUFFER(intrinsic_type)      HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXELBUFFER), intrinsic_type)
#define HCC_DATA_TYPE_ROTEXTURE1D(intrinsic_type)        HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXTURE1D), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXTURE1D(intrinsic_type)        HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXTURE1D), intrinsic_type)
#define HCC_DATA_TYPE_ROTEXTURE1DARRAY(intrinsic_type)   HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXTURE1DARRAY), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXTURE1DARRAY(intrinsic_type)   HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXTURE1DARRAY), intrinsic_type)
#define HCC_DATA_TYPE_ROTEXTURE2D(intrinsic_type)        HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXTURE2D), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXTURE2D(intrinsic_type)        HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXTURE2D), intrinsic_type)
#define HCC_DATA_TYPE_ROTEXTURE2DARRAY(intrinsic_type)   HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXTURE2DARRAY), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXTURE2DARRAY(intrinsic_type)   HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXTURE2DARRAY), intrinsic_type)
#define HCC_DATA_TYPE_ROTEXTURE2DMS(intrinsic_type)      HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXTURE2DMS), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXTURE2DMS(intrinsic_type)      HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXTURE2DMS), intrinsic_type)
#define HCC_DATA_TYPE_ROTEXTURE2DMSARRAY(intrinsic_type) HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXTURE2DMSARRAY), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXTURE2DMSARRAY(intrinsic_type) HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXTURE2DMSARRAY), intrinsic_type)
#define HCC_DATA_TYPE_ROTEXTURE3D(intrinsic_type)        HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(ROTEXTURE3D), intrinsic_type)
#define HCC_DATA_TYPE_RWTEXTURE3D(intrinsic_type)        HCC_DATA_TYPE_INIT(HCC_DATA_TYPE_RESOURCE(RWTEXTURE3D), intrinsic_type)
#define HCC_DATA_TYPE_UINT8                              HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_UINT8)
#define HCC_DATA_TYPE_UINT16                             HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_UINT16)
#define HCC_DATA_TYPE_UINT32                             HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_UINT32)
#define HCC_DATA_TYPE_UINT64                             HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_UINT64)
#define HCC_DATA_TYPE_UINTPTR                            HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_UINTPTR)
#define HCC_DATA_TYPE_INT8                               HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_INT8)
#define HCC_DATA_TYPE_INT16                              HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_INT16)
#define HCC_DATA_TYPE_INT32                              HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_INT32)
#define HCC_DATA_TYPE_INT64                              HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_INT64)
#define HCC_DATA_TYPE_INTPTR                             HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_INTPTR)
#define HCC_DATA_TYPE_HALF                               HCC_DATA_TYPE_STRUCT(HCC_STRUCT_IDX_HALF)
#define HCC_DATA_TYPE_TYPEDEF_HALF                       HCC_DATA_TYPE_TYPEDEF(HCC_TYPEDEF_IDX_HALF)
#define HCC_DATA_TYPE_CONST_MASK                         0x80000000
#define HCC_DATA_TYPE_VOLATILE_MASK                      0x40000000
#define HCC_DATA_TYPE_CONST_AND_VOLATILE_MASK            (HCC_DATA_TYPE_CONST_MASK | HCC_DATA_TYPE_VOLATILE_MASK)
#define HCC_DATA_TYPE_IDX_MASK                           0x3fffff00

typedef U8 HccBasicTypeClass;
enum {
	HCC_BASIC_TYPE_CLASS_VOID,
	HCC_BASIC_TYPE_CLASS_BOOL,
	HCC_BASIC_TYPE_CLASS_UINT,
	HCC_BASIC_TYPE_CLASS_SINT,
	HCC_BASIC_TYPE_CLASS_FLOAT,

	HCC_BASIC_TYPE_CLASS_COUNT,
};

typedef union HccBasic HccBasic;
union HccBasic {
	int8_t   s8;
	int16_t  s16;
	int32_t  s32;
	int64_t  s64;
	uint8_t  u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
	float    f;
	double   d;
};

typedef U8 HccVec;
enum {
	HCC_VEC2BOOL,
	HCC_VEC2I8,
	HCC_VEC2I16,
	HCC_VEC2I32,
	HCC_VEC2I64,
	HCC_VEC2U8,
	HCC_VEC2U16,
	HCC_VEC2U32,
	HCC_VEC2U64,
	HCC_VEC2F16,
	HCC_VEC2F32,
	HCC_VEC2F64,
	HCC_VEC3BOOL,
	HCC_VEC3I8,
	HCC_VEC3I16,
	HCC_VEC3I32,
	HCC_VEC3I64,
	HCC_VEC3U8,
	HCC_VEC3U16,
	HCC_VEC3U32,
	HCC_VEC3U64,
	HCC_VEC3F16,
	HCC_VEC3F32,
	HCC_VEC3F64,
	HCC_VEC4BOOL,
	HCC_VEC4I8,
	HCC_VEC4I16,
	HCC_VEC4I32,
	HCC_VEC4I64,
	HCC_VEC4U8,
	HCC_VEC4U16,
	HCC_VEC4U32,
	HCC_VEC4U64,
	HCC_VEC4F16,
	HCC_VEC4F32,
	HCC_VEC4F64,

	HCC_VEC_COUNT,
};

typedef U8 HccMat;
enum {
	HCC_MAT22F32,
	HCC_MAT22F64,
	HCC_MAT23F32,
	HCC_MAT23F64,
	HCC_MAT24F32,
	HCC_MAT24F64,
	HCC_MAT32F32,
	HCC_MAT32F64,
	HCC_MAT33F32,
	HCC_MAT33F64,
	HCC_MAT34F32,
	HCC_MAT34F64,
	HCC_MAT42F32,
	HCC_MAT42F64,
	HCC_MAT43F32,
	HCC_MAT43F64,
	HCC_MAT44F32,
	HCC_MAT44F64,

	HCC_MAT_COUNT,
};

typedef U8 HccIntrinsicType;
enum {
	HCC_INTRINSIC_TYPE_VOID,
	HCC_INTRINSIC_TYPE_BOOL,
	HCC_INTRINSIC_TYPE_S8,
	HCC_INTRINSIC_TYPE_S16,
	HCC_INTRINSIC_TYPE_S32,
	HCC_INTRINSIC_TYPE_S64,
	HCC_INTRINSIC_TYPE_U8,
	HCC_INTRINSIC_TYPE_U16,
	HCC_INTRINSIC_TYPE_U32,
	HCC_INTRINSIC_TYPE_U64,
	HCC_INTRINSIC_TYPE_F16,
	HCC_INTRINSIC_TYPE_F32,
	HCC_INTRINSIC_TYPE_F64,

	HCC_INTRINSIC_TYPE_VECTOR_START,
#define HCC_INTRINSIC_TYPE_VECTOR_END HCC_INTRINSIC_TYPE_MATRIX_START
	HCC_INTRINSIC_TYPE_MATRIX_START = HCC_INTRINSIC_TYPE_VECTOR_START + HCC_VEC_COUNT,
#define HCC_INTRINSIC_TYPE_MATRIX_END HCC_INTRINSIC_TYPE_COUNT

	HCC_INTRINSIC_TYPE_COUNT = HCC_INTRINSIC_TYPE_MATRIX_START + HCC_MAT_COUNT,
};
static_assert(HCC_VEC2BOOL == HCC_INTRINSIC_TYPE_BOOL - 1 && HCC_VEC2U64 == HCC_INTRINSIC_TYPE_U64 - 1, "hey you changed the order :(");

#define HCC_INTRINSIC_TYPE_FROM_VEC(vec) (HCC_INTRINSIC_TYPE_BOOL + (vec) % 12)
#define HCC_INTRINSIC_TYPE_FROM_MAT(mat) (HCC_INTRINSIC_TYPE_F32 + (mat) % 2)

typedef U16 HccIntrinsicBasicTypeMask;
#define HCC_INTRINSIC_BASIC_TYPE_MASK_SET(ptr, intrinsic_type) (*(ptr)) |= (1 << (intrinsic_type))
#define HCC_INTRINSIC_BASIC_TYPE_MASK_UNSET(ptr, intrinsic_type) (*(ptr)) &= ~(1 << (intrinsic_type))
#define HCC_INTRINSIC_BASIC_TYPE_MASK_IS_SET(v, intrinsic_type) ((bool)((v) & (1 << (intrinsic_type))))

typedef struct HccArrayDataType HccArrayDataType;
struct HccArrayDataType {
	HccDataType element_data_type;
	HccConstantId size_constant_id;
	U32 logically_addressable_elements_count: 30;
	U32 has_pointer: 1;
	U32 has_resource: 1;
};

//
// this is either:
// - HCC_DATA_TYPE_CONSTBUFFER
// - HCC_DATA_TYPE_ROELEMENTBUFFER
// - HCC_DATA_TYPE_RWELEMENTBUFFER
typedef struct HccSimpleBufferDataType HccSimpleBufferDataType;
struct HccSimpleBufferDataType {
	HccDataType element_data_type;
};

typedef struct HccPointerDataType HccPointerDataType;
struct HccPointerDataType {
	HccDataType element_data_type;
};

typedef U16 HccCompoundDataTypeFlags;
enum {
	HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION =            0x1,
	HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_RESOURCE =        0x2,
	HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_POINTER =         0x4,
};

typedef U8 HccCompoundDataTypeKind;
enum {
	HCC_COMPOUND_DATA_TYPE_KIND_DEFAULT,
	HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE,
	HCC_COMPOUND_DATA_TYPE_KIND_FRAGMENT_STATE,
	HCC_COMPOUND_DATA_TYPE_KIND_BUFFER_ELEMENT,
	HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_SET,
	HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_TABLE,
	HCC_COMPOUND_DATA_TYPE_KIND_RESOURCES,
};

typedef struct HccCompoundDataType HccCompoundDataType;
struct HccCompoundDataType {
	U64                       size;
	U64                       align;
	U32                       identifier_token_idx;
	HccStringId               identifier_string_id;
	U32                       fields_start_idx;
	U16                       fields_count;
	U16                       largest_sized_field_idx;
	HccIntrinsicBasicTypeMask has_intrinsic_basic_types;
	HccCompoundDataTypeFlags  flags;
	HccCompoundDataTypeKind   kind;
	U8                        resource_set_slot;
	U32                       logically_addressable_elements_count;
};

typedef U16 HccRasterizerStateFieldKind;
enum {
	HCC_RASTERIZER_STATE_FIELD_KIND_INTERP,
	HCC_RASTERIZER_STATE_FIELD_KIND_POSITION,
	HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP,
};

typedef struct HccCompoundField HccCompoundField;
struct HccCompoundField {
	HccStringId                 identifier_string_id;
	HccDataType                 data_type;
	U32                         identifier_token_idx;
	HccRasterizerStateFieldKind rasterizer_state_field_kind;
};

typedef struct HccEnumDataType HccEnumDataType;
struct HccEnumDataType {
	U32         identifier_token_idx;
	HccStringId identifier_string_id;
	U32         values_start_idx;
	U32         values_count;
};

typedef struct HccEnumValue HccEnumValue;
struct HccEnumValue {
	U32           identifier_token_idx;
	HccStringId   identifier_string_id;
	HccConstantId value_constant_id;
};

typedef struct HccTypedef HccTypedef;
struct HccTypedef {
	U32 identifier_token_idx;
	HccStringId identifier_string_id;
	HccDataType aliased_data_type;
};

//
// inherits HccDataType
typedef U32 HccDecl;
enum {
	HCC_DECL_FUNCTION = HCC_DATA_TYPE_COUNT,
	HCC_DECL_ENUM_VALUE,
	HCC_DECL_LOCAL_VARIABLE,
	HCC_DECL_GLOBAL_VARIABLE,
};
#define HCC_DECL_IS_DATA_TYPE(type) (((type) & 0xff) < HCC_DATA_TYPE_COUNT)
#define HCC_DECL_IS_FUNCTION(type) (((type) & 0xff) == HCC_DECL_FUNCTION)
#define HCC_DECL_IS_ENUM_VALUE(type) (((type) & 0xff) == HCC_DECL_ENUM_VALUE)
#define HCC_DECL_IS_LOCAL_VARIABLE(type) (((type) & 0xff) == HCC_DECL_LOCAL_VARIABLE)
#define HCC_DECL_IS_GLOBAL_VARIABLE(type) (((type) & 0xff) == HCC_DECL_GLOBAL_VARIABLE)
#define HCC_DECL_INIT(type, idx) HCC_DATA_TYPE_INIT(type, idx)
#define HCC_DECL_IDX(type) HCC_DATA_TYPE_IDX(type)

typedef struct HccVariable HccVariable;
struct HccVariable {
	HccStringId   identifier_string_id;
	HccDataType   data_type;
	HccConstantId initializer_constant_id; // if is_static
	U32           identifier_token_idx: 31;
	U32           is_static: 1;
	U32           logical_address_mutations_start_idx;
};

typedef U8 HccFunctionShaderStage;
enum {
	HCC_FUNCTION_SHADER_STAGE_NONE,
	HCC_FUNCTION_SHADER_STAGE_VERTEX,
	HCC_FUNCTION_SHADER_STAGE_FRAGMENT,
	HCC_FUNCTION_SHADER_STAGE_COMPUTE,
	HCC_FUNCTION_SHADER_STAGE_MESHTASK,

	HCC_FUNCTION_SHADER_STAGE_COUNT,
};

typedef U8 HccFunctionFlags;
enum {
	HCC_FUNCTION_FLAGS_STATIC = 0x1,
	HCC_FUNCTION_FLAGS_INLINE = 0x2,
};

#define HCC_FUNCTION_MAX_PARAMS_COUNT 32

typedef struct HccFunction HccFunction;
struct HccFunction {
	U32                    identifier_token_idx;
	HccStringId            identifier_string_id;
	HccDataType            return_data_type;
	U32                    return_data_type_token_idx;
	U32                    params_start_idx;
	U16                    variables_count;
	U8                     params_count;
	HccFunctionFlags       flags;
	HccFunctionShaderStage shader_stage;
	HccExprId              block_expr_id;
	U32                    used_function_indices_start_idx;
	U32                    used_function_indices_count;
	U32                    used_static_variables_start_idx;
	U32                    used_static_variables_count;
	U32                    unsupported_basic_types_deferred_messages_start_idx;
	U32                    unsupported_basic_types_deferred_messages_count;
	U32                    blocks_start_idx;
};

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

	HCC_TYPEDEF_IDX_PVEC_START,
#define HCC_TYPEDEF_IDX_PVEC_END HCC_TYPEDEF_IDX_VEC_START

	HCC_TYPEDEF_IDX_VEC_START = HCC_TYPEDEF_IDX_PVEC_START + HCC_VEC_COUNT,
#define HCC_TYPEDEF_IDX_VEC_END HCC_TYPEDEF_IDX_PMAT_START

	HCC_TYPEDEF_IDX_PMAT_START = HCC_TYPEDEF_IDX_VEC_START + HCC_VEC_COUNT,
#define HCC_TYPEDEF_IDX_PMAT_END HCC_TYPEDEF_IDX_MAT_START

	HCC_TYPEDEF_IDX_MAT_START = HCC_TYPEDEF_IDX_PMAT_START + HCC_MAT_COUNT,
#define HCC_TYPEDEF_IDX_MAT_END (HCC_TYPEDEF_IDX_MAT_START + HCC_MAT_COUNT)

	HCC_TYPEDEF_IDX_VERTEX_INPUT = HCC_TYPEDEF_IDX_MAT_END,
	HCC_TYPEDEF_IDX_FRAGMENT_INPUT,

#define HCC_TYPEDEF_IDX_INTRINSIC_END HCC_TYPEDEF_IDX_USER_START
	HCC_TYPEDEF_IDX_USER_START,
};

enum {
	HCC_STRUCT_IDX_VERTEX_INPUT,
	HCC_STRUCT_IDX_FRAGMENT_INPUT,
	HCC_STRUCT_IDX_HALF,

#define HCC_STRUCT_IDX_INTRINSIC_END HCC_STRUCT_IDX_PVEC_START

	HCC_STRUCT_IDX_PVEC_START,
#define HCC_STRUCT_IDX_PVEC_END HCC_UNION_IDX_VEC_START

	HCC_UNION_IDX_VEC_START = HCC_STRUCT_IDX_PVEC_START + HCC_VEC_COUNT,
#define HCC_UNION_IDX_VEC_END HCC_STRUCT_IDX_PMAT_START

	HCC_STRUCT_IDX_PMAT_START = HCC_UNION_IDX_VEC_START + HCC_VEC_COUNT,
#define HCC_STRUCT_IDX_PMAT_END HCC_UNION_IDX_MAT_START

	HCC_UNION_IDX_MAT_START = HCC_STRUCT_IDX_PMAT_START + HCC_MAT_COUNT,
#define HCC_UNION_IDX_MAT_END HCC_COMPOUND_DATA_TYPE_IDX_USER_START

	HCC_COMPOUND_DATA_TYPE_IDX_USER_START = HCC_UNION_IDX_MAT_START + HCC_MAT_COUNT,
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
	U32 fields_count;
	HccIntrinsicStructField fields[HCC_INTRINSIC_STRUCT_FIELDS_CAP];
};

typedef struct HccIntrinsicFunction HccIntrinsicFunction;
struct HccIntrinsicFunction {
	HccDataType return_data_type;
	U32         params_count;
	HccDataType param_data_types[6];
};

extern bool hcc_resource_type_has_generic_type[HCC_RESOURCE_TYPE_COUNT];
extern const char* hcc_resource_type_strings[HCC_RESOURCE_TYPE_COUNT];
extern U8 hcc_basic_type_size_and_aligns_x86_64_linux[HCC_DATA_TYPE_BASIC_COUNT];
extern U64 hcc_basic_type_int_mins_x86_64_linux[HCC_DATA_TYPE_BASIC_COUNT];
extern U64 hcc_basic_type_int_maxes_x86_64_linux[HCC_DATA_TYPE_BASIC_COUNT];

extern U8 hcc_packed_vec_sizes[HCC_VEC_COUNT];
extern U8 hcc_packed_vec_aligns[HCC_VEC_COUNT];
extern U8 hcc_vec_size_and_aligns[HCC_VEC_COUNT];
extern U8 hcc_packed_mat_sizes[HCC_MAT_COUNT];
extern U8 hcc_packed_mat_aligns[HCC_MAT_COUNT];
extern U8 hcc_mat_sizes[HCC_MAT_COUNT];
extern U8 hcc_mat_aligns[HCC_MAT_COUNT];
extern const char* hcc_intrinsic_function_strings[HCC_FUNCTION_IDX_INTRINSIC_END];
extern char* hcc_function_shader_stage_strings[HCC_FUNCTION_SHADER_STAGE_COUNT];
extern HccIntrinsicTypedef hcc_intrinsic_typedefs[HCC_TYPEDEF_IDX_INTRINSIC_END];
extern HccIntrinsicStruct hcc_intrinsic_structs[HCC_STRUCT_IDX_INTRINSIC_END];
extern HccIntrinsicFunction hcc_intrinsic_functions[HCC_FUNCTION_IDX_INTRINSIC_END];
extern U8 hcc_data_type_basic_type_ranks[HCC_DATA_TYPE_BASIC_COUNT];
extern const char* hcc_data_type_basic_type_strings[HCC_DATA_TYPE_BASIC_COUNT];

HccString hcc_data_type_string(HccCompiler* c, HccDataType data_type);
void hcc_data_type_size_align(HccCompiler* c, HccDataType data_type, U64* size_out, U64* align_out);
void hcc_data_type_print_basic(HccCompiler* c, HccDataType data_type, void* data, FILE* f);
HccDataType hcc_data_type_unsigned_to_signed(HccCompiler* c, HccDataType data_type);
HccDataType hcc_data_type_signed_to_unsigned(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_is_condition(HccCompiler* c, HccDataType data_type);
U32 hcc_data_type_composite_fields_count(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_is_rasterizer_state(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_is_fragment_state(HccCompiler* c, HccDataType data_type);
HccDataType hcc_data_type_from_intrinsic_type(HccCompiler* c, HccIntrinsicType intrinsic_type);
HccIntrinsicBasicTypeMask hcc_data_type_has_intrinsic_basic_types(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_is_same_underlaying_type(HccCompiler* c, HccDataType a, HccDataType b);
bool hcc_data_type_is_same_bitwidth_int(HccCompiler* c, HccDataType a, HccDataType b);
HccCompoundDataType* hcc_data_type_get_resource_table(HccCompiler* c, HccDataType data_type);
HccCompoundDataType* hcc_data_type_get_resource_set(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_is_resource_table_pointer(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_is_resource_set_pointer(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_is_resource_set_or_table_pointer(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_has_resources(HccCompiler* c, HccDataType data_type);
bool hcc_data_type_has_logically_address_elements(HccCompiler* c, HccDataType data_type);
U32 hcc_data_type_logically_address_elements_count(HccCompiler* c, HccDataType data_type);
void hcc_data_type_ensure_compound_type_default_kind(HccCompiler* c, HccDataType data_type, HccErrorCode error_code);
void hcc_data_type_ensure_valid_variable(HccCompiler* c, HccDataType data_type, HccErrorCode error_code);
void hcc_data_type_ensure_compound_type_has_no_resources(HccCompiler* c, HccDataType data_type, HccErrorCode error_code);
void hcc_data_type_ensure_has_no_resources(HccCompiler* c, HccDataType data_type, HccErrorCode error_code);
void hcc_data_type_ensure_has_no_pointers(HccCompiler* c, HccDataType data_type, HccErrorCode error_code);
HccDataType hcc_data_type_strip_pointer(HccCompiler* c, HccDataType data_type);
HccDataType hcc_data_type_strip_all_pointers(HccCompiler* c, HccDataType data_type);

U32 hcc_data_type_token_idx(HccCompiler* c, HccDataType data_type);

HccIntrinsicType hcc_intrinsic_type_from_data_type(HccCompiler* c, HccDataType data_type);
HccDataType hcc_vector_data_type_to_vector_bool(HccDataType data_type);
U32 hcc_vector_data_type_components_count(HccDataType data_type);
HccDataType hcc_vector_data_type_scalar(HccCompiler* c, HccDataType data_type);
HccDataType hcc_matrix_data_type_scalar(HccCompiler* c, HccDataType data_type);
HccDataType hcc_matrix_data_type_column_vector(HccDataType data_type);

HccString hcc_intrinsic_basic_type_mask_string(HccIntrinsicBasicTypeMask mask);

HccBasicTypeClass hcc_basic_type_class(HccCompiler* c, HccDataType data_type);
HccBasic hcc_basic_from_sint(HccCompiler* c, HccDataType data_type, S64 value);
HccBasic hcc_basic_from_uint(HccCompiler* c, HccDataType data_type, U64 value);
HccBasic hcc_basic_from_float(HccCompiler* c, HccDataType data_type, double value);

HccArrayDataType* hcc_array_data_type_get(HccCompiler* c, HccDataType data_type);
HccEnumDataType* hcc_enum_data_type_get(HccCompiler* c, HccDataType data_type);
HccCompoundDataType* hcc_compound_data_type_get(HccCompiler* c, HccDataType data_type);
HccSimpleBufferDataType* hcc_simple_buffer_data_type_get(HccCompiler* c, HccDataType data_type);
HccPointerDataType* hcc_pointer_data_type_get(HccCompiler* c, HccDataType data_type);
HccTypedef* hcc_typedef_get(HccCompiler* c, HccDataType data_type);
HccDataType hcc_typedef_resolve_and_keep_const_volatile(HccCompiler* c, HccDataType data_type);
HccDataType hcc_typedef_resolve_and_strip_const_volatile(HccCompiler* c, HccDataType data_type);

U32 hcc_decl_token_idx(HccCompiler* c, HccDecl decl);
HccFunction* hcc_function_get(HccCompiler* c, HccDecl decl);
HccEnumValue* hcc_enum_value_get(HccCompiler* c, HccDecl decl);
HccVariable* hcc_global_variable_get(HccCompiler* c, HccDecl decl);
U32 hcc_variable_to_string(HccCompiler* c, HccVariable* variable, char* buf, U32 buf_size, bool color);
U32 hcc_function_to_string(HccCompiler* c, HccFunction* function, char* buf, U32 buf_size, bool color);

// ===========================================
//
//
// Common
//
//
// ===========================================

typedef U8 HccBinaryOp;
enum {
	HCC_BINARY_OP_ASSIGN,
	HCC_BINARY_OP_ADD,
	HCC_BINARY_OP_SUBTRACT,
	HCC_BINARY_OP_MULTIPLY,
	HCC_BINARY_OP_DIVIDE,
	HCC_BINARY_OP_MODULO,
	HCC_BINARY_OP_BIT_AND,
	HCC_BINARY_OP_BIT_OR,
	HCC_BINARY_OP_BIT_XOR,
	HCC_BINARY_OP_BIT_SHIFT_LEFT,
	HCC_BINARY_OP_BIT_SHIFT_RIGHT,

	//
	// logical operators (return a bool)
	HCC_BINARY_OP_EQUAL,
	HCC_BINARY_OP_NOT_EQUAL,
	HCC_BINARY_OP_LESS_THAN,
	HCC_BINARY_OP_LESS_THAN_OR_EQUAL,
	HCC_BINARY_OP_GREATER_THAN,
	HCC_BINARY_OP_GREATER_THAN_OR_EQUAL,

	HCC_BINARY_OP_LOGICAL_AND,
	HCC_BINARY_OP_LOGICAL_OR,

	HCC_BINARY_OP_TERNARY,
	HCC_BINARY_OP_COMMA,

	HCC_BINARY_OP_COUNT,
};

// ===========================================
//
//
// Token
//
//
// ===========================================

#define HCC_TOKEN_IDX_INVALID U32_MAX

typedef U8 HccToken;
enum {
	HCC_TOKEN_EOF,
	HCC_TOKEN_IDENT,
	HCC_TOKEN_STRING,
	HCC_TOKEN_INCLUDE_PATH_SYSTEM,
	HCC_TOKEN_BACK_SLASH,
	HCC_TOKEN_HASH,
	HCC_TOKEN_DOUBLE_HASH,
	HCC_TOKEN_MACRO_WHITESPACE,
	HCC_TOKEN_MACRO_PARAM,
	HCC_TOKEN_MACRO_STRINGIFY,
	HCC_TOKEN_MACRO_STRINGIFY_WHITESPACE,
	HCC_TOKEN_MACRO_CONCAT,
	HCC_TOKEN_MACRO_CONCAT_WHITESPACE,

	//
	// symbols
	//
#define HCC_TOKEN_BRACKET_START HCC_TOKEN_CURLY_OPEN
#define HCC_TOKEN_BRACKET_END (HCC_TOKEN_SQUARE_CLOSE + 1)
#define HCC_TOKEN_BRACKET_COUNT (HCC_TOKEN_BRACKET_END - HCC_TOKEN_BRACKET_START)
	HCC_TOKEN_CURLY_OPEN,
	HCC_TOKEN_CURLY_CLOSE,
	HCC_TOKEN_PARENTHESIS_OPEN,
	HCC_TOKEN_PARENTHESIS_CLOSE,
	HCC_TOKEN_SQUARE_OPEN,
	HCC_TOKEN_SQUARE_CLOSE,
	HCC_TOKEN_FULL_STOP,
	HCC_TOKEN_COMMA,
	HCC_TOKEN_SEMICOLON,
	HCC_TOKEN_COLON,
	HCC_TOKEN_PLUS,
	HCC_TOKEN_MINUS,
	HCC_TOKEN_FORWARD_SLASH,
	HCC_TOKEN_ASTERISK,
	HCC_TOKEN_PERCENT,
	HCC_TOKEN_AMPERSAND,
	HCC_TOKEN_PIPE,
	HCC_TOKEN_CARET,
	HCC_TOKEN_EXCLAMATION_MARK,
	HCC_TOKEN_QUESTION_MARK,
	HCC_TOKEN_TILDE,
	HCC_TOKEN_EQUAL,
	HCC_TOKEN_LESS_THAN,
	HCC_TOKEN_GREATER_THAN,

	//
	// grouped symbols
	//
	HCC_TOKEN_LOGICAL_AND,
	HCC_TOKEN_LOGICAL_OR,
	HCC_TOKEN_LOGICAL_EQUAL,
	HCC_TOKEN_LOGICAL_NOT_EQUAL,
	HCC_TOKEN_LESS_THAN_OR_EQUAL,
	HCC_TOKEN_GREATER_THAN_OR_EQUAL,
	HCC_TOKEN_BIT_SHIFT_LEFT,
	HCC_TOKEN_BIT_SHIFT_RIGHT,
	HCC_TOKEN_ADD_ASSIGN,
	HCC_TOKEN_SUBTRACT_ASSIGN,
	HCC_TOKEN_MULTIPLY_ASSIGN,
	HCC_TOKEN_DIVIDE_ASSIGN,
	HCC_TOKEN_MODULO_ASSIGN,
	HCC_TOKEN_BIT_SHIFT_LEFT_ASSIGN,
	HCC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN,
	HCC_TOKEN_BIT_AND_ASSIGN,
	HCC_TOKEN_BIT_XOR_ASSIGN,
	HCC_TOKEN_BIT_OR_ASSIGN,
	HCC_TOKEN_INCREMENT,
	HCC_TOKEN_DECREMENT,

#define HCC_TOKEN_LIT_NUMBERS_START HCC_TOKEN_LIT_UINT
	HCC_TOKEN_LIT_UINT,
	HCC_TOKEN_LIT_ULONG,
	HCC_TOKEN_LIT_ULONGLONG,
	HCC_TOKEN_LIT_SINT,
	HCC_TOKEN_LIT_SLONG,
	HCC_TOKEN_LIT_SLONGLONG,
	HCC_TOKEN_LIT_FLOAT,
	HCC_TOKEN_LIT_DOUBLE,
#define HCC_TOKEN_LIT_NUMBERS_END (HCC_TOKEN_LIT_DOUBLE + 1)

	//
	// keywords
	//
#define HCC_TOKEN_KEYWORDS_START HCC_TOKEN_KEYWORD_VOID
	HCC_TOKEN_KEYWORD_VOID,
	HCC_TOKEN_KEYWORD_BOOL,
	HCC_TOKEN_KEYWORD_CHAR,
	HCC_TOKEN_KEYWORD_SHORT,
	HCC_TOKEN_KEYWORD_INT,
	HCC_TOKEN_KEYWORD_LONG,
	HCC_TOKEN_KEYWORD_FLOAT,
	HCC_TOKEN_KEYWORD_DOUBLE,
	HCC_TOKEN_KEYWORD_GENERIC_FLOAT,
	HCC_TOKEN_KEYWORD_UNSIGNED,
	HCC_TOKEN_KEYWORD_SIGNED,
	HCC_TOKEN_KEYWORD_COMPLEX,
	HCC_TOKEN_KEYWORD_ATOMIC,
	HCC_TOKEN_KEYWORD_RETURN,
	HCC_TOKEN_KEYWORD_IF,
	HCC_TOKEN_KEYWORD_ELSE,
	HCC_TOKEN_KEYWORD_DO,
	HCC_TOKEN_KEYWORD_WHILE,
	HCC_TOKEN_KEYWORD_FOR,
	HCC_TOKEN_KEYWORD_SWITCH,
	HCC_TOKEN_KEYWORD_CASE,
	HCC_TOKEN_KEYWORD_DEFAULT,
	HCC_TOKEN_KEYWORD_BREAK,
	HCC_TOKEN_KEYWORD_CONTINUE,
	HCC_TOKEN_KEYWORD_TRUE,
	HCC_TOKEN_KEYWORD_FALSE,
	HCC_TOKEN_KEYWORD_VERTEX,
	HCC_TOKEN_KEYWORD_FRAGMENT,
	HCC_TOKEN_KEYWORD_GEOMETRY,
	HCC_TOKEN_KEYWORD_TESSELLATION,
	HCC_TOKEN_KEYWORD_COMPUTE,
	HCC_TOKEN_KEYWORD_MESHTASK,
	HCC_TOKEN_KEYWORD_ENUM,
	HCC_TOKEN_KEYWORD_STRUCT,
	HCC_TOKEN_KEYWORD_UNION,
	HCC_TOKEN_KEYWORD_TYPEDEF,
	HCC_TOKEN_KEYWORD_STATIC,
	HCC_TOKEN_KEYWORD_CONST,
	HCC_TOKEN_KEYWORD_AUTO,
	HCC_TOKEN_KEYWORD_REGISTER,
	HCC_TOKEN_KEYWORD_VOLATILE,
	HCC_TOKEN_KEYWORD_EXTERN,
	HCC_TOKEN_KEYWORD_INLINE,
	HCC_TOKEN_KEYWORD_NO_RETURN,
	HCC_TOKEN_KEYWORD_SIZEOF,
	HCC_TOKEN_KEYWORD_ALIGNOF,
	HCC_TOKEN_KEYWORD_ALIGNAS,
	HCC_TOKEN_KEYWORD_STATIC_ASSERT,
	HCC_TOKEN_KEYWORD_RESTRICT,
	HCC_TOKEN_KEYWORD_INTRINSIC,
	HCC_TOKEN_KEYWORD_RASTERIZER_STATE,
	HCC_TOKEN_KEYWORD_FRAGMENT_STATE,
	HCC_TOKEN_KEYWORD_BUFFER_ELEMENT,
	HCC_TOKEN_KEYWORD_RESOURCE_SET,
	HCC_TOKEN_KEYWORD_RESOURCE_TABLE,
	HCC_TOKEN_KEYWORD_RESOURCES,
	HCC_TOKEN_KEYWORD_POSITION,
	HCC_TOKEN_KEYWORD_NOINTERP,
	HCC_TOKEN_KEYWORD_RESOURCE_START,
#define HCC_TOKEN_KEYWORD_RESOURCE_END (HCC_TOKEN_KEYWORD_RESOURCE_START + HCC_RESOURCE_TYPE_COUNT)

#define HCC_TOKEN_KEYWORDS_END HCC_TOKEN_COUNT
#define HCC_TOKEN_KEYWORDS_COUNT (HCC_TOKEN_KEYWORDS_END - HCC_TOKEN_KEYWORDS_START)
#define HCC_TOKEN_IS_KEYWORD(token) (HCC_TOKEN_KEYWORDS_START <= (token) && (token) < HCC_TOKEN_KEYWORDS_END)
#define HCC_TOKEN_IS_LIT_NUMBER(token) (HCC_TOKEN_LIT_NUMBERS_START <= (token) && (token) < HCC_TOKEN_LIT_NUMBERS_END)

	HCC_TOKEN_COUNT = HCC_TOKEN_KEYWORD_RESOURCE_END,
};

typedef union HccTokenValue HccTokenValue;
union HccTokenValue {
	HccConstantId constant_id;
	HccStringId   string_id;
	U32           macro_param_idx;
};

static_assert(sizeof(HccTokenValue) == sizeof(U32), "HccTokenValue has been designed around being 32 bits!");

typedef struct HccTokenBag HccTokenBag;
struct HccTokenBag {
	HccStack(HccToken)      tokens;
	HccStack(U32)           token_location_indices;
	HccStack(HccTokenValue) token_values;
};

typedef struct HccTokenCursor HccTokenCursor;
struct HccTokenCursor {
	U32         tokens_start_idx;
	U32         tokens_end_idx;
	U32         token_idx;
	U32         token_value_idx;
};

extern char* hcc_token_strings[HCC_TOKEN_COUNT];

U32 hcc_token_num_values(HccToken token);
bool hcc_token_concat_is_okay(HccToken before, HccToken after);

U32 hcc_token_cursor_tokens_count(HccTokenCursor* cursor);
HccLocation* hcc_token_bag_location_get(HccCompiler* c, HccTokenBag* bag, U32 token_idx);
void hcc_token_bag_stringify_single(HccCompiler* c, HccTokenBag* bag, HccTokenCursor* cursor, HccPPMacro* macro);
HccToken hcc_token_bag_stringify_single_or_macro_param(HccCompiler* c, HccTokenBag* bag, HccTokenCursor* cursor, U32 args_start_idx, HccTokenBag* args_src_bag, bool false_before_true_after);
HccStringId hcc_token_bag_stringify_range(HccCompiler* c, HccTokenBag* bag, HccTokenCursor* cursor, HccPPMacro* macro);

// ===========================================
//
//
// Preprocessor
//
//
// ===========================================

#define HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG_MASK 0x80000000

typedef struct HccPPMacro HccPPMacro;
struct HccPPMacro {
	HccStringId identifier_string_id;
	HccString   identifier_string;
	HccLocation location;
	HccTokenCursor token_cursor;
	U32 params_start_idx: 23;
	U32 params_count: 8;
	U32 is_function: 1;
	U32 has_va_args: 1;
};

typedef U32 HccPPPredefinedMacro;
enum {
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

typedef U8 HccPPDirective;
enum {
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
	U32            first_id: 31; // a link to the span that is the original #if/n/def
	U32            has_else: 1;
	U32            prev_id;
	U32            next_id;
	U32            last_id; // set when this is the original #if/n/def to link to the matching #endif
};

typedef struct HccPPMacroArg HccPPMacroArg;
struct HccPPMacroArg {
	HccTokenCursor cursor;
	HccLocation* callsite_location;
};

typedef struct HccPPExpand HccPPExpand;
struct HccPPExpand {
	HccPPMacro*    macro;
	HccTokenCursor cursor;
};

typedef struct HccPPEval HccPPEval;
struct HccPPEval {
	union {
		U64 u64;
		S64 s64;
	};
	bool is_signed;
};

typedef struct HccPP HccPP;
struct HccPP {
	HccTokenBag                    macro_token_bag;
	HccStack(HccPPMacro)           macros;
	HccStack(HccStringId)          macro_params;
	HccStack(HccPPMacroArg)        macro_args_stack;
	HccStack(HccPPExpand)          expand_stack;
	HccStack(U32)                  expand_macro_idx_stack;
	HccStack(char)                 stringify_buffer;
	HccStack(HccPPIfSpan*)         if_span_stack;
	HccHashTable(HccStringId, U32) macro_declarations;
};

typedef struct HccPPSetup HccPPSetup;
struct HccPPSetup {
	U32 macro_tokens_cap;
	U32 macro_token_values_cap;
	U32 macros_cap;
	U32 macro_params_cap;
	U32 macro_args_stack_cap;
	U32 expand_stack_cap;
	U32 stringify_buffer_cap;
	U32 if_stack_cap;
};

typedef U8 HccPPExpandFlags;
enum {
	HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION = 0x1,
	HCC_PP_EXPAND_FLAGS_IS_ARGS                   = 0x2,
	HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS              = 0x4,
};

char* hcc_pp_predefined_macro_identifier_strings[HCC_PP_PREDEFINED_MACRO_COUNT];
char* hcc_pp_directive_enum_strings[HCC_PP_DIRECTIVE_COUNT];
char* hcc_pp_directive_strings[HCC_PP_DIRECTIVE_COUNT];
U32 hcc_pp_directive_hashes[HCC_PP_DIRECTIVE_COUNT];

void hcc_pp_init(HccCompiler* c, HccCompilerSetup* setup);
HccPPMacro* hcc_pp_macro_get(HccCompiler* c, U32 macro_idx);
HccPPIfSpan* hcc_pp_if_span_get(HccCompiler* c, U32 if_span_id);
U32 hcc_pp_if_span_id(HccCompiler* c, HccPPIfSpan* if_span);
HccPPIfSpan* hcc_pp_if_span_push(HccCompiler* c, HccPPDirective directive);
void hcc_pp_if_found_if(HccCompiler* c, HccPPDirective directive);
HccPPIfSpan* hcc_pp_if_found_if_counterpart(HccCompiler* c, HccPPDirective directive);
void hcc_pp_if_found_endif(HccCompiler* c);
void hcc_pp_if_found_else(HccCompiler* c, HccPPDirective directive);
void hcc_pp_if_ensure_first_else(HccCompiler* c, HccPPDirective directive);
void hcc_pp_if_ensure_one_is_open(HccCompiler* c, HccPPDirective directive);

void hcc_pp_eval_binary_op(HccCompiler* c, U32* token_idx_mut, HccBinaryOp* binary_op_type_out, U32* precedence_out);
HccPPEval hcc_pp_eval_unary_expr(HccCompiler* c, U32* token_idx_mut, U32* token_value_idx_mut);
HccPPEval hcc_pp_eval_expr(HccCompiler* c, U32 min_precedence, U32* token_idx_mut, U32* token_value_idx_mut);
void hcc_pp_ensure_end_of_directive(HccCompiler* c, HccErrorCode error_code, HccPPDirective directive);

void hcc_pp_parse_define(HccCompiler* c);
void hcc_pp_parse_undef(HccCompiler* c);
void hcc_pp_parse_include(HccCompiler* c);
bool hcc_pp_parse_if(HccCompiler* c);
void hcc_pp_parse_defined(HccCompiler* c);
bool hcc_pp_parse_ifdef(HccCompiler* c, HccPPDirective directive);
void hcc_pp_parse_line(HccCompiler* c);
void hcc_pp_parse_error(HccCompiler* c);
void hcc_pp_parse_pragma(HccCompiler* c);
HccPPDirective hcc_pp_parse_directive_header(HccCompiler* c);
void hcc_pp_parse_directive(HccCompiler* c);
void hcc_pp_skip_false_conditional(HccCompiler* c, bool is_skipping_until_endif);
void hcc_pp_copy_expand_predefined_macro(HccCompiler* c, HccPPPredefinedMacro predefined_macro);
void hcc_pp_copy_expand_macro_begin(HccCompiler* c, HccPPMacro* macro, HccLocation* macro_callsite_location);
bool hcc_pp_is_callable_macro(HccCompiler* c, HccStringId ident_string_id, U32* macro_idx_out);
HccPPExpand* hcc_pp_expand_push_macro(HccCompiler* c, HccPPMacro* macro);
HccPPExpand* hcc_pp_expand_push_macro_arg(HccCompiler* c, U32 param_idx, U32 args_start_idx, HccLocation** callsite_location_out);
void hcc_pp_expand_pop(HccCompiler* c, HccPPExpand* expected_expand);
void hcc_pp_copy_expand_range(HccCompiler* c, HccPPExpand* expand, HccTokenBag* dst_bag, HccTokenBag* src_bag, HccTokenBag* alt_dst_bag, HccLocation* parent_or_child_location, HccLocation* grandparent_location, HccPPExpandFlags flags, HccPPMacro* expand_macro);
void hcc_pp_copy_expand_macro(HccCompiler* c, HccPPMacro* macro, HccLocation* macro_callsite_location, HccLocation* parent_location, HccPPExpand* arg_expand, HccTokenBag* args_src_bag, HccTokenBag* dst_bag, HccTokenBag* alt_dst_bag, HccPPExpandFlags flags);
U32 hcc_pp_process_macro_args(HccCompiler* c, HccPPMacro* macro, HccPPExpand* expand, HccTokenBag* src_bag, HccLocation* parent_location);
HccPPMacroArg* hcc_pp_push_macro_arg(HccCompiler* c, HccPPExpand* expand, HccTokenBag* src_bag, HccLocation* parent_location);
void hcc_pp_finalize_macro_arg(HccCompiler* c, HccPPMacroArg* arg, HccPPExpand* expand, HccTokenBag* src_bag);
void hcc_pp_attach_to_most_parent(HccCompiler* c, HccLocation* location, HccLocation* parent_location);

// ===========================================
//
//
// Token Generator
//
//
// ===========================================

typedef struct HccOpenBracket HccOpenBracket;
struct HccOpenBracket {
	HccToken close_token;
	HccLocation* open_token_location;
};

typedef U8 HccTokenGenRunMode;
enum {
	HCC_TOKENGEN_RUN_MODE_CODE,
	HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST,
	HCC_TOKENGEN_RUN_MODE_PP_INCLUDE_OPERAND,
	HCC_TOKENGEN_RUN_MODE_PP_IF_OPERAND,
	HCC_TOKENGEN_RUN_MODE_PP_OPERAND,
	HCC_TOKENGEN_RUN_MODE_PP_MACRO_ARGS,
	HCC_TOKENGEN_RUN_MODE_PP_CONCAT,
};

typedef struct HccPausedFile HccPausedFile;
struct HccPausedFile {
	U32         if_span_stack_count;
	HccLocation location;
};

typedef struct HccTokenGen HccTokenGen;
struct HccTokenGen {
	HccTokenGenRunMode       run_mode;
	HccTokenBag              token_bag;
	HccTokenBag*             dst_token_bag;
	HccStack(HccPausedFile)  paused_file_stack;
	HccStack(HccOpenBracket) open_bracket_stack;
	HccStack(HccLocation)    token_locations;

	//
	// data used when run_mode == HCC_TOKENGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST
	bool                     macro_is_function;
	bool                     macro_has_va_arg;
	HccStringId*             macro_param_string_ids;
	U32                      macro_params_count;
	U32                      macro_tokens_start_idx;

	HccLocation              location;
	char*                    code;      // is a local copy of location.code_file->code.data
	U32                      code_size; // is a local copy of location.code_file->code.size
	U32                      custom_line_dst;
	U32                      custom_line_src;

	S32 __counter__;
};

typedef struct HccTokenGenSetup HccTokenGenSetup;
struct HccTokenGenSetup {
	U32 tokens_cap;
	U32 token_values_cap;
	U32 token_locations_cap;
	U32 paused_file_stack_cap;
	U32 open_bracket_stack_cap;
};

void hcc_tokengen_init(HccCompiler* c, HccCompilerSetup* setup);
void hcc_tokengen_advance_column(HccCompiler* c, U32 by);
void hcc_tokengen_advance_newline(HccCompiler* c);
U32 hcc_tokengen_display_line(HccCompiler* c);
U32 hcc_tokengen_token_add(HccCompiler* c, HccToken token);
U32 hcc_tokengen_token_value_add(HccCompiler* c, HccTokenValue value);
void hcc_tokengen_count_extra_newlines(HccCompiler* c);
noreturn void hcc_tokengen_bail_error_1(HccCompiler* c, HccErrorCode error_code, ...);
noreturn void hcc_tokengen_bail_error_2_idx(HccCompiler* c, HccErrorCode error_code, U32 other_token_idx, ...);
noreturn void hcc_tokengen_bail_error_2_ptr(HccCompiler* c, HccErrorCode error_code, HccLocation* token_location, HccLocation* other_token_location, ...);
void hcc_tokengen_location_push(HccCompiler* c);
void hcc_tokengen_location_pop(HccCompiler* c);
void hcc_tokengen_location_setup_new_file(HccCompiler* c, HccCodeFile* code_file, bool change_dir);

bool hcc_tokengen_consume_backslash(HccCompiler* c);
void hcc_tokengen_consume_whitespace(HccCompiler* c);
void hcc_tokengen_consume_whitespace_and_newlines(HccCompiler* c);
void hcc_tokengen_consume_until_any_byte(HccCompiler* c, char* terminator_bytes);

HccString hcc_tokengen_parse_ident_from_string(HccCompiler* c, HccString string, HccErrorCode error_code);
HccString hcc_tokengen_parse_ident(HccCompiler* c, HccErrorCode error_code);


U32 hcc_tokengen_parse_num(HccCompiler* c, HccToken* token_out);
void hcc_tokengen_parse_string(HccCompiler* c, char terminator_byte, bool ignore_escape_sequences_except_double_quotes);
U32 hcc_tokengen_find_macro_param(HccCompiler* c, HccStringId ident_string_id);
void hcc_tokengen_consume_hash_for_define_replacement_list(HccCompiler* c);
bool hcc_tokengen_is_first_non_whitespace_on_line(HccCompiler* c);
void hcc_tokengen_bracket_open(HccCompiler* c, HccToken token, HccLocation* location);
void hcc_tokengen_bracket_close(HccCompiler* c, HccToken token, HccLocation* location);
void hcc_tokengen_run(HccCompiler* c, HccTokenBag* dst_token_bag, HccTokenGenRunMode run_mode);

void hcc_tokengen_print(HccCompiler* c, FILE* f);

// ===========================================
//
//
// Syntax Generator: Tokens -> AST
//
//
// ===========================================

typedef U8 HccUnaryOp;
enum {
	HCC_UNARY_OP_LOGICAL_NOT,
	HCC_UNARY_OP_BIT_NOT,
	HCC_UNARY_OP_PLUS,
	HCC_UNARY_OP_NEGATE,
	HCC_UNARY_OP_PRE_INCREMENT,
	HCC_UNARY_OP_PRE_DECREMENT,
	HCC_UNARY_OP_POST_INCREMENT,
	HCC_UNARY_OP_POST_DECREMENT,
	HCC_UNARY_OP_DEREF,
	HCC_UNARY_OP_ADDRESS_OF,

	HCC_UNARY_OP_COUNT,
};

typedef U8 HccExprType;
enum {
	HCC_EXPR_TYPE_NONE,

	HCC_EXPR_TYPE_CALL,
	HCC_EXPR_TYPE_ARRAY_SUBSCRIPT,
	HCC_EXPR_TYPE_CURLY_INITIALIZER,
	HCC_EXPR_TYPE_DESIGNATED_INITIALIZER,
	HCC_EXPR_TYPE_FIELD_ACCESS,
	HCC_EXPR_TYPE_CAST,

	HCC_EXPR_TYPE_LOCAL_VARIABLE,
	HCC_EXPR_TYPE_GLOBAL_VARIABLE,

	HCC_EXPR_TYPE_CONSTANT,
	HCC_EXPR_TYPE_DATA_TYPE,
	HCC_EXPR_TYPE_FUNCTION,
	HCC_EXPR_TYPE_CALL_ARG_LIST,
	HCC_EXPR_TYPE_STMT_IF,
	HCC_EXPR_TYPE_STMT_SWITCH,
	HCC_EXPR_TYPE_STMT_WHILE,
	HCC_EXPR_TYPE_STMT_FOR,
	HCC_EXPR_TYPE_STMT_CASE,
	HCC_EXPR_TYPE_STMT_DEFAULT,
	HCC_EXPR_TYPE_STMT_BREAK,
	HCC_EXPR_TYPE_STMT_CONTINUE,

	HCC_EXPR_TYPE_STMT_RETURN,
	HCC_EXPR_TYPE_STMT_BLOCK,

	//
	// binary ops
	HCC_EXPR_TYPE_BINARY_OP_START,
#define HCC_EXPR_TYPE_BINARY_OP(OP) (HCC_EXPR_TYPE_BINARY_OP_START + HCC_BINARY_OP_##OP)
	HCC_EXPR_TYPE_BINARY_OP_END = HCC_EXPR_TYPE_BINARY_OP_START + HCC_BINARY_OP_COUNT,

	//
	// unary ops
	HCC_EXPR_TYPE_UNARY_OP_START,
#define HCC_EXPR_TYPE_UNARY_OP(OP) (HCC_EXPR_TYPE_UNARY_OP_START + HCC_UNARY_OP_##OP)
	HCC_EXPR_TYPE_UNARY_OP_END = HCC_EXPR_TYPE_UNARY_OP_START + HCC_UNARY_OP_COUNT,
};

typedef struct HccExpr HccExpr;
struct HccExpr {
	union {
		//
		// shared header
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
		};

		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 idx;
		} function;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 expr_rel_idx;
		} unary;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 is_assignment: 1;
			U32 left_expr_rel_idx: 24;
			U32 right_expr_rel_idx;
		} binary;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 id;
		} constant;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 has_return_stmt: 1;
			U32 first_expr_rel_idx: 12;
			U32 variables_count: 12;
			U32 block_idx: 16;
			U32 stmts_count: 16;
		} stmt_block;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 first_expr_rel_idx;
		} curly_initializer;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 value_expr_rel_idx;
			// alt_next_expr_rel_idx is the designated_initializer_idx into HccAstGenCurlyInitializer.designated_initializers
		} designated_initializer;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 25;
			U32 true_stmt_rel_idx;
		} if_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 25;
			U32 block_expr_rel_idx;
			// alt_next_expr_rel_idx is the default_case_expr_rel_idx
		} switch_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 25;
			U32 loop_stmt_rel_idx;
		} while_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 init_expr_rel_idx: 12;
			U32 cond_expr_rel_idx: 13;
			U32 inc_expr_rel_idx: 16;
			U32 loop_stmt_rel_idx: 16;
		} for_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 idx;
		} variable;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 false_expr_rel_idx: 25;
			U32 cond_expr_rel_idx: 13;
			U32 true_expr_rel_idx: 19;
		} ternary;
	};

	union {
		struct { // if is_stmt_block_entry
			U16 alt_next_expr_rel_idx;
			U16 next_expr_rel_idx;
		};
		struct { // this is stored in the true statement of the if statement
			U32 false_stmt_rel_idx: 16;
			U32 true_and_false_stmts_have_return_stmt: 1;
			U32 __unused: 15;
		} if_aux;
		struct {
			U32 case_stmts_count: 17;
			U32 first_case_expr_rel_idx: 15;
		} switch_aux;
		HccDataType data_type;
	};
};

static_assert(sizeof(HccExpr) == 12, "HccExpr must be 12 bytes");

typedef struct HccSwitchState HccSwitchState;
struct HccSwitchState {
	HccExpr* switch_stmt;
	HccExpr* first_switch_case;
	HccExpr* prev_switch_case;
	HccExpr* default_switch_case;
	HccDataType switch_condition_type;
	U32 case_stmts_count;
};

typedef struct HccFieldAccess HccFieldAccess;
struct HccFieldAccess {
	HccDataType data_type;
	U32 idx;
};

typedef struct HccAstGenCurlyInitializerCurly HccAstGenCurlyInitializerCurly;
struct HccAstGenCurlyInitializerCurly {
	U32 nested_elmts_start_idx;
	bool found_designator;
};

typedef struct HccAstGenCurlyInitializerElmt  HccAstGenCurlyInitializerElmt;
struct HccAstGenCurlyInitializerElmt {
	HccDataType data_type; // this is the outer data type that we are initializing at this nested layer
	HccDataType resolved_data_type;
	U64 elmt_idx: 63; // this is the element index into the outer data type that we are initializing
	U64 had_explicit_designator_for_union_field: 1;
};

typedef struct HccAstGenDesignatorInitializer HccAstGenDesignatorInitializer;
struct HccAstGenDesignatorInitializer {
	U32 elmt_indices_start_idx;
	U32 elmt_indices_count;
};

typedef struct HccAstGenCurlyInitializerNested HccAstGenCurlyInitializerNested;
struct HccAstGenCurlyInitializerNested {
	HccExpr* prev_initializer_expr;
	HccExpr* first_initializer_expr;
	U32      nested_elmts_start_idx;
};

typedef struct HccAstGenCurlyInitializer HccAstGenCurlyInitializer;
struct HccAstGenCurlyInitializer {
	union {
		HccCompoundDataType* compound_data_type;
		HccArrayDataType* array_data_type;
	};
	HccCompoundField* compound_fields;
	HccDataType composite_data_type;
	HccDataType elmt_data_type;
	HccDataType resolved_composite_data_type;
	HccDataType resolved_elmt_data_type;
	U64 elmts_end_idx;

	//
	// a stack to keep track the nested curly initializer expressions.
	// these can happen in variable declaration or for compound literals.
	HccStack(HccAstGenCurlyInitializerNested) nested;

	//
	// a stack to keep track of when we open a new set of curly braces
	// and how to return back to the parent pair of curly braces
	HccStack(HccAstGenCurlyInitializerCurly) nested_curlys;

	//
	// a stack to keep track of each nested elements when we tunnel into nested data types
	// so we can tunnel out and resume from where we were
	HccStack(HccAstGenCurlyInitializerElmt) nested_elmts;

	HccExpr* prev_initializer_expr;
	HccExpr* first_initializer_expr;
	U32      nested_elmts_start_idx;

	//
	// each HCC_EXPR_TYPE_DESIGNATED_INITIALIZER node will reference
	// the designated_initializers array
	HccStack(HccAstGenDesignatorInitializer) designated_initializers;
	HccStack(U64) designated_initializer_elmt_indices;
};

typedef U8 HccSpecifier;
enum {
	HCC_SPECIFIER_STATIC,
	HCC_SPECIFIER_INLINE,
	HCC_SPECIFIER_NO_RETURN,

	HCC_SPECIFIER_INTRINSIC,
	HCC_SPECIFIER_RASTERIZER_STATE,
	HCC_SPECIFIER_FRAGMENT_STATE,
	HCC_SPECIFIER_BUFFER_ELEMENT,
	HCC_SPECIFIER_RESOURCE_SET,
	HCC_SPECIFIER_RESOURCE_TABLE,
	HCC_SPECIFIER_RESOURCES,
	HCC_SPECIFIER_POSITION,
	HCC_SPECIFIER_NOINTERP,

	HCC_SPECIFIER_VERTEX,
	HCC_SPECIFIER_FRAGMENT,

	HCC_SPECIFIER_COUNT,
};

typedef U16 HccSpecifierFlags;
enum {
	HCC_SPECIFIER_FLAGS_STATIC =              1 << HCC_SPECIFIER_STATIC,
	HCC_SPECIFIER_FLAGS_INLINE =              1 << HCC_SPECIFIER_INLINE,
	HCC_SPECIFIER_FLAGS_NO_RETURN =           1 << HCC_SPECIFIER_NO_RETURN,

	HCC_SPECIFIER_FLAGS_INTRINSIC =           1 << HCC_SPECIFIER_INTRINSIC,
	HCC_SPECIFIER_FLAGS_RASTERIZER_STATE =    1 << HCC_SPECIFIER_RASTERIZER_STATE,
	HCC_SPECIFIER_FLAGS_FRAGMENT_STATE =      1 << HCC_SPECIFIER_FRAGMENT_STATE,
	HCC_SPECIFIER_FLAGS_BUFFER_ELEMENT =      1 << HCC_SPECIFIER_BUFFER_ELEMENT,
	HCC_SPECIFIER_FLAGS_RESOURCE_SET =        1 << HCC_SPECIFIER_RESOURCE_SET,
	HCC_SPECIFIER_FLAGS_RESOURCE_TABLE =      1 << HCC_SPECIFIER_RESOURCE_TABLE,
	HCC_SPECIFIER_FLAGS_RESOURCES =           1 << HCC_SPECIFIER_RESOURCES,
	HCC_SPECIFIER_FLAGS_POSITION =            1 << HCC_SPECIFIER_POSITION,
	HCC_SPECIFIER_FLAGS_NOINTERP =            1 << HCC_SPECIFIER_NOINTERP,

	HCC_SPECIFIER_FLAGS_VERTEX =              1 << HCC_SPECIFIER_VERTEX,
	HCC_SPECIFIER_FLAGS_FRAGMENT =            1 << HCC_SPECIFIER_FRAGMENT,

	HCC_SPECIFIER_FLAGS_ALL_SHADER_STAGES = HCC_SPECIFIER_FLAGS_VERTEX | HCC_SPECIFIER_FLAGS_FRAGMENT,

	HCC_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS = HCC_SPECIFIER_FLAGS_STATIC,
	HCC_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS =
			HCC_SPECIFIER_FLAGS_INTRINSIC |
			HCC_SPECIFIER_FLAGS_STATIC    |
			HCC_SPECIFIER_FLAGS_INLINE    |
			HCC_SPECIFIER_FLAGS_NO_RETURN |
			HCC_SPECIFIER_FLAGS_ALL_SHADER_STAGES,
	HCC_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS =
		HCC_SPECIFIER_FLAGS_INTRINSIC           |
		HCC_SPECIFIER_FLAGS_RASTERIZER_STATE    |
		HCC_SPECIFIER_FLAGS_FRAGMENT_STATE      |
		HCC_SPECIFIER_FLAGS_FRAGMENT_STATE      |
		HCC_SPECIFIER_FLAGS_BUFFER_ELEMENT      |
		HCC_SPECIFIER_FLAGS_RESOURCE_SET        |
		HCC_SPECIFIER_FLAGS_RESOURCE_TABLE      ,
	HCC_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS = HCC_SPECIFIER_FLAGS_POSITION | HCC_SPECIFIER_FLAGS_NOINTERP,
	HCC_SPECIFIER_FLAGS_ALL_TYPEDEF_SPECIFIER = HCC_SPECIFIER_FLAGS_INTRINSIC,

	HCC_SPECIFIER_FLAGS_ALL =
		HCC_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS     |
		HCC_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS     |
		HCC_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS       |
		HCC_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS |
		HCC_SPECIFIER_FLAGS_ALL_TYPEDEF_SPECIFIER       ,

	HCC_SPECIFIER_FLAGS_ALL_NON_VARIABLE_SPECIFIERS =      HCC_SPECIFIER_FLAGS_ALL & ~HCC_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS,
	HCC_SPECIFIER_FLAGS_ALL_NON_FUNCTION_SPECIFIERS =      HCC_SPECIFIER_FLAGS_ALL & ~HCC_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS,
	HCC_SPECIFIER_FLAGS_ALL_NON_STRUCT_SPECIFIERS =        HCC_SPECIFIER_FLAGS_ALL & ~HCC_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS,
	HCC_SPECIFIER_FLAGS_ALL_NON_STRUCT_FIELD_SPECIFIERS =  HCC_SPECIFIER_FLAGS_ALL & ~HCC_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS,
	HCC_SPECIFIER_FLAGS_ALL_NON_TYPEDEF_SPECIFIERS =       HCC_SPECIFIER_FLAGS_ALL & ~HCC_SPECIFIER_FLAGS_ALL_TYPEDEF_SPECIFIER,
};

typedef U16 HccTypeSpecfier;
enum {
	HCC_TYPE_SPECIFIER_VOID =          0x1,
	HCC_TYPE_SPECIFIER_BOOL =          0x2,
	HCC_TYPE_SPECIFIER_CHAR =          0x4,
	HCC_TYPE_SPECIFIER_SHORT =         0x8,
	HCC_TYPE_SPECIFIER_INT =           0x10,
	HCC_TYPE_SPECIFIER_LONG =          0x20,
	HCC_TYPE_SPECIFIER_LONGLONG =      0x40,
	HCC_TYPE_SPECIFIER_FLOAT =         0x80,
	HCC_TYPE_SPECIFIER_DOUBLE =        0x100,
	HCC_TYPE_SPECIFIER_UNSIGNED =      0x200,
	HCC_TYPE_SPECIFIER_SIGNED =        0x400,
	HCC_TYPE_SPECIFIER_COMPLEX =       0x800,
	HCC_TYPE_SPECIFIER_ATOMIC =        0x1000,
	HCC_TYPE_SPECIFIER_CONST =         0x2000,
	HCC_TYPE_SPECIFIER_VOLATILE =      0x4000,
	HCC_TYPE_SPECIFIER_GENERIC_FLOAT = 0x8000,

	HCC_TYPE_SPECIFIER_TYPES =
		HCC_TYPE_SPECIFIER_VOID  |
		HCC_TYPE_SPECIFIER_BOOL  |
		HCC_TYPE_SPECIFIER_CHAR  |
		HCC_TYPE_SPECIFIER_SHORT |
		HCC_TYPE_SPECIFIER_INT   |
		HCC_TYPE_SPECIFIER_LONG  |
		HCC_TYPE_SPECIFIER_FLOAT |
		HCC_TYPE_SPECIFIER_DOUBLE,

	HCC_TYPE_SPECIFIER_FLOAT_TYPES =
		HCC_TYPE_SPECIFIER_FLOAT        |
		HCC_TYPE_SPECIFIER_DOUBLE       |
		HCC_TYPE_SPECIFIER_GENERIC_FLOAT,

	HCC_TYPE_SPECIFIER_LONG_DOUBLE = HCC_TYPE_SPECIFIER_LONG | HCC_TYPE_SPECIFIER_DOUBLE,
	HCC_TYPE_SPECIFIER_UNSIGNED_SIGNED = HCC_TYPE_SPECIFIER_UNSIGNED | HCC_TYPE_SPECIFIER_SIGNED,
};

typedef struct HccLogicalAddressMutation HccLogicalAddressMutation;
struct HccLogicalAddressMutation {
	U32 block_id;
	HccLocation* location;
};

typedef U16 HccStmtBlockInfo;
#define HCC_STMT_BLOCK_INFO(parent_idx) ((parent_idx) & 0x7fff)
#define HCC_STMT_BLOCK_INFO_SET_IS_CONDITIONAL_BLOCK(ptr) (*(ptr) |= 0x8000)
#define HCC_STMT_BLOCK_INFO_IS_CONDITIONAL_BLOCK(v) ((v) & 0x8000)
#define HCC_STMT_BLOCK_INFO_PARENT_IDX(v) ((v) & 0x7fff)

typedef struct HccAstGen HccAstGen;
struct HccAstGen {
	HccSpecifierFlags specifier_flags;
	U8 resource_set_slot;

	HccTokenBag token_bag;
	U32 token_read_idx;
	U32 token_value_read_idx;

	HccStack(HccVariable)                  function_params_and_variables;
	HccStack(HccFunction)                  functions;
	HccStack(U32)                          function_used_function_indices;
	HccStack(HccDecl)                      function_used_static_variables;
	HccStack(U32)                          entry_point_function_indices;
	HccStack(U32)                          used_function_indices;
	HccStack(U32)                          recursive_function_indices_stack;
	HccStack(HccExpr)                      exprs;
	HccStack(HccLocation)                  expr_locations;
	HccStack(HccVariable)                  global_variables;
	HccStack(HccArrayDataType)             array_data_types;
	HccStack(HccCompoundDataType)          compound_data_types;
	HccStack(HccCompoundField)             compound_fields;
	HccStack(HccTypedef)                   typedefs;
	HccStack(HccEnumDataType)              enum_data_types;
	HccStack(HccEnumValue)                 enum_values;
	HccStack(HccSimpleBufferDataType)      simple_buffer_data_types;
	HccStack(HccPointerDataType)           pointer_data_types;
	HccStack(HccDataType)                  ordered_data_types;
	HccStack(HccFieldAccess)               compound_type_find_fields;
	HccStack(HccLogicalAddressMutation)    logical_address_mutations;
	HccStack(HccStmtBlockInfo)             stmt_block_infos;
	HccHashTable(HccStringId, HccDecl)     global_declarations;
	HccHashTable(HccStringId, HccDataType) struct_declarations;
	HccHashTable(HccStringId, HccDataType) union_declarations;
	HccHashTable(HccStringId, HccDataType) enum_declarations;

	HccAstGenCurlyInitializer curly_initializer;

	HccDataType assign_data_type;

	HccExpr* stmt_block;
	U32      stmt_block_idx;
	HccFunction* function;
	U32 print_variable_base_idx;

	HccSwitchState switch_state;
	bool is_in_loop;

	HccStack(HccStringId) variable_stack_strings;
	HccStack(U32)         variable_stack_var_indices;
	U32                   next_var_idx;

	HccHashTable(HccStringId, U32) field_name_to_token_idx;
};

typedef struct HccAstGenSetup HccAstGenSetup;
struct HccAstGenSetup {
	U32 function_params_and_variables_cap;
	U32 functions_cap;
	U32 exprs_cap;
	U32 expr_locations_cap;
	U32 global_variables_cap;
	U32 function_used_function_indices_cap;
	U32 function_used_static_variables_cap;
	U32 entry_point_function_indices_cap;
	U32 used_function_indices_cap;
	U32 recursive_function_indices_stack_cap;
	U32 array_data_types_cap;
	U32 struct_data_types_cap;
	U32 union_data_types_cap;
	U32 compound_fields_cap;
	U32 compound_type_fields_cap;
	U32 typedefs_cap;
	U32 enum_data_types_cap;
	U32 enum_values_cap;
	U32 ordered_data_types_cap;
	U32 compound_type_find_fields_cap;
	U32 curly_initializer_nested_cap;
	U32 curly_initializer_nested_curlys_cap;
	U32 curly_initializer_nested_elmts_cap;
	U32 curly_initializer_designator_initializers_cap;
	U32 curly_initializer_designator_initializer_elmt_indices_cap;
	U32 variable_stack_cap;
};

extern HccToken hcc_specifier_tokens[HCC_SPECIFIER_COUNT];
extern U8 hcc_unary_op_precedence[HCC_UNARY_OP_COUNT];

void hcc_astgen_init(HccCompiler* c, HccCompilerSetup* setup);

void hcc_astgen_error_1(HccCompiler* c, HccErrorCode error_code, ...);
void hcc_astgen_error_2_idx(HccCompiler* c, HccErrorCode error_code, U32 other_token_idx, ...);
void hcc_astgen_error_2_ptr(HccCompiler* c, HccErrorCode error_code, HccLocation* other_location, ...);
void hcc_astgen_warn_1(HccCompiler* c, HccWarnCode warn_code, ...);
void hcc_astgen_warn_2_idx(HccCompiler* c, HccWarnCode warn_code, U32 other_token_idx, ...);
void hcc_astgen_warn_2_ptr(HccCompiler* c, HccWarnCode warn_code, HccLocation* other_location, ...);
noreturn void hcc_astgen_bail_error_1(HccCompiler* c, HccErrorCode error_code, ...);
noreturn void hcc_astgen_bail_error_1_merge_apply(HccCompiler* c, HccErrorCode error_code, HccLocation* location, ...);
noreturn void hcc_astgen_bail_error_2_idx(HccCompiler* c, HccErrorCode error_code, U32 other_token_idx, ...);
noreturn void hcc_astgen_bail_error_2_ptr(HccCompiler* c, HccErrorCode error_code, HccLocation* other_location, ...);

HccToken hcc_astgen_token_peek(HccCompiler* c);
HccToken hcc_astgen_token_peek_ahead(HccCompiler* c, U32 by);
void hcc_astgen_token_consume(HccCompiler* c, U32 amount);
HccToken hcc_astgen_token_next(HccCompiler* c);
void hcc_astgen_token_value_consume(HccCompiler* c, U32 amount);
HccTokenValue hcc_astgen_token_value_peek(HccCompiler* c);
HccTokenValue hcc_astgen_token_value_next(HccCompiler* c);

void hcc_astgen_data_type_found(HccCompiler* c, HccDataType data_type);
HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name(HccCompiler* c, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id);
HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_checked(HccCompiler* c, HccDataType data_type, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id);
HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_recursive(HccCompiler* c, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id);
void hcc_astgen_static_variable_usage_found(HccCompiler* c, HccDecl decl);
void hcc_astgen_insert_global_declaration(HccCompiler* c, HccStringId identifier_string_id, HccDecl decl);
void hcc_astgen_eval_cast(HccCompiler* c, HccExpr* expr, HccDataType dst_data_type);
bool hcc_stmt_has_return(HccExpr* stmt);
HccExpr* hcc_astgen_alloc_expr(HccCompiler* c, HccExprType type);
HccExpr* hcc_astgen_alloc_expr_many(HccCompiler* c, U32 amount);

const char* hcc_type_specifier_string(HccTypeSpecfier specifier);
void hcc_astgen_data_type_ensure_is_condition(HccCompiler* c, HccDataType data_type);
void hcc_astgen_compound_data_type_validate_field_names(HccCompiler* c, HccDataType outer_data_type, HccCompoundDataType* compound_data_type);
void hcc_astgen_validate_specifiers(HccCompiler* c, HccSpecifierFlags non_specifiers, HccErrorCode invalid_specifier_error_code);
void hcc_astgen_ensure_semicolon(HccCompiler* c);
void hcc_astgen_ensure_not_unsupported_basic_type(HccCompiler* c, HccLocation* location, HccDataType data_type);
bool hcc_data_type_check_compatible_assignment(HccCompiler* c, HccDataType target_data_type, HccExpr** source_expr_mut);
void hcc_data_type_ensure_compatible_assignment(HccCompiler* c, U32 other_token_idx, HccDataType target_data_type, HccExpr** source_expr_mut);
bool hcc_data_type_check_compatible_arithmetic(HccCompiler* c, HccExpr** left_expr_mut, HccExpr** right_expr_mut);
void hcc_data_type_ensure_compatible_arithmetic(HccCompiler* c, U32 other_token_idx, HccExpr** left_expr_mut, HccExpr** right_expr_mut, HccToken operator_token);
void hcc_astgen_ensure_function_args_count(HccCompiler* c, HccFunction* function, U32 args_count);
HccDataType hcc_astgen_deduplicate_array_data_type(HccCompiler* c, HccDataType element_data_type, HccConstantId size_constant_id);
HccDataType hcc_astgen_deduplicate_pointer_data_type(HccCompiler* c, HccDataType element_data_type);
HccDataType hcc_astgen_deduplicate_simple_buffer_data_type(HccCompiler* c, HccResourceType resource_type, HccDataType element_data_type);
void _hcc_astgen_ensure_no_unused_specifiers(HccCompiler* c, char* what);
void hcc_astgen_ensure_no_unused_specifiers_data_type(HccCompiler* c);
void hcc_astgen_ensure_no_unused_specifiers_identifier(HccCompiler* c);

void hcc_astgen_variable_stack_open(HccCompiler* c);
void hcc_astgen_variable_stack_close(HccCompiler* c);
U32 hcc_astgen_variable_stack_add(HccCompiler* c, HccStringId string_id);
U32 hcc_astgen_variable_stack_find(HccCompiler* c, HccStringId string_id);

void hcc_astgen_logical_address_declared(HccCompiler* c, HccVariable* variable);
U32 hcc_astgen_logical_address_mutation_idx_from_expr(HccCompiler* c, HccExpr* expr);
void hcc_astgen_logical_address_apply_assigned(HccCompiler* c, U32 logical_mutation_base_idx, HccDataType data_type, HccLocation* location);
void hcc_astgen_logical_address_assigned(HccCompiler* c, HccExpr* expr, HccLocation* location);
void hcc_astgen_logical_address_assigned_field(HccCompiler* c, HccVariable* variable, U64* elmt_indices, U32 elmts_count, HccLocation* location);
void hcc_astgen_logical_address_used(HccCompiler* c, HccExpr* expr, HccLocation* location);

HccToken hcc_astgen_curly_initializer_start(HccCompiler* c, HccDataType data_type, HccDataType resolved_data_type, HccExpr* first_expr);
HccToken hcc_astgen_curly_initializer_open(HccCompiler* c);
HccToken hcc_astgen_curly_initializer_close(HccCompiler* c, bool is_finished);
bool hcc_astgen_curly_initializer_next_elmt(HccCompiler* c, HccDataType resolved_target_data_type);
HccToken hcc_astgen_curly_initializer_next_elmt_with_designator(HccCompiler* c);
void hcc_astgen_curly_initializer_nested_elmt_push(HccCompiler* c, HccDataType data_type, HccDataType resolved_data_type);
void hcc_astgen_curly_initializer_tunnel_in(HccCompiler* c);
void hcc_astgen_curly_initializer_tunnel_out(HccCompiler* c);
void hcc_astgen_curly_initializer_set_composite(HccCompiler* c, HccDataType data_type, HccDataType resolved_data_type);
HccExpr* hcc_astgen_curly_initializer_generate_designated_initializer(HccCompiler* c);

U32 hcc_astgen_stmt_block_info_alloc(HccCompiler* c, U32 parent_idx);
HccStmtBlockInfo* hcc_astgen_stmt_block_info_get(HccCompiler* c, U32 block_idx);
void hcc_astgen_stmt_block_found_conditional(HccCompiler* c, U32 block_idx);

HccToken hcc_astgen_generate_specifiers(HccCompiler* c);
HccDataType hcc_astgen_generate_enum_data_type(HccCompiler* c);
HccDataType hcc_astgen_generate_compound_data_type(HccCompiler* c);
HccToken hcc_astgen_generate_type_specifiers(HccCompiler* c, HccLocation* location, HccTypeSpecfier* type_specifiers_mut);
HccDataType hcc_astgen_generate_data_type(HccCompiler* c, HccErrorCode error_code);
HccDataType hcc_astgen_generate_pointer_data_type_if_exists(HccCompiler* c, HccDataType element_data_type);
HccDataType hcc_astgen_generate_array_data_type_if_exists(HccCompiler* c, HccDataType element_data_type);
HccDataType hcc_astgen_generate_typedef(HccCompiler* c);
HccDataType hcc_astgen_generate_typedef_with_data_type(HccCompiler* c, HccDataType aliased_data_type);
void hcc_astgen_generate_implicit_cast(HccCompiler* c, HccDataType dst_data_type, HccExpr** expr_mut);
HccExpr* hcc_astgen_generate_unary_op(HccCompiler* c, HccExpr* inner_expr, HccUnaryOp unary_op, HccToken operator_token);
HccExpr* hcc_astgen_generate_unary_expr(HccCompiler* c);
void hcc_astgen_generate_binary_op(HccCompiler* c, HccExprType* binary_op_type_out, U32* precedence_out, bool* is_assignment_out);
HccExpr* hcc_astgen_generate_call_expr(HccCompiler* c, HccExpr* function_expr);
HccExpr* hcc_astgen_generate_array_subscript_expr(HccCompiler* c, HccExpr* array_expr);
HccExpr* hcc_astgen_generate_field_access_expr(HccCompiler* c, HccExpr* left_expr);
HccExpr* hcc_astgen_generate_ternary_expr(HccCompiler* c, HccExpr* cond_expr);
HccExpr* hcc_astgen_generate_expr_(HccCompiler* c, U32 min_precedence, bool no_comma_operator);
HccExpr* hcc_astgen_generate_expr(HccCompiler* c, U32 min_precedence);
HccExpr* hcc_astgen_generate_expr_no_comma_operator(HccCompiler* c, U32 min_precedence);
HccExpr* hcc_astgen_generate_cond_expr(HccCompiler* c);
HccDataType hcc_astgen_generate_variable_decl_array(HccCompiler* c, HccDataType element_data_type);
U32 hcc_astgen_generate_variable_decl(HccCompiler* c, bool is_global, HccDataType element_data_type, HccDataType* data_type_mut, HccExpr** init_expr_out);
HccExpr* hcc_astgen_generate_variable_decl_expr(HccCompiler* c, HccDataType data_type);
HccExpr* hcc_astgen_generate_stmt(HccCompiler* c);
void hcc_astgen_generate_function(HccCompiler* c, HccDataType return_data_type, U32 data_type_token_idx);
void hcc_astgen_recurse_used_functions(HccCompiler* c, U32 function_idx);
void hcc_astgen_generate(HccCompiler* c);

void hcc_astgen_print_expr(HccCompiler* c, HccExpr* expr, U32 indent, FILE* f);
void hcc_astgen_print(HccCompiler* c, FILE* f);

// ===========================================
//
//
// IR
//
//
// ===========================================

typedef U8 HccIROpCode;
enum {
	HCC_IR_OP_CODE_NO_OP,

	//
	// memory access
	HCC_IR_OP_CODE_PTR_STATIC_ALLOC,
	HCC_IR_OP_CODE_PTR_LOAD,
	HCC_IR_OP_CODE_PTR_STORE,
	HCC_IR_OP_CODE_PTR_ACCESS_CHAIN,
	HCC_IR_OP_CODE_PTR_ACCESS_CHAIN_IN_BOUNDS,

	//
	// composite operations
	HCC_IR_OP_CODE_COMPOSITE_INIT,
	HCC_IR_OP_CODE_COMPOSITE_ACCESS_CHAIN_GET,
	HCC_IR_OP_CODE_COMPOSITE_ACCESS_CHAIN_SET,

	//
	// branching
	HCC_IR_OP_CODE_BASIC_BLOCK,
	HCC_IR_OP_CODE_SELECTION_MERGE,
	HCC_IR_OP_CODE_LOOP_MERGE,
	HCC_IR_OP_CODE_BRANCH,
	HCC_IR_OP_CODE_BRANCH_CONDITIONAL,
	HCC_IR_OP_CODE_SWITCH,
	HCC_IR_OP_CODE_PHI,

	HCC_IR_OP_CODE_BINARY_OP_START,
#define HCC_IR_OP_CODE_BINARY_OP(OP) (HCC_IR_OP_CODE_BINARY_OP_START + HCC_BINARY_OP_##OP)
	HCC_IR_OP_CODE_BINARY_OP_END = HCC_IR_OP_CODE_BINARY_OP_START + HCC_BINARY_OP_COUNT,

	HCC_IR_OP_CODE_UNARY_OP_START,
#define HCC_IR_OP_CODE_UNARY_OP(OP) (HCC_IR_OP_CODE_UNARY_OP_START + HCC_UNARY_OP_##OP)
	HCC_IR_OP_CODE_UNARY_OP_END = HCC_IR_OP_CODE_UNARY_OP_START + HCC_UNARY_OP_COUNT,

	//
	// conversion
	HCC_IR_OP_CODE_CONVERT,
	HCC_IR_OP_CODE_BITCAST,
	HCC_IR_OP_CODE_BITCAST_PTR,

	HCC_IR_OP_CODE_FUNCTION_CALL,
	HCC_IR_OP_CODE_FUNCTION_RETURN,
	HCC_IR_OP_CODE_UNREACHABLE,
	HCC_IR_OP_CODE_SELECT,

	HCC_IR_OP_CODE_COUNT,
};

typedef U32 HccIRWord;
typedef HccIRWord HccIRInstr;

//
// inherits HccDataType
typedef HccIRWord HccIROperand;
enum {
	HCC_IR_OPERAND_VALUE = HCC_DATA_TYPE_COUNT,
	HCC_IR_OPERAND_CONSTANT,
	HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24,
	HCC_IR_OPERAND_GLOBAL_VARIABLE,
	HCC_IR_OPERAND_FUNCTION,
	HCC_IR_OPERAND_SHADER_STAGE_INPUT,
	HCC_IR_OPERAND_RASTERIZER_STATE,
};
#define HCC_IR_OPERAND_IS_DATA_TYPE(operand) (((operand) & 0xff) < HCC_DATA_TYPE_COUNT)
#define HCC_IR_OPERAND_INIT(kind, idx) HCC_DATA_TYPE_INIT(kind, idx)

#define HCC_IR_OPERAND_VALUE_INIT(value_idx) (((value_idx) << 8) | HCC_IR_OPERAND_VALUE)
#define HCC_IR_OPERAND_IS_VALUE(operand) (((operand) & 0xff) == HCC_IR_OPERAND_VALUE)
#define HCC_IR_OPERAND_VALUE_IDX(operand) HCC_DATA_TYPE_IDX(operand)

#define HCC_IR_OPERAND_CONSTANT_INIT(constant_id) (((constant_id) << 8) | HCC_IR_OPERAND_CONSTANT)
#define HCC_IR_OPERAND_IS_CONSTANT(operand) (((operand) & 0xff) == HCC_IR_OPERAND_CONSTANT)
#define HCC_IR_OPERAND_CONSTANT_ID(operand) ((HccConstantId) { .idx_plus_one = HCC_DATA_TYPE_IDX(operand) })

#define HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24_INIT(imm_u24) (((imm_u24) << 8) | HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24)
#define HCC_IR_OPERAND_IS_CONSTANT_IMMEDIATE_U24(operand) (((operand) & 0xff) == HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24)
#define HCC_IR_OPERAND_CONSTANT_IMMEDIATE_U24(operand) HCC_DATA_TYPE_IDX(operand)

#define HCC_IR_OPERAND_GLOBAL_VARIABLE_INIT(variable_idx) (((variable_idx) << 8) | HCC_IR_OPERAND_GLOBAL_VARIABLE)
#define HCC_IR_OPERAND_GLOBAL_VARIABLE_IDX(operand) HCC_DATA_TYPE_IDX(operand)
#define HCC_IR_OPERAND_IS_GLOBAL_VARIABLE(operand) (((operand) & 0xff) == HCC_IR_OPERAND_GLOBAL_VARIABLE)

#define HCC_IR_OPERAND_FUNCTION_INIT(function_idx) (((function_idx) << 8) | HCC_IR_OPERAND_FUNCTION)
#define HCC_IR_OPERAND_FUNCTION_IDX(operand) HCC_DATA_TYPE_IDX(operand)
#define HCC_IR_OPERAND_IS_FUNCTION(operand) (((operand) & 0xff) == HCC_IR_OPERAND_FUNCTION)

#define HCC_IR_OPERAND_SHADER_STAGE_INPUT_INIT(field_idx) (((field_idx) << 8) | HCC_IR_OPERAND_SHADER_STAGE_INPUT)
#define HCC_IR_OPERAND_SHADER_STAGE_INPUT_FIELD_IDX(operand) HCC_DATA_TYPE_IDX(operand)
#define HCC_IR_OPERAND_IS_SHADER_STAGE_INPUT(operand) (((operand) & 0xff) == HCC_IR_OPERAND_SHADER_STAGE_INPUT)

#define HCC_IR_OPERAND_RASTERIZER_STATE_INIT(field_idx) (((field_idx) << 8) | HCC_IR_OPERAND_RASTERIZER_STATE)
#define HCC_IR_OPERAND_RASTERIZER_STATE_FIELD_IDX(operand) HCC_DATA_TYPE_IDX(operand)
#define HCC_IR_OPERAND_IS_RASTERIZER_STATE(operand) (((operand) & 0xff) == HCC_IR_OPERAND_RASTERIZER_STATE)

typedef struct HccIRValue HccIRValue;
struct HccIRValue {
	HccDataType data_type;
};

typedef struct HccIRFunction HccIRFunction;
struct HccIRFunction {
	U32 words_start_idx;
	U32 words_count;
	U32 values_start_idx;
	U32 values_count;
	U32 basic_blocks_start_idx;
	U32 basic_blocks_count;
	U32 instructions_count;
	U32 params_count;
};

typedef struct HccIRInstrEntry HccIRInstrEntry;
struct HccIRInstrEntry {
	HccIROperand* operands;
	U32 instr_word_idx;
	HccIROpCode op_code;
	U16 operands_count;
};

typedef struct HccIRInstrIter HccIRInstrIter;
struct HccIRInstrIter {
	HccIRWord* words;
	U32 next_word_idx;
	U32 words_count;
};

typedef struct HccIRBag HccIRBag;
struct HccIRBag {
	HccStack(HccIRWord)  words;
	HccStack(HccIRValue) values;
	HccStack(U32)        basic_block_words_start_indices;
};

typedef struct HccIR HccIR;
struct HccIR {
	HccStack(HccIRFunction) functions;
	HccIRBag                bag;
};

extern const char* hcc_ir_op_code_strings[HCC_IR_OP_CODE_COUNT];

bool hcc_ir_op_code_has_return_value(HccIROpCode op_code);

HccIRInstrIter hcc_ir_instr_iter_init(HccCompiler* c, HccIRFunction* ir_function);
bool hcc_ir_instr_iter_next(HccIRInstrIter* iter, HccIRInstrEntry* entry_out);
HccIRInstr* hcc_ir_instr_iter_peek_next(HccIRInstrIter* iter);
U32 hcc_ir_instr_iter_word_idx(HccIRInstrIter* iter);

HccIRWord* hcc_ir_function_word(HccCompiler* c, HccIRFunction* ir_function, U32 word_idx);
HccIRValue* hcc_ir_function_value(HccCompiler* c, HccIRFunction* ir_function, U32 value_idx);

void hcc_ir_print_operand(HccCompiler* c, HccIROperand operand, FILE* f);
void hcc_ir_print(HccCompiler* c, FILE* f);

// ===========================================
//
//
// IR Generation: AST -> IR
//
//
// ===========================================

typedef struct HccIRGenBranchState HccIRGenBranchState;
struct HccIRGenBranchState {
	bool all_cases_return;
	U32 break_branch_linked_list_head;
	U32 break_branch_linked_list_tail;
	U32 continue_branch_linked_list_head;
	U32 continue_branch_linked_list_tail;
};

typedef struct HccIRGen HccIRGen;
struct HccIRGen {
	HccStack(HccIRFunction) functions;
	HccStack(HccIRWord) words;
	HccStack(HccIRValue) values;
	HccStack(U32) basic_block_words_start_indices;
	HccFunction*   function;
	HccIRFunction* ir_function;

	HccIROperand basic_block_operand;
	HccIROperand last_operand;
	bool do_not_load_variable;
	HccDataType assign_data_type;
	HccIRBranchState branch_state;
	U32 last_instruction_words_start_idx;
};

typedef struct HccIRGenSetup HccIRGenSetup;
struct HccIRGenSetup {
	U32 functions_cap;
	U32 basic_blocks_cap;
	U32 values_cap;
	U32 words_cap;
	U32 function_call_param_data_types_cap;
};

void hcc_irgen_init(HccCompiler* c, HccCompilerSetup* setup);
HccIROpCode hcc_irgen_instruction_op_code(HccIRInstr* instr);
U32 hcc_irgen_instruction_words_count(HccIRInstr* instr);
U32 hcc_irgen_instruction_operands_count(HccIRInstr* instr);
HccIROperand* hcc_irgen_instruction_operands(HccIRInstr* instr);

HccIROperand hcc_irgen_value_add(HccCompiler* c, HccDataType data_type);
HccIROperand hcc_irgen_value_for_param(U32 param_idx);
HccIROperand hcc_irgen_value_ptr_for_param_or_local_variable(HccCompiler* c, U32 local_variable_idx);
HccIROperand hcc_irgen_basic_block_add(HccCompiler* c);
HccIRWord* hcc_irgen_words_add(HccCompiler* c, U32 count);
HccIROperand* hcc_irgen_instruction_add(HccCompiler* c, HccIROpCode op_code, U32 operands_count);
void hcc_irgen_instruction_remove_last(HccCompiler* c, U32 operands_count);
HccIRInstr* hcc_irgen_instruction_get_last(HccCompiler* c);
HccIROpCode hcc_irgen_instruction_get_last_op_code(HccCompiler* c);
U32 hcc_irgen_instruction_get_last_operands_count(HccCompiler* c);
void hcc_irgen_instruction_shrink_last_operands_count(HccCompiler* c, U32 new_amount);
HccDataType hcc_irgen_operand_data_type(HccCompiler* c, HccFunction* function, HccIRFunction* ir_function, HccIROperand ir_operand);

void hcc_irgen_generate_convert_to_bool(HccCompiler* c, HccIROperand cond_operand, bool flip_bool_result);
void hcc_irgen_generate_condition_expr(HccCompiler* c, HccExpr* cond_expr, bool flip_bool_result);
void hcc_irgen_generate_case_instructions(HccCompiler* c, HccExpr* first_stmt);
void hcc_irgen_generate_ptr_load(HccCompiler* c, HccDataType data_type, HccIROperand src_operand);
void hcc_irgen_generate_ptr_store(HccCompiler* c, HccIROperand dst_operand, HccIROperand src_operand);
void hcc_irgen_generate_bitcast(HccCompiler* c, HccDataType dst_data_type, HccIROperand src_operand, bool is_ptr);
void hcc_irgen_generate_bitcast_union_field(HccCompiler* c, HccDataType union_data_type, U32 field_idx, HccIROperand src_operand);
HccIROperand* hcc_irgen_generate_access_chain_start(HccCompiler* c, U32 count);
void hcc_irgen_generate_access_chain_end(HccCompiler* c, HccDataType data_type);
void hcc_irgen_generate_access_chain_instruction(HccCompiler* c, HccExpr* expr, U32 count);
void hcc_irgen_generate_instructions(HccCompiler* c, HccExpr* expr);
void hcc_irgen_setup_function(HccCompiler* c, HccIRFunction* ir_function, HccFunction* function);
void hcc_irgen_generate_function(HccCompiler* c, U32 function_idx);
void hcc_irgen_generate(HccCompiler* c);

// ===========================================
//
//
// IR Optimization
//
//
// ===========================================

typedef bool (*HccIROptTaskFn)(HccCompiler* c);

typedef struct HccIROpt HccIROpt;
struct HccIROpt {
	HccStack(HccIROptTaskFn) tasks;
	HccStack(U16)            value_indices_remap;
	HccIRFunction*           ir_function;
	HccIRFunction            new_ir_function;
};

typedef struct HccIROptSetup HccIROptSetup;
struct HccIROptSetup {
	U32 tasks_cap;
	U32 value_indices_remap_cap;
};

void hcc_iropt_run(HccCompiler* c);
HccIROperand hcc_iropt_remap_value(HccCompiler* c, HccIROperand value_operand);
void hcc_iropt_keep_instr(HccCompiler* c, HccIRInstr* instr);
HccIROperand hcc_iropt_copy_value(HccCompiler* c, HccIRFunction* ir_function, U32 value_idx);

bool hcc_iropt_task_inline_functions(HccCompiler* c);

// ===========================================
//
//
// SPIR-V Generation
//
//
// ===========================================

#if 0

typedef U16 HccSpirvOp;
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

typedef U16 HccSpirvGLSLSTD450Op;
enum {
	HCC_SPIRV_GLSL_STD_450_OP_ROUND = 1,
	HCC_SPIRV_GLSL_STD_450_OP_TRUNC = 3,
	HCC_SPIRV_GLSL_STD_450_OP_FABS = 4,
	HCC_SPIRV_GLSL_STD_450_OP_SABS = 5,
	HCC_SPIRV_GLSL_STD_450_OP_FSIGN = 6,
	HCC_SPIRV_GLSL_STD_450_OP_SSIGN = 7,
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
	HCC_SPIRV_GLSL_STD_450_OP_FMIN = 37,
	HCC_SPIRV_GLSL_STD_450_OP_UMIN = 38,
	HCC_SPIRV_GLSL_STD_450_OP_SMIN = 39,
	HCC_SPIRV_GLSL_STD_450_OP_FMAX = 40,
	HCC_SPIRV_GLSL_STD_450_OP_UMAX = 41,
	HCC_SPIRV_GLSL_STD_450_OP_SMAX = 42,
	HCC_SPIRV_GLSL_STD_450_OP_FCLAMP = 43,
	HCC_SPIRV_GLSL_STD_450_OP_UCLAMP = 44,
	HCC_SPIRV_GLSL_STD_450_OP_SCLAMP = 45,
	HCC_SPIRV_GLSL_STD_450_OP_FMIX = 46,
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
	HCC_SPIRV_GLSL_STD_450_OP_NORMALIZE = 69,
	HCC_SPIRV_GLSL_STD_450_OP_REFLECT = 71,
	HCC_SPIRV_GLSL_STD_450_OP_REFRACT = 72,
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

enum {
	HCC_SPIRV_STORAGE_CLASS_UNIFORM_CONSTANT = 0,
	HCC_SPIRV_STORAGE_CLASS_INPUT = 1,
	HCC_SPIRV_STORAGE_CLASS_UNIFORM = 2,
	HCC_SPIRV_STORAGE_CLASS_OUTPUT = 3,
	HCC_SPIRV_STORAGE_CLASS_WORKGROUP = 4,
	HCC_SPIRV_STORAGE_CLASS_CROSS_WORKGROUP = 5,
	HCC_SPIRV_STORAGE_CLASS_PRIVATE = 6,
	HCC_SPIRV_STORAGE_CLASS_FUNCTION = 7,
	HCC_SPIRV_STORAGE_CLASS_PUSH_CONSTANT = 9,
	HCC_SPIRV_STORAGE_CLASS_IMAGE = 11,
	HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER = 12,
	HCC_SPIRV_STORAGE_CLASS_PHYSICAL_STORAGE_BUFFER = 5349,
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

enum {
	HCC_SPRIV_DECORATION_BUILTIN =  11,
	HCC_SPRIV_DECORATION_FLAT = 14,
	HCC_SPRIV_DECORATION_LOCATION = 30,
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

#define HCC_SPIRV_INSTR_OPERANDS_CAP 24

#define HCC_SPIRV_ID_FROM_INTRINSIC_TYPE(intrinsic_type) (intrinsic_type + 1)
#define HCC_SPIRV_ID_TO_INTRINSIC_TYPE(intrinsic_type) (intrinsic_type - 1)
#define HCC_SPIRV_ID_IS_INTRINSIC_TYPE(id) ((id) && (id) - 1 >= HCC_INTRINSIC_TYPE_BOOL && (id) - 1 < HCC_INTRINSIC_TYPE_COUNT)

typedef U8 HccSpirvTypeKind;
enum {
	HCC_SPIRV_TYPE_KIND_FUNCTION,
	HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE,
	HCC_SPIRV_TYPE_KIND_STATIC_VARIABLE,
	HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER,
	HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_INPUT,
	HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER_OUTPUT,
};

typedef struct HccSpirvTypeEntry HccSpirvTypeEntry;
struct HccSpirvTypeEntry {
	U32 data_types_start_idx;
	U32 spirv_id: 22;
	U32 data_types_count: 7;
	U32 kind: 3; // HccSpirvTypeKind
};

typedef struct HccSpirvTypeTable HccSpirvTypeTable;
struct HccSpirvTypeTable {
	HccStack(HccDataType) data_types;
	HccStack(HccSpirvTypeEntry) entries;
};

typedef struct HccSpirvGen HccSpirvGen;
struct HccSpirvGen {
	HccSpirvTypeTable type_table;
	HccFunction* function;
	HccIRFunction* ir_function;

	HccStack(U32) out_capabilities;
	HccStack(U32) out_entry_points;
	HccStack(U32) out_debug_info;
	HccStack(U32) out_annotations;
	HccStack(U32) out_types_variables_constants;
	HccStack(U32) out_functions;

	U32 pointer_type_inputs_base_id;
	U32 pointer_type_outputs_base_id;
	U64 pointer_type_inputs_made_bitset[4];
	U64 pointer_type_outputs_made_bitset[4];
	U32 compound_type_base_id;
	U32 array_type_base_id;
	HccIntrinsicBasicTypeMask available_basic_types;
	HccCompoundDataType* rasterizer_state_compound_data_type;
	U32 rasterizer_state_variable_start_spirv_id;
	U32 vertex_index_spirv_id;
	U32 instance_index_spirv_id;
	U32 frag_coord_spirv_id;

	U32 value_base_id;
	U32 constant_base_id;
	U32 local_variable_base_spirv_id;
	U32 global_variable_base_spirv_id;
	U32 function_base_spirv_id;
	U32 ext_inst_glsl_std_450_spirv_id;
	U32 next_id;
	HccSpirvOp instr_op;
	U16 instr_operands_count;
	U32 instr_operands[HCC_SPIRV_INSTR_OPERANDS_CAP];
};

typedef struct HccSpirvGenSetup HccSpirvGenSetup;
struct HccSpirvGenSetup {
	U32 out_capabilities_cap;
	U32 out_entry_points_cap;
	U32 out_debug_info_cap;
	U32 out_annotations_cap;
	U32 out_types_variables_constants_cap;
	U32 out_functions_cap;
};

extern HccSpirvOp hcc_spirv_binary_ops[HCC_BINARY_OP_COUNT][HCC_BASIC_TYPE_CLASS_COUNT];
extern HccSpirvOp hcc_spirv_unary_ops[HCC_UNARY_OP_COUNT][HCC_BASIC_TYPE_CLASS_COUNT];

U32 hcc_spirv_type_table_deduplicate_function(HccCompiler* c, HccFunction* function);
U32 hcc_spirv_type_table_deduplicate_variable(HccCompiler* c, HccDataType data_type, HccSpirvTypeKind kind);

void hcc_spirvgen_init(HccCompiler* c, HccCompilerSetup* setup);

U32 hcc_spirvgen_resolve_type_id(HccCompiler* c, HccDataType data_type);
U32 hcc_spirvgen_convert_operand(HccCompiler* c, HccIROperand ir_operand);
U32 hcc_spirvgen_pointer_type_input_id(HccCompiler* c, HccDataType data_type);
U32 hcc_spirvgen_pointer_type_output_id(HccCompiler* c, HccDataType data_type);
bool hcc_spirvgen_basic_type_is_supported(HccCompiler* c, HccIntrinsicType intrinsic_type);

void hcc_spirvgen_instr_start(HccCompiler* c, HccSpirvOp op);
void hcc_spirvgen_instr_add_operand(HccCompiler* c, U32 word);
void hcc_spirvgen_instr_add_converted_operand(HccCompiler* c, HccIROperand ir_operand);
void hcc_spirvgen_instr_add_result_operand(HccCompiler* c);
#define hcc_spirvgen_instr_add_operands_string_lit(c, string) hcc_spirvgen_instr_add_operands_string(c, string, sizeof(string) - 1)
#define hcc_spirvgen_instr_add_operands_string_c(c, string) hcc_spirvgen_instr_add_operands_string(c, string, strlen(string))
void hcc_spirvgen_instr_add_operands_string(HccCompiler* c, char* string, U32 string_size);
void hcc_spirvgen_instr_end(HccCompiler* c);

U32 hcc_spirvgen_generate_variable_type(HccCompiler* c, HccDataType data_type, HccSpirvTypeKind type_kind);
U32 hcc_spirvgen_generate_function_type(HccCompiler* c, HccFunction* function);
void hcc_spirvgen_generate_select(HccCompiler* c, U32 result_spirv_operand, HccDataType dst_type, U32 cond_value_spirv_operand, U32 a_spirv_operand, U32 b_spirv_operand);
void hcc_spirvgen_generate_convert(HccCompiler* c, HccSpirvOp spirv_convert_op, U32 result_spirv_operand, HccDataType dst_type, U32 value_spirv_operand);
void hcc_spirvgen_generate_entry_point_used_global_variable_spirv_ids(HccCompiler* c, HccFunction* function);
void hcc_spirvgen_generate_instr(HccCompiler* c, HccSpirvOp op, HccDataType return_data_type, HccIROperand* operands, U32 operands_count);
void hcc_spirvgen_generate_ext_instr(HccCompiler* c, HccSpirvGLSLSTD450Op op, HccDataType return_data_type, HccIROperand* operands, U32 operands_count);
void hcc_spirvgen_generate_intrinsic_function(HccCompiler* cc, HccFunction* caller_function, HccIRFunction* caller_ir_function, U32 function_idx, HccDataType return_data_type, HccIROperand* operands, U32 operands_count);
void hcc_spirvgen_generate_function(HccCompiler* c, U32 function_idx);
void hcc_spirvgen_generate_intrinsic_types(HccCompiler* c);
void hcc_spirvgen_generate_basic_type_constants(HccCompiler* c);
void hcc_spirvgen_generate_non_basic_type_constants(HccCompiler* c);
void hcc_spirvgen_generate_load(HccCompiler* c, U32 type_spirv_id, U32 result_id, U32 src_spirv_id);
void hcc_spirvgen_generate_store(HccCompiler* c, U32 dst_spirv_id, U32 src_spirv_id);
U32 hcc_spirvgen_generate_access_chain_single_field(HccCompiler* c, U32 base_spirv_id, HccDataType field_data_type, U32 field_idx);
void hcc_spirvgen_generate_intrinsic_input_variable(HccCompiler* c, U32 intrinsic_idx);
void hcc_spirvgen_generate(HccCompiler* c);

void hcc_spirvgen_write_word_many(FILE* f, U32* words, U32 words_count, char* path);
void hcc_spirvgen_write_word(FILE* f, U32 word, char* path);
void hcc_spirvgen_generate_binary(HccCompiler* c);

#endif

// ===========================================
//
//
// Constant Table
//
//
// ===========================================

typedef struct HccConstant HccConstant;
struct HccConstant {
	void* data;
	uint32_t size;
	HccDataType data_type;
};

typedef struct HccConstantEntry HccConstantEntry;
struct HccConstantEntry {
	uint32_t start_idx;
	uint32_t size;
	HccDataType data_type;
	HccStringId debug_string_id;
};

typedef struct HccConstantTable HccConstantTable;
struct HccConstantTable {
	void*             data;
	HccConstantEntry* entries;
	uint32_t          data_used_size;
	uint32_t          data_cap;
	uint32_t          entries_count;
	uint32_t          entries_cap;

	HccDataType       data_type;
	U32               fields_count;
	U32               fields_cap;
	HccConstantId*    data_write_ptr;
};

void hcc_constant_table_init(HccCompiler* c, uint32_t data_cap, uint32_t entries_cap);
HccConstantId hcc_constant_table_deduplicate_basic(HccCompiler* c, HccDataType data_type, HccBasic* basic);
void hcc_constant_table_deduplicate_composite_start(HccCompiler* c, HccDataType data_type);
void hcc_constant_table_deduplicate_composite_add(HccCompiler* c, HccConstantId constant_id);
HccConstantId hcc_constant_table_deduplicate_composite_end(HccCompiler* c);
HccConstantId hcc_constant_table_deduplicate_zero(HccCompiler* c, HccDataType data_type);
HccConstantId _hcc_constant_table_deduplicate_end(HccCompiler* c, HccDataType data_type, void* data, U32 data_size, U32 data_align, HccStringId debug_string_id);
HccConstant hcc_constant_table_get(HccCompiler* c, HccConstantId id);
void hcc_constant_print(HccCompiler* c, HccConstantId constant_id, FILE* f);
bool hcc_constant_as_uint(HccCompiler* c, HccConstant constant, U64* out);
bool hcc_constant_as_sint(HccCompiler* c, HccConstant constant, S64* out);
bool hcc_constant_as_sint32(HccCompiler* c, HccConstant constant, S32* out);
bool hcc_constant_as_float(HccCompiler* c, HccConstant constant, F64* out);

// ===========================================
//
//
// String Table
//
//
// ===========================================

typedef struct HccStringEntry HccStringEntry;
struct HccStringEntry {
	uint32_t start_idx;
	uint32_t size;
};

typedef struct HccStringTable HccStringTable;
struct HccStringTable {
	char*           data;
	HccStringEntry* entries;
	uint32_t        data_used_size;
	uint32_t        data_cap;
	uint32_t        entries_count;
	uint32_t        entries_cap;
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

	HCC_STRING_ID_PVEC_START,
#define HCC_STRING_ID_PVEC_END (HCC_STRING_ID_PVEC_START + HCC_VEC_COUNT)

	HCC_STRING_ID_VEC_START = HCC_STRING_ID_PVEC_END,
#define HCC_STRING_ID_VEC_END (HCC_STRING_ID_VEC_START + HCC_VEC_COUNT)

	HCC_STRING_ID_PMAT_START = HCC_STRING_ID_VEC_END,
#define HCC_STRING_ID_PMAT_END (HCC_STRING_ID_PMAT_START + HCC_MAT_COUNT)

	HCC_STRING_ID_MAT_START = HCC_STRING_ID_PMAT_END,
#define HCC_STRING_ID_MAT_END (HCC_STRING_ID_MAT_START + HCC_MAT_COUNT)

#define HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END HCC_STRING_ID_KEYWORDS_START

	HCC_STRING_ID_KEYWORDS_START = HCC_STRING_ID_MAT_END,
#define HCC_STRING_ID_KEYWORDS_END (HCC_STRING_ID_KEYWORDS_START + HCC_TOKEN_KEYWORDS_COUNT)

	HCC_STRING_ID_FUNCTION_IDXS_START = HCC_STRING_ID_KEYWORDS_END,
#define HCC_STRING_ID_FUNCTION_IDXS_END (HCC_STRING_ID_FUNCTION_IDXS_START + HCC_FUNCTION_IDX_INTRINSIC_END)

	HCC_STRING_ID_PREDEFINED_MACROS_START = HCC_STRING_ID_FUNCTION_IDXS_END,
#define HCC_STRING_ID_PREDEFINED_MACROS_END (HCC_STRING_ID_PREDEFINED_MACROS_START + HCC_PP_PREDEFINED_MACRO_COUNT)

	HCC_STRING_ID_ONCE = HCC_STRING_ID_PREDEFINED_MACROS_END,
	HCC_STRING_ID_DEFINED,
	HCC_STRING_ID___VA_ARGS__,
};

extern char* hcc_string_intrinsic_param_names[HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END];

void hcc_string_table_init(HccStringTable* string_table, U32 data_cap, U32 entries_cap);
#define hcc_string_table_deduplicate_lit(string_table, string_lit) hcc_string_table_deduplicate(string_table, string_lit, sizeof(string_lit) - 1)
#define hcc_string_table_deduplicate_c_string(string_table, c_string) hcc_string_table_deduplicate(string_table, c_string, strlen(c_string))
HccStringId hcc_string_table_deduplicate(HccStringTable* string_table, const char* string, U32 string_size);
HccString hcc_string_table_get(HccStringTable* string_table, HccStringId id);
HccString hcc_string_table_get_or_empty(HccStringTable* string_table, HccStringId id);

// ===========================================
//
//
// Compiler
//
//
// ===========================================

typedef U16 HccOption;
enum {
	HCC_OPTION_CONSTANT_FOLDING,
	HCC_OPTION_PP_UNDEF_EVAL,
	HCC_OPTION_PRINT_COLOR,

	HCC_OPTION_COUNT,
};

typedef struct HccOptions HccOptions;
struct HccOptions {
	U64 bitset[HCC_DIV_ROUND_UP(HCC_OPTION_COUNT, 8)];
};

typedef U8 HccCompilerStage;
enum {
	HCC_COMPILER_STAGE_TOKENGEN,
	HCC_COMPILER_STAGE_ASTGEN,
	HCC_COMPILER_STAGE_IRGEN,
	HCC_COMPILER_STAGE_SPIRVGEN,
};

typedef U32 HccCompilerFlags;
enum {
	HCC_COMPILER_FLAGS_NONE = 0x0,
	HCC_COMPILER_FLAGS_SET_LONG_JMP = 0x1,
	HCC_COMPILER_FLAGS_CHAR_IS_UNSIGNED = 0x2,
};

typedef U8 HccWorkerJobType;
enum {
	HCC_WORKER_JOB_TYPE_TOKENGEN;
	HCC_WORKER_JOB_TYPE_ASTGEN;
	HCC_WORKER_JOB_TYPE_ASTMERGE;
	HCC_WORKER_JOB_TYPE_IRGEN;
	HCC_WORKER_JOB_TYPE_IROPT;
};

typedef struct HccAst HccAst;
struct HccAst {
	HccStack(HccVariable)             function_params_and_variables;
	HccStack(HccFunction)             functions;
	HccStack(U32)                     function_used_function_indices;
	HccStack(HccDecl)                 function_used_static_variables;
	HccStack(U32)                     entry_point_function_indices;
	HccStack(HccExpr)                 exprs;
	HccStack(HccLocation)             expr_locations;
	HccStack(HccVariable)             global_variables;
	HccStack(HccArrayDataType)        array_data_types;
	HccStack(HccCompoundDataType)     compound_data_types;
	HccStack(HccCompoundField)        compound_fields;
	HccStack(HccTypedef)              typedefs;
	HccStack(HccEnumDataType)         enum_data_types;
	HccStack(HccEnumValue)            enum_values;
	HccStack(HccSimpleBufferDataType) simple_buffer_data_types;
	HccStack(HccPointerDataType)      pointer_data_types;
	HccStack(HccDataType)             ordered_data_types;
};

void hcc_ast_init(HccAst* ast, HccAstSetup* setup);
void hcc_ast_merge(HccAst* dst, HccAst* src);

typedef struct HccWorkerJob HccWorkerJob;
struct HccWorkerJob {
	HccWorkerJobType type;
};

typedef struct HccWorker HccWorker;
struct HccWorker {
	HccPP       pp;
	HccTokenGen tokengen;
	HccAstGen   astgen;
	HccIRGen    irgen;
	HccIROpt    iropt;

	HccCompilationUnit* cu;
	HccAst* ast;
};

struct HccCompiler {
	HccCompilerFlags      flags;
	HccArch               arch;
	HccOS                 os;
	HccGfxApi             gfx_api;
	HccResourceModel      resource_model;
	HccCompilerStage      stage;
	HccAllocTag           allocation_failure_alloc_tag;
	HccAllocTag           collection_is_full_alloc_tag;
	U8                    max_resource_set_slot;
	U32                   max_resource_constants_size;
	jmp_buf               compile_entry_jmp_loc;

	HccStringTable        string_table;
	HccConstantTable      constant_table;
	U8*                   basic_type_size_and_aligns; // [HCC_DATA_TYPE_BASIC_COUNT]
	U64*                  basic_type_int_mins; // [HCC_DATA_TYPE_BASIC_COUNT]
	U64*                  basic_type_int_maxes; // [HCC_DATA_TYPE_BASIC_COUNT]
	HccConstantId         basic_type_zero_constant_ids[HCC_DATA_TYPE_BASIC_COUNT];
	HccConstantId         basic_type_one_constant_ids[HCC_DATA_TYPE_BASIC_COUNT];
	HccConstantId         basic_type_minus_one_constant_ids[HCC_DATA_TYPE_BASIC_COUNT];
	HccOptions            options;
	HccMessageSys         message_sys;
	HccStack(char)        string_buffer;
	HccStack(HccString)   include_paths;
	HccStack(HccCodeFile) code_files;
	HccHashTable(HccStringId, HccCodeFileId) path_to_code_file_id_map;
	U32                   code_file_lines_cap;
	U32                   code_file_pp_if_spans_cap;
	HccCodeFile*          concat_buffer_code_file;

	HccPP                 pp;
	HccTokenGen           tokengen;
	HccAstGen             astgen;
	HccIRGen              irgen;
	HccIROpt              iropt;
	//HccSpirvGen           spirvgen;
};

typedef struct HccCompilerSetup HccCompilerSetup;
struct HccCompilerSetup {
	HccPPSetup       pp;
	HccTokenGenSetup tokengen;
	HccAstGenSetup   astgen;
	HccIRGenSetup    irgen;
	HccIROptSetup    iropt;
	//HccSpirvGenSetup spirvgen;

	U32 messages_cap;
	U32 message_strings_cap;
	U32 string_table_data_cap;
	U32 string_table_entries_cap;
	U32 string_buffer_cap;
	U32 include_paths_cap;
	U32 code_files_cap;
	U32 code_file_lines_cap;
	U32 code_file_pp_if_spans_cap;
};

extern HccCompilerSetup hcc_compiler_setup_default;

bool hcc_compiler_init(HccCompiler* c, HccCompilerSetup* setup);
bool hcc_compiler_compile(HccCompiler* c, char* file_path);
noreturn void hcc_compiler_bail(HccCompiler* c);
noreturn void hcc_compiler_bail_allocation_failure(HccCompiler* c, HccAllocTag tag);
noreturn void hcc_compiler_bail_collection_is_full(HccCompiler* c, HccAllocTag tag);

bool hcc_options_is_enabled(HccCompiler* c, HccOption opt);
void hcc_options_set_enabled(HccCompiler* c, HccOption opt);
void hcc_options_set_disabled(HccCompiler* c, HccOption opt);

