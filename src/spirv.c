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
	cu->spirv.descriptor_binding_table = hcc_hash_table_init(HccSPIRVDescriptorBindingEntry, HCC_ALLOC_TAG_SPIRV_DESCRIPTOR_BINDING_TABLE, hcc_spirv_descriptor_binding_key_cmp, hcc_spirv_descriptor_binding_key_hash, decl_table_entries_cap);
	cu->spirv.constant_table = hcc_hash_table_init(HccSPIRVConstantEntry, HCC_ALLOC_TAG_SPIRV_CONSTANT_TABLE, hcc_u32_key_cmp, hcc_u32_key_hash, setup->constant_table.entries_cap);
	cu->spirv.types_and_constants = hcc_stack_init(HccSPIRVTypeOrConstant, HCC_ALLOC_TAG_SPIRV_TYPES_AND_CONSTANTS, types_grow_count, types_reserve_cap + setup->constant_table.entries_cap);
	cu->spirv.type_elmt_ids = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_TYPE_ELMT_IDS, setup->dtt.compound_fields_grow_count, setup->dtt.compound_fields_reserve_cap);
	cu->spirv.entry_points = hcc_stack_init(HccSPIRVEntryPoint, HCC_ALLOC_TAG_SPIRV_ENTRY_POINTS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->spirv.entry_point_global_variable_ids = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_ENTRY_POINT_GLOBAL_VARIABLE_IDS, setup->ast.global_variables_grow_count, setup->ast.global_variables_reserve_cap);
	cu->spirv.global_variable_words = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_GLOBAL_VARIABLE_WORDS, setup->ast.global_variables_grow_count * 4, setup->ast.global_variables_reserve_cap * 4);
	cu->spirv.decorate_words = hcc_stack_init(HccSPIRVId, HCC_ALLOC_TAG_SPIRV_DECORATE_WORDS, setup->ast.global_variables_grow_count * 4, setup->ast.global_variables_reserve_cap * 4);

	HccBasic basic = { .u32 = hcc_options_get_u32(cu->options, HCC_OPTION_KEY_RESOURCE_DESCRIPTORS_MAX) };
	cu->spirv.resource_descriptors_max_constant_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = HCC_SPIRV_SCOPE_DEVICE;
	cu->spirv.scope_device_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = HCC_SPIRV_SCOPE_WORK_GROUP;
	cu->spirv.scope_workgroup_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = HCC_SPIRV_SCOPE_SUB_GROUP;
	cu->spirv.scope_subgroup_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = HCC_SPIRV_MEMORY_SEMANTICS_ACQUIRE_RELEASE | HCC_SPIRV_MEMORY_SEMANTICS_WORK_GROUP_MEMORY;
	cu->spirv.memory_semantics_dispatch_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = HCC_SPIRV_MEMORY_SEMANTICS_ACQUIRE_RELEASE | HCC_SPIRV_MEMORY_SEMANTICS_UNIFORM_MEMORY | HCC_SPIRV_MEMORY_SEMANTICS_IMAGE_MEMORY;
	cu->spirv.memory_semantics_resource_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = HCC_SPIRV_MEMORY_SEMANTICS_ACQUIRE_RELEASE | HCC_SPIRV_MEMORY_SEMANTICS_WORK_GROUP_MEMORY | HCC_SPIRV_MEMORY_SEMANTICS_UNIFORM_MEMORY | HCC_SPIRV_MEMORY_SEMANTICS_IMAGE_MEMORY;
	cu->spirv.memory_semantics_all_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = 0;
	cu->spirv.quad_swap_x_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = 1;
	cu->spirv.quad_swap_y_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
	basic.u32 = 2;
	cu->spirv.quad_swap_diagonal_spirv_id = hcc_spirv_constant_deduplicate(cu, hcc_constant_table_deduplicate_basic(cu, HCC_DATA_TYPE_AML_INTRINSIC_U32, &basic));
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
	bool needs_block
		= storage_class == HCC_SPIRV_STORAGE_CLASS_PUSH_CONSTANT
		|| storage_class == HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER
		|| storage_class == HCC_SPIRV_STORAGE_CLASS_INPUT
		|| storage_class == HCC_SPIRV_STORAGE_CLASS_OUTPUT
		;
	if (!HCC_DATA_TYPE_IS_POINTER(data_type)) {
		storage_class = HCC_SPIRV_STORAGE_CLASS_INVALID;
	} else {
		HCC_DEBUG_ASSERT(storage_class != HCC_SPIRV_STORAGE_CLASS_INVALID, "pointer type cannot have an invalid storage class");
	}

	if (HCC_DATA_TYPE_IS_RESOURCE_DESCRIPTOR(data_type)) {
		HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(data_type);
		if (HCC_RESOURCE_DATA_TYPE_IS_TEXTURE(resource_data_type) && HCC_RESOURCE_DATA_TYPE_ACCESS_MODE(resource_data_type) != HCC_RESOURCE_ACCESS_MODE_SAMPLE) {
			//
			// SUPER HACK: spir-v only allows unique type declaration for OpTypeImage.
			// HCC_RESOURCE_ACCESS_MODE_READ_ONLY & HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY & HCC_RESOURCE_ACCESS_MODE_READ_WRITE,
			// all end up making an identical opTypeImage, so make all of those use the same key for the hash table when deduplicating the type.
			resource_data_type &= ~HCC_RESOURCE_DATA_TYPE_ACCESS_MODE_MASK;
			resource_data_type |= HCC_RESOURCE_ACCESS_MODE_READ_ONLY << HCC_RESOURCE_DATA_TYPE_ACCESS_MODE_SHIFT;
			data_type = HCC_DATA_TYPE(RESOURCE_DESCRIPTOR, resource_data_type);
		}
	}

	HccSPIRVTypeKey key = { .storage_class = storage_class, .data_type = data_type };
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->spirv.type_table, &key);
	HccSPIRVTypeEntry* entry = &cu->spirv.type_table[insert.idx];
	if (insert.is_new) {
		HccSPIRVOp op;
		HccSPIRVOperand* operands;
		uint32_t operands_count;
		HccSPIRVId runtime_array_spirv_id = 0;
		uint32_t num_ids = 1;
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
				operands[2] = hcc_spirv_type_deduplicate(cu, storage_class, dt->element_data_type);
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
			case HCC_DATA_TYPE_RESOURCE: {
				HccSPIRVId spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_AML_INTRINSIC_U32);
				atomic_store(&entry->spirv_id, spirv_id);
				return spirv_id;
			};

			case HCC_DATA_TYPE_RESOURCE_DESCRIPTOR: {
				HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(data_type);
				switch (HCC_RESOURCE_DATA_TYPE_TYPE(resource_data_type)) {
					case HCC_RESOURCE_DATA_TYPE_BUFFER: {
						HccBufferDataType* d = hcc_buffer_data_type_get(cu, data_type);
						HccSPIRVId element_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, d->element_data_type);

						uint64_t element_size;
						uint64_t element_align;
						hcc_data_type_size_align(cu, d->element_data_type, &element_size, &element_align);

						runtime_array_spirv_id = hcc_spirv_next_id(cu);
						HccSPIRVTypeOrConstant* runtime_array_type = hcc_stack_push(cu->spirv.types_and_constants);
						runtime_array_type->op = HCC_SPIRV_OP_TYPE_RUNTIME_ARRAY;
						runtime_array_type->operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 2);
						runtime_array_type->operands_count = 2;
						runtime_array_type->operands[0] = runtime_array_spirv_id;
						runtime_array_type->operands[1] = element_spirv_id;

						operands = hcc_spirv_add_decorate(cu, 3);
						operands[0] = runtime_array_spirv_id;
						operands[1] = HCC_SPIRV_DECORATION_ARRAY_STRIDE;
						operands[2] = element_size;

						op = HCC_SPIRV_OP_TYPE_STRUCT;
						operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 2);
						operands_count = 2;
						operands[1] = runtime_array_spirv_id;
						break;
					};
					case HCC_RESOURCE_DATA_TYPE_TEXTURE:
						op = HCC_SPIRV_OP_TYPE_IMAGE;
						HccAMLIntrinsicDataType sample_data_type = HCC_RESOURCE_DATA_TYPE_TEXTURE_INTRINSIC_TYPE(resource_data_type);
						sample_data_type = HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(sample_data_type);
						operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 8);
						operands_count = 8;
						operands[1] = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE(AML_INTRINSIC, sample_data_type));
						switch (HCC_RESOURCE_DATA_TYPE_TEXTURE_DIM(resource_data_type)) {
							case HCC_TEXTURE_DIM_1D: operands[2] = HCC_SPIRV_DIM_1D; break;
							case HCC_TEXTURE_DIM_2D: operands[2] = HCC_SPIRV_DIM_2D; break;
							case HCC_TEXTURE_DIM_3D: operands[2] = HCC_SPIRV_DIM_3D; break;
							case HCC_TEXTURE_DIM_CUBE: operands[2] = HCC_SPIRV_DIM_CUBE; break;
						}
						operands[3] = 2; // 2 means no indication whether this is a depth texture or not
						operands[4] = HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_ARRAY(resource_data_type);
						operands[5] = HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type);
						operands[6] = HCC_RESOURCE_DATA_TYPE_ACCESS_MODE(resource_data_type) == HCC_RESOURCE_ACCESS_MODE_SAMPLE ? 1 : 2;
						operands[7] = 0; // unknown format
						num_ids = HCC_RESOURCE_DATA_TYPE_ACCESS_MODE(resource_data_type) == HCC_RESOURCE_ACCESS_MODE_SAMPLE ? 2 : 1;
						break;
					case HCC_RESOURCE_DATA_TYPE_SAMPLER:
						op = HCC_SPIRV_OP_TYPE_SAMPLER;
						operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 1);
						operands_count = 1;
						break;
				}
				break;
			};
			default: HCC_ABORT("unhandled data type type: %u", HCC_DATA_TYPE_TYPE(data_type));
		}

		HccSPIRVId spirv_id = hcc_spirv_next_id_many(cu, num_ids);
		operands[0] = spirv_id;

		HccSPIRVTypeOrConstant* type = hcc_stack_push(cu->spirv.types_and_constants);
		type->op = op;
		type->operands = operands;
		type->operands_count = operands_count;

		if (HCC_DATA_TYPE_IS_STRUCT(data_type)) {
			switch (data_type) {
				default:
					if (!needs_block) {
						break;
					}
					hcc_fallthrough;
				case HCC_DATA_TYPE_HCC_VERTEX_SV:
				case HCC_DATA_TYPE_HCC_VERTEX_SV_OUT:
				case HCC_DATA_TYPE_HCC_FRAGMENT_SV:
				case HCC_DATA_TYPE_HCC_FRAGMENT_SV_OUT:
				case HCC_DATA_TYPE_HCC_COMPUTE_SV:
					operands = hcc_spirv_add_decorate(cu, 2);
					operands[0] = spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_BLOCK;
					break;
			}

			HccCompoundDataType* dt = hcc_compound_data_type_get(cu, data_type);
			for (uint32_t field_idx = 0; field_idx < dt->fields_count; field_idx += 1) {
				HccCompoundField* field = &dt->fields[field_idx];
				operands = hcc_spirv_add_member_decorate(cu, 4);
				operands[0] = spirv_id;
				operands[1] = field_idx;
				operands[2] = HCC_SPIRV_DECORATION_OFFSET;
				operands[3] = field->byte_offset;
			}
		} else if (HCC_DATA_TYPE_IS_ARRAY(data_type)) {
			HccArrayDataType* dt = hcc_array_data_type_get(cu, data_type);
			uint64_t element_size;
			uint64_t element_align;
			hcc_data_type_size_align(cu, dt->element_data_type, &element_size, &element_align);

			operands = hcc_spirv_add_decorate(cu, 3);
			operands[0] = spirv_id;
			operands[1] = HCC_SPIRV_DECORATION_ARRAY_STRIDE;
			operands[2] = element_size;
		} else if (HCC_DATA_TYPE_IS_RESOURCE_DESCRIPTOR(data_type)) {
			if (HCC_DATA_TYPE_IS_BUFFER(data_type)) {
				operands = hcc_spirv_add_decorate(cu, 2);
				operands[0] = spirv_id;
				operands[1] = HCC_SPIRV_DECORATION_BLOCK;

				operands = hcc_spirv_add_member_decorate(cu, 4);
				operands[0] = spirv_id;
				operands[1] = 0; // field index
				operands[2] = HCC_SPIRV_DECORATION_OFFSET;
				operands[3] = 0; // byte offset

				atomic_store(&entry->spirv_id, runtime_array_spirv_id);
				return spirv_id;
			} else if (HCC_DATA_TYPE_IS_TEXTURE(data_type) && num_ids == 2) {
				HccSPIRVTypeOrConstant* runtime_array_type = hcc_stack_push(cu->spirv.types_and_constants);
				runtime_array_type->op = HCC_SPIRV_OP_TYPE_SAMPLED_IMAGE;
				runtime_array_type->operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 2);
				runtime_array_type->operands_count = 2;
				runtime_array_type->operands[0] = spirv_id + 1;
				runtime_array_type->operands[1] = spirv_id;
			}
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

