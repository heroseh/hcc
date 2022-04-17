
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

// ===========================================
//
//
// General
//
//
// ===========================================

#ifndef HCC_ALLOC
#define HCC_ALLOC(size, align) malloc(size)
#endif

#ifndef HCC_DEALLOC
#define HCC_DEALLOC(ptr, size, align) free(ptr)
#endif

#define HCC_ALLOC_ELMT(T) HCC_ALLOC(sizeof(T), alignof(T))
#define HCC_DEALLOC_ELMT(T, ptr) HCC_DEALLOC(ptr, sizeof(T), alignof(T))
#define HCC_ALLOC_ARRAY(T, count) HCC_ALLOC((count) * sizeof(T), alignof(T))
#define HCC_DEALLOC_ARRAY(T, ptr, count) HCC_DEALLOC(ptr, (count) * sizeof(T), alignof(T))

#ifndef HCC_STATIC_ASSERT
#define HCC_STATIC_ASSERT(x, msg) int hcc_sa(int hcc_sa[(x)?1:-1])
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

#define HCC_ASSERT_ARRAY_BOUNDS(idx, count) HCC_ASSERT((idx) < (count), "idx '%zu' is out of bounds for an array of count '%zu'", (idx), (count));

#define HCC_STRINGIFY(v) #v
#define HCC_CONCAT_0(a, b) a##b
#define HCC_CONCAT(a, b) HCC_CONCAT_0(a, b)

#if HCC_DEBUG_ASSERTIONS
#define HCC_DEBUG_ASSERT HCC_ASSERT
#else
#define HCC_DEBUG_ASSERT(cond, ...) (void)(cond)
#endif

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

#ifdef __GNUC__
#define HCC_NORETURN __attribute__((noreturn))
#else
#define HCC_NORETURN
#endif

#define HCC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HCC_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define HCC_UNUSED(expr) ((void)(expr))

#ifndef alignof
#define alignof _Alignof
#endif

void _hcc_assert_failed(const char* cond, const char* file, int line, const char* message, ...);
HCC_NORETURN void _hcc_abort(const char* file, int line, const char* message, ...);

// align must be a power of 2
#define HCC_INT_ROUND_UP_ALIGN(i, align) (((i) + ((align) - 1)) & ~((align) - 1))
// align must be a power of 2
#define HCC_INT_ROUND_DOWN_ALIGN(i, align) ((i) & ~((align) - 1))

#define HCC_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
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

#define HCC_DEFINE_ID(Name) typedef struct Name { uint32_t idx_plus_one; } Name
HCC_DEFINE_ID(HccCodeFileId);
HCC_DEFINE_ID(HccStringId);
HCC_DEFINE_ID(HccConstantId);
HCC_DEFINE_ID(HccFunctionTypeId);
HCC_DEFINE_ID(HccExprId);

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

typedef struct HccCompiler HccCompiler;

typedef struct HccLocation HccLocation;
struct HccLocation {
	U32       span_idx;
	U32       parent_location_idx;
	U32       code_start_idx;
	U32       code_end_idx;
	U32       line_start;
	U32       line_end;
	U32       column_start;
	U32       column_end;
};

#define HCC_COMPOUND_TYPE_NESTED_FIELD_CAP 16

// ===========================================
//
//
// Platform Abstraction
//
//
// ===========================================

void hcc_get_last_system_error_string(char* buf_out, U32 buf_out_size);
bool hcc_file_exist(char* path);
U8* hcc_file_read_all_the_code(char* path, U64* size_out);

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

void hcc_hash_table_init(HccHashTable* hash_table);
bool hcc_hash_table_find(HccHashTable* hash_table, U32 key, U32* value_out);
bool hcc_hash_table_find_or_insert(HccHashTable* hash_table, U32 key, U32** value_ptr_out);
bool hcc_hash_table_remove(HccHashTable* hash_table, U32 key, U32* value_out);
void hcc_hash_table_clear(HccHashTable* hash_table);

// ===========================================
//
//
// Syntax Generator
//
//
// ===========================================

typedef U32 HccDataType;
enum {
#define HCC_DATA_TYPE_BASIC_START HCC_DATA_TYPE_VOID
	HCC_DATA_TYPE_VOID,
	HCC_DATA_TYPE_BOOL,
	HCC_DATA_TYPE_U8,
	HCC_DATA_TYPE_U16,
	HCC_DATA_TYPE_U32,
	HCC_DATA_TYPE_U64,
	HCC_DATA_TYPE_S8,
	HCC_DATA_TYPE_S16,
	HCC_DATA_TYPE_S32,
	HCC_DATA_TYPE_S64,
	HCC_DATA_TYPE_F16,
	HCC_DATA_TYPE_F32,
	HCC_DATA_TYPE_F64,
#define HCC_DATA_TYPE_BASIC_END (HCC_DATA_TYPE_F64 + 1)
#define HCC_DATA_TYPE_BASIC_COUNT (HCC_DATA_TYPE_BASIC_END - HCC_DATA_TYPE_BASIC_START)

#define HCC_DATA_TYPE_VECTOR_START HCC_DATA_TYPE_VEC2_START
	HCC_DATA_TYPE_VEC2_START = 16,
#define HCC_DATA_TYPE_VEC2_END HCC_DATA_TYPE_VEC3_START
	HCC_DATA_TYPE_VEC3_START = HCC_DATA_TYPE_VEC2_START + HCC_DATA_TYPE_VEC2_START,
#define HCC_DATA_TYPE_VEC3_END HCC_DATA_TYPE_VEC4_START
	HCC_DATA_TYPE_VEC4_START = HCC_DATA_TYPE_VEC3_START + HCC_DATA_TYPE_VEC2_START,
#define HCC_DATA_TYPE_VEC4_END HCC_DATA_TYPE_MAT2x2_START
#define HCC_DATA_TYPE_VECTOR_END HCC_DATA_TYPE_MAT2x2_START
#define HCC_DATA_TYPE_MATRIX_START HCC_DATA_TYPE_MAT2x2_START
	HCC_DATA_TYPE_MAT2x2_START = HCC_DATA_TYPE_VEC4_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT2x3_START = HCC_DATA_TYPE_MAT2x2_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT2x4_START = HCC_DATA_TYPE_MAT2x3_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT3x2_START = HCC_DATA_TYPE_MAT2x4_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT3x3_START = HCC_DATA_TYPE_MAT3x2_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT3x4_START = HCC_DATA_TYPE_MAT3x3_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT4x2_START = HCC_DATA_TYPE_MAT3x4_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT4x3_START = HCC_DATA_TYPE_MAT4x2_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_MAT4x4_START = HCC_DATA_TYPE_MAT4x3_START + HCC_DATA_TYPE_VEC2_START,
#define HCC_DATA_TYPE_MATRIX_END HCC_DATA_TYPE_STRUCT
	HCC_DATA_TYPE_ENUM = HCC_DATA_TYPE_MAT4x4_START + HCC_DATA_TYPE_VEC2_START,
	HCC_DATA_TYPE_STRUCT,
	HCC_DATA_TYPE_UNION,
	HCC_DATA_TYPE_ARRAY,
	HCC_DATA_TYPE_TYPEDEF,

#define HCC_DATA_TYPE_GENERIC_START HCC_DATA_TYPE_GENERIC_SCALAR
	HCC_DATA_TYPE_GENERIC_SCALAR,
	HCC_DATA_TYPE_GENERIC_VEC2,
	HCC_DATA_TYPE_GENERIC_VEC3,
	HCC_DATA_TYPE_GENERIC_VEC4,
#define HCC_DATA_TYPE_GENERIC_END (HCC_DATA_TYPE_GENERIC_VEC4 + 1)

