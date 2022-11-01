#include "hcc_internal.h"

// ===========================================
//
//
// AST Basic Data Type
//
//
// ===========================================

uint8_t hcc_ast_data_type_basic_type_ranks[HCC_AST_BASIC_DATA_TYPE_COUNT] = {
	[HCC_AST_BASIC_DATA_TYPE_BOOL] = 1,
	[HCC_AST_BASIC_DATA_TYPE_CHAR] = 2,
	[HCC_AST_BASIC_DATA_TYPE_SCHAR] = 2,
	[HCC_AST_BASIC_DATA_TYPE_UCHAR] = 2,
	[HCC_AST_BASIC_DATA_TYPE_SSHORT] = 3,
	[HCC_AST_BASIC_DATA_TYPE_USHORT] = 3,
	[HCC_AST_BASIC_DATA_TYPE_SINT] = 4,
	[HCC_AST_BASIC_DATA_TYPE_UINT] = 4,
	[HCC_AST_BASIC_DATA_TYPE_SLONG] = 5,
	[HCC_AST_BASIC_DATA_TYPE_ULONG] = 5,
	[HCC_AST_BASIC_DATA_TYPE_SLONGLONG] = 6,
	[HCC_AST_BASIC_DATA_TYPE_ULONGLONG] = 6,
	[HCC_AST_BASIC_DATA_TYPE_FLOAT] = 7,
	[HCC_AST_BASIC_DATA_TYPE_DOUBLE] = 8,
};

const char* hcc_ast_data_type_basic_type_strings[HCC_AST_BASIC_DATA_TYPE_COUNT] = {
	[HCC_AST_BASIC_DATA_TYPE_VOID] = "void",
	[HCC_AST_BASIC_DATA_TYPE_BOOL] = "bool",
	[HCC_AST_BASIC_DATA_TYPE_CHAR] = "char",
	[HCC_AST_BASIC_DATA_TYPE_SCHAR] = "signed char",
	[HCC_AST_BASIC_DATA_TYPE_UCHAR] = "unsigned char",
	[HCC_AST_BASIC_DATA_TYPE_SSHORT] = "signed short",
	[HCC_AST_BASIC_DATA_TYPE_USHORT] = "unsigned short",
	[HCC_AST_BASIC_DATA_TYPE_SINT] = "signed int",
	[HCC_AST_BASIC_DATA_TYPE_UINT] = "unsigned int",
	[HCC_AST_BASIC_DATA_TYPE_SLONG] = "signed long",
	[HCC_AST_BASIC_DATA_TYPE_ULONG] = "unsigned long",
	[HCC_AST_BASIC_DATA_TYPE_SLONGLONG] = "signed long long",
	[HCC_AST_BASIC_DATA_TYPE_ULONGLONG] = "unsigned long long",
	[HCC_AST_BASIC_DATA_TYPE_FLOAT] = "float",
	[HCC_AST_BASIC_DATA_TYPE_DOUBLE] = "double",
};

uint8_t hcc_ast_basic_type_size_and_aligns_x86_64_linux[HCC_AST_BASIC_DATA_TYPE_COUNT] = {
	[HCC_AST_BASIC_DATA_TYPE_BOOL] = 1,
	[HCC_AST_BASIC_DATA_TYPE_CHAR] = 1,
	[HCC_AST_BASIC_DATA_TYPE_SCHAR] = 1,
	[HCC_AST_BASIC_DATA_TYPE_UCHAR] = 1,
	[HCC_AST_BASIC_DATA_TYPE_SSHORT] = 2,
	[HCC_AST_BASIC_DATA_TYPE_USHORT] = 2,
	[HCC_AST_BASIC_DATA_TYPE_SINT] = 4,
	[HCC_AST_BASIC_DATA_TYPE_UINT] = 4,
	[HCC_AST_BASIC_DATA_TYPE_SLONG] = 8,
	[HCC_AST_BASIC_DATA_TYPE_ULONG] = 8,
	[HCC_AST_BASIC_DATA_TYPE_SLONGLONG] = 8,
	[HCC_AST_BASIC_DATA_TYPE_ULONGLONG] = 8,
	[HCC_AST_BASIC_DATA_TYPE_FLOAT] = 4,
	[HCC_AST_BASIC_DATA_TYPE_DOUBLE] = 8,
};

uint64_t hcc_ast_basic_type_int_mins_x86_64_linux[HCC_AST_BASIC_DATA_TYPE_COUNT] = {
	[HCC_AST_BASIC_DATA_TYPE_CHAR] = INT8_MIN,
	[HCC_AST_BASIC_DATA_TYPE_SCHAR] = INT8_MIN,
	[HCC_AST_BASIC_DATA_TYPE_SSHORT] = INT16_MIN,
	[HCC_AST_BASIC_DATA_TYPE_SINT] = INT32_MIN,
	[HCC_AST_BASIC_DATA_TYPE_SLONG] = INT64_MIN,
	[HCC_AST_BASIC_DATA_TYPE_SLONGLONG] = INT64_MIN,
	[HCC_AST_BASIC_DATA_TYPE_UCHAR] = 0,
	[HCC_AST_BASIC_DATA_TYPE_USHORT] = 0,
	[HCC_AST_BASIC_DATA_TYPE_UINT] = 0,
	[HCC_AST_BASIC_DATA_TYPE_ULONG] = 0,
	[HCC_AST_BASIC_DATA_TYPE_ULONGLONG] = 0,
};

uint64_t hcc_ast_basic_type_int_maxes_x86_64_linux[HCC_AST_BASIC_DATA_TYPE_COUNT] = {
	[HCC_AST_BASIC_DATA_TYPE_CHAR] = INT8_MAX,
	[HCC_AST_BASIC_DATA_TYPE_SCHAR] = INT8_MAX,
	[HCC_AST_BASIC_DATA_TYPE_SSHORT] = INT16_MAX,
	[HCC_AST_BASIC_DATA_TYPE_SINT] = INT32_MAX,
	[HCC_AST_BASIC_DATA_TYPE_SLONG] = INT64_MAX,
	[HCC_AST_BASIC_DATA_TYPE_SLONGLONG] = INT64_MAX,
	[HCC_AST_BASIC_DATA_TYPE_UCHAR] = UINT8_MAX,
	[HCC_AST_BASIC_DATA_TYPE_USHORT] = UINT16_MAX,
	[HCC_AST_BASIC_DATA_TYPE_UINT] = UINT32_MAX,
	[HCC_AST_BASIC_DATA_TYPE_ULONG] = UINT64_MAX,
	[HCC_AST_BASIC_DATA_TYPE_ULONGLONG] = UINT64_MAX,
};

// ===========================================
//
//
// AST Expressions
//
//
// ===========================================

