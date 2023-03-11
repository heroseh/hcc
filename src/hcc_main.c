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
#include "metadatagen.c"
#include "../interop/hcc_interop.c"
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
	const char* output_file_path = NULL;
	bool debug_time = false;
	while (arg_idx < argc) {
		if (strcmp(argv[arg_idx], "-I") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-I' is missing a following include path. eg. '-I path/to/directory'");
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
				fprintf(stderr, "'-fi' is missing a following input file path to follow '-fi path/to/file.c'");
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

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 2 && path[path_size - 2] == '.' && path[path_size - 1] == 'c')) {
				fprintf(stderr, "'-fi %s' is supposed to have a .c file extension", path);
				exit(1);
			}

			HCC_ENSURE(hcc_task_add_input_code_file(task, input_file_path, NULL));
		} else if (strcmp(argv[arg_idx], "-fo") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-fo' is missing a following output file path to follow '-fo path/to/file.spirv'");
				exit(1);
			}

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 6 && path[path_size - 6] == '.' && path[path_size - 5] == 's' && path[path_size - 4] == 'p' && path[path_size - 3] == 'i' && path[path_size - 2] == 'r' && path[path_size - 1] == 'v')) {
				fprintf(stderr, "'-fo %s' is supposed to have a .spirv file extension", path);
				exit(1);
			}

			output_file_path = argv[arg_idx];
			HccIIO binary_iio;
			hcc_file_open_write(output_file_path, &binary_iio);
			HCC_ENSURE(hcc_task_add_output_binary(task, &binary_iio));
			hcc_iio_flush(&binary_iio);
		} else if (strcmp(argv[arg_idx], "-fomc") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-fomc' is missing a following output file path. eg. '-fomc path/to/file.h");
				exit(1);
			}

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 2 && path[path_size - 2] == '.' && path[path_size - 1] == 'h')) {
				fprintf(stderr, "'-fomc %s' is supposed to have a .h file extension", path);
				exit(1);
			}

			HccIIO iio;
			hcc_file_open_write(path, &iio);
			HCC_ENSURE(hcc_task_add_output_metadata_c(task, &iio));
		} else if (strcmp(argv[arg_idx], "-fomjson") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-fomjson' is missing a following output file path. eg. '-fomjson path/to/file.json");
				exit(1);
			}

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 5 && path[path_size - 5] == '.' && path[path_size - 4] == 'j' && path[path_size - 3] == 's' && path[path_size - 2] == 'o' && path[path_size - 1] == 'n')) {
				fprintf(stderr, "'-fomjson %s' is supposed to have a .json file extension", path);
				exit(1);
			}

			HccIIO iio;
			hcc_file_open_write(path, &iio);
			HCC_ENSURE(hcc_task_add_output_metadata_json(task, &iio));
		} else if (strcmp(argv[arg_idx], "-O") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_SPIRV_OPT, true);
		} else if (strcmp(argv[arg_idx], "--debug-time") == 0) {
			debug_time = true;
		} else if (strcmp(argv[arg_idx], "--debug-ata") == 0) {
			HccIIO stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(&stdout_iio, true);
			HCC_ENSURE(hcc_task_add_output_ast_text(task, &stdout_iio));
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_ATAGEN);
		} else if (strcmp(argv[arg_idx], "--debug-ast") == 0) {
			HccIIO stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(&stdout_iio, true);
			HCC_ENSURE(hcc_task_add_output_ast_text(task, &stdout_iio));
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_ASTLINK);
		} else if (strcmp(argv[arg_idx], "--debug-aml") == 0) {
			HccIIO stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(&stdout_iio, true);
			HCC_ENSURE(hcc_task_add_output_aml_text(task, &stdout_iio));
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_AMLOPT);
		} else if (strcmp(argv[arg_idx], "--help") == 0) {
			printf(
				"hcc version 0.0.1 help:\n"
				"%s OPTIONS\n"
				"OPTIONS:\n"
				"\t-fi   <path>.c     | <path>.c to an C file to compile\n"
				"\t-fo   <path>.spirv | <path>.spirv to where you want the output file to go\n"
				"\t-fomc <path>.h     | <path>.h to where you want the output metadata file to go\n"
				"\t-I    <path>       | add an include search directory path for #include <...>\n"
				"\t-O                 | turns on optimization, currently using spirv-opt\n"
				"\t--help             | displays this prompt and then exits\n"
				"\t--debug-time       | prints the duration of each compiliation stage of the compiler\n"
				"\t--debug-ata        | prints the Abstract Token Array made by the compiler, it will stop after ATA stage\n"
				"\t--debug-ast        | prints the Abstract Syntax Tre made by the compiler, it will stop after ASTGEN stage\n"
				"\t--debug-aml        | prints the Abstract Machine Language made by the compiler, it will stop after AMLGEN stage\n"
				, argv[0]
			);
			exit(0);
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

		if (output_file_path) {
			char shell_command[1024];
			if (hcc_options_get_bool(options, HCC_OPTION_KEY_SPIRV_OPT)) {
				snprintf(shell_command, sizeof(shell_command), "spirv-opt%s --scalar-block-layout -O -o %s %s", HCC_EXE_EXTENSION, output_file_path, output_file_path);
			} else {
				snprintf(shell_command, sizeof(shell_command), "spirv-val%s --scalar-block-layout %s", HCC_EXE_EXTENSION, output_file_path);
			}
			if (hcc_execute_shell_command(shell_command) != 0) {
				exit(1);
			}
		}
	}

	if (debug_time) {
		print_duration("compiler", hcc_compiler_duration(compiler));
		for (HccWorkerJobType job_type = 0; job_type < HCC_WORKER_JOB_TYPE_COUNT; job_type += 1) {
			print_duration(hcc_worker_job_type_strings[job_type], hcc_compiler_worker_job_type_duration(compiler, job_type));
		}
	}

	return 0;
}