	HCC_DATA_TYPE_COUNT,
};

#define HCC_DATA_TYPE_SCALAR(type)                ((type) & (HCC_DATA_TYPE_VEC2_START - 1))
#define HCC_DATA_TYPE_IS_BASIC(type)              ((type) < HCC_DATA_TYPE_BASIC_END)
#define HCC_DATA_TYPE_IS_NON_VOID_BASIC(type)     ((type) > HCC_DATA_TYPE_VOID && (type) < HCC_DATA_TYPE_BASIC_END)
#define HCC_DATA_TYPE_IS_INT(type)                ((type) >= HCC_DATA_TYPE_U8 && (type) <= HCC_DATA_TYPE_S64)
#define HCC_DATA_TYPE_IS_UINT(type)               ((type) >= HCC_DATA_TYPE_U8 && (type) <= HCC_DATA_TYPE_U64)
#define HCC_DATA_TYPE_IS_SINT(type)               ((type) >= HCC_DATA_TYPE_S8 && (type) <= HCC_DATA_TYPE_S64)
#define HCC_DATA_TYPE_IS_FLOAT(type)              ((type) >= HCC_DATA_TYPE_F16 && (type) <= HCC_DATA_TYPE_F64)
#define HCC_DATA_TYPE_IS_VECTOR(type)             ((type) >= HCC_DATA_TYPE_VECTOR_START && (type) < HCC_DATA_TYPE_VECTOR_END)
#define HCC_DATA_TYPE_IS_MATRIX(type)             ((type) >= HCC_DATA_TYPE_MATRIX_START && (type) < HCC_DATA_TYPE_MATRIX_END)
#define HCC_DATA_TYPE_IS_STRUCT(type)             (((type) & 0xff) == HCC_DATA_TYPE_STRUCT)
#define HCC_DATA_TYPE_IS_UNION(type)              (((type) & 0xff) == HCC_DATA_TYPE_UNION)
#define HCC_DATA_TYPE_IS_COMPOUND_TYPE(type)      (HCC_DATA_TYPE_IS_STRUCT(type) || HCC_DATA_TYPE_IS_UNION(type))
#define HCC_DATA_TYPE_IS_ARRAY(type)              (((type) & 0xff) == HCC_DATA_TYPE_ARRAY)
#define HCC_DATA_TYPE_IS_COMPOSITE_TYPE(type)     (HCC_DATA_TYPE_IS_STRUCT(type) || HCC_DATA_TYPE_IS_UNION(type) || HCC_DATA_TYPE_IS_ARRAY(type) || HCC_DATA_TYPE_IS_VECTOR(type) || HCC_DATA_TYPE_IS_MATRIX(type))
#define HCC_DATA_TYPE_IS_TYPEDEF(type)            (((type) & 0xff) == HCC_DATA_TYPE_TYPEDEF)
#define HCC_DATA_TYPE_IS_ENUM_TYPE(type)          (((type) & 0xff) == HCC_DATA_TYPE_ENUM)
#define HCC_DATA_TYPE_IS_GENERIC(type)            ((type) >= HCC_DATA_TYPE_GENERIC_START && (type) < HCC_DATA_TYPE_GENERIC_END)
#define HCC_DATA_TYPE_VECTOR_COMPONENTS(type)     (((type) / HCC_DATA_TYPE_VEC2_START) + 1)
#define HCC_DATA_TYPE_MATRX_COLUMNS(type)         (((type) + 32) / 48)
#define HCC_DATA_TYPE_MATRX_ROWS(type)            ((((((type) - 64) / 16) + 1) & 3) + 2)
#define HCC_DATA_TYPE_INIT(type, idx)             (((idx) << 8) | (type))
#define HCC_DATA_TYPE_IS_CONST(type)              (!!((type) & HCC_DATA_TYPE_CONST_MASK))
#define HCC_DATA_TYPE_IDX(type)                   (((type) & HCC_DATA_TYPE_IDX_MASK) >> 8)
#define HCC_DATA_TYPE_CONST(type)                 ((type) | HCC_DATA_TYPE_CONST_MASK)
#define HCC_DATA_TYPE_STRIP_CONST(type)           ((type) & ~HCC_DATA_TYPE_CONST_MASK)
#define HCC_DATA_TYPE_CONST_MASK                  0x80000000
#define HCC_DATA_TYPE_IDX_MASK                    0x7fffff00

//
// 'basic_type' must be HCC_DATA_TYPE_IS_BASIC(basic_type) == true
#define HCC_DATA_TYPE_VEC2(basic_type)   (HCC_DATA_TYPE_VEC2_START + (basic_type))
#define HCC_DATA_TYPE_VEC3(basic_type)   (HCC_DATA_TYPE_VEC3_START + (basic_type))
#define HCC_DATA_TYPE_VEC4(basic_type)   (HCC_DATA_TYPE_VEC4_START + (basic_type))
#define HCC_DATA_TYPE_MAT2x2(basic_type) (HCC_DATA_TYPE_MAT2x2_START + (basic_type))
#define HCC_DATA_TYPE_MAT2x3(basic_type) (HCC_DATA_TYPE_MAT2x3_START + (basic_type))
#define HCC_DATA_TYPE_MAT2x4(basic_type) (HCC_DATA_TYPE_MAT2x4_START + (basic_type))
#define HCC_DATA_TYPE_MAT3x2(basic_type) (HCC_DATA_TYPE_MAT3x2_START + (basic_type))
#define HCC_DATA_TYPE_MAT3x3(basic_type) (HCC_DATA_TYPE_MAT3x3_START + (basic_type))
#define HCC_DATA_TYPE_MAT3x4(basic_type) (HCC_DATA_TYPE_MAT3x4_START + (basic_type))
#define HCC_DATA_TYPE_MAT4x2(basic_type) (HCC_DATA_TYPE_MAT4x2_START + (basic_type))
#define HCC_DATA_TYPE_MAT4x3(basic_type) (HCC_DATA_TYPE_MAT4x3_START + (basic_type))
#define HCC_DATA_TYPE_MAT4x4(basic_type) (HCC_DATA_TYPE_MAT4x4_START + (basic_type))

