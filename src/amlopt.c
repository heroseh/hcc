
#include "hcc_internal.h"

HccAMLOptFn hcc_aml_opts_phase_0_level_0[] = {
	hcc_amlopt_make_call_graph,
};

HccAMLOptFn hcc_aml_opts_phase_1_level_0[] = {
	hcc_amlopt_check_for_recursion_and_make_ordered_function_list,
};

HccAMLOptFn hcc_aml_opts_phase_2_level_0[] = {
	hcc_amlopt_check_for_unsupported_features,
};

HccAMLOptFn hcc_aml_opts_phase_0_level_1[] = {
	hcc_amlopt_make_call_graph,
};

HccAMLOptFn hcc_aml_opts_phase_1_level_1[] = {
	hcc_amlopt_check_for_recursion_and_make_ordered_function_list,
};

HccAMLOptFn hcc_aml_opts_phase_2_level_1[] = {
	hcc_amlopt_check_for_unsupported_features,
};

HccAMLOptFn hcc_aml_opts_phase_0_level_2[] = {
	hcc_amlopt_make_call_graph,
};

HccAMLOptFn hcc_aml_opts_phase_1_level_2[] = {
	hcc_amlopt_check_for_recursion_and_make_ordered_function_list,
};

HccAMLOptFn hcc_aml_opts_phase_2_level_2[] = {
	hcc_amlopt_check_for_unsupported_features,
};

HccAMLOptFn hcc_aml_opts_phase_0_level_3[] = {
	hcc_amlopt_make_call_graph,
};

HccAMLOptFn hcc_aml_opts_phase_1_level_3[] = {
	hcc_amlopt_check_for_recursion_and_make_ordered_function_list,
};

HccAMLOptFn hcc_aml_opts_phase_2_level_3[] = {
	hcc_amlopt_check_for_unsupported_features,
};

HccAMLOptFn hcc_aml_opts_phase_0_level_s[] = {
	hcc_amlopt_make_call_graph,
};

HccAMLOptFn hcc_aml_opts_phase_1_level_s[] = {
	hcc_amlopt_check_for_recursion_and_make_ordered_function_list,
};

HccAMLOptFn hcc_aml_opts_phase_2_level_s[] = {
	hcc_amlopt_check_for_unsupported_features,
};

HccAMLOptFn hcc_aml_opts_phase_0_level_g[] = {
	hcc_amlopt_make_call_graph,
};

HccAMLOptFn hcc_aml_opts_phase_1_level_g[] = {
	hcc_amlopt_check_for_recursion_and_make_ordered_function_list,
};

HccAMLOptFn hcc_aml_opts_phase_2_level_g[] = {
	hcc_amlopt_check_for_unsupported_features,
};

HccAMLOptFn* hcc_aml_opts[HCC_AML_OPT_PHASE_COUNT][HCC_OPT_LEVEL_COUNT] = {
	[HCC_AML_OPT_PHASE_0] = {
		[HCC_OPT_LEVEL_0] = hcc_aml_opts_phase_0_level_0,
		[HCC_OPT_LEVEL_1] = hcc_aml_opts_phase_0_level_1,
		[HCC_OPT_LEVEL_2] = hcc_aml_opts_phase_0_level_2,
		[HCC_OPT_LEVEL_3] = hcc_aml_opts_phase_0_level_3,
		[HCC_OPT_LEVEL_S] = hcc_aml_opts_phase_0_level_s,
		[HCC_OPT_LEVEL_G] = hcc_aml_opts_phase_0_level_g,
	},
	[HCC_AML_OPT_PHASE_1] = {
		[HCC_OPT_LEVEL_0] = hcc_aml_opts_phase_1_level_0,
		[HCC_OPT_LEVEL_1] = hcc_aml_opts_phase_1_level_1,
		[HCC_OPT_LEVEL_2] = hcc_aml_opts_phase_1_level_2,
		[HCC_OPT_LEVEL_3] = hcc_aml_opts_phase_1_level_3,
		[HCC_OPT_LEVEL_S] = hcc_aml_opts_phase_1_level_s,
		[HCC_OPT_LEVEL_G] = hcc_aml_opts_phase_1_level_g,
	},
	[HCC_AML_OPT_PHASE_2] = {
		[HCC_OPT_LEVEL_0] = hcc_aml_opts_phase_2_level_0,
		[HCC_OPT_LEVEL_1] = hcc_aml_opts_phase_2_level_1,
		[HCC_OPT_LEVEL_2] = hcc_aml_opts_phase_2_level_2,
		[HCC_OPT_LEVEL_3] = hcc_aml_opts_phase_2_level_3,
		[HCC_OPT_LEVEL_S] = hcc_aml_opts_phase_2_level_s,
		[HCC_OPT_LEVEL_G] = hcc_aml_opts_phase_2_level_g,
	},
};

