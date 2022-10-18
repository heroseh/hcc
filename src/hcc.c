#include "hcc_internal.h"

// ===========================================
//
//
// Compilation Unit
//
//
// ===========================================
//
// this is not a traditional compiliation unit from the sense
// of a single C source file and all of it's includes.
// we actually allow multiple C source files to be compiled
// into a single AST and therefore a single binary.
//

void hcc_cu_init(HccCU* cu, HccCUSetup* setup, HccTarget* target) {
	cu->target = *target;
	hcc_constant_table_init(cu, &setup->constant_table);
	hcc_data_type_table_init(cu, &setup->dtt);
	hcc_ast_init(cu, &setup->ast);
	hcc_aml_init(cu, &setup->aml);
}

void hcc_cu_deinit(HccCU* cu) {
	hcc_constant_table_deinit(cu);
	hcc_data_type_table_deinit(cu);
	hcc_ast_deinit(cu);
	hcc_aml_deinit(cu);
}

// ===========================================
//
//
// Compiler Options
//
//
// ===========================================

HccOptionsSetup hcc_options_setup_default = {
	.defines_grow_count = 256,
	.defines_reserve_cap = 1024,
};

HccOptionValue hcc_option_key_defaults[HCC_OPTION_KEY_COUNT] = {
	[HCC_OPTION_KEY_RESOURCE_SET_SLOT_MAX] = { .uint = 4 },
	[HCC_OPTION_KEY_RESOURCE_CONSTANTS_MAX_SIZE] = { .uint = 32 },
};

HccResult hcc_options_init(HccOptionsSetup* setup, HccOptions** o_out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccOptions* options = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccOptions, &_hcc_gs.arena_alctor);
	hcc_options_reset(options);
	options->defines = hcc_stack_init(HccOptionDefine, 0, setup->defines_grow_count, setup->defines_reserve_cap);

	*o_out = options;
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

void hcc_options_deinit(HccOptions* options) {
	hcc_stack_deinit(options->defines);
}

void hcc_options_reset(HccOptions* options) {
	HCC_ZERO_ELMT(options->key_to_value_map);
	HCC_ZERO_ELMT(options->is_set_bitset);
	hcc_stack_clear(options->defines);
	HCC_COPY_ARRAY(options->key_to_value_map, hcc_option_key_defaults);
}

HccResult hcc_options_merge(HccOptions* low_priority, HccOptions* high_priority, HccOptions** o_out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccOptions* options = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccOptions, &_hcc_gs.arena_alctor);

	uint32_t defines_grow_count_l = hcc_stack_grow_count(low_priority->defines);
	uint32_t defines_reserve_cap_l = hcc_stack_reserve_cap(low_priority->defines);
	uint32_t defines_grow_count_h = hcc_stack_grow_count(high_priority->defines);
	uint32_t defines_reserve_cap_h = hcc_stack_reserve_cap(high_priority->defines);
	uint32_t defines_grow_count = HCC_MAX(defines_grow_count_l, defines_grow_count_h);
	uint32_t defines_reserve_cap = HCC_MAX(defines_reserve_cap_l, defines_reserve_cap_h);
	options->defines = hcc_stack_init(HccOptionDefine, 0, defines_grow_count, defines_reserve_cap);
	uint32_t defines_count_l = hcc_stack_count(low_priority->defines);
	uint32_t defines_count_h = hcc_stack_count(high_priority->defines);
	HccOptionDefine* defines = hcc_stack_push_many(options->defines, defines_count_l);
	HCC_COPY_ELMT_MANY(defines, low_priority->defines, defines_count_l);
	defines = hcc_stack_push_many(options->defines, defines_count_h);
	HCC_COPY_ELMT_MANY(defines, high_priority->defines, defines_count_h);

	for (uintptr_t key = 0; key < HCC_OPTION_KEY_COUNT; key += 1) {
		if (hcc_options_is_set(high_priority, key)) {
			hcc_options_set(options, key, hcc_options_get(high_priority, key));
		} else if (hcc_options_is_set(low_priority, key)) {
			hcc_options_set(options, key, hcc_options_get(low_priority, key));
		}
	}

	*o_out = options;
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_options_clone(HccOptions* src, HccOptions** o_out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccOptions* options = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccOptions, &_hcc_gs.arena_alctor);
	*options = *src;

	uint32_t defines_count = hcc_stack_count(src->defines);
	uint32_t defines_grow_count = hcc_stack_grow_count(src->defines);
	uint32_t defines_reserve_cap = hcc_stack_reserve_cap(src->defines);
	options->defines = hcc_stack_init(HccOptionDefine, 0, defines_grow_count, defines_reserve_cap);
	HccOptionDefine* defines = hcc_stack_push_many(options->defines, defines_count);
	HCC_COPY_ELMT_MANY(defines, src->defines, defines_count);

	*o_out = options;
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

bool hcc_options_is_set(HccOptions* options, HccOptionKey key) {
	return options->is_set_bitset[key / 64] & ((uint64_t)1 << (key % 64));
}

HccOptionValue hcc_options_get(HccOptions* options, HccOptionKey key) {
	// not need to check hcc_options_is_set(options, key)
	// as we want to return the value no matter what as it starts off with a default
	return options->key_to_value_map[key];
}

bool hcc_options_get_bool(HccOptions* options, HccOptionKey key) {
	return hcc_options_get(options, key).bool_;
}

uint32_t hcc_options_get_u32(HccOptions* options, HccOptionKey key) {
	return hcc_options_get(options, key).uint;
}

int32_t hcc_options_get_s32(HccOptions* options, HccOptionKey key) {
	return hcc_options_get(options, key).int_;
}

float hcc_options_get_float(HccOptions* options, HccOptionKey key) {
	return hcc_options_get(options, key).float_;
}

HccString hcc_options_get_string(HccOptions* options, HccOptionKey key) {
	return hcc_options_get(options, key).string;
}

void hcc_options_set(HccOptions* options, HccOptionKey key, HccOptionValue value) {
	options->is_set_bitset[key / 64] |= ((uint64_t)1 << (key % 64));
	options->key_to_value_map[key] = value;
}

void hcc_options_set_bool(HccOptions* options, HccOptionKey key, bool value) {
	HccOptionValue v = { .bool_ = value };
	hcc_options_set(options, key, v);
}

void hcc_options_set_u32(HccOptions* options, HccOptionKey key, uint32_t value) {
	HccOptionValue v = { .uint = value };
	hcc_options_set(options, key, v);
}

void hcc_options_set_s32(HccOptions* options, HccOptionKey key, int32_t value) {
	HccOptionValue v = { .int_ = value };
	hcc_options_set(options, key, v);
}

void hcc_options_set_float(HccOptions* options, HccOptionKey key, float value) {
	HccOptionValue v = { .float_ = value };
	hcc_options_set(options, key, v);
}