typedef struct HccArrayDataType HccArrayDataType;
struct HccArrayDataType {
	HccDataType element_data_type;
	HccConstantId size_constant_id;
};

typedef U16 HccCompoundDataTypeFlags;
enum {
	HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION = 0x1,
};

typedef struct HccCompoundDataType HccCompoundDataType;
struct HccCompoundDataType {
	Uptr                     size;
	Uptr                     align;
	U32                      identifier_token_idx;
	HccStringId              identifier_string_id;
	U32                      fields_start_idx;
	U16                      fields_count;
	U16                      largest_sized_field_idx;
	HccCompoundDataTypeFlags flags;
};

typedef struct HccCompoundField HccCompoundField;
struct HccCompoundField {
	HccStringId identifier_string_id;
	HccDataType data_type;
	U32         identifier_token_idx;
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

typedef U8 HccToken;
enum {
#define HCC_TOKEN_INTRINSIC_TYPES_START HCC_DATA_TYPE_BASIC_START
	//
	// INFO:
	// HCC_DATA_TYPE_BASIC_START - HCC_DATA_TYPE_BASIC_END are used as HccToken too!
	//
#define HCC_TOKEN_INTRINSIC_TYPE_VECTORS_START HCC_TOKEN_INTRINSIC_TYPE_VEC2
	HCC_TOKEN_INTRINSIC_TYPE_VEC2 = HCC_DATA_TYPE_BASIC_END,
	HCC_TOKEN_INTRINSIC_TYPE_VEC3,
	HCC_TOKEN_INTRINSIC_TYPE_VEC4,
#define HCC_TOKEN_INTRINSIC_TYPE_VECTORS_END HCC_TOKEN_INTRINSIC_TYPE_MAT2X2
#define HCC_TOKEN_INTRINSIC_TYPE_MATRICES_START HCC_TOKEN_INTRINSIC_TYPE_MAT2X2
	HCC_TOKEN_INTRINSIC_TYPE_MAT2X2,
	HCC_TOKEN_INTRINSIC_TYPE_MAT2X3,
	HCC_TOKEN_INTRINSIC_TYPE_MAT2X4,
	HCC_TOKEN_INTRINSIC_TYPE_MAT3X2,
	HCC_TOKEN_INTRINSIC_TYPE_MAT3X3,
	HCC_TOKEN_INTRINSIC_TYPE_MAT3X4,
	HCC_TOKEN_INTRINSIC_TYPE_MAT4X2,
	HCC_TOKEN_INTRINSIC_TYPE_MAT4X3,
	HCC_TOKEN_INTRINSIC_TYPE_MAT4X4,
#define HCC_TOKEN_INTRINSIC_TYPE_MATRICES_END HCC_TOKEN_EOF
#define HCC_TOKEN_INTRINSIC_TYPES_END HCC_TOKEN_EOF
#define HCC_TOKEN_INTRINSIC_TYPES_COUNT (HCC_TOKEN_INTRINSIC_TYPES_END - HCC_TOKEN_INTRINSIC_TYPES_START)

	HCC_TOKEN_EOF,
	HCC_TOKEN_IDENT,
	HCC_TOKEN_STRING,
	HCC_TOKEN_INCLUDE_PATH_SYSTEM,

	//
	// symbols
	//
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

	HCC_TOKEN_LIT_U32,
	HCC_TOKEN_LIT_U64,
	HCC_TOKEN_LIT_S32,
	HCC_TOKEN_LIT_S64,
	HCC_TOKEN_LIT_F32,
	HCC_TOKEN_LIT_F64,

	//
	// keywords
	//
#define HCC_TOKEN_KEYWORDS_START HCC_TOKEN_KEYWORD_RETURN
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
	HCC_TOKEN_KEYWORD_RO_BUFFER,
	HCC_TOKEN_KEYWORD_RW_BUFFER,
	HCC_TOKEN_KEYWORD_RO_IMAGE1D,
	HCC_TOKEN_KEYWORD_RW_IMAGE1D,
	HCC_TOKEN_KEYWORD_RO_IMAGE2D,
	HCC_TOKEN_KEYWORD_RW_IMAGE2D,
	HCC_TOKEN_KEYWORD_RO_IMAGE3D,
	HCC_TOKEN_KEYWORD_RW_IMAGE3D,
#define HCC_TOKEN_KEYWORDS_END HCC_TOKEN_COUNT
#define HCC_TOKEN_KEYWORDS_COUNT (HCC_TOKEN_KEYWORDS_END - HCC_TOKEN_KEYWORDS_START)
#define HCC_TOKEN_IS_KEYWORD(token) (HCC_TOKEN_KEYWORDS_START <= (token) && (token) < HCC_TOKEN_KEYWORDS_END)

	HCC_TOKEN_COUNT,
};

#define HCC_TOKEN_IS_BASIC_TYPE(token) ((token) < HCC_DATA_TYPE_BASIC_END)

enum {
	HCC_STRING_ID_NULL = 0,

#define HCC_STRING_ID_INTRINSIC_PARAM_NAMES_START HCC_STRING_ID_GENERIC_SCALAR
	HCC_STRING_ID_GENERIC_SCALAR,
	HCC_STRING_ID_GENERIC_VEC2,
	HCC_STRING_ID_GENERIC_VEC3,
	HCC_STRING_ID_GENERIC_VEC4,
	HCC_STRING_ID_SCALAR,
	HCC_STRING_ID_X,
	HCC_STRING_ID_Y,
	HCC_STRING_ID_Z,
	HCC_STRING_ID_W,
#define HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END HCC_STRING_ID_KEYWORDS_START

	HCC_STRING_ID_KEYWORDS_START,
#define HCC_STRING_ID_KEYWORDS_END HCC_STRING_ID_INTRINSIC_TYPES_START