uint8_t hcc_ast_unary_op_precedence[HCC_AST_UNARY_OP_COUNT] = {
	[HCC_AST_UNARY_OP_BIT_NOT] = 2,
	[HCC_AST_UNARY_OP_LOGICAL_NOT] = 2,
	[HCC_AST_UNARY_OP_PLUS] = 2,
	[HCC_AST_UNARY_OP_NEGATE] = 2,
	[HCC_AST_UNARY_OP_PRE_INCREMENT] = 2,
	[HCC_AST_UNARY_OP_PRE_DECREMENT] = 2,
	[HCC_AST_UNARY_OP_POST_INCREMENT] = 1,
	[HCC_AST_UNARY_OP_POST_DECREMENT] = 1,
	[HCC_AST_UNARY_OP_DEREF] = 2,
	[HCC_AST_UNARY_OP_ADDRESS_OF] = 2,
};

// ===========================================
//
//
// AST Declarations
//
//
// ===========================================

const char* hcc_ast_function_shader_stage_strings[HCC_AST_FUNCTION_SHADER_STAGE_COUNT] = {
	[HCC_AST_FUNCTION_SHADER_STAGE_NONE] = "none",
	[HCC_AST_FUNCTION_SHADER_STAGE_VERTEX] = "vertex",
	[HCC_AST_FUNCTION_SHADER_STAGE_FRAGMENT] = "fragment",
	[HCC_AST_FUNCTION_SHADER_STAGE_COMPUTE] = "compute",
	[HCC_AST_FUNCTION_SHADER_STAGE_MESHTASK] = "meshtask",
};

const char* hcc_function_scalarx_strings[HCC_FUNCTION_SCALARX_COUNT] = {
	[HCC_FUNCTION_SCALARX_SWIZZLE] = "swizzle",
	[HCC_FUNCTION_SCALARX_PACK] = "pack",
	[HCC_FUNCTION_SCALARX_UNPACK] = "unpack",
	[HCC_FUNCTION_SCALARX_ANY] = "any",
	[HCC_FUNCTION_SCALARX_ALL] = "all",
	[HCC_FUNCTION_SCALARX_ADD] = "add",
	[HCC_FUNCTION_SCALARX_SUB] = "sub",
	[HCC_FUNCTION_SCALARX_MUL] = "mul",
	[HCC_FUNCTION_SCALARX_DIV] = "div",
	[HCC_FUNCTION_SCALARX_MOD] = "mod",
	[HCC_FUNCTION_SCALARX_EQ] = "eq",
	[HCC_FUNCTION_SCALARX_NEQ] = "neq",
	[HCC_FUNCTION_SCALARX_LT] = "lt",
	[HCC_FUNCTION_SCALARX_LTEQ] = "lteq",
	[HCC_FUNCTION_SCALARX_GT] = "gt",
	[HCC_FUNCTION_SCALARX_GTEQ] = "gteq",
	[HCC_FUNCTION_SCALARX_NOT] = "not",
	[HCC_FUNCTION_SCALARX_NEG] = "neg",
	[HCC_FUNCTION_SCALARX_BITNOT] = "bitnot",
	[HCC_FUNCTION_SCALARX_MIN] = "min",
	[HCC_FUNCTION_SCALARX_MAX] = "max",
	[HCC_FUNCTION_SCALARX_CLAMP] = "clamp",
	[HCC_FUNCTION_SCALARX_SIGN] = "sign",
	[HCC_FUNCTION_SCALARX_ABS] = "abs",
	[HCC_FUNCTION_SCALARX_BITAND] = "bitand",
	[HCC_FUNCTION_SCALARX_BITOR] = "bitor",
	[HCC_FUNCTION_SCALARX_BITXOR] = "bitxor",
	[HCC_FUNCTION_SCALARX_BITSHL] = "bitshl",
	[HCC_FUNCTION_SCALARX_BITSHR] = "bitshr",
	[HCC_FUNCTION_SCALARX_FMA] = "fma",
	[HCC_FUNCTION_SCALARX_FLOOR] = "floor",
	[HCC_FUNCTION_SCALARX_CEIL] = "ceil",
	[HCC_FUNCTION_SCALARX_ROUND] = "round",
	[HCC_FUNCTION_SCALARX_TRUNC] = "trunc",
	[HCC_FUNCTION_SCALARX_FRACT] = "fract",
	[HCC_FUNCTION_SCALARX_RADIANS] = "radians",
	[HCC_FUNCTION_SCALARX_DEGREES] = "degrees",
	[HCC_FUNCTION_SCALARX_STEP] = "step",
	[HCC_FUNCTION_SCALARX_SMOOTHSTEP] = "smoothstep",
	[HCC_FUNCTION_SCALARX_BITSTO] = "bitsto",
	[HCC_FUNCTION_SCALARX_BITSFROM] = "bitsfrom",
	[HCC_FUNCTION_SCALARX_SIN] = "sin",
	[HCC_FUNCTION_SCALARX_COS] = "cos",
	[HCC_FUNCTION_SCALARX_TAN] = "tan",
	[HCC_FUNCTION_SCALARX_ASIN] = "asin",
	[HCC_FUNCTION_SCALARX_ACOS] = "acos",
	[HCC_FUNCTION_SCALARX_ATAN] = "atan",
	[HCC_FUNCTION_SCALARX_SINH] = "sinh",
	[HCC_FUNCTION_SCALARX_COSH] = "cosh",
	[HCC_FUNCTION_SCALARX_TANH] = "tanh",
	[HCC_FUNCTION_SCALARX_ASINH] = "asinh",
	[HCC_FUNCTION_SCALARX_ACOSH] = "acosh",
	[HCC_FUNCTION_SCALARX_ATANH] = "atanh",
	[HCC_FUNCTION_SCALARX_ATAN2] = "atan2",
	[HCC_FUNCTION_SCALARX_POW] = "pow",
	[HCC_FUNCTION_SCALARX_EXP] = "exp",
	[HCC_FUNCTION_SCALARX_LOG] = "log",
	[HCC_FUNCTION_SCALARX_EXP2] = "exp2",
	[HCC_FUNCTION_SCALARX_LOG2] = "log2",
	[HCC_FUNCTION_SCALARX_SQRT] = "sqrt",
	[HCC_FUNCTION_SCALARX_RSQRT] = "rsqrt",
	[HCC_FUNCTION_SCALARX_ISINF] = "isinf",
	[HCC_FUNCTION_SCALARX_ISNAN] = "isnan",
	[HCC_FUNCTION_SCALARX_LERP] = "lerp",
	[HCC_FUNCTION_SCALARX_DOT] = "dot",
	[HCC_FUNCTION_SCALARX_LEN] = "len",
	[HCC_FUNCTION_SCALARX_NORM] = "norm",
	[HCC_FUNCTION_SCALARX_REFLECT] = "reflect",
	[HCC_FUNCTION_SCALARX_REFRACT] = "refract",
	[HCC_FUNCTION_SCALARX_MATRIX_MUL_SCALAR] = "muls",
	[HCC_FUNCTION_SCALARX_MATRIX_MUL_VECTOR] = "mulv",
	[HCC_FUNCTION_SCALARX_MATRIX_TRANSPOSE] = "transpose",
	[HCC_FUNCTION_SCALARX_MATRIX_OUTER_PRODUCT] = "outer_product",
	[HCC_FUNCTION_SCALARX_MATRIX_DETERMINANT] = "determinant",
	[HCC_FUNCTION_SCALARX_MATRIX_INVERSE] = "inverse",
};

