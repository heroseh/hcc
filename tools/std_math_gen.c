#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const char* xyzw_idents[] = { "x", "y", "z", "w" };
static const char* abc_idents[] = { "a", "b", "c" };
static const char* clamp_idents[] = { "v", "min", "max" };
static const char* lerp_idents[] = { "start", "end", "t" };
static const char* invlerp_idents[] = { "start", "end", "v" };
static const char* remap_idents[] = { "v", "from_min", "from_max", "to_min", "to_max" };
static const char* swizzle_idents[] = { "v", "x", "y", "z", "w"  };

typedef enum HalfFn HalfFn;
enum HalfFn {
	HALF_FN_ADD,
	HALF_FN_SUB,
	HALF_FN_MUL,
	HALF_FN_DIV,
	HALF_FN_MOD,
	HALF_FN_EQ,
	HALF_FN_NEQ,
	HALF_FN_LT,
	HALF_FN_LTEQ,
	HALF_FN_GT,
	HALF_FN_GTEQ,
	HALF_FN_NOT,
	HALF_FN_NEG,
	HALF_FN_FMOD,
	HALF_FN_COPYSIGN,
	HALF_FN_FABS,
	HALF_FN_FLOOR,
	HALF_FN_CEIL,
	HALF_FN_ROUND,
	HALF_FN_TRUNC,
	HALF_FN_SIN,
	HALF_FN_COS,
	HALF_FN_TAN,
	HALF_FN_ASIN,
	HALF_FN_ACOS,
	HALF_FN_ATAN,
	HALF_FN_SINH,
	HALF_FN_COSH,
	HALF_FN_TANH,
	HALF_FN_ASINH,
	HALF_FN_ACOSH,
	HALF_FN_ATANH,
	HALF_FN_ATAN2,
	HALF_FN_FMA,
	HALF_FN_POW,
	HALF_FN_EXP,
	HALF_FN_LOG,
	HALF_FN_EXP2,
	HALF_FN_LOG2,

	HALF_FN_COUNT,
};

typedef enum VectorFn VectorFn;
enum VectorFn {
	//
	// needs special code generation
	VECTOR_FN_INIT,
	VECTOR_FN_PINIT,
	VECTOR_FN_MIN_ELMT,
	VECTOR_FN_MAX_ELMT,
	VECTOR_FN_SUM_ELMTS,
	VECTOR_FN_PRODUCT_ELMTS,
	VECTOR_FN_SQUARE,
	VECTOR_FN_SWIZZLE,
	VECTOR_FN_DOT,
	VECTOR_FN_LEN,
	VECTOR_FN_LENSQ,
	VECTOR_FN_NORM,
	VECTOR_FN_DISTANCE,
	VECTOR_FN_REFLECT,
	VECTOR_FN_REFRACT,

	//
	// vector of all types
#define VECTOR_FN_TANY_START VECTOR_FN_PACK
	VECTOR_FN_PACK,
	VECTOR_FN_UNPACK,
	VECTOR_FN_ANY,
	VECTOR_FN_ALL,
	VECTOR_FN_NOT,
#define VECTOR_FN_TANY_END VECTOR_FN_NOT

	//
	// vector uint & int & float functions
#define VECTOR_FN_UIF_START VECTOR_FN_ADD
	VECTOR_FN_ADD,
	VECTOR_FN_ADDS,
	VECTOR_FN_SUB,
	VECTOR_FN_SUBS,
	VECTOR_FN_MUL,
	VECTOR_FN_MULS,
	VECTOR_FN_DIV,
	VECTOR_FN_DIVS,
	VECTOR_FN_MOD,
	VECTOR_FN_MODS,
	VECTOR_FN_EQ,
	VECTOR_FN_EQS,
	VECTOR_FN_NEQ,
	VECTOR_FN_NEQS,
	VECTOR_FN_LT,
	VECTOR_FN_LTS,
	VECTOR_FN_LTEQ,
	VECTOR_FN_LTEQS,
	VECTOR_FN_GT,
	VECTOR_FN_GTS,
	VECTOR_FN_GTEQ,
	VECTOR_FN_GTEQS,
	VECTOR_FN_NEG,
	VECTOR_FN_MIN,
	VECTOR_FN_MINS,
	VECTOR_FN_MAX,
	VECTOR_FN_MAXS,
	VECTOR_FN_CLAMP,
	VECTOR_FN_CLAMPS,
#define VECTOR_FN_UIF_END VECTOR_FN_CLAMPS

	//
	// vector int & float functions
#define VECTOR_FN_IF_START VECTOR_FN_SIGN
	VECTOR_FN_SIGN,
	VECTOR_FN_COPYSIGN,
	VECTOR_FN_ABS,
#define VECTOR_FN_IF_END VECTOR_FN_ABS

	//
	// vector uint & int functions
#define VECTOR_FN_UI_START VECTOR_FN_BITAND
	VECTOR_FN_BITAND,
	VECTOR_FN_BITANDS,
	VECTOR_FN_BITOR,
	VECTOR_FN_BITORS,
	VECTOR_FN_BITXOR,
	VECTOR_FN_BITXORS,
	VECTOR_FN_BITSHL,
	VECTOR_FN_BITSHLS,
	VECTOR_FN_BITSHR,
	VECTOR_FN_BITSHRS,
	VECTOR_FN_BITNOT,
#define VECTOR_FN_UI_END VECTOR_FN_BITNOT

	//
	// vector float functions
#define VECTOR_FN_F_START VECTOR_FN_FMA
	VECTOR_FN_FMA,
	VECTOR_FN_FLOOR,
	VECTOR_FN_CEIL,
	VECTOR_FN_ROUND,
	VECTOR_FN_TRUNC,
	VECTOR_FN_FRACT,
	VECTOR_FN_RADIANS,
	VECTOR_FN_DEGREES,
	VECTOR_FN_STEP,
	VECTOR_FN_SMOOTHSTEP,
	VECTOR_FN_REMAP,
	VECTOR_FN_REMAPS,
	VECTOR_FN_ROUNDTOMULTIPLE,
	VECTOR_FN_ROUNDTOMULTIPLES,
	VECTOR_FN_ROUNDUPTOMULTIPLE,
	VECTOR_FN_ROUNDUPTOMULTIPLES,
	VECTOR_FN_ROUNDDOWNTOMULTIPLE,
	VECTOR_FN_ROUNDDOWNTOMULTIPLES,
	VECTOR_FN_BITSTO,
	VECTOR_FN_BITSFROM,
	VECTOR_FN_SIN,
	VECTOR_FN_COS,
	VECTOR_FN_TAN,
	VECTOR_FN_ASIN,
	VECTOR_FN_ACOS,
	VECTOR_FN_ATAN,
	VECTOR_FN_SINH,
	VECTOR_FN_COSH,
	VECTOR_FN_TANH,
	VECTOR_FN_ASINH,
	VECTOR_FN_ACOSH,
	VECTOR_FN_ATANH,
	VECTOR_FN_ATAN2,
	VECTOR_FN_POW,
	VECTOR_FN_EXP,
	VECTOR_FN_LOG,
	VECTOR_FN_EXP2,
	VECTOR_FN_LOG2,
	VECTOR_FN_SQRT,
	VECTOR_FN_RSQRT,
	VECTOR_FN_APPROXEQ,
	VECTOR_FN_APPROXEQS,
	VECTOR_FN_ISINF,
	VECTOR_FN_ISNAN,
	VECTOR_FN_LERP,
	VECTOR_FN_INVLERP,
#define VECTOR_FN_F_END VECTOR_FN_INVLERP

	VECTOR_FN_COUNT,
};

typedef enum Vector Vector;
enum Vector {
	VECTOR_2,
	VECTOR_3,
	VECTOR_4,

	VECTOR_COUNT,
};

typedef enum Matrix Matrix;
enum Matrix {
	MATRIX_2x2,
	MATRIX_2x3,
	MATRIX_2x4,
	MATRIX_3x2,
	MATRIX_3x3,
	MATRIX_3x4,
	MATRIX_4x2,
	MATRIX_4x3,
	MATRIX_4x4,

	MATRIX_COUNT,
};

typedef enum DataType DataType;
enum DataType {
	DATA_TYPE_BOOL,
	DATA_TYPE_HALF,
	DATA_TYPE_FLOAT,
	DATA_TYPE_DOUBLE,
	DATA_TYPE_I8,
	DATA_TYPE_I16,
	DATA_TYPE_I32,
	DATA_TYPE_I64,
	DATA_TYPE_U8,
	DATA_TYPE_U16,
	DATA_TYPE_U32,
	DATA_TYPE_U64,

	DATA_TYPE_COUNT,
};

typedef enum DataTypeClass DataTypeClass;
enum DataTypeClass {
	DATA_TYPE_CLASS_BOOL,
	DATA_TYPE_CLASS_FLOAT,
	DATA_TYPE_CLASS_UINT,
	DATA_TYPE_CLASS_INT,

	DATA_TYPE_CLASS_COUNT,
};

typedef struct Context Context;
struct Context {
	FILE* f;
	Matrix matrix;
	Vector vector;
	DataType data_type;
};
Context ctx;

const char* half_fn_ident[HALF_FN_COUNT] = {
	[HALF_FN_ADD] = "add",
	[HALF_FN_SUB] = "sub",
	[HALF_FN_MUL] = "mul",
	[HALF_FN_DIV] = "div",
	[HALF_FN_MOD] = "mod",
	[HALF_FN_EQ] = "eq",
	[HALF_FN_NEQ] = "neq",
	[HALF_FN_LT] = "lt",
	[HALF_FN_LTEQ] = "lteq",
	[HALF_FN_GT] = "gt",
	[HALF_FN_GTEQ] = "gteq",
	[HALF_FN_NOT] = "not",
	[HALF_FN_NEG] = "neg",
	[HALF_FN_FMOD] = "fmod",
	[HALF_FN_COPYSIGN] = "copysign",
	[HALF_FN_FABS] = "fabs",
	[HALF_FN_FLOOR] = "floor",
	[HALF_FN_CEIL] = "ceil",
	[HALF_FN_ROUND] = "round",
	[HALF_FN_TRUNC] = "trunc",
	[HALF_FN_SIN] = "sin",
	[HALF_FN_COS] = "cos",
	[HALF_FN_TAN] = "tan",
	[HALF_FN_ASIN] = "asin",
	[HALF_FN_ACOS] = "acos",
	[HALF_FN_ATAN] = "atan",
	[HALF_FN_SINH] = "sinh",
	[HALF_FN_COSH] = "cosh",
	[HALF_FN_TANH] = "tanh",
	[HALF_FN_ASINH] = "asinh",
	[HALF_FN_ACOSH] = "acosh",
	[HALF_FN_ATANH] = "atanh",
	[HALF_FN_ATAN2] = "atan2",
	[HALF_FN_FMA] = "fma",
	[HALF_FN_POW] = "pow",
	[HALF_FN_EXP] = "exp",
	[HALF_FN_LOG] = "log",
	[HALF_FN_EXP2] = "exp2",
	[HALF_FN_LOG2] = "log2",
};

bool half_fn_returns_bool[HALF_FN_COUNT] = {
	[HALF_FN_NOT] = true,
};

unsigned half_fn_num_params[HALF_FN_COUNT] = {
	[HALF_FN_ADD] = 2,
	[HALF_FN_SUB] = 2,
	[HALF_FN_MUL] = 2,
	[HALF_FN_DIV] = 2,
	[HALF_FN_MOD] = 2,
	[HALF_FN_EQ] = 2,
	[HALF_FN_NEQ] = 2,
	[HALF_FN_LT] = 2,
	[HALF_FN_LTEQ] = 2,
	[HALF_FN_GT] = 2,
	[HALF_FN_GTEQ] = 2,
	[HALF_FN_NOT] = 1,
	[HALF_FN_NEG] = 1,
	[HALF_FN_FMOD] = 1,
	[HALF_FN_COPYSIGN] = 2,
	[HALF_FN_FABS] = 1,
	[HALF_FN_FLOOR] = 1,
	[HALF_FN_CEIL] = 1,
	[HALF_FN_ROUND] = 1,
	[HALF_FN_TRUNC] = 1,
	[HALF_FN_SIN] = 1,
	[HALF_FN_COS] = 1,
	[HALF_FN_TAN] = 1,
	[HALF_FN_ASIN] = 1,
	[HALF_FN_ACOS] = 1,
	[HALF_FN_ATAN] = 1,
	[HALF_FN_SINH] = 1,
	[HALF_FN_COSH] = 1,
	[HALF_FN_TANH] = 1,
	[HALF_FN_ASINH] = 1,
	[HALF_FN_ACOSH] = 1,
	[HALF_FN_ATANH] = 1,
	[HALF_FN_ATAN2] = 2,
	[HALF_FN_FMA] = 1,
	[HALF_FN_POW] = 1,
	[HALF_FN_EXP] = 1,
	[HALF_FN_LOG] = 1,
	[HALF_FN_EXP2] = 1,
	[HALF_FN_LOG2] = 1,
};

const char* half_fn_c_unary_operators[HALF_FN_COUNT] = {
	[HALF_FN_NOT] = "!",
	[HALF_FN_NEG] = "-",
};

const char* half_fn_c_binary_operators[HALF_FN_COUNT] = {
	[HALF_FN_ADD] = "+",
	[HALF_FN_SUB] = "-",
	[HALF_FN_MUL] = "*",
	[HALF_FN_DIV] = "/",
	[HALF_FN_EQ] = "==",
	[HALF_FN_NEQ] = "!=",
	[HALF_FN_LT] = "<",
	[HALF_FN_LTEQ] = "<=",
	[HALF_FN_GT] = ">",
	[HALF_FN_GTEQ] = ">=",
};

const char* vector_fn_c_unary_operators[VECTOR_FN_COUNT] = {
	[VECTOR_FN_NOT] = "!",
	[VECTOR_FN_NEG] = "-",
	[VECTOR_FN_BITNOT] = "~",
};

const char* vector_fn_c_binary_operators[VECTOR_FN_COUNT] = {
	[VECTOR_FN_ADD] = "+",
	[VECTOR_FN_ADDS] = "+",
	[VECTOR_FN_SUB] = "-",
	[VECTOR_FN_SUBS] = "-",
	[VECTOR_FN_MUL] = "*",
	[VECTOR_FN_MULS] = "*",
	[VECTOR_FN_DIV] = "/",
	[VECTOR_FN_DIVS] = "/",
	[VECTOR_FN_MOD] = "%",
	[VECTOR_FN_MODS] = "%",
	[VECTOR_FN_EQ] = "==",
	[VECTOR_FN_EQS] = "==",
	[VECTOR_FN_NEQ] = "!=",
	[VECTOR_FN_NEQS] = "!=",
	[VECTOR_FN_LT] = "<",
	[VECTOR_FN_LTS] = "<",
	[VECTOR_FN_LTEQ] = "<=",
	[VECTOR_FN_LTEQS] = "<=",
	[VECTOR_FN_GT] = ">",
	[VECTOR_FN_GTS] = ">",
	[VECTOR_FN_GTEQ] = ">=",
	[VECTOR_FN_GTEQS] = ">=",
	[VECTOR_FN_BITAND] = "&",
	[VECTOR_FN_BITANDS] = "&",
	[VECTOR_FN_BITOR] = "|",
	[VECTOR_FN_BITORS] = "|",
	[VECTOR_FN_BITXOR] = "^",
	[VECTOR_FN_BITXORS] = "^",
	[VECTOR_FN_BITSHL] = "<<",
	[VECTOR_FN_BITSHLS] = "<<",
	[VECTOR_FN_BITSHR] = ">>",
	[VECTOR_FN_BITSHRS] = ">>",
};

bool vector_fn_is_bool_operator[VECTOR_FN_COUNT] = {
	[VECTOR_FN_NOT] = true,
	[VECTOR_FN_EQ] = true,
	[VECTOR_FN_EQS] = true,
	[VECTOR_FN_NEQ] = true,
	[VECTOR_FN_NEQS] = true,
};

bool vector_fn_is_logical_or[VECTOR_FN_COUNT] = {
	[VECTOR_FN_ANY] = true,
};

bool vector_fn_returns_bool[VECTOR_FN_COUNT] = {
	[VECTOR_FN_ANY] = true,
	[VECTOR_FN_ALL] = true,
	[VECTOR_FN_APPROXEQ] = true,
	[VECTOR_FN_APPROXEQS] = true,
};

