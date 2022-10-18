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

void hcc_ast_variable_to_string(HccCU* cu, HccASTVariable* variable, HccIIO* iio, bool color) {
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

void hcc_ast_function_to_string(HccCU* cu, HccASTFunction* function, HccIIO* iio, bool color) {
	char* function_fmt;
	if (color) {
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
		hcc_ast_variable_to_string(cu, param, iio, color);
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

void hcc_ast_file_init(HccASTFile* file, HccASTFileSetup* setup, HccString path) {
	file->path = path;
	file->macros = hcc_stack_init(HccPPMacro, HCC_ALLOC_TAG_AST_FILE_MACROS, setup->macros_grow_count, setup->macros_reserve_cap);
	file->macro_params = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_AST_FILE_MACRO_PARAMS, setup->macro_params_grow_count, setup->macro_params_reserve_cap);
	file->pragma_onced_files = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_AST_FILE_PRAGMA_ONCED_FILES, setup->unique_include_files_grow_count, setup->unique_include_files_reserve_cap);
	file->unique_included_files = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_AST_FILE_UNIQUE_INCLUDED_FILES, setup->unique_include_files_grow_count, setup->unique_include_files_reserve_cap);
	hcc_ata_token_bag_init(&file->token_bag, setup->tokens_grow_count, setup->tokens_reserve_cap, setup->values_grow_count, setup->values_reserve_cap);
	hcc_ata_token_bag_init(&file->macro_token_bag, setup->tokens_grow_count, setup->tokens_reserve_cap, setup->values_grow_count, setup->values_reserve_cap);
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
	cu->ast.files_hash_table = hcc_hash_table_init(HccASTFileEntry, HCC_ALLOC_TAG_AST_FILES_HASH_TABLE, hcc_string_key_cmp, setup->files_cap);
	cu->ast.function_params_and_variables = hcc_stack_init(HccASTVariable, HCC_ALLOC_TAG_AST_FUNCTION_PARAMS_AND_VARIABLES, setup->function_params_and_variables_grow_count, setup->function_params_and_variables_reserve_cap);
	cu->ast.functions = hcc_stack_init(HccASTFunction, HCC_ALLOC_TAG_AST_FUNCTIONS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->ast.exprs = hcc_stack_init(HccASTExpr, HCC_ALLOC_TAG_AST_EXPRS, setup->exprs_grow_count, setup->exprs_reserve_cap);
	cu->ast.global_variables = hcc_stack_init(HccASTVariable, HCC_ALLOC_TAG_AST_GLOBAL_VARIBALES, setup->global_variables_grow_count, setup->global_variables_reserve_cap);
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
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx_string(cu->ast.files_hash_table, &file_path);
	HCC_ASSERT(insert.is_new, "AST File '%s' has already been added to this AST", file_path);

	*out = &cu->ast.files_hash_table[insert.idx].file;
	hcc_ast_file_init(*out, &cu->ast.file_setup, file_path);
}

HccASTFile* hcc_ast_find_file(HccCU* cu, HccString file_path) {
	uintptr_t found_idx = hcc_hash_table_find_idx_string(cu->ast.files_hash_table, &file_path);
	if (found_idx == UINTPTR_MAX) {
		return NULL;
	}

	return &cu->ast.files_hash_table[found_idx].file;
}

void hcc_ast_print_section_header(const char* name, const char* path, HccIIO* iio) {
	char* fmt;
	if (iio->ascii_colors_enabled) {
		fmt =
			"\x1b[1;91m"
			"\n"
			"// ===========================================\n"
			"//\n"
			"// %s: %s\n"
			"//\n"
			"// ===========================================\n"
			"\n"
			"\x1b[0m";
	} else {
		fmt =
			"\n"
			"// ===========================================\n"
			"//\n"
			"// %s: %s\n"
			"//\n"
			"// ===========================================\n"
			"\n";
	}
	hcc_iio_write_fmt(iio, fmt, name, path);
}

void hcc_ast_print(HccCU* cu, HccIIO* iio) {
	for (uint32_t file_idx = 0; file_idx < hcc_hash_table_cap(cu->ast.files_hash_table); file_idx += 1) {
		HccASTFileEntry* entry = &cu->ast.files_hash_table[file_idx];
		if (entry->path.data == NULL) {
			continue;
		}
		HccASTFile* file = &entry->file;
		hcc_ast_print_section_header("Tokens", file->path.data, iio);

		HccATAIter* iter = hcc_ata_iter_start(file);
		HccATAToken token;
		uint32_t line = 1;
		uint32_t last_printed_code_end_idx = 0;
		bool is_first = true;
		HccCodeFile* code_file = NULL;
		const char* file_path_preview;
		while ((token = hcc_ata_iter_next(iter)) != HCC_ATA_TOKEN_EOF) {
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
		}

		hcc_ast_print_section_header("Macros", file->path.data, iio);

		uint32_t file_macros_count;
		HccPPMacro* file_macros = hcc_ast_file_get_macros(file, &file_macros_count);
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

	if (iio->ascii_colors_enabled) {
		hcc_iio_write_fmt(iio, "\x1b[0m");
	}
	hcc_iio_write_fmt(iio, "\n");
}