const char* hcc_intrinsic_function_specific_strings[HCC_FUNCTION_IDX_SPECIFIC_END] = {
	[HCC_FUNCTION_IDX_F16TOF32] = "f16tof32",
	[HCC_FUNCTION_IDX_F16TOF64] = "f16tof64",
	[HCC_FUNCTION_IDX_F32TOF16] = "f32tof16",
	[HCC_FUNCTION_IDX_F64TOF16] = "f64tof16",
	[HCC_FUNCTION_IDX_PACKF16X2F32X2] = "packf16x2f32x2",
	[HCC_FUNCTION_IDX_UNPACKF16X2F32X2] = "unpackf16x2f32x2",
	[HCC_FUNCTION_IDX_PACKU16X2F32X2] = "packu16x2f32x2",
	[HCC_FUNCTION_IDX_UNPACKU16X2F32X2] = "unpacku16x2f32x2",
	[HCC_FUNCTION_IDX_PACKS16X2F32X2] = "packs16x2f32x2",
	[HCC_FUNCTION_IDX_UNPACKS16X2F32X2] = "unpacks16x2f32x2",
	[HCC_FUNCTION_IDX_PACKU8X4F32X4] = "packu8x4f32x4",
	[HCC_FUNCTION_IDX_UNPACKU8X4F32X4] = "unpacku8x4f32x4",
	[HCC_FUNCTION_IDX_PACKS8X4F32X4] = "packs8x4f32x4",
	[HCC_FUNCTION_IDX_UNPACKS8X4F32X4] = "unpacks8x4f32x4",
};

HccASTVariable* hcc_ast_global_variable_get(HccCU* cu, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_GLOBAL_VARIABLE(decl), "internal error: expected a global variable");
	return hcc_stack_get(cu->ast.global_variables, HCC_DECL_AUX(decl));
}

HccLocation* hcc_ast_variable_identifier_location(HccASTVariable* variable) {
	return variable->identifier_location;
}

HccStringId hcc_ast_variable_identifier_string_id(HccASTVariable* variable) {
	return variable->identifier_string_id;
}

HccDataType hcc_ast_variable_data_type(HccASTVariable* variable) {
	return variable->data_type;
}

HccConstantId hcc_ast_variable_initializer_constant_id(HccASTVariable* variable) {
	return variable->initializer_constant_id;
}

void hcc_ast_variable_to_string(HccCU* cu, HccASTVariable* variable, HccIIO* iio) {
	char* fmt;
	if (iio->ascii_colors_enabled) {
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
	HccString type_name = hcc_data_type_string(cu, variable->data_type);
	HccString variable_name = hcc_string_table_get(variable->identifier_string_id);
	hcc_iio_write_fmt(iio, fmt, specifiers, (int)type_name.size, type_name.data, (int)variable_name.size, variable_name.data);
}

HccASTFunction* hcc_ast_function_get(HccCU* cu, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_FUNCTION(decl), "internal error: expected a function declaration");
	return hcc_stack_get(cu->ast.functions, HCC_DECL_AUX(decl));
}

HccASTFunctionShaderStage hcc_ast_function_shader_stage(HccASTFunction* function) {
	return function->shader_stage;
}

bool hcc_ast_function_is_inline(HccASTFunction* function) {
	return function->flags & HCC_AST_FUNCTION_FLAGS_INLINE;
}

bool hcc_ast_function_is_static(HccASTFunction* function) {
	return function->flags & HCC_AST_FUNCTION_FLAGS_STATIC;
}

bool hcc_ast_function_is_extern(HccASTFunction* function) {
	return !(function->flags & HCC_AST_FUNCTION_FLAGS_STATIC);
}

HccLocation* hcc_ast_function_identifier_location(HccASTFunction* function) {
	return function->identifier_location;
}

HccStringId hcc_ast_function_identifier_string_id(HccASTFunction* function) {
	return function->identifier_string_id;
}

HccDataType hcc_ast_function_return_data_type(HccASTFunction* function) {
	return function->return_data_type;
}

HccLocation* hcc_ast_function_return_data_type_location(HccASTFunction* function) {
	return function->return_data_type_location;
}

uint8_t hcc_ast_function_params_count(HccASTFunction* function) {
	return function->params_count;
}

uint16_t hcc_ast_function_variables_count(HccASTFunction* function) {
	return function->variables_count;
}

HccASTVariable* hcc_ast_function_params_and_variables(HccASTFunction* function) {
	return function->params_and_variables;
}

void hcc_ast_function_to_string(HccCU* cu, HccASTFunction* function, HccIIO* iio) {
	char* function_fmt;
	if (iio->ascii_colors_enabled) {
		function_fmt = "\x1b[1;94m%.*s \x1b[97m%.*s\x1b[0m";
	} else {
		function_fmt = "%.*s %.*s";
	}

	HccString return_type_name = hcc_data_type_string(cu, function->return_data_type);
	HccString name = hcc_string_table_get(function->identifier_string_id);
	hcc_iio_write_fmt(iio, function_fmt, (int)return_type_name.size, return_type_name.data, (int)name.size, name.data);
	hcc_iio_write_fmt(iio, "(");
	for (uint32_t param_idx = 0; param_idx < function->params_count; param_idx += 1) {
		HccASTVariable* param = &function->params_and_variables[param_idx];
		hcc_ast_variable_to_string(cu, param, iio);
		if (param_idx + 1 < function->params_count) {
			hcc_iio_write_fmt(iio, ", ");
		}
	}
	hcc_iio_write_fmt(iio, ")");
}

// ===========================================
//
//
// AST File
//
//
// ===========================================

HccString hcc_ast_file_get_path(HccASTFile* file) {
	return file->path;
}

HccPPMacro* hcc_ast_file_get_macros(HccASTFile* file, uint32_t* macros_count_out) {
	*macros_count_out = hcc_stack_count(file->macros);
	return file->macros;
}

HccATAIter* hcc_ast_file_get_ata_iter(HccASTFile* file) {
	return hcc_ata_iter_start(file);
}

