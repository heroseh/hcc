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
	HCC_ENSURE(hcc_task_init(&task_setup, &task));

	int arg_idx = 1;
	while (arg_idx < argc) {
		if (strcmp(argv[arg_idx], "-I") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "command argument stream ended in a '-I', we expect an include path to follow '-I'");
				exit(1);
			}

			const char* include_path = argv[arg_idx];
			if (!hcc_path_exists(include_path)) {
				fprintf(stderr, "-I '%s' path does not exist", include_path);
				exit(1);
			}
			if (!hcc_path_is_directory(include_path)) {
				fprintf(stderr, "-I '%s' is not a directory", include_path);
				exit(1);
			}

			HCC_ENSURE(hcc_task_add_include_path(task, hcc_string_c((char*)include_path)));
		} else if (strcmp(argv[arg_idx], "-fi") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "command argument stream ended in a '-fi', we expect an input file path to follow '-fi'");
				exit(1);
			}

			const char* input_file_path = argv[arg_idx];
			if (!hcc_path_exists(input_file_path)) {
				fprintf(stderr, "-fi '%s' path does not exist", input_file_path);
				exit(1);
			}
			if (!hcc_path_is_file(input_file_path)) {
				fprintf(stderr, "-fi '%s' is not a directory", input_file_path);
				exit(1);
			}

			HCC_ENSURE(hcc_task_add_input_code_file(task, input_file_path, NULL));
		} else if (strcmp(argv[arg_idx], "-fo") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "command argument stream ended in a '-fo', we expect an output file path to follow '-fo'");
				exit(1);
			}

			const char* output_file_path = argv[arg_idx];
			HccIIO binary_iio = hcc_iio_file(fopen(output_file_path, "wb"));
			HCC_ENSURE(hcc_task_add_output_binary(task, &binary_iio));
		} else if (strcmp(argv[arg_idx], "--enable-physical-pointer") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_PHYSICAL_POINTER_ENABLED, true);
		} else if (strcmp(argv[arg_idx], "--output-ast") == 0) {
			HccIIO stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(&stdout_iio, true);
			HCC_ENSURE(hcc_task_add_output_ast_text(task, &stdout_iio));
		} else if (strcmp(argv[arg_idx], "--output-aml") == 0) {
			HccIIO stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(&stdout_iio, true);
			HCC_ENSURE(hcc_task_add_output_aml_text(task, &stdout_iio));
		} else if (strcmp(argv[arg_idx], "--final-job-ast") == 0) {
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_ASTLINK);
		} else if (strcmp(argv[arg_idx], "--final-job-aml") == 0) {
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_AMLOPT);
		} else {
			fprintf(stderr, "invalid argument '%s'", argv[arg_idx]);
			exit(1);
		}

		arg_idx += 1;
	}

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