uint32_t hcc_aml_opts_count[HCC_AML_OPT_PHASE_COUNT][HCC_OPT_LEVEL_COUNT] = {
	[HCC_AML_OPT_PHASE_0] = {
		[HCC_OPT_LEVEL_0] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_0_level_0),
		[HCC_OPT_LEVEL_1] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_0_level_1),
		[HCC_OPT_LEVEL_2] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_0_level_2),
		[HCC_OPT_LEVEL_3] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_0_level_3),
		[HCC_OPT_LEVEL_S] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_0_level_s),
		[HCC_OPT_LEVEL_G] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_0_level_g),
	},
	[HCC_AML_OPT_PHASE_1] = {
		[HCC_OPT_LEVEL_0] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_1_level_0),
		[HCC_OPT_LEVEL_1] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_1_level_1),
		[HCC_OPT_LEVEL_2] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_1_level_2),
		[HCC_OPT_LEVEL_3] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_1_level_3),
		[HCC_OPT_LEVEL_S] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_1_level_s),
		[HCC_OPT_LEVEL_G] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_1_level_g),
	},
	[HCC_AML_OPT_PHASE_2] = {
		[HCC_OPT_LEVEL_0] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_2_level_0),
		[HCC_OPT_LEVEL_1] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_2_level_1),
		[HCC_OPT_LEVEL_2] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_2_level_2),
		[HCC_OPT_LEVEL_3] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_2_level_3),
		[HCC_OPT_LEVEL_S] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_2_level_s),
		[HCC_OPT_LEVEL_G] = HCC_ARRAY_COUNT(hcc_aml_opts_phase_2_level_g),
	},
};

void hcc_amlopt_init(HccWorker* w, HccCompilerSetup* setup) {
	HCC_UNUSED(w);
	HCC_UNUSED(setup);
}

void hcc_amlopt_deinit(HccWorker* w) {
	HCC_UNUSED(w);
}

void hcc_amlopt_reset(HccWorker* w) {
	HCC_UNUSED(w);
}

void hcc_amlopt_error_1(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_amlopt_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, other_location, va_args);
	va_end(va_args);
}

