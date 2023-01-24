#include "hcc_internal.h"

// ===========================================
//
//
// SPIR-V
//
//
// ===========================================

void hcc_spirv_init(HccCU* cu, HccCUSetup* setup) {
	HCC_UNUSED(setup);
	cu->spirv.next_spirv_id = HCC_SPIRV_ID_USER_START;

	cu->spirv.functions = hcc_stack_init(HccSPIRVFunction, HCC_ALLOC_TAG_SPIRV_FUNCTIONS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->spirv.function_words = hcc_stack_init(HccSPIRVWord, HCC_ALLOC_TAG_SPIRV_FUNCTION_WORDS, (uint32_t)(setup->aml.function_alctor.instrs_grow_count * HCC_AML_INSTR_AVERAGE_WORDS), (uint32_t)(setup->aml.function_alctor.instrs_reserve_cap * HCC_AML_INSTR_AVERAGE_WORDS));
	uint32_t types_reserve_cap           =
		setup->dtt.arrays_reserve_cap    +
		setup->dtt.compounds_reserve_cap +
		setup->dtt.pointers_reserve_cap  +
		setup->dtt.buffers_reserve_cap   ;
	uint32_t types_grow_count           =
		setup->dtt.arrays_grow_count    +
		setup->dtt.compounds_grow_count +
		setup->dtt.pointers_grow_count  +
		setup->dtt.buffers_grow_count   ;
	cu->spirv.type_table = hcc_hash_table_init(HccSPIRVTypeEntry, HCC_ALLOC_TAG_SPIRV_TYPE_TABLE, hcc_spirv_type_key_cmp, hcc_spirv_type_key_hash, types_reserve_cap);
	uint32_t decl_table_entries_cap            =
		setup->functions_reserve_cap           +
		setup->ast.global_variables_reserve_cap;
	cu->spirv.decl_table = hcc_hash_table_init(HccSPIRVDeclEntry, HCC_ALLOC_TAG_SPIRV_DECL_TABLE, hcc_u32_key_cmp, hcc_u32_key_hash, decl_table_entries_cap);
	cu->spirv.constant_table = hcc_hash_table_init(HccSPIRVConstantEntry, HCC_ALLOC_TAG_SPIRV_CONSTANT_TABLE, hcc_u32_key_cmp, hcc_u32_key_hash, setup->constant_table.entries_cap);
	cu->spirv.types_and_constants = hcc_stack_init(HccSPIRVTypeOrConstant, HCC_ALLOC_TAG_SPIRV_TYPES_AND_CONSTANTS, types_grow_count, types_reserve_cap + setup->constant_table.entries_cap);
	cu->spirv.type_elmt_ids = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_TYPE_ELMT_IDS, setup->dtt.compound_fields_grow_count, setup->dtt.compound_fields_reserve_cap);
	cu->spirv.entry_points = hcc_stack_init(HccSPIRVEntryPoint, HCC_ALLOC_TAG_SPIRV_ENTRY_POINTS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->spirv.entry_point_global_variable_ids = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_ENTRY_POINT_GLOBAL_VARIABLE_IDS, setup->ast.global_variables_grow_count, setup->ast.global_variables_reserve_cap);
	cu->spirv.global_variable_words = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_GLOBAL_VARIABLE_WORDS, setup->ast.global_variables_grow_count * 4, setup->ast.global_variables_reserve_cap * 4);
	cu->spirv.decorate_words = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_DECORATE_WORDS, setup->ast.global_variables_grow_count * 4, setup->ast.global_variables_reserve_cap * 4);
}

HccSPIRVId hcc_spirv_next_id(HccCU* cu) {
	return atomic_fetch_add(&cu->spirv.next_spirv_id, 1);
}

HccSPIRVId hcc_spirv_next_id_many(HccCU* cu, uint32_t amount) {
	return atomic_fetch_add(&cu->spirv.next_spirv_id, amount);
}