bool vector_fn_return_vector_bool[VECTOR_FN_COUNT] = {
	[VECTOR_FN_NOT] = true,
	[VECTOR_FN_EQ] = true,
	[VECTOR_FN_EQS] = true,
	[VECTOR_FN_NEQ] = true,
	[VECTOR_FN_NEQS] = true,
	[VECTOR_FN_LT] = true,
	[VECTOR_FN_LTS] = true,
	[VECTOR_FN_LTEQ] = true,
	[VECTOR_FN_LTEQS] = true,
	[VECTOR_FN_GT] = true,
	[VECTOR_FN_GTS] = true,
	[VECTOR_FN_GTEQ] = true,
	[VECTOR_FN_GTEQS] = true,
	[VECTOR_FN_ISINF] = true,
	[VECTOR_FN_ISNAN] = true,
};

bool vector_fn_no_operator_or_calls[VECTOR_FN_COUNT] = {
	[VECTOR_FN_PACK] = true,
	[VECTOR_FN_UNPACK] = true,
	[VECTOR_FN_ANY] = true,
	[VECTOR_FN_ALL] = true,
};

bool vector_fn_has_scalar_params[VECTOR_FN_COUNT] = {
	[VECTOR_FN_ADDS] = true,
	[VECTOR_FN_SUBS] = true,
	[VECTOR_FN_MULS] = true,
	[VECTOR_FN_DIVS] = true,
	[VECTOR_FN_MODS] = true,
	[VECTOR_FN_EQS] = true,
	[VECTOR_FN_NEQS] = true,
	[VECTOR_FN_LTS] = true,
	[VECTOR_FN_LTEQS] = true,
	[VECTOR_FN_GTS] = true,
	[VECTOR_FN_GTEQS] = true,
	[VECTOR_FN_MINS] = true,
	[VECTOR_FN_MAXS] = true,
	[VECTOR_FN_CLAMPS] = true,
	[VECTOR_FN_BITANDS] = true,
	[VECTOR_FN_BITORS] = true,
	[VECTOR_FN_BITXORS] = true,
	[VECTOR_FN_BITSHLS] = true,
	[VECTOR_FN_BITSHRS] = true,
	[VECTOR_FN_REMAPS] = true,
	[VECTOR_FN_ROUNDTOMULTIPLES] = true,
	[VECTOR_FN_ROUNDUPTOMULTIPLES] = true,
	[VECTOR_FN_ROUNDDOWNTOMULTIPLES] = true,
};

unsigned vector_fn_number_params[VECTOR_FN_COUNT] = {
	//
	// needs special code generation
	[VECTOR_FN_INIT] = 0,
	[VECTOR_FN_PINIT] = 0,
	[VECTOR_FN_MIN_ELMT] = 1,
	[VECTOR_FN_MAX_ELMT] = 1,
	[VECTOR_FN_SUM_ELMTS] = 1,
	[VECTOR_FN_PRODUCT_ELMTS] = 1,
	[VECTOR_FN_SQUARE] = 1,
	[VECTOR_FN_SWIZZLE] = 0,
	[VECTOR_FN_DOT] = 2,
	[VECTOR_FN_NORM] = 1,
	[VECTOR_FN_DISTANCE] = 2,
	[VECTOR_FN_REFLECT] = 2,
	[VECTOR_FN_REFRACT] = 3,

	//
	// vector of all types
	[VECTOR_FN_PACK] = 1,
	[VECTOR_FN_UNPACK] = 1,
	[VECTOR_FN_ANY] = 1,
	[VECTOR_FN_ALL] = 1,
	[VECTOR_FN_NOT] = 1,

	//
	// vector uint & int & float functions
	[VECTOR_FN_ADD] = 2,
	[VECTOR_FN_ADDS] = 2,
	[VECTOR_FN_SUB] = 2,
	[VECTOR_FN_SUBS] = 2,
	[VECTOR_FN_MUL] = 2,
	[VECTOR_FN_MULS] = 2,
	[VECTOR_FN_DIV] = 2,
	[VECTOR_FN_DIVS] = 2,
	[VECTOR_FN_MOD] = 2,
	[VECTOR_FN_MODS] = 2,
	[VECTOR_FN_EQ] = 2,
	[VECTOR_FN_EQS] = 2,
	[VECTOR_FN_NEQ] = 2,
	[VECTOR_FN_NEQS] = 2,
	[VECTOR_FN_LT] = 2,
	[VECTOR_FN_LTS] = 2,
	[VECTOR_FN_LTEQ] = 2,
	[VECTOR_FN_LTEQS] = 2,
	[VECTOR_FN_GT] = 2,
	[VECTOR_FN_GTS] = 2,
	[VECTOR_FN_GTEQ] = 2,
	[VECTOR_FN_GTEQS] = 2,
	[VECTOR_FN_NEG] = 1,
	[VECTOR_FN_MIN] = 2,
	[VECTOR_FN_MINS] = 2,
	[VECTOR_FN_MAX] = 2,
	[VECTOR_FN_MAXS] = 2,
	[VECTOR_FN_CLAMP] = 3,
	[VECTOR_FN_CLAMPS] = 3,

	//
	// vector int & float functions
	[VECTOR_FN_SIGN] = 1,
	[VECTOR_FN_COPYSIGN] = 2,
	[VECTOR_FN_ABS] = 1,

	//
	// vector uint & int functions
	[VECTOR_FN_BITAND] = 2,
	[VECTOR_FN_BITANDS] = 2,
	[VECTOR_FN_BITOR] = 2,
	[VECTOR_FN_BITORS] = 2,
	[VECTOR_FN_BITXOR] = 2,
	[VECTOR_FN_BITXORS] = 2,
	[VECTOR_FN_BITSHL] = 2,
	[VECTOR_FN_BITSHLS] = 2,
	[VECTOR_FN_BITSHR] = 2,
	[VECTOR_FN_BITSHRS] = 2,
	[VECTOR_FN_BITNOT] = 1,

	//
	// vector float functions
	[VECTOR_FN_FMA] = 3,
	[VECTOR_FN_FLOOR] = 1,
	[VECTOR_FN_CEIL] = 1,
	[VECTOR_FN_ROUND] = 1,
	[VECTOR_FN_TRUNC] = 1,
	[VECTOR_FN_FRACT] = 1,
	[VECTOR_FN_RADIANS] = 1,
	[VECTOR_FN_DEGREES] = 1,
	[VECTOR_FN_STEP] = 1,
	[VECTOR_FN_SMOOTHSTEP] = 1,
	[VECTOR_FN_REMAP] = 5,
	[VECTOR_FN_REMAPS] = 5,
	[VECTOR_FN_ROUNDTOMULTIPLE] = 2,
	[VECTOR_FN_ROUNDTOMULTIPLES] = 2,
	[VECTOR_FN_ROUNDUPTOMULTIPLE] = 2,
	[VECTOR_FN_ROUNDUPTOMULTIPLES] = 2,
	[VECTOR_FN_ROUNDDOWNTOMULTIPLE] = 2,
	[VECTOR_FN_ROUNDDOWNTOMULTIPLES] = 2,
	[VECTOR_FN_BITSTO] = 1,
	[VECTOR_FN_BITSFROM] = 1,
	[VECTOR_FN_SIN] = 1,
	[VECTOR_FN_COS] = 1,
	[VECTOR_FN_TAN] = 1,
	[VECTOR_FN_ASIN] = 1,
	[VECTOR_FN_ACOS] = 1,
	[VECTOR_FN_ATAN] = 1,
	[VECTOR_FN_SINH] = 1,
	[VECTOR_FN_COSH] = 1,
	[VECTOR_FN_TANH] = 1,
	[VECTOR_FN_ASINH] = 1,
	[VECTOR_FN_ACOSH] = 1,
	[VECTOR_FN_ATANH] = 1,
	[VECTOR_FN_ATAN2] = 2,
	[VECTOR_FN_POW] = 1,
	[VECTOR_FN_EXP] = 1,
	[VECTOR_FN_LOG] = 1,
	[VECTOR_FN_EXP2] = 1,
	[VECTOR_FN_LOG2] = 1,
	[VECTOR_FN_SQRT] = 1,
	[VECTOR_FN_RSQRT] = 1,
	[VECTOR_FN_APPROXEQ] = 2,
	[VECTOR_FN_APPROXEQS] = 2,
	[VECTOR_FN_ISINF] = 1,
	[VECTOR_FN_ISNAN] = 1,
	[VECTOR_FN_LERP] = 3,
	[VECTOR_FN_INVLERP] = 3,
};

const char* vector_fn_idents[VECTOR_FN_COUNT] = {
	//
	// needs special code generation
	[VECTOR_FN_INIT] = "",
	[VECTOR_FN_PINIT] = "p",
	[VECTOR_FN_MIN_ELMT] = "minelmt",
	[VECTOR_FN_MAX_ELMT] = "maxelmt",
	[VECTOR_FN_SUM_ELMTS] = "sumelmts",
	[VECTOR_FN_PRODUCT_ELMTS] = "productelmts",
	[VECTOR_FN_SQUARE] = "square",
	[VECTOR_FN_SWIZZLE] = "swizzle",
	[VECTOR_FN_DOT] = "dot",
	[VECTOR_FN_NORM] = "norm",
	[VECTOR_FN_DISTANCE] = "distance",
	[VECTOR_FN_REFLECT] = "reflect",
	[VECTOR_FN_REFRACT] = "refract",

	//
	// vector of all types
	[VECTOR_FN_PACK] = "pack",
	[VECTOR_FN_UNPACK] = "unpack",
	[VECTOR_FN_ANY] = "any",
	[VECTOR_FN_ALL] = "all",
	[VECTOR_FN_NOT] = "not",

	//
	// vector uint & int & float functions
	[VECTOR_FN_ADD] = "add",
	[VECTOR_FN_ADDS] = "adds",
	[VECTOR_FN_SUB] = "sub",
	[VECTOR_FN_SUBS] = "subs",
	[VECTOR_FN_MUL] = "mul",
	[VECTOR_FN_MULS] = "muls",
	[VECTOR_FN_DIV] = "div",
	[VECTOR_FN_DIVS] = "divs",
	[VECTOR_FN_MOD] = "mod",
	[VECTOR_FN_MODS] = "mods",
	[VECTOR_FN_EQ] = "eq",
	[VECTOR_FN_EQS] = "eqs",
	[VECTOR_FN_NEQ] = "neqs",
	[VECTOR_FN_NEQS] = "neqs",
	[VECTOR_FN_LT] = "lt",
	[VECTOR_FN_LTS] = "lts",
	[VECTOR_FN_LTEQ] = "lteqs",
	[VECTOR_FN_LTEQS] = "lteqs",
	[VECTOR_FN_GT] = "gt",
	[VECTOR_FN_GTS] = "gts",
	[VECTOR_FN_GTEQ] = "gteq",
	[VECTOR_FN_GTEQS] = "gteqs",
	[VECTOR_FN_NEG] = "neg",
	[VECTOR_FN_MIN] = "min",
	[VECTOR_FN_MINS] = "mins",
	[VECTOR_FN_MAX] = "max",
	[VECTOR_FN_MAXS] = "maxs",
	[VECTOR_FN_CLAMP] = "clamp",
	[VECTOR_FN_CLAMPS] = "clamps",

	//
	// vector int & float functions
	[VECTOR_FN_SIGN] = "sign",
	[VECTOR_FN_COPYSIGN] = "copysign",
	[VECTOR_FN_ABS] = "abs",

	//
	// vector uint & int functions
	[VECTOR_FN_BITAND] = "bitand",
	[VECTOR_FN_BITANDS] = "bitands",
	[VECTOR_FN_BITOR] = "bitor",
	[VECTOR_FN_BITORS] = "bitors",
	[VECTOR_FN_BITXOR] = "bitxor",
	[VECTOR_FN_BITXORS] = "bitxors",
	[VECTOR_FN_BITSHL] = "bitshl",
	[VECTOR_FN_BITSHLS] = "bitshls",
	[VECTOR_FN_BITSHR] = "bitshr",
	[VECTOR_FN_BITSHRS] = "bitshrs",
	[VECTOR_FN_BITNOT] = "bitnot",

	//
	// vector float functions
	[VECTOR_FN_FMA] = "fma",
	[VECTOR_FN_FLOOR] = "floor",
	[VECTOR_FN_CEIL] = "ceil",
	[VECTOR_FN_ROUND] = "round",
	[VECTOR_FN_TRUNC] = "trunc",
	[VECTOR_FN_FRACT] = "fract",
	[VECTOR_FN_RADIANS] = "radians",
	[VECTOR_FN_DEGREES] = "degrees",
	[VECTOR_FN_STEP] = "step",
	[VECTOR_FN_SMOOTHSTEP] = "smoothstep",
	[VECTOR_FN_REMAP] = "remap",
	[VECTOR_FN_REMAPS] = "remaps",
	[VECTOR_FN_ROUNDTOMULTIPLE] = "roundtomultiple",
	[VECTOR_FN_ROUNDTOMULTIPLES] = "roundtomultiples",
	[VECTOR_FN_ROUNDUPTOMULTIPLE] = "rounduptomultiple",
	[VECTOR_FN_ROUNDUPTOMULTIPLES] = "rounduptomultiples",
	[VECTOR_FN_ROUNDDOWNTOMULTIPLE] = "rounddowntomultiple",
	[VECTOR_FN_ROUNDDOWNTOMULTIPLES] = "rounddowntomultiples",
	[VECTOR_FN_BITSTO] = "bitsto",
	[VECTOR_FN_BITSFROM] = "bitsfrom",
	[VECTOR_FN_SIN] = "sin",
	[VECTOR_FN_COS] = "cos",
	[VECTOR_FN_TAN] = "tan",
	[VECTOR_FN_ASIN] = "asin",
	[VECTOR_FN_ACOS] = "acos",
	[VECTOR_FN_ATAN] = "atan",
	[VECTOR_FN_SINH] = "sinh",
	[VECTOR_FN_COSH] = "cosh",
	[VECTOR_FN_TANH] = "tanh",
	[VECTOR_FN_ASINH] = "asinh",
	[VECTOR_FN_ACOSH] = "acosh",
	[VECTOR_FN_ATANH] = "atanh",
	[VECTOR_FN_ATAN2] = "atan2",
	[VECTOR_FN_POW] = "pow",
	[VECTOR_FN_EXP] = "exp",
	[VECTOR_FN_LOG] = "log",
	[VECTOR_FN_EXP2] = "exp2",
	[VECTOR_FN_LOG2] = "log2",
	[VECTOR_FN_SQRT] = "sqrt",
	[VECTOR_FN_RSQRT] = "rsqrt",
	[VECTOR_FN_APPROXEQ] = "approxeq",
	[VECTOR_FN_APPROXEQS] = "approxeqs",
	[VECTOR_FN_ISINF] = "isinf",
	[VECTOR_FN_ISNAN] = "isnan",
	[VECTOR_FN_LERP] = "lerp",
	[VECTOR_FN_INVLERP] = "invlerp",
};