void hcc_options_set_string(HccOptions* options, HccOptionKey key, HccString value) {
	HccOptionValue v = { .string = value };
	hcc_options_set(options, key, v);
}

HccResult hcc_options_add_define(HccOptions* options, HccString name, HccString value) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	*hcc_stack_push(options->defines) = (HccOptionDefine) {
		.name = name,
		.value = value,
	};

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

// ===========================================
//
//
// Compiler Target
//
//
// ===========================================

bool hcc_target_is_char_unsigned(HccTarget* target) {
	switch (target->arch) {
		case HCC_TARGET_ARCH_X86_64:
			return false;
		default: HCC_ABORT("TODO implement the signiness of the char type for this target configuration");
	}
}

// ===========================================
//
//
// Compiler Task
//
//
// ===========================================

HccTaskSetup hcc_task_setup_default = {
	.cu = {
		.constant_table = {
			.data_grow_size = 1048576,   // 1MB
			.data_reserve_size = 67108864, // 64MB
			.entries_cap = 1048576,
			.composite_fields_buffer_grow_count = 256,
			.composite_fields_buffer_reserve_cap = 1024,
		},
		.dtt = {
			.arrays_grow_count = 1024,
			.arrays_reserve_cap = 131072,
			.compounds_grow_count = 1024,
			.compounds_reserve_cap = 131072,
			.compound_fields_grow_count = 1024,
			.compound_fields_reserve_cap = 131072,
			.typedefs_grow_count = 1024,
			.typedefs_reserve_cap = 131072,
			.enums_grow_count = 1024,
			.enums_reserve_cap = 131072,
			.enum_values_grow_count = 1024,
			.enum_values_reserve_cap = 131072,
			.buffers_grow_count = 1024,
			.buffers_reserve_cap = 131072,
			.pointers_grow_count = 1024,
			.pointers_reserve_cap = 131072,
		},
		.ast = {
			.files_cap = 8192,
			.file_setup = {
				.macros_grow_count = 1024,
				.macros_reserve_cap = 16384,
				.macro_params_grow_count = 1024,
				.macro_params_reserve_cap = 16384,
				.tokens_grow_count = 1024,
				.tokens_reserve_cap = 131072,
				.values_grow_count = 512,
				.values_reserve_cap = 65536,
				.unique_include_files_grow_count = 1024,
				.unique_include_files_reserve_cap = 65546,
			},
			.function_params_and_variables_grow_count = 1024,
			.function_params_and_variables_reserve_cap = 131072,
			.functions_grow_count = 1024,
			.functions_reserve_cap = 131072,
			.exprs_grow_count = 1024,
			.exprs_reserve_cap = 131072,
			.expr_locations_grow_count = 1024,
			.expr_locations_reserve_cap = 131072,
			.global_variables_grow_count = 1024,
			.global_variables_reserve_cap = 131072,
		},
		.aml = {
			.placeholder = 1,
		},
	},
	.options = NULL,
	.target = {
		.arch = HCC_TARGET_ARCH_X86_64,
		.os = HCC_TARGET_OS_LINUX,
		.gfx_api = HCC_TARGET_GFX_API_VULKAN,
		.format = HCC_TARGET_FORMAT_SPIR_V,
		.resource_model = HCC_TARGET_RESOURCE_MODEL_BINDING_AND_BINDLESS,
	},
	.final_worker_job_type = HCC_WORKER_JOB_TYPE_METADATA,
	.include_paths_cap = 1024,
	.messages_cap = 4096,
	.message_strings_cap = 16384,
};

const char* hcc_stage_strings[HCC_STAGE_COUNT] = {
	[HCC_STAGE_CODE] = "code",
	[HCC_STAGE_AST] = "ast",
	[HCC_STAGE_AML] = "aml",
	[HCC_STAGE_BINARY] = "binary",
	[HCC_STAGE_METADATA] = "metadata",
};

const char* hcc_phase_strings[HCC_PHASE_COUNT] = {
	[HCC_PHASE_FRONTEND] = "frontend",
	[HCC_PHASE_OPTIMIZATION] = "optimization",
	[HCC_PHASE_BACKEND] = "backend",
};

const char* hcc_worker_job_type_strings[HCC_WORKER_JOB_TYPE_COUNT] = {
	[HCC_WORKER_JOB_TYPE_ATAGEN] = "ATAGEN",
	[HCC_WORKER_JOB_TYPE_ASTGEN] = "ASTGEN",
	[HCC_WORKER_JOB_TYPE_AMLGEN] = "AMLGEN",
	[HCC_WORKER_JOB_TYPE_AMLOPT] = "AMLOPT",
	[HCC_WORKER_JOB_TYPE_BINARY] = "BINARY",
	[HCC_WORKER_JOB_TYPE_METADATA] = "METADATA",
};

HccTaskInputLocation* hcc_task_input_location_init(HccTask* t, HccOptions* options) {
	HccTaskInputLocation* il = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccTaskInputLocation, &_hcc_gs.arena_alctor);
	il->next = t->input_locations;
	if (options) {
		hcc_options_merge(t->options, options, &il->options);
	} else {
		hcc_options_clone(t->options, &il->options);
	}
	t->input_locations = il;
	return il;
}

HccResult hcc_task_add_output(HccTask* t, HccStage stage, HccEncoding encoding, void* arg) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccTaskOutputLocation* ol = &t->output_stage_locations[stage];
	HCC_ASSERT(ol->arg == NULL, "task output for stage '%s' has already been set", hcc_stage_strings[stage]);
	ol->stage = stage;
	ol->encoding = encoding;
	ol->arg = arg;

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

void hcc_task_output_stage(HccTask* t, HccStage stage) {
	HccTaskOutputLocation* output_location = &t->output_stage_locations[stage];

	switch (output_location->encoding) {
		case HCC_ENCODING_TEXT: {
			HccIIO* iio = output_location->arg;
			switch (stage) {
				case HCC_STAGE_CODE:
					break;
				case HCC_STAGE_AST:
					hcc_ast_print(t->cu, iio);
					break;
				case HCC_STAGE_AML:
					hcc_aml_print(t->cu, iio);
					break;
				case HCC_STAGE_BINARY:
					break;
				case HCC_STAGE_METADATA:
					break;
			}
			break;
		};
		case HCC_ENCODING_BINARY: {
			HccIIO* iio = output_location->arg;
			switch (stage) {
				case HCC_STAGE_CODE:
					break;
				case HCC_STAGE_AST:
					break;
				case HCC_STAGE_AML:
					break;
				case HCC_STAGE_BINARY:
					break;
				case HCC_STAGE_METADATA:
					break;
			}
			break;
		};
		case HCC_ENCODING_RUNTIME_BINARY:
			switch (stage) {
				case HCC_STAGE_CODE:
					break;
				case HCC_STAGE_AST:
					break;
				case HCC_STAGE_AML:
					break;
				case HCC_STAGE_BINARY:
					break;
				case HCC_STAGE_METADATA:
					break;
			}
			break;
	}
}