	HCC_STRING_ID_INTRINSIC_TYPES_START = HCC_STRING_ID_KEYWORDS_START + HCC_TOKEN_KEYWORDS_COUNT,
	HCC_STRING_ID_INTRINSIC_TYPES_END = HCC_STRING_ID_INTRINSIC_TYPES_START + HCC_TOKEN_INTRINSIC_TYPES_COUNT,
};

extern char* hcc_string_intrinsic_param_names[HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END];

typedef union HccTokenValue HccTokenValue;
union HccTokenValue {
	HccConstantId constant_id;
	HccStringId string_id;
};

extern char* hcc_token_strings[HCC_TOKEN_COUNT];

typedef struct HccString HccString;
struct HccString {
	char* data;
	uintptr_t size;
};
#define hcc_string(data, size) ((HccString) { data, size });
#define hcc_string_lit(lit) ((HccString) { lit, sizeof(lit) - 1 });
#define hcc_string_c(string) ((HccString) { string, strlen(string) });
#define hcc_string_eq(a, b) ((a).size == (b).size && memcmp((a).data, (b).data, (a).size) == 0)
#define hcc_string_eq_c(a, c_string) ((a).size == strlen(c_string) && memcmp((a).data, c_string, (a).size) == 0)
#define hcc_string_eq_lit(a, lit) ((a).size == sizeof(lit) - 1 && memcmp((a).data, lit, (a).size) == 0)
static inline HccString hcc_string_slice_start(HccString string, Uptr start) {
	HCC_ASSERT_ARRAY_BOUNDS(start, string.size + 1);
	return hcc_string(string.data + start, string.size - start);
}
static inline HccString hcc_string_slice_end(HccString string, Uptr end) {
	HCC_ASSERT_ARRAY_BOUNDS(end, string.size + 1);
	return hcc_string(string.data, end);
}
static inline HccString hcc_string_slice(HccString string, Uptr start, Uptr end) {
	HCC_ASSERT_ARRAY_BOUNDS(start, string.size + 1);
	HCC_ASSERT_ARRAY_BOUNDS(end, string.size + 1);
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

typedef struct HccVariable HccVariable;
struct HccVariable {
	HccStringId   identifier_string_id;
	HccDataType   data_type;
	HccConstantId initializer_constant_id; // if is_static
	U32           identifier_token_idx: 30;
	U32           is_static: 1;
	U32           is_const: 1;
};

typedef struct HccMacro HccMacro;
struct HccMacro {
	HccStringId identifier_string_id;
	HccString value_string;
	HccLocation location;
	U32 value_string_column_start;
	U32 params_start_idx: 23;
	U32 params_count: 8;
	U32 is_function: 1;
};

typedef U32 HccPredefinedMacro;
enum {
	HCC_PREDEFINED_MACRO___FILE__,
	HCC_PREDEFINED_MACRO___LINE__,
	HCC_PREDEFINED_MACRO___COUNTER__,
	HCC_PREDEFINED_MACRO___HCC__,

	HCC_PREDEFINED_MACRO_COUNT,
};

typedef U8 HccFunctionShaderStage;
enum {
	HCC_FUNCTION_SHADER_STAGE_NONE,
	HCC_FUNCTION_SHADER_STAGE_VERTEX,
	HCC_FUNCTION_SHADER_STAGE_FRAGMENT,
	HCC_FUNCTION_SHADER_STAGE_GEOMETRY,
	HCC_FUNCTION_SHADER_STAGE_TESSELLATION,
	HCC_FUNCTION_SHADER_STAGE_COMPUTE,
	HCC_FUNCTION_SHADER_STAGE_MESHTASK,

	HCC_FUNCTION_SHADER_STAGE_COUNT,
};
extern char* hcc_function_shader_stage_strings[HCC_FUNCTION_SHADER_STAGE_COUNT];

typedef struct HccFunction HccFunction;
struct HccFunction {
	U32                    identifier_token_idx;
	HccStringId            identifier_string_id;
	HccDataType            return_data_type;
	U32                    params_start_idx;
	U16                    variables_count;
	U8                     params_count;
	HccFunctionShaderStage shader_stage;
	HccExprId              block_expr_id;
	U32                    used_static_variables_start_idx;
	U32                    used_static_variables_count;
};

enum {
	HCC_FUNCTION_IDX_VEC2,
	HCC_FUNCTION_IDX_VEC3,
	HCC_FUNCTION_IDX_VEC4,
	HCC_FUNCTION_IDX_MAT2x2,
	HCC_FUNCTION_IDX_MAT2x3,
	HCC_FUNCTION_IDX_MAT2x4,
	HCC_FUNCTION_IDX_MAT3x2,
	HCC_FUNCTION_IDX_MAT3x3,
	HCC_FUNCTION_IDX_MAT3x4,
	HCC_FUNCTION_IDX_MAT4x2,
	HCC_FUNCTION_IDX_MAT4x3,
	HCC_FUNCTION_IDX_MAT4x4,

#define HCC_FUNCTION_IDX_INTRINSIC_END HCC_FUNCTION_IDX_USER_START
	HCC_FUNCTION_IDX_USER_START,
};

typedef struct HccIntrinsicFunction HccIntrinsicFunction;
struct HccIntrinsicFunction {
	char* name;
	HccDataType return_data_type;
	U32 params_count;
	HccVariable params[16];
};

extern HccIntrinsicFunction hcc_intrinsic_functions[HCC_FUNCTION_IDX_INTRINSIC_END];

enum {
	HCC_INTRINSIC_FUNCTION_IDX_USER_START,
};

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

	HCC_BINARY_OP_COUNT,
};

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

	HCC_UNARY_OP_COUNT,
};

typedef U8 HccExprType;
enum {
	HCC_EXPR_TYPE_NONE,

	//
	// binary ops
	HCC_EXPR_TYPE_BINARY_OP_START,
#define HCC_EXPR_TYPE_BINARY_OP(OP) (HCC_EXPR_TYPE_BINARY_OP_START + HCC_BINARY_OP_##OP)
	HCC_EXPR_TYPE_BINARY_OP_END = HCC_EXPR_TYPE_BINARY_OP_START + HCC_BINARY_OP_COUNT,

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

	//
	// unary ops
	HCC_EXPR_TYPE_UNARY_OP_START,
#define HCC_EXPR_TYPE_UNARY_OP(OP) (HCC_EXPR_TYPE_UNARY_OP_START + HCC_UNARY_OP_##OP)
	HCC_EXPR_TYPE_UNARY_OP_END = HCC_EXPR_TYPE_UNARY_OP_START + HCC_UNARY_OP_COUNT,

	HCC_EXPR_TYPE_STMT_RETURN,
	HCC_EXPR_TYPE_STMT_BLOCK,
};

