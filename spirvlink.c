
void hcc_spirvlink_init(HccWorker* w, HccCompilerSetup* setup) {
	w->spirvlink.words = hcc_stack_init(HccSPIRVWord, HCC_ALLOC_TAG_SPIRVLINK_WORDS, setup->backendlink.binary_grow_size / sizeof(HccSPIRVWord), setup->backendlink.binary_reserve_size / sizeof(HccSPIRVWord));
}

void hcc_spirvlink_deinit(HccWorker* w) {
	HCC_UNUSED(w);
}

void hcc_spirvlink_reset(HccWorker* w) {
	hcc_stack_clear(w->spirvlink.words);
	w->spirvlink.instr_op = HCC_SPIRV_OP_NO_OP;
}

HccSPIRVWord* hcc_spirvlink_add_word(HccWorker* w) {
	return hcc_stack_push(w->spirvlink.words);
}

HccSPIRVWord* hcc_spirvlink_add_word_many(HccWorker* w, uint32_t amount) {
	return hcc_stack_push_many(w->spirvlink.words, amount);
}

HccSPIRVOperand* hcc_spirvlink_add_instr(HccWorker* w, HccSPIRVOp op, uint32_t operands_count) {
	uint32_t words_count = operands_count + 1;
	HccSPIRVWord* words = hcc_spirvlink_add_word_many(w, words_count);
	words[0] = (words_count << 16) | op;
	return &words[1];
}

void hcc_spirvlink_instr_start(HccWorker* w, HccSPIRVOp op) {
	HCC_DEBUG_ASSERT(w->spirvlink.instr_op == HCC_SPIRV_OP_NO_OP, "internal error: hcc_spirvlink_instr_end has not be called before a new instruction was started");
	w->spirvlink.instr_op = op;
	w->spirvlink.instr_operands_count = 0;
}

void hcc_spirvlink_instr_add_operand(HccWorker* w, HccSPIRVWord word) {
	HCC_DEBUG_ASSERT(w->spirvlink.instr_op != HCC_SPIRV_OP_NO_OP, "internal error: hcc_spirvlink_instr_start has not been called when making an instruction");
	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(w->spirvlink.instr_operands_count, HCC_SPIRVLINK_INSTR_OPERANDS_CAP);
	w->spirvlink.instr_operands[w->spirvlink.instr_operands_count] = word;
	w->spirvlink.instr_operands_count += 1;
}

void hcc_spirvlink_instr_add_operands_string(HccWorker* w, char* string, uint32_t string_size) {
	uint32_t i = 0;
	for (; string_size - i >= 4; i += 4) {
		HccSPIRVWord word = 0;
		word |= string[i + 0] << 0;
		word |= string[i + 1] << 8;
		word |= string[i + 2] << 16;
		word |= string[i + 3] << 24;
		hcc_spirvlink_instr_add_operand(w, word);
	}

	if (i <= string_size) {
		HccSPIRVWord word = 0;
		if (i + 0 < string_size) word |= string[i + 0] << 0;
		if (i + 1 < string_size) word |= string[i + 1] << 8;
		if (i + 2 < string_size) word |= string[i + 2] << 16;
		if (i + 3 < string_size) word |= string[i + 3] << 24;
		hcc_spirvlink_instr_add_operand(w, word);
	}
}

void hcc_spirvlink_instr_end(HccWorker* w) {
	HCC_DEBUG_ASSERT(w->spirvlink.instr_op != HCC_SPIRV_OP_NO_OP, "internal error: hcc_spirvlink_instr_start has not been called when making an instruction");
	HccSPIRVOperand* operands = hcc_spirvlink_add_instr(w, w->spirvlink.instr_op, w->spirvlink.instr_operands_count);
	for (uint32_t operand_idx = 0; operand_idx < w->spirvlink.instr_operands_count; operand_idx += 1) {
		operands[operand_idx] = w->spirvlink.instr_operands[operand_idx];
	}

	w->spirvlink.instr_op = HCC_SPIRV_OP_NO_OP;
}