void hcc_task_finish(HccTask* t, bool was_successful) {
	HccCompiler* c = t->c;
	HccTime end_time = hcc_time_now(HCC_TIME_MODE_MONOTONIC);
	t->duration = hcc_time_diff(end_time, t->start_time);
	bool is_compiler_finished = atomic_fetch_sub(&c->tasks_running_count, 1) == 1;
	if (is_compiler_finished) {
		c->duration = hcc_time_diff(end_time, c->start_time);
	}

	if (was_successful) {
		for (HccStage stage = 0; stage < HCC_STAGE_COUNT; stage += 1) {
			if (t->output_stage_locations[stage].arg) {
				hcc_task_output_stage(t, stage);
			}
		}
	}

	if (is_compiler_finished) {
		hcc_mutex_unlock(&c->wait_for_all_mutex);
	}
	hcc_mutex_unlock(&t->is_running_mutex);
	t->c = NULL;
}

HccResult hcc_task_init(HccTaskSetup* setup, HccTask** t_out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccTask* t = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccTask, &_hcc_gs.arena_alctor);
	t->cu_setup = setup->cu;
	t->target = setup->target;
	t->options = setup->options;
	t->final_worker_job_type = setup->final_worker_job_type;
	t->include_path_strings = hcc_stack_init(HccString, 0, setup->include_paths_cap, setup->include_paths_cap);
	t->message_sys.elmts = hcc_stack_init(HccMessage, 0, setup->messages_cap, setup->messages_cap);
	t->message_sys.deferred_elmts = hcc_stack_init(HccMessage, 0, setup->messages_cap, setup->messages_cap);
	t->message_sys.locations = hcc_stack_init(HccLocation, 0, setup->messages_cap * 2, setup->messages_cap * 2);
	t->message_sys.strings = hcc_stack_init(char, 0, setup->message_strings_cap, setup->message_strings_cap);

	*t_out = t;
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

void hcc_task_deinit(HccTask* t) {
	hcc_stack_deinit(t->include_path_strings);
	hcc_stack_deinit(t->message_sys.elmts);
	hcc_stack_deinit(t->message_sys.deferred_elmts);
	hcc_stack_deinit(t->message_sys.locations);
	hcc_stack_deinit(t->message_sys.strings);

	hcc_cu_deinit(t->cu);
}

HccResult hcc_task_add_include_path(HccTask* t, HccString path) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	*hcc_stack_push(t->include_path_strings) = path;
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_task_add_input_code_file(HccTask* t, const char* file_path, HccOptions* options) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccTaskInputLocation* il = hcc_task_input_location_init(t, options);
	il->file_path = hcc_path_canonicalize(file_path);

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_task_add_output_ast_text(HccTask* t, HccIIO* iio) {
	return hcc_task_add_output(t, HCC_STAGE_AST, HCC_ENCODING_TEXT, iio);
}

HccResult hcc_task_add_output_ast_binary(HccTask* t, HccIIO* iio) {
	return hcc_task_add_output(t, HCC_STAGE_AST, HCC_ENCODING_BINARY, iio);
}

HccResult hcc_task_add_output_ast(HccTask* t, HccAST** ast_out) {
	return hcc_task_add_output(t, HCC_STAGE_AST, HCC_ENCODING_RUNTIME_BINARY, ast_out);
}

HccResult hcc_task_add_output_aml_text_file(HccTask* t, HccIIO* iio) {
	return hcc_task_add_output(t, HCC_STAGE_AML, HCC_ENCODING_TEXT, iio);
}

HccResult hcc_task_add_output_aml_binary_file(HccTask* t, HccIIO* iio) {
	return hcc_task_add_output(t, HCC_STAGE_AML, HCC_ENCODING_BINARY, iio);
}

HccResult hcc_task_add_output_aml(HccTask* t, HccAML** aml_out) {
	return hcc_task_add_output(t, HCC_STAGE_AML, HCC_ENCODING_RUNTIME_BINARY, aml_out);
}

bool hcc_task_has_started(HccTask* t) {
	return t->c;
}

bool hcc_task_is_complete(HccTask* t) {
	return hcc_mutex_is_locked(&t->is_running_mutex);
}

HccResult hcc_task_result(HccTask* t) {
	if (hcc_task_is_complete(t)) {
		return t->result;
	}

	return (HccResult){ HCC_ERROR_NOT_FINISHED, 0, NULL };
}

HccMessage* hcc_task_messages(HccTask* t, uint32_t* count_out) {
	*count_out = hcc_stack_count(t->message_sys.elmts);
	return t->message_sys.elmts;
}

HccCU* hcc_task_cu(HccTask* t) {
	return t->cu;
}

HccResult hcc_task_wait_for_complete(HccTask* t) {
	hcc_mutex_lock(&t->is_running_mutex);
	hcc_mutex_unlock(&t->is_running_mutex);
	return t->result;
}

HccDuration hcc_task_duration(HccTask* t) {
	return t->duration;
}

HccDuration hcc_task_phase_duration(HccTask* t, HccPhase phase) {
	return t->phase_durations[phase];
}

HccDuration hcc_task_workers_duration_for_type(HccTask* t, HccWorkerJobType type) {
	return t->worker_job_type_durations[type];
}

// ===========================================
//
//
// Worker
//
//
// ===========================================

HccPhase hcc_worker_job_type_phases[HCC_WORKER_JOB_TYPE_COUNT] = {
	[HCC_WORKER_JOB_TYPE_ATAGEN] = HCC_PHASE_FRONTEND,
	[HCC_WORKER_JOB_TYPE_ASTGEN] = HCC_PHASE_FRONTEND,
	[HCC_WORKER_JOB_TYPE_AMLGEN] = HCC_PHASE_FRONTEND,
	[HCC_WORKER_JOB_TYPE_AMLOPT] = HCC_PHASE_OPTIMIZATION,
	[HCC_WORKER_JOB_TYPE_BINARY] = HCC_PHASE_BACKEND,
	[HCC_WORKER_JOB_TYPE_METADATA] = HCC_PHASE_BACKEND,
};

void hcc_worker_init(HccWorker* w, HccCompiler* c, void* call_stack, uintptr_t call_stack_size, HccCompilerSetup* setup) {
	HCC_UNUSED(setup);
	w->c = c;

	HccThreadSetup thread_setup = {
		.thread_main_fn = hcc_worker_main,
		.arg = w,
		.call_stack = call_stack,
		.call_stack_size = call_stack_size,
	};
	hcc_thread_start(&w->thread, &thread_setup);

	w->string_buffer = hcc_stack_init(char, HCC_ALLOC_TAG_WORKER_STRING_BUFFER, setup->worker_string_buffer_grow_size, setup->worker_string_buffer_reserve_size);
	hcc_arena_alctor_init(&w->arena_alctor, HCC_ALLOC_TAG_WORKER_ARENA, setup->worker_arena_size);
}