typedef struct HccExpr HccExpr;
struct HccExpr {
	union {
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
		};
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 idx: 25;
		} function;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 expr_rel_idx: 25;
		} unary;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 is_assignment: 1;
			U32 left_expr_rel_idx: 11;
			U32 right_expr_rel_idx: 13;
		} binary;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 id: 25;
		} constant;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 has_return_stmt: 1;
			U32 stmts_count: 11;
			U32 first_expr_rel_idx: 6;
			U32 variables_count: 6;
		} stmt_block;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 first_expr_rel_idx: 25;
		} curly_initializer;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 entry_indices_count: 15;
			U32 value_expr_rel_idx: 10;
			// alt_next_expr_rel_idx is the entry_indices_start_idx
		} designated_initializer;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 12;
			U32 true_stmt_rel_idx: 13;
		} if_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 12;
			U32 block_expr_rel_idx: 13;
			// alt_next_expr_rel_idx is the default_case_expr_rel_idx
		} switch_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 12;
			U32 loop_stmt_rel_idx: 13;
		} while_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 init_expr_rel_idx: 5;
			U32 cond_expr_rel_idx: 6;
			U32 inc_expr_rel_idx: 7;
			U32 loop_stmt_rel_idx: 7;
		} for_;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 idx: 25;
		} variable;
		struct {
			U32 type: 6; // HccExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 6;
			U32 true_expr_rel_idx: 9;
			U32 false_expr_rel_idx: 10;
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

HCC_STATIC_ASSERT(sizeof(HccExpr) == sizeof(U64), "HccExpr must be 8 bytes");

typedef U16 HccOpt;
enum {
	HCC_OPT_CONSTANT_FOLDING,
	HCC_OPT_PP_UNDEF_EVAL,

	HCC_OPT_COUNT,
};

typedef struct HccOpts HccOpts;
struct HccOpts {
	U64 bitset[HCC_DIV_ROUND_UP(HCC_OPT_COUNT, 8)];
};

typedef struct HccGenericDataTypeState HccGenericDataTypeState;
struct HccGenericDataTypeState {
	HccDataType scalar;
	HccDataType vec2;
	HccDataType vec3;
	HccDataType vec4;
};

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

typedef struct HccCurlyInitializerGen HccCurlyInitializerGen;
struct HccCurlyInitializerGen {
	union {
		HccCompoundDataType* compound_data_type;
		HccArrayDataType* array_data_type;
	};
	HccCompoundField* compound_fields;
	U64 entries_cap;
	HccDataType composite_data_type;
	HccDataType entry_data_type;
	HccDataType resolved_composite_data_type;
	HccDataType resolved_entry_data_type;

	U64* entry_indices;
	HccDataType* data_types;
	bool* found_designators;
	U32 entry_indices_count;
	U32 entry_indices_cap;

	U32* nested_designators_start_entry_indices;
	U32 nested_designators_count;
	U32 nested_designators_cap;

	HccExpr* prev_initializer_expr;
	HccExpr* first_initializer_expr;
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
	HCC_PP_DIRECTIVE_PRAGMA,

	HCC_PP_DIRECTIVE_COUNT,
};

typedef struct HccPPIfSpan HccPPIfSpan;
struct HccPPIfSpan {
	HccPPDirective directive;
	bool           has_else;
	HccLocation    location;
};

typedef U32 HccCodeFileFlags;
enum {
	HCC_CODE_FILE_FLAGS_COMPILATION_UNIT = 0x1,
	HCC_CODE_FILE_FLAGS_INCLUDE_GUARD = 0x2,
};

typedef struct HccCodeFile HccCodeFile;
struct HccCodeFile {
	HccCodeFileFlags flags;
	HccString path_string;
	U8* code;
	U64 code_size;
};

typedef struct HccCodeSpan HccCodeSpan;
struct HccCodeSpan {
	U8*            code;
	U32            code_size;
	HccMacro*      macro; // NULL when this is a file expansion
	U32            macro_args_start_idx;
	HccCodeFile*   code_file;
	HccLocation    location;
	U32*           line_code_start_indices;
	U32            lines_count;
	U32            lines_cap;
	U32            macro_arg_id;
	bool           is_preprocessor_expression;
};

typedef U16 HccAstGenFlags;
enum {
	HCC_ASTGEN_FLAGS_FOUND_STATIC = 0x1,
	HCC_ASTGEN_FLAGS_FOUND_CONST = 0x2,
	HCC_ASTGEN_FLAGS_FOUND_INLINE = 0x4,
	HCC_ASTGEN_FLAGS_FOUND_NO_RETURN = 0x8,
};

typedef struct HccMacroArg HccMacroArg;
struct HccMacroArg {
	U32 callsite_location_idx;
	HccString string;
};

#define _HCC_TOKENIZER_NESTED_BRACKETS_CAP 32

typedef struct HccAstGen HccAstGen;
struct HccAstGen {
	HccAstGenFlags flags;

	HccHashTable(HccStringId, HccCodeFileId) path_to_code_file_id_map;
	HccCodeFile* code_files;
	U32 code_files_count;
	U32 code_files_cap;

	HccPPIfSpan* pp_if_spans_stack;
	U32 pp_if_spans_stack_count;
	U32 pp_if_spans_stack_cap;

	U32* code_span_stack;
	U32 code_span_stack_count;
	U32 code_span_stack_cap;

	HccCodeSpan* code_spans;
	U32 code_spans_count;
	U32 code_spans_cap;

	HccCodeSpan* span;

	HccVariable* function_params_and_variables;
	HccFunction*      functions;
	HccExpr*          exprs;
	HccLocation*      expr_locations;
	U32 function_params_and_variables_count;
	U32 function_params_and_variables_cap;
	U32 functions_count;
	U32 functions_cap;
	U32 exprs_count;
	U32 exprs_cap;

	HccVariable* global_variables;
	U32 global_variables_count;
	U32 global_variables_cap;

	HccDecl* used_static_variables;
	U32 used_static_variables_count;
	U32 used_static_variables_cap;

	HccArrayDataType* array_data_types;
	U32 array_data_types_count;
	U32 array_data_types_cap;

	HccCompoundDataType* compound_data_types;
	U32 compound_data_types_count;
	U32 compound_data_types_cap;
	HccCompoundField* compound_fields;
	U32 compound_fields_count;
	U32 compound_fields_cap;

	HccTypedef* typedefs;
	U32 typedefs_count;
	U32 typedefs_cap;

	HccHashTable(HccStringId, U32) macro_declarations;
	HccMacro* macros;
	U32 macros_count;
	U32 macros_cap;

	HccStringId* macro_params;
	U32 macro_params_count;
	U32 macro_params_cap;

	HccMacroArg* macro_args;
	U32 macro_args_count;
	U32 macro_args_cap;

	HccEnumDataType* enum_data_types;
	U32 enum_data_types_count;
	U32 enum_data_types_cap;
	HccEnumValue* enum_values;
	U32 enum_values_count;
	U32 enum_values_cap;

	HccDataType* ordered_data_types;
	U32 ordered_data_types_count;
	U32 ordered_data_types_cap;

	HccCurlyInitializerGen curly_initializer_gen;

	U32* field_indices;
	U32 field_indices_count;
	U32 field_indices_cap;

	U64* entry_indices;
	U32 entry_indices_count;
	U32 entry_indices_cap;

	char* macro_paste_buffer;
	U32 macro_paste_buffer_size;
	U32 macro_paste_buffer_cap;

	HccDataType assign_data_type;

	HccFieldAccess compound_type_find_fields[HCC_COMPOUND_TYPE_NESTED_FIELD_CAP];
	U16 compound_type_find_fields_count;

	HccExpr* stmt_block;
	HccFunction* print_function;
	U32 print_variable_base_idx;

	HccSwitchState switch_state;
	bool is_in_loop;

	HccGenericDataTypeState generic_data_type_state;

	HccOpts opts;

	HccStringId* variable_stack_strings;
	U32*         variable_stack_var_indices;
	U32          variable_stack_count;
	U32          variable_stack_cap;
	U32          next_var_idx;

	HccHashTable(HccStringId, HccDecl) global_declarations;
	HccHashTable(HccStringId, HccDataType) struct_declarations;
	HccHashTable(HccStringId, HccDataType) union_declarations;
	HccHashTable(HccStringId, HccDataType) enum_declarations;

	HccHashTable(HccStringId, U32) field_name_to_token_idx;

	char* error_info;

	char* string_buffer;
	U32 string_buffer_size;
	U32 string_buffer_cap;

	HccToken* tokens;
	U32*      token_location_indices;

	HccLocation* token_locations;
	U32 token_locations_count;
	U32 token_locations_cap;

	HccTokenValue* token_values;
	HccStringTable string_table;
	HccConstantTable constant_table;
	HccConstantId basic_type_zero_constant_ids[HCC_DATA_TYPE_BASIC_COUNT];
	HccConstantId basic_type_one_constant_ids[HCC_DATA_TYPE_BASIC_COUNT];
	uint32_t token_read_idx;
	uint32_t token_value_read_idx;
	uint32_t tokens_count;
	uint32_t tokens_cap;
	uint32_t token_values_count;
	uint32_t token_values_cap;
	U32 lines_cap;
	const char* file_path;
	HccLocation location;
	bool print_color;

	bool is_preprocessor_if_expression;
	bool is_preprocessor_include;

	char** include_paths;
	U32 include_paths_count;

	HccStringId va_args_string_id;
	HccStringId defined_string_id;
	HccStringId predefined_macro_identifier_string_start_id;

	U32 __counter__;

	HccCodeFileId include_code_file_id;

	U32 preprocessor_nested_level;

	U32 brackets_to_close_count;
	HccToken brackets_to_close[_HCC_TOKENIZER_NESTED_BRACKETS_CAP];
	U32 brackets_to_close_token_indices[_HCC_TOKENIZER_NESTED_BRACKETS_CAP];
};

void hcc_string_buffer_clear(HccAstGen* astgen);
void hcc_string_buffer_append_byte(HccAstGen* astgen, char byte);
void hcc_string_buffer_append_string(HccAstGen* astgen, char* string, U32 string_size);
void hcc_string_buffer_append_fmt(HccAstGen* astgen, char* fmt, ...);

bool hcc_code_file_find_or_insert(HccAstGen* astgen, HccStringId path_string_id, HccCodeFileId* code_file_id_out, HccCodeFile** code_file_out);
HccCodeFile* hcc_code_file_get(HccAstGen* astgen, HccCodeFileId code_file_id);

void hcc_pp_if_span_push(HccAstGen* astgen, HccPPDirective directive);
void hcc_pp_if_span_pop(HccAstGen* astgen);
HccPPIfSpan* hcc_pp_if_span_peek_top(HccAstGen* astgen);

/*
void hcc_pp_if_span_maybe_promote_to_if_guard(HccAstGen* astgen);
void hcc_pp_if_span_finalize_and_pop(HccAstGen* astgen);
bool hcc_pp_if_span_is_if_guard(HccPPIfSpan* pp_if_span);
HccPPIfSpan* hcc_pp_if_span_head(HccAstGen* astgen, HccCodeFile* code_file);
HccPPIfSpan* hcc_pp_if_span_prev(HccAstGen* astgen, HccPPIfSpan* pp_if_span);
HccPPIfSpan* hcc_pp_if_span_next(HccAstGen* astgen, HccPPIfSpan* pp_if_span);
*/

bool hcc_opt_is_enabled(HccOpts* opts, HccOpt opt);
void hcc_opt_set_enabled(HccOpts* opts, HccOpt opt);

HccString hcc_data_type_string(HccAstGen* astgen, HccDataType data_type);
void hcc_data_type_size_align(HccAstGen* astgen, HccDataType data_type, Uptr* size_out, Uptr* align_out);
HccDataType hcc_data_type_resolve_generic(HccAstGen* astgen, HccDataType data_type);
void hcc_data_type_print_basic(HccAstGen* astgen, HccDataType data_type, void* data, FILE* f);
HccDataType hcc_data_type_unsigned_to_signed(HccDataType data_type);
HccDataType hcc_data_type_signed_to_unsigned(HccDataType data_type);

void hcc_string_table_init(HccStringTable* string_table, uint32_t data_cap, uint32_t entries_cap);
#define hcc_string_table_deduplicate_lit(string_table, string_lit) hcc_string_table_deduplicate(string_table, string_lit, sizeof(string_lit) - 1)
#define hcc_string_table_deduplicate_c_string(string_table, c_string) hcc_string_table_deduplicate(string_table, c_string, strlen(c_string))
HccStringId hcc_string_table_deduplicate(HccStringTable* string_table, char* string, uint32_t string_size);
HccString hcc_string_table_get(HccStringTable* string_table, HccStringId id);

void hcc_constant_table_init(HccConstantTable* constant_table, uint32_t data_cap, uint32_t entries_cap);
HccConstantId hcc_constant_table_deduplicate_basic(HccConstantTable* constant_table, HccAstGen* astgen, HccDataType data_type, void* data);
void hcc_constant_table_deduplicate_composite_start(HccConstantTable* constant_table, HccAstGen* astgen, HccDataType data_type);
void hcc_constant_table_deduplicate_composite_add(HccConstantTable* constant_table, HccConstantId constant_id);
HccConstantId hcc_constant_table_deduplicate_composite_end(HccConstantTable* constant_table);
HccConstantId hcc_constant_table_deduplicate_zero(HccConstantTable* constant_table, HccAstGen* astgen, HccDataType data_type);
HccConstant hcc_constant_table_get(HccConstantTable* constant_table, HccConstantId id);
bool hcc_constant_as_uint(HccConstant constant, U64* out);
bool hcc_constant_as_sint(HccConstant constant, S64* out);

void hcc_astgen_ensure_macro_args_count(HccAstGen* astgen, HccMacro* macro, U32 args_count);

typedef struct HccCompilerSetup HccCompilerSetup;
void hcc_astgen_init(HccAstGen* astgen, HccCompilerSetup* setup);
HCC_NORETURN void hcc_astgen_error_1(HccAstGen* astgen, const char* fmt, ...);
HCC_NORETURN void hcc_astgen_error_2(HccAstGen* astgen, U32 other_token_idx, const char* fmt, ...);
HCC_NORETURN void hcc_astgen_token_error_1(HccAstGen* astgen, const char* fmt, ...);
HCC_NORETURN void hcc_astgen_token_error_2(HccAstGen* astgen, U32 other_token_idx, const char* fmt, ...);
void hcc_astgen_add_token(HccAstGen* astgen, HccToken token);
void hcc_astgen_add_token_value(HccAstGen* astgen, HccTokenValue value);
void hcc_astgen_tokenize(HccAstGen* astgen);
HccToken hcc_token_peek(HccAstGen* astgen);
HccToken hcc_token_peek_ahead(HccAstGen* astgen, U32 by);
void hcc_token_consume(HccAstGen* astgen, U32 amount);
HccToken hcc_token_next(HccAstGen* astgen);
void hcc_token_value_consume(HccAstGen* astgen, U32 amount);
HccTokenValue hcc_token_value_peek(HccAstGen* astgen);
HccTokenValue hcc_token_value_next(HccAstGen* astgen);

HccExpr* hcc_astgen_generate_expr(HccAstGen* astgen, U32 min_precedence);
HccExpr* hcc_astgen_generate_stmt(HccAstGen* astgen);
void hcc_astgen_generate(HccAstGen* astgen);

void hcc_astgen_variable_stack_open(HccAstGen* astgen);
void hcc_astgen_variable_stack_close(HccAstGen* astgen);
U32 hcc_astgen_variable_stack_add(HccAstGen* astgen, HccStringId string_id);
U32 hcc_astgen_variable_stack_find(HccAstGen* astgen, HccStringId string_id);

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
	HCC_IR_OP_CODE_LOAD,
	HCC_IR_OP_CODE_STORE,

	HCC_IR_OP_CODE_BINARY_OP_START,
#define HCC_IR_OP_CODE_BINARY_OP(OP) (HCC_IR_OP_CODE_BINARY_OP_START + HCC_BINARY_OP_##OP)
	HCC_IR_OP_CODE_BINARY_OP_END = HCC_IR_OP_CODE_BINARY_OP_START + HCC_BINARY_OP_COUNT,

	HCC_IR_OP_CODE_UNARY_OP_START,
#define HCC_IR_OP_CODE_UNARY_OP(OP) (HCC_IR_OP_CODE_UNARY_OP_START + HCC_UNARY_OP_##OP)
	HCC_IR_OP_CODE_UNARY_OP_END = HCC_IR_OP_CODE_UNARY_OP_START + HCC_UNARY_OP_COUNT,

	HCC_IR_OP_CODE_PHI,

	HCC_IR_OP_CODE_SWITCH,

	HCC_IR_OP_CODE_CONVERT,
	HCC_IR_OP_CODE_BITCAST,

	HCC_IR_OP_CODE_COMPOSITE_INIT,
	HCC_IR_OP_CODE_ACCESS_CHAIN,
	HCC_IR_OP_CODE_FUNCTION_CALL,
	HCC_IR_OP_CODE_FUNCTION_RETURN,
	HCC_IR_OP_CODE_SELECTION_MERGE,
	HCC_IR_OP_CODE_LOOP_MERGE,
	HCC_IR_OP_CODE_BRANCH,
	HCC_IR_OP_CODE_BRANCH_CONDITIONAL,
	HCC_IR_OP_CODE_UNREACHABLE,
	HCC_IR_OP_CODE_SELECT,
};