bool hcc_spirvlink_add_used_global_variable_ids(HccWorker* w, HccDecl function_decl) {
	HccCU* cu = w->cu;

	HccAMLCallNode* node = *hcc_stack_get(cu->aml.function_call_node_lists, HCC_DECL_AUX(function_decl));
	while (node) {
		//
		// descend down the call stack
		if (!hcc_spirvlink_add_used_global_variable_ids(w, node->function_decl)) {
			return false;
		}

		node = node->next_call_node_idx == UINT32_MAX ? NULL : hcc_stack_get(cu->aml.call_graph_nodes, node->next_call_node_idx);
	}

	//
	// add the global if it hasn't been added already
	HccSPIRVFunction* function = hcc_stack_get(cu->spirv.functions, HCC_DECL_AUX(function_decl));
	for (uint32_t global_variable_idx = 0; global_variable_idx < function->global_variables_count; global_variable_idx += 1) {
		HccSPIRVId spirv_id = function->global_variable_ids[global_variable_idx];

		for (uint32_t idx = 0; idx < w->spirvlink.function_unique_globals_count; idx += 1) {
			if (w->spirvlink.function_unique_globals[idx] == spirv_id) {
				goto NEXT;
			}
		}

		HCC_DEBUG_ASSERT_ARRAY_BOUNDS(w->spirvlink.function_unique_globals_count, HCC_FUNCTION_UNIQUE_GLOBALS_CAP);
		w->spirvlink.function_unique_globals[w->spirvlink.function_unique_globals_count] = spirv_id;
		w->spirvlink.function_unique_globals_count += 1;

		hcc_spirvlink_instr_add_operand(w, spirv_id);
NEXT: {}
	}

	return true;
}

