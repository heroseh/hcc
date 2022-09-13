#include "hcc.c"

int main(int argc, char** argv) {
	HCC_UNUSED(argc);
	HCC_UNUSED(argv);
	HccCompilerSetup compiler_setup = hcc_compiler_setup_default;

	HccCompiler compiler = {0};
	hcc_compiler_init(&compiler, &compiler_setup);

	if (!hcc_compiler_compile(&compiler, "tests/demonstrate_really_dumb_very_annoying_no_good_preprocessor_feature.c")) {
		if (compiler.allocation_failure_alloc_tag != HCC_ALLOC_TAG_NONE) {
			printf("failed to allocate '%s'\n", hcc_alloc_tag_strings[compiler.allocation_failure_alloc_tag]);
		}

		if (compiler.collection_is_full_alloc_tag != HCC_ALLOC_TAG_NONE) {
			printf("collection is full '%s'\n", hcc_alloc_tag_strings[compiler.collection_is_full_alloc_tag]);
		}

		for (U32 idx = 0; idx < hcc_stack_count(compiler.message_sys.elmts); idx += 1) {
			HccMessage* message = hcc_stack_get(compiler.message_sys.elmts, idx);
			hcc_message_print(&compiler, message);
		}
	}

	return 0;
}