typedef struct HccIRConst HccIRConst;
struct HccIRConst {
	HccDataType data_type;
	U32 values_offset;
};

typedef struct HccIRValue HccIRValue;
struct HccIRValue {
	HccDataType data_type;
	U16 defined_instruction_idx;
	U16 last_used_instruction_idx;
};

typedef struct HccIRInstr HccIRInstr;
struct HccIRInstr {
	U16 operands_start_idx;
	U8 operands_count;
	U8 op_code;
};

typedef struct HccIRBasicBlock HccIRBasicBlock;
struct HccIRBasicBlock {
	U16 instructions_start_idx;
	U16 instructions_count;
};

//
// inherits HccDataType
typedef U32 HccIROperand;
enum {
	HCC_IR_OPERAND_VALUE = HCC_DATA_TYPE_COUNT,
	HCC_IR_OPERAND_CONSTANT,
	HCC_IR_OPERAND_BASIC_BLOCK,
	HCC_IR_OPERAND_LOCAL_VARIABLE,
	HCC_IR_OPERAND_GLOBAL_VARIABLE,
};
#define HCC_IR_OPERAND_IS_DATA_TYPE(operand) (((operand) & 0xff) < HCC_DATA_TYPE_COUNT)

#define HCC_IR_OPERAND_VALUE_INIT(value_idx) (((value_idx) << 8) | HCC_IR_OPERAND_VALUE)
#define HCC_IR_OPERAND_IS_VALUE(operand) (((operand) & 0xff) == HCC_IR_OPERAND_VALUE)
#define HCC_IR_OPERAND_VALUE_IDX(operand) HCC_DATA_TYPE_IDX(operand)

