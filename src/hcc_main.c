#include <stdlib.h>

#include "core.c"
#include "ata.c"
#include "ast.c"
#include "aml.c"
#include "atagen.c"
#include "astgen.c"
#include "astlink.c"
#include "amlgen.c"
#include "amlopt.c"
#include "spirv.c"
#include "spirvgen.c"
#include "spirvlink.c"
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
	hcc_options_set_bool(options, HCC_OPTION_KEY_PHYSICAL_POINTER_ENABLED, true);

	HccCompiler* compiler;
	HccCompilerSetup compiler_setup = hcc_compiler_setup_default;
	HCC_ENSURE(hcc_compiler_init(&compiler_setup, &compiler))

	HccTask* task;
	HccTaskSetup task_setup = hcc_task_setup_default;
	task_setup.options = options;
	task_setup.final_worker_job_type = HCC_WORKER_JOB_TYPE_BACKENDLINK;
	HCC_ENSURE(hcc_task_init(&task_setup, &task));
	HCC_ENSURE(hcc_task_add_input_code_file(task, "tests/test.c", NULL));

	HccIIO stdout_iio = hcc_iio_file(stdout);
	HccIIO binary_iio = hcc_iio_file(fopen("tests/test.spirv", "wb"));
	hcc_iio_set_ascii_colors_enabled(&stdout_iio, true);
	//HCC_ENSURE(hcc_task_add_output_ast_text(task, &stdout_iio));
	HCC_ENSURE(hcc_task_add_output_aml_text(task, &stdout_iio));
	HCC_ENSURE(hcc_task_add_output_binary(task, &binary_iio));

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
	for (HccWorkerJobType job_type = 0; job_type < HCC_WORKER_JOB_TYPE_COUNT; job_type += 1) {
		print_duration(hcc_worker_job_type_strings[job_type], hcc_compiler_worker_job_type_duration(compiler, job_type));
	}

	return 0;
}