HccSPIRVId hcc_spirv_type_deduplicate(HccCU* cu, HccSPIRVStorageClass storage_class, HccDataType data_type) {
	data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
	data_type = HCC_DATA_TYPE_STRIP_QUALIFIERS(data_type);
	if (HCC_DATA_TYPE_TYPE(data_type) != HCC_DATA_TYPE_POINTER) {
		storage_class = HCC_SPIRV_STORAGE_CLASS_INVALID;
	} else {
		HCC_DEBUG_ASSERT(storage_class != HCC_SPIRV_STORAGE_CLASS_INVALID, "pointer type cannot have an invalid storage class");
	}

	HccSPIRVTypeKey key = { .storage_class = storage_class, .data_type = data_type };
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->spirv.type_table, &key);
	HccSPIRVTypeEntry* entry = &cu->spirv.type_table[insert.idx];
	if (insert.is_new) {
		HccSPIRVOp op;
		HccSPIRVOperand* operands;
		uint32_t operands_count;
		switch (HCC_DATA_TYPE_TYPE(data_type)) {
			case HCC_DATA_TYPE_STRUCT: {
				HccCompoundDataType* dt = hcc_compound_data_type_get(cu, data_type);
				op = HCC_SPIRV_OP_TYPE_STRUCT;
				operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, dt->fields_count + 1);
				operands_count = dt->fields_count + 1;
				for (uint32_t field_idx = 0; field_idx < dt->fields_count; field_idx += 1) {
					HccCompoundField* field = &dt->fields[field_idx];
					operands[1 + field_idx] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, field->data_type);
				}
				break;
			};
			case HCC_DATA_TYPE_UNION: {
				HccCompoundDataType* dt = hcc_compound_data_type_get(cu, data_type);
				op = HCC_SPIRV_OP_TYPE_STRUCT;
				operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 2);
				operands[1] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, dt->fields[dt->largest_sized_field_idx].data_type);
				operands_count = 2;
				break;
			};
			case HCC_DATA_TYPE_ARRAY: {
				HccArrayDataType* dt = hcc_array_data_type_get(cu, data_type);
				op = HCC_SPIRV_OP_TYPE_ARRAY;
				operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 3);
				operands[1] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, dt->element_data_type);
				operands[2] = hcc_spirv_constant_deduplicate(cu, dt->element_count_constant_id);
				operands_count = 3;
				break;
			};
			case HCC_DATA_TYPE_POINTER: {
				HccPointerDataType* dt = hcc_pointer_data_type_get(cu, data_type);
				op = HCC_SPIRV_OP_TYPE_POINTER;
				operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 3);
				operands[1] = storage_class;
				operands[2] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, dt->element_data_type);
				operands_count = 3;
				break;
			};
			case HCC_DATA_TYPE_FUNCTION: {
				HccFunctionDataType* dt = hcc_function_data_type_get(cu, data_type);
				op = HCC_SPIRV_OP_TYPE_FUNCTION;
				operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 2 + dt->params_count);
				operands[1] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, dt->return_data_type);
				for (uint32_t param_idx = 0; param_idx < dt->params_count; param_idx += 1) {
					operands[2 + param_idx] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, dt->params[param_idx]);
				}
				operands_count = 2 + dt->params_count;
				break;
			};
			case HCC_DATA_TYPE_AML_INTRINSIC: {
				HccAMLIntrinsicDataType aml_intrinsic_type = HCC_DATA_TYPE_AUX(data_type);
				if (HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(aml_intrinsic_type) > 1 && HCC_AML_INTRINSIC_DATA_TYPE_ROWS(aml_intrinsic_type) > 1) {
					op = HCC_SPIRV_OP_TYPE_MATRIX;
					operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 3);
					HccAMLIntrinsicDataType vector_intrinsic_type = aml_intrinsic_type & ~HCC_AML_INTRINSIC_DATA_TYPE_ROWS_MASK;
					operands[1] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE(AML_INTRINSIC, vector_intrinsic_type));
					operands[2] = HCC_AML_INTRINSIC_DATA_TYPE_ROWS(aml_intrinsic_type);
					operands_count = 3;
				} else if (HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(aml_intrinsic_type) > 1) {
					op = HCC_SPIRV_OP_TYPE_VECTOR;
					operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 3);
					operands[1] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE(AML_INTRINSIC, HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(aml_intrinsic_type)));
					operands[2] = HCC_AML_INTRINSIC_DATA_TYPE_COLUMNS(aml_intrinsic_type);
					operands_count = 3;
				} else {
					switch (aml_intrinsic_type) {
						case HCC_AML_INTRINSIC_DATA_TYPE_VOID:
							op = HCC_SPIRV_OP_TYPE_VOID;
							operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 1);
							operands_count = 1;
							break;
						case HCC_AML_INTRINSIC_DATA_TYPE_BOOL:
							op = HCC_SPIRV_OP_TYPE_BOOL;
							operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 1);
							operands_count = 1;
							break;
						case HCC_AML_INTRINSIC_DATA_TYPE_S8:
						case HCC_AML_INTRINSIC_DATA_TYPE_S16:
						case HCC_AML_INTRINSIC_DATA_TYPE_S32:
						case HCC_AML_INTRINSIC_DATA_TYPE_S64:
						case HCC_AML_INTRINSIC_DATA_TYPE_U8:
						case HCC_AML_INTRINSIC_DATA_TYPE_U16:
						case HCC_AML_INTRINSIC_DATA_TYPE_U32:
						case HCC_AML_INTRINSIC_DATA_TYPE_U64:
							op = HCC_SPIRV_OP_TYPE_INT;
							operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 3);
							operands[1] = hcc_aml_intrinsic_data_type_scalar_size_aligns[aml_intrinsic_type] * 8;
							operands[2] = HCC_AML_INTRINSIC_DATA_TYPE_IS_SINT(aml_intrinsic_type);
							operands_count = 3;
							break;
						case HCC_AML_INTRINSIC_DATA_TYPE_F16:
						case HCC_AML_INTRINSIC_DATA_TYPE_F32:
						case HCC_AML_INTRINSIC_DATA_TYPE_F64:
							op = HCC_SPIRV_OP_TYPE_FLOAT;
							operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 2);
							operands[1] = hcc_aml_intrinsic_data_type_scalar_size_aligns[aml_intrinsic_type] * 8;
							operands_count = 2;
							break;
					}
				}
				break;
			};
			default: HCC_ABORT("unhandled data type type: %u", HCC_DATA_TYPE_TYPE(data_type));
		}

		HccSPIRVId spirv_id = hcc_spirv_next_id(cu);
		operands[0] = spirv_id;

		HccSPIRVTypeOrConstant* type = hcc_stack_push(cu->spirv.types_and_constants);
		type->op = op;
		type->operands = operands;
		type->operands_count = operands_count;

		atomic_store(&entry->spirv_id, spirv_id);
	} else {
		//
		// spin if another thread has just inserted the entry but not set the spirv id yet
		while (atomic_load(&entry->spirv_id) == 0) {
			HCC_CPU_RELAX();
		}
	}

	return atomic_load(&entry->spirv_id);
}