HccStringId* hcc_ast_file_unique_included_files(HccASTFile* file, uint32_t* count_out) {
	*count_out = hcc_stack_count(&file->unique_included_files);
	return file->unique_included_files;
}

void hcc_ast_file_reset(HccASTFile* file) {
	hcc_stack_clear(file->macros);
	hcc_stack_clear(file->macro_params);
	hcc_ata_token_bag_reset(&file->token_bag);
}

HccResult hcc_ast_file_push_token(HccASTFile* file, HccATAToken token, HccLocation* location) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	hcc_ata_token_bag_push_token(&file->token_bag, token, location);

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_ast_file_push_value(HccASTFile* file, HccATAValue value) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	hcc_ata_token_bag_push_value(&file->token_bag, value);

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

bool hcc_ast_file_pop_token(HccASTFile* file) {
	return hcc_ata_token_bag_pop_token(&file->token_bag);
}

void hcc_ast_file_init(HccASTFile* file, HccCU* cu, HccASTFileSetup* setup, HccString path) {
	file->path = path;
	file->macros = hcc_stack_init(HccPPMacro, HCC_ALLOC_TAG_AST_FILE_MACROS, setup->macros_grow_count, setup->macros_reserve_cap);
	file->macro_params = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_AST_FILE_MACRO_PARAMS, setup->macro_params_grow_count, setup->macro_params_reserve_cap);
	file->pragma_onced_files = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_AST_FILE_PRAGMA_ONCED_FILES, setup->unique_include_files_grow_count, setup->unique_include_files_reserve_cap);
	file->unique_included_files = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_AST_FILE_UNIQUE_INCLUDED_FILES, setup->unique_include_files_grow_count, setup->unique_include_files_reserve_cap);
	hcc_ata_token_bag_init(&file->token_bag, setup->tokens_grow_count, setup->tokens_reserve_cap, setup->values_grow_count, setup->values_reserve_cap);
	hcc_ata_token_bag_init(&file->macro_token_bag, setup->tokens_grow_count, setup->tokens_reserve_cap, setup->values_grow_count, setup->values_reserve_cap);

	file->global_declarations = hcc_hash_table_init(HccDeclEntry, HCC_ALLOC_TAG_AST_FILE_GLOBAL_DECLARATIONS, hcc_u32_key_cmp, hcc_u32_key_hash, cu->global_declarations_cap);
	file->struct_declarations = hcc_hash_table_init(HccDeclEntry, HCC_ALLOC_TAG_AST_FILE_STRUCT_DECLARATIONS, hcc_u32_key_cmp, hcc_u32_key_hash, cu->compounds_cap);
	file->union_declarations = hcc_hash_table_init(HccDeclEntry, HCC_ALLOC_TAG_AST_FILE_UNION_DECLARATIONS, hcc_u32_key_cmp, hcc_u32_key_hash, cu->compounds_cap);
	file->enum_declarations = hcc_hash_table_init(HccDeclEntry, HCC_ALLOC_TAG_AST_FILE_ENUM_DECLARATIONS, hcc_u32_key_cmp, hcc_u32_key_hash, cu->enums_cap);
}

void hcc_ast_file_deinit(HccASTFile* file) {
	hcc_stack_deinit(file->macros);
	hcc_stack_deinit(file->macro_params);
	hcc_ata_token_bag_deinit(&file->token_bag);
	hcc_ata_token_bag_deinit(&file->macro_token_bag);
}

bool hcc_ast_file_has_been_pragma_onced(HccASTFile* file, HccStringId path_string_id) {
	for (uint32_t idx = 0; idx < hcc_stack_count(file->pragma_onced_files); idx += 1) {
		if (file->pragma_onced_files[idx].idx_plus_one == path_string_id.idx_plus_one) {
			return true;
		}
	}

	return false;
}

void hcc_ast_file_set_pragma_onced(HccASTFile* file, HccStringId path_string_id) {
	*hcc_stack_push(file->pragma_onced_files) = path_string_id;
}

void hcc_ast_file_found_included_file(HccASTFile* file, HccStringId path_string_id) {
	for (uint32_t idx = 0; idx < hcc_stack_count(file->unique_included_files); idx += 1) {
		if (file->unique_included_files[idx].idx_plus_one == path_string_id.idx_plus_one) {
			return;
		}
	}

	*hcc_stack_push(file->unique_included_files) = path_string_id;
}

// ===========================================
//
//
// AST - Abstract Syntax Tree
//
//
// ===========================================

void hcc_ast_init(HccCU* cu, HccASTSetup* setup) {
	cu->ast.file_setup = setup->file_setup;
	cu->ast.files_hash_table = hcc_hash_table_init(HccASTFileEntry, HCC_ALLOC_TAG_AST_FILES_HASH_TABLE, hcc_string_key_cmp, hcc_string_key_hash, setup->files_cap);
	cu->ast.function_params_and_variables = hcc_stack_init(HccASTVariable, HCC_ALLOC_TAG_AST_FUNCTION_PARAMS_AND_VARIABLES, setup->function_params_and_variables_grow_count, setup->function_params_and_variables_reserve_cap);
	cu->ast.functions = hcc_stack_init(HccASTFunction, HCC_ALLOC_TAG_AST_FUNCTIONS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->ast.exprs = hcc_stack_init(HccASTExpr, HCC_ALLOC_TAG_AST_EXPRS, setup->exprs_grow_count, setup->exprs_reserve_cap);
	cu->ast.global_variables = hcc_stack_init(HccASTVariable, HCC_ALLOC_TAG_AST_GLOBAL_VARIBALES, setup->global_variables_grow_count, setup->global_variables_reserve_cap);
	cu->ast.unsupported_intrinsic_type_used = hcc_stack_init(HccASTUnsupportedIntrinsicTypeUsed, HCC_ALLOC_TAG_AST_UNSUPPORTED_INTRINSICTYPE_USED, setup->functions_grow_count, setup->functions_reserve_cap);
}

void hcc_ast_deinit(HccCU* cu) {
	for (uint32_t idx = 0; idx < hcc_hash_table_cap(cu->ast.files_hash_table); idx += 1) {
		HccASTFileEntry* entry = &cu->ast.files_hash_table[idx];
		if (entry->path.data) {
			hcc_ast_file_deinit(&entry->file);
		}
	}
	hcc_hash_table_deinit(cu->ast.files_hash_table);
	hcc_stack_deinit(cu->ast.function_params_and_variables);
	hcc_stack_deinit(cu->ast.functions);
	hcc_stack_deinit(cu->ast.exprs);
	hcc_stack_deinit(cu->ast.global_variables);
}

void hcc_ast_add_file(HccCU* cu, HccString file_path, HccASTFile** out) {
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->ast.files_hash_table, &file_path);
	HCC_ASSERT(insert.is_new, "AST File '%s' has already been added to this AST", file_path);

	*out = &cu->ast.files_hash_table[insert.idx].file;
	hcc_ast_file_init(*out, cu, &cu->ast.file_setup, file_path);
}

