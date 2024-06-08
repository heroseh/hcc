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

	w->spirvgen.value_map = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRVLINK_WORDS, setup->astgen.function_params_and_variables_reserve_cap, setup->astgen.function_params_and_variables_reserve_cap);
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
		case HCC_AML_OPERAND_VALUE: {
			uint32_t value_idx = HCC_AML_OPERAND_AUX(aml_operand);
			spirv_id = w->spirvgen.value_map[value_idx];
			if (spirv_id) {
				return spirv_id;
			}

			spirv_id = w->spirvgen.value_base_id + value_idx;

			// the first 'params_count' value registers are the immuatable parameters.
			// all of the mutable parameters and variables come after that.
			const HccAMLFunction* aml_function = w->spirvgen.aml_function;
			if (aml_function->shader_stage != HCC_SHADER_STAGE_NONE && value_idx < aml_function->params_count * 2) {
				HccAMLOp op = HCC_AML_OP_NO_OP;
				switch (aml_function->shader_stage) {
					case HCC_SHADER_STAGE_VERTEX:
						switch (value_idx) {
							case HCC_VERTEX_SHADER_PARAM_BC: spirv_id = w->spirvgen.bc_spirv_id; break;
							default: HCC_ABORT("unhandled vertex shader parameter: %u", value_idx);
						}
						break;
					case HCC_SHADER_STAGE_PIXEL:
						switch (value_idx) {
							case HCC_PIXEL_SHADER_PARAM_BC: spirv_id = w->spirvgen.bc_spirv_id; break;
							default: HCC_ABORT("unhandled pixel shader parameter: %u", value_idx);
						}
						break;
					case HCC_SHADER_STAGE_COMPUTE:
						switch (value_idx) {
							case HCC_COMPUTE_SHADER_PARAM_BC: spirv_id = w->spirvgen.bc_spirv_id; break;
							default: HCC_ABORT("unhandled compute shader parameter: %u", value_idx);
						}
						break;
					default: HCC_ABORT("unhandled shader stage: %u", aml_function->shader_stage);
				}
			}
			w->spirvgen.value_map[value_idx] = spirv_id;
			break;
		};
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