HccSPIRVId hcc_spirv_decl_deduplicate(HccCU* cu, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_TYPE(decl) == HCC_DECL_FUNCTION || HCC_DECL_TYPE(decl) == HCC_DECL_GLOBAL_VARIABLE, "expected decl to be a function or a global variable");

	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->spirv.decl_table, &decl);
	HccSPIRVDeclEntry* entry = &cu->spirv.decl_table[insert.idx];
	if (insert.is_new) {
		HccSPIRVId spirv_id = hcc_spirv_next_id(cu);

		switch (HCC_DECL_TYPE(decl)) {
			case HCC_DECL_GLOBAL_VARIABLE: {
				HccASTVariable* ast_global_variable = hcc_ast_global_variable_get(cu, decl);
				HccSPIRVOperand* operands = hcc_spirv_add_global_variable(cu, 4);
				HccSPIRVStorageClass storage_class = HCC_SPIRV_STORAGE_CLASS_PRIVATE; // TODO add HCC_SPIRV_STORAGE_CLASS_WORKGROUP support
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, hcc_pointer_data_type_deduplicate(cu, ast_global_variable->data_type));
				operands[1] = spirv_id;
				operands[2] = storage_class;
				operands[3] = hcc_spirv_constant_deduplicate(cu, ast_global_variable->initializer_constant_id);
				break;
			};
		}

		atomic_store(&entry->spirv_id, spirv_id);
	} else {
		//
		// spin if another thread has just inserted the entry but not set the spirv id yet
		while (atomic_load(&entry->spirv_id) == 0) {
			HCC_CPU_RELAX();
		}
	}

	return atomic_load(&entry->spirv_id);
}

