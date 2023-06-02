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
#include <hmaths.c>

void print_duration(const char* what, HccDuration d) {
	printf("%s took: %.2fms\n", what, hcc_duration_to_f32_millisecs(d));
}

int main(int argc, char** argv) {
	hcc_register_segfault_handler();

	HccSetup hcc_setup = hcc_setup_default;
	HCC_ENSURE(hcc_init(&hcc_setup));

	HccOptions* options;
	HccOptionsSetup options_setup = hcc_options_setup_default;
	HCC_ENSURE(hcc_options_init(&options_setup, &options));

	HccCompiler* compiler;
	HccCompilerSetup compiler_setup = hcc_compiler_setup_default;
	HCC_ENSURE(hcc_compiler_init(&compiler_setup, &compiler));

	HccTask* task;
	HccTaskSetup task_setup = hcc_task_setup_default;
	task_setup.options = options;
	HCC_ENSURE(hcc_task_init(&task_setup, &task));

	int arg_idx = 1;
	const char* output_file_path = NULL;
	bool output_final_file = true;
	bool has_input = false;
	bool debug_time = false;
	const char* hlsl_dir = NULL;
	const char* msl_dir = NULL;
	bool enable_stdout_color = true;
#ifdef HCC_OS_LINUX
	enable_stdout_color &= isatty(fileno(stdout)) && strcmp(getenv("TERM"), "xterm-256color") == 0;
#endif
	while (arg_idx < argc) {
		if (strcmp(argv[arg_idx], "-I") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-I' is missing a following include path. eg. '-I path/to/directory'\n");
				exit(1);
			}

			const char* include_path = argv[arg_idx];
			if (!hcc_path_exists(include_path)) {
				fprintf(stderr, "-I '%s' path does not exist\n", include_path);
				exit(1);
			}
			if (!hcc_path_is_directory(include_path)) {
				fprintf(stderr, "-I '%s' is not a directory\n", include_path);
				exit(1);
			}

			HCC_ENSURE(hcc_task_add_include_path(task, hcc_string_c((char*)include_path)));
		} else if (strcmp(argv[arg_idx], "-fi") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-fi' is missing a following input file path to follow '-fi path/to/file.c'\n");
				exit(1);
			}

			const char* input_file_path = argv[arg_idx];
			if (!hcc_path_exists(input_file_path)) {
				fprintf(stderr, "-fi '%s' path does not exist\n", input_file_path);
				exit(1);
			}
			if (!hcc_path_is_file(input_file_path)) {
				fprintf(stderr, "-fi '%s' is not a directory\n", input_file_path);
				exit(1);
			}

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 2 && path[path_size - 2] == '.' && path[path_size - 1] == 'c')) {
				fprintf(stderr, "'-fi %s' is supposed to have a .c file extension\n", path);
				exit(1);
			}

			HCC_ENSURE(hcc_task_add_input_code_file(task, input_file_path, NULL));
			has_input = true;
		} else if (strcmp(argv[arg_idx], "-fo") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-fo' is missing a following output file path to follow '-fo path/to/file.spirv'\n");
				exit(1);
			}

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 6 && path[path_size - 6] == '.' && path[path_size - 5] == 's' && path[path_size - 4] == 'p' && path[path_size - 3] == 'i' && path[path_size - 2] == 'r' && path[path_size - 1] == 'v')) {
				fprintf(stderr, "'-fo %s' is supposed to have a .spirv file extension\n", path);
				exit(1);
			}

			if (output_file_path) {
				fprintf(stderr, "there can only be a single '-fo' argument... '-fo %s' is the second '-fo' argument\n", path);
				exit(1);
			}

			output_file_path = argv[arg_idx];
			HccIIO* binary_iio = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccIIO, &_hcc_gs.arena_alctor);
			hcc_file_open_write(output_file_path, binary_iio);
			HCC_ENSURE(hcc_task_add_output_binary(task, binary_iio));
		} else if (strcmp(argv[arg_idx], "-fomc") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-fomc' is missing a following output file path. eg. '-fomc path/to/file.h\n");
				exit(1);
			}

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 2 && path[path_size - 2] == '.' && path[path_size - 1] == 'h')) {
				fprintf(stderr, "'-fomc %s' is supposed to have a .h file extension\n", path);
				exit(1);
			}

			HccIIO* iio = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccIIO, &_hcc_gs.arena_alctor);
			hcc_file_open_write(path, iio);
			HCC_ENSURE(hcc_task_add_output_metadata_c(task, iio));
		} else if (strcmp(argv[arg_idx], "-fomjson") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'-fomjson' is missing a following output file path. eg. '-fomjson path/to/file.json\n");
				exit(1);
			}

			const char* path = argv[arg_idx];
			uint32_t path_size = strlen(path);
			if (!(path_size > 5 && path[path_size - 5] == '.' && path[path_size - 4] == 'j' && path[path_size - 3] == 's' && path[path_size - 2] == 'o' && path[path_size - 1] == 'n')) {
				fprintf(stderr, "'-fomjson %s' is supposed to have a .json file extension\n", path);
				exit(1);
			}

			HccIIO* iio = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccIIO, &_hcc_gs.arena_alctor);
			hcc_file_open_write(path, iio);
			HCC_ENSURE(hcc_task_add_output_metadata_json(task, iio));
		} else if (strcmp(argv[arg_idx], "-O") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_SPIRV_OPT, true);
		} else if (strcmp(argv[arg_idx], "--hlsl-packing") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_HLSL_PACKING, true);
		} else if (strcmp(argv[arg_idx], "--hlsl") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'--hlsl' is missing a following input directory path to follow '--hlsl path/to/directory'\n");
				exit(1);
			}

			hlsl_dir = argv[arg_idx];

			if (hcc_path_exists(hlsl_dir) && hcc_path_is_file(hlsl_dir)) {
				fprintf(stderr, "--hlsl '%s' path is a file and not a directory\n", hlsl_dir);
				exit(1);
			}
			hcc_options_set_bool(options, HCC_OPTION_KEY_HLSL_PACKING, true);
		} else if (strcmp(argv[arg_idx], "--msl") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'--msl' is missing a following input directory path to follow '--msl path/to/directory'\n");
				exit(1);
			}

			msl_dir = argv[arg_idx];

			if (hcc_path_exists(msl_dir) && hcc_path_is_file(msl_dir)) {
				fprintf(stderr, "--msl '%s' path is a file and not a directory\n", msl_dir);
				exit(1);
			}
		} else if (strcmp(argv[arg_idx], "--max-descriptors") == 0) {
			arg_idx += 1;
			if (arg_idx == argc) {
				fprintf(stderr, "'--max-descriptors' is missing a following integer for the maximum number of descriptors\n");
				exit(1);
			}

			const char* a = argv[arg_idx];
			uint32_t size = strlen(a);

			char* end_ptr;
			long num = strtoul(a, &end_ptr, 10);
			if (a + size != end_ptr) {
				fprintf(stderr, "'--max-descriptors %s' argument is not an unsigned integer\n", a);
				exit(1);
			}

			hcc_options_set_u32(options, HCC_OPTION_KEY_RESOURCE_DESCRIPTORS_MAX, num);
		} else if (strcmp(argv[arg_idx], "--disable-color") == 0) {
			enable_stdout_color = false;
		} else if (strcmp(argv[arg_idx], "--enable-int8") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_INT8_ENABLED, true);
		} else if (strcmp(argv[arg_idx], "--enable-int16") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_INT16_ENABLED, true);
		} else if (strcmp(argv[arg_idx], "--enable-int64") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_INT64_ENABLED, true);
		} else if (strcmp(argv[arg_idx], "--enable-float16") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_FLOAT16_ENABLED, true);
		} else if (strcmp(argv[arg_idx], "--enable-float64") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_FLOAT64_ENABLED, true);
		} else if (strcmp(argv[arg_idx], "--enable-unordered-swizzling") == 0) {
			hcc_options_set_bool(options, HCC_OPTION_KEY_UNORDERED_SWIZZLING_ENABLED, true);
		} else if (strcmp(argv[arg_idx], "--debug-time") == 0) {
			debug_time = true;
		} else if (strcmp(argv[arg_idx], "--debug-ata") == 0) {
			HccIIO* stdout_iio = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccIIO, &_hcc_gs.arena_alctor);
			*stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(stdout_iio, enable_stdout_color);
			HCC_ENSURE(hcc_task_add_output_ast_text(task, stdout_iio));
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_ATAGEN);
			output_final_file = false;
		} else if (strcmp(argv[arg_idx], "--debug-ast") == 0) {
			HccIIO* stdout_iio = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccIIO, &_hcc_gs.arena_alctor);
			*stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(stdout_iio, enable_stdout_color);
			HCC_ENSURE(hcc_task_add_output_ast_text(task, stdout_iio));
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_ASTLINK);
			output_final_file = false;
		} else if (strcmp(argv[arg_idx], "--debug-aml") == 0) {
			HccIIO* stdout_iio = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccIIO, &_hcc_gs.arena_alctor);
			*stdout_iio = hcc_iio_file(stdout);
			hcc_iio_set_ascii_colors_enabled(stdout_iio, enable_stdout_color);
			HCC_ENSURE(hcc_task_add_output_aml_text(task, stdout_iio));
			hcc_task_set_final_worker_job_type(task, HCC_WORKER_JOB_TYPE_AMLOPT);
			output_final_file = false;
		} else if (strcmp(argv[arg_idx], "--help") == 0) {
			printf(
				"hcc version 0.1.0 help:\n"
				"%s OPTIONS\n"
				"OPTIONS:\n"
				"\t-fi   <path>.c               | <path>.c to a C file to compile\n"
				"\t-fo   <path>.spirv           | <path>.spirv to where you want the output file to go\n"
				"\t-fomc <path>.h               | <path>.h to where you want the output metadata file to go\n"
				"\t-I    <path>                 | add an include search directory path for #include <...>\n"
				"\t-O                           | turn on optimizations, currently using spirv-opt\n"
				"\t--hlsl-packing               | errors on bundled constants if they do not follow the HLSL packing rules for cbuffers. --hlsl also enables this\n"
				"\t--hlsl <path>                | path to a directory where the HLSL files will go. requires spirv-cross to be installed\n"
				"\t--msl  <path>                | path to a directory where the MSL files will go. requires spirv-cross to be installed\n"
				"\t--max-descriptors <int>      | sets the size of the resource descriptors arrays\n"
				"\t--disable-color              | disables color output when printing to stdout\n"
				"\t--enable-int8                | enables 8bit integer support\n"
				"\t--enable-int16               | enables 16bit integer support\n"
				"\t--enable-int64               | enables 64bit integer support\n"
				"\t--enable-float16             | enables 16bit float support\n"
				"\t--enable-float64             | enables 64bit float support\n"
				"\t--enable-unordered-swizzling | allows for vector swizzling x, y, z, w out of order eg. .zyx or .xx or .yyzz \n"
				"\t--help                       | displays this prompt and then exits\n"
				"\t--debug-time                 | prints the duration of each compiliation stage of the compiler\n"
				"\t--debug-ata                  | prints the Abstract Token Array made by the compiler, it will stop after ATAGEN stage\n"
				"\t--debug-ast                  | prints the Abstract Syntax Tree made by the compiler, it will stop after ASTGEN stage\n"
				"\t--debug-aml                  | prints the Abstract Machine Language made by the compiler, it will stop after AMLGEN stage\n"
				, argv[0]
			);
			exit(0);
		} else {
			fprintf(stderr, "invalid argument '%s'\n", argv[arg_idx]);
			exit(1);
		}

		arg_idx += 1;
	}

	{
		HccString path = hcc_path_replace_file_name(hcc_string_c(argv[0]), hcc_string_lit("libc"));
		HCC_ENSURE(hcc_task_add_include_path(task, path));
	}

	{
		HccString path = hcc_path_replace_file_name(hcc_string_c(argv[0]), hcc_string_lit("libhccintrinsics"));
		HCC_ENSURE(hcc_task_add_include_path(task, path));
	}

	{
		HccString path = hcc_path_replace_file_name(hcc_string_c(argv[0]), hcc_string_lit("libhmaths"));
		HCC_ENSURE(hcc_task_add_include_path(task, path));

		path = hcc_path_replace_file_name(hcc_string_c(argv[0]), hcc_string_lit("libhmaths/hmaths.c"));
		HCC_ENSURE(hcc_task_add_input_code_file(task, path.data, NULL));
	}

	if (!has_input) {
		fprintf(stderr, "missing input file/s. please call hcc with one or more '-fi' flag/s followed by the .c file/s you wish to compile\n");
		exit(1);
	}

	if (!output_file_path) {
		fprintf(stderr, "missing the output file. please call hcc with a '-fo' flag followed by the .spirv file you wish to output\n");
		exit(1);
	}

	hcc_compiler_dispatch_task(compiler, task);
	HccResult result = hcc_task_wait_for_complete(task);

	HccIIO iio = hcc_iio_file(stdout);
	hcc_iio_set_ascii_colors_enabled(&iio, enable_stdout_color);

	uint32_t messages_count;
	HccMessage* messages = hcc_task_messages(task, &messages_count);
	for (uint32_t idx = 0; idx < messages_count; idx += 1) {
		HccMessage* m = hcc_stack_get(messages, idx);
		hcc_message_print(&iio, m);
	}

	HccDuration hlsl_duration = {0};
	HccDuration msl_duration = {0};
	if (result.code == HCC_ERROR_MESSAGES) {
		return 1;
	} else {
		HCC_ENSURE(result);

		if (output_file_path && output_final_file) {
			char shell_command[1024];
			if (hcc_options_get_bool(options, HCC_OPTION_KEY_SPIRV_OPT)) {
				snprintf(shell_command, sizeof(shell_command), "spirv-opt%s --scalar-block-layout -O -o %s %s", HCC_EXE_EXTENSION, output_file_path, output_file_path);
			} else {
				snprintf(shell_command, sizeof(shell_command), "spirv-val%s --scalar-block-layout %s", HCC_EXE_EXTENSION, output_file_path);
			}
			int res = hcc_execute_shell_command(shell_command);
			if (res != 0) {
				if (res == 1 || res == 256 /* 1 || 256 is return by the SPIR-V tools for a general error */) {
					printf("Please report this error on the HCC github issue tracker\n");
					exit(1);
				} else {
					printf(
						"WARNING: successfully wrote output file '%s' but failed to execute '%s'.\n"
						"make sure you have the SPIR-V tools installed if you want SPIR-V validation and optimization done in this compiler",
						output_file_path, shell_command
					);
				}
			}

			if (hlsl_dir) {
				HccTime hlsl_start_time = hcc_time_now(HCC_TIME_MODE_MONOTONIC);
				HccCU* cu = task->cu;
				if (!hcc_make_directory(hlsl_dir) && !hcc_path_is_directory(hlsl_dir)) {
					char buf[1024];
					hcc_get_last_system_error_string(buf, sizeof(buf));
					fprintf(stderr, "failed to make directory at '%s': %s\n", hlsl_dir, buf);
					exit(1);
				}

				for (uint32_t shader_idx = 0; shader_idx < hcc_stack_count(cu->shader_function_decls); shader_idx += 1) {
					HccDecl decl = cu->shader_function_decls[shader_idx];
					HccASTFunction* function = hcc_ast_function_get(cu, decl);
					HccString function_name = hcc_string_table_get(function->identifier_string_id);

					snprintf(
						shell_command, sizeof(shell_command),
						"spirv-cross%s --hlsl --shader-model 67 --entry %.*s %s --output %s/%.*s.hlsl",
						HCC_EXE_EXTENSION, (int)function_name.size, function_name.data, output_file_path, hlsl_dir, (int)function_name.size, function_name.data
					);

					int res = hcc_execute_shell_command(shell_command);
					if (res != 0) {
						if (res == 256 /* 256 is return by the SPIR-V tools for a general error */) {
							printf("Please report this error on the HCC github issue tracker\n");
							exit(1);
						} else {
							printf(
								"WARNING: successfully wrote output file '%s' but failed to execute '%s'.\n"
								"make sure you have the SPIR-V tools installed if you want SPIR-V validation and optimization done in this compiler",
								output_file_path, shell_command
							);
						}
					}
				}
				hlsl_duration = hcc_time_elapsed(hlsl_start_time, HCC_TIME_MODE_MONOTONIC);
			}

			if (msl_dir) {
				HccTime msl_start_time = hcc_time_now(HCC_TIME_MODE_MONOTONIC);
				HccCU* cu = task->cu;
				if (!hcc_make_directory(msl_dir) && !hcc_path_is_directory(msl_dir)) {
					char buf[1024];
					hcc_get_last_system_error_string(buf, sizeof(buf));
					fprintf(stderr, "failed to make directory at '%s': %s\n", msl_dir, buf);
					exit(1);
				}

				for (uint32_t shader_idx = 0; shader_idx < hcc_stack_count(cu->shader_function_decls); shader_idx += 1) {
					HccDecl decl = cu->shader_function_decls[shader_idx];
					HccASTFunction* function = hcc_ast_function_get(cu, decl);
					HccString function_name = hcc_string_table_get(function->identifier_string_id);

					snprintf(
						shell_command, sizeof(shell_command),
						"spirv-cross%s --msl --msl-version 20100 --msl-argument-buffers --entry %.*s %s --output %s/%.*s.msl",
						HCC_EXE_EXTENSION, (int)function_name.size, function_name.data, output_file_path, msl_dir, (int)function_name.size, function_name.data
					);

					int res = hcc_execute_shell_command(shell_command);
					if (res != 0) {
						if (res == 256 /* 256 is return by the SPIR-V tools for a general error */) {
							printf("Please report this error on the HCC github issue tracker\n");
							exit(1);
						} else {
							printf(
								"WARNING: successfully wrote output file '%s' but failed to execute '%s'.\n"
								"make sure you have the SPIR-V tools installed if you want SPIR-V validation and optimization done in this compiler",
								output_file_path, shell_command
							);
						}
					}
				}
				msl_duration = hcc_time_elapsed(msl_start_time, HCC_TIME_MODE_MONOTONIC);
			}
		}
	}

	if (debug_time) {
		print_duration("compiler", hcc_compiler_duration(compiler));
		for (HccWorkerJobType job_type = 0; job_type < HCC_WORKER_JOB_TYPE_COUNT; job_type += 1) {
			print_duration(hcc_worker_job_type_strings[job_type], hcc_compiler_worker_job_type_duration(compiler, job_type));
		}
		if (hlsl_dir) {
			print_duration("HLSL (spirv-cross)", hlsl_duration);
		}
		if (msl_dir) {
			print_duration("MSL (spirv-cross)", msl_duration);
		}
	}

	return 0;
}

