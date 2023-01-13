#include "hcc_internal.h"

// ===========================================
//
//
// SPIR-V Generator
//
//
// ===========================================

void hcc_spirvgen_init(HccWorker* w, HccCompilerSetup* setup) {
	HCC_UNUSED(w);
	HCC_UNUSED(setup);
}

void hcc_spirvgen_deinit(HccWorker* w) {
	HCC_UNUSED(w);
}

void hcc_spirvgen_reset(HccWorker* w) {
	w->spirvgen.function_unique_globals_count = 0;
}

HccSPIRVId hcc_spirvgen_convert_operand(HccWorker* w, HccAMLOperand aml_operand) {
	HccSPIRVId spirv_id;
	switch (HCC_AML_OPERAND_TYPE(aml_operand)) {
		case HCC_AML_OPERAND_VALUE:
			spirv_id = w->spirvgen.value_base_id + HCC_AML_OPERAND_AUX(aml_operand);
			break;
		case HCC_AML_OPERAND_CONSTANT:
			spirv_id = hcc_spirv_constant_deduplicate(w->cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operand)));
			break;
		case HCC_AML_OPERAND_BASIC_BLOCK:
			spirv_id = w->spirvgen.basic_block_base_id + HCC_AML_OPERAND_AUX(aml_operand);
			break;
		case HCC_AML_OPERAND_BASIC_BLOCK_PARAM:
			spirv_id = w->spirvgen.basic_block_param_base_id + HCC_AML_OPERAND_AUX(aml_operand);
			break;
		case HCC_DECL_GLOBAL_VARIABLE:
			spirv_id = hcc_spirv_decl_deduplicate(w->cu, (HccDecl)aml_operand);
			hcc_spirvgen_found_global(w, spirv_id);
			break;
		case HCC_DECL_FUNCTION:
			spirv_id = hcc_spirv_decl_deduplicate(w->cu, (HccDecl)aml_operand);
			break;
		case HCC_DECL_ENUM_VALUE: {
			HccEnumValue* enum_value = hcc_enum_value_get(w->cu, (HccDecl)aml_operand);
			spirv_id = hcc_spirv_constant_deduplicate(w->cu, enum_value->constant_id);
			break;
		};
		case HCC_DECL_LOCAL_VARIABLE:
			HCC_ABORT("we shouldn't have access to local variables from the AST in the SPIR-V");
			break;
		default:
			spirv_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INVALID, (HccDataType)aml_operand);
			break;
	}
	return spirv_id;
}

void hcc_spirvgen_found_global(HccWorker* w, HccSPIRVId spirv_id) {
	for (uint32_t idx = 0; idx < w->spirvgen.function_unique_globals_count; idx += 1) {
		if (w->spirvgen.function_unique_globals[idx] == spirv_id) {
			return;
		}
	}

	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(w->spirvgen.function_unique_globals_count, HCC_FUNCTION_UNIQUE_GLOBALS_CAP);
	w->spirvgen.function_unique_globals[w->spirvgen.function_unique_globals_count] = spirv_id;
	w->spirvgen.function_unique_globals_count += 1;
}

