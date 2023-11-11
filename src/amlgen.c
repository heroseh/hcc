#include "hcc_internal.h"

// ===========================================
//
//
// AML Generator
//
//
// ===========================================

void hcc_amlgen_init(HccWorker* w, HccCompilerSetup* setup) {
	HCC_UNUSED(w);
	HCC_UNUSED(setup);
	w->amlgen.temp_operands = hcc_stack_init(HccAMLOperand, HCC_ALLOC_TAG_SPIRVLINK_WORDS, setup->astgen.function_params_and_variables_reserve_cap, setup->astgen.function_params_and_variables_reserve_cap);
}

void hcc_amlgen_deinit(HccWorker* w) {
	HCC_UNUSED(w);
}

void hcc_amlgen_reset(HccWorker* w) {
	HCC_UNUSED(w);
}

void hcc_amlgen_error_1(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);
}

HccAMLOperand hcc_amlgen_value_add(HccWorker* w, HccDataType data_type) {
	return hcc_aml_function_value_add(w->amlgen.function, data_type);
}

HccAMLOperand hcc_amlgen_basic_block_add(HccWorker* w, HccLocation* location) {
	HccLocation** dst_location = hcc_stack_push_thread_safe(w->cu->aml.locations);
	*dst_location = location;
	w->amlgen.last_op = HCC_AML_OP_BASIC_BLOCK;
	w->amlgen.last_location = location;
	w->amlgen.is_inside_basic_block = true;
	uint32_t location_idx = dst_location - w->cu->aml.locations;
	return hcc_aml_function_basic_block_add(w->amlgen.function, location_idx);
}

HccAMLOperand hcc_amlgen_basic_block_param_add(HccWorker* w, HccDataType data_type) {
	return hcc_aml_function_basic_block_param_add(w->amlgen.function, data_type);
}

void hcc_amlgen_basic_block_param_src_add(HccWorker* w, HccAMLOperand basic_block_operand, HccAMLOperand operand) {
	hcc_aml_function_basic_block_param_src_add(w->amlgen.function, basic_block_operand, operand);
}

HccAMLOperand* hcc_amlgen_instr_add(HccWorker* w, HccLocation* location, HccAMLOp op, uint16_t operands_count) {
	HCC_DEBUG_ASSERT(location, "expected the location for this AML instruction to not be NULL");
	HCC_DEBUG_ASSERT(op != HCC_AML_OP_BASIC_BLOCK, "please use hcc_amlgen_basic_block_add to add new basic block");

	if (!w->amlgen.is_inside_basic_block) {
		w->amlgen.is_inside_basic_block = true;
		hcc_amlgen_basic_block_add(w, location);
	}

	HccLocation** dst_location = hcc_stack_push_thread_safe(w->cu->aml.locations);
	*dst_location = location;
	w->amlgen.last_op = op;
	w->amlgen.last_location = location;
	uint32_t location_idx = dst_location - w->cu->aml.locations;
	HccAMLOperand* operands = hcc_aml_function_instr_add(w->amlgen.function, location_idx, op, operands_count);

	if (op == HCC_AML_OP_BRANCH || op == HCC_AML_OP_BRANCH_CONDITIONAL || op == HCC_AML_OP_SWITCH || op == HCC_AML_OP_RETURN) {
		w->amlgen.is_inside_basic_block = false;
	}

	return operands;
}

HccAMLOperand hcc_amlgen_instr_add_1(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0) {
	HccAMLOperand* operands = hcc_amlgen_instr_add(w, location, op, 1);
	operands[0] = operand_0;
	return operand_0;
}

HccAMLOperand hcc_amlgen_instr_add_2(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0, HccAMLOperand operand_1) {
	HccAMLOperand* operands = hcc_amlgen_instr_add(w, location, op, 2);
	operands[0] = operand_0;
	operands[1] = operand_1;
	return operand_0;
}

HccAMLOperand hcc_amlgen_instr_add_3(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0, HccAMLOperand operand_1, HccAMLOperand operand_2) {
	HccAMLOperand* operands = hcc_amlgen_instr_add(w, location, op, 3);
	operands[0] = operand_0;
	operands[1] = operand_1;
	operands[2] = operand_2;
	return operand_0;
}

HccAMLOperand hcc_amlgen_instr_add_4(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand operand_0, HccAMLOperand operand_1, HccAMLOperand operand_2, HccAMLOperand operand_3) {
	HccAMLOperand* operands = hcc_amlgen_instr_add(w, location, op, 4);
	operands[0] = operand_0;
	operands[1] = operand_1;
	operands[2] = operand_2;
	operands[3] = operand_3;
	return operand_0;
}

HccAMLOperand hcc_amlgen_local_variable_operand(HccWorker* w, uint32_t local_variable_idx) {
	// the first 'params_count' value registers are the immuatable parameters.
	// all of the mutable parameters and variables come after that.
	if (local_variable_idx < w->amlgen.ast_function->params_count * 2) {
		HccAMLValue* value = &w->amlgen.function->values[w->amlgen.ast_function->params_count + local_variable_idx];
		if (value->data_type == 0) {
			return HCC_AML_OPERAND(VALUE, local_variable_idx); // return immutable parameter as the parameter was create with const
		}
	}
	return HCC_AML_OPERAND(VALUE, w->amlgen.ast_function->params_count + local_variable_idx);
}

HccAMLOperand hcc_amlgen_current_basic_block(HccWorker* w) {
	return HCC_AML_OPERAND(BASIC_BLOCK, w->amlgen.function->basic_blocks_count - 1);
}

HccAMLOperand hcc_amlgen_generate_convert_to_bool(HccWorker* w, HccLocation* location, HccAMLOperand src_operand, HccDataType src_data_type, bool flip_bool_result) {
	HCC_DEBUG_ASSERT((HCC_DATA_TYPE_IS_AML_INTRINSIC(src_data_type) && HCC_AML_INTRINSIC_DATA_TYPE_ROWS(HCC_DATA_TYPE_AUX(src_data_type)) == 1) || HCC_DATA_TYPE_IS_RESOURCE(src_data_type), "only intrinsic scalar and vector data types are convertable to bool");

	HccDataType return_data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_BOOL | (HCC_DATA_TYPE_AUX(src_data_type) & HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS_MASK));
	HccAMLOperand* operands = hcc_amlgen_instr_add(w, location, flip_bool_result ? HCC_AML_OP_EQUAL : HCC_AML_OP_NOT_EQUAL, 3);
	operands[0] = hcc_amlgen_value_add(w, return_data_type);
	operands[1] = src_operand;
	operands[2] = HCC_AML_OPERAND(CONSTANT, hcc_constant_table_deduplicate_zero(w->cu, src_data_type).idx_plus_one);
	return operands[0];
}