bool hcc_amlopt_check_for_recursion_and_make_ordered_function_list_(HccWorker* w, HccDecl function_decl, HccShaderStage used_in_shader_stage) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_FUNCTION(function_decl), "internal error: expected a function declaration");
	HCC_DEBUG_ASSERT(!HCC_DECL_IS_FORWARD_DECL(function_decl), "internal error: expected a function declaration that is not a forward declaration");
	HccCU* cu = w->cu;

	// append function decl
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(w->amlopt.function_recursion_call_stack_count, HCC_FUNCTION_CALL_STACK_CAP);
	w->amlopt.function_recursion_call_stack[w->amlopt.function_recursion_call_stack_count] = function_decl;
	w->amlopt.function_recursion_call_stack_count += 1;

	//
	// check for recursion
	const HccAMLFunction* aml_function = hcc_aml_function_get(cu, function_decl);
	for (uint32_t idx = 0; idx < w->amlopt.function_recursion_call_stack_count - 1; idx += 1) {
		if (w->amlopt.function_recursion_call_stack[idx] == function_decl) {
			const char* callstack = hcc_ast_function_callstack_string(cu, w->amlopt.function_recursion_call_stack, w->amlopt.function_recursion_call_stack_count);
			HccString identifier_string = hcc_string_table_get(aml_function->identifier_string_id);
			hcc_amlopt_error_1(w, HCC_ERROR_CODE_FUNCTION_RECURSION, aml_function->identifier_location, (int)identifier_string.size, identifier_string.data, callstack);
			return false;
		}
	}

	HccAMLCallNode* node = *hcc_stack_get(cu->aml.function_call_node_lists, HCC_DECL_AUX(function_decl));
	while (node) {
		//
		// descend down the call stack
		if (!hcc_amlopt_check_for_recursion_and_make_ordered_function_list_(w, node->function_decl, used_in_shader_stage)) {
			return false;
		}

		node = node->next_call_node_idx == UINT32_MAX ? NULL : hcc_stack_get(cu->aml.call_graph_nodes, node->next_call_node_idx);
	}

	for (uint32_t aml_word_idx = 0; aml_word_idx < aml_function->words_count; ) {
		HccAMLInstr* aml_instr = &aml_function->words[aml_word_idx];
		HccAMLOp aml_op = HCC_AML_INSTR_OP(aml_instr);
		HccAMLOperand* aml_operands = HCC_AML_INSTR_OPERANDS(aml_instr);
		uint32_t aml_operands_count = HCC_AML_INSTR_OPERANDS_COUNT(aml_instr);

		if (used_in_shader_stage != HCC_SHADER_STAGE_PIXEL) {
			switch (aml_op) {
				case HCC_AML_OP_SAMPLE_TEXTURE:
				case HCC_AML_OP_SAMPLE_MIP_BIAS_TEXTURE:
				{
					const char* callstack = hcc_ast_function_callstack_string(cu, w->amlopt.function_recursion_call_stack, w->amlopt.function_recursion_call_stack_count);
					HccString identifier_string = hcc_string_table_get(aml_function->identifier_string_id);
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_SAMPLE_TEXTURE_WITH_IMPLICIT_MIP_OUTSIDE_OF_PIXEL_SHADER, hcc_aml_instr_location(cu, aml_instr), (int)identifier_string.size, identifier_string.data, callstack);
					break;
				};
				case HCC_AML_OP_DISCARD_PIXEL:
				case HCC_AML_OP_DDX:
				case HCC_AML_OP_DDY:
				case HCC_AML_OP_FWIDTH:
				case HCC_AML_OP_DDX_FINE:
				case HCC_AML_OP_DDY_FINE:
				case HCC_AML_OP_FWIDTH_FINE:
				case HCC_AML_OP_DDX_COARSE:
				case HCC_AML_OP_DDY_COARSE:
				case HCC_AML_OP_FWIDTH_COARSE:
				{
					const char* callstack = hcc_ast_function_callstack_string(cu, w->amlopt.function_recursion_call_stack, w->amlopt.function_recursion_call_stack_count);
					HccString identifier_string = hcc_string_table_get(aml_function->identifier_string_id);
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_FUNCTION_CANNOT_BE_USED_OUTSIDE_OF_A_PIXEL_SHADER, hcc_aml_instr_location(cu, aml_instr), (int)identifier_string.size, identifier_string.data, callstack);
					break;
				};
			}
		}

		if (used_in_shader_stage != HCC_SHADER_STAGE_COMPUTE) {
			for (uint32_t operand_idx = 0; operand_idx < aml_operands_count; operand_idx += 1) {
				if (HCC_AML_OPERAND_TYPE(aml_operands[operand_idx]) == HCC_DECL_GLOBAL_VARIABLE){
					HccASTVariable* variable = hcc_ast_global_variable_get(w->cu, aml_operands[operand_idx]);
					if (aml_function->shader_stage != HCC_SHADER_STAGE_COMPUTE && variable->storage_duration == HCC_AST_STORAGE_DURATION_DISPATCH_GROUP) {
						HccLocation* instr_location = hcc_aml_instr_location(cu, aml_instr);
						const char* callstack = hcc_ast_function_callstack_string(cu, w->amlopt.function_recursion_call_stack, w->amlopt.function_recursion_call_stack_count);
						hcc_amlopt_error_1(w, HCC_ERROR_CODE_DISPATCH_GROUP_ONLY_FOR_COMPUTE, instr_location, callstack);
					}
				}
			}
		}

CONTINUE:
		aml_word_idx += HCC_AML_INSTR_WORDS_COUNT(aml_instr);
	}

	//
	// deduplicate append the function to the optimize_functions array
	// in an ordered way from the most deepest calls to the entry points
	uint32_t idx = 0;
	HccStack(HccDecl) optimize_functions = hcc_aml_optimize_functions(cu);
	while (1) {
		uint32_t count = hcc_stack_count(optimize_functions);
		for (; idx < count; idx += 1) {
			if (optimize_functions[idx] == function_decl) {
				goto END;
			}
		}
		hcc_spin_mutex_lock(&cu->aml.optimize_functions_mutex);
		if (count == hcc_stack_count(optimize_functions)) {
			*hcc_stack_push(optimize_functions) = function_decl;
		}
		hcc_spin_mutex_unlock(&cu->aml.optimize_functions_mutex);
	}

END:{}
	// pop function decl
	w->amlopt.function_recursion_call_stack_count -= 1;
	return true;
}