void hcc_worker_deinit(HccWorker* w) {
	hcc_stack_deinit(w->string_buffer);
	hcc_arena_alctor_deinit(&w->arena_alctor);
}

HccLocation* hcc_worker_alloc_location(HccWorker* w) {
	return HCC_ARENA_ALCTOR_ALLOC_ELMT(HccLocation, &w->arena_alctor);
}

HccTask* hcc_worker_task(HccWorker* w) {
	return w->job.task;
}

HccCU* hcc_worker_cu(HccWorker* w) {
	return w->job.task->cu;
}

void hcc_worker_start_job(HccWorker* w) {
	w->job_start_time = hcc_time_now(HCC_TIME_MODE_MONOTONIC);
#if HCC_ENABLE_WORKER_LOGGING
	printf("[WORKER %2u] starting job %s\n", (int)(w - w->c->workers), hcc_worker_job_type_strings[w->job.type]);
#endif
}

void hcc_worker_end_job(HccWorker* w) {
	HccTask* t = w->job.task;
	HccCompiler* c = t->c;
	HccTime end_time = hcc_time_now(HCC_TIME_MODE_MONOTONIC);
	printf("[WORKER %2u] ending job %s\n", (int)(w - w->c->workers), hcc_worker_job_type_strings[w->job.type]);
	if (c == NULL) {
		//
		// task has failed and is already classed as finished
		return;
	}

	//
	// update the overall worker type duration in the task and compiler
	HccDuration duration = hcc_time_diff(end_time, w->job_start_time);
	t->worker_job_type_durations[w->job.type] = hcc_duration_add(t->worker_job_type_durations[w->job.type], duration);
	c->worker_job_type_durations[w->job.type] = hcc_duration_add(c->worker_job_type_durations[w->job.type], duration);

	if (atomic_fetch_sub(&t->running_jobs_count, 1) == 1) {
		//
		// set the phase duration in the task and add it to the compiler's overall copy
		t->phase_durations[t->phase] = hcc_time_diff(end_time, t->phase_start_times[t->phase]);
		c->phase_durations[t->phase] = hcc_duration_add(c->phase_durations[t->phase], t->phase_durations[t->phase]);

		if (t->phase == hcc_worker_job_type_phases[t->final_worker_job_type]) {
			//
			// we have finished all jobs and have reached the phase where we end.
			// t->final_worker_job_type also stops any jobs being added that
			// are later than it at the start of the hcc_compiler_give_worker_job function.
			hcc_task_finish(w->job.task, true);
			return;
		}

		//
		// setup the next phase worker jobs
		switch (t->phase) {
			case HCC_PHASE_FRONTEND:
				// TODO: loop over all functions and hcc_compiler_give_worker_job for AMLOPT
				break;
			case HCC_PHASE_OPTIMIZATION:
				// TODO: queue job for the backend
				break;
			case HCC_PHASE_BACKEND:
				break;
		}
		t->phase += 1;
		t->phase_start_times[t->phase] = end_time;
	}
}

void hcc_worker_main(void* arg) {
	HccWorker* w = arg;
	HccCompiler* c = w->c;
	HCC_SET_BAIL_JMP_LOC_WORKER();

	while (1) {
		if (!hcc_compiler_take_or_wait_then_take_worker_job(c, &w->job)) {
			//
			// kill the thread when the compiler is stopping
			return;
		}

		hcc_worker_start_job(w);

		switch (w->job.type) {
			case HCC_WORKER_JOB_TYPE_ATAGEN:
				if (!(w->initialized_generators_bitset & (1 << w->job.type))) {
					hcc_atagen_init(w, &c->atagen_setup);
					w->initialized_generators_bitset |= (1 << w->job.type);
				}
				hcc_atagen_reset(w);
				hcc_atagen_generate(w);
				break;
			case HCC_WORKER_JOB_TYPE_ASTGEN:
				break;
			case HCC_WORKER_JOB_TYPE_AMLGEN:
				break;
			case HCC_WORKER_JOB_TYPE_AMLOPT:
				break;
			case HCC_WORKER_JOB_TYPE_BINARY:
				break;
			case HCC_WORKER_JOB_TYPE_METADATA:
				break;
		}

		hcc_worker_end_job(w);
	}
}

// ===========================================
//
//
// Compiler
//
//
// ===========================================

HccCompilerSetup hcc_compiler_setup_default = {
	.atagen = {
		.ppgen = {
			.expand_stack_grow_count = 256,
			.expand_stack_reserve_cap = 1024,
			.stringify_buffer_grow_count = 1024,
			.stringify_buffer_reserve_cap = 32768,
			.if_stack_grow_count = 256,
			.if_stack_reserve_cap = 1024,
			.macro_declarations_cap = 131072,
			.macro_args_stack_grow_count = 1024,
			.macro_args_stack_reserve_cap = 131072,
		},
		.paused_file_stack_grow_count = 256,
		.paused_file_stack_reserve_cap = 1024,
		.open_bracket_stack_grow_count = 256,
		.open_bracket_stack_reserve_cap = 1024,
	},
	.worker_string_buffer_grow_size = 4096,
	.worker_string_buffer_reserve_size = 65536,
	.worker_arena_size = 16384,
	.workers_count = 0,
	.worker_jobs_queue_cap = 4096,
	.worker_call_stack_size = 65536,
};

void hcc_compiler_give_worker_job(HccCompiler* c, HccTask* t, HccWorkerJobType job_type, void* arg) {
	if (t->final_worker_job_type < job_type) {
		// job is out of scope for the requested compilation
		return;
	}

	//
	// spin until we are the thread to claim a slot to write the job into
	uint32_t tail_idx = atomic_load(&c->worker_job_queue.tail_idx);
	while (1) {
		uint32_t head_idx = atomic_load(&c->worker_job_queue.head_idx);
		uint32_t count = tail_idx >= head_idx
			? tail_idx - head_idx
			: tail_idx + (c->worker_job_queue.cap - head_idx);

		if (count + 1 >= c->worker_job_queue.cap) {
			hcc_bail(HCC_ERROR_COLLECTION_FULL, HCC_ALLOC_TAG_WORKER_JOB_QUEUE);
		}

		uint32_t next_tail_idx = (tail_idx + 1) % c->worker_job_queue.cap;
		if (atomic_compare_exchange_weak(&c->worker_job_queue.tail_idx, &tail_idx, next_tail_idx)) {
			break;
		}
	}

	HccWorkerJob* job = &c->worker_job_queue.data[tail_idx];
	job->type = job_type;
	job->task = t;
	job->arg = arg;

	//
	// tell the worker threads that there is a new job
	atomic_fetch_add(&t->running_jobs_count, 1);
	hcc_semaphore_give(&c->worker_job_queue.semaphore, 1);
}