const char* vector_fn_docs[VECTOR_FN_COUNT] = {
	//
	// needs special code generation
	[VECTOR_FN_INIT] = "initializes a new vector with 2, 3 or 4 components of any of the following types:\n//\tbool, half, float, double, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, or int64_t",
	[VECTOR_FN_PINIT] ="initializes a new _packed_ vector with 2, 3 or 4 components of any of the following types:\n//\tbool, half, float, double, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, or int64_t",
	[VECTOR_FN_MIN_ELMT] = "returns the minimum value from each of the components in 'v'",
	[VECTOR_FN_MAX_ELMT] = "returns the maximum value from each of the components in 'v' aka. L infinity norm",
	[VECTOR_FN_SUM_ELMTS] = "returns the sum of all of the components in 'v'",
	[VECTOR_FN_PRODUCT_ELMTS] = "returns the product of all of the components in 'v'",
	[VECTOR_FN_SQUARE] = "returns a vector where each component is the square of itself",
	[VECTOR_FN_SWIZZLE] = "returns a vector that is a a shuffled version of 'v' that is constructed like so:\n//\tv4f(v.array[x], v.array[y], v.array[z], v.array[w]);",
	[VECTOR_FN_DOT] = "returns a vector which is the dot product of 'a' and 'b'",
	[VECTOR_FN_LEN] = "returns a euclidean length of the vector 'v' aka. L2 norm",
	[VECTOR_FN_LENSQ] = "returns the squared euclidean length of the vector 'v', this avoid doing the square root. useful when you want to compare of one length is less than another vector length without paying the cost of a sqrt instruction",
	[VECTOR_FN_NORM] = "returns a version of 'v' where the magnatude is a unit length of 1.0",
	[VECTOR_FN_DISTANCE] = "returns the distance between 'a' and 'b'",
	[VECTOR_FN_REFLECT] = "returns a vector that is vector 'v' reflected against surface 'normal'",
	[VECTOR_FN_REFRACT] = "returns the refraction vector for vector 'v' against surface 'normal' with the ratio 'eta'",

	//
	// vector of all types
	[VECTOR_FN_PACK] = "converts vector type that is natively aligned to a packed vector which is aligned to it's component alignment",
	[VECTOR_FN_UNPACK] = "converts _packed_ vector type that is aligned to it's component alignment aligned to a vector which is natively aligned",
	[VECTOR_FN_ANY] = "returns true if _any_ of the vector components are a non-zero value, otherwise false is returned",
	[VECTOR_FN_ALL] = "returns true if _all_ of the vector components are a non-zero value, otherwise false is returned",
	[VECTOR_FN_NOT] = "returns a boolean vector where each component is true if the component in 'v' is a zero value, otherwise it would be false",

	//
	// vector uint & int & float functions
	[VECTOR_FN_ADD] = "returns a vector where each component is the result from adding that component in 'a' to that component in 'b'",
	[VECTOR_FN_ADDS] = "returns a vector where each component is the result from adding that component in 'v' to the value of 's'",
	[VECTOR_FN_SUB] = "returns a vector where each component is the result from subtracting that component in 'a' to that component in 'b'",
	[VECTOR_FN_SUBS] = "returns a vector where each component is the result from subtracting that component in 'v' to the value of 's'",
	[VECTOR_FN_MUL] = "returns a vector where each component is the result from multiplying that component in 'a' to that component in 'b'",
	[VECTOR_FN_MULS] = "returns a vector where each component is the result from multiplying that component in 'v' to the value of 's'",
	[VECTOR_FN_DIV] = "returns a vector where each component is the result from dividing that component in 'a' to that component in 'b'",
	[VECTOR_FN_DIVS] = "returns a vector where each component is the result from dividing that component in 'v' to the value of 's'",
	[VECTOR_FN_MOD] = "returns a vector where each component is the result from moduloing that component in 'a' to that component in 'b'",
	[VECTOR_FN_MODS] = "returns a vector where each component is the result from moduloing that component in 'v' to the value 's'",
	[VECTOR_FN_EQ] = "returns a boolean vector where each component is true when that component in 'a' is equal to that component in 'b'",
	[VECTOR_FN_EQS] = "returns a boolean vector where each component is true when that component in 'v' is equal to the value 's'",
	[VECTOR_FN_NEQ] = "returns a boolean vector where each component is true when that component in 'a' is not equal to that component in 'b'",
	[VECTOR_FN_NEQS] = "returns a boolean vector where each component is true when that component in 'v' is not equal to the value 's'",
	[VECTOR_FN_LT] = "returns a boolean vector where each component is true when that component in 'a' is less than to that component in 'b'",
	[VECTOR_FN_LTS] = "returns a boolean vector where each component is true when that component in 'v' is less than to the value 's'",
	[VECTOR_FN_LTEQ] = "returns a boolean vector where each component is true when that component in 'a' is less than or equal to that component in 'b'",
	[VECTOR_FN_LTEQS] = "returns a boolean vector where each component is true when that component in 'v' is less than or equal to the value 's'",
	[VECTOR_FN_GT] = "returns a boolean vector where each component is true when that component in 'a' is greater than to that component in 'b'",
	[VECTOR_FN_GTS] = "returns a boolean vector where each component is true when that component in 'v' is greater than to the value 's'",
	[VECTOR_FN_GTEQ] = "returns a boolean vector where each component is true when that component in 'a' is greater than or equal to that component in 'b'",
	[VECTOR_FN_GTEQS] = "returns a boolean vector where each component is true when that component in 'v' is greater than or equal to the value 's'",
	[VECTOR_FN_NEG] = "returns a vector where each component is the result from negating that component in 'v'",
	[VECTOR_FN_MIN] = "returns a vector where each component is the minimum between that component in 'a' and that component in 'b'",
	[VECTOR_FN_MINS] = "returns a vector where each component is the minimum between that component in 'a' and 's'",
	[VECTOR_FN_MAX] = "returns a vector where each component is the maximum between that component in 'a' and that component in 'b'",
	[VECTOR_FN_MAXS] = "returns a vector where each component is the maximum between that component in 'a' and 's'",
	[VECTOR_FN_CLAMP] = "returns a vector where each component is clamped between the minimum value that is the component in 'min' and the maximum value that is the component in 'max'",
	[VECTOR_FN_CLAMPS] = "returns a vector where each component is clamped between the minimum value 'min' and the maximum value 'max'",

	//
	// vector int & float functions
	[VECTOR_FN_SIGN] = "returns a vector where each component is -1 or 1 depending on the sign of that component that is in 'v'",
	[VECTOR_FN_COPYSIGN] = "returns a vector where each component is that component in 'v' with sign of that component in 'sign'",
	[VECTOR_FN_ABS] = "returns a vector where each component is the absolute of that component in 'v'",

	//
	// vector uint & int functions
	[VECTOR_FN_BITAND] = "returns a vector where each component is the result from bitwise anding that component in 'a' to that component in 'b'",
	[VECTOR_FN_BITANDS] = "returns a vector where each component is the result from bitwise anding that component in 'v' to the value 's'",
	[VECTOR_FN_BITOR] = "returns a vector where each component is the result from bitwise oring that component in 'a' to that component in 'b'",
	[VECTOR_FN_BITORS] = "returns a vector where each component is the result from bitwise oring that component in 'v' to the value 's'",
	[VECTOR_FN_BITXOR] = "returns a vector where each component is the result from bitwise xoring that component in 'a' to that component in 'b'",
	[VECTOR_FN_BITXORS] = "returns a vector where each component is the result from bitwise xoring that component in 'v' to the value 's'",
	[VECTOR_FN_BITSHL] = "returns a vector where each component is the result from bitwise shifting that component in 'v' to the left by the component in 'b'",
	[VECTOR_FN_BITSHLS] = "returns a vector where each component is the result from bitwise shifting that component in 'v' to the left by the value 's'",
	[VECTOR_FN_BITSHR] = "returns a vector where each component is the result from bitwise shifting that component in 'v' to the right by the component in 'b'",
	[VECTOR_FN_BITSHRS] = "returns a vector where each component is the result from bitwise shifting that component in 'v' to the right by the value 's'",
	[VECTOR_FN_BITNOT] = "returns a vector where each component is the result from bitwise noting that component in 'v'",

	//
	// vector float functions
	[VECTOR_FN_FMA] = "returns a vector where each component (x) is calculated like so x = (a.x * b.x) + c.x",
	[VECTOR_FN_FLOOR] = "return a vector where each component is the result of apply 'floor' to that component in 'v'",
	[VECTOR_FN_CEIL] = "return a vector where each component is the result of apply 'ceil' to that component in 'v'",
	[VECTOR_FN_ROUND] = "return a vector where each component is the result of apply 'round' to that component in 'v'",
	[VECTOR_FN_TRUNC] = "return a vector where each component is the result of apply 'trunc' to that component in 'v'",
	[VECTOR_FN_FRACT] = "return a vector where each component is the result of apply 'fract' to that component in 'v'",
	[VECTOR_FN_RADIANS] = "return a vector where each component is the result of apply 'radians' to that component in 'v'",
	[VECTOR_FN_DEGREES] = "return a vector where each component is the result of apply 'degrees' to that component in 'v'",
	[VECTOR_FN_STEP] = "return a vector where each component is the result of apply 'step' to that component in 'v'",
	[VECTOR_FN_SMOOTHSTEP] = "return a vector where each component is the result of apply 'smoothstep' to that component in 'v'",
	[VECTOR_FN_REMAP] = "return a vector where each component is the result of apply 'remap' to that component in 'v', 'from_min', 'from_max', 'to_min' and 'to_max'",
	[VECTOR_FN_REMAPS] = "return a vector where each component is the result of apply 'remap' to that component in 'v' with scalar 'from_min', 'from_max', 'to_min' and 'to_max'",
	[VECTOR_FN_ROUNDTOMULTIPLE] = "return a vector where each component is the result of apply 'roundtomultiple' to that component in 'v' and 'multiple'",
	[VECTOR_FN_ROUNDTOMULTIPLES] = "return a vector where each component is the result of apply 'roundtomultiple' to that component in 'v' with scalar 'multiple'",
	[VECTOR_FN_ROUNDUPTOMULTIPLE] = "return a vector where each component is the result of apply 'rounduptomultiple' to that component in 'v' and 'multiple'",
	[VECTOR_FN_ROUNDUPTOMULTIPLES] = "return a vector where each component is the result of apply 'rounduptomultiple' to that component in 'v' with scalar 'multiple'",
	[VECTOR_FN_ROUNDDOWNTOMULTIPLE] = "return a vector where each component is the result of apply 'rounddowntomultiple' to that component in 'v' and 'multiple'",
	[VECTOR_FN_ROUNDDOWNTOMULTIPLES] = "return a vector where each component is the result of apply 'rounddowntomultiple' to that component in 'v' with scalar 'multiple'",
	[VECTOR_FN_BITSTO] = "return a vector where each component is the result of apply 'bitsto' to that component in 'v'",
	[VECTOR_FN_BITSFROM] = "return a vector where each component is the result of apply 'bitsfrom' to that component in 'v'",
	[VECTOR_FN_SIN] = "return a vector where each component is the result of apply 'sin' to that component in 'v'",
	[VECTOR_FN_COS] = "return a vector where each component is the result of apply 'cos' to that component in 'v'",
	[VECTOR_FN_TAN] = "return a vector where each component is the result of apply 'tan' to that component in 'v'",
	[VECTOR_FN_ASIN] = "return a vector where each component is the result of apply 'asin' to that component in 'v'",
	[VECTOR_FN_ACOS] = "return a vector where each component is the result of apply 'acos' to that component in 'v'",
	[VECTOR_FN_ATAN] = "return a vector where each component is the result of apply 'atan' to that component in 'v'",
	[VECTOR_FN_SINH] = "return a vector where each component is the result of apply 'sinh' to that component in 'v'",
	[VECTOR_FN_COSH] = "return a vector where each component is the result of apply 'cosh' to that component in 'v'",
	[VECTOR_FN_TANH] = "return a vector where each component is the result of apply 'tanh' to that component in 'v'",
	[VECTOR_FN_ASINH] = "return a vector where each component is the result of apply 'asinh' to that component in 'v'",
	[VECTOR_FN_ACOSH] = "return a vector where each component is the result of apply 'acosh' to that component in 'v'",
	[VECTOR_FN_ATANH] = "return a vector where each component is the result of apply 'atanh' to that component in 'v'",
	[VECTOR_FN_ATAN2] = "return a vector where each component is the result of apply 'atan2' to that component in 'v'",
	[VECTOR_FN_POW] = "return a vector where each component is the result of apply 'pow' to that component in 'v'",
	[VECTOR_FN_EXP] = "return a vector where each component is the result of apply 'exp' to that component in 'v'",
	[VECTOR_FN_LOG] = "return a vector where each component is the result of apply 'log' to that component in 'v'",
	[VECTOR_FN_EXP2] = "return a vector where each component is the result of apply 'exp2' to that component in 'v'",
	[VECTOR_FN_LOG2] = "return a vector where each component is the result of apply 'log2' to that component in 'v'",
	[VECTOR_FN_SQRT] = "return a vector where each component is the result of apply 'sqrt' to that component in 'v'",
	[VECTOR_FN_RSQRT] = "return a vector where each component is the result of apply 'rsqrt' to that component in 'v'",
	[VECTOR_FN_APPROXEQ] = "return a true if each component in 'a' is 'epsilon' away from that component that is in 'b'",
	[VECTOR_FN_APPROXEQS] = "return a true if each component in 'v' is 'epsilon' away from 's'",
	[VECTOR_FN_ISINF] = "return a vector where each component is the result of apply 'isinf' to that component in 'v'",
	[VECTOR_FN_ISNAN] = "return a vector where each component is the result of apply 'isnan' to that component in 'v'",
	[VECTOR_FN_LERP] = "return a vector where each component is the result of apply 'lerp' to that component in 'start', 'end' and 't'",
	[VECTOR_FN_INVLERP] = "return a vector where each component is the result of apply 'invlerp' to that component in 'start', 'end' and 't'",
};

static const char* vector_identifiers[] = {
	[VECTOR_2] = "vec2",
	[VECTOR_3] = "vec3",
	[VECTOR_4] = "vec4",
};

static const char* vector_identifiers_cap[] = {
	[VECTOR_2] = "VEC2",
	[VECTOR_3] = "VEC3",
	[VECTOR_4] = "VEC4",
};

static const char* vector_suffixes[] = {
	[VECTOR_2] = "v2",
	[VECTOR_3] = "v3",
	[VECTOR_4] = "v4",
};

static const char* vector_suffixes_cap[] = {
	[VECTOR_2] = "V2",
	[VECTOR_3] = "V3",
	[VECTOR_4] = "V4",
};

static unsigned vector_comps[] = {
	[VECTOR_2] = 2,
	[VECTOR_3] = 3,
	[VECTOR_4] = 4,
};

static unsigned vector_align_comps[] = {
	[VECTOR_2] = 2,
	[VECTOR_3] = 4,
	[VECTOR_4] = 4,
};

static const char* matrix_identifiers[] = {
	[MATRIX_2x2] = "mat2x2",
	[MATRIX_2x3] = "mat2x3",
	[MATRIX_2x4] = "mat2x4",
	[MATRIX_3x2] = "mat3x2",
	[MATRIX_3x3] = "mat3x3",
	[MATRIX_3x4] = "mat3x4",
	[MATRIX_4x2] = "mat4x2",
	[MATRIX_4x3] = "mat4x3",
	[MATRIX_4x4] = "mat4x4",
};

static const char* matrix_suffixes[] = {
	[MATRIX_2x2] = "m2x2",
	[MATRIX_2x3] = "m2x3",
	[MATRIX_2x4] = "m2x4",
	[MATRIX_3x2] = "m3x2",
	[MATRIX_3x3] = "m3x3",
	[MATRIX_3x4] = "m3x4",
	[MATRIX_4x2] = "m4x2",
	[MATRIX_4x3] = "m4x3",
	[MATRIX_4x4] = "m4x4",
};

static const char* matrix_suffixes_cap[] = {
	[MATRIX_2x2] = "M2X2",
	[MATRIX_2x3] = "M2X3",
	[MATRIX_2x4] = "M2X4",
	[MATRIX_3x2] = "M3X2",
	[MATRIX_3x3] = "M3X3",
	[MATRIX_3x4] = "M3X4",
	[MATRIX_4x2] = "M4X2",
	[MATRIX_4x3] = "M4X3",
	[MATRIX_4x4] = "M4X4",
};

static unsigned matrix_columns_count[] = {
	[MATRIX_2x2] = 2,
	[MATRIX_2x3] = 2,
	[MATRIX_2x4] = 2,
	[MATRIX_3x2] = 3,
	[MATRIX_3x3] = 3,
	[MATRIX_3x4] = 3,
	[MATRIX_4x2] = 4,
	[MATRIX_4x3] = 4,
	[MATRIX_4x4] = 4,
};

static unsigned matrix_rows_count[] = {
	[MATRIX_2x2] = 2,
	[MATRIX_2x3] = 3,
	[MATRIX_2x4] = 4,
	[MATRIX_3x2] = 2,
	[MATRIX_3x3] = 3,
	[MATRIX_3x4] = 4,
	[MATRIX_4x2] = 2,
	[MATRIX_4x3] = 3,
	[MATRIX_4x4] = 4,
};

