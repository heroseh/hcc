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
static const char* approxeq_idents[] = { "a", "b", "epsilon" };
static const char* approxeqs_idents[] = { "v", "s", "epsilon" };
static const char* smoothstep_idents[] = { "edge0", "edge1", "v" };

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
	VECTOR_FN_SSUB,
	VECTOR_FN_MUL,
	VECTOR_FN_MULS,
	VECTOR_FN_DIV,
	VECTOR_FN_DIVS,
	VECTOR_FN_SDIV,
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
	VECTOR_FN_COPYSIGNS,
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
	VECTOR_FN_FMAS,
	VECTOR_FN_FMASS,
	VECTOR_FN_FLOOR,
	VECTOR_FN_CEIL,
	VECTOR_FN_ROUND,
	VECTOR_FN_TRUNC,
	VECTOR_FN_FRACT,
	VECTOR_FN_RADIANS,
	VECTOR_FN_DEGREES,
	VECTOR_FN_STEP,
	VECTOR_FN_STEPS,
	VECTOR_FN_SMOOTHSTEP,
	VECTOR_FN_SMOOTHSTEPS,
	VECTOR_FN_SMOOTHSTEPSS,
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
	DATA_TYPE_S8,
	DATA_TYPE_S16,
	DATA_TYPE_S32,
	DATA_TYPE_S64,
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
	[VECTOR_FN_SSUB] = "-",
	[VECTOR_FN_MUL] = "*",
	[VECTOR_FN_MULS] = "*",
	[VECTOR_FN_DIV] = "/",
	[VECTOR_FN_DIVS] = "/",
	[VECTOR_FN_SDIV] = "/",
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
};

bool vector_is_not_intrinsic[VECTOR_FN_COUNT] = {
	[VECTOR_FN_ADDS] = true,
	[VECTOR_FN_SUBS] = true,
	[VECTOR_FN_SSUB] = true,
	[VECTOR_FN_MULS] = true,
	[VECTOR_FN_DIVS] = true,
	[VECTOR_FN_SDIV] = true,
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
	[VECTOR_FN_APPROXEQS] = true,
	[VECTOR_FN_FMAS] = true,
	[VECTOR_FN_FMASS] = true,
	[VECTOR_FN_STEPS] = true,
	[VECTOR_FN_SMOOTHSTEPS] = true,
	[VECTOR_FN_SMOOTHSTEPSS] = true,

	[VECTOR_FN_REMAP] = true,
	[VECTOR_FN_ROUNDTOMULTIPLE] = true,
	[VECTOR_FN_ROUNDUPTOMULTIPLE] = true,
	[VECTOR_FN_ROUNDDOWNTOMULTIPLE] = true,
	[VECTOR_FN_APPROXEQ] = true,
	[VECTOR_FN_COPYSIGN] = true,
	[VECTOR_FN_COPYSIGNS] = true,
	[VECTOR_FN_INVLERP] = true,
	[VECTOR_FN_LENSQ] = true,
	[VECTOR_FN_MIN_ELMT] = true,
	[VECTOR_FN_MAX_ELMT] = true,
	[VECTOR_FN_SUM_ELMTS] = true,
	[VECTOR_FN_PRODUCT_ELMTS] = true,
	[VECTOR_FN_SQUARE] = true,
};

VectorFn vector_scalar_non_scalar_fn[VECTOR_FN_COUNT] = {
	[VECTOR_FN_ADDS] = VECTOR_FN_ADD,
	[VECTOR_FN_SUBS] = VECTOR_FN_SUB,
	[VECTOR_FN_SSUB] = VECTOR_FN_SUB,
	[VECTOR_FN_MULS] = VECTOR_FN_MUL,
	[VECTOR_FN_DIVS] = VECTOR_FN_DIV,
	[VECTOR_FN_SDIV] = VECTOR_FN_DIV,
	[VECTOR_FN_MODS] = VECTOR_FN_MOD,
	[VECTOR_FN_EQS] = VECTOR_FN_EQ,
	[VECTOR_FN_NEQS] = VECTOR_FN_NEQ,
	[VECTOR_FN_LTS] = VECTOR_FN_LT,
	[VECTOR_FN_LTEQS] = VECTOR_FN_LTEQ,
	[VECTOR_FN_GTS] = VECTOR_FN_GT,
	[VECTOR_FN_GTEQS] = VECTOR_FN_GTEQ,
	[VECTOR_FN_MINS] = VECTOR_FN_MIN,
	[VECTOR_FN_MAXS] = VECTOR_FN_MAX,
	[VECTOR_FN_CLAMPS] = VECTOR_FN_CLAMP,
	[VECTOR_FN_BITANDS] = VECTOR_FN_BITAND,
	[VECTOR_FN_BITORS] = VECTOR_FN_BITOR,
	[VECTOR_FN_BITXORS] = VECTOR_FN_BITXOR,
	[VECTOR_FN_BITSHLS] = VECTOR_FN_BITSHL,
	[VECTOR_FN_BITSHRS] = VECTOR_FN_BITSHR,
	[VECTOR_FN_REMAPS] = VECTOR_FN_REMAP,
	[VECTOR_FN_ROUNDTOMULTIPLES] = VECTOR_FN_ROUNDTOMULTIPLE,
	[VECTOR_FN_ROUNDUPTOMULTIPLES] = VECTOR_FN_ROUNDUPTOMULTIPLE,
	[VECTOR_FN_ROUNDDOWNTOMULTIPLES] = VECTOR_FN_ROUNDDOWNTOMULTIPLE,
	[VECTOR_FN_APPROXEQS] = VECTOR_FN_APPROXEQ,
	[VECTOR_FN_COPYSIGNS] = VECTOR_FN_COPYSIGN,
	[VECTOR_FN_FMAS] = VECTOR_FN_FMA,
	[VECTOR_FN_FMASS] = VECTOR_FN_FMA,
	[VECTOR_FN_STEPS] = VECTOR_FN_STEP,
	[VECTOR_FN_SMOOTHSTEPS] = VECTOR_FN_SMOOTHSTEP,
	[VECTOR_FN_SMOOTHSTEPSS] = VECTOR_FN_SMOOTHSTEP,
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
	[VECTOR_FN_APPROXEQ] = true,
	[VECTOR_FN_APPROXEQS] = true,
	[VECTOR_FN_ISINF] = true,
	[VECTOR_FN_ISNAN] = true,
};