bool hcc_compiler_take_or_wait_then_take_worker_job(HccCompiler* c, HccWorkerJob* job_out) {
	hcc_semaphore_take_or_wait_then_take(&c->worker_job_queue.semaphore);
	if (atomic_load(&c->flags) & HCC_COMPILER_FLAGS_IS_STOPPING) {
		return false;
	}

	//
	// spin until we are the thread to claim this worker job
	uint32_t head_idx = atomic_load(&c->worker_job_queue.head_idx);
	while (1) {
		// pull the data out before we store the updated head_idx back
		*job_out = c->worker_job_queue.data[head_idx];

		uint32_t next_head_idx = (head_idx + 1) % c->worker_job_queue.cap;
		if (atomic_compare_exchange_weak(&c->worker_job_queue.head_idx, &head_idx, next_head_idx)) {
			break;
		}
	}

	return true;
}

HccResult hcc_compiler_init(HccCompilerSetup* setup, HccCompiler** c_out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccCompiler* c = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccCompiler, &_hcc_gs.arena_alctor);
	hcc_clear_bail_jmp_loc();

	HCC_SET_BAIL_JMP_LOC_COMPILER();

	uint32_t workers_count = setup->workers_count;
	if (workers_count == 0) {
		workers_count = 1; //hcc_logical_cores_count();
	}

	c->atagen_setup = setup->atagen;

	//
	// allocate the job queue magic ring buffer
	uint32_t worker_jobs_size = setup->worker_jobs_queue_cap * sizeof(HccWorkerJob);
	worker_jobs_size = hcc_uint32_round_up_to_multiple(worker_jobs_size, _hcc_gs.virt_mem_page_size);
	hcc_virt_mem_magic_ring_buffer_alloc(HCC_ALLOC_TAG_WORKER_JOB_QUEUE, NULL, worker_jobs_size, (void**)&c->worker_job_queue.data);
	c->worker_job_queue.cap = setup->worker_jobs_queue_cap;

	//
	// reserve address space for all of the worker call stacks separated by a page
	// that we not be committed to crash on stack overflows.
	uintptr_t page_size = hcc_virt_mem_page_size();
	uintptr_t worker_call_stack_size = HCC_INT_ROUND_UP_ALIGN(setup->worker_call_stack_size, page_size);

	uintptr_t worker_call_stacks_size
		= (uintptr_t)worker_call_stack_size * (uintptr_t)workers_count
		+ page_size * (uintptr_t)(workers_count + 1);
	worker_call_stacks_size = HCC_INT_ROUND_UP_ALIGN(worker_call_stacks_size, hcc_virt_mem_reserve_align());

	c->worker_call_stacks_size = worker_call_stacks_size;
	hcc_virt_mem_reserve(HCC_ALLOC_TAG_WORKER_CALL_STACKS, NULL, worker_call_stacks_size, (void**)&c->worker_call_stacks_addr);

	void* call_stack = c->worker_call_stacks_addr;
	c->workers = HCC_ARENA_ALCTOR_ALLOC_ARRAY_THREAD_SAFE(HccWorker, &_hcc_gs.arena_alctor, workers_count);
	for (uint32_t worker_idx = 0; worker_idx < workers_count; worker_idx += 1) {
		call_stack = HCC_PTR_ADD(call_stack, page_size);
		hcc_virt_mem_commit(HCC_ALLOC_TAG_WORKER_CALL_STACKS, call_stack, worker_call_stack_size, HCC_VIRT_MEM_PROTECTION_READ_WRITE);
		hcc_worker_init(&c->workers[worker_idx], c, call_stack, worker_call_stack_size, setup);
		call_stack = HCC_PTR_ADD(call_stack, worker_call_stack_size);
	}

	hcc_mutex_init(&c->wait_for_all_mutex);

	hcc_semaphore_init(&c->worker_job_queue.semaphore, 0);

	*c_out = c;
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_compiler_deinit(HccCompiler* c) {
	HCC_SET_BAIL_JMP_LOC_COMPILER();

	atomic_fetch_or(&c->flags, HCC_COMPILER_FLAGS_IS_STOPPING);
	hcc_semaphore_give(&c->worker_job_queue.semaphore, c->workers_count); // wake up all the workers to make stop their threads
	HccResult result = hcc_compiler_wait_for_all_tasks(c);
	for (uint32_t idx = 0; idx < c->workers_count; idx += 1) {
		hcc_worker_deinit(&c->workers[idx]);
	}

	hcc_clear_bail_jmp_loc();
	return result;
}

HccResult hcc_compiler_dispatch_task(HccCompiler* c, HccTask* t) {
	HCC_SET_BAIL_JMP_LOC_COMPILER();

	HCC_ASSERT(t->input_locations, "task has no input");
	HCC_ASSERT(t->c == NULL, "task has is already being used by a compiler instance");
	t->c = c;
	hcc_mutex_lock(&t->is_running_mutex);

	t->result = HCC_RESULT_SUCCESS;
	t->flags &= ~(HCC_TASK_FLAGS_IS_RESULT_SET);
	t->phase = HCC_PHASE_FRONTEND;
	HCC_ZERO_ARRAY(t->worker_job_type_durations);
	HCC_ZERO_ARRAY(t->phase_durations);
	HCC_ZERO_ELMT(&t->duration);
	t->start_time = hcc_time_now(HCC_TIME_MODE_MONOTONIC);
	t->phase_start_times[HCC_PHASE_FRONTEND] = t->start_time;

	if (t->cu) {
		hcc_cu_deinit(t->cu);
		t->cu = NULL;
	}

	t->cu = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccCU, &_hcc_gs.arena_alctor);
	hcc_cu_init(t->cu, &t->cu_setup, &t->target);

	if (atomic_fetch_add(&c->tasks_running_count, 1) == 0) {
		//
		// first task is being added, so lets reset some state.
		//
		hcc_mutex_lock(&c->wait_for_all_mutex);
		c->result_data.result = HCC_RESULT_SUCCESS;
		c->flags &= ~(HCC_COMPILER_FLAGS_IS_RESULT_SET | HCC_COMPILER_FLAGS_IS_STOPPING);
		if (_hcc_gs.flags & HCC_FLAGS_ENABLE_STACKTRACE) {
			c->result_data.result.stacktrace = c->result_data.result_stacktrace;
			c->result_data.result_stacktrace[0] = '\0';
		}
		HCC_ZERO_ARRAY(c->worker_job_type_durations);
		HCC_ZERO_ARRAY(c->phase_durations);
		HCC_ZERO_ELMT(&c->duration);
		c->start_time = t->start_time;

		//
		// CONTRIBUTOR: we want to keep the code files in future and only
		// throw them away if the user calls hcc_clear_code_files
		// and we want to update them when a timestamp mismatches that we can an update from the OS
		//
		// - write a timestamp abstraction
		// - write the code that refetches the file when the timestamp is out of date
		//
		HCC_CONTRIBUTOR_TASK(keep code files around between compiles);
		hcc_clear_code_files(c);
	}

	{
		HccTaskInputLocation* il = t->input_locations;
		while (il) {
			hcc_compiler_give_worker_job(c, t, HCC_WORKER_JOB_TYPE_ATAGEN, il);
			il = il->next;
		}
	}

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_compiler_wait_for_all_tasks(HccCompiler* c) {
	if (atomic_load(&c->tasks_running_count)) {
		hcc_mutex_lock(&c->wait_for_all_mutex);
		hcc_mutex_unlock(&c->wait_for_all_mutex);
	}

	return c->result_data.result;
}