HccAMLOperand hcc_amlgen_generate_instrs(HccWorker* w, HccASTExpr* expr, bool want_variable_ref) {
	if (expr == NULL) {
		return 0;
	}

	switch (expr->type) {
		case HCC_AST_EXPR_TYPE_CURLY_INITIALIZER: {
			HccCU* cu = w->cu;

			//
			// make a local variable that we can mutate while we initialize the variable's fields
			HccDataType variable_data_type = hcc_data_type_lower_ast_to_aml(cu, expr->data_type);
			HccDataType ptr_data_type = hcc_pointer_data_type_deduplicate(w->cu, variable_data_type);
			HccAMLOperand variable_operand = hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_STATIC_ALLOC, hcc_amlgen_value_add(w, ptr_data_type), variable_data_type);

			HccASTExpr* initializer_expr = expr->curly_initializer.first_expr;
			while (initializer_expr) {
				HCC_DEBUG_ASSERT(initializer_expr->type == HCC_AST_EXPR_TYPE_DESIGNATED_INITIALIZER, "internal error: expected a designated initializer");
				uint64_t* elmt_indices = hcc_stack_get(cu->ast.designated_initializer_elmt_indices, initializer_expr->designated_initializer.elmt_indices_start_idx);

				//
				// generate a series of HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS & HCC_AML_OP_BITCAST as we traverse through structures and arrays
				// to get a pointer that we can store the value in.
				HccDataType data_type = variable_data_type;
				HccAMLInstr* access_chain_instr = &w->amlgen.function->words[w->amlgen.function->words_count];
				uint32_t elmts_count = initializer_expr->designated_initializer.elmts_count;
				if (initializer_expr->designated_initializer.is_bitfield | initializer_expr->designated_initializer.is_swizzle) {
					elmts_count -= 1;
				}
				HccAMLOperand dst_elmt_operand = hcc_amlgen_generate_instr_access_chain_start(w, initializer_expr->location, HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS, variable_operand, elmts_count);
				for (uint32_t elmt_indices_idx = 0; elmt_indices_idx < elmts_count; elmt_indices_idx += 1) {
					uint64_t elmt_idx = elmt_indices[elmt_indices_idx];
					if (HCC_DATA_TYPE_IS_COMPOUND(data_type)) {
						HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(cu, data_type);
						HccCompoundField* field = &compound_data_type->fields[elmt_idx];
						elmt_idx = field->storage_field_idx;
					}

					if (HCC_DATA_TYPE_IS_UNION(data_type)) {
						hcc_amlgen_generate_instr_access_chain_end(w, data_type);

						//
						// bitcast to the union type
						dst_elmt_operand = hcc_amlgen_generate_bitcast_union_field(w, initializer_expr->location, data_type, elmt_idx, dst_elmt_operand);

						//
						// start another access chain if we still have more fields we are going to access
						if (elmt_indices_idx + 1 < elmts_count) {
							uint32_t expected_operands_count = elmts_count - elmt_idx;
							access_chain_instr = &w->amlgen.function->words[w->amlgen.function->words_count];
							dst_elmt_operand = hcc_amlgen_generate_instr_access_chain_start(w, initializer_expr->location, HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS, dst_elmt_operand, expected_operands_count);
						}
					} else if (
						HCC_DATA_TYPE_IS_STRUCT(data_type) ||
						HCC_DATA_TYPE_IS_ARRAY(data_type) ||
						(HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type) && HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(HCC_DATA_TYPE_AUX(data_type)) > 1)
					) {
						//
						// standard structure or array access, so just poke the element index in the access chain operands
						HccBasic TODO_int_64_support_plz = hcc_basic_from_uint(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, elmt_idx);
						HccConstantId elmt_idx_constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &TODO_int_64_support_plz);
						hcc_amlgen_generate_instr_access_chain_set_next_operand(w, HCC_AML_OPERAND(CONSTANT, elmt_idx_constant_id.idx_plus_one));
					} else {
						HCC_ABORT("unexpected field access data type for curly initializer: %u", HCC_DATA_TYPE_TYPE(data_type));
					}

					//
					// now make the the current data_type that field we have just accessed
					if (HCC_DATA_TYPE_IS_ARRAY(data_type)) {
						HccArrayDataType* array_data_type = hcc_array_data_type_get(cu, data_type);
						data_type = array_data_type->element_data_type;
					} else if (HCC_DATA_TYPE_IS_COMPOUND(data_type)) {
						HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(cu, data_type);
						HccCompoundField* field = &compound_data_type->storage_fields[elmt_idx];
						data_type = field->data_type;
					} else if (HCC_DATA_TYPE_IS_AML_INTRINSIC(data_type)) {
						data_type = HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(HCC_DATA_TYPE_AUX(data_type)));
					}
					data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
				}

				hcc_amlgen_generate_instr_access_chain_end(w, data_type);

				//
				// now compute the value_operand and store it in the variable we are constructing
				HccAMLOperand value_operand;
				if (initializer_expr->designated_initializer.value_expr) {
					value_operand = hcc_amlgen_generate_instrs(w, initializer_expr->designated_initializer.value_expr, false);
				} else {
					HccConstantId zeroed_constant_id = hcc_constant_table_deduplicate_zero(cu, data_type);
					value_operand = HCC_AML_OPERAND(CONSTANT, zeroed_constant_id.idx_plus_one);
				}

				if (initializer_expr->designated_initializer.is_bitfield) {
					HccCompoundDataType* dt = hcc_compound_data_type_get(w->cu, data_type);
					uint32_t field_idx = elmt_indices[initializer_expr->designated_initializer.elmts_count - 1];

					HccAMLOperand old_assignee_operand = w->amlgen.assignee_operand;
					w->amlgen.assignee_operand = value_operand;

					hcc_amlgen_generate_bitfield_store(w, expr, dt, &dt->fields[field_idx], dst_elmt_operand);

					w->amlgen.assignee_operand = old_assignee_operand;
				} else if (initializer_expr->designated_initializer.is_swizzle) {
					HccSwizzle swizzle = elmt_indices[initializer_expr->designated_initializer.elmts_count - 1] - 4;

					HccAMLOperand old_assignee_operand = w->amlgen.assignee_operand;
					w->amlgen.assignee_operand = value_operand;

					hcc_amlgen_generate_swizzle_store(w, expr->location, swizzle, data_type, dst_elmt_operand);

					w->amlgen.assignee_operand = old_assignee_operand;
				} else {
					hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_STORE, dst_elmt_operand, value_operand);
				}

				initializer_expr = initializer_expr->next_stmt;
			}

			return want_variable_ref
				? variable_operand
				: hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, variable_data_type), variable_operand);
		};
		case HCC_AST_EXPR_TYPE_CAST: {
			HccASTExpr* src_expr = expr->cast_.expr;
			HccAMLOperand src_operand = hcc_amlgen_generate_instrs(w, src_expr, false);

			HccDataType dst_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->data_type);
			HccDataType src_data_type = hcc_data_type_lower_ast_to_aml(w->cu, src_expr->data_type);
			if (dst_data_type == src_data_type) {
				return src_operand;
			}

			if (dst_data_type == HCC_DATA_TYPE_AML_INTRINSIC_BOOL) {
				return hcc_amlgen_generate_convert_to_bool(w, expr->location, src_operand, src_data_type, false);
			} else {
				return hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_CONVERT, hcc_amlgen_value_add(w, dst_data_type), src_operand);
			}
		};
		case HCC_AST_EXPR_TYPE_LOCAL_VARIABLE: {
			HccAMLOperand value_operand = hcc_amlgen_local_variable_operand(w, HCC_DECL_AUX(expr->variable.decl));
			HccASTVariable* variable = &w->amlgen.ast_function->params_and_variables[HCC_DECL_AUX(expr->variable.decl)];
			return want_variable_ref
				? value_operand
				: hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, hcc_data_type_lower_ast_to_aml(w->cu, variable->data_type)), value_operand);
		};
		case HCC_AST_EXPR_TYPE_GLOBAL_VARIABLE: {
			HccAMLOperand value_operand = expr->variable.decl;
			HccASTVariable* variable = hcc_ast_global_variable_get(w->cu, expr->variable.decl);
			return want_variable_ref
				? value_operand
				: hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, hcc_data_type_lower_ast_to_aml(w->cu, variable->data_type)), value_operand);
		};
		case HCC_AST_EXPR_TYPE_CONSTANT:
			return HCC_AML_OPERAND(CONSTANT, expr->constant.id.idx_plus_one);

		case HCC_AST_EXPR_TYPE_FUNCTION:
			return (HccAMLOperand)hcc_decl_resolve_and_keep_qualifiers(w->cu, expr->function.decl);

		case HCC_AST_EXPR_TYPE_BINARY_OP: {
			if (expr->binary.op < HCC_AST_BINARY_OP_LANG_FEATURES_START) {
				//
				// generate the (left or left_ref) and right operand
				HccAMLOperand right_operand = hcc_amlgen_generate_instrs(w, expr->binary.right_expr, false);
				bool left_is_bitfield_or_swizzle = expr->binary.left_expr->type == HCC_AST_EXPR_TYPE_BINARY_OP && (expr->binary.left_expr->binary.is_bitfield | expr->binary.left_expr->binary.is_swizzle);

				HccAMLOperand old_assignee_operand;
				HccASTBinaryOp old_assignee_binary_op;
				if (expr->binary.op == HCC_AST_BINARY_OP_ASSIGN && left_is_bitfield_or_swizzle) {
					old_assignee_operand = w->amlgen.assignee_operand;
					w->amlgen.assignee_operand = right_operand;
				}

				HccAMLOperand left_ref_operand = hcc_amlgen_generate_instrs(w, expr->binary.left_expr, expr->binary.op == HCC_AST_BINARY_OP_ASSIGN && !left_is_bitfield_or_swizzle);

				if (expr->binary.op == HCC_AST_BINARY_OP_ASSIGN && left_is_bitfield_or_swizzle) {
					w->amlgen.assignee_operand = old_assignee_operand;
					// hcc_amlgen_generate_bitfield_store returns the truncated version of right_operand
					return left_ref_operand;
				}

				if (expr->binary.op == HCC_AST_BINARY_OP_ASSIGN) {
					//
					// we are an assign, so just store the right operand in the left ref memory location
					hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_STORE, left_ref_operand, right_operand);
					return right_operand;
				}

				//
				// we requested a value and not a reference since this is not an assignment
				HccDataType left_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->binary.left_expr->data_type);
				HccAMLOperand left_operand = left_ref_operand;

				//
				// work out the result value register
				HccDataType result_data_type = expr->binary.op < HCC_AST_BINARY_OP_EQUAL ? left_data_type : HCC_DATA_TYPE_AML_INTRINSIC_BOOL;
				HccAMLOperand result_operand = hcc_amlgen_value_add(w, result_data_type);

				//
				// generate the binary operator instruction
				HccAMLOp op;
				switch (expr->binary.op) {
					case HCC_AST_BINARY_OP_ADD: op = HCC_AML_OP_ADD; break;
					case HCC_AST_BINARY_OP_SUBTRACT: op = HCC_AML_OP_SUBTRACT; break;
					case HCC_AST_BINARY_OP_MULTIPLY: op = HCC_AML_OP_MULTIPLY; break;
					case HCC_AST_BINARY_OP_DIVIDE: op = HCC_AML_OP_DIVIDE; break;
					case HCC_AST_BINARY_OP_MODULO: op = HCC_AML_OP_MODULO; break;
					case HCC_AST_BINARY_OP_BIT_AND: op = HCC_AML_OP_BIT_AND; break;
					case HCC_AST_BINARY_OP_BIT_OR: op = HCC_AML_OP_BIT_OR; break;
					case HCC_AST_BINARY_OP_BIT_XOR: op = HCC_AML_OP_BIT_XOR; break;
					case HCC_AST_BINARY_OP_BIT_SHIFT_LEFT: op = HCC_AML_OP_BIT_SHIFT_LEFT; break;
					case HCC_AST_BINARY_OP_BIT_SHIFT_RIGHT: op = HCC_AML_OP_BIT_SHIFT_RIGHT; break;
					case HCC_AST_BINARY_OP_EQUAL: op = HCC_AML_OP_EQUAL; break;
					case HCC_AST_BINARY_OP_NOT_EQUAL: op = HCC_AML_OP_NOT_EQUAL; break;
					case HCC_AST_BINARY_OP_LESS_THAN: op = HCC_AML_OP_LESS_THAN; break;
					case HCC_AST_BINARY_OP_LESS_THAN_OR_EQUAL: op = HCC_AML_OP_LESS_THAN_OR_EQUAL; break;
					case HCC_AST_BINARY_OP_GREATER_THAN: op = HCC_AML_OP_GREATER_THAN; break;
					case HCC_AST_BINARY_OP_GREATER_THAN_OR_EQUAL: op = HCC_AML_OP_GREATER_THAN_OR_EQUAL; break;
					default: HCC_ABORT("unhandled binary operator: %u", expr->binary.op);
				}
				hcc_amlgen_instr_add_3(w, expr->location, op, result_operand, left_operand, right_operand);

				return result_operand;
			} else {
				switch (expr->binary.op) {
					case HCC_AST_BINARY_OP_LOGICAL_AND:
					case HCC_AST_BINARY_OP_LOGICAL_OR: {
						uint32_t success_idx = 1 + (expr->binary.op != HCC_AST_BINARY_OP_LOGICAL_AND);
						uint32_t converging_idx = 1 + (expr->binary.op == HCC_AST_BINARY_OP_LOGICAL_AND);

						//
						// current basic block
						HccAMLOperand left_operand = hcc_amlgen_generate_instrs_condition(w, expr->binary.left_expr);
						HccAMLOperand branch_conditional_basic_block_operand = hcc_amlgen_current_basic_block(w);
						HccAMLOperand* selection_merge_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_SELECTION_MERGE, 1);
						HccAMLOperand* cond_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH_CONDITIONAL, 4);

						//
						// success basic block
						HccAMLOperand success_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
						HccAMLOperand right_operand = hcc_amlgen_generate_instrs_condition(w, expr->binary.right_expr);
						HccAMLOperand final_success_basic_block = hcc_amlgen_current_basic_block(w);
						HccAMLOperand* success_converging_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 2);

						//
						// converging basic block which will have the result as it's only parameter
						HccAMLOperand converging_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
						selection_merge_operands[0] = converging_basic_block;

						HccBasic basic = { .u8 = expr->binary.op == HCC_AST_BINARY_OP_LOGICAL_OR };
						HccConstantId converging_arg_constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_BOOL, &basic);
						cond_branch_operands[0] = left_operand;
						cond_branch_operands[success_idx] = success_basic_block;
						cond_branch_operands[converging_idx] = converging_basic_block;
						cond_branch_operands[3] = HCC_AML_OPERAND(CONSTANT, converging_arg_constant_id.idx_plus_one); // converging block arg for when the short-circuit succeeds

						success_converging_branch_operands[0] = converging_basic_block;
						success_converging_branch_operands[1] = right_operand; // converging block arg for when the short-circuit fails

						//
						// add the result basic block parameter and return that operand
						HccAMLOperand operand = hcc_amlgen_basic_block_param_add(w, HCC_DATA_TYPE_AML_INTRINSIC_BOOL);
						hcc_amlgen_basic_block_param_src_add(w, branch_conditional_basic_block_operand, cond_branch_operands[3]);
						hcc_amlgen_basic_block_param_src_add(w, final_success_basic_block, success_converging_branch_operands[1]);
						return operand;
					};
					case HCC_AST_BINARY_OP_TERNARY: {
						HccASTExpr* cond_expr = expr->binary.left_expr;
						HccASTExpr* right_expr = expr->binary.right_expr;
						HccASTExpr* result_true_expr = right_expr->binary.left_expr;
						HccASTExpr* result_false_expr = right_expr->binary.right_expr;
						bool result_true_is_constant = result_true_expr->type == HCC_AST_EXPR_TYPE_CONSTANT;
						bool result_false_is_constant = result_false_expr->type == HCC_AST_EXPR_TYPE_CONSTANT;
						HccAMLOperand basic_block_operand = hcc_amlgen_current_basic_block(w);
						HccDataType result_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->data_type);

						//
						// current basic block
						HccAMLOperand cond_operand = hcc_amlgen_generate_instrs_condition(w, cond_expr);
						if (result_true_is_constant && result_false_is_constant) {
							//
							// no need to branch and avoid any runtime evaluation for the result left or result right expressions as they are both constants.
							// so just emit a select instruction.
							HccAMLOperand result_true_operand = HCC_AML_OPERAND(CONSTANT, result_true_expr->constant.id.idx_plus_one);
							HccAMLOperand result_false_operand = HCC_AML_OPERAND(CONSTANT, result_false_expr->constant.id.idx_plus_one);
							return hcc_amlgen_instr_add_4(w, expr->location, HCC_AML_OP_SELECT,
								hcc_amlgen_value_add(w, result_data_type), cond_operand, result_true_operand, result_false_operand);
						}

						//
						// for the true & false blocks, if they are constants:
						//     it's block doesn't exist and it will branch straight from the original conditional branch to the converging block with it's result.
						// for the true & false blocks, if they are NOT constants:
						//     it's block does exist with it's result evaluated inside that block that will branch to the converging block with it's result.
						//
						HccAMLOperand* selection_merge_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_SELECTION_MERGE, 1);
						HccAMLOperand* cond_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH_CONDITIONAL, 3 + result_true_is_constant + result_false_is_constant);
						cond_branch_operands[0] = cond_operand;

						//
						// generate the true and/or false blocks if they need to exist
						HccAMLOperand* true_converging_branch_operands;
						HccAMLOperand true_final_basic_block;
						if (!result_true_is_constant) {
							HccAMLOperand true_basic_block_operand = hcc_amlgen_basic_block_add(w, expr->location);
							HccAMLOperand result_true_operand = hcc_amlgen_generate_instrs(w, result_true_expr, false);
							true_converging_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 2);
							true_converging_branch_operands[1] = result_true_operand;

							true_final_basic_block = hcc_amlgen_current_basic_block(w);
							cond_branch_operands[1] = true_basic_block_operand;
						}
						HccAMLOperand* false_converging_branch_operands;
						HccAMLOperand false_final_basic_block;
						if (!result_false_is_constant) {
							HccAMLOperand false_basic_block_operand = hcc_amlgen_basic_block_add(w, expr->location);
							HccAMLOperand result_false_operand = hcc_amlgen_generate_instrs(w, result_false_expr, false);
							false_converging_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 2);
							false_converging_branch_operands[1] = result_false_operand;

							false_final_basic_block = hcc_amlgen_current_basic_block(w);
							cond_branch_operands[2] = false_basic_block_operand;
						}

						//
						// converging basic block which will have the result as it's only parameter
						HccAMLOperand converging_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
						selection_merge_operands[0] = converging_basic_block;

						//
						// add the result basic block parameter that will be returned
						HccAMLOperand operand = hcc_amlgen_basic_block_param_add(w, result_data_type);

						//
						// patch the converging basic block operand into the branching instructions that target it.
						if (result_true_is_constant) {
							HccAMLOperand result_true_operand = HCC_AML_OPERAND(CONSTANT, result_true_expr->constant.id.idx_plus_one);
							cond_branch_operands[1] = converging_basic_block;
							cond_branch_operands[3] = result_true_operand;
							hcc_amlgen_basic_block_param_src_add(w, basic_block_operand, cond_branch_operands[3]);
						} else {
							true_converging_branch_operands[0] = converging_basic_block;
							hcc_amlgen_basic_block_param_src_add(w, true_final_basic_block, true_converging_branch_operands[1]);
						}
						if (result_false_is_constant) {
							HccAMLOperand result_false_operand = HCC_AML_OPERAND(CONSTANT, result_false_expr->constant.id.idx_plus_one);
							cond_branch_operands[2] = converging_basic_block;
							cond_branch_operands[3 + result_true_is_constant] = result_false_operand;
							hcc_amlgen_basic_block_param_src_add(w, basic_block_operand, cond_branch_operands[3]);
						} else {
							false_converging_branch_operands[0] = converging_basic_block;
							hcc_amlgen_basic_block_param_src_add(w, false_final_basic_block, false_converging_branch_operands[1]);
						}

						return operand;
					};
					case HCC_AST_BINARY_OP_COMMA: {
						HccAMLOperand left_operand = hcc_amlgen_generate_instrs(w, expr->binary.left_expr, false);
						HccAMLOperand right_operand = hcc_amlgen_generate_instrs(w, expr->binary.right_expr, false);
						return right_operand;
					};
					case HCC_AST_BINARY_OP_FIELD_ACCESS:
					case HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT:
					case HCC_AST_BINARY_OP_ARRAY_SUBSCRIPT: {
						HccAMLOperand result_operand;
						result_operand = hcc_amlgen_generate_instr_access_chain(w, expr, 0, want_variable_ref);
						if (!want_variable_ref && expr->type == HCC_AST_EXPR_TYPE_LOCAL_VARIABLE && HCC_DECL_AUX(expr->variable.decl) < w->amlgen.ast_function->params_count && w->amlgen.ast_function->shader_stage != HCC_SHADER_STAGE_NONE) {
							hcc_amlgen_error_1(w, HCC_ERROR_CODE_BUILT_IN_WRITE_ONLY_POINTER_CANNOT_BE_READ, expr->location);
						}

						return result_operand;
					};
					case HCC_AST_BINARY_OP_CALL: {
						HccASTExpr* arg_expr = expr->binary.right_expr;
						uint32_t temp_operands_start_idx = hcc_stack_count(w->amlgen.temp_operands);

						//
						// generate the arguments left-right as this is super crazy undefined.
						// clang does left-right, gcc does right-left ON THE SAME ARCHITECTURE WTF!?!?
						uint32_t args_count = 0;
						while (arg_expr) {
							HccAMLOperand arg_operand = hcc_amlgen_generate_instrs(w, arg_expr, false);
							*hcc_stack_push(w->amlgen.temp_operands) = arg_operand;
							arg_expr = arg_expr->next_stmt;
							args_count += 1;
						};

						//
						// generate the callee
						HccASTExpr* callee_expr = expr->binary.left_expr;
						HccDecl function_decl = 0;
						if (callee_expr->type == HCC_AST_EXPR_TYPE_FUNCTION) {
							function_decl = hcc_decl_resolve_and_keep_qualifiers(w->cu, callee_expr->function.decl);
						}
						HccAMLOperand callee_operand = hcc_amlgen_generate_instrs(w, callee_expr, false);
						HccDataType return_data_type = hcc_decl_return_data_type(w->cu, callee_expr->data_type);
						return_data_type = hcc_data_type_lower_ast_to_aml(w->cu, return_data_type);
						bool has_return_value = return_data_type != HCC_DATA_TYPE_AML_INTRINSIC_VOID;

						HccAMLOperand return_operand;
						if (callee_expr->type != HCC_AST_EXPR_TYPE_FUNCTION || HCC_AML_OPERAND_AUX(function_decl) >= HCC_FUNCTION_IDX_USER_START) {
							//
							// now create the call instruction
							HccAMLOperand* operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_CALL, args_count + 2);
							operands[0] = hcc_amlgen_value_add(w, return_data_type);
							operands[1] = callee_operand;
							for (uint32_t arg_idx = 0; arg_idx < args_count; arg_idx += 1) {
								operands[2 + arg_idx] = *hcc_stack_get(w->amlgen.temp_operands, temp_operands_start_idx + arg_idx);
							}

							return_operand = operands[0];
						} else {
							uint32_t function_idx = HCC_AML_OPERAND_AUX(function_decl);

							HccAMLOp op = HCC_AML_OP_NO_OP;
							switch (function_idx) {
								case HCC_FUNCTION_IDX_F16TOF32: op = HCC_AML_OP_CONVERT; break;
								case HCC_FUNCTION_IDX_F16TOF64: op = HCC_AML_OP_CONVERT; break;
								case HCC_FUNCTION_IDX_F32TOF16: op = HCC_AML_OP_CONVERT; break;
								case HCC_FUNCTION_IDX_F64TOF16: op = HCC_AML_OP_CONVERT; break;
								case HCC_FUNCTION_IDX_PACK_F16X2_F32X2: op = HCC_AML_OP_PACK_F16X2_F32X2; break;
								case HCC_FUNCTION_IDX_UNPACK_F16X2_F32X2: op = HCC_AML_OP_UNPACK_F16X2_F32X2; break;
								case HCC_FUNCTION_IDX_PACK_U16X2_F32X2: op = HCC_AML_OP_PACK_U16X2_F32X2; break;
								case HCC_FUNCTION_IDX_UNPACK_U16X2_F32X2: op = HCC_AML_OP_UNPACK_U16X2_F32X2; break;
								case HCC_FUNCTION_IDX_PACK_S16X2_F32X2: op = HCC_AML_OP_PACK_S16X2_F32X2; break;
								case HCC_FUNCTION_IDX_UNPACK_S16X2_F32X2: op = HCC_AML_OP_UNPACK_S16X2_F32X2; break;
								case HCC_FUNCTION_IDX_PACK_U8X4_F32X4: op = HCC_AML_OP_PACK_U8X4_F32X4; break;
								case HCC_FUNCTION_IDX_UNPACK_U8X4_F32X4: op = HCC_AML_OP_UNPACK_U8X4_F32X4; break;
								case HCC_FUNCTION_IDX_PACK_S8X4_F32X4: op = HCC_AML_OP_PACK_S8X4_F32X4; break;
								case HCC_FUNCTION_IDX_UNPACK_S8X4_F32X4: op = HCC_AML_OP_UNPACK_S8X4_F32X4; break;
								case HCC_FUNCTION_IDX_DISCARD_PIXEL: op = HCC_AML_OP_DISCARD_PIXEL; break;
								case HCC_FUNCTION_IDX_MEMORY_BARRIER_RESOURCE: op = HCC_AML_OP_MEMORY_BARRIER_RESOURCE; break;
								case HCC_FUNCTION_IDX_MEMORY_BARRIER_DISPATCH_GROUP: op = HCC_AML_OP_MEMORY_BARRIER_DISPATCH_GROUP; break;
								case HCC_FUNCTION_IDX_MEMORY_BARRIER_ALL: op = HCC_AML_OP_MEMORY_BARRIER_ALL; break;
								case HCC_FUNCTION_IDX_CONTROL_BARRIER_RESOURCE: op = HCC_AML_OP_CONTROL_BARRIER_RESOURCE; break;
								case HCC_FUNCTION_IDX_CONTROL_BARRIER_DISPATCH_GROUP: op = HCC_AML_OP_CONTROL_BARRIER_DISPATCH_GROUP; break;
								case HCC_FUNCTION_IDX_CONTROL_BARRIER_ALL: op = HCC_AML_OP_CONTROL_BARRIER_ALL; break;
								case HCC_FUNCTION_IDX_HPRINT_STRING:
									op = HCC_AML_OP_HPRINT_STRING;
									w->amlgen.temp_operands[temp_operands_start_idx + 0] = hcc_amlgen_generate_resource_descriptor_load(w, expr->location, w->amlgen.temp_operands[temp_operands_start_idx + 0]);
									break;
								default: {
									HCC_DEBUG_ASSERT(HCC_FUNCTION_IDX_MANY_START <= function_idx && function_idx < HCC_FUNCTION_IDX_MANY_END, "unhandled intrinsic function");
									uint32_t many = (function_idx - HCC_FUNCTION_IDX_MANY_START) / HCC_AML_INTRINSIC_DATA_TYPE_COUNT;
									switch (many) {
										case HCC_FUNCTION_MANY_NOT: {
											HccAMLOperand src_operand = *hcc_stack_get(w->amlgen.temp_operands, temp_operands_start_idx + 0);
											HccDataType src_data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, src_operand);

											if (HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(HCC_DATA_TYPE_AUX(src_data_type)) != HCC_AML_INTRINSIC_DATA_TYPE_BOOL) {
												return_operand = hcc_amlgen_generate_convert_to_bool(w, expr->location, src_operand, src_data_type, true);
												goto CALL_END;
											}

											return_operand = hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_XOR,
												hcc_amlgen_value_add(w, return_data_type),
												src_operand,
												HCC_AML_OPERAND(CONSTANT, hcc_constant_table_deduplicate_one(w->cu, src_data_type).idx_plus_one));
											goto CALL_END;
										};
										case HCC_FUNCTION_MANY_BITNOT: {
											HccAMLOperand src_operand = *hcc_stack_get(w->amlgen.temp_operands, temp_operands_start_idx + 0);
											HccDataType src_data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, src_operand);

											return_operand = hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_XOR,
												hcc_amlgen_value_add(w, return_data_type),
												src_operand,
												HCC_AML_OPERAND(CONSTANT, hcc_constant_table_deduplicate_minus_one(w->cu, src_data_type).idx_plus_one));
											goto CALL_END;
										};
										case HCC_FUNCTION_MANY_BITSTO:
										case HCC_FUNCTION_MANY_BITSFROM: {
											HccAMLOperand src_operand = *hcc_stack_get(w->amlgen.temp_operands, temp_operands_start_idx + 0);
											HccDataType src_data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, src_operand);

											return_operand = hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_BITCAST,
												hcc_amlgen_value_add(w, return_data_type),
												src_operand);
											goto CALL_END;
										};

										case HCC_FUNCTION_MANY_ANY:
										case HCC_FUNCTION_MANY_ALL:
										case HCC_FUNCTION_MANY_QUAD_ANY:
										case HCC_FUNCTION_MANY_QUAD_ALL:
										case HCC_FUNCTION_MANY_WAVE_ACTIVE_ANY:
										case HCC_FUNCTION_MANY_WAVE_ACTIVE_ALL:
										{
											HccAMLOperand src_operand = *hcc_stack_get(w->amlgen.temp_operands, temp_operands_start_idx + 0);
											HccDataType src_data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, src_operand);

											if (HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(HCC_DATA_TYPE_AUX(src_data_type)) != HCC_AML_INTRINSIC_DATA_TYPE_BOOL) {
												return hcc_amlgen_generate_convert_to_bool(w, expr->location, src_operand, src_data_type, false);
											}

											return_operand = hcc_amlgen_instr_add_2(w, expr->location, hcc_intrinisic_function_many_aml_ops[many],
												hcc_amlgen_value_add(w, return_data_type),
												src_operand);
											goto CALL_END;
										};

										case HCC_FUNCTION_MANY_SAMPLE_TEXTURE:
										case HCC_FUNCTION_MANY_SAMPLE_MIP_BIAS_TEXTURE:
										case HCC_FUNCTION_MANY_SAMPLE_MIP_GRADIENT_TEXTURE:
										case HCC_FUNCTION_MANY_SAMPLE_MIP_LEVEL_TEXTURE:
										case HCC_FUNCTION_MANY_GATHER_RED_TEXTURE:
										case HCC_FUNCTION_MANY_GATHER_GREEN_TEXTURE:
										case HCC_FUNCTION_MANY_GATHER_BLUE_TEXTURE:
										case HCC_FUNCTION_MANY_GATHER_ALPHA_TEXTURE: {
											w->amlgen.temp_operands[temp_operands_start_idx + 0] = hcc_amlgen_generate_resource_descriptor_load(w, expr->location, w->amlgen.temp_operands[temp_operands_start_idx + 0]);
											w->amlgen.temp_operands[temp_operands_start_idx + 1] = hcc_amlgen_generate_resource_descriptor_load(w, expr->location, w->amlgen.temp_operands[temp_operands_start_idx + 1]);
											op = hcc_intrinisic_function_many_aml_ops[many];
											break;
										};

										case HCC_FUNCTION_MANY_LOAD_TEXTURE:
										case HCC_FUNCTION_MANY_FETCH_TEXTURE:
										case HCC_FUNCTION_MANY_STORE_TEXTURE:
										case HCC_FUNCTION_MANY_LOAD_BYTE_BUFFER:
										case HCC_FUNCTION_MANY_STORE_BYTE_BUFFER:
										{
											w->amlgen.temp_operands[temp_operands_start_idx + 0] = hcc_amlgen_generate_resource_descriptor_load(w, expr->location, w->amlgen.temp_operands[temp_operands_start_idx + 0]);
											op = hcc_intrinisic_function_many_aml_ops[many];
											break;
										};

										case HCC_FUNCTION_MANY_ADDR_RO_TEXTURE:
										case HCC_FUNCTION_MANY_ADDR_RW_TEXTURE:
										{
											w->amlgen.temp_operands[temp_operands_start_idx + 0] = hcc_amlgen_generate_resource_descriptor_addr(w, expr->location, w->amlgen.temp_operands[temp_operands_start_idx + 0]);
											op = hcc_intrinisic_function_many_aml_ops[many];
											break;
										};

										default:
											op = hcc_intrinisic_function_many_aml_ops[many];
											break;
									}
									break;
								};
							}
							HCC_DEBUG_ASSERT(op != HCC_AML_OP_NO_OP, "unhandled intrinsic function_idx '%u' when converting from AST -> AML", function_idx);

							uint32_t params_offset = has_return_value ? 1 : 0;
							HccAMLOperand* operands = hcc_amlgen_instr_add(w, expr->location, op, args_count + params_offset);
							if (has_return_value) {
								operands[0] = hcc_amlgen_value_add(w, return_data_type);
							}
							for (uint32_t arg_idx = 0; arg_idx < args_count; arg_idx += 1) {
								operands[params_offset + arg_idx] = *hcc_stack_get(w->amlgen.temp_operands, temp_operands_start_idx + arg_idx);
							}
							return_operand = operands[0];
						}