static DataTypeClass data_type_classes[] = {
	[DATA_TYPE_BOOL] = DATA_TYPE_CLASS_BOOL,
	[DATA_TYPE_HALF] = DATA_TYPE_CLASS_FLOAT,
	[DATA_TYPE_FLOAT] = DATA_TYPE_CLASS_FLOAT,
	[DATA_TYPE_DOUBLE] = DATA_TYPE_CLASS_FLOAT,
	[DATA_TYPE_I8] = DATA_TYPE_CLASS_INT,
	[DATA_TYPE_I16] = DATA_TYPE_CLASS_INT,
	[DATA_TYPE_I32] = DATA_TYPE_CLASS_INT,
	[DATA_TYPE_I64] = DATA_TYPE_CLASS_INT,
	[DATA_TYPE_U8] = DATA_TYPE_CLASS_UINT,
	[DATA_TYPE_U16] = DATA_TYPE_CLASS_UINT,
	[DATA_TYPE_U32] = DATA_TYPE_CLASS_UINT,
	[DATA_TYPE_U64] = DATA_TYPE_CLASS_UINT,
};

static const char* data_type_identifiers[] = {
	[DATA_TYPE_BOOL] = "bool",
	[DATA_TYPE_HALF] = "half",
	[DATA_TYPE_FLOAT] = "float",
	[DATA_TYPE_DOUBLE] = "double",
	[DATA_TYPE_I8] = "int8_t",
	[DATA_TYPE_I16] = "int16_t",
	[DATA_TYPE_I32] = "int32_t",
	[DATA_TYPE_I64] = "int64_t",
	[DATA_TYPE_U8] = "uint8_t",
	[DATA_TYPE_U16] = "uint16_t",
	[DATA_TYPE_U32] = "uint32_t",
	[DATA_TYPE_U64] = "uint64_t",
};

static const char* data_type_suffixes[] = {
	[DATA_TYPE_BOOL] = "bool",
	[DATA_TYPE_HALF] = "h",
	[DATA_TYPE_FLOAT] = "f",
	[DATA_TYPE_DOUBLE] = "d",
	[DATA_TYPE_I8] = "i8",
	[DATA_TYPE_I16] = "i16",
	[DATA_TYPE_I32] = "i32",
	[DATA_TYPE_I64] = "i64",
	[DATA_TYPE_U8] = "u8",
	[DATA_TYPE_U16] = "u16",
	[DATA_TYPE_U32] = "u32",
	[DATA_TYPE_U64] = "u64",
};

static const char* data_type_suffixes_cap[] = {
	[DATA_TYPE_BOOL] = "BOOL",
	[DATA_TYPE_HALF] = "H",
	[DATA_TYPE_FLOAT] = "F",
	[DATA_TYPE_DOUBLE] = "D",
	[DATA_TYPE_I8] = "I8",
	[DATA_TYPE_I16] = "I16",
	[DATA_TYPE_I32] = "I32",
	[DATA_TYPE_I64] = "I64",
	[DATA_TYPE_U8] = "U8",
	[DATA_TYPE_U16] = "U16",
	[DATA_TYPE_U32] = "U32",
	[DATA_TYPE_U64] = "U64",
};

static unsigned data_type_sizes[] = {
	[DATA_TYPE_BOOL] = 1,
	[DATA_TYPE_HALF] = 2,
	[DATA_TYPE_FLOAT] = 4,
	[DATA_TYPE_DOUBLE] = 8,
	[DATA_TYPE_I8] = 1,
	[DATA_TYPE_I16] = 2,
	[DATA_TYPE_I32] = 4,
	[DATA_TYPE_I64] = 8,
	[DATA_TYPE_U8] = 1,
	[DATA_TYPE_U16] = 2,
	[DATA_TYPE_U32] = 4,
	[DATA_TYPE_U64] = 8,
};

const char* half_fn_param_ident(HalfFn fn, unsigned param_idx) {
	unsigned num_params = half_fn_num_params[fn];
	switch (num_params) {
		case 1: return "v";
		case 2:
			switch (fn) {
			case HALF_FN_COPYSIGN: return param_idx ? "sign" : "v";
			case HALF_FN_ATAN2: return param_idx ? "x" : "y";
			default: break;
			}
			return abc_idents[param_idx];
	}
	return NULL;
}

bool is_an_operator(VectorFn fn) {
	if (ctx.data_type == DATA_TYPE_HALF) {
		return false;
	}

	if (vector_fn_c_binary_operators[fn]) {
		if (ctx.data_type == DATA_TYPE_BOOL) {
			return vector_fn_is_bool_operator[fn];
		}
		if (fn == VECTOR_FN_MOD || fn == VECTOR_FN_MODS) {
			return data_type_classes[ctx.data_type] != DATA_TYPE_CLASS_FLOAT;
		}
		return true;
	}

	if (vector_fn_c_unary_operators[fn]) {
		return true;
	}

	return false;
}

bool is_function_compatible(VectorFn fn) {
	switch (data_type_classes[ctx.data_type]) {
		case DATA_TYPE_CLASS_BOOL:
			return VECTOR_FN_TANY_START <= fn && fn <= VECTOR_FN_TANY_END;
		case DATA_TYPE_CLASS_FLOAT:
			return
				(VECTOR_FN_TANY_START <= fn && fn <= VECTOR_FN_TANY_END) ||
				(VECTOR_FN_UIF_START <= fn && fn <= VECTOR_FN_UIF_END)   ||
				(VECTOR_FN_IF_START <= fn && fn <= VECTOR_FN_IF_END)     ||
				(VECTOR_FN_F_START <= fn && fn <= VECTOR_FN_F_END)        ;
		case DATA_TYPE_CLASS_UINT:
			return
				(VECTOR_FN_TANY_START <= fn && fn <= VECTOR_FN_TANY_END) ||
				(VECTOR_FN_UIF_START <= fn && fn <= VECTOR_FN_UIF_END)   ||
				(VECTOR_FN_UI_START <= fn && fn <= VECTOR_FN_UI_END)      ;
		case DATA_TYPE_CLASS_INT:
			return
				(VECTOR_FN_TANY_START <= fn && fn <= VECTOR_FN_TANY_END) ||
				(VECTOR_FN_UIF_START <= fn && fn <= VECTOR_FN_UIF_END)   ||
				(VECTOR_FN_UI_START <= fn && fn <= VECTOR_FN_UI_END)     ||
				(VECTOR_FN_IF_START <= fn && fn <= VECTOR_FN_IF_END)      ;
		default: return false;
	}
}

bool function_param_is_scalar(VectorFn fn, unsigned param_idx) {
	switch (fn) {
	case VECTOR_FN_ADDS:
	case VECTOR_FN_SUBS:
	case VECTOR_FN_MULS:
	case VECTOR_FN_DIVS:
	case VECTOR_FN_MODS:
	case VECTOR_FN_EQS:
	case VECTOR_FN_NEQS:
	case VECTOR_FN_LTS:
	case VECTOR_FN_LTEQS:
	case VECTOR_FN_GTS:
	case VECTOR_FN_GTEQS:
	case VECTOR_FN_MINS:
	case VECTOR_FN_MAXS:
	case VECTOR_FN_CLAMPS:
	case VECTOR_FN_BITANDS:
	case VECTOR_FN_BITORS:
	case VECTOR_FN_BITXORS:
	case VECTOR_FN_BITSHLS:
	case VECTOR_FN_BITSHRS:
	case VECTOR_FN_REMAPS:
	case VECTOR_FN_ROUNDTOMULTIPLES:
	case VECTOR_FN_ROUNDUPTOMULTIPLES:
	case VECTOR_FN_ROUNDDOWNTOMULTIPLES:
		return param_idx;
	default: break;
	}
	return false;
}

const char* function_param_ident(VectorFn fn, unsigned param_idx) {
	switch (fn) {
	case VECTOR_FN_INIT:
	case VECTOR_FN_PINIT:
		return xyzw_idents[param_idx];
	case VECTOR_FN_SWIZZLE: return swizzle_idents[param_idx];
	default: break;
	}

	unsigned num_params = vector_fn_number_params[fn];
	switch (num_params) {
		case 0: return xyzw_idents[param_idx];
		case 1: return "v";
		case 2:
			if (vector_fn_has_scalar_params[fn]) {
				return param_idx ? "s" : "v";
			}
			switch (fn) {
			case VECTOR_FN_COPYSIGN: return param_idx ? "sign" : "v";
			case VECTOR_FN_ATAN2: return param_idx ? "x" : "y";
			default: break;
			}
			return abc_idents[param_idx];
		case 3:
			switch (fn) {
			case VECTOR_FN_CLAMP:
			case VECTOR_FN_CLAMPS: return clamp_idents[param_idx];
			case VECTOR_FN_FMA: return abc_idents[param_idx];
			case VECTOR_FN_LERP: return lerp_idents[param_idx];
			case VECTOR_FN_INVLERP: return invlerp_idents[param_idx];
			default: break;
			}
			break;
		case 5:
			switch (fn) {
			case VECTOR_FN_REMAP:
			case VECTOR_FN_REMAPS: return remap_idents[param_idx];
			default: break;
			}
			break;
	}
	return NULL;
}

void print_section_header(char* title, char* comment) {
	fprintf(ctx.f,
	"// ===========================================\n"
	"//\n"
	"//\n"
	"// %s\n"
	"//\n"
	"//\n"
	"// ===========================================\n"
	"%s\n", title, comment
	);
}

void print_section_header_libc_ext() {
	print_section_header("Libc Math extensions", "");
}

void print_section_header_half() {
	print_section_header("Half type aka. float 16 bit", "");
}

void print_section_header_scalar() {
	print_section_header("Scalar Math", "");
}

void print_section_header_packed_vector() {
	print_section_header(
		"Packed Vector",
		"//\n"
		"// these packed vectors have alignment of their component type.\n"
		"// they should only be used to transport vectors between CPU and GPU if the native alignment adds too much padding\n"
		"//\n"
	);
}

void print_section_header_vector() {
	print_section_header(
		"Vector",
		"//\n"
		"// these vectors have native alignment where their alignment is the same as their size\n"
		"// these vectors should be used when performing maths operations\n"
		"// the vec3 is rounded up to the size and align of a vec4 due to hardware limitations\n"
		"//\n"
	);
}

void print_section_header_matrix() {
}

void print_vector_fn_docs(VectorFn fn) {
	fprintf(ctx.f,"\n//\n// %s\n", vector_fn_docs[fn]);
}

void print_entry(const char* string) {
	const char* prev_special = string, *next_special = string;
	while ((next_special = strchr(next_special, '$'))) {
		unsigned size = next_special - prev_special;
		fprintf(ctx.f,"%.*s", size, prev_special);
		next_special += 1; // skip '$'
		switch (*next_special) {
		case 'd':
			next_special += 1; // skip 'd'
			switch (*next_special) {
			case 'i':
				fprintf(ctx.f,"%s", data_type_identifiers[ctx.data_type]);
				break;
			case 'I':
				fprintf(ctx.f,"%-8s", data_type_identifiers[ctx.data_type]);
				break;
			case 'x':
				fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
				break;
			case 'X':
				fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
				break;
			case 'b':
				fprintf(ctx.f,"%u", data_type_sizes[ctx.data_type] * 8);
				break;
			case 'm':
				switch (ctx.data_type) {
					case DATA_TYPE_I8: fprintf(ctx.f,"0x80"); break;
					case DATA_TYPE_I16: fprintf(ctx.f,"0x8000"); break;
					case DATA_TYPE_I32: fprintf(ctx.f,"0x800000"); break;
					case DATA_TYPE_I64: fprintf(ctx.f,"0x80000000"); break;
					default: break;
				}
				break;
			}
			break;
		case 'v':
			next_special += 1; // skip 'v'
			switch (*next_special) {
				case 'a':
					fprintf(ctx.f,"%u", vector_align_comps[ctx.vector] * data_type_sizes[ctx.data_type]);
					break;
				case 'b':
					fprintf(ctx.f,"%s", vector_identifiers[ctx.vector]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type + 4]);
					break;
				case 'B':
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type + 4]);
					break;
				case 'c':
					fprintf(ctx.f,"%u", vector_comps[ctx.vector]);
					break;
				case 'i':
					fprintf(ctx.f,"%s", vector_identifiers[ctx.vector]);
					break;
				case 'I':
					fprintf(ctx.f,"%s", vector_identifiers_cap[ctx.vector]);
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					break;
				case '2':
					fprintf(ctx.f,"%s", vector_identifiers[VECTOR_2]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					break;
				case '3':
					fprintf(ctx.f,"%s", vector_identifiers[VECTOR_3]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					break;
				case 's':
					fprintf(ctx.f,"%s", vector_identifiers[ctx.vector]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					break;
				case 'S':
					fprintf(ctx.f,"%s", vector_identifiers[ctx.vector]);
					fprintf(ctx.f,"%-4s", data_type_suffixes[ctx.data_type]);
					break;
				case 'f':
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					break;
				case 'x':
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					break;
				case 'X':
					fprintf(ctx.f,"%s", vector_suffixes_cap[ctx.vector]);
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					break;
				case 'z':
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					fprintf(ctx.f,"%-4s", data_type_suffixes[ctx.data_type]);
					break;
				case 'Z':
					fprintf(ctx.f,"%s", vector_suffixes_cap[ctx.vector]);
					fprintf(ctx.f,"%-4s", data_type_suffixes_cap[ctx.data_type]);
					break;
			}
			break;
		case 'm':
			next_special += 1; // skip 'm'
			switch (*next_special) {
				case 't':
					fprintf(ctx.f,"%s", matrix_identifiers[ctx.matrix]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					break;
				case 'c':
					fprintf(ctx.f,"%u", matrix_columns_count[ctx.matrix]);
					break;
				case 'r':
					fprintf(ctx.f,"%u", matrix_rows_count[ctx.matrix]);
					break;
				case 's':
					fprintf(ctx.f,"%-2u", matrix_columns_count[ctx.matrix] * matrix_rows_count[ctx.matrix]);
					break;
				case 'S':
					fprintf(ctx.f,"%-2u", 4 * matrix_rows_count[ctx.matrix]);
					break;
				case 'v':
					fprintf(ctx.f,"%s%s", vector_identifiers[ctx.matrix / 3], data_type_suffixes[ctx.data_type]);
					break;
				case 'V':
					fprintf(ctx.f,"%s%s", vector_identifiers[ctx.matrix % 3], data_type_suffixes[ctx.data_type]);
					break;
				case 'x':
					fprintf(ctx.f,"%s", matrix_suffixes[ctx.matrix]);
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					break;
				case 'X':
					fprintf(ctx.f,"%s", matrix_suffixes_cap[ctx.matrix]);
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					break;
			}
			break;
		}

		next_special += 1;
		prev_special = next_special;
	}
	fprintf(ctx.f,"%s", prev_special);
}

FILE* open_file_write(const char* path) {
	FILE* f = fopen(path, "w");
	if (f == NULL) {
		fprintf(stderr, "failed to open file at '%s'\n", path);
		exit(1);
	}
	return f;
}

void print_file_header() {
	fprintf(ctx.f,
		"// !?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!\n"
		"// !?!?!? WARNING CONTRIBUTOR ?!?!?!\n"
		"// !?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!\n"
		"// this file is generated by tools/std_math_gen.c\n"
		"// please edit that file and regenerate this one if you want to make edits\n"
		"\n"
	);
}

void print_header_file_footer(const char* guard) {
	fprintf(ctx.f, "\n#endif // %s\n" , guard);
}

void generate_math_types_header_file() {
	ctx.f = open_file_write("libhccstd/math_types.h");
	print_file_header();
	fprintf(ctx.f,
		"#ifndef _HCC_STD_MATH_TYPES_H_\n"
		"#define _HCC_STD_MATH_TYPES_H_\n"
	);

	print_section_header_libc_ext();
	fprintf(ctx.f,
		"#define INFINITYF INFINITY\n"
		"#define INFINITYD INFINITY\n"
		"#define NEGINFINITYF (-INFINITY)\n"
		"#define NEGINFINITYD (-INFINITY)\n"
		"#define NANF NAN\n"
		"#define NAND NAN\n"
		"\n"
	);

	print_section_header_half();
	fprintf(ctx.f,
		"typedef struct half { uint16_t _bits; } half;\n"
		"#define ZEROH ((half){ _bits = 0; })\n"
		"#define INFINITYH ((half){ _bits = 0x7c00; })\n"
		"#define NEGINFINITYH ((half){ _bits = 0xfc00; })\n"
		"#define NANH ((half){ _bits = 0xffff; })\n"
		"\n"
	);

	print_section_header_packed_vector();
	//
	// all of the packed vector types
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef struct p$vS { $dI x; $dI y; } p$vs;\n");
		}

		fprintf(ctx.f,"\n");

		ctx.vector = VECTOR_3;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef struct p$vS { $dI x; $dI y; $dI z; } p$vs;\n");
		}

		fprintf(ctx.f,"\n");

		ctx.vector = VECTOR_4;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef struct p$vS { $dI x; $dI y; $dI z; $dI w; } p$vs;\n");
		}
	}

	//
	// packed vector constructors ewww
	{
		print_vector_fn_docs(VECTOR_FN_PINIT);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define p$vz(x, y)       ((p$vS){ { x, y } })\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define p$vz(x, y, z)    ((p$vS){ { x, y, z } })\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define p$vz(x, y, z, w) ((p$vS){ { x, y, z, w } })\n");
		}

		fprintf(ctx.f,"\n");
	}


	print_section_header_vector();
	for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
		ctx.vector = vector;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef union $vS $vs;\n");
		}
		fprintf(ctx.f,"\n");
	}

	ctx.vector = VECTOR_2;
	for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"HCC_INTRINSIC union $vs {\n"
				"\t_Alignas($va)\n"
				"\tstruct { $di x; $di y; };\n"
				"\tstruct { $di r; $di g; };\n"
				"\tstruct { $di width; $di height; };\n"
				"\t$di array[2];\n"
			"};\n\n");
	}

	ctx.vector = VECTOR_3;
	for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"HCC_INTRINSIC union $vs {\n"
				"\t_Alignas($va)\n"
				"\tstruct { $di x; $di y; $di z; $di _w; };\n"
				"\tstruct { $di r; $di g; $di b; $di _a; };\n"
				"\t$v2 xy;\n"
				"\t$v2 rg;\n"
				"\t$di array[4];\n"
			"};\n\n");
	}

	ctx.vector = VECTOR_4;
	for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"HCC_INTRINSIC union $vs {\n"
				"\t_Alignas($va)\n"
				"\tstruct { $di x; $di y; $di z; $di w; };\n"
				"\tstruct { $di r; $di g; $di b; $di a; };\n"
				"\tstruct { $v2 top_left; $v2 bottom_right; };\n"
				"\tstruct { $v2 bottom_left; $v2 top_right; };\n"
				"\tstruct { $v2 xy; $v2 zw; };\n"
				"\tstruct { $v2 rg; $v2 ba; };\n"
				"\tstruct { $di _; $di __; $di width; $di height; };\n"
				"\t$v3 xyz;\n"
				"\t$v3 rgb;\n"
				"\t$di array[4];\n"
			"};\n");
	}

	//
	// vector constructors ewww
	{
		print_vector_fn_docs(VECTOR_FN_INIT);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define $vz(x, y)       (($vS){ { x, y } })\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define $vz(x, y, z)    (($vS){ { x, y, z } })\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define $vz(x, y, z, w) (($vS){ { x, y, z, w } })\n");
		}
	}
	fprintf(ctx.f,"\n");

	//
	// vector zero
	{
		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
				ctx.data_type = data_type;
				print_entry("#define ZERO$vZ (($vS){0})\n");
			}
		}
	}
	fprintf(ctx.f,"\n");

	//
	// vector infinity
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define INFINITY$vX $vx(INFINITY$dX, INFINITY$dX)\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define INFINITY$vX $vx(INFINITY$dX, INFINITY$dX, INFINITY$dX)\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define INFINITY$vX $vx(INFINITY$dX, INFINITY$dX, INFINITY$dX, INFINITY$dX)\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// vector neginfinity
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NEGINFINITY$vX $vx(NEGINFINITY$dX, NEGINFINITY$dX)\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NEGINFINITY$vX $vx(NEGINFINITY$dX, NEGINFINITY$dX, NEGINFINITY$dX)\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NEGINFINITY$vX $vx(NEGINFINITY$dX, NEGINFINITY$dX, NEGINFINITY$dX, NEGINFINITY$dX)\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// vector nan
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NAN$vX $vx(NAN$dX, NAN$dX)\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NAN$vX $vx(NAN$dX, NAN$dX, NAN$dX)\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NAN$vX $vx(NAN$dX, NAN$dX, NAN$dX, NAN$dX)\n");
		}
		fprintf(ctx.f,"\n");
	}

	print_section_header(
		"Packed Matrix",
		""
	);

	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC typedef union p$mt { $dI cols[$mc][$mr]; p$mv vcols[$mc]; $dI scalars[$ms]; } p$mt;\n");
		}
	}
	fprintf(ctx.f,"\n");

	print_section_header(
		"Matrix",
		""
	);

	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC typedef union $mt { $dI cols[4][$mr]; vec4$dx vcols[$mr]; $dI scalars[$mS]; } $mt;\n");
		}
	}
	fprintf(ctx.f,"\n");

	{
		for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
			ctx.matrix = matrix;
			unsigned cols = matrix / 3 + 2;
			unsigned rows = matrix % 3 + 2;
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				if (cols == 4 && rows == 4) {
					print_entry("#define IDENTITY$mX (($mt) { .cols[0][0] = 1.0, .cols[1][1] = 1.0, .cols[2][2] = 1.0, .cols[3][3] = 1.0 })\n");
				} else if (cols >= 3 && rows >= 3) {
					print_entry("#define IDENTITY$mX (($mt) { .cols[0][0] = 1.0, .cols[1][1] = 1.0, .cols[2][2] = 1.0 })\n");
				} else {
					print_entry("#define IDENTITY$mX (($mt) { .cols[0][0] = 1.0, .cols[1][1] = 1.0 })\n");
				}
			}
		}
	}

	print_header_file_footer("_HCC_STD_MATH_TYPES_H_");
}