void hcc_spirvlink_link(HccWorker* w) {
	HccCU* cu = w->cu;
	HccSPIRVOperand* operands = NULL;

	HccSPIRVWord magic_number = 0x07230203;
	*hcc_spirvlink_add_word(w) = magic_number;

	HccSPIRVWord major_version = 1;
	HccSPIRVWord minor_version = 6;
	HccSPIRVWord version = (major_version << 16) | (minor_version << 8);
	*hcc_spirvlink_add_word(w) = version;

	HccSPIRVWord generator_number = 0; // TODO: when we are feeling ballsy enough, register with the khronos folks and get a number for the lang.
	*hcc_spirvlink_add_word(w) = generator_number;

	HccSPIRVId* dst_highest_id = hcc_spirvlink_add_word(w);

	HccSPIRVWord reserved_instruction_schema = 0;
	*hcc_spirvlink_add_word(w) = reserved_instruction_schema;

	operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
	operands[0] = HCC_SPIRV_CAPABILITY_SHADER;

	operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
	operands[0] = HCC_SPIRV_CAPABILITY_VULKAN_MEMORY_MODEL;

	operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
	operands[0] = HCC_SPIRV_CAPABILITY_VULKAN_MEMORY_MODEL_DEVICE_SCOPE;

	operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
	operands[0] = HCC_SPIRV_CAPABILITY_STORAGE_IMAGE_READ_WITHOUT_FORMAT;

	operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
	operands[0] = HCC_SPIRV_CAPABILITY_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT;

	operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
	operands[0] = HCC_SPIRV_CAPABILITY_DEMOTE_HELPER_INVOCATION;

	if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_INT8_ENABLED)) {
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_INT8;
	}

	if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_INT16_ENABLED)) {
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_INT16;
	}

	if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_INT64_ENABLED)) {
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_INT64;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_INT64_ATOMICS;
	}

	if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_FLOAT16_ENABLED)) {
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_FLOAT16;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_FLOAT16_BUFFER;
	}

	if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_FLOAT64_ENABLED)) {
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_FLOAT64;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_FLOAT16_BUFFER;
	}

	if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_PHYSICAL_POINTER_ENABLED)) {
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_CAPABILITY, 1);
		operands[0] = HCC_SPIRV_CAPABILITY_PHYSICAL_STORAGE_BUFFER;
	}

	hcc_spirvlink_instr_start(w, HCC_SPIRV_OP_EXT_INST_IMPORT);
	hcc_spirvlink_instr_add_operand(w, HCC_SPIRV_ID_GLSL_STD_450);
	hcc_spirvlink_instr_add_operands_string_lit(w, "GLSL.std.450");
	hcc_spirvlink_instr_end(w);

	hcc_spirvlink_instr_start(w, HCC_SPIRV_OP_MEMORY_MODEL);
	if (hcc_options_get_bool(cu->options, HCC_OPTION_KEY_PHYSICAL_POINTER_ENABLED)) {
		hcc_spirvlink_instr_add_operand(w, HCC_SPIRV_ADDRESS_MODEL_PHYSICAL_STORAGE_BUFFER_64);
	} else {
		hcc_spirvlink_instr_add_operand(w, HCC_SPIRV_ADDRESS_MODEL_LOGICAL);
	}
	hcc_spirvlink_instr_add_operand(w, HCC_SPIRV_MEMORY_MODEL_VULKAN);
	hcc_spirvlink_instr_end(w);

	for (uint32_t entry_point_idx = 0; entry_point_idx < hcc_stack_count(cu->spirv.entry_points); entry_point_idx += 1) {
		HccSPIRVEntryPoint* entry_point = &cu->spirv.entry_points[entry_point_idx];

		hcc_spirvlink_instr_start(w, HCC_SPIRV_OP_ENTRY_POINT);

		HccSPIRVWord execution_model;
		switch (entry_point->shader_stage) {
			case HCC_SHADER_STAGE_VERTEX: execution_model = HCC_SPIRV_EXECUTION_MODEL_VERTEX; break;
			case HCC_SHADER_STAGE_PIXEL: execution_model = HCC_SPIRV_EXECUTION_MODEL_FRAGMENT; break;
			case HCC_SHADER_STAGE_COMPUTE: execution_model = HCC_SPIRV_EXECUTION_MODEL_GL_COMPUTE; break;
			case HCC_SHADER_STAGE_MESH_TASK:
			case HCC_SHADER_STAGE_MESH:
			default: HCC_ABORT("unhandled shader stage: %u", entry_point->shader_stage);
		}
		hcc_spirvlink_instr_add_operand(w, execution_model);

		hcc_spirvlink_instr_add_operand(w, entry_point->spirv_id);

		HccString name = hcc_string_table_get(entry_point->identifier_string_id);
		hcc_spirvlink_instr_add_operands_string(w, name.data, name.size);

		w->spirvlink.function_unique_globals_count = 0;
		hcc_spirvlink_add_used_global_variable_ids(w, entry_point->function_decl);

		hcc_spirvlink_instr_end(w);
	}

	for (uint32_t entry_point_idx = 0; entry_point_idx < hcc_stack_count(cu->spirv.entry_points); entry_point_idx += 1) {
		HccSPIRVEntryPoint* entry_point = &cu->spirv.entry_points[entry_point_idx];
		switch (entry_point->shader_stage) {
			case HCC_SHADER_STAGE_VERTEX:
				break;
			case HCC_SHADER_STAGE_PIXEL:
				operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_EXECUTION_MODE, 2);
				operands[0] = entry_point->spirv_id;
				operands[1] = HCC_SPIRV_EXECUTION_MODE_ORIGIN_UPPER_LEFT;
				break;
			case HCC_SHADER_STAGE_COMPUTE: {
				const HccAMLFunction* aml_function = hcc_aml_function_get(cu, entry_point->function_decl);
				operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_EXECUTION_MODE, 5);
				operands[0] = entry_point->spirv_id;
				operands[1] = HCC_SPIRV_EXECUTION_MODE_LOCAL_SIZE;
				operands[2] = aml_function->compute_dispatch_group_size_x;
				operands[3] = aml_function->compute_dispatch_group_size_y;
				operands[4] = aml_function->compute_dispatch_group_size_z;
				break;
			};
			case HCC_SHADER_STAGE_MESH_TASK:
				break;
			case HCC_SHADER_STAGE_MESH:
				break;
			default: HCC_ABORT("unhandled shader stage: %u", entry_point->shader_stage);
		}
	}

	HccSPIRVId vertex_sv_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_HCC_VERTEX_SV);
	HccSPIRVId vertex_sv_out_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_HCC_VERTEX_SV_OUT);
	HccSPIRVId pixel_sv_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_HCC_PIXEL_SV);
	HccSPIRVId pixel_sv_out_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_HCC_PIXEL_SV_OUT);
	HccSPIRVId compute_sv_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INVALID, HCC_DATA_TYPE_HCC_COMPUTE_SV);

	HccSPIRVId variable_input_vertex_sv_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INPUT, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_HCC_VERTEX_SV));
	HccSPIRVId variable_output_vertex_sv_out_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_OUTPUT, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_HCC_VERTEX_SV_OUT));
	HccSPIRVId variable_input_pixel_sv_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INPUT, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_HCC_PIXEL_SV));
	HccSPIRVId variable_output_pixel_sv_out_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_OUTPUT, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_HCC_PIXEL_SV_OUT));
	HccSPIRVId variable_input_compute_sv_type_id = hcc_spirv_type_deduplicate(w->cu, HCC_SPIRV_STORAGE_CLASS_INPUT, hcc_pointer_data_type_deduplicate(cu, HCC_DATA_TYPE_HCC_COMPUTE_SV));

	{ // HccVertexSV
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = vertex_sv_type_id;
		operands[1] = HCC_VERTEX_SV_VERTEX_IDX;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_VERTEX_INDEX;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = vertex_sv_type_id;
		operands[1] = HCC_VERTEX_SV_INSTANCE_IDX;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_INSTANCE_INDEX;
	}

	{ // HccVertexSVOut
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = vertex_sv_out_type_id;
		operands[1] = HCC_VERTEX_SV_OUT_POSITION;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_POSITION;
	}

	{ // HccPixelSV
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = pixel_sv_type_id;
		operands[1] = HCC_PIXEL_SV_PIXEL_COORD;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_FRAG_COORD;
	}

	{ // HccPixelSVOut
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = pixel_sv_out_type_id;
		operands[1] = HCC_PIXEL_SV_OUT_DEPTH;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_FRAG_DEPTH;
	}

	{ // HccComputeSV
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = compute_sv_type_id;
		operands[1] = HCC_COMPUTE_SV_DISPATCH_IDX;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_GLOBAL_INVOCATION_ID;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = compute_sv_type_id;
		operands[1] = HCC_COMPUTE_SV_DISPATCH_GROUP_IDX;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_WORK_GROUP_ID;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = compute_sv_type_id;
		operands[1] = HCC_COMPUTE_SV_DISPATCH_LOCAL_IDX;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_LOCAL_INVOCATION_ID;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_MEMBER_DECORATE, 4);
		operands[0] = compute_sv_type_id;
		operands[1] = HCC_COMPUTE_SV_DISPATCH_LOCAL_FLAT_IDX;
		operands[2] = HCC_SPIRV_DECORATION_BUILTIN;
		operands[3] = HCC_SPIRV_BUILTIN_LOCAL_INVOCATION_INDEX;
	}

	HccSPIRVWord* words = hcc_spirvlink_add_word_many(w, hcc_stack_count(cu->spirv.decorate_words));
	HCC_COPY_ELMT_MANY(words, cu->spirv.decorate_words, hcc_stack_count(cu->spirv.decorate_words));

	for (uint32_t type_idx = 0; type_idx < hcc_stack_count(cu->spirv.types_and_constants); type_idx += 1) {
		HccSPIRVTypeOrConstant* type_or_constant = &cu->spirv.types_and_constants[type_idx];
		operands = hcc_spirvlink_add_instr(w, type_or_constant->op, type_or_constant->operands_count);
		for (uint32_t operand_idx = 0; operand_idx < type_or_constant->operands_count; operand_idx += 1) {
			operands[operand_idx] = type_or_constant->operands[operand_idx];
		}
	}

	{
		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_VARIABLE, 3);
		operands[0] = variable_input_vertex_sv_type_id;
		operands[1] = HCC_SPIRV_ID_VARIABLE_INPUT_VERTEX_SV;
		operands[2] = HCC_SPIRV_STORAGE_CLASS_INPUT;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_VARIABLE, 3);
		operands[0] = variable_output_vertex_sv_out_type_id;
		operands[1] = HCC_SPIRV_ID_VARIABLE_OUTPUT_VERTEX_SV_OUT;
		operands[2] = HCC_SPIRV_STORAGE_CLASS_OUTPUT;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_VARIABLE, 3);
		operands[0] = variable_input_pixel_sv_type_id;
		operands[1] = HCC_SPIRV_ID_VARIABLE_INPUT_PIXEL_SV;
		operands[2] = HCC_SPIRV_STORAGE_CLASS_INPUT;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_VARIABLE, 3);
		operands[0] = variable_output_pixel_sv_out_type_id;
		operands[1] = HCC_SPIRV_ID_VARIABLE_OUTPUT_PIXEL_SV_OUT;
		operands[2] = HCC_SPIRV_STORAGE_CLASS_OUTPUT;

		operands = hcc_spirvlink_add_instr(w, HCC_SPIRV_OP_VARIABLE, 3);
		operands[0] = variable_input_compute_sv_type_id;
		operands[1] = HCC_SPIRV_ID_VARIABLE_INPUT_COMPUTE_SV;
		operands[2] = HCC_SPIRV_STORAGE_CLASS_INPUT;

		HccSPIRVWord* words = hcc_spirvlink_add_word_many(w, hcc_stack_count(cu->spirv.global_variable_words));
		HCC_COPY_ELMT_MANY(words, cu->spirv.global_variable_words, hcc_stack_count(cu->spirv.global_variable_words));
	}

	HccStack(HccDecl) optimize_functions = hcc_aml_optimize_functions(cu);
	for (uint32_t idx = 0; idx < hcc_stack_count(optimize_functions); idx += 1) {
		HccDecl function_decl = optimize_functions[idx];

		HccSPIRVFunction* function = hcc_stack_get(cu->spirv.functions, HCC_DECL_AUX(function_decl));
		HccSPIRVWord* words = hcc_spirvlink_add_word_many(w, function->words_count);
		HCC_COPY_ELMT_MANY(words, function->words, function->words_count);
	}

	*dst_highest_id = atomic_load(&cu->spirv.next_spirv_id);

	cu->spirv.final_binary_words = w->spirvlink.words;
	cu->spirv.final_binary_words_count = hcc_stack_count(w->spirvlink.words);
}