bool vector_fn_no_operator_or_calls[VECTOR_FN_COUNT] = {
	[VECTOR_FN_PACK] = true,
	[VECTOR_FN_UNPACK] = true,
	[VECTOR_FN_ANY] = true,
	[VECTOR_FN_ALL] = true,
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
	[VECTOR_FN_SSUB] = 2,
	[VECTOR_FN_MUL] = 2,
	[VECTOR_FN_MULS] = 2,
	[VECTOR_FN_DIV] = 2,
	[VECTOR_FN_DIVS] = 2,
	[VECTOR_FN_SDIV] = 2,
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
	[VECTOR_FN_COPYSIGNS] = 2,
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
	[VECTOR_FN_FMAS] = 3,
	[VECTOR_FN_FMASS] = 3,
	[VECTOR_FN_FLOOR] = 1,
	[VECTOR_FN_CEIL] = 1,
	[VECTOR_FN_ROUND] = 1,
	[VECTOR_FN_TRUNC] = 1,
	[VECTOR_FN_FRACT] = 1,
	[VECTOR_FN_RADIANS] = 1,
	[VECTOR_FN_DEGREES] = 1,
	[VECTOR_FN_STEP] = 2,
	[VECTOR_FN_STEPS] = 2,
	[VECTOR_FN_SMOOTHSTEP] = 3,
	[VECTOR_FN_SMOOTHSTEPS] = 3,
	[VECTOR_FN_SMOOTHSTEPSS] = 3,
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
	[VECTOR_FN_POW] = 2,
	[VECTOR_FN_EXP] = 1,
	[VECTOR_FN_LOG] = 1,
	[VECTOR_FN_EXP2] = 1,
	[VECTOR_FN_LOG2] = 1,
	[VECTOR_FN_SQRT] = 1,
	[VECTOR_FN_RSQRT] = 1,
	[VECTOR_FN_APPROXEQ] = 3,
	[VECTOR_FN_APPROXEQS] = 3,
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
	[VECTOR_FN_SSUB] = "ssub",
	[VECTOR_FN_MUL] = "mul",
	[VECTOR_FN_MULS] = "muls",
	[VECTOR_FN_DIV] = "div",
	[VECTOR_FN_DIVS] = "divs",
	[VECTOR_FN_SDIV] = "sdiv",
	[VECTOR_FN_MOD] = "mod",
	[VECTOR_FN_MODS] = "mods",
	[VECTOR_FN_EQ] = "eq",
	[VECTOR_FN_EQS] = "eqs",
	[VECTOR_FN_NEQ] = "neq",
	[VECTOR_FN_NEQS] = "neqs",
	[VECTOR_FN_LT] = "lt",
	[VECTOR_FN_LTS] = "lts",
	[VECTOR_FN_LTEQ] = "lteq",
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
	[VECTOR_FN_COPYSIGNS] = "copysigns",
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
	[VECTOR_FN_FMAS] = "fmas",
	[VECTOR_FN_FMASS] = "fmass",
	[VECTOR_FN_FLOOR] = "floor",
	[VECTOR_FN_CEIL] = "ceil",
	[VECTOR_FN_ROUND] = "round",
	[VECTOR_FN_TRUNC] = "trunc",
	[VECTOR_FN_FRACT] = "fract",
	[VECTOR_FN_RADIANS] = "radians",
	[VECTOR_FN_DEGREES] = "degrees",
	[VECTOR_FN_STEP] = "step",
	[VECTOR_FN_STEPS] = "steps",
	[VECTOR_FN_SMOOTHSTEP] = "smoothstep",
	[VECTOR_FN_SMOOTHSTEPS] = "smoothsteps",
	[VECTOR_FN_SMOOTHSTEPSS] = "smoothstepss",
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
	[VECTOR_FN_LENSQ] = "returns the squared euclidean length of the vector 'v', this avoids doing the square root. useful when you want to compare of one length is less than another vector length without paying the cost of a sqrt instruction",
	[VECTOR_FN_NORM] = "returns a version of 'v' where the magnatude is a unit length of 1.f",
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
	[VECTOR_FN_SSUB] = "returns a vector where each component is the result from subtracting the value of 's' to that component in 'v'",
	[VECTOR_FN_MUL] = "returns a vector where each component is the result from multiplying that component in 'a' to that component in 'b'",
	[VECTOR_FN_MULS] = "returns a vector where each component is the result from multiplying that component in 'v' to the value of 's'",
	[VECTOR_FN_DIV] = "returns a vector where each component is the result from dividing that component in 'a' to that component in 'b'",
	[VECTOR_FN_DIVS] = "returns a vector where each component is the result from dividing that component in 'v' to the value of 's'",
	[VECTOR_FN_SDIV] = "returns a vector where each component is the result from dividing the value of 's' to that component in 'v'",
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
	[VECTOR_FN_SIGN] = "returns a vector where each component is -1, 0, or 1 depending on the sign of that component that is in 'v'",
	[VECTOR_FN_COPYSIGN] = "returns a vector where each component is that component in 'v' with sign of that component in 'sign'",
	[VECTOR_FN_COPYSIGNS] = "returns a vector where each component is that component in 'v' with sign of 'sign'",
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
	[VECTOR_FN_FMAS] = "returns a vector where each component (x) is calculated like so x = (a.x * b.x) + c",
	[VECTOR_FN_FMASS] = "returns a vector where each component (x) is calculated like so x = (a.x * b) + c",
	[VECTOR_FN_FLOOR] = "return a vector where each component is the result of appling 'floor' to that component in 'v'",
	[VECTOR_FN_CEIL] = "return a vector where each component is the result of appling 'ceil' to that component in 'v'",
	[VECTOR_FN_ROUND] = "return a vector where each component is the result of appling 'round' to that component in 'v'",
	[VECTOR_FN_TRUNC] = "return a vector where each component is the result of appling 'trunc' to that component in 'v'",
	[VECTOR_FN_FRACT] = "return a vector where each component is the result of appling 'fract' to that component in 'v'",
	[VECTOR_FN_RADIANS] = "return a vector where each component is the result of appling 'radians' to that component in 'v'",
	[VECTOR_FN_DEGREES] = "return a vector where each component is the result of appling 'degrees' to that component in 'v'",
	[VECTOR_FN_STEP] = "return a vector where each component is the result of appling 'step' to that component in 'v'",
	[VECTOR_FN_STEPS] = "return a vector where each component is the result of appling 'step' to that component in 'v'",
	[VECTOR_FN_SMOOTHSTEP] = "return a vector where each component is the result of appling 'smoothstep' to that component in 'v'",
	[VECTOR_FN_SMOOTHSTEPS] = "return a vector where each component is the result of appling 'smoothstep' to that component in 'v'",
	[VECTOR_FN_SMOOTHSTEPSS] = "return a vector where each component is the result of appling 'smoothstep' to that component in 'v'",
	[VECTOR_FN_REMAP] = "return a vector where each component is the result of appling 'remap' to that component in 'v', 'from_min', 'from_max', 'to_min' and 'to_max'",
	[VECTOR_FN_REMAPS] = "return a vector where each component is the result of appling 'remap' to that component in 'v' with scalar 'from_min', 'from_max', 'to_min' and 'to_max'",
	[VECTOR_FN_ROUNDTOMULTIPLE] = "return a vector where each component is the result of appling 'roundtomultiple' to that component in 'v' and 'multiple'",
	[VECTOR_FN_ROUNDTOMULTIPLES] = "return a vector where each component is the result of appling 'roundtomultiple' to that component in 'v' with scalar 'multiple'",
	[VECTOR_FN_ROUNDUPTOMULTIPLE] = "return a vector where each component is the result of appling 'rounduptomultiple' to that component in 'v' and 'multiple'",
	[VECTOR_FN_ROUNDUPTOMULTIPLES] = "return a vector where each component is the result of appling 'rounduptomultiple' to that component in 'v' with scalar 'multiple'",
	[VECTOR_FN_ROUNDDOWNTOMULTIPLE] = "return a vector where each component is the result of appling 'rounddowntomultiple' to that component in 'v' and 'multiple'",
	[VECTOR_FN_ROUNDDOWNTOMULTIPLES] = "return a vector where each component is the result of appling 'rounddowntomultiple' to that component in 'v' with scalar 'multiple'",
	[VECTOR_FN_BITSTO] = "return a vector where each component is the result of appling 'bitsto' to that component in 'v'",
	[VECTOR_FN_BITSFROM] = "return a vector where each component is the result of appling 'bitsfrom' to that component in 'v'",
	[VECTOR_FN_SIN] = "return a vector where each component is the result of appling 'sin' to that component in 'v'",
	[VECTOR_FN_COS] = "return a vector where each component is the result of appling 'cos' to that component in 'v'",
	[VECTOR_FN_TAN] = "return a vector where each component is the result of appling 'tan' to that component in 'v'",
	[VECTOR_FN_ASIN] = "return a vector where each component is the result of appling 'asin' to that component in 'v'",
	[VECTOR_FN_ACOS] = "return a vector where each component is the result of appling 'acos' to that component in 'v'",
	[VECTOR_FN_ATAN] = "return a vector where each component is the result of appling 'atan' to that component in 'v'",
	[VECTOR_FN_SINH] = "return a vector where each component is the result of appling 'sinh' to that component in 'v'",
	[VECTOR_FN_COSH] = "return a vector where each component is the result of appling 'cosh' to that component in 'v'",
	[VECTOR_FN_TANH] = "return a vector where each component is the result of appling 'tanh' to that component in 'v'",
	[VECTOR_FN_ASINH] = "return a vector where each component is the result of appling 'asinh' to that component in 'v'",
	[VECTOR_FN_ACOSH] = "return a vector where each component is the result of appling 'acosh' to that component in 'v'",
	[VECTOR_FN_ATANH] = "return a vector where each component is the result of appling 'atanh' to that component in 'v'",
	[VECTOR_FN_ATAN2] = "return a vector where each component is the result of appling 'atan2' to that component in 'v'",
	[VECTOR_FN_POW] = "return a vector where each component is the result of appling 'pow' to that component in 'v'",
	[VECTOR_FN_EXP] = "return a vector where each component is the result of appling 'exp' to that component in 'v'",
	[VECTOR_FN_LOG] = "return a vector where each component is the result of appling 'log' to that component in 'v'",
	[VECTOR_FN_EXP2] = "return a vector where each component is the result of appling 'exp2' to that component in 'v'",
	[VECTOR_FN_LOG2] = "return a vector where each component is the result of appling 'log2' to that component in 'v'",
	[VECTOR_FN_SQRT] = "return a vector where each component is the result of appling 'sqrt' to that component in 'v'",
	[VECTOR_FN_RSQRT] = "return a vector where each component is the result of appling 'rsqrt' to that component in 'v'",
	[VECTOR_FN_APPROXEQ] = "returns true if each component in 'a' is 'epsilon' away from that component that is in 'b'",
	[VECTOR_FN_APPROXEQS] = "returns true if each component in 'v' is 'epsilon' away from 's'",
	[VECTOR_FN_ISINF] = "return a vector where each component is the result of appling 'isinf' to that component in 'v'",
	[VECTOR_FN_ISNAN] = "return a vector where each component is the result of appling 'isnan' to that component in 'v'",
	[VECTOR_FN_LERP] = "return a vector where each component is the result of appling 'lerp' to that component in 'start', 'end' and 't'",
	[VECTOR_FN_INVLERP] = "return a vector where each component is the result of appling 'invlerp' to that component in 'start', 'end' and 't'",
};

static const char* vector_suffixes[] = {
	[VECTOR_2] = "x2",
	[VECTOR_3] = "x3",
	[VECTOR_4] = "x4",
};

