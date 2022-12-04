#include <stdlib.h>

#include "core.c"
#include "ata.c"
#include "ast.c"
#include "aml.c"
#include "atagen.c"
#include "astgen.c"
#include "astlink.c"
#include "hcc.c"

void print_duration(const char* what, HccDuration d) {
	printf("%s took: %.2fms\n", what, hcc_duration_to_f32_millisecs(d));
}

int main(int argc, char** argv) {
	HCC_UNUSED(argc);
	HCC_UNUSED(argv);

	HccSetup hcc_setup = hcc_setup_default;
	HCC_ENSURE(hcc_init(&hcc_setup));

	HccOptions* options;
	HccOptionsSetup options_setup = hcc_options_setup_default;
	HCC_ENSURE(hcc_options_init(&options_setup, &options));

	HccCompiler* compiler;
	HccCompilerSetup compiler_setup = hcc_compiler_setup_default;
	HCC_ENSURE(hcc_compiler_init(&compiler_setup, &compiler))

	HccTask* task;
	HccTaskSetup task_setup = hcc_task_setup_default;
	task_setup.options = options;
	task_setup.final_worker_job_type = HCC_WORKER_JOB_TYPE_ASTLINK;
	HCC_ENSURE(hcc_task_init(&task_setup, &task));
	HCC_ENSURE(hcc_task_add_input_code_file(task, "tests/astgen_test.c", NULL));
	HCC_ENSURE(hcc_task_add_input_code_file(task, "tests/astgen_test2.c", NULL));

	HccIIO stdout_iio = hcc_iio_file(stdout);
	hcc_iio_set_ascii_colors_enabled(&stdout_iio, true);
	HCC_ENSURE(hcc_task_add_output_ast_text(task, &stdout_iio));

	hcc_compiler_dispatch_task(compiler, task);
	HccResult result = hcc_task_wait_for_complete(task);

	if (result.code == HCC_ERROR_MESSAGES) {
		HccIIO iio = hcc_iio_file(stdout);
		hcc_iio_set_ascii_colors_enabled(&iio, true);

		uint32_t messages_count;
		HccMessage* messages = hcc_task_messages(task, &messages_count);
		for (uint32_t idx = 0; idx < messages_count; idx += 1) {
			HccMessage* m = hcc_stack_get(messages, idx);
			hcc_message_print(&iio, m);
		}
		return 1;
	} else {
		HCC_ENSURE(result);
	}

	print_duration("compiler", hcc_compiler_duration(compiler));
	for (HccPhase phase = 0; phase < HCC_PHASE_COUNT; phase += 1) {
		print_duration(hcc_phase_strings[phase], hcc_compiler_phase_duration(compiler, phase));
	}

	return 0;
}