CALL_END:{}
						//
						// restore the temp_operands back to the size it was
						hcc_stack_resize(w->amlgen.temp_operands, temp_operands_start_idx);

						if (want_variable_ref) {
							HccDataType data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, return_operand);
							HccAMLOperand value_operand = hcc_amlgen_value_add(w, hcc_pointer_data_type_deduplicate(w->cu, data_type));
							HccAMLOperand dst_operand = hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_STATIC_ALLOC, value_operand, data_type);
							hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_STORE, dst_operand, return_operand);
							return_operand = dst_operand;
						}

						return return_operand;
					};
				}
			}
			break;
		};
		case HCC_AST_EXPR_TYPE_UNARY_OP: {
			HccASTExpr* src_expr = expr->unary.expr;
			HccAMLOperand src_operand =
				hcc_amlgen_generate_instrs(w, src_expr,
					expr->unary.op == HCC_AST_UNARY_OP_PRE_INCREMENT || expr->unary.op == HCC_AST_UNARY_OP_PRE_DECREMENT ||
					expr->unary.op == HCC_AST_UNARY_OP_POST_INCREMENT || expr->unary.op == HCC_AST_UNARY_OP_POST_DECREMENT ||
					expr->unary.op == HCC_AST_UNARY_OP_ADDRESS_OF
				);

			HccDataType dst_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->data_type);
			HccDataType src_data_type = hcc_data_type_lower_ast_to_aml(w->cu, src_expr->data_type);
			switch (expr->unary.op) {
				case HCC_AST_UNARY_OP_LOGICAL_NOT: {
					if (src_data_type != HCC_DATA_TYPE_AML_INTRINSIC_BOOL) {
						return hcc_amlgen_generate_convert_to_bool(w, expr->location, src_operand, src_data_type, true);
					}

					return hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_XOR,
						hcc_amlgen_value_add(w, dst_data_type),
						src_operand,
						HCC_AML_OPERAND(CONSTANT, hcc_constant_table_deduplicate_one(w->cu, src_data_type).idx_plus_one));
				};
				case HCC_AST_UNARY_OP_BIT_NOT:
					return hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_XOR,
						hcc_amlgen_value_add(w, dst_data_type),
						src_operand,
						HCC_AML_OPERAND(CONSTANT, hcc_constant_table_deduplicate_minus_one(w->cu, src_data_type).idx_plus_one));

				case HCC_AST_UNARY_OP_NEGATE:
					return hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_NEGATE, hcc_amlgen_value_add(w, dst_data_type), src_operand);

				case HCC_AST_UNARY_OP_PRE_INCREMENT:
				case HCC_AST_UNARY_OP_POST_INCREMENT:
				case HCC_AST_UNARY_OP_PRE_DECREMENT:
				case HCC_AST_UNARY_OP_POST_DECREMENT: {
					// the variable to store the result into
					HccAMLOperand dst_operand = src_operand;

					// load the variable
					src_operand = hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, src_data_type), src_operand);

					HccAMLOp op = expr->unary.op == HCC_AST_UNARY_OP_PRE_INCREMENT || expr->unary.op == HCC_AST_UNARY_OP_POST_INCREMENT
						? HCC_AML_OP_ADD
						: HCC_AML_OP_SUBTRACT;

					HccAMLOperand result_operand = hcc_amlgen_instr_add_3(w, expr->location, op,
						hcc_amlgen_value_add(w, dst_data_type),
						src_operand,
						HCC_AML_OPERAND(CONSTANT, hcc_constant_table_deduplicate_one(w->cu, src_data_type).idx_plus_one));

					// store the result into the variable
					hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_STORE, dst_operand, result_operand);

					return expr->unary.op == HCC_AST_UNARY_OP_PRE_INCREMENT || expr->unary.op == HCC_AST_UNARY_OP_PRE_DECREMENT
						? result_operand
						: src_operand;
				};
				case HCC_AST_UNARY_OP_DEREF:
					return hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, dst_data_type), src_operand);

				case HCC_AST_UNARY_OP_ADDRESS_OF:
				case HCC_AST_UNARY_OP_PLUS:
					return src_operand; // do nothing as these are a pure language construct
			}
			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_IF: {
			HccAMLOperand cond_operand = hcc_amlgen_generate_instrs_condition(w, expr->if_.cond_expr);

			HccAMLOperand* selection_merge_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_SELECTION_MERGE, 1);
			HccAMLOperand* cond_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH_CONDITIONAL, 3);
			cond_branch_operands[0] = cond_operand;

			HccAMLOperand true_basic_block_operand = hcc_amlgen_basic_block_add(w, expr->location);
			cond_branch_operands[1] = true_basic_block_operand;
			hcc_amlgen_generate_instrs(w, expr->if_.true_stmt, false);
			true_basic_block_operand = hcc_amlgen_current_basic_block(w);

			HccAMLBasicBlock* true_basic_block = &w->amlgen.function->basic_blocks[HCC_AML_OPERAND_AUX(true_basic_block_operand)];
			HccAMLOperand* true_branch_operands = NULL;
			if (true_basic_block->terminating_instr_word_idx == UINT32_MAX) {
				true_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 1);
			}

			HccASTExpr* false_stmt = expr->if_.false_stmt;
			HccAMLOperand* false_branch_operands = NULL;
			if (false_stmt) {
				HccAMLOperand false_basic_block_operand = hcc_amlgen_basic_block_add(w, expr->location);
				cond_branch_operands[2] = false_basic_block_operand;
				hcc_amlgen_generate_instrs(w, false_stmt, false);
				false_basic_block_operand = hcc_amlgen_current_basic_block(w);
				HccAMLBasicBlock* false_basic_block = &w->amlgen.function->basic_blocks[HCC_AML_OPERAND_AUX(false_basic_block_operand)];
				if (false_basic_block->terminating_instr_word_idx == UINT32_MAX) {
					false_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 1);
				}
			}

			HccAMLOperand converging_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
			if (true_branch_operands) {
				true_branch_operands[0] = converging_basic_block;
			}
			if (false_stmt) {
				if (false_branch_operands) {
					false_branch_operands[0] = converging_basic_block;
				}
			} else {
				cond_branch_operands[2] = converging_basic_block;
			}
			selection_merge_operands[0] = converging_basic_block;

			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_SWITCH: {
			HccAMLOperand cond_operand = hcc_amlgen_generate_instrs(w, expr->switch_.cond_expr, false);

			HccAMLOperand* selection_merge_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_SELECTION_MERGE, 1);
			HccAMLOperand* switch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_SWITCH, (expr->switch_.case_stmts_count * 2) + 2);
			switch_operands[0] = cond_operand;

			//
			// backup the switch state tracking in case we are inside of another switch statement.
			HccAMLOperand* prev_switch_case_operands = w->amlgen.switch_case_operands;
			uint32_t prev_switch_case_idx = w->amlgen.switch_case_idx;
			uint32_t prev_break_stmt_list_head_id = w->amlgen.break_stmt_list_head_id;
			uint32_t prev_break_stmt_list_prev_id = w->amlgen.break_stmt_list_prev_id;

			//
			// setup the switch state tracking and generate the code inside the switch statement
			w->amlgen.switch_case_operands = &switch_operands[2];
			w->amlgen.switch_case_idx = 0;
			w->amlgen.break_stmt_list_head_id = 0;
			w->amlgen.break_stmt_list_prev_id = 0;
			hcc_amlgen_generate_instrs(w, expr->switch_.block_expr, false);

			HccAMLOperand converging_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
			if (!expr->switch_.has_default_case) {
				switch_operands[1] = converging_basic_block;
			}

			//
			// patch the HCC_AML_OP_BRANCH instructions for the break statements
			// to jump to the new converging basic block we have made.
			HccAMLWord* words = w->amlgen.function->words;
			uint32_t break_word_id = w->amlgen.break_stmt_list_head_id;
			while (break_word_id) {
				uint32_t next_break_word_id = words[break_word_id - 1];
				words[break_word_id - 1] = converging_basic_block;
				break_word_id = next_break_word_id;
			}
			selection_merge_operands[0] = converging_basic_block;

			//
			// restore the switch state tracking for the outer switch statement
			w->amlgen.switch_case_operands = prev_switch_case_operands;
			w->amlgen.switch_case_idx = prev_switch_case_idx;
			w->amlgen.break_stmt_list_head_id = prev_break_stmt_list_head_id;
			w->amlgen.break_stmt_list_prev_id = prev_break_stmt_list_prev_id;
			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_WHILE:
		case HCC_AST_EXPR_TYPE_STMT_FOR: {
			HccASTExpr* init_stmt;
			HccASTExpr* cond_expr;
			HccASTExpr* inc_stmt;
			HccASTExpr* loop_stmt;
			bool is_do_while_loop;
			if (expr->type == HCC_AST_EXPR_TYPE_STMT_FOR) {
				is_do_while_loop = false;
				init_stmt = expr->for_.init_stmt;
				cond_expr = expr->for_.cond_expr;
				inc_stmt = expr->for_.inc_stmt;
				loop_stmt = expr->for_.loop_stmt;
			} else {
				is_do_while_loop = expr->while_.cond_expr > expr->while_.loop_stmt;
				init_stmt = NULL;
				cond_expr = expr->while_.cond_expr;
				inc_stmt = NULL;
				loop_stmt = expr->while_.loop_stmt;
			}

			//
			// backup the loop state tracking in case we are inside of another loop statement.
			uint32_t prev_continue_stmt_list_head_id = w->amlgen.continue_stmt_list_head_id;
			uint32_t prev_continue_stmt_list_prev_id = w->amlgen.continue_stmt_list_prev_id;
			uint32_t prev_break_stmt_list_head_id = w->amlgen.break_stmt_list_head_id;
			uint32_t prev_break_stmt_list_prev_id = w->amlgen.break_stmt_list_prev_id;

			//
			// setup the loop state tracking and generate the code inside the loop statement
			w->amlgen.continue_stmt_list_head_id = 0;
			w->amlgen.continue_stmt_list_prev_id = 0;
			w->amlgen.break_stmt_list_head_id = 0;
			w->amlgen.break_stmt_list_prev_id = 0;

			if (init_stmt) {
				hcc_amlgen_generate_instrs(w, init_stmt, false);
			}

			HccAMLOperand loop_header_basic_block = hcc_amlgen_basic_block_add(w, expr->location);

			HccAMLOperand cond_operand;
			if (!is_do_while_loop && cond_expr) {
				cond_operand = hcc_amlgen_generate_instrs_condition(w, cond_expr);
			}

			HccAMLOperand* cond_branch_operands;
			HccAMLOperand* loop_merge_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_LOOP_MERGE, 2);
			if (is_do_while_loop || cond_expr == NULL) {
				cond_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 1);
			} else {
				cond_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH_CONDITIONAL, 3);
				cond_branch_operands[0] = cond_operand;
			}

			HccAMLOperand loop_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
			hcc_amlgen_generate_instrs(w, loop_stmt, false);
			cond_branch_operands[is_do_while_loop || cond_expr == NULL ? 0 : 1] = loop_basic_block;

			// make a continue block here as SPIR-V wants a single branch instruction that
			// restarts the loop. this can be optimized out on other platform in the AMLOPT anyway.
			HccAMLOperand continue_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
			if (is_do_while_loop) {
				HccAMLOperand cond_operand = hcc_amlgen_generate_instrs_condition(w, cond_expr);
				cond_branch_operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH_CONDITIONAL, 3);
				cond_branch_operands[0] = cond_operand;
				cond_branch_operands[1] = loop_header_basic_block;
			} else {
				if (inc_stmt) {
					hcc_amlgen_generate_instrs(w, inc_stmt, false);
				}
				hcc_amlgen_instr_add_1(w, expr->location, HCC_AML_OP_BRANCH, loop_header_basic_block);
			}

			HccAMLOperand converging_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
			cond_branch_operands[2] = converging_basic_block;

			loop_merge_operands[0] = converging_basic_block;
			loop_merge_operands[1] = continue_basic_block;

			//
			// patch the HCC_AML_OP_BRANCH instructions for the continue statements
			// to jump to the condition basic block we have made.
			HccAMLWord* words = w->amlgen.function->words;
			uint32_t continue_word_id = w->amlgen.continue_stmt_list_head_id;
			while (continue_word_id) {
				uint32_t next_continue_word_id = words[continue_word_id - 1];
				words[continue_word_id - 1] = continue_basic_block;
				continue_word_id = next_continue_word_id;
			}

			//
			// patch the HCC_AML_OP_BRANCH instructions for the break statements
			// to jump to the converging basic block we have made.
			uint32_t break_word_id = w->amlgen.break_stmt_list_head_id;
			while (break_word_id) {
				uint32_t next_break_word_id = words[break_word_id - 1];
				words[break_word_id - 1] = converging_basic_block;
				break_word_id = next_break_word_id;
			}

			//
			// restore the loop state tracking for the outer loop statement
			w->amlgen.continue_stmt_list_head_id = prev_continue_stmt_list_head_id;
			w->amlgen.continue_stmt_list_prev_id = prev_continue_stmt_list_prev_id;
			w->amlgen.break_stmt_list_head_id = prev_break_stmt_list_head_id;
			w->amlgen.break_stmt_list_prev_id = prev_break_stmt_list_prev_id;
			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_CASE: {
			HccAMLOperand case_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
			w->amlgen.switch_case_operands[w->amlgen.switch_case_idx * 2 + 0] = HCC_AML_OPERAND(CONSTANT, expr->case_.constant_id.idx_plus_one);
			w->amlgen.switch_case_operands[w->amlgen.switch_case_idx * 2 + 1] = case_basic_block;
			w->amlgen.switch_case_idx += 1;
			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_DEFAULT: {
			HccAMLOperand default_basic_block = hcc_amlgen_basic_block_add(w, expr->location);
			w->amlgen.switch_case_operands[-1] = default_basic_block;
			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_BREAK: {
			// add the branch instruction that get completed later at the end of the HCC_AST_EXPR_TYPE_STMT_SWITCH, HCC_AST_EXPR_TYPE_STMT_WHILE or HCC_AST_EXPR_TYPE_STMT_FOR code
			HccAMLOperand* operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 1);

			// calculate the id from the start of the words array for this function
			// to where the basic block operand goes in our branch instruction.
			HccAMLWord* words = w->amlgen.function->words;
			HccAMLWord* dst_basic_block_word_ptr = &operands[0];
			uint32_t dst_basic_block_word_id = (dst_basic_block_word_ptr - words) + 1;

			// link the operand together in a list with the other break statement branch instructions
			if (w->amlgen.break_stmt_list_prev_id) {
				HccAMLWord* prev_linked_word = &words[w->amlgen.break_stmt_list_prev_id - 1];
				*prev_linked_word = dst_basic_block_word_id;
			} else {
				w->amlgen.break_stmt_list_head_id = dst_basic_block_word_id;
			}
			w->amlgen.break_stmt_list_prev_id = dst_basic_block_word_id;
			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_CONTINUE: {
			// add the branch instruction that get completed later at the end of the HCC_AST_EXPR_TYPE_STMT_WHILE or HCC_AST_EXPR_TYPE_STMT_FOR code
			HccAMLOperand* operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_BRANCH, 1);

			// calculate the id from the start of the words array for this function
			// to where the basic block operand goes in our branch instruction.
			HccAMLWord* words = w->amlgen.function->words;
			HccAMLWord* dst_basic_block_word_ptr = &operands[0];
			uint32_t dst_basic_block_word_id = (dst_basic_block_word_ptr - words) + 1;

			// link the operand together in a list with the other continue statement branch instructions
			if (w->amlgen.continue_stmt_list_prev_id) {
				HccAMLWord* prev_linked_word = &words[w->amlgen.continue_stmt_list_prev_id - 1];
				*prev_linked_word = dst_basic_block_word_id;
			} else {
				w->amlgen.continue_stmt_list_head_id = dst_basic_block_word_id;
			}
			w->amlgen.continue_stmt_list_prev_id = dst_basic_block_word_id;
			break;
		};
		case HCC_AST_EXPR_TYPE_STMT_RETURN: {
			HccAMLOperand last_operand = hcc_amlgen_generate_instrs(w, expr->return_.expr, false);
			HccAMLOperand* operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_RETURN, 1);
			operands[0] = last_operand;
			return 0;
		};
		case HCC_AST_EXPR_TYPE_STMT_BLOCK: {
			HccASTExpr* stmt = expr->stmt_block.first_stmt;
			while (stmt) {
				hcc_amlgen_generate_instrs(w, stmt, false);
				stmt = stmt->next_stmt;
			}
			break;
		};
		default: HCC_ABORT("unhandled expr type: %u\n", expr->type);
	}

	return 0;
}