HccSPIRVId hcc_spirv_decl_deduplicate(HccCU* cu, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_TYPE(decl) == HCC_DECL_FUNCTION || HCC_DECL_TYPE(decl) == HCC_DECL_GLOBAL_VARIABLE, "expected decl to be a function or a global variable");

	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->spirv.decl_table, &decl);
	HccSPIRVDeclEntry* entry = &cu->spirv.decl_table[insert.idx];
	if (insert.is_new) {
		HccSPIRVId spirv_id = hcc_spirv_next_id(cu);

		switch (HCC_DECL_TYPE(decl)) {
			case HCC_DECL_GLOBAL_VARIABLE: {
				HccASTVariable* ast_global_variable = hcc_ast_global_variable_get(cu, decl);
				HccSPIRVStorageClass storage_class;
				bool has_initializer;
				switch (ast_global_variable->storage_duration) {
					case HCC_AST_STORAGE_DURATION_DISPATCH_GROUP:
						storage_class =  HCC_SPIRV_STORAGE_CLASS_WORK_GROUP;
						has_initializer = false;
						break;
					case HCC_AST_STORAGE_DURATION_THREAD:
						storage_class =  HCC_SPIRV_STORAGE_CLASS_PRIVATE;
						has_initializer = true;
						break;
					case HCC_AST_STORAGE_DURATION_STATIC:
					case HCC_AST_STORAGE_DURATION_AUTOMATIC:
						HCC_ABORT("unsupported storage duration for SPIR-V backend: %u", ast_global_variable->storage_duration);
						break;
				}
				HccSPIRVOperand* operands = hcc_spirv_add_global_variable(cu, 3 + has_initializer);
				operands[0] = hcc_spirv_type_deduplicate(cu, storage_class, hcc_pointer_data_type_deduplicate(cu, ast_global_variable->data_type));
				operands[1] = spirv_id;
				operands[2] = storage_class;
				if (has_initializer) {
					operands[3] = hcc_spirv_constant_deduplicate(cu, ast_global_variable->initializer_constant_id);
				}
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

void hcc_spirv_resource_descriptor_binding_deduplicate(HccCU* cu, HccDataType data_type, HccSPIRVDescriptorBindingInfo* info_out) {
	HCC_DEBUG_ASSERT(HCC_DATA_TYPE_IS_RESOURCE_DESCRIPTOR(data_type), "expected data type to be a resource data type but got %u", data_type);
	HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(data_type);
	HccSPIRVId data_type_spirv_id = hcc_spirv_type_deduplicate(cu, HCC_SPIRV_STORAGE_CLASS_INVALID, data_type);
	HccSPIRVStorageClass storage_class = HCC_RESOURCE_DATA_TYPE_TYPE(resource_data_type) == HCC_RESOURCE_DATA_TYPE_BUFFER ? HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER : HCC_SPIRV_STORAGE_CLASS_UNIFORM_CONSTANT;
	HccSPIRVId data_type_ptr_spirv_id = hcc_spirv_type_deduplicate(cu, storage_class, hcc_pointer_data_type_deduplicate(cu, data_type));

	HccSPIRVDescriptorBindingKey key;
	key.resource_data_type = resource_data_type;
	key._padding = 0;
	key.element_data_type = 0;
	if (HCC_RESOURCE_DATA_TYPE_IS_BUFFER(resource_data_type)) {
		key.element_data_type = hcc_buffer_data_type_get(cu, data_type)->element_data_type;
	}

	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->spirv.descriptor_binding_table, &key);
	HccSPIRVDescriptorBindingEntry* entry = &cu->spirv.descriptor_binding_table[insert.idx];
	if (insert.is_new) {
		HccSPIRVDescriptorSetBinding binding;
		switch (HCC_RESOURCE_DATA_TYPE_TYPE(resource_data_type)) {
			case HCC_RESOURCE_DATA_TYPE_BUFFER:
				binding = HCC_SPIRV_DESCRIPTOR_SET_BINDING_STORAGE_BUFFER;
				break;
			case HCC_RESOURCE_DATA_TYPE_TEXTURE:
				binding = HCC_RESOURCE_DATA_TYPE_ACCESS_MODE(resource_data_type) == HCC_RESOURCE_ACCESS_MODE_SAMPLE ? HCC_SPIRV_DESCRIPTOR_SET_BINDING_SAMPLED_IMAGE : HCC_SPIRV_DESCRIPTOR_SET_BINDING_STORAGE_IMAGE;
				break;
			case HCC_RESOURCE_DATA_TYPE_SAMPLER:
				binding = HCC_SPIRV_DESCRIPTOR_SET_BINDING_SAMPLER;
				break;
		}

		HccSPIRVTypeOrConstant* array_type = hcc_stack_push(cu->spirv.types_and_constants);
		array_type->op = HCC_SPIRV_OP_TYPE_ARRAY;
		array_type->operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 3);
		array_type->operands_count = 3;
		array_type->operands[0] = hcc_spirv_next_id(cu);
		array_type->operands[1] = data_type_spirv_id;
		array_type->operands[2] = cu->spirv.resource_descriptors_max_constant_spirv_id;

		HccSPIRVTypeOrConstant* pointer_array_type = hcc_stack_push(cu->spirv.types_and_constants);
		pointer_array_type->op = HCC_SPIRV_OP_TYPE_POINTER;
		pointer_array_type->operands = hcc_stack_push_many(cu->spirv.type_elmt_ids, 3);
		pointer_array_type->operands_count = 3;
		pointer_array_type->operands[0] = hcc_spirv_next_id(cu);
		pointer_array_type->operands[1] = storage_class;
		pointer_array_type->operands[2] = array_type->operands[0];

		HccSPIRVId variable_spirv_id = hcc_spirv_next_id(cu);
		HccSPIRVOperand* operands = hcc_spirv_add_global_variable(cu, 3);
		operands[0] = pointer_array_type->operands[0];
		operands[1] = variable_spirv_id;
		operands[2] = storage_class;

		if (resource_data_type != HCC_RESOURCE_DATA_TYPE_ROSAMPLER) {
			switch (HCC_RESOURCE_DATA_TYPE_ACCESS_MODE(resource_data_type)) {
				case HCC_RESOURCE_ACCESS_MODE_READ_ONLY:
					operands = hcc_spirv_add_decorate(cu, 2);
					operands[0] = variable_spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_NON_WRITABLE;
					break;
				case HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY:
					operands = hcc_spirv_add_decorate(cu, 2);
					operands[0] = variable_spirv_id;
					operands[1] = HCC_SPIRV_DECORATION_NON_READABLE;
					break;
				case HCC_RESOURCE_ACCESS_MODE_READ_WRITE:
					break;
			}
		}

		HccSPIRVId descriptor_binding_variable_spirv_id = variable_spirv_id;

		operands = hcc_spirv_add_decorate(cu, 3);
		operands[0] = descriptor_binding_variable_spirv_id;
		operands[1] = HCC_SPIRV_DECORATION_DESCRIPTOR_SET;
		operands[2] = 0;

		operands = hcc_spirv_add_decorate(cu, 3);
		operands[0] = descriptor_binding_variable_spirv_id;
		operands[1] = HCC_SPIRV_DECORATION_BINDING;
		operands[2] = binding;

		atomic_store(&entry->variable_spirv_id, descriptor_binding_variable_spirv_id);
	} else {
		//
		// spin if another thread has just inserted the entry but not set the spirv id yet
		while (atomic_load(&entry->variable_spirv_id) == 0) {
			HCC_CPU_RELAX();
		}
	}

	info_out->variable_spirv_id = atomic_load(&entry->variable_spirv_id);
	info_out->data_type_spirv_id = data_type_spirv_id;
	info_out->data_type_ptr_spirv_id = data_type_ptr_spirv_id;
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

HccSPIRVStorageClass hcc_spirv_storage_class_from_aml_operand(HccCU* cu, const HccAMLFunction* aml_function, HccAMLOperand aml_operand) {
	switch (HCC_AML_OPERAND_TYPE(aml_operand)) {
		case HCC_AML_OPERAND_VALUE:
			if (aml_function->shader_stage != HCC_SHADER_STAGE_NONE && HCC_AML_OPERAND_AUX(aml_operand) < aml_function->params_count) {
				uint32_t param_idx = HCC_AML_OPERAND_AUX(aml_operand);
				switch (aml_function->shader_stage) {
					case HCC_SHADER_STAGE_VERTEX:
						switch (param_idx) {
							case HCC_VERTEX_SHADER_PARAM_VERTEX_SV:
								return HCC_SPIRV_STORAGE_CLASS_INPUT;
							case HCC_VERTEX_SHADER_PARAM_VERTEX_SV_OUT:
								return HCC_SPIRV_STORAGE_CLASS_OUTPUT;
							case HCC_VERTEX_SHADER_PARAM_BC:
								return HCC_SPIRV_STORAGE_CLASS_PUSH_CONSTANT;
							case HCC_VERTEX_SHADER_PARAM_RASTERIZER_STATE:
								return HCC_SPIRV_STORAGE_CLASS_OUTPUT;
							default:
								HCC_ABORT("unhandled parameter %u", param_idx);
						}
						break;
					case HCC_SHADER_STAGE_FRAGMENT:
						switch (param_idx) {
							case HCC_FRAGMENT_SHADER_PARAM_FRAGMENT_SV:
								return HCC_SPIRV_STORAGE_CLASS_INPUT;
							case HCC_FRAGMENT_SHADER_PARAM_FRAGMENT_SV_OUT:
								return HCC_SPIRV_STORAGE_CLASS_OUTPUT;
							case HCC_FRAGMENT_SHADER_PARAM_BC:
								return HCC_SPIRV_STORAGE_CLASS_PUSH_CONSTANT;
							case HCC_FRAGMENT_SHADER_PARAM_RASTERIZER_STATE:
								return HCC_SPIRV_STORAGE_CLASS_INPUT;
							case HCC_FRAGMENT_SHADER_PARAM_FRAGMENT_STATE:
								return HCC_SPIRV_STORAGE_CLASS_OUTPUT;
							default:
								HCC_ABORT("unhandled parameter %u", param_idx);
						}
						break;
					case HCC_SHADER_STAGE_COMPUTE:
						switch (param_idx) {
							case HCC_COMPUTE_SHADER_PARAM_COMPUTE_SV:
								return HCC_SPIRV_STORAGE_CLASS_INPUT;
							case HCC_COMPUTE_SHADER_PARAM_BC:
								return HCC_SPIRV_STORAGE_CLASS_PUSH_CONSTANT;
							default:
								HCC_ABORT("unhandled parameter %u", param_idx);
						}
						break;
					default:
						HCC_ABORT("unhandled shader stage %u", aml_function->shader_stage);
				}
			}
			HccDataType data_type = hcc_aml_operand_data_type(cu, aml_function, aml_operand);
			if (HCC_DATA_TYPE_IS_BUFFER(data_type)) {
				return HCC_SPIRV_STORAGE_CLASS_STORAGE_BUFFER;
			}
			return HCC_SPIRV_STORAGE_CLASS_FUNCTION;

		case HCC_DECL_GLOBAL_VARIABLE: {
			HccASTVariable* variable = hcc_ast_global_variable_get(cu, (HccDecl)aml_operand);
			return variable->storage_duration == HCC_AST_STORAGE_DURATION_THREAD ? HCC_SPIRV_STORAGE_CLASS_PRIVATE : HCC_SPIRV_STORAGE_CLASS_WORK_GROUP;
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

HccSPIRVOperand* hcc_spirv_add_member_decorate(HccCU* cu, uint32_t operands_count) {
	uint32_t words_count = operands_count + 1;
	HccSPIRVWord* words = hcc_stack_push_many(cu->spirv.decorate_words, words_count);
	words[0] = (words_count << 16) | HCC_SPIRV_OP_MEMBER_DECORATE;
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

bool hcc_spirv_descriptor_binding_key_cmp(void* a, void* b, uintptr_t size) {
	HCC_UNUSED(size);
	HccSPIRVDescriptorBindingKey* a_ = a;
	HccSPIRVDescriptorBindingKey* b_ = b;
	return a_->resource_data_type == b_->resource_data_type && a_->element_data_type == b_->element_data_type;
}

HccHash hcc_spirv_descriptor_binding_key_hash(void* key, uintptr_t size) {
	HCC_UNUSED(size);
	return hcc_hash_fnv(key, sizeof(HccSPIRVDescriptorBindingKey), HCC_HASH_FNV_INIT);
}