HccResult hcc_compiler_clear_mem_arenas(HccCompiler* c) {
	HCC_SET_BAIL_JMP_LOC_COMPILER();
	for (uint32_t worker_idx = 0; worker_idx < c->workers_count; worker_idx += 1) {
		hcc_arena_alctor_reset(&c->workers[worker_idx].arena_alctor);
	}

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccDuration hcc_compiler_duration(HccCompiler* c) {
	return c->duration;
}

HccDuration hcc_compiler_phase_duration(HccCompiler* c, HccPhase phase) {
	return c->phase_durations[phase];
}

HccDuration hcc_compiler_workers_duration_for_type(HccCompiler* c, HccWorkerJobType type) {
	return c->worker_job_type_durations[type];
}

// ===========================================
//
//
// String Table
//
//
// ===========================================

const char* hcc_string_intrinsic_param_names[HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END] = {
	[HCC_STRING_ID_UINT8_T] = "uint8_t",
	[HCC_STRING_ID_UINT16_T] = "uint16_t",
	[HCC_STRING_ID_UINT32_T] = "uint32_t",
	[HCC_STRING_ID_UINT64_T] = "uint64_t",
	[HCC_STRING_ID_UINTPTR_T] = "uintptr_t",
	[HCC_STRING_ID_INT8_T] = "int8_t",
	[HCC_STRING_ID_INT16_T] = "int16_t",
	[HCC_STRING_ID_INT32_T] = "int32_t",
	[HCC_STRING_ID_INT64_T] = "int64_t",
	[HCC_STRING_ID_INTPTR_T] = "intptr_t",
	[HCC_STRING_ID_HCC_VERTEX_INPUT] = "HccVertexInput",
	[HCC_STRING_ID_VERTEX_IDX] = "vertex_idx",
	[HCC_STRING_ID_INSTANCE_IDX] = "instance_idx",
	[HCC_STRING_ID_HCC_FRAGMENT_INPUT] = "HccFragmentInput",
	[HCC_STRING_ID_FRAG_COORD] = "frag_coord",
	[HCC_STRING_ID_HALF] = "half",
	[HCC_STRING_ID__BITS] = "_bits",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_BOOLX2] = "boolx2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S8X2] =   "s8x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S16X2] =  "s16x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S32X2] =  "s32x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S64X2] =  "s64x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U8X2] =   "u8x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U16X2] =  "u16x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U32X2] =  "u32x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U64X2] =  "u64x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X2] =  "f16x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X2] =  "f32x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X2] =  "f64x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_BOOLX3] = "boolx3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S8X3] =   "s8x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S16X3] =  "s16x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S32X3] =  "s32x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S64X3] =  "s64x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U8X3] =   "u8x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U16X3] =  "u16x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U32X3] =  "u32x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U64X3] =  "u64x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X3] =  "f16x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X3] =  "f32x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X3] =  "f64x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_BOOLX4] = "boolx4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S8X4] =   "s8x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S16X4] =  "s16x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S32X4] =  "s32x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S64X4] =  "s64x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U8X4] =   "u8x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U16X4] =  "u16x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U32X4] =  "u32x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U64X4] =  "u64x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X4] =  "f16x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X4] =  "f32x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X4] =  "f64x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X2X2] ="f16x2x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X2X3] ="f32x2x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X2X4] ="f64x2x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X3X2] ="f16x3x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X3X3] ="f32x3x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X3X4] ="f64x3x4",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X4X2] ="f16x4x2",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X4X3] ="f32x4x3",
	[HCC_STRING_ID_SCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X4X4] ="f64x4x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_BOOLX2] = "pboolx2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S8X2] =   "ps8x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S16X2] =  "ps16x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S32X2] =  "ps32x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S64X2] =  "ps64x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U8X2] =   "pu8x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U16X2] =  "pu16x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U32X2] =  "pu32x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U64X2] =  "pu64x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X2] =  "pf16x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X2] =  "pf32x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X2] =  "pf64x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_BOOLX3] = "pboolx3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S8X3] =   "ps8x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S16X3] =  "ps16x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S32X3] =  "ps32x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S64X3] =  "ps64x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U8X3] =   "pu8x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U16X3] =  "pu16x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U32X3] =  "pu32x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U64X3] =  "pu64x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X3] =  "pf16x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X3] =  "pf32x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X3] =  "pf64x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_BOOLX4] = "pboolx4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S8X4] =   "ps8x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S16X4] =  "ps16x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S32X4] =  "ps32x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_S64X4] =  "ps64x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U8X4] =   "pu8x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U16X4] =  "pu16x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U32X4] =  "pu32x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_U64X4] =  "pu64x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X4] =  "pf16x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X4] =  "pf32x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X4] =  "pf64x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X2X2] = "pf16x2x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X2X3] = "pf32x2x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X2X4] = "pf64x2x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X3X2] = "pf16x3x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X3X3] = "pf32x3x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X3X4] = "pf64x3x4",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F16X4X2] = "pf16x4x2",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F32X4X3] = "pf32x4x3",
	[HCC_STRING_ID_PSCALARX_START + HCC_AML_INTRINSIC_DATA_TYPE_F64X4X4] = "pf64x4x4",
};

void hcc_string_table_intrinsic_add(uint32_t expected_string_id, const char* string) {
	HccStringId id;
	hcc_string_table_deduplicate(string, strlen(string), &id);
	HCC_DEBUG_ASSERT(id.idx_plus_one == expected_string_id, "intrinsic string id for '%s' does not match! expected '%u' but got '%u'", string, expected_string_id, id.idx_plus_one);
}

