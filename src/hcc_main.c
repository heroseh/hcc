#include "hcc.c"

int main(int argc, char** argv) {
	HCC_UNUSED(argc);
	HCC_UNUSED(argv);

	HccCompilerSetup compiler_setup = hcc_compiler_setup_default;

	HccCompiler compiler = {0};
	hcc_compiler_init(&compiler, &compiler_setup);

	hcc_compiler_compile(&compiler, "tests/test.c");

	return 0;
}


