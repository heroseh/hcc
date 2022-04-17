#include "hcc.c"

int main(int argc, char** argv) {
	HCC_UNUSED(argc);
	HCC_UNUSED(argv);

	HccCompilerSetup compiler_setup = {
		.tokens_cap = 64 * 1024,
		.lines_cap = 64 * 1024,
		.functions_cap = 64 * 1024,
		.function_params_and_variables_cap = 64 * 1024,
		.exprs_cap = 64 * 1024,
		.variable_stack_cap = 64 * 1024,
		.string_table_data_cap = 64 * 1024 * 1024,
		.string_table_entries_cap = 64 * 1024,
	};

	HccCompiler compiler = {0};
	hcc_compiler_init(&compiler, &compiler_setup);

	hcc_compiler_compile(&compiler, "tests/test.c");

	printf("found '%u' tokens\n", compiler.astgen.tokens_count);

	return 0;
}