bool hcc_amlopt_ensure_supported_type(HccWorker* w, HccDataType data_type, HccLocation* location) {
	HccCU* cu = w->cu;
	if (HCC_DATA_TYPE_IS_POINTER(data_type)) {
		HccDataType element_data_type = hcc_pointer_data_type_get(cu, data_type)->element_data_type;
		if (HCC_DATA_TYPE_IS_UNION(element_data_type) && !hcc_options_get_bool(cu->options, HCC_OPTION_KEY_PHYSICAL_POINTER_ENABLED)) {
			HccString data_type_string = hcc_data_type_string(cu, element_data_type);
			hcc_amlopt_error_1(w, HCC_ERROR_CODE_UNION_ONLY_ALLOW_WITH_PHYSICAL_POINTERS, location, (int)data_type_string.size, data_type_string.data);
			return true;
		}
	}

	HccAMLScalarDataTypeMask mask = hcc_data_type_scalar_data_types_mask(cu, data_type) & ~cu->supported_scalar_data_types_mask;
	if (mask == 0) {
		return false;
	}

	HccLocation* data_type_location = hcc_data_type_location(cu, data_type);
	HccString data_type_string = hcc_data_type_string(cu, data_type);
	HccString mask_string = hcc_aml_scalar_data_type_mask_string(mask);
	hcc_amlopt_error_2(w, HCC_ERROR_CODE_UNSUPPORTED_INTRINSIC_TYPE_USED, location, data_type_location, (int)data_type_string.size, data_type_string.data, (int)mask_string.size, mask_string.data);
	return true;
}

const HccAMLFunction* hcc_amlopt_make_call_graph(HccWorker* w, HccDecl function_decl, const HccAMLFunction* aml_function) {
	HccCU* cu = w->cu;
	HccAMLCallNode* head = NULL;
	HccAMLCallNode* tail = NULL;

	for (uint32_t aml_word_idx = 0; aml_word_idx < aml_function->words_count; ) {
		HccAMLInstr* aml_instr = &aml_function->words[aml_word_idx];
		HccAMLOp aml_op = HCC_AML_INSTR_OP(aml_instr);

		if (aml_op == HCC_AML_OP_CALL) {
			HccAMLOperand* aml_operands = HCC_AML_INSTR_OPERANDS(aml_instr);
			HccDecl decl = aml_operands[1];
			if (!HCC_DECL_IS_FUNCTION(decl)) {
				goto CONTINUE;
			}

			HccAMLCallNode* call_node = hcc_stack_push_thread_safe(cu->aml.call_graph_nodes);
			call_node->function_decl = decl;
			call_node->next_call_node_idx = UINT32_MAX;

			if (head) {
				tail->next_call_node_idx = call_node - cu->aml.call_graph_nodes;
			} else {
				head = call_node;
			}

			tail = call_node;
		}

CONTINUE:
		aml_word_idx += HCC_AML_INSTR_WORDS_COUNT(aml_instr);
	}

	*hcc_stack_get(cu->aml.function_call_node_lists, HCC_DECL_AUX(function_decl)) = head;

	HccStack(HccDecl) optimize_functions = hcc_aml_optimize_functions(cu);
	*hcc_stack_push_thread_safe(optimize_functions) = function_decl;

	return aml_function;
}

const HccAMLFunction* hcc_amlopt_check_for_recursion_and_make_ordered_function_list(HccWorker* w, HccDecl function_decl, const HccAMLFunction* aml_function) {
	if (aml_function->shader_stage == HCC_SHADER_STAGE_NONE) {
		return aml_function;
	}

	w->amlopt.function_recursion_call_stack_count = 0;
	hcc_amlopt_check_for_recursion_and_make_ordered_function_list_(w, function_decl, aml_function->shader_stage);

	return aml_function;
}