HccSPIRVId hcc_spirvgen_convert_to_spirv_bool(HccWorker* w, HccSPIRVFunction* function, HccSPIRVId src_operand, HccDataType src_data_type) {
	uint32_t columns = HCC_DATA_TYPE_IS_AML_INTRINSIC(src_data_type) ? HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(HCC_DATA_TYPE_AUX(src_data_type)) : 1;
	HccSPIRVId dst_operand = hcc_spirv_next_id(w->cu);
	HccSPIRVOp op = hcc_basic_type_class(w->cu, src_data_type) == HCC_BASIC_TYPE_CLASS_FLOAT ? HCC_SPIRV_OP_F_UNORD_NOT_EQUAL : HCC_SPIRV_OP_I_NOT_EQUAL;
	HccSPIRVOperand* operands = hcc_spirv_function_add_instr(function, op, 4);
	operands[0] = HCC_SPIRV_ID_TYPE_BOOL + columns - 1;
	operands[1] = dst_operand;
	operands[2] = src_operand;
	operands[3] = hcc_spirv_constant_deduplicate(w->cu, hcc_constant_table_deduplicate_zero(w->cu, src_data_type));
	return dst_operand;
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
	const HccAMLFunction* aml_function = hcc_aml_function_get(cu, function_decl);
	HCC_ZERO_ELMT_MANY(w->spirvgen.value_map, aml_function->values_count);

	HccSPIRVFunction* function = hcc_stack_get(cu->spirv.functions, HCC_DECL_AUX(function_decl));
	w->spirvgen.function = function;
	w->spirvgen.aml_function = aml_function;
	w->spirvgen.value_base_id = hcc_spirv_next_id_many(cu, aml_function->values_count);
	w->spirvgen.basic_block_base_id = hcc_spirv_next_id_many(cu, aml_function->basic_blocks_count);
	w->spirvgen.basic_block_param_base_id = hcc_spirv_next_id_many(cu, aml_function->basic_block_params_count);

	function->words_cap = (uint32_t)floorf((float)aml_function->words_cap * 1.5f);
	function->words = hcc_stack_push_many(cu->spirv.function_words, function->words_cap);

	HccSPIRVId function_spirv_id = hcc_spirv_decl_deduplicate(cu, function_decl);
	hcc_spirv_add_name(cu, function_spirv_id, hcc_string_table_get(hcc_decl_identifier_string_id(cu, function_decl)));

	HccDataType function_data_type = aml_function->function_data_type;
	w->spirvgen.bc_spirv_id = HCC_SPIRV_ID_INVALID;
	if (aml_function->shader_stage != HCC_SHADER_STAGE_NONE) {
		function_data_type = hcc_function_data_type_deduplicate(cu, HCC_DATA_TYPE_AML_INTRINSIC_VOID, NULL, 0, 0);

		uint32_t param_idx;
		switch (aml_function->shader_stage) {
			case HCC_SHADER_STAGE_VERTEX:
				param_idx = HCC_VERTEX_SHADER_PARAM_BC;
				break;
			case HCC_SHADER_STAGE_PIXEL:
				param_idx = HCC_PIXEL_SHADER_PARAM_BC;
				break;
			case HCC_SHADER_STAGE_COMPUTE:
				param_idx = HCC_COMPUTE_SHADER_PARAM_BC;
				break;
			HCC_ABORT("unhandled shader type %u", aml_function->shader_stage);
		}

		//
		// add the bundled constants global variable for this shader
		w->spirvgen.bc_spirv_id = hcc_spirv_next_id(cu);
		HccDataType bc_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, aml_function->values[param_idx].data_type);
		HccDataType bc_elmt_data_type = hcc_data_type_strip_pointer(cu, bc_data_type);
		HccSPIRVOperand* operands = hcc_spirv_add_global_variable(cu, 3);
		HccSPIRVStorageClass storage_class = HCC_SPIRV_STORAGE_CLASS_PUSH_CONSTANT;
		hcc_spirv_decorate_block_deduplicate(cu, hcc_spirv_type_deduplicate(cu, storage_class, hcc_decl_resolve_and_strip_qualifiers(cu, bc_elmt_data_type)));
		operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, bc_data_type);
		operands[1] = w->spirvgen.bc_spirv_id;
		operands[2] = storage_class;
		hcc_spirvgen_found_global(w, w->spirvgen.bc_spirv_id);
	}

	HccSPIRVOperand* operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_FUNCTION, 4);
	operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, aml_function->return_data_type);
	operands[1] = function_spirv_id;
	operands[2] = HCC_SPIRV_FUNCTION_CTRL_NONE;
	operands[3] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, function_data_type);

	w->spirvgen.rasterizer_state_variable_base_spirv_id = HCC_SPIRV_ID_INVALID;
	w->spirvgen.pixel_state_variable_base_spirv_id = HCC_SPIRV_ID_INVALID;
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
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_VERTEX_IDX);
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_INSTANCE_IDX);
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_OUTPUT_POSITION);

			//
			// create the rasterizer state output variables
			HccDataType rasterizer_state_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, aml_function->values[HCC_VERTEX_SHADER_PARAM_RASTERIZER_STATE].data_type);
			HccDataType rasterizer_state_data_type = hcc_data_type_strip_pointer(cu, rasterizer_state_ptr_data_type);
			if (rasterizer_state_data_type != HCC_DATA_TYPE_AML_INTRINSIC_VOID) {
				HccCompoundDataType* rasterizer_state_compound_data_type = hcc_compound_data_type_get(cu, rasterizer_state_data_type);
				w->spirvgen.rasterizer_state_variable_base_spirv_id = hcc_spirv_next_id_many(cu, rasterizer_state_compound_data_type->fields_count);

				for (uint32_t field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
					HccCompoundField* field = &rasterizer_state_compound_data_type->fields[field_idx];
					HccDataType field_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, hcc_pointer_data_type_deduplicate(cu, field->data_type));
					HccSPIRVId field_variable_spirv_id = w->spirvgen.rasterizer_state_variable_base_spirv_id + field_idx;

					operands = hcc_spirv_add_global_variable(cu, 3);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_OUTPUT, field_ptr_data_type);
					operands[1] = field_variable_spirv_id;
					operands[2] = HCC_SPIRV_STORAGE_CLASS_OUTPUT;

					operands = hcc_spirv_add_decorate(cu, 3);
					operands[0] = field_variable_spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_LOCATION;
					operands[2] = field_idx;

					if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP) {
						operands = hcc_spirv_add_decorate(cu, 2);
						operands[0] = field_variable_spirv_id;
						operands[1] = HCC_SPIRV_DECORATION_FLAT;
					}

					hcc_spirvgen_found_global(w, field_variable_spirv_id);
				}
			}
			break;
		};
		case HCC_SHADER_STAGE_PIXEL: {
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_FRAG_COORD);
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_OUTPUT_FRAG_DEPTH);

			//
			// create the pixel state output variables
			HccDataType pixel_state_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, aml_function->values[HCC_PIXEL_SHADER_PARAM_PIXEL_STATE].data_type);
			HccDataType pixel_state_data_type = hcc_data_type_strip_pointer(cu, pixel_state_ptr_data_type);
			if (pixel_state_data_type != HCC_DATA_TYPE_CONST(HCC_DATA_TYPE_AML_INTRINSIC_VOID)) {
				HccCompoundDataType* pixel_state_compound_data_type = hcc_compound_data_type_get(cu, pixel_state_data_type);
				w->spirvgen.pixel_state_variable_base_spirv_id = hcc_spirv_next_id_many(cu, pixel_state_compound_data_type->fields_count);

				for (uint32_t field_idx = 0; field_idx < pixel_state_compound_data_type->fields_count; field_idx += 1) {
					HccCompoundField* field = &pixel_state_compound_data_type->fields[field_idx];
					HccDataType field_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, hcc_pointer_data_type_deduplicate(cu, field->data_type));
					HccSPIRVId field_variable_spirv_id = w->spirvgen.pixel_state_variable_base_spirv_id + field_idx;

					operands = hcc_spirv_add_global_variable(cu, 3);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_OUTPUT, field_ptr_data_type);
					operands[1] = field_variable_spirv_id;
					operands[2] = HCC_SPIRV_STORAGE_CLASS_OUTPUT;

					operands = hcc_spirv_add_decorate(cu, 3);
					operands[0] = field_variable_spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_LOCATION;
					operands[2] = field_idx;

					hcc_spirvgen_found_global(w, field_variable_spirv_id);
				}
			}

			//
			// create the rasterizer state input variables
			HccDataType rasterizer_state_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, aml_function->values[HCC_VERTEX_SHADER_PARAM_RASTERIZER_STATE].data_type);
			HccDataType rasterizer_state_data_type = hcc_data_type_strip_pointer(cu, rasterizer_state_ptr_data_type);
			if (rasterizer_state_data_type != HCC_DATA_TYPE_CONST(HCC_DATA_TYPE_AML_INTRINSIC_VOID)) {
				HccCompoundDataType* rasterizer_state_compound_data_type = hcc_compound_data_type_get(cu, rasterizer_state_data_type);
				w->spirvgen.rasterizer_state_variable_base_spirv_id = hcc_spirv_next_id_many(cu, rasterizer_state_compound_data_type->fields_count);

				for (uint32_t field_idx = 0; field_idx < rasterizer_state_compound_data_type->fields_count; field_idx += 1) {
					HccCompoundField* field = &rasterizer_state_compound_data_type->fields[field_idx];
					HccDataType field_ptr_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_CONST(field->data_type)));
					HccSPIRVId field_variable_spirv_id = w->spirvgen.rasterizer_state_variable_base_spirv_id + field_idx;

					operands = hcc_spirv_add_global_variable(cu, 3);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INPUT, field_ptr_data_type);
					operands[1] = field_variable_spirv_id;
					operands[2] = HCC_SPIRV_STORAGE_CLASS_INPUT;

					operands = hcc_spirv_add_decorate(cu, 3);
					operands[0] = field_variable_spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_LOCATION;
					operands[2] = field_idx;

					if (field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP) {
						operands = hcc_spirv_add_decorate(cu, 2);
						operands[0] = field_variable_spirv_id;
						operands[1] = HCC_SPIRV_DECORATION_FLAT;
					}

					hcc_spirvgen_found_global(w, field_variable_spirv_id);
				}
			}
			break;
		};
		case HCC_SHADER_STAGE_COMPUTE:
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_IDX);
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_GROUP_IDX);
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_LOCAL_IDX);
			hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_LOCAL_FLAT_IDX);
			break;
		case HCC_SHADER_STAGE_MESH_TASK:
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

				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_function, aml_operands[1]);

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
				HccDataType return_data_type = HCC_DATA_TYPE_STRIP_QUALIFIERS(hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]));

				HccSPIRVId builtin_variable_spirv_id = HCC_SPIRV_ID_INVALID;
				HccSPIRVStorageClass spirv_storage_class;
				if (HCC_AML_OPERAND_TYPE(aml_operands[1]) == HCC_AML_OPERAND_VALUE && w->spirvgen.aml_function->shader_stage != HCC_SHADER_STAGE_NONE) {
					uint32_t variable_idx = HCC_AML_OPERAND_AUX(aml_operands[1]);
					switch (w->spirvgen.aml_function->shader_stage) {
						case HCC_SHADER_STAGE_NONE:
							break;
						case HCC_SHADER_STAGE_VERTEX:
							if (variable_idx < 4) {
								HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[2])));
								uint32_t field_idx = hcc_constant_read_32(constant);
								switch (variable_idx) {
									case HCC_VERTEX_SHADER_PARAM_VERTEX_SV:
										spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_INPUT;
										switch (field_idx) {
										case 0: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_VERTEX_IDX; break;
										case 1: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_INSTANCE_IDX; break;
										default: HCC_ABORT("unhandle vertex sv field");
										}
										break;
									case HCC_VERTEX_SHADER_PARAM_VERTEX_SV_OUT:
										spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_OUTPUT;
										switch (field_idx) {
										case 0: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_OUTPUT_POSITION; break;
										default: HCC_ABORT("unhandle vertex sv out field");
										}
										break;
									case HCC_VERTEX_SHADER_PARAM_BC:
										break;
									case HCC_VERTEX_SHADER_PARAM_RASTERIZER_STATE:
										spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_OUTPUT;
										builtin_variable_spirv_id = w->spirvgen.rasterizer_state_variable_base_spirv_id + field_idx;
										break;
								}
							}
							break;
						case HCC_SHADER_STAGE_PIXEL:
							if (variable_idx < 5) {
								HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[2])));
								uint32_t field_idx = hcc_constant_read_32(constant);
								switch (variable_idx) {
								case HCC_PIXEL_SHADER_PARAM_PIXEL_SV:
									spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_INPUT;
									switch (field_idx) {
									case 0: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_FRAG_COORD; break;
									default: HCC_ABORT("unhandle pixel sv field");
									}
									break;
								case HCC_PIXEL_SHADER_PARAM_PIXEL_SV_OUT:
									spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_OUTPUT;
									switch (field_idx) {
									case 0: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_OUTPUT_FRAG_DEPTH; break;
									default: HCC_ABORT("unhandle pixel sv out field");
									}
									break;
								case HCC_PIXEL_SHADER_PARAM_BC:
									break;
								case HCC_PIXEL_SHADER_PARAM_RASTERIZER_STATE:
									spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_INPUT;
									builtin_variable_spirv_id = w->spirvgen.rasterizer_state_variable_base_spirv_id + field_idx;
									break;
								case HCC_PIXEL_SHADER_PARAM_PIXEL_STATE:
									spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_OUTPUT;
									builtin_variable_spirv_id = w->spirvgen.pixel_state_variable_base_spirv_id + field_idx;
									break;
								}
							}
							break;
						case HCC_SHADER_STAGE_COMPUTE:
							if (variable_idx < 2) {
								HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[2])));
								uint32_t field_idx = hcc_constant_read_32(constant);
								switch (variable_idx) {
								case HCC_COMPUTE_SHADER_PARAM_COMPUTE_SV:
									spirv_storage_class = HCC_SPIRV_STORAGE_CLASS_INPUT;
									switch (field_idx) {
									case 0: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_IDX; break;
									case 1: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_GROUP_IDX; break;
									case 2: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_LOCAL_IDX; break;
									case 3: builtin_variable_spirv_id = HCC_SPIRV_ID_VARIABLE_INPUT_DISPATCH_LOCAL_FLAT_IDX; break;
									default: HCC_ABORT("unhandle pixel sv field");
									}
									break;
								case HCC_COMPUTE_SHADER_PARAM_BC:
									break;
								}
							}
							break;
						default: HCC_ABORT("unimplemented shader stage");
					}
				}

				if (builtin_variable_spirv_id != HCC_SPIRV_ID_INVALID) {
					if (aml_operands_count == 3) {
						w->spirvgen.value_map[HCC_AML_OPERAND_AUX(aml_operands[0])] = builtin_variable_spirv_id;
					} else {
						operands = hcc_spirv_function_add_instr(function, aml_op == HCC_AML_OP_PTR_ACCESS_CHAIN ? HCC_SPIRV_OP_ACCESS_CHAIN : HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, aml_operands_count);
						operands[0] = hcc_spirv_type_deduplicate(cu, spirv_storage_class, return_data_type);
						operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
						operands[2] = builtin_variable_spirv_id;
						for (uint32_t operand_idx = 3; operand_idx < aml_operands_count; operand_idx += 1) {
							operands[operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
						}
					}
				} else {
					HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_function, aml_operands[1]);
					operands = hcc_spirv_function_add_instr(function, aml_op == HCC_AML_OP_PTR_ACCESS_CHAIN ? HCC_SPIRV_OP_ACCESS_CHAIN : HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, aml_operands_count + 1);
					operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
					operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
					operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
					for (uint32_t operand_idx = 2; operand_idx < aml_operands_count; operand_idx += 1) {
						operands[operand_idx + 1] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
					}
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
				HccSPIRVOperand src_operand = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccSPIRVId cond_operand = hcc_spirvgen_convert_to_spirv_bool(w, function, src_operand, src_data_type);

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_BRANCH_CONDITIONAL, 3);
				operands[0] = cond_operand;
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
			case HCC_AML_OP_NEGATE:
			{
				HccDataType data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				if (HCC_DATA_TYPE_IS_RESOURCE(data_type)) {
					data_type = HCC_DATA_TYPE_AML_INTRINSIC_U32;
				}
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
					case HCC_AML_OP_NEGATE:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_S_NEGATE; break;
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
			case HCC_AML_OP_EQUAL:
			case HCC_AML_OP_NOT_EQUAL:
			case HCC_AML_OP_LESS_THAN:
			case HCC_AML_OP_LESS_THAN_OR_EQUAL:
			case HCC_AML_OP_GREATER_THAN:
			case HCC_AML_OP_GREATER_THAN_OR_EQUAL:
			{
				HccDataType data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				if (HCC_DATA_TYPE_IS_RESOURCE(data_type)) {
					data_type = HCC_DATA_TYPE_AML_INTRINSIC_U32;
				}
				HccBasicTypeClass type_class = hcc_basic_type_class(cu, data_type);

				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_EQUAL:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_BOOL: op = HCC_SPIRV_OP_I_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_I_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_I_EQUAL; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_F_UNORD_EQUAL; break;
						}
						break;
					case HCC_AML_OP_NOT_EQUAL:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_BOOL: op = HCC_SPIRV_OP_I_NOT_EQUAL; break;
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
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u' with type_class '%u'", aml_op, type_class);

				HccSPIRVId result_operand = hcc_spirv_next_id(cu);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				uint32_t columns = HCC_DATA_TYPE_IS_AML_INTRINSIC(return_data_type) ? HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(HCC_DATA_TYPE_AUX(return_data_type)) : 1;
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 1);
				operands[0] = HCC_SPIRV_ID_TYPE_BOOL + columns - 1;
				operands[1] = result_operand;
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[2]);

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECT, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = result_operand;
				operands[3] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_one(w->cu, return_data_type));
				operands[4] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, return_data_type));
				break;
			};
			case HCC_AML_OP_SELECT:
			{
				HccSPIRVOperand src_operand = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HccSPIRVId cond_operand = hcc_spirvgen_convert_to_spirv_bool(w, function, src_operand, src_data_type);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECT, aml_operands_count + 1);
				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_function, aml_operands[1]);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = cond_operand;
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				operands[4] = hcc_spirvgen_convert_operand(w, aml_operands[3]);
				break;
			};
			case HCC_AML_OP_ANY:
			case HCC_AML_OP_ALL:
			{
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_ANY: op = HCC_SPIRV_OP_ANY; break;
					case HCC_AML_OP_ALL: op = HCC_SPIRV_OP_ALL; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);

				HccSPIRVOperand src_operand = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HccSPIRVId converted_operand = hcc_spirvgen_convert_to_spirv_bool(w, function, src_operand, src_data_type);

				uint32_t columns = HCC_DATA_TYPE_IS_AML_INTRINSIC(return_data_type) ? HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(HCC_DATA_TYPE_AUX(return_data_type)) : 1;
				HccSPIRVId result_operand = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 1);
				operands[0] = HCC_SPIRV_ID_TYPE_BOOL + columns - 1;
				operands[1] = result_operand;
				operands[2] = converted_operand;

				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_function, aml_operands[1]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECT, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = result_operand;
				operands[3] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_one(w->cu, return_data_type));
				operands[4] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, return_data_type));
				break;
			};
			case HCC_AML_OP_ISINF:
			case HCC_AML_OP_ISNAN:
			{
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_ISINF: op = HCC_SPIRV_OP_ISINF; break;
					case HCC_AML_OP_ISNAN: op = HCC_SPIRV_OP_ISNAN; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);

				uint32_t columns = HCC_DATA_TYPE_IS_AML_INTRINSIC(return_data_type) ? HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(HCC_DATA_TYPE_AUX(return_data_type)) : 1;
				HccSPIRVId result_operand = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 1);
				operands[0] = HCC_SPIRV_ID_TYPE_BOOL + columns - 1;
				operands[1] = result_operand;
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);

				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_function, aml_operands[1]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECT, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = result_operand;
				operands[3] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_one(w->cu, return_data_type));
				operands[4] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, return_data_type));
				break;
			};
			case HCC_AML_OP_BIT_AND:
			case HCC_AML_OP_BIT_OR:
			case HCC_AML_OP_BIT_XOR:
			case HCC_AML_OP_BIT_SHIFT_LEFT:
			case HCC_AML_OP_DOT:
			case HCC_AML_OP_BITCAST:
			case HCC_AML_OP_DDX:
			case HCC_AML_OP_DDY:
			case HCC_AML_OP_FWIDTH:
			case HCC_AML_OP_DDX_FINE:
			case HCC_AML_OP_DDY_FINE:
			case HCC_AML_OP_FWIDTH_FINE:
			case HCC_AML_OP_DDX_COARSE:
			case HCC_AML_OP_DDY_COARSE:
			case HCC_AML_OP_FWIDTH_COARSE:
			case HCC_AML_OP_BITCOUNT:
			{
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_BIT_AND: op = HCC_SPIRV_OP_BITWISE_AND; break;
					case HCC_AML_OP_BIT_OR: op = HCC_SPIRV_OP_BITWISE_OR; break;
					case HCC_AML_OP_BIT_XOR: op = HCC_SPIRV_OP_BITWISE_XOR; break;
					case HCC_AML_OP_BIT_SHIFT_LEFT: op = HCC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL; break;
					case HCC_AML_OP_DOT: op = HCC_SPIRV_OP_DOT; break;
					case HCC_AML_OP_BITCAST: op = HCC_SPIRV_OP_BITCAST; break;
					case HCC_AML_OP_DDX: op = HCC_SPIRV_OP_DPDX; break;
					case HCC_AML_OP_DDY: op = HCC_SPIRV_OP_DPDY; break;
					case HCC_AML_OP_FWIDTH: op = HCC_SPIRV_OP_FWIDTH; break;
					case HCC_AML_OP_DDX_FINE: op = HCC_SPIRV_OP_DPDX_FINE; break;
					case HCC_AML_OP_DDY_FINE: op = HCC_SPIRV_OP_DPDY_FINE; break;
					case HCC_AML_OP_FWIDTH_FINE: op = HCC_SPIRV_OP_FWIDTH_FINE; break;
					case HCC_AML_OP_DDX_COARSE: op = HCC_SPIRV_OP_DPDX_COARSE; break;
					case HCC_AML_OP_DDY_COARSE: op = HCC_SPIRV_OP_DPDY_COARSE; break;
					case HCC_AML_OP_FWIDTH_COARSE: op = HCC_SPIRV_OP_FWIDTH_COARSE; break;
					case HCC_AML_OP_BITCOUNT: op = HCC_SPIRV_OP_BIT_COUNT; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 1);
				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_function, aml_operands[1]);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				for (uint32_t operand_idx = 0; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[1 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};
			case HCC_AML_OP_CONVERT: {
				HccDataType dst_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccSPIRVId result_type = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				HccSPIRVId result_operand = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId src_operand = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				if (HCC_DATA_TYPE_IS_RESOURCE(dst_data_type) || HCC_DATA_TYPE_IS_RESOURCE(src_data_type)) {
					op = HCC_SPIRV_OP_COPY_OBJECT;
				} else {
					HccBasicTypeClass dst_type_class = hcc_basic_type_class(cu, dst_data_type);
					HccBasicTypeClass src_type_class = hcc_basic_type_class(cu, src_data_type);
					switch (dst_type_class) {
						case HCC_BASIC_TYPE_CLASS_BOOL:
							switch (src_type_class) {
								case HCC_BASIC_TYPE_CLASS_BOOL:
									HCC_ABORT("unhandled conversion to from bool to bool, this should not happen");
									break;
								case HCC_BASIC_TYPE_CLASS_UINT:
								case HCC_BASIC_TYPE_CLASS_SINT: {
									HccSPIRVId dst_operand = hcc_spirvgen_convert_to_spirv_bool(w, function, src_operand, src_data_type);
									src_operand = dst_operand;
									op = HCC_SPIRV_OP_SELECT;
									break;
								};
								case HCC_BASIC_TYPE_CLASS_FLOAT: {
									HccSPIRVId dst_operand = hcc_spirv_next_id(cu);
									operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_F_UNORD_NOT_EQUAL, 4);
									operands[0] = HCC_SPIRV_ID_TYPE_BOOL;
									operands[1] = dst_operand;
									operands[2] = src_operand;
									operands[3] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, src_data_type));

									src_operand = dst_operand;
									op = HCC_SPIRV_OP_SELECT;
									break;
									break;
								};
							}
							break;
						case HCC_BASIC_TYPE_CLASS_UINT:
							switch (src_type_class) {
								case HCC_BASIC_TYPE_CLASS_UINT:
									op = HCC_SPIRV_OP_U_CONVERT;
									break;
								case HCC_BASIC_TYPE_CLASS_BOOL:
									if (HCC_DATA_TYPE_STRIP_QUALIFIERS(dst_data_type) == HCC_DATA_TYPE_AML_INTRINSIC_U32) {
										op = HCC_SPIRV_OP_BITCAST;
										break;
									}
									hcc_fallthrough;
								case HCC_BASIC_TYPE_CLASS_SINT: {
									HccDataType signed_dst_data_type = hcc_data_type_unsigned_to_signed(cu, dst_data_type);
									if (HCC_DATA_TYPE_STRIP_QUALIFIERS(signed_dst_data_type) != HCC_DATA_TYPE_STRIP_QUALIFIERS(src_data_type)) {
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
								case HCC_BASIC_TYPE_CLASS_UINT: {
									HccDataType unsigned_dst_data_type = hcc_data_type_signed_to_unsigned(cu, dst_data_type);
									if (HCC_DATA_TYPE_STRIP_QUALIFIERS(unsigned_dst_data_type) != HCC_DATA_TYPE_STRIP_QUALIFIERS(src_data_type)) {
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
								case HCC_BASIC_TYPE_CLASS_BOOL:
									if (HCC_DATA_TYPE_STRIP_QUALIFIERS(dst_data_type) == HCC_DATA_TYPE_AML_INTRINSIC_S32) {
										op = HCC_SPIRV_OP_COPY_OBJECT;
										break;
									}
									hcc_fallthrough;
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
								case HCC_BASIC_TYPE_CLASS_UINT:
									op = HCC_SPIRV_OP_CONVERT_U_TO_F;
									break;
								case HCC_BASIC_TYPE_CLASS_BOOL:
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
				}

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

			case HCC_AML_OP_SELECTION_MERGE: {
				HccAMLInstr* next_aml_instr = &aml_function->words[aml_word_idx + HCC_AML_INSTR_WORDS_COUNT(aml_instr)];
				HccAMLOperand* next_aml_operands = HCC_AML_INSTR_OPERANDS(next_aml_instr);
				HccAMLOp next_aml_op = HCC_AML_INSTR_OP(next_aml_instr);
				HccSPIRVId cond_operand;
				if (next_aml_op == HCC_AML_OP_BRANCH_CONDITIONAL) {
					cond_operand = hcc_spirv_next_id(cu);
					HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, next_aml_operands[0]);
					HccSPIRVOperand src_operand = hcc_spirvgen_convert_operand(w, next_aml_operands[0]);
					cond_operand = hcc_spirvgen_convert_to_spirv_bool(w, function, src_operand, src_data_type);
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECTION_MERGE, 2);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = HCC_SPIRV_SELECTION_CONTROL_NONE;

				if (next_aml_op == HCC_AML_OP_BRANCH_CONDITIONAL) {
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_BRANCH_CONDITIONAL, 3);
					operands[0] = cond_operand;
					operands[1] = hcc_spirvgen_convert_operand(w, next_aml_operands[1]);
					operands[2] = hcc_spirvgen_convert_operand(w, next_aml_operands[2]);

					aml_word_idx += HCC_AML_INSTR_WORDS_COUNT(aml_instr) + HCC_AML_INSTR_WORDS_COUNT(next_aml_instr);
					continue;
				}
				break;
			};

			case HCC_AML_OP_LOOP_MERGE:
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOOP_MERGE, 3);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[2] = HCC_SPIRV_LOOP_CONTROL_NONE;
				break;

			case HCC_AML_OP_RESOURCE_DESCRIPTOR_LOAD:
			case HCC_AML_OP_RESOURCE_DESCRIPTOR_ADDR: {
				HccDataType data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccSPIRVDescriptorBindingInfo info;
				hcc_spirv_resource_descriptor_binding_deduplicate(w->cu, data_type, &info);
				hcc_spirvgen_found_global(w, info.variable_spirv_id);
				HccSPIRVId descriptor_idx_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[1]);

				bool is_buffer = HCC_DATA_TYPE_IS_BUFFER(data_type);
				HccSPIRVId result_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId intermediate_result_id = is_buffer || aml_op == HCC_AML_OP_RESOURCE_DESCRIPTOR_ADDR ? result_id : hcc_spirv_next_id(cu);

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, 4 + is_buffer);
				operands[0] = info.data_type_ptr_spirv_id;
				operands[1] = intermediate_result_id;
				operands[2] = info.variable_spirv_id;
				operands[3] = descriptor_idx_spirv_id;
				if (is_buffer) {
					// because of GLSL and it's terrible design, storage buffers need to be a structure and we only have a single element which is a runtime array
					// this final operand will access that runtime array.
					operands[4] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32));
				} else if (aml_op == HCC_AML_OP_RESOURCE_DESCRIPTOR_LOAD) {
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3);
					operands[0] = info.data_type_spirv_id;
					operands[1] = result_id;
					operands[2] = intermediate_result_id;
				}

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
			case HCC_AML_OP_CROSS:
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
					case HCC_AML_OP_CROSS: op = HCC_SPIRV_GLSL_STD_450_OP_CROSS; break;
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

			case HCC_AML_OP_SHUFFLE: {
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_VECTOR_SHUFFLE, aml_operands_count + 1);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				for (uint32_t operand_idx = 3; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[1 + operand_idx] = aml_operands[operand_idx];
				}
				break;
			};

			case HCC_AML_OP_BITLSB:
			case HCC_AML_OP_BITMSB:
			{
				HccSPIRVGLSLSTD450Op op = HCC_SPIRV_GLSL_STD_450_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_BITLSB: op = HCC_SPIRV_GLSL_STD_450_OP_FIND_I_LSB; break;
					case HCC_AML_OP_BITMSB: op = HCC_SPIRV_GLSL_STD_450_OP_FIND_U_MSB; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_GLSL_STD_450_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType input_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_AML_INTRINSIC(input_data_type), "expected a intrinsic type here");
				HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(input_data_type);
				HccAMLIntrinsicDataType scalar_data_type = HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(intrinsic_data_type);

				HCC_ASSERT(scalar_data_type != HCC_AML_INTRINSIC_DATA_TYPE_U64, "64 bit bitlsb & bitmsb functions haven't been implemented yet!");
				uint16_t column_bits = intrinsic_data_type & HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS_MASK;
				HccDataType return_data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_U32 | column_bits);

				HccSPIRVId dst_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId other_dst_spirv_id = dst_spirv_id;
				HccSPIRVId src_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				if (scalar_data_type != HCC_AML_INTRINSIC_DATA_TYPE_U32) {
					HccSPIRVId result_id0 = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_U_CONVERT, 3);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
					operands[1] = result_id0;
					operands[2] = src_spirv_id;

					dst_spirv_id = hcc_spirv_next_id(cu);
					src_spirv_id = result_id0;
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_EXT_INST, aml_operands_count + 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = dst_spirv_id;
				operands[2] = HCC_SPIRV_ID_GLSL_STD_450;
				operands[3] = op;
				operands[4] = src_spirv_id;

				break;
			};

			case HCC_AML_OP_DISCARD_PIXEL:
				hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_DEMOTE_TO_HELPER_INVOCATION, 0);
				break;

			case HCC_AML_OP_QUAD_SWAP_X:
			case HCC_AML_OP_QUAD_SWAP_Y:
			case HCC_AML_OP_QUAD_SWAP_DIAGONAL:
			{
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_GROUP_NON_UNIFORM_QUAD_SWAP, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				switch (aml_op) {
					case HCC_AML_OP_QUAD_SWAP_X: operands[4] = cu->spirv.quad_swap_x_spirv_id; break;
					case HCC_AML_OP_QUAD_SWAP_Y: operands[4] = cu->spirv.quad_swap_y_spirv_id; break;
					case HCC_AML_OP_QUAD_SWAP_DIAGONAL: operands[4] = cu->spirv.quad_swap_diagonal_spirv_id; break;
				}
				break;
			};

			case HCC_AML_OP_QUAD_ANY:
			case HCC_AML_OP_QUAD_ALL:
			{
				HccSPIRVId bool_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_BOOL);
				HccSPIRVId input_v_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HccSPIRVOp combine_op = aml_op == HCC_AML_OP_QUAD_ANY ? HCC_SPIRV_OP_LOGICAL_OR : HCC_SPIRV_OP_LOGICAL_AND;

				// swap x
				HccSPIRVId result_id0 = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_GROUP_NON_UNIFORM_QUAD_SWAP, 5);
				operands[0] = bool_spirv_id;
				operands[1] = result_id0;
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				operands[3] = input_v_spirv_id;
				operands[4] = cu->spirv.quad_swap_x_spirv_id;

				// bit and/or swap x result
				HccSPIRVId result_id1 = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, combine_op, 4);
				operands[0] = bool_spirv_id;
				operands[1] = result_id1;
				operands[2] = input_v_spirv_id;
				operands[3] = result_id0;

				// swap y
				HccSPIRVId result_id2 = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_GROUP_NON_UNIFORM_QUAD_SWAP, 5);
				operands[0] = bool_spirv_id;
				operands[1] = result_id2;
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				operands[3] = result_id1;
				operands[4] = cu->spirv.quad_swap_y_spirv_id;

				// bit and/or swap x result
				operands = hcc_spirv_function_add_instr(function, combine_op, 4);
				operands[0] = bool_spirv_id;
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = result_id1;
				operands[3] = result_id2;
				break;
			};

			case HCC_AML_OP_WAVE_ACTIVE_ANY:
			case HCC_AML_OP_WAVE_ACTIVE_ALL: {
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_WAVE_ACTIVE_ANY: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_ANY; break;
					case HCC_AML_OP_WAVE_ACTIVE_ALL: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_ALL; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType src_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HccSPIRVOperand src_operand = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HccSPIRVId converted_operand = hcc_spirvgen_convert_to_spirv_bool(w, function, src_operand, src_data_type);

				HccSPIRVId result_operand = hcc_spirv_next_id(cu);
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 2);
				operands[0] = HCC_SPIRV_ID_TYPE_BOOL;
				operands[1] = result_operand;
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				operands[3] = converted_operand;

				HccSPIRVStorageClass storage_class = hcc_spirv_storage_class_from_aml_operand(cu, aml_function, aml_operands[1]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECT, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = result_operand;
				operands[3] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_one(w->cu, return_data_type));
				operands[4] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, return_data_type));
				break;
			};

			case HCC_AML_OP_QUAD_READ_THREAD:
			case HCC_AML_OP_WAVE_READ_THREAD:
			case HCC_AML_OP_WAVE_ACTIVE_ALL_EQUAL: {
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				switch (aml_op) {
					case HCC_AML_OP_QUAD_READ_THREAD: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_QUAD_BROADCAST; break;
					case HCC_AML_OP_WAVE_READ_THREAD: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_SHUFFLE; break;
					case HCC_AML_OP_WAVE_ACTIVE_ALL_EQUAL: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_ALL_EQUAL; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 2);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				for (uint32_t operand_idx = 1; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[2 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};

			case HCC_AML_OP_WAVE_ACTIVE_MIN:
			case HCC_AML_OP_WAVE_ACTIVE_MAX:
			case HCC_AML_OP_WAVE_ACTIVE_SUM:
			case HCC_AML_OP_WAVE_ACTIVE_PREFIX_SUM:
			case HCC_AML_OP_WAVE_ACTIVE_PRODUCT:
			case HCC_AML_OP_WAVE_ACTIVE_PREFIX_PRODUCT:
			case HCC_AML_OP_WAVE_ACTIVE_BIT_AND:
			case HCC_AML_OP_WAVE_ACTIVE_BIT_OR:
			case HCC_AML_OP_WAVE_ACTIVE_BIT_XOR: {
				HccDataType data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HccBasicTypeClass type_class = hcc_basic_type_class(cu, data_type);

				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				uint32_t group_op = HCC_SPIRV_GROUP_OPERATION_REDUCE;
				switch (aml_op) {
					case HCC_AML_OP_WAVE_ACTIVE_MIN:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_U_MIN; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_S_MIN; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_F_MIN; break;
						}
						break;
					case HCC_AML_OP_WAVE_ACTIVE_MAX:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_U_MAX; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_S_MAX; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_F_MAX; break;
						}
						break;
					case HCC_AML_OP_WAVE_ACTIVE_PREFIX_SUM:
						group_op = HCC_SPIRV_GROUP_OPERATION_EXCLUSIVE_SCAN;
						hcc_fallthrough;
					case HCC_AML_OP_WAVE_ACTIVE_SUM:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_I_ADD; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_I_ADD; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_F_ADD; break;
						}
						break;
					case HCC_AML_OP_WAVE_ACTIVE_PREFIX_PRODUCT:
						group_op = HCC_SPIRV_GROUP_OPERATION_EXCLUSIVE_SCAN;
						hcc_fallthrough;
					case HCC_AML_OP_WAVE_ACTIVE_PRODUCT:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_I_MUL; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_I_MUL; break;
							case HCC_BASIC_TYPE_CLASS_FLOAT: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_F_MUL; break;
						}
						break;
					case HCC_AML_OP_WAVE_ACTIVE_BIT_AND: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_BITWISE_AND; break;
					case HCC_AML_OP_WAVE_ACTIVE_BIT_OR: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_BITWISE_OR; break;
					case HCC_AML_OP_WAVE_ACTIVE_BIT_XOR: op = HCC_SPIRV_OP_GROUP_NON_UNIFORM_BITWISE_XOR; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				operands[3] = group_op;
				for (uint32_t operand_idx = 1; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[3 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};

			case HCC_AML_OP_WAVE_ACTIVE_COUNT_BITS:
			case HCC_AML_OP_WAVE_ACTIVE_PREFIX_COUNT_BITS: {
				HccDataType ballot_return_data_type = HCC_DATA_TYPE_AML_INTRINSIC_U32X4;
				HccSPIRVId ballot_result_id = hcc_spirv_next_id(cu);

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_GROUP_NON_UNIFORM_BALLOT, 4);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, ballot_return_data_type);
				operands[1] = ballot_result_id;
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[1]);

				uint32_t group_op = HCC_SPIRV_GROUP_OPERATION_REDUCE;
				switch (aml_op) {
					case HCC_AML_OP_WAVE_ACTIVE_PREFIX_COUNT_BITS: group_op = HCC_SPIRV_GROUP_OPERATION_EXCLUSIVE_SCAN; break;
				}

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_GROUP_NON_UNIFORM_BALLOT_BIT_COUNT, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = cu->spirv.scope_subgroup_spirv_id;
				operands[3] = group_op;
				operands[4] = ballot_result_id;
				break;
			};

			case HCC_AML_OP_MEMORY_BARRIER_RESOURCE:
			case HCC_AML_OP_MEMORY_BARRIER_DISPATCH_GROUP:
			case HCC_AML_OP_MEMORY_BARRIER_ALL: {
				HccSPIRVId memory_scope;
				HccSPIRVId memory_semantics;
				switch (aml_op) {
					case HCC_AML_OP_MEMORY_BARRIER_RESOURCE:
						memory_scope = cu->spirv.scope_device_spirv_id;
						memory_semantics = cu->spirv.memory_semantics_resource_spirv_id;
						break;
					case HCC_AML_OP_MEMORY_BARRIER_DISPATCH_GROUP:
						memory_scope = cu->spirv.scope_workgroup_spirv_id;
						memory_semantics = cu->spirv.memory_semantics_dispatch_spirv_id;
						break;
					case HCC_AML_OP_MEMORY_BARRIER_ALL:
						memory_scope = cu->spirv.scope_device_spirv_id;
						memory_semantics = cu->spirv.memory_semantics_all_spirv_id;
						break;
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_MEMORY_BARRIER, 2);
				operands[0] = memory_scope;
				operands[1] = memory_semantics;
				break;
			};

			case HCC_AML_OP_WAVE_THREAD_IS_FIRST: {
				HccSPIRVId execution_scope = cu->spirv.scope_subgroup_spirv_id;
				HccSPIRVId result_operand = hcc_spirv_next_id(cu);

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_GROUP_NON_UNIFORM_ELECT, 3);
				operands[0] = HCC_SPIRV_ID_TYPE_BOOL;
				operands[1] = result_operand;
				operands[2] = execution_scope;

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SELECT, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = result_operand;
				operands[3] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_one(w->cu, return_data_type));
				operands[4] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, return_data_type));
				break;
			};

			case HCC_AML_OP_WAVE_THREAD_IDX: {
				hcc_spirvgen_found_global(w, HCC_SPIRV_ID_VARIABLE_INPUT_SUBGROUP_LOCAL_INVOCATION_ID);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = HCC_SPIRV_ID_VARIABLE_INPUT_SUBGROUP_LOCAL_INVOCATION_ID;
				break;
			};

			case HCC_AML_OP_CONTROL_BARRIER_RESOURCE:
			case HCC_AML_OP_CONTROL_BARRIER_DISPATCH_GROUP:
			case HCC_AML_OP_CONTROL_BARRIER_ALL: {
				HccSPIRVId execution_scope = cu->spirv.scope_workgroup_spirv_id;
				HccSPIRVId memory_scope;
				HccSPIRVId memory_semantics;
				switch (aml_op) {
					case HCC_AML_OP_CONTROL_BARRIER_RESOURCE:
						memory_scope = cu->spirv.scope_device_spirv_id;
						memory_semantics = cu->spirv.memory_semantics_resource_spirv_id;
						break;
					case HCC_AML_OP_CONTROL_BARRIER_DISPATCH_GROUP:
						memory_scope = cu->spirv.scope_workgroup_spirv_id;
						memory_semantics = cu->spirv.memory_semantics_dispatch_spirv_id;
						break;
					case HCC_AML_OP_CONTROL_BARRIER_ALL:
						memory_scope = cu->spirv.scope_device_spirv_id;
						memory_semantics = cu->spirv.memory_semantics_all_spirv_id;
						break;
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_CONTROL_BARRIER, 3);
				operands[0] = execution_scope;
				operands[1] = memory_scope;
				operands[2] = memory_semantics;
				break;
			};

			case HCC_AML_OP_HPRINT_STRING: {
				HccSPIRVId buffer_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId u32_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32);
				HccSPIRVId u32_ptr_buffer_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32));
				HccSPIRVId base_idx_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HCC_DEBUG_ASSERT(HCC_AML_OPERAND_IS_CONSTANT(aml_operands[2]), "expected a string constant");
				HccSPIRVId string_spirv_id = hcc_spirv_constant_deduplicate(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[2])));

				uint32_t string_words_count = hcc_spirv_string_words_count(hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(aml_operands[2]))).size);
				for (uint32_t idx = 0; idx < string_words_count; idx += 1) {
					HccBasic src_idx_basic = { .u32 = idx };
					HccConstantId src_idx_constant_id = hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &src_idx_basic);
					HccSPIRVId src_idx_spirv_id = hcc_spirv_constant_deduplicate(cu, src_idx_constant_id);

					HccSPIRVId dst_idx_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_I_ADD, 4);
					operands[0] = u32_type_spirv_id;
					operands[1] = dst_idx_spirv_id;
					operands[2] = base_idx_spirv_id;
					operands[3] = src_idx_spirv_id;

					HccSPIRVId dst_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, 4);
					operands[0] = u32_ptr_buffer_type_spirv_id;
					operands[1] = dst_spirv_id;
					operands[2] = buffer_spirv_id;
					operands[3] = dst_idx_spirv_id;

					HccSPIRVId src_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_COMPOSITE_EXTRACT, 4);
					operands[0] = u32_type_spirv_id;
					operands[1] = src_spirv_id;
					operands[2] = string_spirv_id;
					operands[3] = idx;

					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_STORE, 2);
					operands[0] = dst_spirv_id;
					operands[1] = src_spirv_id;
				}

				break;
			};

			case HCC_AML_OP_ATOMIC_LOAD: {
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_ATOMIC_LOAD, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[3] = cu->spirv.scope_device_spirv_id;
				operands[4] = cu->spirv.memory_semantics_all_load_spirv_id;
				break;
			};

			case HCC_AML_OP_ATOMIC_STORE: {
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_ATOMIC_STORE, 4);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = cu->spirv.scope_device_spirv_id;
				operands[2] = cu->spirv.memory_semantics_all_store_spirv_id;
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				break;
			};

			case HCC_AML_OP_ATOMIC_COMPARE_EXCHANGE: {
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_ATOMIC_COMPARE_EXCHANGE, 8);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[3] = cu->spirv.scope_device_spirv_id;
				operands[4] = cu->spirv.memory_semantics_all_spirv_id;
				operands[5] = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_U32));
				operands[6] = hcc_spirvgen_convert_operand(w, aml_operands[3]);
				operands[7] = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				break;
			};

			case HCC_AML_OP_ATOMIC_EXCHANGE:
			case HCC_AML_OP_ATOMIC_ADD:
			case HCC_AML_OP_ATOMIC_SUB:
			case HCC_AML_OP_ATOMIC_MIN:
			case HCC_AML_OP_ATOMIC_MAX:
			case HCC_AML_OP_ATOMIC_BIT_AND:
			case HCC_AML_OP_ATOMIC_BIT_OR:
			case HCC_AML_OP_ATOMIC_BIT_XOR: {
				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				HccDataType data_type = hcc_data_type_strip_pointer(cu, hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]));
				HccBasicTypeClass type_class = hcc_basic_type_class(cu, data_type);

				switch (aml_op) {
					case HCC_AML_OP_ATOMIC_EXCHANGE: op = HCC_SPIRV_OP_ATOMIC_EXCHANGE; break;
					case HCC_AML_OP_ATOMIC_ADD: op = HCC_SPIRV_OP_ATOMIC_I_ADD; break;
					case HCC_AML_OP_ATOMIC_SUB: op = HCC_SPIRV_OP_ATOMIC_I_SUB; break;
					case HCC_AML_OP_ATOMIC_MIN:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_ATOMIC_U_MIN; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_ATOMIC_S_MIN; break;
						}
						break;
					case HCC_AML_OP_ATOMIC_MAX:
						switch (type_class) {
							case HCC_BASIC_TYPE_CLASS_UINT: op = HCC_SPIRV_OP_ATOMIC_U_MAX; break;
							case HCC_BASIC_TYPE_CLASS_SINT: op = HCC_SPIRV_OP_ATOMIC_S_MAX; break;
						}
						break;
					case HCC_AML_OP_ATOMIC_BIT_AND: op = HCC_SPIRV_OP_ATOMIC_AND; break;
					case HCC_AML_OP_ATOMIC_BIT_OR: op = HCC_SPIRV_OP_ATOMIC_OR; break;
					case HCC_AML_OP_ATOMIC_BIT_XOR: op = HCC_SPIRV_OP_ATOMIC_XOR; break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				operands = hcc_spirv_function_add_instr(function, op, aml_operands_count + 3);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[3] = cu->spirv.scope_device_spirv_id;
				operands[4] = cu->spirv.memory_semantics_all_spirv_id;
				for (uint32_t operand_idx = 2; operand_idx < aml_operands_count; operand_idx += 1) {
					operands[3 + operand_idx] = hcc_spirvgen_convert_operand(w, aml_operands[operand_idx]);
				}
				break;
			};

			case HCC_AML_OP_LOAD_TEXTURE:
			case HCC_AML_OP_FETCH_TEXTURE:
			case HCC_AML_OP_ADDR_TEXTURE:
			{
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType texture_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_RESOURCE_DESCRIPTOR(texture_data_type), "expected resource descriptor type");
				HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(texture_data_type);
				HccDataType intermediate_return_data_type = return_data_type;
				uint32_t num_components;
				if (aml_op != HCC_AML_OP_ADDR_TEXTURE) {
					HccAMLIntrinsicDataType sample_data_type = HCC_RESOURCE_DATA_TYPE_TEXTURE_INTRINSIC_TYPE(resource_data_type);
					num_components = HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(sample_data_type);
					sample_data_type = HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(sample_data_type);
					sample_data_type = HCC_AML_INTRINSIC_DATA_TYPE(sample_data_type, 4, 1);
					intermediate_return_data_type = HCC_DATA_TYPE(AML_INTRINSIC, sample_data_type);
				}

				HccSPIRVId index_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				HccSPIRVId ms_sample_spirv_id;
				if (HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type)) { // extract the u32x2 if Texture2D and u32x3 if Texture2DMS
					HccSPIRVId new_index_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_VECTOR_SHUFFLE, 6);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32X2);
					operands[1] = new_index_spirv_id;
					operands[2] = index_spirv_id;
					operands[3] = index_spirv_id;
					for (uint32_t idx = 0; idx < 2; idx += 1) {
						operands[4 + idx] = idx;
					}

					// extract the multi-sample sample index
					ms_sample_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_COMPOSITE_EXTRACT, 4);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32);
					operands[1] = ms_sample_spirv_id;
					operands[2] = index_spirv_id;
					operands[3] = 2;
				}

				bool needs_intermediate_value = intermediate_return_data_type != return_data_type;
				HccSPIRVId result_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId intermediate_result_id = needs_intermediate_value ? hcc_spirv_next_id(cu) : result_id;

				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				uint32_t extra_operands_count = HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type) * 2 + (aml_op == HCC_AML_OP_FETCH_TEXTURE) * 2;
				switch (aml_op) {
					case HCC_AML_OP_LOAD_TEXTURE:  op = HCC_SPIRV_OP_IMAGE_READ; break;
					case HCC_AML_OP_FETCH_TEXTURE: op = HCC_SPIRV_OP_IMAGE_FETCH; break;
					case HCC_AML_OP_ADDR_TEXTURE:  op = HCC_SPIRV_OP_TEXEL_POINTER; extra_operands_count = 1; break;
				}
				operands = hcc_spirv_function_add_instr(function, op, 4 + extra_operands_count);
				operands[0] = hcc_spirv_type_deduplicate(cu, aml_op == HCC_AML_OP_ADDR_TEXTURE ? HCC_SPIRV_STORAGE_CLASS_IMAGE : HCC_SPIRV_STORAGE_CLASS_INVALID, intermediate_return_data_type);
				operands[1] = intermediate_result_id;
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[3] = index_spirv_id;
				if (aml_op == HCC_AML_OP_ADDR_TEXTURE) {
					operands[4]
						= HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type)
						? ms_sample_spirv_id
						: hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_zero(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_U32))
						;
				} else {
					if (HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type)) {
						operands[4] = HCC_SPIRV_IMAGE_OPERAND_SAMPLE;
						operands[5] = ms_sample_spirv_id;
					}
					if (aml_op == HCC_AML_OP_FETCH_TEXTURE) {
						// it is okay to hardcode 4 and 5 since multisampled textures do not have mips
						operands[4] = HCC_SPIRV_IMAGE_OPERAND_LOD;
						operands[5] = hcc_spirvgen_convert_operand(w, aml_operands[3]);
					}
				}

				if (needs_intermediate_value) {
					if (num_components == 1) {
						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_COMPOSITE_EXTRACT, 4);
						operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
						operands[1] = result_id;
						operands[2] = intermediate_result_id;
						operands[3] = 0;
					} else {
						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_VECTOR_SHUFFLE, 4 + num_components);
						operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
						operands[1] = result_id;
						operands[2] = intermediate_result_id;
						operands[3] = intermediate_result_id;
						for (uint32_t idx = 0; idx < num_components; idx += 1) {
							operands[4 + idx] = idx;
						}
					}
				}

				break;
			};
			case HCC_AML_OP_SAMPLE_TEXTURE:
			case HCC_AML_OP_SAMPLE_MIP_BIAS_TEXTURE:
			case HCC_AML_OP_SAMPLE_MIP_GRADIENT_TEXTURE:
			case HCC_AML_OP_SAMPLE_MIP_LEVEL_TEXTURE:
			{
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType texture_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_RESOURCE_DESCRIPTOR(texture_data_type), "expected resource descriptor type");
				HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(texture_data_type);
				HccAMLIntrinsicDataType sample_data_type = HCC_RESOURCE_DATA_TYPE_TEXTURE_INTRINSIC_TYPE(resource_data_type);
				uint32_t num_components = HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(sample_data_type);
				sample_data_type = HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(sample_data_type);
				sample_data_type = HCC_AML_INTRINSIC_DATA_TYPE(sample_data_type, 4, 1);
				HccDataType intermediate_return_data_type = HCC_DATA_TYPE(AML_INTRINSIC, sample_data_type);

				HccSPIRVId sampled_image_spirv_id = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SAMPLED_IMAGE, 4);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, texture_data_type) + 1; // HCC_SPIRV_OP_TYPE_SAMPLED_IMAGE is created + 1 of it's HCC_SPIRV_OP_TYPE_IMAGE
				operands[1] = sampled_image_spirv_id;
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]); // texture
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[2]); // sampler

				bool needs_intermediate_value = intermediate_return_data_type != return_data_type;
				HccSPIRVId result_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId intermediate_result_id = needs_intermediate_value ? hcc_spirv_next_id(cu) : result_id;

				HccSPIRVOp op = HCC_SPIRV_OP_NO_OP;
				uint32_t extra_args = 0;
				switch(aml_op) {
					case HCC_AML_OP_SAMPLE_TEXTURE:
						op = HCC_SPIRV_OP_IMAGE_SAMPLE_IMPLICIT_LOD;
						break;
					case HCC_AML_OP_SAMPLE_MIP_BIAS_TEXTURE:
						op = HCC_SPIRV_OP_IMAGE_SAMPLE_IMPLICIT_LOD;
						extra_args = 2;
						break;
					case HCC_AML_OP_SAMPLE_MIP_GRADIENT_TEXTURE:
						op = HCC_SPIRV_OP_IMAGE_SAMPLE_EXPLICIT_LOD;
						extra_args = 3;
						break;
					case HCC_AML_OP_SAMPLE_MIP_LEVEL_TEXTURE:
						op = HCC_SPIRV_OP_IMAGE_SAMPLE_EXPLICIT_LOD;
						extra_args = 2;
						break;
				}
				HCC_DEBUG_ASSERT(op != HCC_SPIRV_OP_NO_OP, "unhandled conversion to SPIR-V from AML for aml_op '%u'", aml_op);

				operands = hcc_spirv_function_add_instr(function, op, 4 + extra_args);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, intermediate_return_data_type);
				operands[1] = intermediate_result_id;
				operands[2] = sampled_image_spirv_id;
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[3]); // coordinate
				switch(aml_op) {
					case HCC_AML_OP_SAMPLE_TEXTURE:
						break;
					case HCC_AML_OP_SAMPLE_MIP_BIAS_TEXTURE:
						operands[4] = HCC_SPIRV_IMAGE_OPERAND_BIAS;
						operands[5] = hcc_spirvgen_convert_operand(w, aml_operands[4]); // mip bias
						break;
					case HCC_AML_OP_SAMPLE_MIP_GRADIENT_TEXTURE:
						operands[4] = HCC_SPIRV_IMAGE_OPERAND_GRAD;
						operands[5] = hcc_spirvgen_convert_operand(w, aml_operands[4]); // ddx
						operands[6] = hcc_spirvgen_convert_operand(w, aml_operands[5]); // ddy
						break;
					case HCC_AML_OP_SAMPLE_MIP_LEVEL_TEXTURE:
						operands[4] = HCC_SPIRV_IMAGE_OPERAND_LOD;
						operands[5] = hcc_spirvgen_convert_operand(w, aml_operands[4]); // mip_level
						break;
				}

				if (needs_intermediate_value) {
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_VECTOR_SHUFFLE, 4 + num_components);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
					operands[1] = result_id;
					operands[2] = intermediate_result_id;
					operands[3] = intermediate_result_id;
					for (uint32_t idx = 0; idx < num_components; idx += 1) {
						operands[4 + idx] = idx;
					}
				}

				break;
			};
			case HCC_AML_OP_GATHER_RED_TEXTURE:
			case HCC_AML_OP_GATHER_GREEN_TEXTURE:
			case HCC_AML_OP_GATHER_BLUE_TEXTURE:
			case HCC_AML_OP_GATHER_ALPHA_TEXTURE:
			{
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HccDataType texture_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[1]);
				HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_RESOURCE_DESCRIPTOR(texture_data_type), "expected resource descriptor type");

				HccSPIRVId sampled_image_spirv_id = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_SAMPLED_IMAGE, 4);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, texture_data_type) + 1; // HCC_SPIRV_OP_TYPE_SAMPLED_IMAGE is created + 1 of it's HCC_SPIRV_OP_TYPE_IMAGE
				operands[1] = sampled_image_spirv_id;
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]); // texture
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[2]); // sampler

				HccBasic basic = hcc_basic_from_uint(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, aml_op - HCC_AML_OP_GATHER_RED_TEXTURE);
				HccSPIRVId component_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IMAGE_GATHER, 5);
				operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[2] = sampled_image_spirv_id;
				operands[3] = hcc_spirvgen_convert_operand(w, aml_operands[3]); // coordinate
				operands[4] = component_spirv_id;

				break;
			};
			case HCC_AML_OP_STORE_TEXTURE:
			{
				HccDataType texture_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_RESOURCE_DESCRIPTOR(texture_data_type), "expected resource descriptor type");
				HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(texture_data_type);

				HccSPIRVId index_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				HccSPIRVId ms_sample_spirv_id;
				if (HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type)) { // extract the u32x2 if Texture2D and u32x3 if Texture2DArray
					HccSPIRVId new_index_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_VECTOR_SHUFFLE, 6);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32X2);
					operands[1] = new_index_spirv_id;
					operands[2] = index_spirv_id;
					operands[3] = index_spirv_id;
					for (uint32_t idx = 0; idx < 2; idx += 1) {
						operands[4 + idx] = idx;
					}

					// extract the multi-sample sample index
					ms_sample_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_COMPOSITE_EXTRACT, 4);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32);
					operands[1] = ms_sample_spirv_id;
					operands[2] = index_spirv_id;
					operands[3] = 2;
				}

				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IMAGE_WRITE, 3 + HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type) * 2);
				operands[0] = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				operands[1] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				if (HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type)) {
					operands[3] = HCC_SPIRV_IMAGE_OPERAND_SAMPLE;
					operands[4] = ms_sample_spirv_id;
				}

				break;
			};

			case HCC_AML_OP_LOAD_BYTE_BUFFER: {
				HccDataType return_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[0]);
				HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_AML_INTRINSIC(return_data_type), "expected aml intrinsic type");
				HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(return_data_type);
				uint32_t scalars_count = HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(intrinsic_data_type);

				HccBasic four_basic = { .u32 = 4 };
				HccConstantId four_constant_id = hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &four_basic);

				HccSPIRVId src_byte_buffer_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				HccSPIRVId word_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32);
				HccSPIRVId ptr_word_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32));

				HccSPIRVId word_idx_spirv_id = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_U_DIV, 4);
				operands[0] = word_type_spirv_id;
				operands[1] = word_idx_spirv_id;
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				operands[3] = hcc_spirv_constant_deduplicate(w->cu, four_constant_id);

				HccSPIRVId dst_intermediate_spirv_id;
				HccSPIRVId dst_intermediate_type_spirv_id;
				HccSPIRVId dst_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, return_data_type);
				HccSPIRVId dst_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				if (HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(intrinsic_data_type) == HCC_DATA_TYPE_AML_INTRINSIC_U32) {
					dst_intermediate_spirv_id = dst_spirv_id;
					dst_intermediate_type_spirv_id = dst_type_spirv_id;
				} else {
					dst_intermediate_spirv_id = hcc_spirv_next_id(cu);
					dst_intermediate_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE(HCC_AML_INTRINSIC_DATA_TYPE_U32, scalars_count, 1)));
				}

				if (scalars_count == 1) {
					HccSPIRVId ptr_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, 4);
					operands[0] = ptr_word_type_spirv_id;
					operands[1] = ptr_spirv_id;
					operands[2] = src_byte_buffer_spirv_id;
					operands[3] = word_idx_spirv_id;

					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3);
					operands[0] = word_type_spirv_id;
					operands[1] = dst_intermediate_spirv_id;
					operands[2] = ptr_spirv_id;
				} else {
					HccSPIRVId loaded_spirv_id[4];

					HccBasic two_basic = { .u32 = 2 };
					HccConstantId two_constant_id = hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &two_basic);

					HccBasic three_basic = { .u32 = 3 };
					HccConstantId three_constant_id = hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &three_basic);

					HccConstantId add_constant_ids[4] = {
						hcc_constant_table_deduplicate_zero(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32),
						hcc_constant_table_deduplicate_one(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32),
						two_constant_id,
						three_constant_id,
					};

					for (uint32_t idx = 0; idx < scalars_count; idx += 1) {
						loaded_spirv_id[idx] = hcc_spirv_next_id(cu);

						HccSPIRVId added_word_idx_spirv_id = hcc_spirv_next_id(cu);
						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_I_ADD, 4);
						operands[0] = word_type_spirv_id;
						operands[1] = added_word_idx_spirv_id;
						operands[2] = word_idx_spirv_id;
						operands[3] = hcc_spirv_constant_deduplicate(w->cu, add_constant_ids[idx]);

						HccSPIRVId ptr_spirv_id = hcc_spirv_next_id(cu);
						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, 4);
						operands[0] = ptr_word_type_spirv_id;
						operands[1] = ptr_spirv_id;
						operands[2] = src_byte_buffer_spirv_id;
						operands[3] = added_word_idx_spirv_id;

						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_LOAD, 3);
						operands[0] = word_type_spirv_id;
						operands[1] = loaded_spirv_id[idx];
						operands[2] = ptr_spirv_id;
					}

					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_COMPOSITE_CONSTRUCT, 2 + scalars_count);
					operands[0] = dst_intermediate_type_spirv_id;
					operands[1] = dst_intermediate_spirv_id;
					for (uint32_t idx = 0; idx < scalars_count; idx += 1) {
						operands[2 + idx] = loaded_spirv_id[idx];
					}
				}

				if (dst_spirv_id != dst_intermediate_spirv_id) {
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_BITCAST, 3);
					operands[0] = dst_type_spirv_id;
					operands[1] = dst_spirv_id;
					operands[2] = dst_intermediate_spirv_id;
				}

				break;
			};

			case HCC_AML_OP_STORE_BYTE_BUFFER: {
				HccDataType store_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[2]);
				HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_AML_INTRINSIC(store_data_type), "expected aml intrinsic type");
				HccAMLIntrinsicDataType intrinsic_data_type = HCC_DATA_TYPE_AUX(store_data_type);
				uint32_t scalars_count = HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(intrinsic_data_type);

				HccBasic four_basic = { .u32 = 4 };
				HccConstantId four_constant_id = hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &four_basic);

				HccSPIRVId dst_byte_buffer_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[0]);
				HccSPIRVId word_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32);
				HccSPIRVId ptr_word_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32));

				HccSPIRVId word_idx_spirv_id = hcc_spirv_next_id(cu);
				operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_U_DIV, 4);
				operands[0] = word_type_spirv_id;
				operands[1] = word_idx_spirv_id;
				operands[2] = hcc_spirvgen_convert_operand(w, aml_operands[1]);
				operands[3] = hcc_spirv_constant_deduplicate(w->cu, four_constant_id);

				HccSPIRVId src_value_spirv_id = hcc_spirvgen_convert_operand(w, aml_operands[2]);
				if (HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(intrinsic_data_type) != HCC_DATA_TYPE_AML_INTRINSIC_U32) {
					HccSPIRVId value_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_BITCAST, 3);
					operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE(HCC_AML_INTRINSIC_DATA_TYPE_U32, scalars_count, 1)));
					operands[1] = value_spirv_id;
					operands[2] = src_value_spirv_id;

					src_value_spirv_id = value_spirv_id;
				}

				if (scalars_count == 1) {
					HccSPIRVId ptr_spirv_id = hcc_spirv_next_id(cu);
					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, 4);
					operands[0] = ptr_word_type_spirv_id;
					operands[1] = ptr_spirv_id;
					operands[2] = dst_byte_buffer_spirv_id;
					operands[3] = word_idx_spirv_id;

					operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_STORE, 2);
					operands[0] = ptr_spirv_id;
					operands[1] = src_value_spirv_id;
				} else {
					HccBasic two_basic = { .u32 = 2 };
					HccConstantId two_constant_id = hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &two_basic);

					HccBasic three_basic = { .u32 = 3 };
					HccConstantId three_constant_id = hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &three_basic);

					HccConstantId add_constant_ids[4] = {
						hcc_constant_table_deduplicate_zero(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32),
						hcc_constant_table_deduplicate_one(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32),
						two_constant_id,
						three_constant_id,
					};

					for (uint32_t idx = 0; idx < scalars_count; idx += 1) {
						HccSPIRVId added_word_idx_spirv_id = hcc_spirv_next_id(cu);
						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_I_ADD, 4);
						operands[0] = word_type_spirv_id;
						operands[1] = added_word_idx_spirv_id;
						operands[2] = word_idx_spirv_id;
						operands[3] = hcc_spirv_constant_deduplicate(w->cu, add_constant_ids[idx]);

						HccSPIRVId ptr_spirv_id = hcc_spirv_next_id(cu);
						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_IN_BOUNDS_ACCESS_CHAIN, 4);
						operands[0] = ptr_word_type_spirv_id;
						operands[1] = ptr_spirv_id;
						operands[2] = dst_byte_buffer_spirv_id;
						operands[3] = added_word_idx_spirv_id;

						HccSPIRVId field_value_spirv_id = hcc_spirv_next_id(cu);
						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_COMPOSITE_EXTRACT, 4);
						operands[0] = word_type_spirv_id;
						operands[1] = field_value_spirv_id;
						operands[2] = src_value_spirv_id;
						operands[3] = idx;

						operands = hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_STORE, 2);
						operands[0] = ptr_spirv_id;
						operands[1] = field_value_spirv_id;
					}
				}

				break;
			};

			default: HCC_ABORT("unhandled AML OP: %u '%s'", aml_op, hcc_aml_op_code_strings[aml_op]);
		}

		aml_word_idx += HCC_AML_INSTR_WORDS_COUNT(aml_instr);
	}

	hcc_spirv_function_add_instr(function, HCC_SPIRV_OP_FUNCTION_END, 0);

	function->global_variable_ids = hcc_stack_push_many_thread_safe(cu->spirv.entry_point_global_variable_ids, w->spirvgen.function_unique_globals_count);
	function->global_variables_count = w->spirvgen.function_unique_globals_count;
	HCC_COPY_ELMT_MANY(function->global_variable_ids, w->spirvgen.function_unique_globals, w->spirvgen.function_unique_globals_count);

	if (aml_function->shader_stage != HCC_SHADER_STAGE_NONE) {
		HccSPIRVEntryPoint* ep = hcc_stack_push_thread_safe(cu->spirv.entry_points);
		ep->shader_stage = aml_function->shader_stage;
		ep->identifier_string_id = aml_function->identifier_string_id;
		ep->spirv_id = function_spirv_id;
		ep->function_decl = function_decl;
	}
}