HccAMLOperand hcc_amlgen_generate_instrs_condition(HccWorker* w, HccASTExpr* cond_expr) {
	HccAMLOperand cond_operand = hcc_amlgen_generate_instrs(w, cond_expr, false);
	HccDataType cond_data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, cond_operand);
	if (cond_data_type != HCC_DATA_TYPE_AML_INTRINSIC_BOOL) {
		return hcc_amlgen_generate_convert_to_bool(w, cond_expr->location, cond_operand, cond_data_type, false);
	}

	return cond_operand;
}

HccAMLOperand hcc_amlgen_generate_instr_access_chain(HccWorker* w, HccASTExpr* expr, uint32_t count, bool want_variable_ref) {
	HccAMLOperand result_operand = 0;
	uint32_t child_count = 0;
	bool child_want_variable_ref = want_variable_ref;
	if (expr->type == HCC_AST_EXPR_TYPE_BINARY_OP) {
		switch (expr->binary.op) {
			case HCC_AST_BINARY_OP_FIELD_ACCESS:
			case HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT: {
				HccASTExpr* left_expr = expr->binary.left_expr;
				HccDataType left_data_type = hcc_data_type_lower_ast_to_aml(w->cu, left_expr->data_type);
				HccDataType compound_data_type = left_data_type;
				if (expr->binary.op == HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT) {
					compound_data_type = hcc_data_type_strip_pointer(w->cu, left_data_type);
				}

				HccCompoundDataType* dt;
				HccCompoundField* field;
				uint32_t field_idx;
				bool is_bitfield;
				bool is_swizzle;
				if (HCC_DATA_TYPE_IS_COMPOUND(compound_data_type)) {
					dt = hcc_compound_data_type_get(w->cu, compound_data_type);
					field = &dt->fields[expr->binary.field_idx];
					is_bitfield = field->is_bitfield;
					field_idx = field->storage_field_idx;
					is_swizzle = false;
				} else if (HCC_DATA_TYPE_IS_VECTOR(compound_data_type)) {
					field_idx = expr->binary.field_idx;
					is_bitfield = false;
					is_swizzle = field_idx >= 4;
				} else {
					HCC_UNREACHABLE();
				}

				//
				// if we have '->' operator and/or a union, we want to end the generation of the access chain here
				// so we can load and/or bitcast the pointer.
				if (expr->binary.op == HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT || HCC_DATA_TYPE_IS_UNION(left_data_type) || is_bitfield || is_swizzle) {
					child_count = 0;
					child_want_variable_ref = expr->binary.op != HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT;
				} else {
					child_count = count + 1;
				}

				//
				// recursive down the left expression tree and generate the access chain
				result_operand = hcc_amlgen_generate_instr_access_chain(w, left_expr, child_count, child_want_variable_ref);

				if (expr->binary.op == HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT || HCC_DATA_TYPE_IS_UNION(left_data_type) || is_bitfield || is_swizzle) {
					hcc_amlgen_generate_instr_access_chain_end(w, left_data_type);

					if (is_swizzle) {
						HccSwizzle swizzle = field_idx - 4;
						if (w->amlgen.assignee_operand) {
							return hcc_amlgen_generate_swizzle_store(w, expr->location, swizzle, left_data_type, result_operand);
						} else {
							uint32_t dst_elmts_count = hcc_swizzle_num_elmts_dst(swizzle);
							HccDataType dst_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->data_type);
							HccAMLOperand dst_operand = hcc_amlgen_value_add(w, dst_data_type);

							uint32_t indices[4];
							hcc_swizzle_extract_indices(swizzle, indices);

							result_operand = hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, left_data_type), result_operand);
							HccAMLOperand* operands = hcc_amlgen_instr_add(w, expr->location, HCC_AML_OP_SHUFFLE, 3 + dst_elmts_count);
							operands[0] = dst_operand;
							operands[1] = result_operand;
							operands[2] = result_operand;
							for (uint32_t idx = 0; idx < dst_elmts_count; idx += 1) {
								operands[3 + idx] = indices[idx];
							}
							return dst_operand;
						}
					} else if (is_bitfield) {
						HCC_DEBUG_ASSERT(count == 0, "expected bitfield to be the last in the access chain");
						HCC_DEBUG_ASSERT(!want_variable_ref, "expected bitfield not the be wanted as a reference");

						HccAMLOperand base_ptr_operand = result_operand;
						result_operand = 0;
						if (w->amlgen.assignee_operand) {
							return hcc_amlgen_generate_bitfield_store(w, expr, dt, field, base_ptr_operand);
						} else {
							result_operand = hcc_amlgen_generate_bitfield_load(w, expr, dt, field, base_ptr_operand);
						}
						return result_operand;
					} else {
						if (HCC_DATA_TYPE_IS_UNION(left_data_type)) {
							//
							// bitcast if we are accessing a union field
							result_operand = hcc_amlgen_generate_bitcast_union_field(w, expr->location, left_data_type, field_idx, result_operand);
						}

						result_operand = hcc_amlgen_generate_instr_access_chain_start(w, expr->location, HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS, result_operand, count + !HCC_DATA_TYPE_IS_UNION(left_data_type));
					}
				}

				//
				// poke the field index in if it is a struct field access aka. wasn't handled via a union bitcast
				if (!HCC_DATA_TYPE_IS_UNION(left_data_type)) {
					HccBasic basic = hcc_basic_from_sint(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_S32, field_idx);
					HccConstantId constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_S32, &basic);
					hcc_amlgen_generate_instr_access_chain_set_next_operand(w, HCC_AML_OPERAND(CONSTANT, constant_id.idx_plus_one));
				}

				goto END;
			};
			case HCC_AST_BINARY_OP_ARRAY_SUBSCRIPT: {
				//
				// generate the indexee from the right expression
				HccAMLOperand right_operand = hcc_amlgen_generate_instrs(w, expr->binary.right_expr, false);

				HccASTExpr* left_expr = expr->binary.left_expr;
				HccDataType left_data_type = hcc_data_type_lower_ast_to_aml(w->cu, left_expr->data_type);

				//
				// if we are indexing a pointer data type, we want to end the generation of the access chain here
				// so we can load the pointer.
				uint32_t child_count;
				if (HCC_DATA_TYPE_IS_POINTER(left_data_type) || HCC_DATA_TYPE_IS_RESOURCE(left_data_type)) {
					child_count = 0;
					child_want_variable_ref = false;
				} else {
					child_count = count + 1;
				}

				//
				// recursive down the left expression tree and generate the access chain
				result_operand = hcc_amlgen_generate_instr_access_chain(w, left_expr, child_count, child_want_variable_ref);

				if (HCC_DATA_TYPE_IS_POINTER(left_data_type) || HCC_DATA_TYPE_IS_RESOURCE(left_data_type)) {
					hcc_amlgen_generate_instr_access_chain_end(w, left_data_type);

					//
					// load in pointer if we have are indexing one and start the next access chain
					if (HCC_DATA_TYPE_IS_RESOURCE(left_data_type)) {
						result_operand = hcc_amlgen_generate_resource_descriptor_load(w, expr->location, result_operand);
					}
					result_operand = hcc_amlgen_generate_instr_access_chain_start(w, expr->location, HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS, result_operand, count + 1);
				}

				hcc_amlgen_generate_instr_access_chain_set_next_operand(w, right_operand);
				goto END;
			};
			default: break;
		}
	}

	HccAMLOperand variable_ref = hcc_amlgen_generate_instrs(w, expr, true);
	if (count == 0) {
		return variable_ref;
	}

	result_operand = hcc_amlgen_generate_instr_access_chain_start(w, expr->location, HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS, variable_ref, count);