static const char* vector_suffixes_cap[] = {
	[VECTOR_2] = "X2",
	[VECTOR_3] = "X3",
	[VECTOR_4] = "X4",
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

static const char* matrix_suffixes[] = {
	[MATRIX_2x2] = "x2x2",
	[MATRIX_2x3] = "x2x3",
	[MATRIX_2x4] = "x2x4",
	[MATRIX_3x2] = "x3x2",
	[MATRIX_3x3] = "x3x3",
	[MATRIX_3x4] = "x3x4",
	[MATRIX_4x2] = "x4x2",
	[MATRIX_4x3] = "x4x3",
	[MATRIX_4x4] = "x4x4",
};

static const char* matrix_suffixes_cap[] = {
	[MATRIX_2x2] = "X2X2",
	[MATRIX_2x3] = "X2X3",
	[MATRIX_2x4] = "X2X4",
	[MATRIX_3x2] = "X3X2",
	[MATRIX_3x3] = "X3X3",
	[MATRIX_3x4] = "X3X4",
	[MATRIX_4x2] = "X4X2",
	[MATRIX_4x3] = "X4X3",
	[MATRIX_4x4] = "X4X4",
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
	[DATA_TYPE_S8] = DATA_TYPE_CLASS_INT,
	[DATA_TYPE_S16] = DATA_TYPE_CLASS_INT,
	[DATA_TYPE_S32] = DATA_TYPE_CLASS_INT,
	[DATA_TYPE_S64] = DATA_TYPE_CLASS_INT,
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
	[DATA_TYPE_S8] = "int8_t",
	[DATA_TYPE_S16] = "int16_t",
	[DATA_TYPE_S32] = "int32_t",
	[DATA_TYPE_S64] = "int64_t",
	[DATA_TYPE_U8] = "uint8_t",
	[DATA_TYPE_U16] = "uint16_t",
	[DATA_TYPE_U32] = "uint32_t",
	[DATA_TYPE_U64] = "uint64_t",
};

static const char* data_type_suffixes[] = {
	[DATA_TYPE_BOOL] = "bool",
	[DATA_TYPE_HALF] = "f16",
	[DATA_TYPE_FLOAT] = "f32",
	[DATA_TYPE_DOUBLE] = "f64",
	[DATA_TYPE_S8] = "s8",
	[DATA_TYPE_S16] = "s16",
	[DATA_TYPE_S32] = "s32",
	[DATA_TYPE_S64] = "s64",
	[DATA_TYPE_U8] = "u8",
	[DATA_TYPE_U16] = "u16",
	[DATA_TYPE_U32] = "u32",
	[DATA_TYPE_U64] = "u64",
};

static const char* data_type_suffixes_cap[] = {
	[DATA_TYPE_BOOL] = "BOOL",
	[DATA_TYPE_HALF] = "F16",
	[DATA_TYPE_FLOAT] = "F32",
	[DATA_TYPE_DOUBLE] = "F64",
	[DATA_TYPE_S8] = "S8",
	[DATA_TYPE_S16] = "S16",
	[DATA_TYPE_S32] = "S32",
	[DATA_TYPE_S64] = "S64",
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
	[DATA_TYPE_S8] = 1,
	[DATA_TYPE_S16] = 2,
	[DATA_TYPE_S32] = 4,
	[DATA_TYPE_S64] = 8,
	[DATA_TYPE_U8] = 1,
	[DATA_TYPE_U16] = 2,
	[DATA_TYPE_U32] = 4,
	[DATA_TYPE_U64] = 8,
};

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

bool vector_fn_has_scalar_params(VectorFn fn) { return vector_scalar_non_scalar_fn[fn] != 0; }

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
	case VECTOR_FN_APPROXEQS:
	case VECTOR_FN_COPYSIGNS:
	case VECTOR_FN_FMASS:
	case VECTOR_FN_STEPS:
	case VECTOR_FN_SMOOTHSTEPSS:
		return param_idx;
	case VECTOR_FN_SSUB:
	case VECTOR_FN_SDIV:
		return param_idx == 0;
	case VECTOR_FN_FMAS:
	case VECTOR_FN_SMOOTHSTEPS:
	case VECTOR_FN_APPROXEQ:
		return param_idx > 1;
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
			switch (fn) {
			case VECTOR_FN_COPYSIGN:
			case VECTOR_FN_COPYSIGNS:
				return param_idx ? "sign" : "v";
			case VECTOR_FN_ATAN2: return param_idx ? "x" : "y";
			case VECTOR_FN_STEP:
			case VECTOR_FN_STEPS:
				return param_idx ? "edge" : "v";
			case VECTOR_FN_SSUB:
			case VECTOR_FN_SDIV:
				return param_idx ? "v" : "s";
			default: break;
			}
			if (vector_fn_has_scalar_params(fn)) {
				return param_idx ? "s" : "v";
			}
			return abc_idents[param_idx];
		case 3:
			switch (fn) {
			case VECTOR_FN_CLAMP:
			case VECTOR_FN_CLAMPS: return clamp_idents[param_idx];
			case VECTOR_FN_FMA:
			case VECTOR_FN_FMAS:
			case VECTOR_FN_FMASS:
				return abc_idents[param_idx];
			case VECTOR_FN_LERP: return lerp_idents[param_idx];
			case VECTOR_FN_INVLERP: return invlerp_idents[param_idx];
			case VECTOR_FN_APPROXEQ: return approxeq_idents[param_idx];
			case VECTOR_FN_APPROXEQS: return approxeqs_idents[param_idx];
			case VECTOR_FN_SMOOTHSTEP:
			case VECTOR_FN_SMOOTHSTEPS:
			case VECTOR_FN_SMOOTHSTEPSS:
				return smoothstep_idents[param_idx];
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

void print_section_header_half() {
	print_section_header("Half type aka. float 16 bit", "");
}

void print_section_header_scalar() {
	print_section_header("Scalar", "");
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
					case DATA_TYPE_S8: fprintf(ctx.f,"0x80"); break;
					case DATA_TYPE_S16: fprintf(ctx.f,"0x8000"); break;
					case DATA_TYPE_S32: fprintf(ctx.f,"0x800000"); break;
					case DATA_TYPE_S64: fprintf(ctx.f,"0x80000000"); break;
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
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type + 4]);
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					break;
				case 'c':
					fprintf(ctx.f,"%u", vector_comps[ctx.vector]);
					break;
				case '2':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes[VECTOR_2]);
					break;
				case '3':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes[VECTOR_3]);
					break;
				case 's':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					break;
				case 'S':
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes_cap[ctx.vector]);
					break;
				case 'x':
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					break;
				case 'z':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%-4s", vector_suffixes[ctx.vector]);
					break;
				case 'Z':
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					fprintf(ctx.f,"%-4s", vector_suffixes_cap[ctx.vector]);
					break;
			}
			break;
		case 'm':
			next_special += 1; // skip 'm'
			switch (*next_special) {
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
					fprintf(ctx.f,"%s%s", data_type_suffixes[ctx.data_type], vector_suffixes[ctx.matrix / 3]);
					break;
				case 'V':
					fprintf(ctx.f,"%s%s", data_type_suffixes[ctx.data_type], vector_suffixes[ctx.matrix % 3]);
					break;
				case 'x':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", matrix_suffixes[ctx.matrix]);
					break;
				case 'X':
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					fprintf(ctx.f,"%s", matrix_suffixes_cap[ctx.matrix]);
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
		"\n"
	);
	fprintf(ctx.f,"\n");

	print_entry(
		"#ifndef HCC_ENABLE_VECTOR_EXTENSIONS\n"
		"#if defined(__HCC__) || defined(__clang__)\n"
		"#define HCC_ENABLE_VECTOR_EXTENSIONS 1\n"
		"#else\n"
		"#define HCC_ENABLE_VECTOR_EXTENSIONS 0\n"
		"#endif\n"
		"#endif\n"
		"\n"
	);

	print_entry(
		"#if HCC_ENABLE_VECTOR_EXTENSIONS\n"
		"#if defined(__HCC__)\n"
		"#define HCC_DEFINE_VECTOR(vector_t, scalar_t, num_comps) typedef __hcc_vector_t(scalar_t, num_comps) vector_t\n"
		"#elif defined(__clang__)\n"
		"#define HCC_DEFINE_VECTOR(vector_t, scalar_t, num_comps) typedef scalar_t vector_t __attribute__((ext_vector_type(num_comps))) __attribute__((aligned(sizeof(scalar_t))))\n"
		"#endif\n"
		"#endif\n"
		"\n"
	);

	print_entry(
		"#if defined(__HCC__)\n"
		"typedef __hcc_half_t half;\n"
		"#define HALF_CONST(lit, bits) (lit)\n"
		"#define HCC_NATIVE_F16_SUPPORT\n"
		"#elif defined(__clang__)\n"
		"typedef _Float16 half;\n"
		"#define HALF_CONST(lit, bits) (lit)\n"
		"#define HCC_NATIVE_F16_SUPPORT\n"
		"#else\n"
		"typedef struct half { uint16_t _bits; } half;\n"
		"#define HALF_CONST(lit, bits_) ((half) { .bits = bits_ })\n"
		"#endif\n"
		"\n"
	);

	print_section_header_scalar();
	fprintf(ctx.f,
		"\n"
		"#define PI_F16 HALF_CONST(3.14159265358979323846f16, 0x4248)\n"
		"#define PI_F32 3.14159265358979323846f\n"
		"#define PI_F64 3.14159265358979323846\n"
		"#define INFINITY_F16 HALF_CONST(1.f16 / 0.f16, 0x7c00)\n"
		"#define INFINITY_F32 (1.f / 0.f)\n"
		"#define INFINITY_F64 (1.0 / 0.0)\n"
		"#define NAN_F16 HALF_CONST(0.f16 / 0.f16, 0xfc00)\n"
		"#define NAN_F32 (0.f / 0.f)\n"
		"#define NAN_F64 (0.0 / 0.0)\n"
		"\n"
	);

	print_section_header_packed_vector();
	//
	// all of the packed vector types
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef struct p$vs { $dI x; $dI y; } p$vs;\n");
		}

		fprintf(ctx.f,"\n");

		ctx.vector = VECTOR_3;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef struct p$vs { $dI x; $dI y; $dI z; } p$vs;\n");
		}

		fprintf(ctx.f,"\n");

		ctx.vector = VECTOR_4;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef struct p$vs { $dI x; $dI y; $dI z; $dI w; } p$vs;\n");
		}
	}

	//
	// packed vector constructors ewww
	{
		print_vector_fn_docs(VECTOR_FN_PINIT);

		ctx.vector = VECTOR_2;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define p$vs(x, y) ((p$vs){ x, y })\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define p$vs(x, y, z) ((p$vs){ x, y, z })\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define p$vs(x, y, z, w) ((p$vs){ x, y, z, w })\n");
		}

		fprintf(ctx.f,"\n");
	}


	print_section_header_vector();
	print_entry("#if HCC_ENABLE_VECTOR_EXTENSIONS\n");
	for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
		ctx.vector = vector;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("HCC_DEFINE_VECTOR($vs, $di, $vc);\n");
		}
		fprintf(ctx.f,"\n");
	}
	print_entry("#else //!HCC_ENABLE_VECTOR_EXTENSIONS\n");

	for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
		ctx.vector = vector;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("typedef struct $vs $vs;\n");
		}
		fprintf(ctx.f,"\n");
	}

	ctx.vector = VECTOR_2;
	for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"struct $vs {\n"
				"\t_Alignas($va)\n"
				"\tstruct { $di x; $di y; };\n"
				"\tstruct { $di r; $di g; };\n"
			"};\n\n");
	}

	ctx.vector = VECTOR_3;
	for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"struct $vs {\n"
				"\t_Alignas($va)\n"
				"\tstruct { $di x; $di y; $di z; $di _w; };\n"
				"\tstruct { $di r; $di g; $di b; $di _a; };\n"
				"\t$v2 xy;\n"
				"\t$v2 rg;\n"
			"};\n\n");
	}

	ctx.vector = VECTOR_4;
	for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
		ctx.data_type = data_type;
		print_entry(
			"struct $vs {\n"
				"\t_Alignas($va)\n"
				"\tstruct { $di x; $di y; $di z; $di w; };\n"
				"\tstruct { $di r; $di g; $di b; $di a; };\n"
				"\tstruct { $v2 xy; $v2 zw; };\n"
				"\tstruct { $v2 rg; $v2 ba; };\n"
				"\t$v3 xyz;\n"
				"\t$v3 rgb;\n"
			"};\n");
	}

	print_entry("#endif\n");

	//
	// vector constructors ewww
	{
		print_vector_fn_docs(VECTOR_FN_INIT);

		ctx.vector = VECTOR_2;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define $vs(x, y) (($vs){ x, y })\n");
			print_entry("#define $vss(s) (($vs){ s, s })\n");
		}
		fprintf(ctx.f,"\n");
		ctx.vector = VECTOR_3;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define $vs(x, y, z) (($vs){ x, y, z })\n");
			print_entry("#define $vss(s) (($vs){ s, s, s })\n");
			print_entry("#define $vssv2(x, v) (($vs){ x, (v).x, (v).y })\n");
			print_entry("#define $vsv2s(v, z) (($vs){ (v).x, (v).y, z })\n");
		}
		fprintf(ctx.f,"\n");
		ctx.vector = VECTOR_4;
		for (DataType data_type = 0; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define $vs(x, y, z, w) (($vs){ x, y, z, w })\n");
			print_entry("#define $vss(s) (($vs){ s, s, s, s })\n");
			print_entry("#define $vssv2(x, y, v) (($vs){ x, y, (v).x, (v).y })\n");
			print_entry("#define $vsv2s(v, z, w) (($vs){ (v).x, (v).y, z, w })\n");
			print_entry("#define $vssv2s(x, v, w) (($vs){ x, (v).x, (v).y, w })\n");
			print_entry("#define $vssv3(x, v) (($vs){ x, (v).x, (v).y, (v).z })\n");
			print_entry("#define $vsv3s(v, w) (($vs){ (v).x, (v).y, (v).z, w })\n");
		}
	}
	fprintf(ctx.f,"\n");

	//
	// vector infinity
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define INFINITY_$vS $vs(INFINITY_$dX, INFINITY_$dX)\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define INFINITY_$vS $vs(INFINITY_$dX, INFINITY_$dX, INFINITY_$dX)\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define INFINITY_$vS $vs(INFINITY_$dX, INFINITY_$dX, INFINITY_$dX, INFINITY_$dX)\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// vector neginfinity
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NEGINFINITY_$vS $vs(NEGINFINITY_$dX, NEGINFINITY_$dX)\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NEGINFINITY_$vS $vs(NEGINFINITY_$dX, NEGINFINITY_$dX, NEGINFINITY_$dX)\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NEGINFINITY_$vS $vs(NEGINFINITY_$dX, NEGINFINITY_$dX, NEGINFINITY_$dX, NEGINFINITY_$dX)\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// vector nan
	{
		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NAN_$vS $vs(NAN_$dX, NAN_$dX)\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NAN_$vS $vs(NAN_$dX, NAN_$dX, NAN_$dX)\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("#define NAN_$vS $vs(NAN_$dX, NAN_$dX, NAN_$dX, NAN_$dX)\n");
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
			print_entry("typedef struct p$mx { p$mv cols[$mr]; } p$mx;\n");
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
			print_entry("typedef struct $mx { $mv cols[$mr]; } $mx;\n");
		}
		fprintf(ctx.f,"\n");
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
					print_entry("#define IDENTITY_$mX (($mx) { .cols[0].x = 1.f, .cols[1].y = 1.f, .cols[2].z = 1.f, .cols[3].w = 1.f })\n");
				} else if (cols >= 3 && rows >= 3) {
					print_entry("#define IDENTITY_$mX (($mx) { .cols[0].x = 1.f, .cols[1].y = 1.f, .cols[2].z = 1.f })\n");
				} else {
					print_entry("#define IDENTITY_$mX (($mx) { .cols[0].x = 1.f, .cols[1].y = 1.f })\n");
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
	fprintf(ctx.f,"\n");

	print_section_header("Misc", "");

	//
	// swizzle operands
	if (0) {
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

	ctx.data_type = DATA_TYPE_HALF;
	print_section_header_half();
	print_entry(
		"float f16tof32(half v);\n"
		"double f16tof64(half v);\n"
		"half f32tof16(float v);\n"
		"half f64tof16(double v);\n"

		"static inline half add_f16(half a, half b) { return f32tof16(f16tof32(a) + f16tof32(b)); }\n"
		"static inline half sub_f16(half a, half b) { return f32tof16(f16tof32(a) - f16tof32(b)); }\n"
		"static inline half mul_f16(half a, half b) { return f32tof16(f16tof32(a) * f16tof32(b)); }\n"
		"static inline half div_f16(half a, half b) { return f32tof16(f16tof32(a) / f16tof32(b)); }\n"
		"static inline bool eq_f16(half a, half b) { return f16tof32(a) == f16tof32(b); }\n"
		"static inline bool neq_f16(half a, half b) { return f16tof32(a) != f16tof32(b); }\n"
		"static inline bool lt_f16(half a, half b) { return f16tof32(a) < f16tof32(b); }\n"
		"static inline bool lteq_f16(half a, half b) { return f16tof32(a) <= f16tof32(b); }\n"
		"static inline bool gt_f16(half a, half b) { return f16tof32(a) > f16tof32(b); }\n"
		"static inline bool gteq_f16(half a, half b) { return f16tof32(a) >= f16tof32(b); }\n"
		"static inline bool not_f16(half v) { return !f16tof32(v); }\n"
		"static inline half neg_f16(half v) { return f32tof16(-f16tof32(v)); }\n"
		"\n"
	);

	print_section_header_scalar();

	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("bool isinf_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("bool isnan_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di mod_$dx($di a, $di b);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di floor_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di ceil_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di round_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di trunc_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di sin_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di cos_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di tan_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di asin_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di acos_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di atan_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di sinh_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di cosh_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di tanh_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di asinh_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di acosh_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di atanh_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di atan2_$dx($di y, $di x);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di fma_$dx($di a, $di b, $di c);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di sqrt_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di pow_$dx($di a, $di b);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di exp_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di log_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di exp2_$dx($di v);\n");
	}
	print_entry("\n");
	for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di log2_$dx($di v);\n");
	}
	print_entry("\n");

	//
	// scalar min
	{
		fprintf(ctx.f,
			"//\n"
			"// returns the minimum value between 'a' and 'b'\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half min_$dx(half a, half b) { return lt_$dx(a, b) ? a : b; }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di min_$dx($di a, $di b) { return a < b ? a : b; }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half max_$dx(half a, half b) { return gt_$dx(a, b) ? a : b; }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di max_$dx($di a, $di b) { return a > b ? a : b; }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half clamp_$dx(half v, half min, half max) { return gt_$dx(v, max) ? max : (gteq_$dx(v, min) ? v : min); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di clamp_$dx($di v, $di min, $di max) { return v > max ? max : (v >= min ? v : min); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar abs
	{
		fprintf(ctx.f,
			"//\n"
			"// returns the absolute (positive) value of 'v'\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di abs_$dx($di v) { return f16tof32(v) < 0.f ? neg_f16(v) : v; }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di abs_$dx($di v) { return v < 0.f ? -v : v; }\n");
		}
		for (DataType data_type = DATA_TYPE_S8; data_type <= DATA_TYPE_S64; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di abs_$dx($di v) { return (v &= ~$dm); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar rsqrt
	{
		fprintf(ctx.f,
			"//\n"
			"// returns the reciprocal square root of 'v' aka. inverse square root\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half rsqrt_$dx(half v) { return f32tof16(1.f / sqrt_f32(f16tof32(v))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di rsqrt_$dx($di v) { return 1.f / sqrt_$dx(v); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar approxeq
	{
		fprintf(ctx.f,
			"//\n"
			"// returns true if 'a' and 'b' are 'epsilon' away from eachother\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline bool approxeq_$dx(half a, half b, half epsilon) { return abs_f32(f16tof32(a) - f16tof32(b)) <= f16tof32(epsilon); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline bool approxeq_$dx($di a, $di b, $di epsilon) { return abs_$dx(a - b) <= epsilon; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar sign
	{
		fprintf(ctx.f,
			"//\n"
			"// returns -1 if 'v' is less than 0, 1 if 'v' is greater than 0 or 0 if 'v' is 0\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di sign_$dx($di v) { return f32tof16(f16tof32(v) == 0.f ? 0.f : (f16tof32(v) < 0.f ? -1.f : 1.f)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di sign_$dx($di v) { return v == 0.f ? 0.f : (v < 0.f ? -1.f : 1.f); }\n");
		}
		for (DataType data_type = DATA_TYPE_S8; data_type <= DATA_TYPE_S64; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di sign_$dx($di v) { return v == 0 ? 0 : (v < 0 ? -1 : 1); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar copysign
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a 'v' with sign copied from 'sign'\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di copysign_$dx($di v, $di sign) { return f32tof16(f16tof32(v) * (f16tof32(sign) < 0.f ? -1.f : 1.f)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di copysign_$dx($di v, $di sign) { return v * (sign < 0.f ? -1.f : 1.f); }\n");
		}
		for (DataType data_type = DATA_TYPE_S8; data_type <= DATA_TYPE_S64; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di copysign_$dx($di v, $di sign) { return v | (sign & $dm); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar lerp
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a linear interpolation from 'start' to 'end' at the point of 't' where 't' = 0.f = 'start' and 't' = 1.f = 'end'\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half lerp_$dx(half start, half end, half t) { return add_$dx(mul_$dx(sub_$dx(end, start), t), start); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di lerp_$dx($di start, $di end, $di t) { return (end - start) * t + start; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar inv lerp
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a value from 0.f to 1.f at the point where 'v' is in relation to 'start' and 'end' where 'v' = 0.f = 'start' and 'v' = 1.f = 'end'\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half invlerp_$dx(half start, half end, half v) { return div_$dx(sub_$dx(v, start), sub_$dx(end, start)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di invlerp_$dx($di start, $di end, $di v) { return (v - start) / (end - start); }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half fract_$dx(half v) { return sub_$dx(v, floor_$dx(v)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di fract_$dx($di v) { return v - floor_$dx(v); }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half degrees_$dx(half v) { return f32tof16(f16tof32(v) * (180.f / PI_F32)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di degrees_$dx($di v) { return v * (180.f / PI_$dX); }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half radians_$dx(half v) { return f32tof16(f16tof32(v) * (PI_F32 / 180.f)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di radians_$dx($di v) { return v * (PI_$dX / 180.f); }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar step
	{
		fprintf(ctx.f,
			"//\n"
			"// returns 0.f if 'v' < 'edge', otherwise 1.f is returned\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half step_$dx(half edge, half v) { return f32tof16(f16tof32(v) ? 0.f : 1.f); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di step_$dx($di edge, $di v) { return v < edge ? 0.f : 1.f; }\n");
		}
		fprintf(ctx.f,"\n");
	}

	//
	// scalar smoothstep
	{
		fprintf(ctx.f,
			"//\n"
			"// returns a smooth Hermite interpolation between 0.f and 1.f when 'edge0' < 'x' < 'edge1'\n"
		);
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half smoothstep_$dx($di edge0, $di edge1, $di v) { float t = clamp_f32((f16tof32(v) - f16tof32(edge0)) / (f16tof32(edge1) - f16tof32(edge0)), 0.f, 1.f); return f32tof16(t * t * (3.f - 2.f * t)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di smoothstep_$dx($di edge0, $di edge1, $di v) { $di t = clamp_$dx((v - edge0) / (edge1 - edge0), 0.f, 1.f); return t * t * (3.f - 2.f * t); }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half remap_$dx(half v, half from_min, half from_max, half to_min, half to_max) { return add_$dx(to_min, div_$dx(mul_$dx(sub_$dx(v, from_min), sub_$dx(to_max, to_min)), sub_$dx(from_max, from_min))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di remap_$dx($di v, $di from_min, $di from_max, $di to_min, $di to_max) { return to_min + (v - from_min) * (to_max - to_min) / (from_max - from_min); }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half roundtomultiple_$dx(half v, half multiple) { v = fma_$dx(multiple, f32tof16(0.5f), v); $di rem = mod_$dx(v, multiple); if (gt_$dx(v, f32tof16(0.f))) { return sub_$dx(v, rem); } else { return sub_$dx(sub_$dx(v, rem), multiple); } }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di roundtomultiple_$dx($di v, $di multiple) { v = fma_$dx(multiple, 0.5f, v); $di rem = mod_$dx(v, multiple); if (v > 0.f) { return v - rem; } else { return v - rem - multiple; } }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half rounduptomultiple_$dx(half v, half multiple) { $di rem = mod_$dx(v, multiple); if (gt_$dx(v, f32tof16(0.f))) { return sub_$dx(add_$dx(v, multiple), rem); } else { return sub_$dx(v, rem); } }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di rounduptomultiple_$dx($di v, $di multiple) { $di rem = mod_$dx(v, multiple); if (v > 0.f) { return v + multiple - rem; } else { return v - rem; } }\n");
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
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half rounddowntomultiple_$dx(half v, half multiple) { $di rem = mod_$dx(v, multiple); if (gt_$dx(v, f32tof16(0.f))) { return sub_$dx(v, rem); } else { return sub_$dx(sub_$dx(v, rem), multiple); } }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di rounddowntomultiple_$dx($di v, $di multiple) { $di rem = mod_$dx(v, multiple); if (v > 0.f) { return v - rem; } else { return v - rem - multiple; } }\n");
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
			print_entry("static inline $di bitsto_$dx(uint$db_t v) { union { uint$db_t u; $di f; } d = { .u = v }; return d.f; }\n");
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
			print_entry("static inline uint$db_t bitsfrom_$dx($di v) { union { uint$db_t u; $di f; } d = { .f = v }; return d.u; }\n");
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
	if (0) {
		print_vector_fn_docs(VECTOR_FN_SWIZZLE);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs swizzle$vs($vs v, uint8_t x, uint8_t y) { return $vs(v.array[x], v.array[y]); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs swizzle$vs($vs v, uint8_t x, uint8_t y, uint8_t z) { return $vs(v.array[x], v.array[y], v.array[z]); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs swizzle$vs($vs v, uint8_t x, uint8_t y, uint8_t z, uint8_t w) { return $vs(v.array[x], v.array[y], v.array[z], v.array[w]); }\n");
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
				if (vector_is_not_intrinsic[fn]) {
					print_entry("static inline ");
				} else {
					print_entry("static inline ");
				}

				if (vector_fn_return_vector_bool[fn]) {
					print_entry("bool$vx ");
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
				print_entry("_$vs(");
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

				if (vector_fn_has_scalar_params(fn) && !vector_is_not_intrinsic[vector_scalar_non_scalar_fn[fn]]) { // has scalar params and the callee function is intrinsic
					fprintf(ctx.f," { ");
					for (unsigned param_idx = 0; param_idx < vector_fn_number_params[fn]; param_idx += 1) {
						if (!function_param_is_scalar(fn, param_idx)) {
							continue;
						}
						print_entry("$vs ");
						fprintf(ctx.f,"%ss", function_param_ident(fn, param_idx));
						print_entry(" = $vss(");
						fprintf(ctx.f, "%s", function_param_ident(fn, param_idx));
						print_entry("); ");
					}

					fprintf(ctx.f,"return %s", vector_fn_idents[vector_scalar_non_scalar_fn[fn]]);
					print_entry("_$vs(");
					for (unsigned param_idx = 0; param_idx < vector_fn_number_params[fn]; param_idx += 1) {
						fprintf(ctx.f,
							"%s%s%s",
							function_param_ident(fn, param_idx),
							function_param_is_scalar(fn, param_idx) ? "s" : "",
							param_idx + 1 < vector_fn_number_params[fn] ? ", " : ""
						);
					}
					print_entry("); }");
				} else {
					fprintf(ctx.f," { return ");

					if (vector_fn_return_vector_bool[fn]) {
						print_entry("bool$vx(");
					} else if (fn == VECTOR_FN_BITSFROM) {
						print_entry("$vb(");
					} else if (fn == VECTOR_FN_PACK) {
						print_entry("p$vs(");
					} else if (!vector_fn_returns_bool[fn]) {
						print_entry("$vs(");
					}

					unsigned num_comp = vector_comps[vector];
					for (unsigned comp = 0; comp < num_comp; comp += 1) {
						if (fn == VECTOR_FN_ALL || fn == VECTOR_FN_ANY) {
							if (ctx.data_type == DATA_TYPE_HALF) {
								print_entry("f16tof32(");
							}
							print_entry(function_param_ident(fn, 0));
							print_entry(".");
							print_entry(xyzw_idents[comp]);
							if (ctx.data_type == DATA_TYPE_HALF) {
								print_entry(")");
							}
						} else if (vector_fn_no_operator_or_calls[fn]) {
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
							const char* call_ident = vector_fn_has_scalar_params(fn) ? vector_fn_idents[vector_scalar_non_scalar_fn[fn]] : fn_ident;
							print_entry(call_ident);
							print_entry("_$dx(");
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

				fprintf(ctx.f,"\n");
			}
		}
	}

	//
	// vector cross
	{
		print_vector_fn_docs(VECTOR_FN_DOT);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half cross_$vs($vs a, $vs b) { return sub_$dx(mul_$dx(a.x, b.y), mul_$dx(b.x, a.y)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di cross_$vs($vs a, $vs b) { return (a.x * b.y) - (b.x * a.y); }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half cross_$vs($vs a, $vs b) { return add_$dx(mul_$dx(a.x, b.x), add_$dx(mul_$dx(a.y, b.y), mul_$dx(a.z, b.z))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry(
				"static inline $vs cross_$vs($vs a, $vs b) {\n"
				"\treturn $vs(\n"
				"\t\ta.y * b.z - a.z * b.y,\n"
				"\t\ta.z * b.x - a.x * b.z,\n"
				"\t\ta.x * b.y - a.y * b.x\n"
				"\t);\n"
				"}\n"
			);
		}
	}

	//
	// vector dot
	{
		print_vector_fn_docs(VECTOR_FN_DOT);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half dot_$vs($vs a, $vs b) { return add_$dx(mul_$dx(a.x, b.x), mul_$dx(a.y, b.y)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di dot_$vs($vs a, $vs b) { return (a.x * b.x) + (a.y * b.y); }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half dot_$vs($vs a, $vs b) { return add_$dx(mul_$dx(a.x, b.x), add_$dx(mul_$dx(a.y, b.y), mul_$dx(a.z, b.z))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di dot_$vs($vs a, $vs b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }\n");
		}
		ctx.vector = VECTOR_4;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half dot_$vs($vs a, $vs b) { return add_$dx(mul_$dx(a.x, b.x), add_$dx(mul_$dx(a.y, b.y), add_$dx(mul_$dx(a.z, b.z), mul_$dx(a.w, b.w)))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di dot_$vs($vs a, $vs b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w); }\n");

		}
	}

	//
	// vector len
	{
		print_vector_fn_docs(VECTOR_FN_LEN);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half len_$vs($vs v) { return sqrt_$dx(add_$dx(mul_$dx(v.x, v.x), mul_$dx(v.y, v.y))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di len_$vs($vs v) { return sqrt_$dx((v.x * v.x) + (v.y * v.y)); }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half len_$vs($vs v) { return sqrt_$dx(add_$dx(mul_$dx(v.x, v.x), add_$dx(mul_$dx(v.y, v.y), mul_$dx(v.z, v.z)))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di len_$vs($vs v) { return sqrt_$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z)); }\n");
		}
		ctx.vector = VECTOR_4;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline half len_$vs($vs v) { return sqrt_$dx(add_$dx(mul_$dx(v.x, v.x), add_$dx(mul_$dx(v.y, v.y), add_$dx(mul_$dx(v.z, v.z), mul_$dx(v.w, v.w))))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di len_$vs($vs v) { return sqrt_$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w)); }\n");

		}
	}

	//
	// vector lensq
	{
		print_vector_fn_docs(VECTOR_FN_LENSQ);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di lensq_$vs($vs v) { return dot_$vs(v, v); }\n");
		}
		for (DataType data_type = DATA_TYPE_S8; data_type <= DATA_TYPE_U64; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di lensq_$vs($vs v) { return (v.x * v.x) + (v.y * v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di lensq_$vs($vs v) { return dot_$vs(v, v); }\n");
		}
		for (DataType data_type = DATA_TYPE_S8; data_type <= DATA_TYPE_U64; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di lensq_$vs($vs v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di lensq_$vs($vs v) { return dot_$vs(v, v); }\n");
		}
		for (DataType data_type = DATA_TYPE_S8; data_type <= DATA_TYPE_U64; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di lensq_$vs($vs v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w); }\n");
		}
	}

	//
	// vector norm
	{
		print_vector_fn_docs(VECTOR_FN_NORM);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $vs norm_$vs($vs v) { half k = rsqrt_$dx(add_$dx(mul_$dx(v.x, v.x), mul_$dx(v.y, v.y))); return $vs(mul_$dx(v.x, k), mul_$dx(v.y, k)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs norm_$vs($vs v) { $di k = rsqrt_$dx((v.x * v.x) + (v.y * v.y)); return $vs(v.x * k, v.y * k); }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $vs norm_$vs($vs v) { half k = rsqrt_$dx(add_$dx(mul_$dx(v.x, v.x), add_$dx(mul_$dx(v.y, v.y), mul_$dx(v.z, v.z)))); return $vs(mul_$dx(v.x, k), mul_$dx(v.y, k), mul_$dx(v.z, k)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs norm_$vs($vs v) { $di k = rsqrt_$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z)); return $vs(v.x * k, v.y * k, v.z * k); }\n");
		}
		ctx.vector = VECTOR_4;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $vs norm_$vs($vs v) { half k = rsqrt_$dx(add_$dx(mul_$dx(v.x, v.x), add_$dx(mul_$dx(v.y, v.y), add_$dx(mul_$dx(v.z, v.z), mul_$dx(v.w, v.w))))); return $vs(mul_$dx(v.x, k), mul_$dx(v.y, k), mul_$dx(v.z, k), mul_$dx(v.w, k)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs norm_$vs($vs v) { $di k = rsqrt_$dx((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w)); return $vs(v.x * k, v.y * k, v.z * k, v.w * k); }\n");

		}
	}

	//
	// vector reflect
	{
		print_vector_fn_docs(VECTOR_FN_REFLECT);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			ctx.data_type = DATA_TYPE_HALF;
			print_entry("$vs reflect_$vs($vs v, $vs normal);\n");
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry("$vs reflect_$vs($vs v, $vs normal);\n");
			}
		}
	}

	//
	// vector refract
	{
		print_vector_fn_docs(VECTOR_FN_REFRACT);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			ctx.data_type = DATA_TYPE_HALF;
			print_entry("$vs refract_$vs($vs v, $vs normal, $di eta);\n");
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry("$vs refract_$vs($vs v, $vs normal, $di eta);\n");
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
			print_entry("static inline $di minelmt_$vs($vs v) { return min_$dx(v.x, v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di minelmt_$vs($vs v) { return min_$dx(v.x, min_$dx(v.y, v.z)); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di minelmt_$vs($vs v) { return min_$dx(v.x, min_$dx(v.y, min_$dx(v.z, v.w))); }\n");
		}
	}

	//
	// vector maxelmt
	{
		print_vector_fn_docs(VECTOR_FN_MAX_ELMT);

		ctx.vector = VECTOR_2;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di maxelmt_$vs($vs v) { return max_$dx(v.x, v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di maxelmt_$vs($vs v) { return max_$dx(v.x, max_$dx(v.y, v.z)); }\n");
		}
		ctx.vector = VECTOR_4;
		for (DataType data_type = DATA_TYPE_HALF; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di maxelmt_$vs($vs v) { return max_$dx(v.x, max_$dx(v.y, max_$dx(v.z, v.w))); }\n");
		}
	}

	//
	// vector sumelmts
	{
		print_vector_fn_docs(VECTOR_FN_SUM_ELMTS);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di sumelmts_$vs($vs v) { return add_f16(v.x, v.y); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di sumelmts_$vs($vs v) { return v.x + v.y; }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di sumelmts_$vs($vs v) { return add_f16(v.x, add_f16(v.y, v.z)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di sumelmts_$vs($vs v) { return v.x + v.y + v.z; }\n");
		}
		ctx.vector = VECTOR_4;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di sumelmts_$vs($vs v) { return add_f16(v.x, add_f16(v.y, add_f16(v.z, v.w))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di sumelmts_$vs($vs v) { return v.x + v.y + v.z + v.w; }\n");
		}
	}

	//
	// vector productelmts
	{
		print_vector_fn_docs(VECTOR_FN_PRODUCT_ELMTS);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di productelmts_$vs($vs v) { return mul_f16(v.x, v.y); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di productelmts_$vs($vs v) { return v.x * v.y; }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di productelmts_$vs($vs v) { return mul_f16(v.x, mul_f16(v.y, v.z)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di productelmts_$vs($vs v) { return v.x * v.y * v.z; }\n");
		}
		ctx.vector = VECTOR_4;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $di productelmts_$vs($vs v) { return mul_f16(v.x, mul_f16(v.y, mul_f16(v.z, v.w))); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $di productelmts_$vs($vs v) { return v.x * v.y * v.z * v.w; }\n");
		}
	}

	//
	// vector square
	{
		print_vector_fn_docs(VECTOR_FN_SQUARE);

		ctx.vector = VECTOR_2;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $vs square_$vs($vs v) { return $vs(mul_$dx(v.x, v.x), mul_$dx(v.y, v.y)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs square_$vs($vs v) { return $vs(v.x * v.x, v.y * v.y); }\n");
		}
		ctx.vector = VECTOR_3;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $vs square_$vs($vs v) { return $vs(mul_$dx(v.x, v.x), mul_$dx(v.y, v.y), mul_$dx(v.z, v.z)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs square_$vs($vs v) { return $vs(v.x * v.x, v.y * v.y, v.z * v.z); }\n");
		}
		ctx.vector = VECTOR_4;
		ctx.data_type = DATA_TYPE_HALF;
		print_entry("static inline $vs square_$vs($vs v) { return $vs(mul_$dx(v.x, v.x), mul_$dx(v.y, v.y), mul_$dx(v.z, v.z), mul_$dx(v.w, v.w)); }\n");
		for (DataType data_type = DATA_TYPE_FLOAT; data_type < DATA_TYPE_COUNT; data_type += 1) {
			ctx.data_type = data_type;
			print_entry("static inline $vs square_$vs($vs v) { return $vs(v.x * v.x, v.y * v.y, v.z * v.z, v.w * v.w); }\n");
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// packs a f32x2 into a 32 bit integer comprised of two float 16 bits\n"
		"uint32_t pack_f16x2_f32x2(f32x2 v);\n"
		"\n"
		"//\n"
		"// unpacks a f32x2 from a 32 bit integer comprised of two float 16 bits\n"
		"f32x2 unpack_f16x2_f32x2(uint32_t v);\n"
		"\n"
		"//\n"
		"// packs a unsigned normalized f32x2 into a 32 bit integer where each component is given 16 bits\n"
		"uint32_t pack_u16x2_f32x2(f32x2 v);\n"
		"\n"
		"//\n"
		"// unpacks a unsigned normalized f32x2 from a 32 bit integer where each component is given 16 bits\n"
		"f32x2 unpack_u16x2_f32x2(uint32_t v);\n"
		"\n"
		"//\n"
		"// packs a signed normalized f32x2 into a 32 bit integer where each component is given 16 bits\n"
		"uint32_t pack_s16x2_f32x2(f32x2 v);\n"
		"\n"
		"//\n"
		"// unpacks a signed normalized f32x2 from a 32 bit integer where each component is given 16 bits\n"
		"f32x2 unpack_s16x2_f32x2(uint32_t v);\n"
		"\n"
		"//\n"
		"// packs a unsigned normalized f32x4 into a 32 bit integer where each component is given 8 bits\n"
		"uint32_t pack_s8x4_f32x4(f32x4 v);\n"
		"\n"
		"//\n"
		"// unpacks a unsigned normalized f32x4 from a 32 bit integer where each component is given 8 bits\n"
		"f32x4 unpack_u8x4_f32x4(uint32_t v);\n"
		"\n"
		"//\n"
		"// packs a signed normalized f32x4 into a 32 bit integer where each component is given 8 bits\n"
		"uint32_t pack_s8x4_f32x4(f32x4 v);\n"
		"\n"
		"//\n"
		"// unpacks a signed normalized f32x4 from a 32 bit integer where each component is given 8 bits\n"
		"f32x4 unpack_u8x4_f32x4(uint32_t v);\n"
		"\n"
	);

	print_section_header(
		"Matrix",
		""
	);

	fprintf(ctx.f,
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
				snprintf(leftname, sizeof(leftname), "%sx%ux%u", data_type_suffixes[data_type], leftc, leftr);
				char rightname[128];
				snprintf(rightname, sizeof(rightname), "%sx%ux%u", data_type_suffixes[data_type], rightc, rightr);
				fprintf(ctx.f, "%s mul_%sx%ux%u_%sx%ux%u(%s a, %s b);\n", leftname, data_type_suffixes[data_type], leftc, leftr, data_type_suffixes[data_type], rightc, rightr, leftname, rightname);
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
			print_entry("$mx muls_$mx($mx m, $di s);\n");
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
				snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], leftc, leftr);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "%sx%u", data_type_suffixes[data_type], comp_count);
				fprintf(ctx.f, "%s mul_%sx%ux%u_%sx%u(%s m, %s v);\n", vectorname, data_type_suffixes[data_type], leftc, leftr, data_type_suffixes[data_type], comp_count, matrixname, vectorname);
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
				snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], leftc, leftr);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "%sx%u", data_type_suffixes[data_type], comp_count);
				fprintf(ctx.f, "%s mul_%sx%u_%sx%ux%u(%s v, %s m);\n", vectorname, data_type_suffixes[data_type], comp_count, data_type_suffixes[data_type], leftc, leftr, vectorname, matrixname);
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
			snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], matrixc, matrixr);
			char retmatrixname[128];
			snprintf(retmatrixname, sizeof(retmatrixname), "%sx%ux%u", data_type_suffixes[data_type], matrixr, matrixc);
			fprintf(ctx.f, "%s transpose_%sx%ux%u(%s m);\n", retmatrixname, data_type_suffixes[data_type], matrixc, matrixr, matrixname);
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
				snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], leftcomp_count, rightcomp_count);
				char leftvectorname[128];
				snprintf(leftvectorname, sizeof(leftvectorname), "%sx%u", data_type_suffixes[data_type], leftcomp_count);
				char rightvectorname[128];
				snprintf(rightvectorname, sizeof(rightvectorname), "%sx%u", data_type_suffixes[data_type], rightcomp_count);
				fprintf(ctx.f, "%s outerproduct_%sx%u_%sx%u(%s c, %s r);\n", matrixname, data_type_suffixes[data_type], leftcomp_count, data_type_suffixes[data_type], rightcomp_count, leftvectorname, rightvectorname);
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
			print_entry("$di determinant_$mx($mx m);\n");
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
			print_entry("$mx inverse_$mx($mx m);\n");
		}
	}

	fprintf(ctx.f,
		"\n"
		"//\n"
		"// on HCC include the math.c file automatically so that all shaders files get the implementation of the larger functions compiled in.\n"
		"#ifdef __HCC__\n"
		"#include \"math.c\"\n"
		"#endif // __HCC__\n"
	);

	print_header_file_footer("_HCC_STD_MATH_H_");
}

void print_mat3x3_determinant(void) {
	print_entry(
		"\t$di s[3];\n"
		"\ts[0] = (m.cols[1].y * m.cols[2].z) - (m.cols[2].y * m.cols[1].z);\n"
		"\ts[1] = (m.cols[1].x * m.cols[2].z) - (m.cols[1].z * m.cols[2].x);\n"
		"\ts[2] = (m.cols[1].x * m.cols[2].y) - (m.cols[1].y * m.cols[2].x);\n"
		"\t$di det = s[0] - s[1] + s[2];\n"
	);
}

void print_mat4x4_determinant(void) {
	print_entry(
		"\t$di s[6];\n"
		"\t$di c[6];\n"
		"\n"
		"\ts[0] = m.cols[0].x*m.cols[1].y - m.cols[1].x*m.cols[0].y;\n"
		"\ts[1] = m.cols[0].x*m.cols[1].z - m.cols[1].x*m.cols[0].z;\n"
		"\ts[2] = m.cols[0].x*m.cols[1].w - m.cols[1].x*m.cols[0].w;\n"
		"\ts[3] = m.cols[0].y*m.cols[1].z - m.cols[1].y*m.cols[0].z;\n"
		"\ts[4] = m.cols[0].y*m.cols[1].w - m.cols[1].y*m.cols[0].w;\n"
		"\ts[5] = m.cols[0].z*m.cols[1].w - m.cols[1].z*m.cols[0].w;\n"
		"\n"
		"\tc[0] = m.cols[2].x*m.cols[3].y - m.cols[3].x*m.cols[2].y;\n"
		"\tc[1] = m.cols[2].x*m.cols[3].z - m.cols[3].x*m.cols[2].z;\n"
		"\tc[2] = m.cols[2].x*m.cols[3].w - m.cols[3].x*m.cols[2].w;\n"
		"\tc[3] = m.cols[2].y*m.cols[3].z - m.cols[3].y*m.cols[2].z;\n"
		"\tc[4] = m.cols[2].y*m.cols[3].w - m.cols[3].y*m.cols[2].w;\n"
		"\tc[5] = m.cols[2].z*m.cols[3].w - m.cols[3].z*m.cols[2].w;\n"
		"\n"
		"\t$di det = s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0];\n"
	);
}

void generate_math_file() {
	ctx.f = open_file_write("libhccstd/math.c");
	print_file_header();

	print_section_header("Half type aka. float 16 bit", "");
	fprintf(ctx.f,
		"float f16tof32(half v) {\n"
		"#ifdef HCC_NATIVE_F16_SUPPORT\n"
			"\treturn (float)v;\n"
		"#else\n"
			"\tif ((v._bits & 0x7c00) == 0x7c00) { // inf, -inf or nan\n"
				"\t\tif (v._bits & 0x03ff) return NAN_F32;\n"
				"\t\telse if (v._bits & 0x8000) return -INFINITY_F32;\n"
				"\t\telse return INFINITY_F32;\n"
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
		"#endif\n"
		"}\n"
		"\n"
		"double f16tof64(half v) {\n"
			"\treturn (double)f16tof32(v);\n"
		"}\n"
		"\n"
		"half f32tof16(float v) {\n"
		"#ifdef HCC_NATIVE_F16_SUPPORT\n"
			"\treturn (half)v;\n"
		"#else\n"
			"\tif (isinf_f32(v)) return (half){ ._bits = v < 0.f ? 0xfc00 : 0x7c00 };\n"
			"\tif (isnan_f32(v)) return (half){ ._bits = 0xffff };\n"
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
		"#endif\n"
		"}\n"
		"\n"
		"half f64tof16(double v) {\n"
			"\treturn f32tof16((float)v);\n"
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
				"$vs reflect_$vs($vs v, $vs normal) {\n"
					"\t$di dot_2 = mul_$dx(dot_$vs(normal, v), f32tof16(2.f));\n"
					"\treturn sub_$vs(v, muls_$vs(normal, dot_2));\n"
				"}\n"
			);
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry(
					"$vs reflect_$vs($vs v, $vs normal) {\n"
						"\t$di dot_2 = dot_$vs(normal, v) * 2.f;\n"
						"\treturn sub_$vs(v, muls_$vs(normal, dot_2));\n"
					"}\n"
				);
			}
		}
	}

	//
	// vector refract
	{
		print_vector_fn_docs(VECTOR_FN_REFRACT);

		for (Vector vector = VECTOR_2; vector < VECTOR_COUNT; vector += 1) {
			ctx.vector = vector;
			ctx.data_type = DATA_TYPE_HALF;
			print_entry(
				"$vs refract_$vs($vs v, $vs normal, $di eta) {\n"
					"\t$di dot = dot_$vs(normal, v);\n"
					"\t$di inv_dot_sq = sub_$dx(f32tof16(1.f), mul_$dx(dot, dot));\n"
					"\t$di eta_sq = mul_$dx(eta, eta);\n"
					"\t$di k = sub_$dx(f32tof16(1.f), mul_$dx(eta_sq, inv_dot_sq));\n"
					"\tif (lt_$dx(k, f32tof16(0.f))) {\n"
						"\t\treturn $vss(f32tof16(0.f));\n"
					"\t}\n"
					"\treturn sub_$vs(muls_$vs(v, eta), muls_$vs(normal, ((add_$dx(mul_$dx(eta, dot_$vs(normal, v)), sqrt_$dx(k))))));\n"
				"}\n"
			);
			for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
				ctx.data_type = data_type;
				print_entry(
					"$vs refract_$vs($vs v, $vs normal, $di eta) {\n"
						"\t$di dot = dot_$vs(normal, v);\n"
						"\t$di inv_dot_sq = 1.f - dot * dot;\n"
						"\t$di eta_sq = eta * eta;\n"
						"\t$di k = 1.f - eta_sq * inv_dot_sq;\n"
						"\tif (k < 0.f) {\n"
							"\t\treturn $vss(0.f);\n"
						"\t}\n"
						"\treturn sub_$vs(muls_$vs(v, eta), muls_$vs(normal, ((eta * dot_$vs(normal, v) + sqrt_f32(k)))));\n"
					"}\n"
				);
			}
		}
	}

	fprintf(ctx.f,
		"\n"
		"uint32_t pack_f16x2_f32x2(f32x2 v) {\n"
			"\treturn\n"
				"\t\t((uint32_t)bitsfrom_f16(f32tof16(v.x)) << 0)  ||\n"
				"\t((uint32_t)bitsfrom_f16(f32tof16(v.y)) << 16)  ;\n"
		"}\n"
		"\n"
		"f32x2 unpack_f16x2_f32x2(uint32_t v) {\n"
			"\treturn f32x2(\n"
				"\t\tf16tof32(bitsto_f16(v & 0xffff)),\n"
				"\t\tf16tof32(bitsto_f16(v >> 16))\n"
			"\t);\n"
		"}\n"
		"\n"
		"uint32_t pack_u16x2_f32x2(f32x2 v) {\n"
			"\tv = round_f32x2(muls_f32x2(clamps_f32x2(v, 0.f, 1.f), 65535.f));\n"
			"\treturn\n"
				"\t\t((uint32_t)v.x << 0) ||\n"
				"\t\t((uint32_t)v.y << 16) ;\n"
		"}\n"
		"\n"
		"f32x2 unpack_u16x2_f32x2(uint32_t v) {\n"
			"\treturn f32x2(\n"
				"\t\t(float)(v & 0xffff) / 65535.f,\n"
				"\t\t(float)(v >> 16) / 65535.f\n"
			"\t);\n"
		"}\n"
		"\n"
		"uint32_t pack_s16x2_f32x2(f32x2 v) {\n"
			"\tv = round_f32x2(muls_f32x2(clamps_f32x2(v, -1.f, 1.f), 32767.f));\n"
			"\treturn\n"
				"\t\t((uint32_t)(uint16_t)(int16_t)v.x << 0) ||\n"
				"\t\t((uint32_t)(uint16_t)(int16_t)v.y << 16) ;\n"
		"}\n"
		"\n"
		"f32x2 unpack_s16x2_f32x2(uint32_t v) {\n"
			"\treturn f32x2(\n"
				"\t\tclamp_f32((float)(int32_t)(int16_t)(v & 0xffff) / 32767.f, -1.f, 1.f),\n"
				"\t\tclamp_f32((float)(int32_t)(int16_t)(v >> 16) / 32767.f, -1.f, 1.f)\n"
			"\t);\n"
		"}\n"
		"\n"
		"uint32_t pack_u8x4_f32x4(f32x4 v) {\n"
			"\tv = round_f32x4(muls_f32x4(clamps_f32x4(v, 0.f, 1.f), 255.f));\n"
			"\treturn\n"
				"\t\t((uint32_t)v.x << 0)  ||\n"
				"\t\t((uint32_t)v.y << 8)  ||\n"
				"\t\t((uint32_t)v.z << 16) ||\n"
				"\t\t((uint32_t)v.w << 24)  ;\n"
		"}\n"
		"\n"
		"f32x4 unpack_u8x4_f32x4(uint32_t v) {\n"
			"\treturn f32x4(\n"
				"\t\t(float)((v >> 0)  & 0xff) / 255.f,\n"
				"\t\t(float)((v >> 8)  & 0xff) / 255.f,\n"
				"\t\t(float)((v >> 16) & 0xff) / 255.f,\n"
				"\t\t(float)((v >> 24) & 0xff) / 255.f\n"
			"\t);\n"
		"}\n"
		"\n"
		"uint32_t pack_s8x4_f32x4(f32x4 v) {\n"
			"\tv = round_f32x4(muls_f32x4(clamps_f32x4(v, -1.f, 1.f), 127.f));\n"
			"\treturn\n"
				"\t\t((uint32_t)(uint8_t)(int8_t)v.x << 0)  ||\n"
				"\t\t((uint32_t)(uint8_t)(int8_t)v.y << 8)  ||\n"
				"\t\t((uint32_t)(uint8_t)(int8_t)v.z << 16) ||\n"
				"\t\t((uint32_t)(uint8_t)(int8_t)v.w << 24)  ;\n"
		"}\n"
		"\n"
		"f32x4 unpack_s8x4_f32x4(uint32_t v) {\n"
			"\treturn clamps_f32x4(\n"
				"\t\tf32x4(\n"
					"\t\t\t((float)(int32_t)(int8_t)((v >> 0)  & 0xff) / 127.f),\n"
					"\t\t\t((float)(int32_t)(int8_t)((v >> 8)  & 0xff) / 127.f),\n"
					"\t\t\t((float)(int32_t)(int8_t)((v >> 16) & 0xff) / 127.f),\n"
					"\t\t\t((float)(int32_t)(int8_t)((v >> 24) & 0xff) / 127.f)\n"
				"\t\t), -1.f, 1.f\n"
			"\t);\n"
		"}\n"
		"\n"
	);

	print_section_header(
		"Matrix",
		""
	);

	static char* xyzw = "xyzw";
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
				snprintf(leftname, sizeof(leftname), "%sx%ux%u", data_type_suffixes[data_type], leftc, leftr);
				char rightname[128];
				snprintf(rightname, sizeof(rightname), "%sx%ux%u", data_type_suffixes[data_type], rightc, rightr);
				fprintf(ctx.f, "%s mul_%sx%ux%u_%sx%ux%u(%s a, %s b) {\n", leftname, data_type_suffixes[data_type], leftc, leftr, data_type_suffixes[data_type], rightc, rightr, leftname, rightname);
				fprintf(ctx.f, "\t%s m;\n", leftname);
				for (int c = 0; c < leftc; c += 1) {
					for (int r = 0; r < leftr; r += 1) {
						fprintf(ctx.f, "\tm.cols[%u].%c = ", c, xyzw[r]);
						for (int k = 0; k < leftc; k += 1) {
							fprintf(ctx.f, "a.cols[%u].%c * b.cols[%u].%c", k, xyzw[r], c, xyzw[k]);
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
			print_entry("$mx muls_$mx($mx m, $di s) {\n");
			for (int c = 0; c < ccount; c += 1) {
				for (int r = 0; r < rcount; r += 1) {
					fprintf(ctx.f, "\tm.cols[%u].%c *= s;\n", c, xyzw[r]);
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
				snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], leftc, leftr);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "%sx%u", data_type_suffixes[data_type], comp_count);
				fprintf(ctx.f, "%s mul_%sx%ux%u_%sx%u(%s m, %s v) {\n", vectorname, data_type_suffixes[data_type], leftc, leftr, data_type_suffixes[data_type], comp_count, matrixname, vectorname);
				fprintf(ctx.f, "\t%s ret;\n", vectorname);
				for (int r = 0; r < comp_count; r += 1) {
					fprintf(ctx.f, "\tret.%c = ", xyzw[r]);
					for (int c = 0; c < leftc; c += 1) {
						fprintf(ctx.f, "m.cols[%u].%c * v.%c", c, xyzw[r], xyzw[c]);
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
				snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], leftc, leftr);
				char vectorname[128];
				snprintf(vectorname, sizeof(vectorname), "%sx%u", data_type_suffixes[data_type], comp_count);
				fprintf(ctx.f, "%s mul_%sx%u_%sx%ux%u(%s v, %s m) {\n", vectorname, data_type_suffixes[data_type], comp_count, data_type_suffixes[data_type], leftc, leftr, vectorname, matrixname);
				fprintf(ctx.f, "\t%s ret;\n", vectorname);
				for (int r = 0; r < comp_count; r += 1) {
					fprintf(ctx.f, "\tret.%c = ", xyzw[r]);
					for (int c = 0; c < leftc; c += 1) {
						fprintf(ctx.f, "v.%c * m.cols[%u].%c", xyzw[c], r, xyzw[c]);
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
			snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], matrixc, matrixr);
			char retmatrixname[128];
			snprintf(retmatrixname, sizeof(retmatrixname), "%sx%ux%u", data_type_suffixes[data_type], matrixr, matrixc);
			fprintf(ctx.f, "%s transpose_%sx%ux%u(%s m) {\n", retmatrixname, data_type_suffixes[data_type], matrixc, matrixr, matrixname);
			fprintf(ctx.f, "\t%s ret;\n", retmatrixname);
			for (int c = 0; c < matrixc; c += 1) {
				for (int r = 0; r < matrixr; r += 1) {
					fprintf(ctx.f, "\tret.cols[%u].%c = m.cols[%u].%c;\n", r, xyzw[c], c, xyzw[r]);
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
				snprintf(matrixname, sizeof(matrixname), "%sx%ux%u", data_type_suffixes[data_type], leftcomp_count, rightcomp_count);
				char leftvectorname[128];
				snprintf(leftvectorname, sizeof(leftvectorname), "%sx%u", data_type_suffixes[data_type], leftcomp_count);
				char rightvectorname[128];
				snprintf(rightvectorname, sizeof(rightvectorname), "%sx%u", data_type_suffixes[data_type], rightcomp_count);
				fprintf(ctx.f, "%s outerproduct_%sx%u_%sx%u(%s c, %s r) {\n", matrixname, data_type_suffixes[data_type], leftcomp_count, data_type_suffixes[data_type], rightcomp_count, leftvectorname, rightvectorname);
				fprintf(ctx.f, "\t%s ret;\n", matrixname);
				for (int c = 0; c < leftcomp_count; c += 1) {
					for (int r = 0; r < rightcomp_count; r += 1) {
						fprintf(ctx.f, "\tret.cols[%u].%c = c.%c * r.%c;\n", c, xyzw[r], xyzw[c], xyzw[r]);
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
			"$di determinant_$mx($mx m) {\n"
			"\treturn m.cols[0].x * m.cols[1].y - m.cols[1].x * m.cols[0].y;\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_3x3;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di determinant_$mx($mx m) {\n");
		print_mat3x3_determinant();
		print_entry(
			"\treturn det;\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_4x4;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$di determinant_$mx($mx m) {\n");
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
			"$mx inverse_$mx($mx m) {\n"
			"\t$di inv_det = 1.f / determinant_$mx(m);\n"
			"\t$mx ret;\n"
			"\tret.cols[0].x = m.cols[1].y * inv_det;\n"
			"\tret.cols[0].y = -m.cols[0].y * inv_det;\n"
			"\t\n"
			"\tret.cols[1].x = -m.cols[1].x * inv_det;\n"
			"\tret.cols[1].y = m.cols[0].x * inv_det;\n"
			"\treturn ret;\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_3x3;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$mx inverse_$mx($mx m) {\n");
		print_mat3x3_determinant();
		print_entry(
			"\t$di inv_det = 1.f / det;\n"
			"\t$mx ret;\n"
			"\tret.cols[0].x = s[0] * inv_det;\n"
			"\tret.cols[0].y = (m.cols[0].z * m.cols[2].y - m.cols[0].y * m.cols[2].z) * inv_det;\n"
			"\tret.cols[0].z = (m.cols[0].y * m.cols[1].z - m.cols[0].z * m.cols[1].y) * inv_det;\n"
			"\t\n"
			"\tret.cols[1].x = -s[1] * inv_det;\n"
			"\tret.cols[1].y = (m.cols[0].x * m.cols[2].z - m.cols[0].z * m.cols[2].x) * inv_det;\n"
			"\tret.cols[1].z = (m.cols[1].x * m.cols[0].z - m.cols[0].x * m.cols[1].z) * inv_det;\n"
			"\t\n"
			"\tret.cols[2].x = s[2] * inv_det;\n"
			"\tret.cols[2].y = (m.cols[2].x * m.cols[0].y - m.cols[0].x * m.cols[2].y) * inv_det;\n"
			"\tret.cols[2].z = (m.cols[0].x * m.cols[1].y - m.cols[1].x * m.cols[0].y) * inv_det;\n"
			"\treturn ret;\n"
			"}\n"
		);
	}
	ctx.matrix = MATRIX_4x4;
	for (DataType data_type = DATA_TYPE_FLOAT; data_type <= DATA_TYPE_DOUBLE; data_type += 1) {
		ctx.data_type = data_type;
		print_entry("$mx inverse_$mx($mx m) {\n");
		print_mat4x4_determinant();
		print_entry(
			"\t$di inv_det = 1.f / det;\n"
			"\t$mx ret;\n"
			"\tret.cols[0].x = ( m.cols[1].y * c[5] - m.cols[1].z * c[4] + m.cols[1].w * c[3]) * inv_det;\n"
			"\tret.cols[0].y = (-m.cols[0].y * c[5] + m.cols[0].z * c[4] - m.cols[0].w * c[3]) * inv_det;\n"
			"\tret.cols[0].z = ( m.cols[3].y * s[5] - m.cols[3].z * s[4] + m.cols[3].w * s[3]) * inv_det;\n"
			"\tret.cols[0].w = (-m.cols[2].y * s[5] + m.cols[2].z * s[4] - m.cols[2].w * s[3]) * inv_det;\n"
			"\t\n"
			"\tret.cols[1].x = (-m.cols[1].x * c[5] + m.cols[1].z * c[2] - m.cols[1].w * c[1]) * inv_det;\n"
			"\tret.cols[1].y = ( m.cols[0].x * c[5] - m.cols[0].z * c[2] + m.cols[0].w * c[1]) * inv_det;\n"
			"\tret.cols[1].z = (-m.cols[3].x * s[5] + m.cols[3].z * s[2] - m.cols[3].w * s[1]) * inv_det;\n"
			"\tret.cols[1].w = ( m.cols[2].x * s[5] - m.cols[2].z * s[2] + m.cols[2].w * s[1]) * inv_det;\n"
			"\t\n"
			"\tret.cols[2].x = ( m.cols[1].x * c[4] - m.cols[1].y * c[2] + m.cols[1].w * c[0]) * inv_det;\n"
			"\tret.cols[2].y = (-m.cols[0].x * c[4] + m.cols[0].y * c[2] - m.cols[0].w * c[0]) * inv_det;\n"
			"\tret.cols[2].z = ( m.cols[3].x * s[4] - m.cols[3].y * s[2] + m.cols[3].w * s[0]) * inv_det;\n"
			"\tret.cols[2].w = (-m.cols[2].x * s[4] + m.cols[2].y * s[2] - m.cols[2].w * s[0]) * inv_det;\n"
			"\t\n"
			"\tret.cols[3].x = (-m.cols[1].x * c[3] + m.cols[1].y * c[1] - m.cols[1].z * c[0]) * inv_det;\n"
			"\tret.cols[3].y = ( m.cols[0].x * c[3] - m.cols[0].y * c[1] + m.cols[0].z * c[0]) * inv_det;\n"
			"\tret.cols[3].z = (-m.cols[3].x * s[3] + m.cols[3].y * s[1] - m.cols[3].z * s[0]) * inv_det;\n"
			"\tret.cols[3].w = ( m.cols[2].x * s[3] - m.cols[2].y * s[1] + m.cols[2].z * s[0]) * inv_det;\n"
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