void hcc_spirvgen_generate(HccWorker* w) {
	HccCU* cu = w->cu;
	HccDecl function_decl = (HccDecl)(uintptr_t)w->job.arg;
	HccASTFunction* ast_function = hcc_ast_function_get(cu, function_decl);
	const HccAMLFunction* aml_function = hcc_aml_function_get(cu, function_decl);
	HccSPIRVFunction* function = hcc_stack_get(cu->spirv.functions, HCC_DECL_AUX(function_decl));
	w->spirvgen.function = function;
	w->spirvgen.value_base_id = hcc_spirv_next_id_many(cu, aml_function->values_count);
	w->spirvgen.basic_block_base_id = hcc_spirv_next_id_many(cu, aml_function->basic_blocks_count);
	w->spirvgen.basic_block_param_base_id = hcc_spirv_next_id_many(cu, aml_function->basic_block_params_count);

	function->words_cap = (uint32_t)floorf((float)aml_function->words_count * 1.5f);
	function->words = hcc_stack_push_many(cu->spirv.function_words, function->words_cap);

	HccSPIRVId function_spirv_id = hcc_spirv_decl_deduplicate(cu, function_decl);

	HccDataType function_data_type = aml_function->function_data_type;
	if (aml_function->shader_stage != HCC_SHADER_STAGE_NONE) {
		function_data_type = hcc_function_data_type_deduplicate(cu, HCC_DATA_TYPE_AML_INTRINSIC_VOID, NULL, 0, 0);
	}

	HccSPIRVOperand* operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_FUNCTION, 4);
	operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, aml_function->return_data_type);
	operands[1] = function_spirv_id;
	operands[2] = HCC_SPIRV_FUNCTION_CTRL_NONE;
	operands[3] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, function_data_type);

	HccSPIRVId rasterizer_state_variable_start_spirv_id;
	HccSPIRVId fragment_state_variable_start_spirv_id;
	uint32_t rasterizer_state_variable_position_field_idx;
	switch (aml_function->shader_stage) {
		case HCC_SHADER_STAGE_NONE:
			//
			// create the function parameters
			for (uint32_t param_idx = 0; param_idx < aml_function->params_count; param_idx += 1) {
				HccAMLValue* param_value = &aml_function->values[param_idx];
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_FUNCTION_PARAMETER, 2);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_FUNCTION, param_value->data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, HCC_AML_OPERAND(VALUE, param_idx));
			}
			break;
		case HCC_SHADER_STAGE_VERTEX: {
			//
			// create the rasterizer state output variables
			HccDataType rasterizer_state_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, ast_function->params_and_variables[HCC_VERTEX_SHADER_PARAM_RASTERIZER_STATE].data_type);
			HccCompoundDataType* rasterizer_state_compound_data_type = hcc_compound_data_type_get(cu, hcc_data_type_strip_pointer(cu, rasterizer_state_ptr_data_type));
			rasterizer_state_variable_start_spirv_id = hcc_spirv_next_id_many(cu, rasterizer_state_compound_data_type->fields_count);
			for (uint32_t field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = &rasterizer_state_compound_data_type->fields[field_idx];
				HccSPIRVId variable_spirv_id = rasterizer_state_variable_start_spirv_id + field_idx;

				if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_POSITION) {
					rasterizer_state_variable_position_field_idx = field_idx;
					continue;
				}

				operands = hcc_spirv_add_global_variable(cu, 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_OUTPUT, hcc_pointer_data_type_deduplicate(cu, field->data_type));
				operands[1] = variable_spirv_id;
				operands[2] = HCC_SPIRV_STORAGE_CLASS_OUTPUT;

				operands = hcc_spirv_add_decorate(cu, 3);
				operands[0] = variable_spirv_id;
				operands[1] = HCC_SPIRV_DECORATION_LOCATION;
				operands[2] = field_idx;

				if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP) {
					operands = hcc_spirv_add_decorate(cu, 2);
					operands[0] = variable_spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_FLAT;
				}
			}
			break;
		};
		case HCC_SHADER_STAGE_FRAGMENT: {
			//
			// create the fragment state output variables
			HccDataType fragment_state_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, ast_function->params_and_variables[HCC_FRAGMENT_SHADER_PARAM_FRAGMENT_STATE].data_type);
			HccCompoundDataType* fragment_state_compound_data_type = hcc_compound_data_type_get(cu, hcc_data_type_strip_pointer(cu, fragment_state_ptr_data_type));
			fragment_state_variable_start_spirv_id = hcc_spirv_next_id_many(cu, fragment_state_compound_data_type->fields_count);
			for (uint32_t field_idx = 0; field_idx < fragment_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = &fragment_state_compound_data_type->fields[field_idx];
				HccSPIRVId variable_spirv_id = fragment_state_variable_start_spirv_id + field_idx;

				operands = hcc_spirv_add_global_variable(cu, 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_OUTPUT, hcc_pointer_data_type_deduplicate(cu, field->data_type));
				operands[1] = variable_spirv_id;
				operands[2] = HCC_SPIRV_STORAGE_CLASS_OUTPUT;

				operands = hcc_spirv_add_decorate(cu, 3);
				operands[0] = variable_spirv_id;
				operands[1] = HCC_SPIRV_DECORATION_LOCATION;
				operands[2] = field_idx;
			}

			//
			// create the rasterizer state input variables
			HccDataType rasterizer_state_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, ast_function->params_and_variables[HCC_FRAGMENT_SHADER_PARAM_RASTERIZER_STATE].data_type);
			HccCompoundDataType* rasterizer_state_compound_data_type = hcc_compound_data_type_get(cu, hcc_data_type_strip_pointer(cu, rasterizer_state_ptr_data_type));
			rasterizer_state_variable_start_spirv_id = hcc_spirv_next_id_many(cu, rasterizer_state_compound_data_type->fields_count);
			for (uint32_t field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = &rasterizer_state_compound_data_type->fields[field_idx];
				HccSPIRVId variable_spirv_id = rasterizer_state_variable_start_spirv_id + field_idx;

				if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_POSITION) {
					rasterizer_state_variable_position_field_idx = field_idx;
					continue;
				}

				operands = hcc_spirv_add_global_variable(cu, 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INPUT, hcc_pointer_data_type_deduplicate(cu, field->data_type));
				operands[1] = variable_spirv_id;
				operands[2] = HCC_SPIRV_STORAGE_CLASS_INPUT;

				operands = hcc_spirv_add_decorate(cu, 3);
				operands[0] = variable_spirv_id;
				operands[1] = HCC_SPIRV_DECORATION_LOCATION;
				operands[2] = field_idx;

				if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP) {
					operands = hcc_spirv_add_decorate(cu, 2);
					operands[0] = variable_spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_FLAT;
				}
			}
			break;
		};
		case HCC_SHADER_STAGE_COMPUTE:
		case HCC_SHADER_STAGE_MESHTASK:
		case HCC_SHADER_STAGE_MESH:
			HCC_ABORT("TODO");
	}

	//
	// hardcode the first basic block to be inserted as local variables must be inside the first basic block in SPIR-V
	operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LABEL, 1);
	operands[0] = hcc_spirvgen_convert_operand(w, HCC_AML_OPERAND(BASIC_BLOCK, 0));

	//
	// create the local variables
	for (uint32_t aml_word_idx = 0; aml_word_idx < aml_function->words_count; ) {
		HccAMLInstr* aml_instr = &aml_function->words[aml_word_idx];
		HccAMLOperand* aml_operands = HCC_AML_INSTR_OPERANDS(aml_instr);
		uint32_t aml_operands_count = HCC_AML_INSTR_OPERANDS_COUNT(aml_instr);
		HccAMLOp aml_op = HCC_AML_INSTR_OP(aml_instr);

		if (aml_op == HCC_AML_OP_PTR_STATIC_ALLOC) {
			HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);

			operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_VARIABLE, 3);
			operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_FUNCTION, return_data_type);
			operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
			operands[2] = HCC_SPIRV_STORAGE_CLASS_FUNCTION;
		}

		aml_word_idx += HCC_AML_INSTR_WORDS_COUNT(aml_instr);
	}

	for (uint32_t aml_word_idx = 0; aml_word_idx < aml_function->words_count; ) {
		HccAMLInstr* aml_instr = &aml_function->words[aml_word_idx];
		HccAMLOperand* aml_operands = HCC_AML_INSTR_OPERANDS(aml_instr);
		uint32_t aml_operands_count = HCC_AML_INSTR_OPERANDS_COUNT(aml_instr);
		HccAMLOp aml_op = HCC_AML_INSTR_OP(aml_instr);

		switch (aml_op) {
			case HCC_AML_OP_PTR_STATIC_ALLOC:
				// these translate into OpVariable in SPIR-V and are required to be at the start of a function.
				// so we do this before this loop.
				break;
			case HCC_AML_OP_PTR_LOAD: {
				uint32_t memory_operands = HCC_SPIRV_MEMORY_OPERANDS_NONE;
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType pointee_data_type = hcc_data_type_strip_pointer(cu, hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]));
				if (HCC_DATA_TYPE_IS_VOLATILE(pointee_data_type)) {
					memory_operands |= HCC_SPIRV_MEMORY_OPERANDS_VOLATILE;
				}

				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_operands[1]);

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3 + (memory_operands != 0));
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				if (memory_operands) {
					operands[3] = memory_operands;
				}
				break;
			};
			case HCC_AML_OP_PTR_STORE: {
				uint32_t memory_operands = HCC_SPIRV_MEMORY_OPERANDS_NONE;
				HccDataType pointee_data_type = hcc_data_type_strip_pointer(cu, hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]));
				if (HCC_DATA_TYPE_IS_VOLATILE(pointee_data_type)) {
					memory_operands |= HCC_SPIRV_MEMORY_OPERANDS_VOLATILE;
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_STORE, 2 + (memory_operands != 0));
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				if (memory_operands) {
					operands[2] = memory_operands;
				}
				break;
			};
			case HCC_AML_OP_PTR_ACCESS_CHAIN:
			case HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS: {
				uint32_t memory_operands = HCC_SPIRV_MEMORY_OPERANDS_NONE;
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);

				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_operands[1]);

				operands = hcc_spirv_function_add_instr(function, aml_op == HCC_AML_OP_PTR_ACCESS_CHAIN ? HCC_SPIRV_OP_ACCESS_CHAIN : HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, aml_operands_count + 1);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				for (uint32_t operand_idx = 2; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[operand_idx + 1] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};
			case HCC_AML_OP_COMPOSITE_INIT:
			case HCC_AML_OP_COMPOSITE_ACCESS_CHAIN_GET:
			case HCC_AML_OP_COMPOSITE_ACCESS_CHAIN_SET:
				HCC_ABORT("TODO");
				break;
			case HCC_AML_OP_BASIC_BLOCK: {
				if (aml_operands[0] == HCC_AML_OPERAND(BASIC_BLOCK, 0)) {
					// the first basic block is hardcoded before this loop
					break;
				}

				HccAMLBasicBlock* basic_block = &aml_function->basic_blocks[HCC_AML_OPERAND_AUX(aml_operands[0])];
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LABEL, 1);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);

				for (uint32_t param_idx = 0; param_idx < basic_block->params_count; param_idx += 1) {
					HccAMLBasicBlockParam* param = &aml_function->basic_block_params[basic_block->params_start_idx + param_idx];
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_PHI, 2 + (param->srcs_count * 2));
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, param->data_type);
					operands[1] = hcc_spirvgen_convert_operand(w, HCC_AML_OPERAND(BASIC_BLOCK_PARAM, basic_block->params_start_idx + param_idx));
					for (uint32_t src_idx = 0; src_idx < param->srcs_count; src_idx += 1) {
						HccAMLBasicBlockParamSrc* src = &aml_function->basic_block_param_srcs[param->srcs_start_idx + src_idx];
						operands[2 + (src_idx * 2) + 0] = hcc_spirvgen_convert_operand(w, src->operand);
						operands[2 + (src_idx * 2) + 1] = hcc_spirvgen_convert_operand(w, src->basic_block_operand);
					}
				}
				break;
			};
			case HCC_AML_OP_BRANCH: {
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_BRANCH, 1);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				break;
			};
			case HCC_AML_OP_BRANCH_CONDITIONAL: {
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_BRANCH_CONDITIONAL, 3);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				break;
			};
			case HCC_AML_OP_SWITCH:
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SWITCH, aml_operands_count);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				for (uint32_t operand_idx = 2; operand_idx < aml_operands_count; operand_idx += 2) {
					HCC_DEBUG_ASSERT(HCC_AML_OPERAND_IS_CONSTANT(aml_operands[operand_idx + 0]), "expected switch case constant");
					HCC_DEBUG_ASSERT(HCC_AML_OPERAND_IS_BASIC_BLOCK(aml_operands[operand_idx + 1]), "expected switch case basic block");
					HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[operand_idx + 0])));
					operands[operand_idx + 0] = hcc_constant_read_32(constant);
					operands[operand_idx + 1] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx + 1]);
				}
				break;
			case HCC_AML_OP_ADD:
			case HCC_AML_OP_SUBTRACT:
			case HCC_AML_OP_MULTIPLY:
			case HCC_AML_OP_DIVIDE:
			case HCC_AML_OP_MODULO:
			case HCC_AML_OP_BIT_SHIFT_RIGHT:
			case HCC_AML_OP_EQUAL:
			case HCC_AML_OP_NOT_EQUAL:
			case HCC_AML_OP_LESS_THAN:
			case HCC_AML_OP_LESS_THAN_OR_EQUAL:
			case HCC_AML_OP_GREATER_THAN:
			case HCC_AML_OP_GREATER_THAN_OR_EQUAL:
			case HCC_AML_OP_NEGATE:
			{
				HccDataType data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HccBasicTypeClass type_class = hcc_basic_type_class(cu, data_type);

				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_ADD:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_I_ADD; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_I_ADD; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_ADD; break;
						}
						break;
					case HCC_AML_OP_SUBTRACT:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_I_SUB; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_I_SUB; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_SUB; break;
						}
						break;
					case HCC_AML_OP_MULTIPLY:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_I_MUL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_I_MUL; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_MUL; break;
						}
						break;
					case HCC_AML_OP_DIVIDE:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_U_DIV; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_S_DIV; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_DIV; break;
						}
						break;
					case HCC_AML_OP_MODULO:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_U_MOD; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_S_MOD; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_MOD; break;
						}
						break;
					case HCC_AML_OP_BIT_SHIFT_RIGHT:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_LOGICAL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_BITWISE_SHIFT_RIGHT_ARITHMETIC; break;
						}
						break;
					case HCC_AML_OP_EQUAL:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_BOOL: op = HCC_SPIRV_OP_LOGICAL_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_I_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_I_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_UNORD_EQUAL; break;
						}
						break;
					case HCC_AML_OP_NOT_EQUAL:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_BOOL: op = HCC_SPIRV_OP_LOGICAL_NOT_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_I_NOT_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_I_NOT_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_UNORD_NOT_EQUAL; break;
						}
						break;
					case HCC_AML_OP_LESS_THAN:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_U_LESS_THAN; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_S_LESS_THAN; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_UNORD_LESS_THAN; break;
						}
						break;
					case HCC_AML_OP_LESS_THAN_OR_EQUAL:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_U_LESS_THAN_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_S_LESS_THAN_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_UNORD_LESS_THAN_EQUAL; break;
						}
						break;
					case HCC_AML_OP_GREATER_THAN:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_U_GREATER_THAN; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_S_GREATER_THAN; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_UNORD_GREATER_THAN; break;
						}
						break;
					case HCC_AML_OP_GREATER_THAN_OR_EQUAL:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_U_GREATER_THAN_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_S_GREATER_THAN_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_UNORD_GREATER_THAN_EQUAL; break;
						}
						break;
					case HCC_AML_OP_NEGATE:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_S_NEGATE; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_NEGATE; break;
						}
						break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u' with type_class '%u'", aml_op, type_class);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 1);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				for (uint32_t operand_idx = 0; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[1 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};
			case HCC_AML_OP_BIT_AND:
			case HCC_AML_OP_BIT_OR:
			case HCC_AML_OP_BIT_XOR:
			case HCC_AML_OP_BIT_SHIFT_LEFT:
			case HCC_AML_OP_SELECT:
			case HCC_AML_OP_ANY:
			case HCC_AML_OP_ALL:
			case HCC_AML_OP_ISINF:
			case HCC_AML_OP_ISNAN:
			case HCC_AML_OP_DOT:
			case HCC_AML_OP_MATRIX_MUL:
			case HCC_AML_OP_MATRIX_MUL_SCALAR:
			case HCC_AML_OP_MATRIX_MUL_VECTOR:
			case HCC_AML_OP_VECTOR_MUL_MATRIX:
			case HCC_AML_OP_MATRIX_TRANSPOSE:
			case HCC_AML_OP_MATRIX_OUTER_PRODUCT:
			{
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_BIT_AND: op = HCC_SPIRV_OP_BITWISE_AND; break;
					case HCC_AML_OP_BIT_OR: op = HCC_SPIRV_OP_BITWISE_OR; break;
					case HCC_AML_OP_BIT_XOR: op = HCC_SPIRV_OP_BITWISE_XOR; break;
					case HCC_AML_OP_BIT_SHIFT_LEFT: op = HCC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL; break;
					case HCC_AML_OP_SELECT: op = HCC_SPIRV_OP_SELECT; break;
					case HCC_AML_OP_ANY: op = HCC_SPIRV_OP_ANY; break;
					case HCC_AML_OP_ALL: op = HCC_SPIRV_OP_ALL; break;
					case HCC_AML_OP_ISINF: op = HCC_SPIRV_OP_ISINF; break;
					case HCC_AML_OP_ISNAN: op = HCC_SPIRV_OP_ISNAN; break;
					case HCC_AML_OP_DOT: op = HCC_SPIRV_OP_DOT; break;
					case HCC_AML_OP_MATRIX_MUL: op = HCC_SPIRV_OP_MATRIX_TIMES_MATRIX; break;
					case HCC_AML_OP_MATRIX_MUL_SCALAR: op = HCC_SPIRV_OP_MATRIX_TIMES_SCALAR; break;
					case HCC_AML_OP_MATRIX_MUL_VECTOR: op = HCC_SPIRV_OP_MATRIX_TIMES_VECTOR; break;
					case HCC_AML_OP_VECTOR_MUL_MATRIX: op = HCC_SPIRV_OP_VECTOR_TIMES_MATRIX; break;
					case HCC_AML_OP_MATRIX_TRANSPOSE: op = HCC_SPIRV_OP_TRANSPOSE; break;
					case HCC_AML_OP_MATRIX_OUTER_PRODUCT: op = HCC_SPIRV_OP_OUTER_PRODUCT; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 1);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				for (uint32_t operand_idx = 0; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[1 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};
			case HCC_AML_OP_BITCAST: {
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_BITCAST, aml_operands_count + 1);
				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_operands[1]);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				break;
			};
			case HCC_AML_OP_CONVERT: {
				HccDataType dst_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HccBasicTypeClass dst_type_class = hcc_basic_type_class(cu, dst_data_type);
				HccBasicTypeClass src_type_class = hcc_basic_type_class(cu, src_data_type);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccSPIRVId result_type = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				HccSPIRVId result_operand = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId src_operand = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (dst_type_class) {
					///////////////////////////////////////
					// case HCC_BASIC_TYPE_CLASS_BOOL:
					// ^^ this is handled in the HccAMLGen, see calls to hcc_amlgen_generate_convert_to_bool
					///////////////////////////////////////

					case HCC_BASIC_TYPE_CLASS_UINT:
						switch (src_type_class) {
							case HCC_BASIC_TYPE_CLASS_BOOL:
								op = HCC_SPIRV_OP_SELECT;
								break;
							case HCC_BASIC_TYPE_CLASS_UINT:
								op = HCC_SPIRV_OP_U_CONVERT;
								break;
							case HCC_BASIC_TYPE_CLASS_SINT: {
								HccDataType signed_dst_data_type = hcc_data_type_unsigned_to_signed(cu, dst_data_type);
								if (signed_dst_data_type != src_data_type) {
									HccSPIRVId dst_operand = hcc_spirv_next_id(cu);

									operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_S_CONVERT, 3);
									operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, signed_dst_data_type);
									operands[1] = dst_operand;
									operands[2] = src_operand;

									src_operand = dst_operand;
								}
								op = HCC_SPIRV_OP_BITCAST;
								break;
							};
							case HCC_BASIC_TYPE_CLASS_FLOAT:
								op = HCC_SPIRV_OP_CONVERT_F_TO_U;
								break;
						}
						break;
					case HCC_BASIC_TYPE_CLASS_SINT:
						switch (src_type_class) {
							case HCC_BASIC_TYPE_CLASS_BOOL:
								op = HCC_SPIRV_OP_SELECT;
								break;
							case HCC_BASIC_TYPE_CLASS_UINT: {
								HccDataType unsigned_dst_data_type = hcc_data_type_signed_to_unsigned(cu, dst_data_type);
								if (unsigned_dst_data_type != src_data_type) {
									HccSPIRVId dst_operand = hcc_spirv_next_id(cu);

									operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_U_CONVERT, 3);
									operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, unsigned_dst_data_type);
									operands[1] = dst_operand;
									operands[2] = src_operand;

									src_operand = dst_operand;
								}
								op = HCC_SPIRV_OP_BITCAST;
								break;
							};
							case HCC_BASIC_TYPE_CLASS_SINT:
								op = HCC_SPIRV_OP_S_CONVERT;
								break;
							case HCC_BASIC_TYPE_CLASS_FLOAT:
								op = HCC_SPIRV_OP_CONVERT_F_TO_S;
								break;
						}
						break;
					case HCC_BASIC_TYPE_CLASS_FLOAT:
						switch (src_type_class) {
							case HCC_BASIC_TYPE_CLASS_BOOL:
								op = HCC_SPIRV_OP_SELECT;
								break;
							case HCC_BASIC_TYPE_CLASS_UINT:
								op = HCC_SPIRV_OP_CONVERT_U_TO_F;
								break;
							case HCC_BASIC_TYPE_CLASS_SINT:
								op = HCC_SPIRV_OP_CONVERT_S_TO_F;
								break;
							case HCC_BASIC_TYPE_CLASS_FLOAT:
								op = HCC_SPIRV_OP_F_CONVERT;
								break;
						}
						break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for dst_type_class '%u' with src_type_class '%u'", dst_type_class, src_type_class);

				if (op == HCC_SPIRV_OP_SELECT) {
					operands = hcc_spirv_function_add_instr(function, op, 5);
					operands[0] = result_type;
					operands[1] = result_operand;
					operands[2] = src_operand;
					operands[3] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_one(w->cu, dst_data_type));
					operands[4] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, dst_data_type));
				} else {
					operands = hcc_spirv_function_add_instr(function, op, 3);
					operands[0] = result_type;
					operands[1] = result_operand;
					operands[2] = src_operand;
				}

				break;
			};
			case HCC_AML_OP_CALL: {
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDecl function_decl = aml_operands[1];
				if (!HCC_DECL_IS_FUNCTION(function_decl)) {
					HCC_ABORT("TODO report this error using the error system: we do not allow function pointers that can not be compiled out");
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_FUNCTION_CALL, aml_operands_count + 1);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				for (uint32_t operand_idx = 0; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[1 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};
			case HCC_AML_OP_RETURN:
				if (aml_operands[0]) {
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_RETURN_VALUE, 1);
					operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				} else {
					hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_RETURN, 0);
				}
				break;
			case HCC_AML_OP_UNREACHABLE:
				hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_UNREACHABLE, 0);
				break;

			case HCC_AML_OP_SELECTION_MERGE:
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECTION_MERGE, 2);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = HCC_SPIRV_SELECTION_CONTROL_NONE;
				break;

			case HCC_AML_OP_LOOP_MERGE:
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOOP_MERGE, 3);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[2] = HCC_SPIRV_LOOP_CONTROL_NONE;
				break;

			case HCC_AML_OP_SYSTEM_VALUE_LOAD: {
				uint32_t return_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);

				HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[1])));
				uint32_t system_value_idx = hcc_constant_read_32(constant);

				switch (aml_function->shader_stage) {
					case HCC_SHADER_STAGE_VERTEX:
						switch (system_value_idx) {
							case HCC_VERTEX_SV_VERTEX_IDX:
								operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3);
								operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_S32);
								operands[1] = return_spirv_id;
								operands[2] = HCC_SPIRV_ID_VARIABLE_INPUT_VERTEX_INDEX;
								hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_VERTEX_INDEX);
								break;
							case HCC_VERTEX_SV_INSTANCE_IDX:
								operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3);
								operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_S32);
								operands[1] = return_spirv_id;
								operands[2] = HCC_SPIRV_ID_VARIABLE_INPUT_INSTANCE_INDEX;
								hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_INSTANCE_INDEX);
								break;
						}
						break;
					case HCC_SHADER_STAGE_FRAGMENT:
					default: HCC_ABORT("unhandled shader stage: %u", aml_function->shader_stage);
				}
				break;
			};

			case HCC_AML_OP_RASTERIZER_STATE_LOAD_FIELD: {
				uint32_t return_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);

				HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[1])));
				uint32_t field_idx = hcc_constant_read_32(constant);

				HccSPIRVId src_spirv_id;
				if (field_idx == rasterizer_state_variable_position_field_idx) {
					src_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_FRAG_COORD;
				} else {
					src_spirv_id = rasterizer_state_variable_start_spirv_id + field_idx;
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]));
				operands[1] = return_spirv_id;
				operands[2] = src_spirv_id;
				hcc_spirvgen_found_global(w, src_spirv_id);
				break;
			};

			case HCC_AML_OP_RASTERIZER_STATE_STORE_FIELD: {
				uint32_t src_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);

				HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[1])));
				uint32_t field_idx = hcc_constant_read_32(constant);

				HccSPIRVId dst_spirv_id;
				if (field_idx == rasterizer_state_variable_position_field_idx) {
					dst_spirv_id = HCC_SPIRV_ID_VARIABLE_OUTPUT_POSITION;
				} else {
					dst_spirv_id = rasterizer_state_variable_start_spirv_id + field_idx;
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_STORE, 2);
				operands[0] = dst_spirv_id;
				operands[1] = src_spirv_id;
				hcc_spirvgen_found_global(w, dst_spirv_id);
				break;
			};

			case HCC_AML_OP_FRAGMENT_STATE_STORE_FIELD: {
				uint32_t src_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);

				HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[1])));
				uint32_t field_idx = hcc_constant_read_32(constant);

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_STORE, 2);
				operands[0] = fragment_state_variable_start_spirv_id + field_idx;
				operands[1] = src_spirv_id;
				hcc_spirvgen_found_global(w, fragment_state_variable_start_spirv_id + field_idx);
				break;
			};

			case HCC_AML_OP_MIN:
			case HCC_AML_OP_MAX:
			case HCC_AML_OP_CLAMP:
			case HCC_AML_OP_SIGN:
			case HCC_AML_OP_ABS:
			{
				HccDataType data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HccBasicTypeClass type_class = hcc_basic_type_class(cu, data_type);

				HccSPIRVGLSLSTD450Op op = HCC_SPIRV_GLSL_STD_450_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_MIN:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_GLSL_STD_450_OP_U_MIN; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_GLSL_STD_450_OP_S_MIN; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_GLSL_STD_450_OP_F_MIN; break;
						}
						break;
					case HCC_AML_OP_MAX:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_GLSL_STD_450_OP_U_MAX; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_GLSL_STD_450_OP_S_MAX; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_GLSL_STD_450_OP_F_MAX; break;
						}
						break;
					case HCC_AML_OP_CLAMP:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_GLSL_STD_450_OP_U_CLAMP; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_GLSL_STD_450_OP_S_CLAMP; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_GLSL_STD_450_OP_F_CLAMP; break;
						}
						break;
					case HCC_AML_OP_SIGN:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_GLSL_STD_450_OP_S_SIGN; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_GLSL_STD_450_OP_F_SIGN; break;
						}
						break;
					case HCC_AML_OP_ABS:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_GLSL_STD_450_OP_S_ABS; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_GLSL_STD_450_OP_F_ABS; break;
						}
						break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_GLSL_STD_450_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_EXT_INST, aml_operands_count + 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = HCC_SPIRV_ID_GLSL_STD_450;
				operands[3] = op;
				for (uint32_t operand_idx = 1; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[3 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};
			case HCC_AML_OP_FMA:
			case HCC_AML_OP_FLOOR:
			case HCC_AML_OP_CEIL:
			case HCC_AML_OP_ROUND:
			case HCC_AML_OP_TRUNC:
			case HCC_AML_OP_FRACT:
			case HCC_AML_OP_RADIANS:
			case HCC_AML_OP_DEGREES:
			case HCC_AML_OP_STEP:
			case HCC_AML_OP_SMOOTHSTEP:
			case HCC_AML_OP_SIN:
			case HCC_AML_OP_COS:
			case HCC_AML_OP_TAN:
			case HCC_AML_OP_ASIN:
			case HCC_AML_OP_ACOS:
			case HCC_AML_OP_ATAN:
			case HCC_AML_OP_SINH:
			case HCC_AML_OP_COSH:
			case HCC_AML_OP_TANH:
			case HCC_AML_OP_ASINH:
			case HCC_AML_OP_ACOSH:
			case HCC_AML_OP_ATANH:
			case HCC_AML_OP_ATAN2:
			case HCC_AML_OP_POW:
			case HCC_AML_OP_EXP:
			case HCC_AML_OP_LOG:
			case HCC_AML_OP_EXP2:
			case HCC_AML_OP_LOG2:
			case HCC_AML_OP_SQRT:
			case HCC_AML_OP_RSQRT:
			case HCC_AML_OP_LERP:
			case HCC_AML_OP_LEN:
			case HCC_AML_OP_NORM:
			case HCC_AML_OP_REFLECT:
			case HCC_AML_OP_REFRACT:
			case HCC_AML_OP_PACK_F16X2_F32X2:
			case HCC_AML_OP_UNPACK_F16X2_F32X2:
			case HCC_AML_OP_PACK_U16X2_F32X2:
			case HCC_AML_OP_UNPACK_U16X2_F32X2:
			case HCC_AML_OP_PACK_S16X2_F32X2:
			case HCC_AML_OP_UNPACK_S16X2_F32X2:
			case HCC_AML_OP_PACK_U8X4_F32X4:
			case HCC_AML_OP_UNPACK_U8X4_F32X4:
			case HCC_AML_OP_PACK_S8X4_F32X4:
			case HCC_AML_OP_UNPACK_S8X4_F32X4:
			{
				HccSPIRVGLSLSTD450Op op = HCC_SPIRV_GLSL_STD_450_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_FMA: op = HCC_SPIRV_GLSL_STD_450_OP_FMA; break;
					case HCC_AML_OP_FLOOR: op = HCC_SPIRV_GLSL_STD_450_OP_FLOOR; break;
					case HCC_AML_OP_CEIL: op = HCC_SPIRV_GLSL_STD_450_OP_CEIL; break;
					case HCC_AML_OP_ROUND: op = HCC_SPIRV_GLSL_STD_450_OP_ROUND; break;
					case HCC_AML_OP_TRUNC: op = HCC_SPIRV_GLSL_STD_450_OP_TRUNC; break;
					case HCC_AML_OP_FRACT: op = HCC_SPIRV_GLSL_STD_450_OP_FRACT; break;
					case HCC_AML_OP_RADIANS: op = HCC_SPIRV_GLSL_STD_450_OP_RADIANS; break;
					case HCC_AML_OP_DEGREES: op = HCC_SPIRV_GLSL_STD_450_OP_DEGREES; break;
					case HCC_AML_OP_STEP: op = HCC_SPIRV_GLSL_STD_450_OP_STEP; break;
					case HCC_AML_OP_SMOOTHSTEP: op = HCC_SPIRV_GLSL_STD_450_OP_SMOOTHSTEP; break;
					case HCC_AML_OP_SIN: op = HCC_SPIRV_GLSL_STD_450_OP_SIN; break;
					case HCC_AML_OP_COS: op = HCC_SPIRV_GLSL_STD_450_OP_COS; break;
					case HCC_AML_OP_TAN: op = HCC_SPIRV_GLSL_STD_450_OP_TAN; break;
					case HCC_AML_OP_ASIN: op = HCC_SPIRV_GLSL_STD_450_OP_ASIN; break;
					case HCC_AML_OP_ACOS: op = HCC_SPIRV_GLSL_STD_450_OP_ACOS; break;
					case HCC_AML_OP_ATAN: op = HCC_SPIRV_GLSL_STD_450_OP_ATAN; break;
					case HCC_AML_OP_SINH: op = HCC_SPIRV_GLSL_STD_450_OP_SINH; break;
					case HCC_AML_OP_COSH: op = HCC_SPIRV_GLSL_STD_450_OP_COSH; break;
					case HCC_AML_OP_TANH: op = HCC_SPIRV_GLSL_STD_450_OP_TANH; break;
					case HCC_AML_OP_ASINH: op = HCC_SPIRV_GLSL_STD_450_OP_ASINH; break;
					case HCC_AML_OP_ACOSH: op = HCC_SPIRV_GLSL_STD_450_OP_ACOSH; break;
					case HCC_AML_OP_ATANH: op = HCC_SPIRV_GLSL_STD_450_OP_ATANH; break;
					case HCC_AML_OP_ATAN2: op = HCC_SPIRV_GLSL_STD_450_OP_ATAN2; break;
					case HCC_AML_OP_POW: op = HCC_SPIRV_GLSL_STD_450_OP_POW; break;
					case HCC_AML_OP_EXP: op = HCC_SPIRV_GLSL_STD_450_OP_EXP; break;
					case HCC_AML_OP_LOG: op = HCC_SPIRV_GLSL_STD_450_OP_LOG; break;
					case HCC_AML_OP_EXP2: op = HCC_SPIRV_GLSL_STD_450_OP_EXP2; break;
					case HCC_AML_OP_LOG2: op = HCC_SPIRV_GLSL_STD_450_OP_LOG2; break;
					case HCC_AML_OP_SQRT: op = HCC_SPIRV_GLSL_STD_450_OP_SQRT; break;
					case HCC_AML_OP_RSQRT: op = HCC_SPIRV_GLSL_STD_450_OP_INVERSE_SQRT; break;
					case HCC_AML_OP_LERP: op = HCC_SPIRV_GLSL_STD_450_OP_F_MIX; break;
					case HCC_AML_OP_LEN: op = HCC_SPIRV_GLSL_STD_450_OP_LENGTH; break;
					case HCC_AML_OP_NORM: op = HCC_SPIRV_GLSL_STD_450_OP_NORMALIZE; break;
					case HCC_AML_OP_REFLECT: op = HCC_SPIRV_GLSL_STD_450_OP_REFLECT; break;
					case HCC_AML_OP_REFRACT: op = HCC_SPIRV_GLSL_STD_450_OP_REFRACT; break;
					case HCC_AML_OP_PACK_F16X2_F32X2: op = HCC_SPIRV_GLSL_STD_450_OP_PACK_HALF2X16; break;
					case HCC_AML_OP_UNPACK_F16X2_F32X2: op = HCC_SPIRV_GLSL_STD_450_OP_UNPACK_HALF2X16; break;
					case HCC_AML_OP_PACK_U16X2_F32X2: op = HCC_SPIRV_GLSL_STD_450_OP_PACK_UNORM2X16; break;
					case HCC_AML_OP_UNPACK_U16X2_F32X2: op = HCC_SPIRV_GLSL_STD_450_OP_UNPACK_UNORM2X16; break;
					case HCC_AML_OP_PACK_S16X2_F32X2: op = HCC_SPIRV_GLSL_STD_450_OP_PACK_SNORM2X16; break;
					case HCC_AML_OP_UNPACK_S16X2_F32X2: op = HCC_SPIRV_GLSL_STD_450_OP_UNPACK_SNORM2X16; break;
					case HCC_AML_OP_PACK_U8X4_F32X4: op = HCC_SPIRV_GLSL_STD_450_OP_PACK_UNORM4X8; break;
					case HCC_AML_OP_UNPACK_U8X4_F32X4: op = HCC_SPIRV_GLSL_STD_450_OP_UNPACK_UNORM4X8; break;
					case HCC_AML_OP_PACK_S8X4_F32X4: op = HCC_SPIRV_GLSL_STD_450_OP_PACK_SNORM4X8; break;
					case HCC_AML_OP_UNPACK_S8X4_F32X4: op = HCC_SPIRV_GLSL_STD_450_OP_UNPACK_SNORM4X8; break;
					case HCC_AML_OP_MATRIX_DETERMINANT: op = HCC_SPIRV_GLSL_STD_450_OP_DETERMINANT; break;
					case HCC_AML_OP_MATRIX_INVERSE: op = HCC_SPIRV_GLSL_STD_450_OP_MATRIX_INVERSE; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_GLSL_STD_450_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_EXT_INST, aml_operands_count + 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = HCC_SPIRV_ID_GLSL_STD_450;
				operands[3] = op;
				for (uint32_t operand_idx = 1; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[3 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};

			case HCC_AML_OP_PACK:
			case HCC_AML_OP_UNPACK:
				HCC_ABORT("TODO");
				break;

			default: HCC_ABORT("unhandled AML OP: %u '%s'", aml_op, hcc_aml_op_code_strings[aml_op]);
		}

		aml_word_idx += HCC_AML_INSTR_WORDS_COUNT(aml_instr);
	}

	hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_FUNCTION_END, 0);

	if (aml_function->shader_stage != HCC_SHADER_STAGE_NONE) {
		HccSPIRVEntryPoint* ep = hcc_stack_push_thread_safe(cu->spirv.entry_points);
		ep->shader_stage = aml_function->shader_stage;
		ep->identifier_string_id = aml_function->identifier_string_id;
		ep->spirv_id = function_spirv_id;
		ep->global_variable_ids = hcc_stack_push_many_thread_safe(cu->spirv.entry_point_global_variable_ids, w->spirvgen.function_unique_globals_count);
		ep->global_variables_count = w->spirvgen.function_unique_globals_count;
		HCC_COPY_ELMT_MANY(ep->global_variable_ids, w->spirvgen.function_unique_globals, w->spirvgen.function_unique_globals_count);
	}
}