END: {}

	if (count == 0) {
		HccDataType dst_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->data_type);
		hcc_amlgen_generate_instr_access_chain_end(w, dst_data_type);

		if (!want_variable_ref) {
			result_operand = hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, dst_data_type), result_operand);
		}
	}

	return result_operand;
}

HccAMLOperand hcc_amlgen_generate_instr_access_chain_start(HccWorker* w, HccLocation* location, HccAMLOp op, HccAMLOperand base_ptr_operand, uint32_t count) {
	if (count == 0) {
		return base_ptr_operand;
	}

	uint32_t extra_operands_count;
	switch (op) {
		case HCC_AML_OP_PTR_ACCESS_CHAIN:
		case HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS:
			extra_operands_count = 2;
			break;
		default: HCC_ABORT("unexpected access chain op: %u\n", op);
	}

	HccAMLOperand* operands = hcc_amlgen_instr_add(w, location, op, count + extra_operands_count);

	switch (op) {
		case HCC_AML_OP_PTR_ACCESS_CHAIN:
		case HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS:
			operands[0] = hcc_amlgen_value_add(w, (HccDataType)0);
			operands[1] = base_ptr_operand;
			break;
		default: HCC_ABORT("unexpected access chain op: %u\n", op);
	}

	w->amlgen.access_chain_op = op;
	w->amlgen.access_chain_operands = operands;
	w->amlgen.access_chain_operand_idx = extra_operands_count;
	return operands[0];
}