#define HCC_IR_OPERAND_CONSTANT_INIT(constant_id) (((constant_id) << 8) | HCC_IR_OPERAND_CONSTANT)
#define HCC_IR_OPERAND_IS_CONSTANT(operand) (((operand) & 0xff) == HCC_IR_OPERAND_CONSTANT)
#define HCC_IR_OPERAND_CONSTANT_ID(operand) ((HccConstantId) { .idx_plus_one = HCC_DATA_TYPE_IDX(operand) })

#define HCC_IR_OPERAND_BASIC_BLOCK_INIT(basic_block_idx) (((basic_block_idx) << 8) | HCC_IR_OPERAND_BASIC_BLOCK)
#define HCC_IR_OPERAND_IS_BASIC_BLOCK(operand) (((operand) & 0xff) == HCC_IR_OPERAND_BASIC_BLOCK)
#define HCC_IR_OPERAND_BASIC_BLOCK_IDX(operand) HCC_DATA_TYPE_IDX(operand)

#define HCC_IR_OPERAND_LOCAL_VARIABLE_INIT(variable_idx) (((variable_idx) << 8) | HCC_IR_OPERAND_LOCAL_VARIABLE)
#define HCC_IR_OPERAND_GLOBAL_VARIABLE_INIT(variable_idx) (((variable_idx) << 8) | HCC_IR_OPERAND_GLOBAL_VARIABLE)
#define HCC_IR_OPERAND_VARIABLE_IDX(operand) HCC_DATA_TYPE_IDX(operand)

typedef struct HccIRFunction HccIRFunction;
struct HccIRFunction {
	U32 basic_blocks_start_idx;
	U32 instructions_start_idx;
	U32 values_start_idx;
	U32 operands_start_idx;
	U16 basic_blocks_count;
	U16 instructions_count;
	U16 values_count;
	U16 operands_count;
};