const HccAMLFunction* hcc_amlopt_check_for_unsupported_features(HccWorker* w, HccDecl function_decl, const HccAMLFunction* aml_function) {
	HccCU* cu = w->cu;
	HccASTFunction* ast_function = hcc_ast_function_get(cu, function_decl);
	for (uint32_t param_idx = 0; param_idx < aml_function->params_count; param_idx += 1) {
		hcc_amlopt_ensure_supported_type(w, aml_function->values[param_idx].data_type, ast_function->params_and_variables[param_idx].identifier_location);
	}

	if (aml_function->shader_stage != HCC_SHADER_STAGE_NONE) {
		if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_HLSL_PACKING)) {
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

			HccDataType bc_data_type = hcc_data_type_strip_pointer(cu, hcc_decl_resolve_and_strip_qualifiers(cu, aml_function->values[param_idx].data_type));
			HccCompoundDataType* bc_compound_data_type = hcc_compound_data_type_get(cu, bc_data_type);
			uint64_t expected_offset = 0;
			for (uint32_t field_idx = 0; field_idx < bc_compound_data_type->fields_count; field_idx += 1) {
				HccCompoundField* field = &bc_compound_data_type->fields[field_idx];
				HccDataType field_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, field->data_type);
				if (HCC_DATA_TYPE_IS_STRUCT(field_data_type)) {
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_HLSL_PACKING_NO_STRUCT, field->identifier_location);
					break;
				}
				if (HCC_DATA_TYPE_IS_UNION(field_data_type)) {
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_HLSL_PACKING_NO_UNION, field->identifier_location);
					break;
				}
				if (HCC_DATA_TYPE_IS_ARRAY(field_data_type)) {
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_HLSL_PACKING_NO_ARRAY, field->identifier_location);
					break;
				}

				uint64_t size, align;
				hcc_data_type_size_align(cu, field_data_type, &size, &align);
				if (size < 4) {
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_HLSL_PACKING_SIZE_UNDER_4_BYTE, field->identifier_location);
					break;
				}

				if (field->byte_offset != expected_offset) {
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_HLSL_PACKING_IMPLICIT_PADDING, field->identifier_location);
					break;
				}

				if ((field->byte_offset % 16) + size > 16) {
					hcc_amlopt_error_1(w, HCC_ERROR_CODE_HLSL_PACKING_OVERFLOW_16_BYTE_BOUNDARY, field->identifier_location);
					break;
				}

				expected_offset += size;
			}
		}
	}

	uint32_t last_line = 0;
	uint32_t last_column = 0;
	for (uint32_t aml_word_idx = 0; aml_word_idx < aml_function->words_count; ) {
		HccAMLInstr* aml_instr = &aml_function->words[aml_word_idx];
		HccAMLOp aml_op = HCC_AML_INSTR_OP(aml_instr);
		HccAMLOperand* aml_operands = HCC_AML_INSTR_OPERANDS(aml_instr);
		uint32_t aml_operands_count = HCC_AML_INSTR_OPERANDS_COUNT(aml_instr);

		switch (aml_op) {
			case HCC_AML_OP_BASIC_BLOCK:
			case HCC_AML_OP_PTR_ACCESS_CHAIN:
			case HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS:
				goto CONTINUE;
		}

		HccLocation* instr_location = hcc_aml_instr_location(cu, aml_instr);
		if (instr_location->line_start == last_line && instr_location->column_start == last_column) {
			// report 1 error per source code character
			goto CONTINUE;
		}

		for (uint32_t operand_idx = 0; operand_idx < aml_operands_count; operand_idx += 1) {
			HccDataType operand_data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operands[operand_idx]);
			if (operand_data_type) {
				if (hcc_amlopt_ensure_supported_type(w, operand_data_type, instr_location)) {
					// only report 1 error per instruction
					break;
				}
			}
		}

		last_line = instr_location->line_start;
		last_column = instr_location->column_start;

CONTINUE:
		aml_word_idx += HCC_AML_INSTR_WORDS_COUNT(aml_instr);
	}

	HccStack(HccDecl) optimize_functions = hcc_aml_optimize_functions(cu);
	*hcc_stack_push_thread_safe(optimize_functions) = function_decl;

	return aml_function;
}

void hcc_amlopt_optimize(HccWorker* w) {
	HccCU* cu = w->cu;
	HccDecl function_decl = (HccDecl)(uintptr_t)w->job.arg;
	HccAMLFunction* aml_function = hcc_aml_function_take_ref(cu, function_decl);
	HccAtomic(HccAMLFunction*)* dst_aml_function = hcc_stack_get(cu->aml.functions, HCC_DECL_AUX(function_decl));

	HccAMLOptFn* opts = hcc_aml_opts[cu->aml.opt_phase][aml_function->opt_level];
	uint32_t opts_count = hcc_aml_opts_count[cu->aml.opt_phase][aml_function->opt_level];

	for (uint32_t opt_idx = 0; opt_idx < opts_count; opt_idx += 1) {
		HccAMLOptFn optimize_fn = opts[opt_idx];
		HccAMLFunction* new_aml_function = (HccAMLFunction*)optimize_fn(w, function_decl, aml_function);

		if (aml_function != new_aml_function) {
			//
			// optimization made a new function, so lets:
			// - store the new function in the array of functions
			// - return the old function reference and potentially deallocate it.
			new_aml_function->ref_count = 1;
			atomic_store(dst_aml_function, new_aml_function);

			aml_function->can_free = true;
			hcc_aml_function_return_ref(cu, aml_function);
		}

		aml_function = new_aml_function;
	}

	hcc_aml_function_return_ref(cu, aml_function);
}