void hcc_amlgen_generate_instr_access_chain_set_next_operand(HccWorker* w, HccAMLOperand operand) {
	w->amlgen.access_chain_operands[w->amlgen.access_chain_operand_idx] = operand;
	w->amlgen.access_chain_operand_idx += 1;
}

void hcc_amlgen_generate_instr_access_chain_end(HccWorker* w, HccDataType dst_data_type) {
	if (w->amlgen.access_chain_operands == NULL) {
		return;
	}

	//
	// patch operands count of the the last instruction, so it matches how many we actually added
	HccAMLInstr* instr = HCC_AML_OPERANDS_TO_INSTR(w->amlgen.access_chain_operands);
	uint32_t expected_operands_count = HCC_AML_INSTR_OPERANDS_COUNT(instr);
	uint32_t operands_count = w->amlgen.access_chain_operand_idx;
	*instr = HCC_AML_INSTR(HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS, operands_count);
	w->amlgen.function->words_count -= expected_operands_count - operands_count;

	switch (w->amlgen.access_chain_op) {
		case HCC_AML_OP_PTR_ACCESS_CHAIN:
		case HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS:
			w->amlgen.function->values[HCC_AML_OPERAND_AUX(w->amlgen.access_chain_operands[0])].data_type = hcc_pointer_data_type_deduplicate(w->cu, dst_data_type);
			w->amlgen.access_chain_operands = NULL;
			break;
		default: HCC_ABORT("unexpected access chain op: %u\n", w->amlgen.access_chain_op);
	}
}