void hcc_string_table_init(HccStringTable* string_table, uint32_t data_grow_count, uint32_t data_reserve_cap, uint32_t entries_cap) {
	string_table->entries_hash_table = hcc_hash_table_init(HccStringEntry, HCC_ALLOC_TAG_STRING_TABLE_ENTRIES, hcc_string_key_cmp, entries_cap);
	string_table->id_to_entry_map = hcc_stack_init(uint32_t, HCC_ALLOC_TAG_STRING_TABLE_ID_TO_ENTRY_MAP, entries_cap, entries_cap);
	hcc_stack_resize(string_table->id_to_entry_map, entries_cap);
	string_table->data = hcc_stack_init(char, HCC_ALLOC_TAG_STRING_TABLE_DATA, data_grow_count, data_reserve_cap);
	string_table->next_id = 1;

	for (uint32_t expected_string_id = HCC_STRING_ID_INTRINSIC_PARAM_NAMES_START; expected_string_id < HCC_STRING_ID_INTRINSIC_PARAM_NAMES_END; expected_string_id += 1) {
		const char* string = hcc_string_intrinsic_param_names[expected_string_id];
		if (string) {
			hcc_string_table_intrinsic_add(expected_string_id, string);
		} else {
			hcc_string_table_alloc_next_id(&_hcc_gs.string_table);
		}
	}

	for (HccATAToken t = HCC_ATA_TOKEN_KEYWORDS_START; t < HCC_ATA_TOKEN_KEYWORDS_END; t += 1) {
		const char* string = hcc_ata_token_strings[t];
		uint32_t expected_string_id = HCC_STRING_ID_KEYWORDS_START + (t - HCC_ATA_TOKEN_KEYWORDS_START);
		hcc_string_table_intrinsic_add(expected_string_id, string);
	}

	for (HccPPPredefinedMacro m = 0; m < HCC_PP_PREDEFINED_MACRO_COUNT; m += 1) {
		const char* string = hcc_pp_predefined_macro_identifier_strings[m];
		uint32_t expected_string_id = HCC_STRING_ID_PREDEFINED_MACROS_START + m;
		hcc_string_table_intrinsic_add(expected_string_id, string);
	}

	for (uint32_t f = 0; f < HCC_FUNCTION_IDX_INTRINSIC_END; f += 1) {
		//const char* string = hcc_intrinsic_function_strings[f];
		//uint32_t expected_string_id = HCC_STRING_ID_FUNCTION_IDXS_START + f;
		//hcc_string_table_intrinsic_add(expected_string_id, string);
	}

	hcc_string_table_intrinsic_add(HCC_STRING_ID_ONCE, "once");
	hcc_string_table_intrinsic_add(HCC_STRING_ID_DEFINED, "defined");
	hcc_string_table_intrinsic_add(HCC_STRING_ID___VA_ARGS__, "__VA_ARGS__");
}

void hcc_string_table_deinit(HccStringTable* string_table) {
	hcc_hash_table_deinit(string_table->entries_hash_table);
	hcc_stack_deinit(string_table->id_to_entry_map);
	hcc_stack_deinit(string_table->data);
}

HccStringId hcc_string_table_alloc_next_id(HccStringTable* string_table) {
	return HccStringId(atomic_fetch_add(&string_table->next_id, 1));
}