void generate_math_header_file() {
	ctx.f = open_file_write("libhccstd/math.h");
	print_file_header();
	fprintf(ctx.f,
		"#ifndef _HCC_STD_MATH_H_\n"
		"#define _HCC_STD_MATH_H_\n"
	);
	fprintf(ctx.f,"#include \"math_types.h\"\n");

	print_section_header("Misc", "");

	//
	// swizzle operands
	{
		for (unsigned x = 0; x < 2; x += 1) {
			for (unsigned y = 0; y < 2; y += 1) {
				fprintf(ctx.f,"#define %s%s %u, %u\n", xyzw_idents[x], xyzw_idents[y], x, y);
			}
		}

		for (unsigned x = 0; x < 3; x += 1) {
			for (unsigned y = 0; y < 3; y += 1) {
				for (unsigned z = 0; z < 3; z += 1) {
					fprintf(ctx.f,"#define %s%s%s %u, %u, %u\n", xyzw_idents[x], xyzw_idents[y], xyzw_idents[z], x, y, z);
				}
			}
		}

		for (unsigned x = 0; x < 4; x += 1) {
			for (unsigned y = 0; y < 4; y += 1) {
				for (unsigned z = 0; z < 4; z += 1) {
					for (unsigned w = 0; w < 4; w += 1) {
						fprintf(ctx.f,"#define %s%s%s%s %u, %u, %u, %u\n", xyzw_idents[x], xyzw_idents[y], xyzw_idents[z], xyzw_idents[w], x, y, z, w);
					}
				}
			}
		}
		fprintf(ctx.f,"\n");
	}

	print_section_header_libc_ext();
	fprintf(ctx.f,
		"#define isinff isinf\n"
		"#define isinfd isinf\n"
		"#define isnanf isnan\n"
		"#define isnand isnan\n"
		"#define fmodd fmod\n"
		"#define copysignd copysign\n"
		"#define fabsd fabs\n"
		"#define floord floor\n"
		"#define ceild ceil\n"
		"#define roundd round\n"
		"#define truncd trunc\n"
		"#define sind sin\n"
		"#define cosd cos\n"
		"#define tand tan\n"
		"#define asind asin\n"
		"#define acosd acos\n"
		"#define atand atan\n"
		"#define sinhd sinh\n"
		"#define coshd cosh\n"
		"#define tanhd tanh\n"
		"#define asinhd asinh\n"
		"#define acoshd acosh\n"
		"#define atanhd atanh\n"
		"#define atan2d atan2\n"
		"#define fmad fma\n"
		"#define powd pow\n"
		"#define expd exp\n"
		"#define logd log\n"
		"#define exp2d exp2\n"
		"#define log2d log2\n"
		"\n"
	);

	print_section_header_half();
	fprintf(ctx.f,
		"HCC_INTRINSIC float htof(half v);\n"
		"HCC_INTRINSIC double htod(half v);\n"
		"HCC_INTRINSIC half ftoh(float v);\n"
		"HCC_INTRINSIC half dtoh(double v);\n"

		"HCC_INTRINSIC static inline bool isinfh(half v) { (v._bits & 0x7c00) == 0x7c00 && (v._bits & 0x03ff) == 0; }\n"
		"HCC_INTRINSIC static inline bool isnanh(half v) { (v._bits & 0x7c00) == 0x7c00 && v._bits & 0x03ff; }\n"
	);

	for (HalfFn fn = 0; fn < HALF_FN_COUNT; fn += 1) {
		const char* return_type;
		if (half_fn_returns_bool[fn]) {
			return_type = "bool";
		} else {
			return_type = "half";
		}

		bool is_operator = half_fn_c_unary_operators[fn] || half_fn_c_binary_operators[fn];

		const char* ident = half_fn_ident[fn];
		fprintf(ctx.f,"HCC_INTRINSIC %s %sh(", return_type, ident);
		for (unsigned param_idx = 0; param_idx < half_fn_num_params[fn]; param_idx += 1) {
			fprintf(ctx.f,"half ");
			print_entry(half_fn_param_ident(fn, param_idx));
			if (param_idx + 1 < half_fn_num_params[fn]) {
				print_entry(", ");
			}
		}
		fprintf(ctx.f,") { return ");
		if (!half_fn_returns_bool[fn]) {
			fprintf(ctx.f,"ftoh(");
		}

		if (!is_operator) {
			fprintf(ctx.f,"%sf(", ident);
		}

		for (unsigned param_idx = 0; param_idx < half_fn_num_params[fn]; param_idx += 1) {
			const char* ident = half_fn_param_ident(fn, param_idx);
			if (half_fn_c_unary_operators[fn]) {
				fprintf(ctx.f,"%s", half_fn_c_unary_operators[fn]);
			}
			fprintf(ctx.f,"htof(%s)", ident);
			if (param_idx + 1 < half_fn_num_params[fn]) {
				if (half_fn_c_binary_operators[fn]) {
 					fprintf(ctx.f," %s ", half_fn_c_binary_operators[fn]);
				} else {
					print_entry(", ");
				}
			}
		}
		if (!half_fn_returns_bool[fn]) {
			fprintf(ctx.f,")");
		}
		if (!is_operator) {
			fprintf(ctx.f,")");
		}
		fprintf(ctx.f,"; }\n");
	}

	fprintf(ctx.f,"\n");

	print_section_header_scalar();
	
	//
	// scalar min
	{
		fprintf(ctx.f,
			"//\n"
			"// returns the minimum value between 'a' and 'b'\n"
		);
		print_entry("HCC_INTRINSIC static inline half minh(half a, half b) { return lth(a, b) ? a : b; }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di min$dx($di a, $di b) { return a < b ? a : b; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar max
	{
		fprintf(ctx.f,
			"//\n"
			"// returns the maximum value between 'a' and 'b'\n"
		);
		print_entry("HCC_INTRINSIC static inline half maxh(half a, half b) { return gth(a, b) ? a : b; }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di max$dx($di a, $di b) { return a > b ? a : b; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar clamp
	{
		fprintf(ctx.f,
			"//\n"
			"// clamps 'v' so that it is inbetween 'a' and 'b'\n"
		);
		print_entry("HCC_INTRINSIC static inline half clamph(half v, half min, half max) { return gth(v, max) ? max : (gteqh(v, min) ? v : min); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di clamp$dx($di v, $di min, $di max) { return v > max ? max : (v >= min ? v : min); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar sign
	{
		fprintf(ctx.f,
			"//\n"
			"// returns the reciprocal square root of 'v' aka. inverse square root\n"
		);
		print_entry("HCC_INTRINSIC static inline half rsqrth(half v) { return ftoh(1.f / sqrtf(htof(v))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di rsqrt$dx($di v) { return 1.f / sqrt$dx(v); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar sign
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a 'v' with sign copied from 'sign'\n"
		);
		for (DataType data_type = DATA_TYPE_I8; data_type <= DATA_TYPE_I64; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di copysign$dx($di v, $di sign) { return v | (sign & $dm); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar lerp
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a linear interpolation from 'start' to 'end' at the point of 't' where 't' = 0.0 = 'start' and 't' = 1.0 = 'end'\n"
		);
		print_entry("HCC_INTRINSIC static inline half lerph(half start, half end, half t) { return addh(mulh(subh(end, start), t) + start); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di lerp$dx($di start, $di end, $di t) { return (end - start) * t + start; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar inv lerp
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a value from 0.0 to 1.0 at the point where 'v' is in relation to 'start' and 'end' where 'v' = 0.0 = 'start' and 'v' = 1.0 = 'end'\n"
		);
		print_entry("HCC_INTRINSIC static inline half invlerph(half start, half end, half v) { return divh(subh(value, start), subh(end, start)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di invlerp$dx($di start, $di end, $di v) { return (value - start) / (end - start); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar fract
	{
		fprintf(ctx.f,
			"//\n"
			"// returns the fractional part of a 'v'\n"
		);
		print_entry("HCC_INTRINSIC static inline half fracth(half v) { return subh(v, floorh(v)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di fract$dx($di v) { return v - floor$dx(v); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar degrees
	{
		fprintf(ctx.f,
			"//\n"
			"// converts 'v' radians to degrees\n"
		);
		print_entry("HCC_INTRINSIC static inline half degreesh(half v) { return ftoh(htof(v) * (180.0 / M_PI)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di degrees$dx($di v) { return v * (180.0 / M_PI); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar radians
	{
		fprintf(ctx.f,
			"//\n"
			"// converts 'v' degrees to radians\n"
		);
		print_entry("HCC_INTRINSIC static inline half radiansh(half v) { return ftoh(htof(v) * (M_PI / 180.0)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di radians$dx($di v) { return v * (M_PI / 180.0); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar step
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 0.0 if 'v' < 'edge', otherwise 1.0 is returned\n"
		);
		print_entry("HCC_INTRINSIC static inline half steph(half edge, half v) { return ftoh(htof(v) ? 0.0 : 1.0); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di step$dx($di edge, $di v) { return v < edge ? 0.0 : 1.0; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar smoothstep
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a smooth Hermite interpolation between 0.0 and 1.0 when 'edge0' < 'x' < 'edge1'\n"
		);
		print_entry("HCC_INTRINSIC static inline half smoothsteph(half edge, half v) { return ftoh($di t = (htof(value) - htof(start)) / (htof(end) - htof(start)); return t * t * (3.0 - 2.0 * t)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di smoothstep$dx($di edge0, $di edge1, $di v) { $di t = (value - start) / (end - start); return t * t * (3.0 - 2.0 * t); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar remap
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 'v' remapped from a range of 'from_min' to 'from_max' to the range of 'to_min' to 'to_max'\n"
		);
		print_entry("HCC_INTRINSIC static inline half remaph(half v, half from_min, half from_max, half to_min, half to_max) { return addh(to_min, divh(mulh(subh(v, from_min), subh(to_max, to_min)), subh(from_max, from_min))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di remap$dx($di v, $di from_min, $di from_max, $di to_min, $di to_max) { return to_min + (v - from_min) * (to_max - to_min) / (from_max - from_min); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar roundtomultiple
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 'v' rounded to the nearest 'multiple'\n"
		);
		print_entry("HCC_INTRINSIC static inline half roundtomultipleh(half v, half multiple) { v = fmah(multiple, ftoh(0.5), v); float rem = fmod%di(v, multiple); if (gth(v, 0.0)) { return subh(v, rem); } else { return subh(subh(v, rem), multiple); } }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di roundtomultiple$dx($di v, $di multiple) { v += multiple * 0.5; float rem = fmod%di(v, multiple); if (v > 0.0) { return v - rem; } else { return v - rem - multiple; } }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar rounduptomultiple
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 'v' rounded _up_ to the nearest 'multiple'\n"
		);
		print_entry("HCC_INTRINSIC static inline half rounduptomultipleh(half v, half multiple) { float rem = fmod%di(v, multiple); if (gth(v, 0.0)) { return subh(addh(v, multiple), rem); } else { return subh(v, rem); } }; }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di rounduptomultiple$dx($di v, $di multiple) { mod%di(v, multiple); if (v > 0.0) { return v + multiple - rem; } else { return v - rem; } }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar rounddowntomultiple
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 'v' rounded _down_ to the nearest 'multiple'\n"
		);
		print_entry("HCC_INTRINSIC static inline half rounddowntomultipleh(half v, half multiple) { float rem = fmod%di(v, multiple); if (gth(v, 0.0)) { return subh(v, rem); } else { return subh(subh(v, rem), multiple); } }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di rounddowntomultiple$dx($di v, $di multiple) { mod%di(v, multiple); if (v > 0.0) { return v - rem; } else { return v - rem - multiple; } }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar bitsto
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 'v' bitcasted into a float from an integer, no convertion is performed\n"
		);
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di bitsto$dx(uint$db_t v) { union { uint$db_t u; $di f; } d = { .u = v }; return d.f; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar bitsfrom
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 'v' bitcasted into an integer from a float, no convertion is performed\n"
		);
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline uint$db_t bitsfrom$dx($di v) { union { uint$db_t u; $di f; } d = { .f = v }; return d.u; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	print_section_header_vector();
	fprintf(ctx.f,
		"#ifdef __HCC__\n"
		"#define castv2(T, v) __hcc_castv2(vec2##T, v)\n"
		"#define castv3(T, v) __hcc_castv3(vec3##T, v)\n"
		"#define castv4(T, v) __hcc_castv4(vec4##T, v)\n"
		"#else\n"
		"#define castv2(T, v) v2##T((v).x, (v).y)\n"
		"#define castv3(T, v) v3##T((v).x, (v).y, (v).z)\n"
		"#define castv4(T, v) v4##T((v).x, (v).y, (v).z, (v).w)\n"
		"#endif\n"
	);

	//
	// vector swizzle
	{
		print_vector_fn_docs(VECTOR_FN_SWIZZLE);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs swizzle$vx($vs v, uint8_t x, uint8_t y) { return $vx(v.array[x], v.array[y]); }; }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs swizzle$vx($vs v, uint8_t x, uint8_t y, uint8_t z) { return $vx(v.array[x], v.array[y], v.array[z]); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs swizzle$vx($vs v, uint8_t x, uint8_t y, uint8_t z, uint8_t w) { return $vx(v.array[x], v.array[y], v.array[z], v.array[w]); }\n");
		}
	}

	for (VectorFn fn = 0; fn < VECTOR_FN_COUNT; fn += 1) {
		bool has_compatible = false;
		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
				ctx.data_type = data_type;
				if (is_function_compatible(fn)) {
					has_compatible = true;
					break;
				}
			}
		}

		if (!has_compatible) {
			continue;
		}

		print_vector_fn_docs(fn);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
				ctx.data_type = data_type;
				if (!is_function_compatible(fn)) {
					continue;
				}
				print_entry("HCC_INTRINSIC static inline ");

				if (vector_fn_return_vector_bool[fn]) {
					print_entry("$vibool ");
				} else if (vector_fn_returns_bool[fn]) {
					print_entry("bool ");
				} else if (fn == VECTOR_FN_BITSFROM) {
					print_entry("$vb ");
				} else if (fn == VECTOR_FN_PACK) {
					print_entry("p$vs ");
				} else {
					print_entry("$vs ");
				}

				const char* fn_ident = vector_fn_idents[fn];
				print_entry(fn_ident);
				print_entry("$vx(");
				for (unsigned param_idx = 0; param_idx < vector_fn_number_params[fn]; param_idx += 1) {
					if (function_param_is_scalar(fn, param_idx)) {
						print_entry("$di ");
					} else if (fn == VECTOR_FN_BITSTO) {
						print_entry("$vb ");
					} else if (fn == VECTOR_FN_UNPACK) {
						print_entry("p$vs ");
					} else {
						print_entry("$vs ");
					}
					print_entry(function_param_ident(fn, param_idx));
					if (param_idx + 1 < vector_fn_number_params[fn]) {
						print_entry(", ");
					}
				}
				print_entry(")");

				{
					fprintf(ctx.f," { ");

					if (vector_fn_return_vector_bool[fn]) {
						print_entry("$vfbool(");
					} else if (fn == VECTOR_FN_BITSFROM) {
						print_entry("$vB(");
					} else if (fn == VECTOR_FN_PACK) {
						print_entry("p$vx(");
					} else if (!vector_fn_returns_bool[fn]) {
						print_entry("$vx(");
					}

					unsigned num_comp = vector_comps[vector];
					for (unsigned comp = 0; comp < num_comp; comp += 1) {
						if (vector_fn_no_operator_or_calls[fn]) {
							print_entry(function_param_ident(fn, 0));
							print_entry(".");
							print_entry(xyzw_idents[comp]);
						} else if (is_an_operator(fn)) {
							switch (vector_fn_number_params[fn]) {
								case 2:
									print_entry(function_param_ident(fn, 0));
									print_entry(".");
									print_entry(xyzw_idents[comp]);
									print_entry(" ");
									print_entry(vector_fn_c_binary_operators[fn]);
									print_entry(" ");
									print_entry(function_param_ident(fn, 1));
									if (!function_param_is_scalar(fn, 1)) {
										print_entry(".");
										print_entry(xyzw_idents[comp]);
									}
									break;
								case 1:
									print_entry(vector_fn_c_unary_operators[fn]);
									print_entry(function_param_ident(fn, 0));
									print_entry(".");
									print_entry(xyzw_idents[comp]);
									break;
							}
						} else {
							const char* call_ident = vector_fn_has_scalar_params[fn] ? vector_fn_idents[fn - 1] : fn_ident;
							print_entry(call_ident);
							print_entry("$dx(");
							for (unsigned param_idx = 0; param_idx < vector_fn_number_params[fn]; param_idx += 1) {
								print_entry(function_param_ident(fn, param_idx));
								if (!function_param_is_scalar(fn, param_idx)) {
									print_entry(".");
									print_entry(xyzw_idents[comp]);
								}
								if (param_idx + 1 < vector_fn_number_params[fn]) {
									print_entry(", ");
								}
							}
							print_entry(")");
						}
						if (comp + 1 < num_comp) {
							if (vector_fn_returns_bool[fn]) {
								print_entry(vector_fn_is_logical_or[fn] ? " || " : " && ");
							} else {
								print_entry(", ");
							}
						}
					}

					if (vector_fn_return_vector_bool[fn] || !vector_fn_returns_bool[fn]) {
						print_entry(")");
					}

					fprintf(ctx.f,"; }");
				}

				fprintf(ctx.f,";\n");
			}
		}
	}

	//
	// vector dot
	{
		print_vector_fn_docs(VECTOR_FN_DOT);

		ctx.vector = VECTOR_2;
		print_entry("HCC_INTRINSIC static inline half dotv2h(vec2h a, vec2h b) { return addh(mulh(a.x, b.x), mulh(a.y, b.y)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di dot$vx($vs a, $vs b { return (a.x * b.x) + (a.y * b.y); }\n");
		}
		ctx.vector = VECTOR_3;
		print_entry("HCC_INTRINSIC static inline half dotv3h(vec3h a, vec3h b) { return addh(mulh(a.x, b.x), addh(mulh(a.y, b.y), mulh(a.z, b.z))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di dot$vx($vs a, $vs b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }\n");
		}
		ctx.vector = VECTOR_4;
		print_entry("HCC_INTRINSIC static inline half dotv4h(vec4h a, vec4h b) { return addh(mulh(a.x, b.x), addh(mulh(a.y, b.y), addh(mulh(a.z, b.z), mulh(a.w, b.w)))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di dot$vx($vs a, $vs b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w); }\n");

		}
	}

	//
	// vector len
	{
		print_vector_fn_docs(VECTOR_FN_LEN);

		ctx.vector = VECTOR_2;
		print_entry("HCC_INTRINSIC static inline half lenv2h(vec2h v) { return sqrth(addh(mulh(v.x, v.x), mulh(v.y, v.y))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di len$vx($vs v) { return sqrt$dx((v.x * v.x) + (v.y * v.y)); }\n");
		}
		ctx.vector = VECTOR_3;
		print_entry("HCC_INTRINSIC static inline half lenv3h(vec3h v) { return sqrth(addh(mulh(v.x, v.x), addh(mulh(v.y, v.y), mulh(v.z, v.z)))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di len$vx($vs v) { return sqrt$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z)); }\n");
		}
		ctx.vector = VECTOR_4;
		print_entry("HCC_INTRINSIC static inline half lenv4h(vec4h v) { return sqrth(addh(mulh(v.x, v.x), addh(mulh(v.y, v.y), addh(mulh(v.z, v.z), mulh(v.w, v.w))))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di len$vx($vs v) { return sqrt$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w)); }\n");

		}
	}

	//
	// vector lensq
	{
		print_vector_fn_docs(VECTOR_FN_LENSQ);

		ctx.vector = VECTOR_2;
		print_entry("HCC_INTRINSIC static inline half lensqv2h(vec2h v) { return addh(mulh(v.x, v.x), mulh(v.y, v.y)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di lensq$vx($vs v) { return (v.x * v.x) + (v.y * v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		print_entry("HCC_INTRINSIC static inline half lensqv3h(vec3h v) { return addh(mulh(v.x, v.x), addh(mulh(v.y, v.y), mulh(v.z, v.z))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di lensq$vx($vs v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z); }\n");
		}
		ctx.vector = VECTOR_4;
		print_entry("HCC_INTRINSIC static inline half lensqv4h(vec4h v) { return addh(mulh(v.x, v.x), addh(mulh(v.y, v.y), addh(mulh(v.z, v.z), mulh(v.w, v.w)))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di lensq$vx($vs v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w); }\n");

		}
	}

	//
	// vector norm
	{
		print_vector_fn_docs(VECTOR_FN_NORM);

		ctx.vector = VECTOR_2;
		print_entry("HCC_INTRINSIC static inline vec2h normv2h(vec2h v) { half k = rsqrth(addh(mulh(v.x, v.x), mulh(v.y, v.y))); return v2h(mulh(v.x, k), mulh(v.y, k)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs norm$vx($vs v) { $di k = rsqrt$dx((v.x * v.x) + (v.y * v.y)); return $vx(v.x * k, v.y * k); }\n");
		}
		ctx.vector = VECTOR_3;
		print_entry("HCC_INTRINSIC static inline vec3h normv3h(vec3h v) { half k = rsqrth(addh(mulh(v.x, v.x), addh(mulh(v.y, v.y), mulh(v.z, v.z)))); return v3h(mulh(v.x, k), mulh(v.y, k), mulh(v.z, k)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs norm$vx($vs v) { $di k = rsqrt$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z)); return $vx(v.x * k, v.y * k, v.z * k); }\n");
		}
		ctx.vector = VECTOR_4;
		print_entry("HCC_INTRINSIC static inline vec4h normv4h(vec4h v) { half k = rsqrth(addh(mulh(v.x, v.x), addh(mulh(v.y, v.y), addh(mulh(v.z, v.z), mulh(v.w, v.w))))); return v4h(mulh(v.x, k), mulh(v.y, k), mulh(v.z, k), mulh(v.w, k)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs norm$vx($vs v) { $di k = rsqrt$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w)); return $vx(v.x * k, v.y * k, v.z * k, v.w * k); }\n");

		}
	}

	//
	// vector reflect
	{
		print_vector_fn_docs(VECTOR_FN_REFLECT);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			ctx.data_type = DATA_TYPE_HALF;
			print_entry("HCC_INTRINSIC $vs reflect$vx($vs v, $vs normal);\n");
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry("HCC_INTRINSIC $vs reflect$vx($vs v, $vs normal);\n");
			}
		}
	}

	//
	// vector reflect
	{
		print_vector_fn_docs(VECTOR_FN_REFRACT);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			ctx.data_type = DATA_TYPE_HALF;
			print_entry("HCC_INTRINSIC $vs refract$vx($vs v, $vs normal, float eta);\n");
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry("HCC_INTRINSIC $vs refract$vx($vs v, $vs normal, float eta);\n");
			}
		}
	}

	//
	// vector minelmt
	{
		print_vector_fn_docs(VECTOR_FN_MIN_ELMT);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di minelmt$vx($vs v) return min$dx(v.x, v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di minelmt$vx($vs v) return min$dx(v.x, min$dx(v.y, v.z)); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di minelmt$vx($vs v) return min$dx(v.x, min$dx(v.y, min$dx(v.z, v.w))); }\n");
		}
	}

	//
	// vector maxelmt
	{
		print_vector_fn_docs(VECTOR_FN_MAX_ELMT);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di maxelmt$vx($vs v) return max$dx(v.x, v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di maxelmt$vx($vs v) return max$dx(v.x, max$dx(v.y, v.z)); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di maxelmt$vx($vs v) return max$dx(v.x, max$dx(v.y, max$dx(v.z, v.w))); }\n");
		}
	}

	//
	// vector sumelmts
	{
		print_vector_fn_docs(VECTOR_FN_SUM_ELMTS);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di sumelmts$vx($vs v) return v.x + v.y; }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di sumelmts$vx($vs v) return v.x + v.y + v.z; }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di sumelmts$vx($vs v) return v.x + v.y + v.z + v.w; }\n");
		}
	}

	//
	// vector productelmts
	{
		print_vector_fn_docs(VECTOR_FN_PRODUCT_ELMTS);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di productelmts$vx($vs v) return v.x * v.y; }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di productelmts$vx($vs v) return v.x * v.y * v.z; }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $di productelmts$vx($vs v) return v.x * v.y * v.z * v.w; }\n");
		}
	}

	//
	// vector square
	{
		print_vector_fn_docs(VECTOR_FN_SQUARE);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("HCC_INTRINSIC static inline $vs square$vx($vs v) { return $vx(mulh(v.x, v.x), mulh(v.y, v.y)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs square$vx($vs v) { return $vx(v.x * v.x, v.y * v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("HCC_INTRINSIC static inline $vs square$vx($vs v) { return $vx(mulh(v.x, v.x), mulh(v.y, v.y), mulh(v.z, v.z)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs square$vx($vs v) { return $vx(v.x * v.x, v.y * v.y, v.z * v.z); }\n");
		}
		ctx.vector = VECTOR_4;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("HCC_INTRINSIC static inline $vs square$vx($vs v) { return $vx(mulh(v.x, v.x), mulh(v.y, v.y), mulh(v.z, v.z), mulh(v.w, v.w)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline $vs square$vx($vs v) { return $vx(v.x * v.x, v.y * v.y, v.z * v.z, v.w * v.w); }\n");
		}
	}

	//
	// vector approxeq
	{
		print_vector_fn_docs(VECTOR_FN_APPROXEQ);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline vec2bool approxeq$vx($vs a, $vs b, $di epsilon) { return vec2bool(approxeq$dx(a.x, b.x, epsilon), approxeq$dx(a.y, b.y, epsilon)); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline vec3bool approxeq$vx($vs a, $vs b, $di epsilon) { return vec3bool(approxeq$dx(a.x, b.x, epsilon), approxeq$dx(a.y, b.y, epsilon), approxeq$dx(a.z, b.z, epsilon)); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline vec4bool approxeq$vx($vs a, $vs b, $di epsilon) { return vec4bool(approxeq$dx(a.x, b.x, epsilon), approxeq$dx(a.y, b.y, epsilon), approxeq$dx(a.z, b.z, epsilon), approxeq$dx(a.w, b.w, epsilon)); }\n");

		}
	}

	//
	// vector approxeq scalar
	{
		print_vector_fn_docs(VECTOR_FN_APPROXEQS);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline vec2bool approxeqs$vx($vs v, $di s, $di epsilon) { return vec2bool(approxeqs$dx(v.x, s, epsilon), approxeqs$dx(v.y, s, epsilon)); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline vec3bool approxeqs$vx($vs v, $di s, $di epsilon) { return vec3bool(approxeqs$dx(v.x, s, epsilon), approxeqs$dx(v.y, s, epsilon), approxeqs$dx(v.z, s, epsilon)); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC static inline vec4bool approxeqs$vx($vs v, $di s, $di epsilon) { return vec4bool(approxeqs$dx(v.x, s, epsilon), approxeqs$dx(v.y, s, epsilon), approxeqs$dx(v.z, s, epsilon), approxeqs$dx(v.w, s, epsilon)); }\n");

		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// packs a vec2f into a 32 bit integer comprised of two float 16 bits\n"
		"HCC_INTRINSIC U32 packf16x2v2f(vec2f v)\n"
		"\n"
		"//\n"
		"// unpacks a vec2f from a 32 bit integer comprised of two float 16 bits\n"
		"HCC_INTRINSIC vec2f unpackf16x2v2f(U32 v);\n"
		"\n"
		"//\n"
		"// packs a unsigned normalized vec2f into a 32 bit integer where each component is given 16 bits\n"
		"HCC_INTRINSIC U32 packu16x2v2f(vec2f v);\n"
		"\n"
		"//\n"
		"// unpacks a unsigned normalized vec2f from a 32 bit integer where each component is given 16 bits\n"
		"HCC_INTRINSIC vec2f unpacku16x2v2f(U32 v);\n"
		"\n"
		"//\n"
		"// packs a signed normalized vec2f into a 32 bit integer where each component is given 16 bits\n"
		"HCC_INTRINSIC U32 packs16x2v2f(vec2f v);\n"
		"\n"
		"//\n"
		"// unpacks a signed normalized vec2f from a 32 bit integer where each component is given 16 bits\n"
		"HCC_INTRINSIC vec2f unpacks16x2v2f(U32 v);\n"
		"\n"
		"//\n"
		"// packs a unsigned normalized vec4f into a 32 bit integer where each component is given 8 bits\n"
		"HCC_INTRINSIC U32 packu8x4v4f(vec4f v);\n"
		"\n"
		"//\n"
		"// unpacks a unsigned normalized vec4f from a 32 bit integer where each component is given 8 bits\n"
		"HCC_INTRINSIC vec4f unpacku8x4v4f(U32 v);\n"
		"\n"
		"//\n"
		"// packs a signed normalized vec4f into a 32 bit integer where each component is given 8 bits\n"
		"HCC_INTRINSIC U32 packs8x4v4f(vec4f v);\n"
		"\n"
		"//\n"
		"// unpacks a signed normalized vec4f from a 32 bit integer where each component is given 8 bits\n"
		"HCC_INTRINSIC vec4f unpacks8x4v4f(U32 v);\n"
		"\n"
	);

	print_section_header(
		"Matrix",
		""
	);

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a matrix that is a result of multipling matrix 'a' with matrix 'b'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		unsigned leftc = matrix_columns_count[matrix];
		unsigned leftr = matrix_rows_count[matrix];
		for (Matrix rightmatrix = 0; rightmatrix < MATRIX_COUNT; rightmatrix += 1) {
			unsigned rightc = matrix_columns_count[rightmatrix];
			unsigned rightr = matrix_rows_count[rightmatrix];
			if (leftc != rightr || leftr != rightc) {
				continue;
			}
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char leftname[128];
				snprintf(leftname, sizeof(leftname), "mat%ux%u%s", leftc, leftr, data_type_suffixes[data_type]);
				char rightname[128];
				snprintf(rightname, sizeof(rightname), "mat%ux%u%s", rightc, rightr, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s mulm%ux%um%ux%u%s(%s a, %s b);\n", leftname, leftc, leftr, rightc, rightr, data_type_suffixes[data_type], leftname, rightname);
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a matrix that is a result of multipling matrix 'm' with scalar 's'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC $mt mulsm$mcx$mr$dx($mt m, $di s);\n");
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a vector that is a result of multipling matrix 'm' with vector 'v'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		unsigned leftc = matrix_columns_count[matrix];
		unsigned leftr = matrix_rows_count[matrix];
		for (Vector vector = 0; vector < VECTOR_COUNT; vector += 1) {
			unsigned comp_count = vector_comps[vector];
			if (leftc != comp_count) {
				continue;
			}
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char matrixname[128];
				snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", leftc, leftr, data_type_suffixes[data_type]);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "vec%u%s", comp_count, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s mulm%ux%uv%u%s(%s m, %s v);\n", vectorname, leftc, leftr, comp_count, data_type_suffixes[data_type], matrixname, vectorname);
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a vector that is a result of multipling vector 'v' with matrix 'm'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		unsigned leftc = matrix_columns_count[matrix];
		unsigned leftr = matrix_rows_count[matrix];
		for (Vector vector = 0; vector < VECTOR_COUNT; vector += 1) {
			unsigned comp_count = vector_comps[vector];
			if (leftr != comp_count) {
				continue;
			}
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char matrixname[128];
				snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", leftc, leftr, data_type_suffixes[data_type]);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "vec%u%s", comp_count, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s invmulm%ux%uv%u%s(%s m, %s v);\n", vectorname, leftc, leftr, comp_count, data_type_suffixes[data_type], matrixname, vectorname);
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns the transposed matrix of matrix 'm'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			char matrixname[128];
			unsigned matrixc = matrix_columns_count[matrix];
			unsigned matrixr = matrix_rows_count[matrix];
			snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", matrixc, matrixr, data_type_suffixes[data_type]);
			char retmatrixname[128];
			snprintf(retmatrixname, sizeof(retmatrixname), "mat%ux%u%s", matrixr, matrixc, data_type_suffixes[data_type]);
			fprintf(ctx.f, "HCC_INTRINSIC %s transposem%ux%u%s(%s m) {\n", retmatrixname, matrixc, matrixr, data_type_suffixes[data_type], matrixname);
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a matrix from the outer product of vector 'a' and vector 'b'\n"
	);
	for (Vector leftvector = 0; leftvector < VECTOR_COUNT; leftvector += 1) {
		unsigned leftcomp_count = vector_comps[leftvector];
		for (Vector rightvector = 0; rightvector < VECTOR_COUNT; rightvector += 1) {
			unsigned rightcomp_count = vector_comps[rightvector];
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char matrixname[128];
				snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", leftcomp_count, rightcomp_count, data_type_suffixes[data_type]);
				char leftvectorname[128];
				snprintf(leftvectorname, sizeof(leftvectorname), "vec%u%s", leftcomp_count, data_type_suffixes[data_type]);
				char rightvectorname[128];
				snprintf(rightvectorname, sizeof(rightvectorname), "vec%u%s", rightcomp_count, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s outerproductv%uv%u%s(%s c, %s r);\n", matrixname, leftcomp_count, rightcomp_count, data_type_suffixes[data_type], leftvectorname, rightvectorname);
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns the determinant of matrix 'm'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		if (ctx.matrix % 4) {
			continue;
		}

		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC $mt determinantm$mcx$mr$dx($mt m);\n");
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns the inverse of matrix 'm'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		if (ctx.matrix % 4) {
			continue;
		}

		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC $mt inversem$mcx$mr$dx($mt m);\n");
		}
	}

	print_header_file_footer("_HCC_STD_MATH_H_");
}

void print_mat3x3_determinant(void) {
	print_entry(
		"\t$di s[3];\n"
		"\ts[0] = (m.cols[1][1] * m.cols[2][2]) - (m.cols[2][1] * m.cols[1][2]);\n"
		"\ts[1] = (m.cols[1][0] * m.cols[2][2]) - (m.cols[1][2] * m.cols[2][0]);\n"
		"\ts[2] = (m.cols[1][0] * m.cols[2][1]) - (m.cols[1][1] * m.cols[2][0]);\n"
		"\t$di det = s[0] - s[1] + s[2];\n"
	);
}

void print_mat4x4_determinant(void) {
	print_entry(
		"\t$di s[6];\n"
		"\t$di c[6];\n"
		"\n"
		"\ts[0] = m.cols[0][0]*m.cols[1][1] - m.cols[1][0]*m.cols[0][1];\n"
		"\ts[1] = m.cols[0][0]*m.cols[1][2] - m.cols[1][0]*m.cols[0][2];\n"
		"\ts[2] = m.cols[0][0]*m.cols[1][3] - m.cols[1][0]*m.cols[0][3];\n"
		"\ts[3] = m.cols[0][1]*m.cols[1][2] - m.cols[1][1]*m.cols[0][2];\n"
		"\ts[4] = m.cols[0][1]*m.cols[1][3] - m.cols[1][1]*m.cols[0][3];\n"
		"\ts[5] = m.cols[0][2]*m.cols[1][3] - m.cols[1][2]*m.cols[0][3];\n"
		"\n"
		"\tc[0] = m.cols[2][0]*m.cols[3][1] - m.cols[3][0]*m.cols[2][1];\n"
		"\tc[1] = m.cols[2][0]*m.cols[3][2] - m.cols[3][0]*m.cols[2][2];\n"
		"\tc[2] = m.cols[2][0]*m.cols[3][3] - m.cols[3][0]*m.cols[2][3];\n"
		"\tc[3] = m.cols[2][1]*m.cols[3][2] - m.cols[3][1]*m.cols[2][2];\n"
		"\tc[4] = m.cols[2][1]*m.cols[3][3] - m.cols[3][1]*m.cols[2][3];\n"
		"\tc[5] = m.cols[2][2]*m.cols[3][3] - m.cols[3][2]*m.cols[2][3];\n"
		"\n"
		"\t$di det = s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0];\n"
	);
}

void generate_math_file() {
	ctx.f = open_file_write("libhccstd/math.c");
	print_file_header();

	print_section_header("Half type aka. float 16 bit", "");
	fprintf(ctx.f,
		"HCC_INTRINSIC float htof(half v) {\n"
			"\tif ((v._bits & 0x7c00) == 0x7c00) { // inf, -inf or nan\n"
				"\t\tif (v._bits & 0x03ff) return NAN;\n"
				"\t\telse if (v._bits & 0x8000) return -INFINITY;\n"
				"\t\telse return INFINITY;\n"
			"\t}\n"
		"\n"
			"\tunion { float f; uint32_t u; } t1;\n"
			"\tuint32_t t2;\n"
			"\tuint32_t t3;\n"
		"\n"
			"\tt1.u = v._bits & 0x7fff;      // non-sign bits\n"
			"\tt2 = v._bits & 0x8000;        // sign bit\n"
			"\tt3 = v._bits & 0x7c00;        // exponent\n"
		"\n"
			"\tt1.u <<= 13;                 // align mantissa on MSB\n"
			"\tt2 <<= 16;                   // shift sign bit into position\n"
		"\n"
			"\tt1.u += 0x38000000;          // adjust bias\n"
		"\n"
			"\tt1.u = (t3 == 0 ? 0 : t1.u); // denormals-as-zero\n"
		"\n"
			"\tt1.u |= t2;                  // re-insert sign bit\n"
		"\n"
			"\treturn t1.f;\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC double htod(half v) {\n"
			"\treturn (double)htof(v);\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC half ftoh(float v) {\n"
			"\tif (isinf(v)) return (half){ ._bits = v < 0.0 ? 0xfc00 : 0x7c00 };\n"
			"\tif (isnan(v)) return (half){ ._bits = 0xffff };\n"
		"\n"
			"\tunion { float f; uint32_t u; } vu = { .f = v };\n"
			"\tuint32_t t1;\n"
			"\tuint32_t t2;\n"
			"\tuint32_t t3;\n"
		"\n"
			"\tt1 = vu.u & 0x7fffffff;                // non-sign bits\n"
			"\tt2 = vu.u & 0x80000000;                // sign bit\n"
			"\tt3 = vu.u & 0x7f800000;                // exponent\n"
		"\n"
			"\tt1 >>= 13;                             // align mantissa on MSB\n"
			"\tt2 >>= 16;                             // shift sign bit into position\n"
		"\n"
			"\tt1 -= 0x1c000;                         // adjust bias\n"
		"\n"
			"\tt1 = (t3 < 0x38800000) ? 0 : t1;       // flush-to-zero\n"
			"\tt1 = (t3 > 0x8e000000) ? 0x7bff : t1;  // clamp-to-max\n"
			"\tt1 = (t3 == 0 ? 0 : t1);               // denormals-as-zero\n"
		"\n"
			"\tt1 |= t2;                              // re-insert sign bit\n"
		"\n"
			"\treturn (half){ ._bits = t1 };\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC half dtoh(double v) {\n"
			"\treturn ftoh((float)v);\n"
		"}\n"
	);
	fprintf(ctx.f, "\n");

	print_section_header_vector();

	//
	// vector reflect
	{
		print_vector_fn_docs(VECTOR_FN_REFLECT);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			ctx.data_type = DATA_TYPE_HALF;
			print_entry(
				"HCC_INTRINSIC $vs reflect$vx($vs v, $vs normal) {\n"
					"\t$di dot_2 = mulh(dot$vx(normal, v), ftoh(2.f));\n"
					"\treturn sub$vx(v, mul$vx(normal, dot_2));\n"
				"}\n"
			);
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry(
					"HCC_INTRINSIC $vs reflect$vx($vs v, $vs normal) {\n"
						"\t$di dot_2 = dot$vx(normal, v) * 2.0;\n"
						"\treturn sub$vx(v, mul$vx(normal, dot_2));\n"
					"}\n"
				);
			}
		}
	}

	//
	// vector reflect
	{
		print_vector_fn_docs(VECTOR_FN_REFRACT);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			ctx.data_type = DATA_TYPE_HALF;
			print_entry(
				"HCC_INTRINSIC $vs refract$vx($vs v, $vs normal, float eta) {\n"
					"\tfloat dot = dot$vx(normal, v);\n"
					"\tfloat inv_dot_sq = subh(1.0, mulh(dot, dot));\n"
					"\tfloat eta_sq = mulh(eta, eta);\n"
					"\tfloat k = subh(1.f, mulh(eta_sq, inv_dot_sq);\n"
					"\tif (lth(k, 0.0)) {\n"
						"\t\treturn ZERO$vX;\n"
					"\t}\n"
					"\treturn sub$vx(muls$vx(v, eta), muls$vx(normal, ((addh(mulh(eta, dot$vx(normal, v)), sqrtf(k))))));\n"
				"}\n"
			);
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry(
					"HCC_INTRINSIC $vs refract$vx($vs v, $vs normal, float eta) {\n"
						"\tfloat dot = dot$vx(normal, v);\n"
						"\tfloat inv_dot_sq = 1.0 - dot * dot;\n"
						"\tfloat eta_sq = eta * eta;\n"
						"\tfloat k = 1.0 - eta_sq * inv_dot_sq;\n"
						"\tif (k < 0.0) {\n"
							"\t\treturn ZERO$vX;\n"
						"\t}\n"
						"\treturn sub$vx(muls$vx(v, eta), muls$vx(normal, ((eta * dot$vx(normal, v) + sqrtf(k)))));\n"
					"}\n"
				);
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"HCC_INTRINSIC U32 packf16x2v2f(vec2f v) {\n"
			"\treturn\n"
				"\t\t((U32)htobits(ftoh(v.x)) << 0)  ||\n"
				"\t((U32)htobits(ftoh(v.y)) << 16)  ;\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC vec2f unpackf16x2v2f(U32 v) {\n"
			"\treturn vec2f(\n"
				"\t\thtof(bitstoh(v & 0xffff)),\n"
				"\t\thtof(bitstoh(v >> 16))\n"
			"\t);\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC U32 packu16x2v2f(vec2f v) {\n"
			"\tv = roundv2f(mulsv2f(clampv2f(v, 0.f, 1.f), 65535.f));\n"
			"\treturn\n"
				"\t\t((U32)v.x << 0) ||\n"
				"\t\t((U32)v.y << 16) ;\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC vec2f unpacku16x2v2f(U32 v) {\n"
			"\treturn vec2f(\n"
				"\t\t(float)(v & 0xffff) / 65535.f,\n"
				"\t\t(float)(v >> 16) / 65535.f\n"
			"\t);\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC U32 packs16x2v2f(vec2f v) {\n"
			"\tv = roundv2f(mulsv2f(clampv2f(v, -1.f, 1.f), 32767.f));\n"
			"\treturn\n"
				"\t\t((U32)(U16)(S16)v.x << 0) ||\n"
				"\t\t((U32)(U16)(S16)v.y << 16) ;\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC vec2f unpacks16x2v2f(U32 v) {\n"
			"\treturn vec2f(\n"
				"\t\tclampv2f((float)(S32)(S16)(v & 0xffff) / 32767.f, -1.f, 1.f),\n"
				"\t\tclampv2f((float)(S32)(S16)(v >> 16) / 32767.f, -1.f, 1.f)\n"
			"\t);\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC U32 packu8x4v4f(vec4f v) {\n"
			"\tv = roundv4f(mulsv4f(clampv4f(v, 0.f, 1.f), 255.f));\n"
			"\treturn\n"
				"\t\t((U32)v.x << 0)  ||\n"
				"\t\t((U32)v.y << 8)  ||\n"
				"\t\t((U32)v.z << 16) ||\n"
				"\t\t((U32)v.w << 24)  ;\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC vec4f unpacku8x4v4f(U32 v) {\n"
			"\treturn vec4f(\n"
				"\t\t(float)((v >> 0)  & 0xff) / 255.f,\n"
				"\t\t(float)((v >> 8)  & 0xff) / 255.f,\n"
				"\t\t(float)((v >> 16) & 0xff) / 255.f,\n"
				"\t\t(float)((v >> 24) & 0xff) / 255.f\n"
			"\t);\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC U32 packs8x4v4f(vec4f v) {\n"
			"\tv = roundv4f(mulsv4f(clampv4f(v, -1.f, 1.f), 127.f));\n"
			"\treturn\n"
				"\t\t((U32)(U8)(S8)v.x << 0)  ||\n"
				"\t\t((U32)(U8)(S8)v.y << 8)  ||\n"
				"\t\t((U32)(U8)(S8)v.z << 16) ||\n"
				"\t\t((U32)(U8)(S8)v.w << 24)  ;\n"
		"}\n"
		"\n"
		"HCC_INTRINSIC vec4f unpacks8x4v4f(U32 v) {\n"
			"\treturn clampsv4f(\n"
				"\t\tvec4f(\n"
					"\t\t\t((float)(S32)(S8)((v >> 0)  & 0xff) / 127.f),\n"
					"\t\t\t((float)(S32)(S8)((v >> 8)  & 0xff) / 127.f),\n"
					"\t\t\t((float)(S32)(S8)((v >> 16) & 0xff) / 127.f),\n"
					"\t\t\t((float)(S32)(S8)((v >> 24) & 0xff) / 127.f)\n"
				"\t\t), -1.f, 1.f\n"
			"\t);\n"
		"}\n"
		"\n"
	);

	print_section_header(
		"Matrix",
		""
	);

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a matrix that is a result of multipling matrix 'a' with matrix 'b'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		unsigned leftc = matrix_columns_count[matrix];
		unsigned leftr = matrix_rows_count[matrix];
		for (Matrix rightmatrix = 0; rightmatrix < MATRIX_COUNT; rightmatrix += 1) {
			unsigned rightc = matrix_columns_count[rightmatrix];
			unsigned rightr = matrix_rows_count[rightmatrix];
			if (leftc != rightr || leftr != rightc) {
				continue;
			}
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char leftname[128];
				snprintf(leftname, sizeof(leftname), "mat%ux%u%s", leftc, leftr, data_type_suffixes[data_type]);
				char rightname[128];
				snprintf(rightname, sizeof(rightname), "mat%ux%u%s", rightc, rightr, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s mulm%ux%um%ux%u%s(%s a, %s b) {\n", leftname, leftc, leftr, rightc, rightr, data_type_suffixes[data_type], leftname, rightname);
				fprintf(ctx.f, "\t%s m;\n", leftname);
				for (int c = 0; c < leftc; c += 1) {
					for (int r = 0; r < leftr; r += 1) {
						fprintf(ctx.f, "\tm.cols[%u][%u] = ", c, r);
						for (int k = 0; k < leftc; k += 1) {
							fprintf(ctx.f, "a.cols[%u][%u] * b.cols[%u][%u]", k, r, c, k);
							fprintf(ctx.f, k + 1 == leftc ? ";\n" : " + ");
						}
					}
				}
				fprintf(ctx.f, "\treturn m;\n");
				fprintf(ctx.f, "}\n");
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a matrix that is a result of multipling matrix 'm' with scalar 's'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		unsigned ccount = matrix_columns_count[matrix];
		unsigned rcount = matrix_rows_count[matrix];
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_INTRINSIC $mt mulsm$mcx$mr$dx($mt m, $di s) {\n");
			for (int c = 0; c < ccount; c += 1) {
				for (int r = 0; r < rcount; r += 1) {
					fprintf(ctx.f, "\tm.cols[%u][%u] *= s;\n", c, r);
				}
			}
			fprintf(ctx.f, "\treturn m;\n");
			fprintf(ctx.f, "}\n");
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a vector that is a result of multipling matrix 'm' with vector 'v'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		unsigned leftc = matrix_columns_count[matrix];
		unsigned leftr = matrix_rows_count[matrix];
		for (Vector vector = 0; vector < VECTOR_COUNT; vector += 1) {
			unsigned comp_count = vector_comps[vector];
			if (leftc != comp_count) {
				continue;
			}
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char matrixname[128];
				snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", leftc, leftr, data_type_suffixes[data_type]);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "vec%u%s", comp_count, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s mulm%ux%uv%u%s(%s m, %s v) {\n", vectorname, leftc, leftr, comp_count, data_type_suffixes[data_type], matrixname, vectorname);
				fprintf(ctx.f, "\t%s ret;\n", vectorname);
				for (int r = 0; r < comp_count; r += 1) {
					fprintf(ctx.f, "\tret.array[%u] = ", r);
					for (int c = 0; c < leftc; c += 1) {
						fprintf(ctx.f, "m.cols[%u][%u] * v.array[%u]", c, r, r);
						fprintf(ctx.f, c + 1 == leftc ? ";\n" : " + ");
					}
				}
				fprintf(ctx.f, "\treturn ret;\n");
				fprintf(ctx.f, "}\n");
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a vector that is a result of multipling vector 'v' with matrix 'm'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		unsigned leftc = matrix_columns_count[matrix];
		unsigned leftr = matrix_rows_count[matrix];
		for (Vector vector = 0; vector < VECTOR_COUNT; vector += 1) {
			unsigned comp_count = vector_comps[vector];
			if (leftr != comp_count) {
				continue;
			}
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char matrixname[128];
				snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", leftc, leftr, data_type_suffixes[data_type]);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "vec%u%s", comp_count, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s invmulm%ux%uv%u%s(%s m, %s v) {\n", vectorname, leftc, leftr, comp_count, data_type_suffixes[data_type], matrixname, vectorname);
				fprintf(ctx.f, "\t%s ret;\n", vectorname);
				for (int r = 0; r < comp_count; r += 1) {
					fprintf(ctx.f, "\tret.array[%u] = ", r);
					for (int c = 0; c < leftc; c += 1) {
						fprintf(ctx.f, "v.array[%u] * m.cols[%u][%u]", c, c, r);
						fprintf(ctx.f, c + 1 == leftc ? ";\n" : " + ");
					}
				}
				fprintf(ctx.f, "\treturn ret;\n");
				fprintf(ctx.f, "}\n");
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns the transposed matrix of matrix 'm'\n"
	);
	for (Matrix matrix = 0; matrix < MATRIX_COUNT; matrix += 1) {
		ctx.matrix = matrix;
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			char matrixname[128];
			unsigned matrixc = matrix_columns_count[matrix];
			unsigned matrixr = matrix_rows_count[matrix];
			snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", matrixc, matrixr, data_type_suffixes[data_type]);
			char retmatrixname[128];
			snprintf(retmatrixname, sizeof(retmatrixname), "mat%ux%u%s", matrixr, matrixc, data_type_suffixes[data_type]);
			fprintf(ctx.f, "HCC_INTRINSIC %s transposem%ux%u%s(%s m) {\n", retmatrixname, matrixc, matrixr, data_type_suffixes[data_type], matrixname);
			fprintf(ctx.f, "\t%s ret;\n", retmatrixname);
			for (int c = 0; c < matrixc; c += 1) {
				for (int r = 0; r < matrixr; r += 1) {
					fprintf(ctx.f, "\tret.cols[%u][%u] = m.cols[%u][%u];\n", r, c, c, r);
				}
			}
			fprintf(ctx.f, "\treturn ret;\n");
			fprintf(ctx.f, "}\n");
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns a matrix from the outer product of vector 'a' and vector 'b'\n"
	);
	for (Vector leftvector = 0; leftvector < VECTOR_COUNT; leftvector += 1) {
		unsigned leftcomp_count = vector_comps[leftvector];
		for (Vector rightvector = 0; rightvector < VECTOR_COUNT; rightvector += 1) {
			unsigned rightcomp_count = vector_comps[rightvector];
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				char matrixname[128];
				snprintf(matrixname, sizeof(matrixname), "mat%ux%u%s", leftcomp_count, rightcomp_count, data_type_suffixes[data_type]);
				char leftvectorname[128];
				snprintf(leftvectorname, sizeof(leftvectorname), "vec%u%s", leftcomp_count, data_type_suffixes[data_type]);
				char rightvectorname[128];
				snprintf(rightvectorname, sizeof(rightvectorname), "vec%u%s", rightcomp_count, data_type_suffixes[data_type]);
				fprintf(ctx.f, "HCC_INTRINSIC %s outerproductv%uv%u%s(%s c, %s r) {\n", matrixname, leftcomp_count, rightcomp_count, data_type_suffixes[data_type], leftvectorname, rightvectorname);
				fprintf(ctx.f, "\t%s ret;\n", matrixname);
				for (int c = 0; c < leftcomp_count; c += 1) {
					for (int r = 0; r < rightcomp_count; r += 1) {
						fprintf(ctx.f, "\tret.cols[%u][%u] = c[%u] * r[%u];\n", c, r, c, r);
					}
				}
				fprintf(ctx.f, "\treturn ret;\n");
				fprintf(ctx.f, "}\n");
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns the determinant of matrix 'm'\n"
	);
	ctx.matrix = MATRIX_2x2;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"HCC_INTRINSIC $di determinantm$mcx$mr$dx($mt m) {\n"
			"\treturn m.cols[0][0] * m.cols[1][1] - m.cols[1][0] * m.cols[0][1];\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_3x3;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("HCC_INTRINSIC $di determinantm$mcx$mr$dx($mt m) {\n");
		print_mat3x3_determinant();
		print_entry(
			"\treturn det;\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_4x4;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("HCC_INTRINSIC $di determinantm$mcx$mr$dx($mt m) {\n");
		print_mat4x4_determinant();
		print_entry(
			"\treturn det;\n"
			"}\n"
		);
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// returns the inverse of matrix 'm'\n"
	);
	ctx.matrix = MATRIX_2x2;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"HCC_INTRINSIC $mt inversem$mcx$mr$dx($mt m) {\n"
			"\t$di inv_det = 1.0 / determinantm$mcx$mr$dx(m);\n"
			"\t$mt ret;\n"
			"\tret.cols[0][0] = m.cols[1][1] * inv_det;\n"
			"\tret.cols[0][1] = -m.cols[0][1] * inv_det;\n"
			"\t\n"
			"\tret.cols[1][0] = -m.cols[1][0] * inv_det;\n"
			"\tret.cols[1][1] = m.cols[0][0] * inv_det;\n"
			"\treturn ret;\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_3x3;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("HCC_INTRINSIC $mt inversem$mcx$mr$dx($mt m) {\n");
		print_mat3x3_determinant();
		print_entry(
			"\t$di inv_det = 1.0 / det;\n"
			"\t$mt ret;\n"
			"\tret.cols[0][0] = s[0] * inv_det;\n"
			"\tret.cols[0][1] = (m.cols[0][2] * m.cols[2][1] - m.cols[0][1] * m.cols[2][2]) * inv_det;\n"
			"\tret.cols[0][2] = (m.cols[0][1] * m.cols[1][2] - m.cols[0][2] * m.cols[1][1]) * inv_det;\n"
			"\t\n"
			"\tret.cols[1][0] = -s[1] * inv_det;\n"
			"\tret.cols[1][1] = (m.cols[0][0] * m.cols[2][2] - m.cols[0][2] * m.cols[2][0]) * inv_det;\n"
			"\tret.cols[1][2] = (m.cols[1][0] * m.cols[0][2] - m.cols[0][0] * m.cols[1][2]) * inv_det;\n"
			"\t\n"
			"\tret.cols[2][0] = s[2] * inv_det;\n"
			"\tret.cols[2][1] = (m.cols[2][0] * m.cols[0][1] - m.cols[0][0] * m.cols[2][1]) * inv_det;\n"
			"\tret.cols[2][2] = (m.cols[0][0] * m.cols[1][1] - m.cols[1][0] * m.cols[0][1]) * inv_det;\n"
			"\treturn ret;\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_4x4;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("HCC_INTRINSIC $mt inversem$mcx$mr$dx($mt m) {\n");
		print_mat4x4_determinant();
		print_entry(
			"\t$di inv_det = 1.0 / det;\n"
			"\t$mt ret;\n"
			"\tret.cols[0][0] = ( m.cols[1][1] * c[5] - m.cols[1][2] * c[4] + m.cols[1][3] * c[3]) * inv_det;\n"
			"\tret.cols[0][1] = (-m.cols[0][1] * c[5] + m.cols[0][2] * c[4] - m.cols[0][3] * c[3]) * inv_det;\n"
			"\tret.cols[0][2] = ( m.cols[3][1] * s[5] - m.cols[3][2] * s[4] + m.cols[3][3] * s[3]) * inv_det;\n"
			"\tret.cols[0][3] = (-m.cols[2][1] * s[5] + m.cols[2][2] * s[4] - m.cols[2][3] * s[3]) * inv_det;\n"
			"\t\n"
			"\tret.cols[1][0] = (-m.cols[1][0] * c[5] + m.cols[1][2] * c[2] - m.cols[1][3] * c[1]) * inv_det;\n"
			"\tret.cols[1][1] = ( m.cols[0][0] * c[5] - m.cols[0][2] * c[2] + m.cols[0][3] * c[1]) * inv_det;\n"
			"\tret.cols[1][2] = (-m.cols[3][0] * s[5] + m.cols[3][2] * s[2] - m.cols[3][3] * s[1]) * inv_det;\n"
			"\tret.cols[1][3] = ( m.cols[2][0] * s[5] - m.cols[2][2] * s[2] + m.cols[2][3] * s[1]) * inv_det;\n"
			"\t\n"
			"\tret.cols[2][0] = ( m.cols[1][0] * c[4] - m.cols[1][1] * c[2] + m.cols[1][3] * c[0]) * inv_det;\n"
			"\tret.cols[2][1] = (-m.cols[0][0] * c[4] + m.cols[0][1] * c[2] - m.cols[0][3] * c[0]) * inv_det;\n"
			"\tret.cols[2][2] = ( m.cols[3][0] * s[4] - m.cols[3][1] * s[2] + m.cols[3][3] * s[0]) * inv_det;\n"
			"\tret.cols[2][3] = (-m.cols[2][0] * s[4] + m.cols[2][1] * s[2] - m.cols[2][3] * s[0]) * inv_det;\n"
			"\t\n"
			"\tret.cols[3][0] = (-m.cols[1][0] * c[3] + m.cols[1][1] * c[1] - m.cols[1][2] * c[0]) * inv_det;\n"
			"\tret.cols[3][1] = ( m.cols[0][0] * c[3] - m.cols[0][1] * c[1] + m.cols[0][2] * c[0]) * inv_det;\n"
			"\tret.cols[3][2] = (-m.cols[3][0] * s[3] + m.cols[3][1] * s[1] - m.cols[3][2] * s[0]) * inv_det;\n"
			"\tret.cols[3][3] = ( m.cols[2][0] * s[3] - m.cols[2][1] * s[1] + m.cols[2][2] * s[0]) * inv_det;\n"
			"\treturn ret;\n"
			"}\n"
		);
	}
}

int main(int argc, char** argv) {
	generate_math_types_header_file();
	generate_math_header_file();
	generate_math_file();
	printf("Success! you will find libhccstd/{math_types.h, math.h, math.c} files have been updated\n");
	return 0;
}