HccAMLOperand hcc_amlgen_generate_swizzle_store(HccWorker* w, HccLocation* location, HccSwizzle swizzle, HccDataType dst_data_type, HccAMLOperand dst_ptr_operand) {
	uint32_t dst_elmts_count = HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(HCC_DATA_TYPE_AUX(dst_data_type));
	HccAMLOperand dst_operand = hcc_amlgen_value_add(w, dst_data_type);

	uint32_t indices[4];
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;
	if (swizzle <= HCC_SWIZZLE_WW) {
		swizzle -= HCC_SWIZZLE_XX;
		indices[swizzle % 4] = dst_elmts_count + 1;
		indices[swizzle / 4] = dst_elmts_count + 0;
	} else if (swizzle <= HCC_SWIZZLE_WWW) {
		swizzle -= HCC_SWIZZLE_XXX;
		int shifted = swizzle / 4;
		indices[swizzle % 4] = dst_elmts_count + 2;
		indices[shifted % 4] = dst_elmts_count + 1;
		indices[shifted / 4] = dst_elmts_count + 0;
	} else {
		swizzle -= HCC_SWIZZLE_XXXX;
		int shifted = swizzle / 4;
		int shifted_shifted = shifted / 4;
		indices[swizzle % 4] = dst_elmts_count + 3;
		indices[shifted % 4] = dst_elmts_count + 2;
		indices[shifted_shifted % 4] = dst_elmts_count + 1;
		indices[shifted_shifted / 4] = dst_elmts_count + 0;
	}

	HccAMLOperand loaded_operand = hcc_amlgen_instr_add_2(w, location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, dst_data_type), dst_ptr_operand);
	HccAMLOperand* operands = hcc_amlgen_instr_add(w, location, HCC_AML_OP_SHUFFLE, 3 + dst_elmts_count);
	operands[0] = dst_operand;
	operands[1] = loaded_operand;
	operands[2] = w->amlgen.assignee_operand;
	for (uint32_t idx = 0; idx < dst_elmts_count; idx += 1) {
		operands[3 + idx] = indices[idx];
	}

	hcc_amlgen_instr_add_2(w, location, HCC_AML_OP_PTR_STORE, dst_ptr_operand, dst_operand);
	return dst_operand;
}

HccAMLOperand hcc_amlgen_generate_bitfield_load(HccWorker* w, HccASTExpr* expr, HccCompoundDataType* dt, HccCompoundField* field, HccAMLOperand base_ptr_operand) {
	HccDataType result_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->data_type);
	HccAMLOperand result_operand = 0;
	HccDataType unsigned_result_data_type = result_data_type;
	if (hcc_basic_type_class(w->cu, result_data_type) == HCC_BASIC_TYPE_CLASS_SINT) {
		unsigned_result_data_type = hcc_data_type_signed_to_unsigned(w->cu, result_data_type);
	}

	uint32_t storage_field_end_idx = field->storage_field_idx + field->storage_fields_count;
	uint32_t bit_offset = field->bit_offset;
	uint32_t bits_count = field->bits_count;
	uint32_t dst_bit_offset = 0;
	for (uint32_t storage_field_idx = field->storage_field_idx; storage_field_idx < storage_field_end_idx; storage_field_idx += 1) {
		HccCompoundField* storage_field = &dt->storage_fields[storage_field_idx];
		HccDataType storage_field_data_type = hcc_data_type_lower_ast_to_aml(w->cu, storage_field->data_type);
		HccDataType storage_field_ptr_data_type = hcc_pointer_data_type_deduplicate(w->cu, storage_field_data_type);

		HccBasic storage_field_idx_basic = hcc_basic_from_uint(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_S32, storage_field_idx);
		HccConstantId storage_field_idx_constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_S32, &storage_field_idx_basic);

		HccAMLOperand storage_field_ptr_operand = hcc_amlgen_value_add(w, storage_field_ptr_data_type);
		hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS,
			storage_field_ptr_operand, base_ptr_operand, HCC_AML_OPERAND(CONSTANT, storage_field_idx_constant_id.idx_plus_one));

		HccAMLOperand storage_field_operand = hcc_amlgen_value_add(w, storage_field_data_type);
		hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD,
			storage_field_operand, storage_field_ptr_operand);

		uint64_t size, align;
		hcc_data_type_size_align(w->cu, storage_field_data_type, &size, &align);
		uint32_t field_storage_field_bits_count = HCC_MIN(size * 8, bits_count);

		uint64_t mask = (((uint64_t)1 << field_storage_field_bits_count) - 1) << bit_offset;
		HccBasic mask_basic = hcc_basic_from_uint(w->cu, storage_field_data_type, mask);
		HccConstantId mask_constant_id = hcc_constant_table_deduplicate_basic(w->cu, storage_field_data_type, &mask_basic);

		HccAMLOperand and_result_operand = hcc_amlgen_value_add(w, storage_field_data_type);
		hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_AND,
			and_result_operand, storage_field_operand, HCC_AML_OPERAND(CONSTANT, mask_constant_id.idx_plus_one));

		HccAMLOperand converted_and_result_operand;
		if (unsigned_result_data_type == storage_field_data_type) {
			converted_and_result_operand = and_result_operand;
		} else {
			converted_and_result_operand = hcc_amlgen_value_add(w, unsigned_result_data_type);
			hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_CONVERT, converted_and_result_operand, and_result_operand);
		}

		HccAMLOperand shifted_result_operand;
		if (dst_bit_offset == bit_offset) {
			shifted_result_operand = converted_and_result_operand;
		} else {
			uint64_t shift_amount;
			HccAMLOp aml_op;
			if (dst_bit_offset < bit_offset) {
				shift_amount = bit_offset - dst_bit_offset;
				aml_op = HCC_AML_OP_BIT_SHIFT_RIGHT;
			} else {
				shift_amount = dst_bit_offset - bit_offset;
				aml_op = HCC_AML_OP_BIT_SHIFT_LEFT;
			}

			HccBasic shift_basic = hcc_basic_from_uint(w->cu, unsigned_result_data_type, shift_amount);
			HccConstantId shift_constant_id = hcc_constant_table_deduplicate_basic(w->cu, unsigned_result_data_type, &shift_basic);

			shifted_result_operand = hcc_amlgen_value_add(w, unsigned_result_data_type);
			hcc_amlgen_instr_add_3(w, expr->location, aml_op,
				shifted_result_operand, converted_and_result_operand, HCC_AML_OPERAND(CONSTANT, shift_constant_id.idx_plus_one));
		}

		if (result_operand) {
			HccAMLOperand ord_result_operand = hcc_amlgen_value_add(w, unsigned_result_data_type);
			hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_OR,
				ord_result_operand, result_operand, shifted_result_operand);

			result_operand = ord_result_operand;
		} else {
			result_operand = shifted_result_operand;
		}

		bit_offset = 0;
		bits_count -= field_storage_field_bits_count;
		dst_bit_offset += field_storage_field_bits_count;
	}

	if (unsigned_result_data_type != result_data_type) {
		HccAMLOperand converted_result_operand = hcc_amlgen_value_add(w, result_data_type);
		hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_CONVERT, converted_result_operand, result_operand);
		result_operand = converted_result_operand;
	}

	return result_operand;
}