typedef struct HccIRBranchState HccIRBranchState;
struct HccIRBranchState {
	bool all_cases_return;
	U32 break_branch_linked_list_head;
	U32 break_branch_linked_list_tail;
	U32 continue_branch_linked_list_head;
	U32 continue_branch_linked_list_tail;
};

typedef struct HccIR HccIR;
struct HccIR {
	HccIRFunction* functions;
	U32 functions_count;
	U32 functions_cap;

	HccIRBasicBlock* basic_blocks;
	U32 basic_blocks_count;
	U32 basic_blocks_cap;

	HccIRValue* values;
	U32 values_count;
	U32 values_cap;

	HccIRInstr* instructions;
	U32 instructions_count;
	U32 instructions_cap;

	HccIROperand* operands;
	U32 operands_count;
	U32 operands_cap;

	HccIROperand last_operand;
	bool do_not_load_variable;
	HccDataType assign_data_type;

	HccIRBranchState branch_state;
};

void hcc_ir_init(HccIR* ir);
void hcc_ir_generate(HccIR* ir, HccAstGen* astgen);

// ===========================================
//
//
// SPIR-V
//
//
// ===========================================

typedef U16 HccSpirvOp;
enum {
	HCC_SPIRV_OP_NO_OP = 0,
	HCC_SPIRV_OP_EXTENSION = 10,
	HCC_SPIRV_OP_MEMORY_MODEL = 14,
	HCC_SPIRV_OP_ENTRY_POINT = 15,
	HCC_SPIRV_OP_EXECUTION_MODE = 16,
	HCC_SPIRV_OP_CAPABILITY = 17,
	HCC_SPIRV_OP_TYPE_VOID = 19,
	HCC_SPIRV_OP_TYPE_BOOL = 20,
	HCC_SPIRV_OP_TYPE_INT = 21,
	HCC_SPIRV_OP_TYPE_FLOAT = 22,
	HCC_SPIRV_OP_TYPE_VECTOR = 23,
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
	HCC_SPIRV_OP_VARIABLE = 59,
	HCC_SPIRV_OP_LOAD = 61,
	HCC_SPIRV_OP_STORE = 62,
	HCC_SPIRV_OP_ACCESS_CHAIN = 65,
	HCC_SPIRV_OP_DECORATE = 71,
	HCC_SPIRV_OP_COMPOSITE_CONSTRUCT = 80,
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

enum {
	HCC_SPIRV_ADDRESS_MODEL_LOGICAL = 0,
	HCC_SPIRV_ADDRESS_MODEL_PHYSICAL_STORAGE_BUFFER_64 = 5348,
};

enum {
	HCC_SPIRV_MEMORY_MODEL_GLSL450 = 1,
	HCC_SPIRV_MEMORY_MODEL_VULKAN = 3,
};

enum {
	HCC_SPIRV_EXECUTION_MODE_ORIGIN_LOWER_LEFT = 8,
};

enum {
	HCC_SPIRV_CAPABILITY_SHADER = 1,
	HCC_SPIRV_CAPABILITY_VULKAN_MEMORY_MODEL = 5345,
	HCC_SPIRV_CAPABILITY_PHYSICAL_STORAGE_BUFFER = 5347,
};

enum {
	HCC_SPIRV_STORAGE_CLASS_INPUT = 1,
	HCC_SPIRV_STORAGE_CLASS_UNIFORM = 2,
	HCC_SPIRV_STORAGE_CLASS_OUTPUT = 3,
	HCC_SPIRV_STORAGE_CLASS_PRIVATE = 6,
	HCC_SPIRV_STORAGE_CLASS_FUNCTION = 7,
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
	HCC_SPRIV_DECORATION_LOCATION = 30,
};

#define HCC_SPIRV_INSTR_OPERANDS_CAP 24

typedef U8 HccSpirvTypeKind;
enum {
	HCC_SPIRV_TYPE_KIND_FUNCTION,
	HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE,
	HCC_SPIRV_TYPE_KIND_STATIC_VARIABLE,
	HCC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER,
};

typedef struct HccSpirvTypeEntry HccSpirvTypeEntry;
struct HccSpirvTypeEntry {
	U32 data_types_start_idx;
	U32 spirv_id: 22;
	U32 data_types_count: 8;
	U32 kind: 2; // HccSpirvTypeKind
};

typedef struct HccSpirvTypeTable HccSpirvTypeTable;
struct HccSpirvTypeTable {
	HccDataType* data_types;
	HccSpirvTypeEntry* entries;
	U32 data_types_count;
	U32 data_types_cap;
	U32 entries_count;
	U32 entries_cap;
};

typedef struct HccSpirv HccSpirv;
struct HccSpirv {
	HccSpirvTypeTable type_table;

	U32* out_capabilities;
	U32 out_capabilities_count;
	U32 out_capabilities_cap;

	U32* out_entry_points;
	U32 out_entry_points_count;
	U32 out_entry_points_cap;

	U32* out_debug_info;
	U32 out_debug_info_count;
	U32 out_debug_info_cap;

	U32* out_annotations;
	U32 out_annotations_count;
	U32 out_annotations_cap;

	U32* out_types_variables_constants;
	U32 out_types_variables_constants_count;
	U32 out_types_variables_constants_cap;

	U32* out_functions;
	U32 out_functions_count;
	U32 out_functions_cap;

	U32 shader_stage_function_type_spirv_id;

	U32 pointer_type_inputs_base_id;
	U32 pointer_type_outputs_base_id;
	U64 pointer_type_inputs_made_bitset[4];
	U64 pointer_type_outputs_made_bitset[4];
	U32 compound_type_base_id;
	U32 array_type_base_id;

	U32 value_base_id;
	U32 constant_base_id;
	U32 basic_block_base_spirv_id;
	U32 local_variable_base_spirv_id;
	U32 global_variable_base_spirv_id;
	U32 next_id;
	HccSpirvOp instr_op;
	U16 instr_operands_count;
	U32 instr_operands[HCC_SPIRV_INSTR_OPERANDS_CAP];
};

void hcc_spirv_init(HccCompiler* c);
void hcc_spirv_generate(HccCompiler* c);

// ===========================================
//
//
// Compiler
//
//
// ===========================================

struct HccCompiler {
	HccAstGen astgen;
	HccIR ir;
	HccSpirv spirv;
	U16 available_basic_types;
};

struct HccCompilerSetup {
	uint32_t tokens_cap;
	uint32_t lines_cap;
	uint32_t functions_cap;
	uint32_t function_params_and_variables_cap;
	uint32_t exprs_cap;
	uint32_t variable_stack_cap;
	uint32_t string_table_data_cap;
	uint32_t string_table_entries_cap;
};

void hcc_compiler_init(HccCompiler* compiler, HccCompilerSetup* setup);

void hcc_compiler_compile(HccCompiler* compiler, char* file_path);