HccASTFile* hcc_ast_find_file(HccCU* cu, HccString file_path) {
	uintptr_t found_idx = hcc_hash_table_find_idx(cu->ast.files_hash_table, &file_path);
	if (found_idx == UINTPTR_MAX) {
		return NULL;
	}

	return &cu->ast.files_hash_table[found_idx].file;
}

void hcc_ast_print_section_header(const char* name, const char* path, HccIIO* iio) {
	if (iio->ascii_colors_enabled) {
		hcc_iio_write_fmt(iio, "\x1b[1;91m");
	}

	if (path) {
		const char* fmt =
			"\n"
			"// ===========================================\n"
			"//\n"
			"// %s: %s\n"
			"//\n"
			"// ===========================================\n"
			"\n";
		hcc_iio_write_fmt(iio, fmt, name, path);
	} else {
		const char* fmt =
			"\n"
			"// ===========================================\n"
			"//\n"
			"// %s\n"
			"//\n"
			"// ===========================================\n"
			"\n";
		hcc_iio_write_fmt(iio, fmt, name);
	}

	if (iio->ascii_colors_enabled) {
		hcc_iio_write_fmt(iio, "\x1b[0m");
	}

}

#if 0
void hcc_astgen_print_expr(HccWorker* w, HccASTExpr* expr, uint32_t indent, FILE* f) {
	static char* indent_chars = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	fprintf(f, "%.*s", indent, indent_chars);
	if (!expr->is_stmt_block_entry) {
		HccString data_type_name = hcc_data_type_string(w->cu, expr->data_type);
		fprintf(f, "(%.*s)", (int)data_type_name.size, data_type_name.data);
	}

	const char* expr_name;
	switch (expr->type) {
		case HCC_EXPR_TYPE_CONSTANT: expr_name = "EXPR_CONSTANT"; goto CONSTANT;
		case HCC_EXPR_TYPE_STMT_CASE: expr_name = "STMT_CASE"; goto CONSTANT;
CONSTANT: {
			fprintf(f, "%s ", expr_name);
			HccConstantId constant_id = { .idx_plus_one = expr->constant.id };
			hcc_constant_print(w, constant_id, stdout);
			break;
		};
		case HCC_EXPR_TYPE_STMT_BLOCK: {
			uint32_t stmts_count = expr->stmt_block.stmts_count;
			fprintf(f, "STMT_BLOCK[%u] {\n", stmts_count);
			HccASTExpr* stmt = &expr[expr->stmt_block.first_expr_rel_idx];
			uint32_t variables_count = expr->stmt_block.variables_count;
			for (uint32_t i = 0; i < variables_count; i += 1) {
				char buf[1024] = "<CURLY_INITIALIZER_RESULT>";
				uint32_t variable_idx = w->astgen.print_variable_base_idx + i;
				HccASTVariable* variable = hcc_stack_get(w->astgen.function_params_and_variables, w->astgen.function->params_start_idx + variable_idx);
				if (variable->identifier_string_id.idx_plus_one) {
					hcc_variable_to_string(w, variable, buf, sizeof(buf), false);
				}
				fprintf(f, "%.*sLOCAL_VARIABLE(#%u): %s", indent + 1, indent_chars, variable_idx, buf);
				if (variable->initializer_constant_id.idx_plus_one) {
					fprintf(f, " = ");
					hcc_constant_print(w, variable->initializer_constant_id, f);
				}
				fprintf(f, "\n");
			}
			w->astgen.print_variable_base_idx += variables_count;

			for (uint32_t i = 0; i < stmts_count; i += 1) {
				hcc_astgen_print_expr(w, stmt, indent + 1, f);
				stmt = &stmt[stmt->next_expr_rel_idx];
			}
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_FUNCTION: {
			HccASTFunction* function = hcc_stack_get(w->astgen.functions, expr->function.idx);
			char buf[1024];
			hcc_function_to_string(w, function, buf, sizeof(buf), false);
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
			HccASTExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			hcc_astgen_print_expr(w, unary_expr, indent + 1, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_CAST: {
			fprintf(f, "EXPR_CAST: {\n");
			HccASTExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			hcc_astgen_print_expr(w, unary_expr, indent + 1, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_IF: {
			fprintf(f, "%s: {\n", "STMT_IF");

			HccASTExpr* cond_expr = &expr[expr->if_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, cond_expr, indent + 2, f);

			HccASTExpr* true_stmt = &expr[expr->if_.true_stmt_rel_idx];
			fprintf(f, "%.*sTRUE_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, true_stmt, indent + 2, f);

			if (true_stmt->if_aux.false_stmt_rel_idx) {
				HccASTExpr* false_stmt = &true_stmt[true_stmt->if_aux.false_stmt_rel_idx];
				fprintf(f, "%.*sFALSE_STMT:\n", indent + 1, indent_chars);
				hcc_astgen_print_expr(w, false_stmt, indent + 2, f);
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_SWITCH: {
			fprintf(f, "%s: {\n", "STMT_SWITCH");

			HccASTExpr* block_expr = &expr[expr->switch_.block_expr_rel_idx];
			hcc_astgen_print_expr(w, block_expr, indent + 1, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_WHILE: {
			fprintf(f, "%s: {\n", expr->while_.cond_expr_rel_idx > expr->while_.loop_stmt_rel_idx ? "STMT_DO_WHILE" : "STMT_WHILE");

			HccASTExpr* cond_expr = &expr[expr->while_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, cond_expr, indent + 2, f);

			HccASTExpr* loop_stmt = &expr[expr->while_.loop_stmt_rel_idx];
			fprintf(f, "%.*sLOOP_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, loop_stmt, indent + 2, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_STMT_FOR: {
			fprintf(f, "%s: {\n", "STMT_FOR");

			HccASTExpr* init_expr = &expr[expr->for_.init_expr_rel_idx];
			fprintf(f, "%.*sINIT_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, init_expr, indent + 2, f);

			HccASTExpr* cond_expr = &expr[expr->for_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, cond_expr, indent + 2, f);

			HccASTExpr* inc_expr = &expr[expr->for_.inc_expr_rel_idx];
			fprintf(f, "%.*sINCREMENT_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, inc_expr, indent + 2, f);

			HccASTExpr* loop_stmt = &expr[expr->for_.loop_stmt_rel_idx];
			fprintf(f, "%.*sLOOP_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, loop_stmt, indent + 2, f);

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
			HccASTExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HccASTExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			hcc_astgen_print_expr(w, left_expr, indent + 1, f);
			hcc_astgen_print_expr(w, right_expr, indent + 1, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_CURLY_INITIALIZER: {
			fprintf(f, "%s: {\n", "EXPR_CURLY_INITIALIZER");

			////////////////////////////////////////////////////////////////////////////
			// skip the internal variable expression that sits at the start of the initializer_expr list
			HccASTExpr* initializer_expr = &expr[expr->curly_initializer.first_expr_rel_idx];
			uint32_t expr_rel_idx;
			////////////////////////////////////////////////////////////////////////////

			while (1) {
				expr_rel_idx = initializer_expr->next_expr_rel_idx;
				if (expr_rel_idx == 0) {
					break;
				}
				initializer_expr = &initializer_expr[expr_rel_idx];

				HccAstGenDesignatorInitializer* di = &w->astgen.curly_initializer.designated_initializers[initializer_expr->alt_next_expr_rel_idx];
				uint64_t* elmt_indices = hcc_stack_get(w->astgen.curly_initializer.designated_initializer_elmt_indices, di->elmt_indices_start_idx);
				fprintf(f, "%.*s", indent + 1, indent_chars);
				HccDataType data_type = expr->data_type;
				if (di->elmt_indices_count) {
					for (uint32_t idx = 0; idx < di->elmt_indices_count; idx += 1) {
						data_type = hcc_typedef_resolve_and_strip_qualifiers(w->cu, data_type);
						uint64_t entry_idx = elmt_indices[idx];
						if (HCC_DATA_TYPE_IS_ARRAY(data_type)) {
							HccArrayDataType* array_data_type = hcc_array_data_type_get(w->cu, data_type);
							fprintf(f, "[%zu]", entry_idx);
							data_type = array_data_type->element_data_type;
						} else if (HCC_DATA_TYPE_IS_COMPOUND(data_type)) {
							HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(w->cu, data_type);
							HccCompoundField* field = &compound_data_type->fields[entry_idx];
							if (field->identifier_string_id.idx_plus_one) {
								HccString identifier_string = hcc_string_table_get(field->identifier_string_id);
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
					HccASTExpr* value_expr = initializer_expr - initializer_expr->designated_initializer.value_expr_rel_idx;
					if (value_expr->type == HCC_EXPR_TYPE_CURLY_INITIALIZER) {
						fprintf(f, "\n");
					}
					hcc_astgen_print_expr(w, value_expr, value_expr->type == HCC_EXPR_TYPE_CURLY_INITIALIZER ? indent + 2 : 0, f);
				} else {
					fprintf(f, "<ZERO>\n");
				}
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_FIELD_ACCESS: {
			fprintf(f, "%s: {\n", "EXPR_FIELD_ACCESS");

			HccASTExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			uint32_t field_idx = expr->binary.right_expr_rel_idx;
			hcc_astgen_print_expr(w, left_expr, indent + 1, f);

			HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(w->cu, left_expr->data_type);
			HccCompoundField* field = &compound_data_type->fields[field_idx];

			HccString field_data_type_name = hcc_data_type_string(w->cu, field->data_type);
			if (field->identifier_string_id.idx_plus_one) {
				HccString identifier_string = hcc_string_table_get(field->identifier_string_id);
				fprintf(f, "%.*sfield_idx(%u): %.*s %.*s\n", indent + 1, indent_chars, field_idx, (int)field_data_type_name.size, field_data_type_name.data, (int)identifier_string.size, identifier_string.data);
			} else {
				fprintf(f, "%.*sfield_idx(%u): %.*s\n", indent + 1, indent_chars, field_idx, (int)field_data_type_name.size, field_data_type_name.data);
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_CALL_ARG_LIST: {
			fprintf(f, "EXPR_CALL_ARG_LIST: {\n");
			HccASTExpr* arg_expr = expr;
			uint32_t args_count = ((U8*)expr)[1];
			U8* next_arg_expr_rel_indices = &((U8*)expr)[2];
			for (uint32_t i = 0; i < args_count; i += 1) {
				arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
				hcc_astgen_print_expr(w, arg_expr, indent + 1, f);
			}
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HCC_EXPR_TYPE_LOCAL_VARIABLE: {
			char buf[1024];
			HccASTVariable* variable = hcc_stack_get(w->astgen.function_params_and_variables, w->astgen.function->params_start_idx + expr->variable.idx);
			hcc_variable_to_string(w, variable, buf, sizeof(buf), false);
			fprintf(f, "LOCAL_VARIABLE(#%u): %s", expr->variable.idx, buf);
			break;
		};
		case HCC_EXPR_TYPE_GLOBAL_VARIABLE: {
			char buf[1024];
			HccASTVariable* variable = hcc_stack_get(w->astgen.global_variables, expr->variable.idx);
			hcc_variable_to_string(w, variable, buf, sizeof(buf), false);
			fprintf(f, "GLOBAL_VARIABLE(#%u): %s", expr->variable.idx, buf);
			break;
		};
		case HCC_EXPR_TYPE_BINARY_OP(TERNARY): {
			fprintf(f, "%s: {\n", "STMT_TERNARY");

			HccASTExpr* cond_expr = expr - expr->ternary.cond_expr_rel_idx;
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, cond_expr, indent + 2, f);

			HccASTExpr* true_stmt = expr - expr->ternary.true_expr_rel_idx;
			fprintf(f, "%.*sTRUE_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, true_stmt, indent + 2, f);

			HccASTExpr* false_stmt = expr - expr->ternary.false_expr_rel_idx;
			fprintf(f, "%.*sFALSE_STMT:\n", indent + 1, indent_chars);
			hcc_astgen_print_expr(w, false_stmt, indent + 2, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		default:
			HCC_ABORT("unhandled expr type %u\n", expr->type);
	}
	fprintf(f, "\n");
}
#endif

void hcc_ast_print(HccCU* cu, HccIIO* iio) {
	for (uint32_t file_idx = 0; file_idx < hcc_hash_table_cap(cu->ast.files_hash_table); file_idx += 1) {
		HccASTFileEntry* entry = &cu->ast.files_hash_table[file_idx];
		if (entry->path.data == NULL) {
			continue;
		}
		HccASTFile* file = &entry->file;
		hcc_ast_print_section_header("Tokens", file->path.data, iio);

		HccATAIter* iter = hcc_ata_iter_start(file);
		HccATAToken token = hcc_ata_iter_peek(iter);
		uint32_t line = 1;
		uint32_t last_printed_code_end_idx = 0;
		bool is_first = true;
		HccCodeFile* code_file = NULL;
		const char* file_path_preview;
		while (token != HCC_ATA_TOKEN_EOF) {
			HccLocation* location = hcc_ata_iter_location(iter);
			while (location->parent_location) {
				location = location->parent_location;
			}

			if (location->code_file != code_file) {
				code_file = location->code_file;
				line = location->line_start - 1;
				last_printed_code_end_idx = location->code_start_idx;

				{
					uint32_t idx = code_file->path_string.size;
					uint32_t remaining_forward_slashes = 2;
					while (idx) {
						if (code_file->path_string.data[idx] == '/') {
							remaining_forward_slashes -= 1;
							if (remaining_forward_slashes == 0) {
								idx += 1;
								break;
							}
						}

						idx -= 1;
					}
					file_path_preview = &code_file->path_string.data[idx];
				}
			}

			if (line < location->line_end) {
				if (!is_first) {
					hcc_iio_write_fmt(iio, "\n");
				}
				is_first = false;

				char* fmt;
				if (iio->ascii_colors_enabled) {
					fmt = "\x1b[1;96m[file: %s, line: %5u]:\x1b[0m %.*s\n";
				} else {
					fmt = "[file: %s, line: %5u]: %.*s\n";
				}

				//
				// skip all the whitespace before this token starts
				uint32_t idx = last_printed_code_end_idx;
				while (idx < location->code_start_idx) {
					idx += 1;
				}
				last_printed_code_end_idx = idx;

				//
				// measure the line
				while (idx < code_file->code.size) {
					if (code_file->code.data[idx] == '\n') {
						break;
					}
					idx += 1;
				}
				uint32_t size = idx - last_printed_code_end_idx;

				//
				// print the line
				hcc_iio_write_fmt(iio, fmt, file_path_preview, location->line_start, size, &code_file->code.data[last_printed_code_end_idx]);
				last_printed_code_end_idx = idx + 1; // + 1 to skip the \n
				line = location->line_end;
			}

			if (iio->ascii_colors_enabled) {
				hcc_iio_write_fmt(iio, "\x1b[1;97m");
			}

			HccATAValue value;
			HccString string;
			const char* suffix;
			switch (token) {
				case HCC_ATA_TOKEN_IDENT:
					value = hcc_ata_iter_next_value(iter);
					string = hcc_string_table_get(value.string_id);
					hcc_iio_write_fmt(iio, "%.*s\n", (int)string.size, string.data);
					break;
				case HCC_ATA_TOKEN_STRING:
					value = hcc_ata_iter_next_value(iter);
					string = hcc_string_table_get(value.string_id);
					hcc_iio_write_fmt(iio, "\"%.*s\"\n", (int)string.size, string.data);
					break;
				case HCC_ATA_TOKEN_LIT_SINT: suffix = ""; goto CONSTANT;
				case HCC_ATA_TOKEN_LIT_SLONG: suffix = "l"; goto CONSTANT;
				case HCC_ATA_TOKEN_LIT_SLONGLONG: suffix = "ll"; goto CONSTANT;
				case HCC_ATA_TOKEN_LIT_UINT: suffix = "u"; goto CONSTANT;
				case HCC_ATA_TOKEN_LIT_ULONG: suffix = "ul"; goto CONSTANT;
				case HCC_ATA_TOKEN_LIT_ULONGLONG: suffix = "ull"; goto CONSTANT;
				case HCC_ATA_TOKEN_LIT_FLOAT: suffix = "f"; goto CONSTANT;
				case HCC_ATA_TOKEN_LIT_DOUBLE: suffix = ""; goto CONSTANT;
CONSTANT: {}
					value = hcc_ata_iter_next_value(iter);
					hcc_constant_print(cu, value.constant_id, iio);
					hcc_iio_write_fmt(iio, "%s\n", suffix);
					hcc_ata_iter_next_value(iter); // skip the string id
					break;
				default:
					hcc_iio_write_fmt(iio, "%s\n", hcc_ata_token_strings[token]);
					break;
			}

			token = hcc_ata_iter_next(iter);
		}

		uint32_t file_macros_count;
		HccPPMacro* file_macros = hcc_ast_file_get_macros(file, &file_macros_count);
		if (file_macros_count) {
			hcc_ast_print_section_header("Macros", file->path.data, iio);
		}
		for (uint32_t macro_idx = 0; macro_idx < file_macros_count; macro_idx += 1) {
			HccPPMacro* macro = &file_macros[macro_idx];
			HccATATokenCursor cursor = macro->token_cursor;
			char buf[1024];
			HccStringId string_id = hcc_ata_token_bag_stringify_range(&file->macro_token_bag, &cursor, macro, buf, sizeof(buf), NULL);

			HccString string = hcc_string_table_get(string_id);
			char* fmt;
			if (iio->ascii_colors_enabled) {
				fmt = "\x1b[1;96m#define\x1b[0m %.*s";
			} else {
				fmt = "#define %.*s";
			}

			hcc_iio_write_fmt(iio, fmt, (int)macro->identifier_string.size, macro->identifier_string.data);
			char macro_param_names[1024];
			if (macro->is_function) {
				hcc_iio_write_fmt(iio, "(");
				for (uint32_t idx = 0; idx < macro->params_count; idx += 1) {
					HccStringId param_string_id = macro->params[idx];
					HccString param_string = hcc_string_table_get(param_string_id);
					hcc_iio_write_fmt(iio, "%.*s", (int)param_string.size, param_string.data);
					if (idx + 1 < macro->params_count) {
						hcc_iio_write_fmt(iio, ", ");
					}
				}
				hcc_iio_write_fmt(iio, ")");
			}
			hcc_iio_write_fmt(iio, " %.*s\n", (int)string.size, string.data);
		}
	}

	if (hcc_stack_count(cu->dtt.enums)) {
		hcc_ast_print_section_header("Enum Data Types", NULL, iio);

		for (uint32_t enum_type_idx = 0; enum_type_idx < hcc_stack_count(cu->dtt.enums); enum_type_idx += 1) {
			HccEnumDataType* d = hcc_stack_get(cu->dtt.enums, enum_type_idx);
			HccString name = hcc_string_lit("<anonymous>");
			if (d->identifier_string_id.idx_plus_one) {
				name = hcc_string_table_get(d->identifier_string_id);
			}
			hcc_iio_write_fmt(iio, "ENUM(#%u): %.*s {\n", enum_type_idx, (int)name.size, name.data);
			for (uint32_t value_idx = 0; value_idx < d->values_count; value_idx += 1) {
				HccEnumValue* value = &d->values[value_idx];
				HccString identifier = hcc_string_table_get_or_empty(value->identifier_string_id);

				HccConstant constant = hcc_constant_table_get(cu, value->constant_id);

				int64_t v;
				HCC_DEBUG_ASSERT(hcc_constant_as_sint(cu, constant, &v), "internal error: expected to be a signed int");
				hcc_iio_write_fmt(iio, "\t%.*s = %ld\n", (int)identifier.size, identifier.data, v);
			}
			hcc_iio_write_fmt(iio, "}\n");
		}
	}

	if (hcc_stack_count(cu->dtt.compounds)) {
		hcc_ast_print_section_header("Compound Data Types", NULL, iio);

		for (uint32_t compound_type_idx = 0; compound_type_idx < hcc_stack_count(cu->dtt.compounds); compound_type_idx += 1) {
			HccCompoundDataType* d = hcc_stack_get(cu->dtt.compounds, compound_type_idx);
			HccString name = hcc_string_lit("<anonymous>");
			if (d->identifier_string_id.idx_plus_one) {
				name = hcc_string_table_get(d->identifier_string_id);
			}
			char* compound_name = d->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION ? "UNION" : "STRUCT";
			hcc_iio_write_fmt(iio, "%s(#%u): %.*s {\n", compound_name, compound_type_idx, (int)name.size, name.data);
			hcc_iio_write_fmt(iio, "\tsize: %zu\n", d->size);
			hcc_iio_write_fmt(iio, "\talign: %zu\n", d->align);
			hcc_iio_write_fmt(iio, "\tfields: {\n");
			for (uint32_t field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
				HccCompoundField* field = &d->fields[field_idx];
				HccString data_type_name = hcc_data_type_string(cu, field->data_type);
				hcc_iio_write_fmt(iio, "\t\t%.*s ", (int)data_type_name.size, data_type_name.data);
				if (field->identifier_string_id.idx_plus_one) {
					HccString identifier = hcc_string_table_get_or_empty(field->identifier_string_id);
					hcc_iio_write_fmt(iio, "%.*s\n", (int)identifier.size, identifier.data);
				} else {
					hcc_iio_write_fmt(iio, "\n");
				}
			}
			hcc_iio_write_fmt(iio, "\t}\n");
			hcc_iio_write_fmt(iio, "}\n");
		}
	}

	if (hcc_stack_count(cu->dtt.arrays)) {
		hcc_ast_print_section_header("Array Data Types", NULL, iio);

		for (uint32_t array_type_idx = 0; array_type_idx < hcc_stack_count(cu->dtt.arrays); array_type_idx += 1) {
			HccArrayDataType* d = hcc_stack_get(cu->dtt.arrays, array_type_idx);
			HccString data_type_name = hcc_data_type_string(cu, d->element_data_type);

			HccConstant constant = hcc_constant_table_get(cu, d->element_count_constant_id);

			uint64_t count;
			HCC_DEBUG_ASSERT(hcc_constant_as_uint(cu, constant, &count), "internal error: expected to be a unsigned int");

			hcc_iio_write_fmt(iio, "ARRAY(#%u): %.*s[%zu]\n", array_type_idx, (int)data_type_name.size, data_type_name.data, count);
		}
	}

	if (hcc_stack_count(cu->dtt.typedefs)) {
		hcc_ast_print_section_header("Typedefs", NULL, iio);

		for (uint32_t typedefs_idx = 0; typedefs_idx < hcc_stack_count(cu->dtt.typedefs); typedefs_idx += 1) {
			HccTypedef* d = hcc_stack_get(cu->dtt.typedefs, typedefs_idx);
			HccString name = hcc_string_table_get_or_empty(d->identifier_string_id);
			HccString aliased_data_type_name = hcc_data_type_string(cu, d->aliased_data_type);
			hcc_iio_write_fmt(iio, "typedef(#%u) %.*s %.*s\n", typedefs_idx, (int)aliased_data_type_name.size, aliased_data_type_name.data, (int)name.size, name.data);
		}
	}

	if (hcc_stack_count(cu->ast.global_variables)) {
		hcc_ast_print_section_header("Global Variables", NULL, iio);

		for (uint32_t variable_idx = 0; variable_idx < hcc_stack_count(cu->ast.global_variables); variable_idx += 1) {
			HccASTVariable* variable = hcc_stack_get(cu->ast.global_variables, variable_idx);

			hcc_iio_write_fmt(iio, "GLOBAL_VARIABLE(#%u): ", variable_idx);
			hcc_ast_variable_to_string(cu, variable, iio);
			hcc_iio_write_fmt(iio, " = ");
			hcc_constant_print(cu, variable->initializer_constant_id, iio);
			hcc_iio_write_fmt(iio, "\n");
		}
	}

	if (hcc_stack_count(cu->ast.functions)) {
		hcc_ast_print_section_header("Functions", NULL, iio);

		for (uint32_t function_idx = 0; function_idx < hcc_stack_count(cu->ast.functions); function_idx += 1) {
			HccASTFunction* function = hcc_stack_get(cu->ast.functions, function_idx);
			if (function->identifier_string_id.idx_plus_one == 0) {
				continue;
			}
			HccString name = hcc_string_table_get_or_empty(function->identifier_string_id);
			HccString return_data_type_name = hcc_data_type_string(cu, function->return_data_type);
			hcc_iio_write_fmt(iio, "Function(#%u): %.*s {\n", function_idx, (int)name.size, name.data);
			hcc_iio_write_fmt(iio, "\treturn_type: %.*s\n", (int)return_data_type_name.size, return_data_type_name.data);
			hcc_iio_write_fmt(iio, "\tshader_stage: %s\n", hcc_ast_function_shader_stage_strings[function->shader_stage]);
			hcc_iio_write_fmt(iio, "\tstatic: %s\n", function->flags & HCC_AST_FUNCTION_FLAGS_STATIC ? "true" : "false");
			hcc_iio_write_fmt(iio, "\tinline: %s\n", function->flags & HCC_AST_FUNCTION_FLAGS_INLINE ? "true" : "false");
			if (function->params_count) {
				hcc_iio_write_fmt(iio, "\tparams[%u]: {\n", function->params_count);
				for (uint32_t param_idx = 0; param_idx < function->params_count; param_idx += 1) {
					HccASTVariable* param = &function->params_and_variables[param_idx];
					HccString type_name = hcc_data_type_string(cu, param->data_type);
					HccString param_name = hcc_string_table_get(param->identifier_string_id);
					hcc_iio_write_fmt(iio, "\t\t%.*s %.*s\n", (int)type_name.size, type_name.data, (int)param_name.size, param_name.data);
				}
				hcc_iio_write_fmt(iio, "\t}\n");
			}
			if (function->block_expr_id.idx_plus_one) {
#if 0
				HccASTExpr* expr = &w->astgen.exprs[function->block_expr_id.idx_plus_one - 1];
				hcc_astgen_print_expr(w, expr, 1, f);
#endif
			}
			hcc_iio_write_fmt(iio, "}\n");
		}
	}

	if (iio->ascii_colors_enabled) {
		hcc_iio_write_fmt(iio, "\x1b[0m");
	}

	hcc_iio_write_fmt(iio, "\n");
}