HccAMLOperand hcc_amlgen_generate_bitfield_store(HccWorker* w, HccASTExpr* expr, HccCompoundDataType* dt, HccCompoundField* field, HccAMLOperand base_ptr_operand) {
	HccDataType result_data_type = hcc_data_type_lower_ast_to_aml(w->cu, expr->data_type);
	HccAMLOperand result_operand = 0;
	HccDataType field_data_type = hcc_data_type_lower_ast_to_aml(w->cu, field->data_type);

	HccAMLOperand src_operand = w->amlgen.assignee_operand;
	uint32_t storage_field_end_idx = field->storage_field_idx + field->storage_fields_count;
	uint32_t bit_offset = field->bit_offset;
	uint32_t bits_count = field->bits_count;
	uint32_t src_bit_offset = 0;
	for (uint32_t storage_field_idx = field->storage_field_idx; storage_field_idx < storage_field_end_idx; storage_field_idx += 1) {
		HccCompoundField* storage_field = &dt->storage_fields[storage_field_idx];
		HccDataType storage_field_data_type = hcc_data_type_lower_ast_to_aml(w->cu, storage_field->data_type);
		HccDataType storage_field_ptr_data_type = hcc_pointer_data_type_deduplicate(w->cu, storage_field_data_type);

		HccDataType unsigned_storage_field_data_type = storage_field_data_type;
		if (hcc_basic_type_class(w->cu, storage_field_data_type) == HCC_BASIC_TYPE_CLASS_SINT) {
			unsigned_storage_field_data_type = hcc_data_type_signed_to_unsigned(w->cu, storage_field_data_type);
		}

		HccBasic storage_field_idx_basic = hcc_basic_from_uint(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_S32, storage_field_idx);
		HccConstantId storage_field_idx_constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AML_INTRINSIC_S32, &storage_field_idx_basic);

		HccAMLOperand storage_field_ptr_operand = hcc_amlgen_value_add(w, storage_field_ptr_data_type);
		hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS,
			storage_field_ptr_operand, base_ptr_operand, HCC_AML_OPERAND(CONSTANT, storage_field_idx_constant_id.idx_plus_one));

		HccAMLOperand storage_field_operand = hcc_amlgen_value_add(w, storage_field_data_type);
		hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_LOAD,
			storage_field_operand, storage_field_ptr_operand);

		uint64_t size, align;
		hcc_data_type_size_align(w->cu, storage_field_data_type, &size, &align);
		uint32_t field_storage_field_bits_count = HCC_MIN(size * 8, bits_count);

		uint64_t mask = (((uint64_t)1 << field_storage_field_bits_count) - 1) << bit_offset;
		HccBasic mask_basic = hcc_basic_from_uint(w->cu, storage_field_data_type, mask);
		HccConstantId mask_constant_id = hcc_constant_table_deduplicate_basic(w->cu, storage_field_data_type, &mask_basic);

		uint64_t clear_mask = ~((((uint64_t)1 << field_storage_field_bits_count) - 1) << bit_offset);
		HccBasic clear_mask_basic = hcc_basic_from_uint(w->cu, storage_field_data_type, clear_mask);
		HccConstantId clear_mask_constant_id = hcc_constant_table_deduplicate_basic(w->cu, storage_field_data_type, &clear_mask_basic);

		HccAMLOperand and_operand = hcc_amlgen_value_add(w, storage_field_data_type);
		hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_AND,
			and_operand, storage_field_operand, HCC_AML_OPERAND(CONSTANT, clear_mask_constant_id.idx_plus_one));

		HccAMLOperand converted_src_operand;
		if (hcc_aml_operand_data_type(w->cu, w->amlgen.function, src_operand) == unsigned_storage_field_data_type) {
			converted_src_operand = src_operand;
		} else {
			converted_src_operand = hcc_amlgen_value_add(w, unsigned_storage_field_data_type);
			hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_CONVERT, converted_src_operand, src_operand);
		}

		HccAMLOperand shifted_src_operand;
		if (src_bit_offset == bit_offset) {
			shifted_src_operand = converted_src_operand;
		} else {
			uint64_t shift_amount;
			HccAMLOp aml_op;
			if (src_bit_offset < bit_offset) {
				shift_amount = bit_offset - src_bit_offset;
				aml_op = HCC_AML_OP_BIT_SHIFT_LEFT;
			} else {
				shift_amount = src_bit_offset - bit_offset;
				aml_op = HCC_AML_OP_BIT_SHIFT_RIGHT;
			}

			HccBasic shift_basic = hcc_basic_from_uint(w->cu, unsigned_storage_field_data_type, shift_amount);
			HccConstantId shift_constant_id = hcc_constant_table_deduplicate_basic(w->cu, unsigned_storage_field_data_type, &shift_basic);

			shifted_src_operand = hcc_amlgen_value_add(w, unsigned_storage_field_data_type);
			hcc_amlgen_instr_add_3(w, expr->location, aml_op,
				shifted_src_operand, converted_src_operand, HCC_AML_OPERAND(CONSTANT, shift_constant_id.idx_plus_one));

			if (unsigned_storage_field_data_type != storage_field_data_type) {
				HccAMLOperand converted_storage_field_operand = hcc_amlgen_value_add(w, storage_field_data_type);
				hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_CONVERT, converted_storage_field_operand, shifted_src_operand);
				shifted_src_operand = converted_storage_field_operand;
			}
		}

		HccAMLOperand and_src_operand = hcc_amlgen_value_add(w, storage_field_data_type);
		hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_AND,
			and_src_operand, shifted_src_operand, HCC_AML_OPERAND(CONSTANT, mask_constant_id.idx_plus_one));

		HccAMLOperand ord_operand = hcc_amlgen_value_add(w, storage_field_data_type);
		hcc_amlgen_instr_add_3(w, expr->location, HCC_AML_OP_BIT_OR,
			ord_operand, and_operand, and_src_operand);

		hcc_amlgen_instr_add_2(w, expr->location, HCC_AML_OP_PTR_STORE,
			storage_field_ptr_operand, ord_operand);

		bit_offset = 0;
		bits_count -= field_storage_field_bits_count;
		src_bit_offset += field_storage_field_bits_count;
	}

	return src_operand;
}

HccAMLOperand hcc_amlgen_generate_bitcast_union_field(HccWorker* w, HccLocation* location, HccDataType union_data_type, uint32_t storage_field_idx, HccAMLOperand union_ptr_operand) {
	HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(w->cu, union_data_type);
	HccCompoundField* field = &compound_data_type->storage_fields[storage_field_idx];
	HccDataType field_data_type = hcc_data_type_lower_ast_to_aml(w->cu, field->data_type);
	HccDataType dst_data_type = hcc_pointer_data_type_deduplicate(w->cu, field_data_type);
	return hcc_amlgen_instr_add_2(w, location, HCC_AML_OP_BITCAST,
		hcc_amlgen_value_add(w, dst_data_type),
		union_ptr_operand);
}

HccAMLOperand hcc_amlgen_generate_resource_descriptor_load(HccWorker* w, HccLocation* location, HccAMLOperand operand) {
	HccDataType resource_data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, operand);
	if (HCC_DATA_TYPE_IS_POINTER(resource_data_type)) {
		resource_data_type = hcc_data_type_strip_pointer(w->cu, resource_data_type);
		operand = hcc_amlgen_instr_add_2(w, location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, resource_data_type), operand);
	}
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_RESOURCE(resource_data_type), "expected data type to be a resource data type but got %u", resource_data_type);
	HccDataType descriptor_data_type = HCC_DATA_TYPE(RESOURCE_DESCRIPTOR, HCC_DATA_TYPE_AUX(resource_data_type));
	return hcc_amlgen_instr_add_2(w, location, HCC_AML_OP_RESOURCE_DESCRIPTOR_LOAD, hcc_amlgen_value_add(w, descriptor_data_type), operand);
}

HccAMLOperand hcc_amlgen_generate_resource_descriptor_addr(HccWorker* w, HccLocation* location, HccAMLOperand operand) {
	HccDataType resource_data_type = hcc_aml_operand_data_type(w->cu, w->amlgen.function, operand);
	if (HCC_DATA_TYPE_IS_POINTER(resource_data_type)) {
		resource_data_type = hcc_data_type_strip_pointer(w->cu, resource_data_type);
		operand = hcc_amlgen_instr_add_2(w, location, HCC_AML_OP_PTR_LOAD, hcc_amlgen_value_add(w, resource_data_type), operand);
	}
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_RESOURCE(resource_data_type), "expected data type to be a resource data type but got %u", resource_data_type);
	HccDataType descriptor_data_type = HCC_DATA_TYPE(RESOURCE_DESCRIPTOR, HCC_DATA_TYPE_AUX(resource_data_type));
	return hcc_amlgen_instr_add_2(w, location, HCC_AML_OP_RESOURCE_DESCRIPTOR_ADDR, hcc_amlgen_value_add(w, descriptor_data_type), operand);
}

void hcc_amlgen_generate(HccWorker* w) {
	HccDecl function_decl = (HccDecl)(uintptr_t)w->job.arg;
	HccASTFunction* ast_function = hcc_ast_function_get(w->cu, function_decl);

	HccAMLFunction* function = hcc_aml_function_alctor_alloc(w->cu, ast_function->max_instrs_count);
	function->identifier_location = ast_function->identifier_location;
	function->identifier_string_id = ast_function->identifier_string_id;
	function->function_data_type = ast_function->function_data_type;
	function->return_data_type = hcc_data_type_lower_ast_to_aml(w->cu, ast_function->return_data_type);
	function->shader_stage = ast_function->shader_stage;
	function->params_count = ast_function->params_count;
	function->compute_dispatch_group_size_x = ast_function->compute_dispatch_group_size_x;
	function->compute_dispatch_group_size_y = ast_function->compute_dispatch_group_size_y;
	function->compute_dispatch_group_size_z = ast_function->compute_dispatch_group_size_z;
	function->opt_level = ast_function->opt_level;
	*hcc_stack_get(w->cu->aml.functions, HCC_DECL_AUX(function_decl)) = function;
	w->amlgen.function_decl = function_decl;
	w->amlgen.ast_function = ast_function;
	w->amlgen.function = function;

	//
	// immutable parameters
	for (uint32_t param_idx = 0; param_idx < ast_function->params_count; param_idx += 1) {
		HccASTVariable* variable = &ast_function->params_and_variables[param_idx];
		HccDataType data_type = hcc_data_type_lower_ast_to_aml(w->cu, variable->data_type);
		hcc_amlgen_value_add(w, data_type);
	}

	HccASTExpr* block_expr = ast_function->block_expr;
	hcc_amlgen_basic_block_add(w, block_expr->location);

	//
	// mutable parameters
	for (uint32_t variable_idx = 0; variable_idx < ast_function->params_count; variable_idx += 1) {
		HccASTVariable* variable = &ast_function->params_and_variables[variable_idx];
		if (HCC_DATA_TYPE_IS_CONST(variable->data_type)) {
			HccAMLOperand value_operand = hcc_amlgen_value_add(w, 0); // add a dummy value if parameter was marked as const
		} else {
			HccDataType data_type = hcc_data_type_lower_ast_to_aml(w->cu, variable->data_type);
			HccDataType ptr_data_type = hcc_pointer_data_type_deduplicate(w->cu, data_type);
			HccAMLOperand value_operand = hcc_amlgen_value_add(w, ptr_data_type);
			hcc_amlgen_instr_add_2(w, variable->identifier_location, HCC_AML_OP_PTR_STATIC_ALLOC, value_operand, data_type);
		}
	}

	//
	// store the value from the immutable parameters in the mutable parameters
	for (uint32_t param_idx = 0; param_idx < ast_function->params_count; param_idx += 1) {
		HccASTVariable* variable = &ast_function->params_and_variables[param_idx];
		if (!HCC_DATA_TYPE_IS_CONST(variable->data_type)) {
			HccAMLOperand dst_operand = hcc_amlgen_local_variable_operand(w, param_idx);
			HccAMLOperand src_operand = HCC_AML_OPERAND(VALUE, param_idx);
			hcc_amlgen_instr_add_2(w, variable->identifier_location, HCC_AML_OP_PTR_STORE, dst_operand, src_operand);
		}
	}

	//
	// variables
	for (uint32_t variable_idx = ast_function->params_count; variable_idx < ast_function->variables_count; variable_idx += 1) {
		HccASTVariable* variable = &ast_function->params_and_variables[variable_idx];
		HccDataType data_type = hcc_data_type_lower_ast_to_aml(w->cu, variable->data_type);
		HccDataType ptr_data_type = hcc_pointer_data_type_deduplicate(w->cu, data_type);
		HccAMLOperand value_operand = hcc_amlgen_value_add(w, ptr_data_type);
		hcc_amlgen_instr_add_2(w, variable->identifier_location, HCC_AML_OP_PTR_STATIC_ALLOC, value_operand, data_type);
	}

	hcc_amlgen_generate_instrs(w, block_expr, false);

	if (w->amlgen.last_op != HCC_AML_OP_RETURN) {
		if (ast_function->return_data_type == 0) {
			HccAMLOperand* operands = hcc_amlgen_instr_add(w, w->amlgen.last_location, HCC_AML_OP_RETURN, 1);
			operands[0] = 0;
		} else {
			hcc_amlgen_instr_add(w, w->amlgen.last_location, HCC_AML_OP_UNREACHABLE, 0);
		}
	}

	HccStack(HccDecl) optimize_functions = hcc_aml_optimize_functions(w->cu);
	*hcc_stack_push_thread_safe(optimize_functions) = function_decl;
}