HccResult hcc_string_table_deduplicate(const char* string, uint32_t string_size, HccStringId* out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccStringTable* string_table = &_hcc_gs.string_table;
	HccString str = hcc_string((char*)string, string_size);
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx_string(string_table->entries_hash_table, &str);
	HccStringEntry* entry = &string_table->entries_hash_table[insert.idx];
	if (insert.is_new) {
		char* dst = hcc_stack_push_many_thread_safe(string_table->data, str.size);
		memcpy(dst, str.data, str.size);
		str.data = dst;
		entry->string = str;
		uint32_t id = atomic_fetch_add(&string_table->next_id, 1);
		atomic_store(&entry->id, id);
		*hcc_stack_get(string_table->id_to_entry_map, id) = insert.idx;
	} else {
		//
		// if another thread has just inserted this string into the string table.
		// then wait until the entry identifier has been assigned
		while (atomic_load(&entry->id) == 0) {
			HCC_CPU_RELAX();
		}
	}

	out->idx_plus_one = atomic_load(&entry->id);
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccString hcc_string_table_get(HccStringId id) {
	HCC_DEBUG_ASSERT_NON_ZERO(id.idx_plus_one);
	HccStringTable* string_table = &_hcc_gs.string_table;
	uint32_t entry_idx = *hcc_stack_get(string_table->id_to_entry_map, id.idx_plus_one);
	return string_table->entries_hash_table[entry_idx].string;
}

HccString hcc_string_table_get_or_empty(HccStringId id) {
	HccStringTable* string_table = &_hcc_gs.string_table;
	if (id.idx_plus_one == 0 || id.idx_plus_one >= hcc_hash_table_cap(string_table->entries_hash_table)) {
		return hcc_string(NULL, 0);
	}
	uint32_t entry_idx = *hcc_stack_get(string_table->id_to_entry_map, id.idx_plus_one);
	return string_table->entries_hash_table[entry_idx].string;
}

// ===========================================
//
//
// Code File
//
//
// ===========================================

HccResult hcc_code_file_init(HccCodeFile* code_file, HccString path_string, bool do_not_open_file) {
	code_file->path_string = path_string;
	code_file->line_code_start_indices = hcc_stack_init(uint32_t, 0, _hcc_gs.code_file_lines_grow_count, _hcc_gs.code_file_lines_reserve_cap);
	code_file->pp_if_spans = hcc_stack_init(HccPPIfSpan, 0, _hcc_gs.code_file_pp_if_spans_grow_count, _hcc_gs.code_file_pp_if_spans_reserve_cap);
	hcc_stack_push_many(code_file->line_code_start_indices, 2);

	if (!do_not_open_file) {
		HccIIO iio;
		if (!_hcc_gs.file_open_read_fn(path_string.data, &iio)) {
			return HccResult(HCC_ERROR_FILE_OPEN_READ, 0, NULL);
		}

		uintptr_t alloc_size = HCC_INT_ROUND_UP_ALIGN(iio.size + _HCC_TOKENIZER_LOOK_HEAD_SIZE, _hcc_gs.virt_mem_reserve_align);
		hcc_virt_mem_reserve_commit(HCC_ALLOC_TAG_CODE, NULL, alloc_size, HCC_VIRT_MEM_PROTECTION_READ_WRITE, (void**)&code_file->code.data);

		code_file->code.size = iio.size;
		if (hcc_iio_read(&iio, code_file->code.data, iio.size) == UINTPTR_MAX) {
			return HccResult(HCC_ERROR_FILE_READ, 0, NULL);
		}
	}

	atomic_fetch_or(&code_file->flags, HCC_CODE_FILE_FLAGS_IS_LOADED);
	return HCC_RESULT_SUCCESS;
}

void hcc_code_file_deinit(HccCodeFile* code_file) {
	hcc_stack_deinit(code_file->line_code_start_indices);
	hcc_stack_deinit(code_file->pp_if_spans);
	uintptr_t alloc_size = HCC_INT_ROUND_UP_ALIGN(code_file->code.size + _HCC_TOKENIZER_LOOK_HEAD_SIZE, _hcc_gs.virt_mem_reserve_align);
	hcc_virt_mem_release(HCC_ALLOC_TAG_CODE, code_file->code.data, alloc_size);
}

HccCodeFile* hcc_code_file_find(HccString file_path) {
	uintptr_t idx = hcc_hash_table_find_idx_string(_hcc_gs.path_to_code_file_map, &file_path);
	return idx == UINTPTR_MAX ? NULL : &_hcc_gs.path_to_code_file_map[idx].file;
}

HccResult hcc_code_file_find_or_insert(HccString file_path, HccCodeFile** code_file_out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	HccHashTableInsert insert = hcc_hash_table_find_insert_idx_string(_hcc_gs.path_to_code_file_map, &file_path);
	HccCodeFile* code_file = &_hcc_gs.path_to_code_file_map[insert.idx].file;
	*code_file_out = code_file;
	HccResult result = HCC_RESULT_SUCCESS;
	if (insert.is_new) {
		result = hcc_code_file_init(*code_file_out, file_path, false);
	} else {
		//
		// in the case another thread has just initialized the code file.
		// wait until the file has been fully loaded into memory.
		while (!(atomic_load(&code_file->flags) & HCC_CODE_FILE_FLAGS_IS_LOADED)) {
			HCC_CPU_RELAX();
		}
	}

	if (HCC_IS_SUCCESS(result)) {
		HccCodeFileFlags flags = atomic_load(&code_file->flags);
		while (!(flags & HCC_CODE_FILE_FLAGS_HAS_STARTED_BEING_PARSED)) {
			if (atomic_compare_exchange_weak(&code_file->flags, &flags, flags | HCC_CODE_FILE_FLAGS_HAS_STARTED_BEING_PARSED)) {
				result.code = HCC_SUCCESS_IS_NEW;
				break;
			}
			HCC_CPU_RELAX();
		}
	}

	hcc_clear_bail_jmp_loc();
	return result;
}

HccString hcc_code_file_path_string(HccCodeFile* code_file) {
	return code_file->path_string;
}

HccString hcc_code_file_code(HccCodeFile* code_file) {
	return code_file->code;
}

uint32_t hcc_code_file_line_size(HccCodeFile* code_file, uint32_t line) {
	uint32_t code_start_idx = *hcc_stack_get(code_file->line_code_start_indices, line);
	uint32_t code_end_idx;
	if (line >= hcc_code_file_lines_count(code_file)) {
		code_end_idx = code_file->code.size;
	} else {
		code_end_idx = *hcc_stack_get(code_file->line_code_start_indices, line + 1);

		//
		// move back until we reach the final character on a non empty line
		while (code_end_idx) {
			code_end_idx -= 1;
			uint8_t byte = code_file->code.data[code_end_idx];
			if (byte != '\r' && byte != '\n') {
				code_end_idx += 1;
				break;
			}
		}
	}

	if (code_start_idx < code_end_idx) {
		return code_end_idx - code_start_idx;
	} else {
		return 0;
	}

}

uint32_t hcc_code_file_lines_count(HccCodeFile* code_file) {
	return hcc_stack_count(code_file->line_code_start_indices) - 1;
}

// ===========================================
//
//
// Location
//
//
// ===========================================

void hcc_location_merge_apply(HccLocation* before, HccLocation* after) {
	before->code_end_idx = after->code_end_idx;
	if (before->line_start != after->line_start) {
		before->column_start = after->column_start;
	}
	before->column_end = after->column_end;
	before->line_start = after->line_start;
	before->line_end = after->line_end;
}

// ===========================================
//
//
// Library
//
//
// ===========================================

HccSetup hcc_setup_default = {
	.flags = HCC_FLAGS_NONE,
	.global_mem_arena_size = 8192,
	.alloc_event_fn = NULL,
	.alloc_event_userdata = NULL,
	.path_canonicalize_fn = hcc_path_canonicalize_internal,
	.file_open_read_fn = hcc_file_open_read,
	.string_table_data_grow_count = 1048576,   // 1MB
	.string_table_data_reserve_cap = 67108864, // 64MB
	.string_table_entries_cap = 1048576,
	.code_files_cap = 8192,
	.code_file_lines_grow_count = 512,
	.code_file_lines_reserve_cap = 131072,
	.code_file_pp_if_spans_grow_count = 256,
	.code_file_pp_if_spans_reserve_cap = 16384,
};

HccResult hcc_init(HccSetup* setup) {
	_hcc_gs.flags = setup->flags;
	_hcc_gs.alloc_event_fn = setup->alloc_event_fn;
	_hcc_gs.alloc_event_userdata = setup->alloc_event_userdata;
	_hcc_gs.path_canonicalize_fn = setup->path_canonicalize_fn;
	_hcc_gs.file_open_read_fn = setup->file_open_read_fn;
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	hcc_virt_mem_update_page_size_reserve_align();
	hcc_arena_alctor_init(&_hcc_gs.arena_alctor, HCC_ALLOC_TAG_GLOBAL_MEM_ARENA, setup->global_mem_arena_size);
	hcc_string_table_init(&_hcc_gs.string_table, setup->string_table_data_grow_count, setup->string_table_data_reserve_cap, setup->string_table_entries_cap);

	_hcc_gs.path_to_code_file_map = hcc_hash_table_init(HccCodeFileEntry, 0, hcc_string_key_cmp, setup->code_files_cap);
	_hcc_gs.code_file_lines_grow_count = setup->code_file_lines_grow_count;
	_hcc_gs.code_file_lines_reserve_cap = setup->code_file_lines_reserve_cap;
	_hcc_gs.code_file_pp_if_spans_grow_count = setup->code_file_pp_if_spans_grow_count;
	_hcc_gs.code_file_pp_if_spans_reserve_cap = setup->code_file_pp_if_spans_reserve_cap;

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

void hcc_deinit() {
	hcc_arena_alctor_deinit(&_hcc_gs.arena_alctor);
	hcc_string_table_deinit(&_hcc_gs.string_table);
	for (uint32_t idx = 0; idx < hcc_hash_table_cap(_hcc_gs.path_to_code_file_map); idx += 1) {
		HccCodeFileEntry* entry = &_hcc_gs.path_to_code_file_map[idx];
		if (entry->path_string.data) {
			hcc_code_file_deinit(&entry->file);
		}
	}
	hcc_hash_table_deinit(_hcc_gs.path_to_code_file_map);
}

HccResult hcc_clear_global_mem_arena() {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	hcc_arena_alctor_reset(&_hcc_gs.arena_alctor);

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_clear_code_files() {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	for (uint32_t idx = 0; idx < hcc_hash_table_cap(_hcc_gs.path_to_code_file_map); idx += 1) {
		HccCodeFileEntry* entry = &_hcc_gs.path_to_code_file_map[idx];
		if (entry->path_string.data) {
			hcc_code_file_deinit(&entry->file);
		}
	}
	hcc_hash_table_clear(_hcc_gs.path_to_code_file_map);

	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

HccResult hcc_alloc_location(HccLocation** out) {
	HCC_SET_BAIL_JMP_LOC_GLOBAL();

	*out = HCC_ARENA_ALCTOR_ALLOC_ELMT_THREAD_SAFE(HccLocation, &_hcc_gs.arena_alctor);
	hcc_clear_bail_jmp_loc();
	return HCC_RESULT_SUCCESS;
}