HccSPIRVId hcc_spirv_constant_deduplicate(HccCU* cu, HccConstantId constant_id) {
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->spirv.constant_table, &constant_id);
	HccSPIRVConstantEntry* entry = &cu->spirv.constant_table[insert.idx];
	if (insert.is_new) {
		HccSPIRVId spirv_id = hcc_spirv_next_id(cu);
		HccConstant c = hcc_constant_table_get(cu, constant_id);

		HccSPIRVOp op;
		HccSPIRVOperand* operands;
		uint32_t operands_count;
		if (c.size == 0) {
			operands_count = 2;
			op = HCC_SPIRV_OP_CONSTANT_NULL;
			operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, operands_count);
		} else if (c.data_type == HCC_DATA_TYPE_AML_INTRINSIC_BOOL) {
			operands_count = 2;
			op = *(bool*)c.data ? HCC_SPIRV_OP_CONSTANT_TRUE : HCC_SPIRV_OP_CONSTANT_FALSE;
			operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, operands_count);
		} else if (HCC_DATA_TYPE_TYPE(c.data_type) == HCC_DATA_TYPE_AML_INTRINSIC) {
			HccAMLIntrinsicDataType intrin = HCC_DATA_TYPE_AUX(c.data_type);
			if (HCC_AML_INTRINSIC_DATA_TYPE_IS_SCALAR(intrin)) {
				HccSPIRVWord words[2];
				bool is_64 = false;
				switch (intrin) {
					case HCC_AML_INTRINSIC_DATA_TYPE_S8:
					case HCC_AML_INTRINSIC_DATA_TYPE_U8:
						words[0] = *(uint8_t*)c.data;
						break;
					case HCC_AML_INTRINSIC_DATA_TYPE_S16:
					case HCC_AML_INTRINSIC_DATA_TYPE_U16:
					case HCC_AML_INTRINSIC_DATA_TYPE_F16:
						words[0] = *(uint16_t*)c.data;
						break;
					case HCC_AML_INTRINSIC_DATA_TYPE_S32:
					case HCC_AML_INTRINSIC_DATA_TYPE_U32:
					case HCC_AML_INTRINSIC_DATA_TYPE_F32:
						words[0] = *(uint32_t*)c.data;
						break;
					case HCC_AML_INTRINSIC_DATA_TYPE_S64:
					case HCC_AML_INTRINSIC_DATA_TYPE_U64:
					case HCC_AML_INTRINSIC_DATA_TYPE_F64:
						is_64 = true;
						words[0] = ((uint32_t*)c.data)[0];
						words[1] = ((uint32_t*)c.data)[1];
						break;
					default: HCC_ABORT("unhandled constant AML data type %u", intrin);
				}

				operands_count = is_64 ? 4 : 3;
				op = HCC_SPIRV_OP_CONSTANT;
				operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, operands_count);
				operands[2] = words[0];
				if (is_64) {
					operands[3] = words[1];
				}
			} else {
				HCC_ABORT("unhandled constant AML data type %u", intrin);
			}
		} else {
			HCC_ABORT("unhandled constant data type %u", c.data_type);
		}

		operands[0] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, c.data_type);
		operands[1] = spirv_id;

		HccSPIRVTypeOrConstant* constant = hcc_stack_push(cu->spirv.types_and_constants);
		constant->op = op;
		constant->operands = operands;
		constant->operands_count = operands_count;

		atomic_store(&entry->spirv_id, spirv_id);
	} else {
		//
		// spin if another thread has just inserted the entry but not set the spirv id yet
		while (atomic_load(&entry->spirv_id) == 0) {
			HCC_CPU_RELAX();
		}
	}

	return atomic_load(&entry->spirv_id);
}

HccSPIRVStorageClass hcc_spirv_storage_class_from_aml_operand(HccCU* cu, HccAMLOperand aml_operand) {
	switch (HCC_AML_OPERAND_TYPE(aml_operand)) {
		case HCC_AML_OPERAND_VALUE:
			return HCC_SPIRV_STORAGE_CLASS_FUNCTION;

		case HCC_DECL_GLOBAL_VARIABLE: {
			HccASTVariable* variable = hcc_ast_global_variable_get(cu, (HccDecl)aml_operand);
			return variable->storage_duration == HCC_AST_STORAGE_DURATION_THREAD ? HCC_SPIRV_STORAGE_CLASS_PRIVATE : HCC_SPIRV_STORAGE_CLASS_WORKGROUP;
		};
		case HCC_DECL_LOCAL_VARIABLE: {
			HCC_ABORT("we shouldn't have access to local variables from the AST in the AML");
			break;
		};
		default:
			return HCC_SPIRV_STORAGE_CLASS_INVALID;
	}
}

HccSPIRVOperand* hcc_spirv_add_global_variable(HccCU* cu, uint32_t operands_count) {
	uint32_t words_count = operands_count + 1;
	HccSPIRVWord* words = hcc_stack_push_many(cu->spirv.global_variable_words, words_count);
	words[0] = (words_count << 16) | HCC_SPIRV_OP_VARIABLE;
	return &words[1];
}

HccSPIRVOperand* hcc_spirv_add_decorate(HccCU* cu, uint32_t operands_count) {
	uint32_t words_count = operands_count + 1;
	HccSPIRVWord* words = hcc_stack_push_many(cu->spirv.decorate_words, words_count);
	words[0] = (words_count << 16) | HCC_SPIRV_OP_DECORATE;
	return &words[1];
}

HccSPIRVOperand* hcc_spirv_function_add_instr(HccSPIRVFunction* function, HccSPIRVOp op, uint32_t operands_count) {
	uint32_t words_count = operands_count + 1;
	HCC_DEBUG_ASSERT_ARRAY_RESIZE(function->words_count + words_count, function->words_cap);
	HccSPIRVWord* words = &function->words[function->words_count];
	function->words_count += words_count;

	words[0] = (words_count << 16) | op;
	return &words[1];
}

bool hcc_spirv_type_key_cmp(void* a, void* b, uintptr_t size) {
	HCC_UNUSED(size);
	HccSPIRVTypeKey* a_ = a;
	HccSPIRVTypeKey* b_ = b;
	return a_->storage_class == b_->storage_class && a_->data_type == b_->data_type;
}

HccHash hcc_spirv_type_key_hash(void* key, uintptr_t size) {
	HCC_UNUSED(size);
	return hcc_hash_fnv(key, sizeof(HccSPIRVTypeKey), HCC_HASH_FNV_INIT);
}

