#include "hcc_internal.h"

// ===========================================
//
//
// AML Function
//
//
// ===========================================

HccAMLFunction* hcc_aml_function_take_ref(HccCU* cu, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_FUNCTION(decl), "internal error: expected a function declaration");
	HCC_DEBUG_ASSERT(!HCC_DECL_IS_FORWARD_DECL(decl), "internal error: expected a function declaration that is not a forward declaration");

	HccAtomic(HccAMLFunction*)* function_ptr = hcc_stack_get(cu->aml.functions, HCC_DECL_AUX(decl));
	while (1) {
		HccAMLFunction* function = atomic_load(function_ptr);
		if (atomic_fetch_add(&function->ref_count, 1) != 0) {
			return function;
		}

		atomic_fetch_sub(&function->ref_count, 1);
	}
}

void hcc_aml_function_return_ref(HccCU* cu, HccAMLFunction* function) {
	if (atomic_fetch_sub(&function->ref_count, 1) != 1 || !atomic_load(&function->can_free)) {
		return;
	}

	hcc_aml_function_alctor_dealloc(cu, function);
}

HccAMLOperand hcc_aml_function_value_add(HccAMLFunction* function, HccDataType data_type) {
	HCC_DEBUG_ASSERT_ARRAY_RESIZE(function->values_count + 1, function->values_cap);

	uint32_t value_idx = function->values_count;
	function->values_count += 1;

	HccAMLValue* value = &function->values[value_idx];
	value->data_type = data_type;

	return HCC_AML_OPERAND(VALUE, value_idx);
}

HccAMLOperand hcc_aml_function_basic_block_add(HccAMLFunction* function, uint32_t location_idx) {
	HCC_DEBUG_ASSERT_ARRAY_RESIZE(function->basic_blocks_count + 1, function->basic_blocks_cap);

	//
	// if the previous basic block does not branch or return, then lets make it explicit fallthrough to this basic block
	if (function->basic_blocks_count && function->basic_blocks[function->basic_blocks_count - 1].terminating_instr_word_idx == UINT32_MAX) {
		HccAMLOperand* operands = hcc_aml_function_instr_add(function, location_idx, HCC_AML_OP_BRANCH, 1);
		operands[0] = HCC_AML_OPERAND(BASIC_BLOCK, function->basic_blocks_count);
	}

	uint32_t word_idx = function->words_count;
	uint32_t basic_block_idx = function->basic_blocks_count;
	function->basic_blocks_count += 1;

	HccAMLBasicBlock* basic_block = &function->basic_blocks[basic_block_idx];
	basic_block->word_idx = word_idx;
	basic_block->terminating_instr_word_idx = UINT32_MAX;
	basic_block->params_start_idx = function->basic_block_params_count;
	basic_block->params_count = 0;

	HccAMLOperand* operands = hcc_aml_function_instr_add(function, location_idx, HCC_AML_OP_BASIC_BLOCK, 1);
	operands[0] = HCC_AML_OPERAND(BASIC_BLOCK, basic_block_idx);

	return HCC_AML_OPERAND(BASIC_BLOCK, basic_block_idx);
}

HccAMLOperand hcc_aml_function_basic_block_param_add(HccAMLFunction* function, HccDataType data_type) {
	HCC_DEBUG_ASSERT_ARRAY_RESIZE(function->basic_block_params_count + 1, function->basic_block_params_cap);

	uint32_t basic_block_param_idx = function->basic_block_params_count;
	function->basic_block_params_count += 1;

	HccAMLBasicBlockParam* basic_block_param = &function->basic_block_params[basic_block_param_idx];
	basic_block_param->data_type = data_type;
	basic_block_param->srcs_start_idx = function->basic_block_param_srcs_count;
	basic_block_param->srcs_count = 0;

	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(function->basic_blocks_count - 1, function->basic_blocks_count);
	function->basic_blocks[function->basic_blocks_count - 1].params_count += 1;

	return HCC_AML_OPERAND(BASIC_BLOCK_PARAM, basic_block_param_idx);
}

void hcc_aml_function_basic_block_param_src_add(HccAMLFunction* function, HccAMLOperand basic_block_operand, HccAMLOperand operand) {
	HCC_DEBUG_ASSERT_ARRAY_RESIZE(function->basic_block_param_srcs_count + 1, function->basic_block_param_srcs_cap);
	HCC_DEBUG_ASSERT(HCC_AML_OPERAND_IS_BASIC_BLOCK(basic_block_operand), "expected a basic block operand");

	uint32_t basic_block_param_src_idx = function->basic_block_param_srcs_count;
	function->basic_block_param_srcs_count += 1;

	HccAMLBasicBlockParamSrc* basic_block_param_src = &function->basic_block_param_srcs[basic_block_param_src_idx];
	basic_block_param_src->basic_block_operand = basic_block_operand;
	basic_block_param_src->operand = operand;

	HCC_DEBUG_ASSERT_ARRAY_BOUNDS(function->basic_block_params_count - 1, function->basic_block_params_count);
	function->basic_block_params[function->basic_block_params_count - 1].srcs_count += 1;
}

HccAMLOperand* hcc_aml_function_instr_add(HccAMLFunction* function, uint32_t location_idx, HccAMLOp op, uint16_t operands_count) {
	HCC_DEBUG_ASSERT_ARRAY_RESIZE(function->words_count + operands_count + 2, function->words_cap);
	uint32_t word_idx = function->words_count;

	if (op == HCC_AML_OP_BRANCH || op == HCC_AML_OP_BRANCH_CONDITIONAL || op == HCC_AML_OP_SWITCH || op == HCC_AML_OP_RETURN) {
		function->basic_blocks[function->basic_blocks_count - 1].terminating_instr_word_idx = word_idx;
	}

	function->words_count += operands_count + 2;

	HccAMLWord* words = &function->words[word_idx];
	words[0] = HCC_AML_INSTR(op, operands_count);
	words[1] = location_idx;

	return &words[2];
}

// ===========================================
//
//
// AML Function Allocator
//
//
// ===========================================

void hcc_aml_function_alctor_init(HccCU* cu, HccAMLFunctionAlctorSetup* setup) {
	uint32_t words_grow_count = (uint32_t)ceilf((float)setup->instrs_grow_count * HCC_AML_INSTR_AVERAGE_WORDS);
	uint32_t values_grow_count = (uint32_t)ceilf((float)setup->instrs_grow_count * HCC_AML_INSTR_AVERAGE_VALUES);
	uint32_t basic_blocks_grow_count = (uint32_t)ceilf((float)setup->instrs_grow_count * HCC_AML_INSTR_AVERAGE_BASIC_BLOCKS);
	uint32_t basic_block_params_grow_count = (uint32_t)ceilf((float)setup->instrs_grow_count * HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAMS);
	uint32_t basic_block_param_srcs_grow_count = (uint32_t)ceilf((float)setup->instrs_grow_count * HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAM_SRCS);
	uint32_t words_reserve_cap = (uint32_t)ceilf((float)setup->instrs_reserve_cap * HCC_AML_INSTR_AVERAGE_WORDS);
	uint32_t values_reserve_cap = (uint32_t)ceilf((float)setup->instrs_reserve_cap * HCC_AML_INSTR_AVERAGE_VALUES);
	uint32_t basic_blocks_reserve_cap = (uint32_t)ceilf((float)setup->instrs_reserve_cap * HCC_AML_INSTR_AVERAGE_BASIC_BLOCKS);
	uint32_t basic_block_params_reserve_cap = (uint32_t)ceilf((float)setup->instrs_reserve_cap * HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAMS);
	uint32_t basic_block_param_srcs_reserve_cap = (uint32_t)ceilf((float)setup->instrs_reserve_cap * HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAM_SRCS);

	HccAMLFunctionAlctor* function_alctor = &cu->aml.function_alctor;
	function_alctor->functions_pool = hcc_stack_init(HccAMLFunction, HCC_ALLOC_TAG_AML_FUNCTION_ALCTOR_NODES_POOL, setup->functions_grow_count, setup->functions_reserve_cap);
	function_alctor->words_pool = hcc_stack_init(HccAMLWord, HCC_ALLOC_TAG_AML_FUNCTION_ALCTOR_WORDS_POOL, words_grow_count, words_reserve_cap);
	function_alctor->values_pool = hcc_stack_init(HccAMLValue, HCC_ALLOC_TAG_AML_FUNCTION_ALCTOR_VALUES_POOL, values_grow_count, values_reserve_cap);
	function_alctor->basic_blocks_pool = hcc_stack_init(HccAMLBasicBlock, HCC_ALLOC_TAG_AML_FUNCTION_ALCTOR_BASIC_BLOCKS_POOL, basic_blocks_grow_count, basic_blocks_reserve_cap);
	function_alctor->basic_block_params_pool = hcc_stack_init(HccAMLBasicBlockParam, HCC_ALLOC_TAG_AML_FUNCTION_ALCTOR_BASIC_BLOCK_PARAMS_POOL, basic_block_params_grow_count, basic_block_params_reserve_cap);
	function_alctor->basic_block_param_srcs_pool = hcc_stack_init(HccAMLBasicBlockParamSrc, HCC_ALLOC_TAG_AML_FUNCTION_ALCTOR_BASIC_BLOCK_PARAM_SRCS_POOL, basic_block_param_srcs_grow_count, basic_block_param_srcs_reserve_cap);
}

void hcc_aml_function_alctor_deinit(HccCU* cu) {
	HccAMLFunctionAlctor* function_alctor = &cu->aml.function_alctor;
	hcc_stack_deinit(function_alctor->functions_pool);
	hcc_stack_deinit(function_alctor->words_pool);
	hcc_stack_deinit(function_alctor->values_pool);
	hcc_stack_deinit(function_alctor->basic_blocks_pool);
	hcc_stack_deinit(function_alctor->basic_block_params_pool);
	hcc_stack_deinit(function_alctor->basic_block_param_srcs_pool);
}

uint32_t hcc_aml_function_alctor_instr_count_round_up_log2(HccCU* cu, uint32_t max_instrs_count) {
	HCC_UNUSED(cu);

	//
	// calculate the next power of two (in log2) that can fit this instructions count
	uint32_t instr_count_log2 = hcc_mostsetbitidx32(max_instrs_count - 1) + 1;

	//
	// round up to the minimum log2
	instr_count_log2 = HCC_MAX(instr_count_log2, HCC_AML_FUNCTION_ALLOCATOR_INTSR_MIN_LOG2);
	return instr_count_log2;
}

HccAMLFunction* hcc_aml_function_alctor_alloc(HccCU* cu, uint32_t max_instrs_count) {
	uint32_t instr_count_log2 = hcc_aml_function_alctor_instr_count_round_up_log2(cu, max_instrs_count);
	HCC_ASSERT(
		instr_count_log2 < HCC_AML_FUNCTION_ALLOCATOR_INTSR_MAX_LOG2,
		"reached a the maximum number of rounded up instructions for a function. max_instrs_count: %u",
		max_instrs_count
	);

	//
	// get the linked list head function pointer for this instruction count in log2
	HccAMLFunctionAlctor* function_alctor = &cu->aml.function_alctor;
	HccAtomic(HccAMLFunction*)* next_free_ptr = &function_alctor->free_functions_by_instr_log2[instr_count_log2 - HCC_AML_FUNCTION_ALLOCATOR_INTSR_MIN_LOG2];

	//
	// try to steal the next free function from the head of the linked list
	HccAMLFunction* next_free_function = atomic_load(next_free_ptr);
	while (next_free_function) {
		while (next_free_function == HCC_AML_FUNCTION_SENTINAL) {
			HCC_CPU_RELAX();
			next_free_function = atomic_load(next_free_ptr);
		}

		if (atomic_compare_exchange_weak(next_free_ptr, &next_free_function, HCC_AML_FUNCTION_SENTINAL)) {
			atomic_store(next_free_ptr, next_free_function->next_free);
			next_free_function->identifier_location = NULL;
			next_free_function->identifier_string_id.idx_plus_one = 0;
			next_free_function->params_count = 0;
			next_free_function->words_count = 0;
			next_free_function->values_count = 0;
			next_free_function->basic_blocks_count = 0;
			next_free_function->basic_block_params_count = 0;
			next_free_function->ref_count = 1;
			next_free_function->can_free = false;
			next_free_function->next_free = NULL;
			return next_free_function;
		}
	}

	//
	// we have failed to get any recycled memory,
	// lets allocate from the pools directly and create our own new function.
	//

	uint32_t instr_count = 1 << instr_count_log2;
	uint32_t words_cap = (uint32_t)ceilf((float)instr_count * HCC_AML_INSTR_AVERAGE_WORDS);
	uint32_t values_cap = (uint32_t)ceilf((float)instr_count * HCC_AML_INSTR_AVERAGE_VALUES);
	uint32_t basic_blocks_cap = (uint32_t)ceilf((float)instr_count * HCC_AML_INSTR_AVERAGE_BASIC_BLOCKS);
	uint32_t basic_block_params_cap = (uint32_t)ceilf((float)instr_count * HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAMS);
	uint32_t basic_block_param_srcs_cap = (uint32_t)ceilf((float)instr_count * HCC_AML_INSTR_AVERAGE_BASIC_BLOCK_PARAM_SRCS);
	HccAMLWord* words = hcc_stack_push_many_thread_safe(function_alctor->words_pool, words_cap);
	HccAMLValue* values = hcc_stack_push_many_thread_safe(function_alctor->values_pool, values_cap);
	HccAMLBasicBlock* basic_blocks = hcc_stack_push_many_thread_safe(function_alctor->basic_blocks_pool, basic_blocks_cap);
	HccAMLBasicBlockParam* basic_block_params = hcc_stack_push_many_thread_safe(function_alctor->basic_block_params_pool, basic_block_params_cap);
	HccAMLBasicBlockParamSrc* basic_block_param_srcs = hcc_stack_push_many_thread_safe(function_alctor->basic_block_param_srcs_pool, basic_block_param_srcs_cap);

	HccAMLFunction* function = hcc_stack_push(function_alctor->functions_pool);
	function->identifier_location = NULL;
	function->identifier_string_id.idx_plus_one = 0;
	function->params_count = 0;
	function->can_free = false;
	function->ref_count = 1;
	function->next_free = NULL;
	function->words = words;
	function->values = values;
	function->basic_blocks = basic_blocks;
	function->basic_block_params = basic_block_params;
	function->basic_block_param_srcs = basic_block_param_srcs;
	function->words_count = 0;
	function->words_cap = words_cap;
	function->values_count = 0;
	function->values_cap = values_cap;
	function->basic_blocks_count = 0;
	function->basic_blocks_cap = basic_blocks_cap;
	function->basic_block_params_count = 0;
	function->basic_block_params_cap = basic_block_params_cap;
	function->basic_block_param_srcs_count = 0;
	function->basic_block_param_srcs_cap = basic_block_param_srcs_cap;
	function->found_texture_sample_location = NULL;

	return function;
}

void hcc_aml_function_alctor_dealloc(HccCU* cu, HccAMLFunction* function) {
	uint32_t instr_count_log2 = hcc_aml_function_alctor_instr_count_round_up_log2(cu, (uint32_t)ceilf((float)function->words_cap / HCC_AML_INSTR_AVERAGE_WORDS));
	HCC_ASSERT(instr_count_log2 < HCC_AML_FUNCTION_ALLOCATOR_INTSR_MAX_LOG2, "how could the deallocated function be larger than the maximum allocation?");

	//
	// get the linked list head function pointer for this instruction count in log2
	HccAMLFunctionAlctor* function_alctor = &cu->aml.function_alctor;
	HccAtomic(HccAMLFunction*)* next_free_ptr = &function_alctor->free_functions_by_instr_log2[instr_count_log2 - HCC_AML_FUNCTION_ALLOCATOR_INTSR_MIN_LOG2];

	//
	// make our function the free list head
	HccAMLFunction* next_free_function = atomic_load(next_free_ptr);
	while (1) {
		function->next_free = next_free_function;
		if (atomic_compare_exchange_weak(next_free_ptr, &next_free_function, function)) {
			return;
		}
	}
}

// ===========================================
//
//
// AML Operation
//
//
// ===========================================

const char* hcc_aml_op_code_strings[HCC_AML_OP_COUNT] = {
	[HCC_AML_OP_NO_OP] = "NO_OP",
	[HCC_AML_OP_PTR_STATIC_ALLOC] = "PTR_STATIC_ALLOC",
	[HCC_AML_OP_PTR_LOAD] = "PTR_LOAD",
	[HCC_AML_OP_PTR_STORE] = "PTR_STORE",
	[HCC_AML_OP_PTR_ACCESS_CHAIN] = "PTR_ACCESS_CHAIN",
	[HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS] = "PTR_ACCESS_CHAIN_IN_BOUNDS",
	[HCC_AML_OP_COMPOSITE_INIT] = "COMPOSITE_INIT",
	[HCC_AML_OP_COMPOSITE_ACCESS_CHAIN_GET] = "COMPOSITE_ACCESS_CHAIN_GET",
	[HCC_AML_OP_COMPOSITE_ACCESS_CHAIN_SET] = "COMPOSITE_ACCESS_CHAIN_SET",
	[HCC_AML_OP_BASIC_BLOCK] = "BASIC_BLOCK",
	[HCC_AML_OP_BRANCH] = "BRANCH",
	[HCC_AML_OP_BRANCH_CONDITIONAL] = "BRANCH_CONDITIONAL",
	[HCC_AML_OP_SWITCH] = "SWITCH",
	[HCC_AML_OP_ADD] = "ADD",
	[HCC_AML_OP_SUBTRACT] = "SUBTRACT",
	[HCC_AML_OP_MULTIPLY] = "MULTIPLY",
	[HCC_AML_OP_DIVIDE] = "DIVIDE",
	[HCC_AML_OP_MODULO] = "MODULO",
	[HCC_AML_OP_BIT_AND] = "BIT_AND",
	[HCC_AML_OP_BIT_OR] = "BIT_OR",
	[HCC_AML_OP_BIT_XOR] = "BIT_XOR",
	[HCC_AML_OP_BIT_SHIFT_LEFT] = "BIT_SHIFT_LEFT",
	[HCC_AML_OP_BIT_SHIFT_RIGHT] = "BIT_SHIFT_RIGHT",
	[HCC_AML_OP_EQUAL] = "EQUAL",
	[HCC_AML_OP_NOT_EQUAL] = "NOT_EQUAL",
	[HCC_AML_OP_LESS_THAN] = "LESS_THAN",
	[HCC_AML_OP_LESS_THAN_OR_EQUAL] = "LESS_THAN_OR_EQUAL",
	[HCC_AML_OP_GREATER_THAN] = "GREATER_THAN",
	[HCC_AML_OP_GREATER_THAN_OR_EQUAL] = "GREATER_THAN_OR_EQUAL",
	[HCC_AML_OP_NEGATE] = "NEGATE",
	[HCC_AML_OP_CONVERT] = "CONVERT",
	[HCC_AML_OP_BITCAST] = "BITCAST",
	[HCC_AML_OP_CALL] = "CALL",
	[HCC_AML_OP_RETURN] = "RETURN",
	[HCC_AML_OP_UNREACHABLE] = "UNREACHABLE",
	[HCC_AML_OP_SELECT] = "SELECT",
	[HCC_AML_OP_SELECTION_MERGE] = "SELECTION_MERGE",
	[HCC_AML_OP_LOOP_MERGE] = "LOOP_MERGE",
	[HCC_AML_OP_RESOURCE_DESCRIPTOR_LOAD] = "RESOURCE_DESCRIPTOR_LOAD",
	[HCC_AML_OP_MIN] = "MIN",
	[HCC_AML_OP_MAX] = "MAX",
	[HCC_AML_OP_CLAMP] = "CLAMP",
	[HCC_AML_OP_SIGN] = "SIGN",
	[HCC_AML_OP_ABS] = "ABS",
	[HCC_AML_OP_FMA] = "FMA",
	[HCC_AML_OP_FLOOR] = "FLOOR",
	[HCC_AML_OP_CEIL] = "CEIL",
	[HCC_AML_OP_ROUND] = "ROUND",
	[HCC_AML_OP_TRUNC] = "TRUNC",
	[HCC_AML_OP_FRACT] = "FRACT",
	[HCC_AML_OP_RADIANS] = "RADIANS",
	[HCC_AML_OP_DEGREES] = "DEGREES",
	[HCC_AML_OP_STEP] = "STEP",
	[HCC_AML_OP_SMOOTHSTEP] = "SMOOTHSTEP",
	[HCC_AML_OP_SIN] = "SIN",
	[HCC_AML_OP_COS] = "COS",
	[HCC_AML_OP_TAN] = "TAN",
	[HCC_AML_OP_ASIN] = "ASIN",
	[HCC_AML_OP_ACOS] = "ACOS",
	[HCC_AML_OP_ATAN] = "ATAN",
	[HCC_AML_OP_SINH] = "SINH",
	[HCC_AML_OP_COSH] = "COSH",
	[HCC_AML_OP_TANH] = "TANH",
	[HCC_AML_OP_ASINH] = "ASINH",
	[HCC_AML_OP_ACOSH] = "ACOSH",
	[HCC_AML_OP_ATANH] = "ATANH",
	[HCC_AML_OP_ATAN2] = "ATAN2",
	[HCC_AML_OP_POW] = "POW",
	[HCC_AML_OP_EXP] = "EXP",
	[HCC_AML_OP_LOG] = "LOG",
	[HCC_AML_OP_EXP2] = "EXP2",
	[HCC_AML_OP_LOG2] = "LOG2",
	[HCC_AML_OP_SQRT] = "SQRT",
	[HCC_AML_OP_RSQRT] = "RSQRT",
	[HCC_AML_OP_ISINF] = "ISINF",
	[HCC_AML_OP_ISNAN] = "ISNAN",
	[HCC_AML_OP_LERP] = "LERP",
	[HCC_AML_OP_ANY] = "ANY",
	[HCC_AML_OP_ALL] = "ALL",
	[HCC_AML_OP_DOT] = "DOT",
	[HCC_AML_OP_LEN] = "LEN",
	[HCC_AML_OP_NORM] = "NORM",
	[HCC_AML_OP_REFLECT] = "REFLECT",
	[HCC_AML_OP_REFRACT] = "REFRACT",
	[HCC_AML_OP_PACK_F16X2_F32X2] = "PACK_F16X2_F32X2",
	[HCC_AML_OP_UNPACK_F16X2_F32X2] = "UNPACK_F16X2_F32X2",
	[HCC_AML_OP_PACK_U16X2_F32X2] = "PACK_U16X2_F32X2",
	[HCC_AML_OP_UNPACK_U16X2_F32X2] = "UNPACK_U16X2_F32X2",
	[HCC_AML_OP_PACK_S16X2_F32X2] = "PACK_S16X2_F32X2",
	[HCC_AML_OP_UNPACK_S16X2_F32X2] = "UNPACK_S16X2_F32X2",
	[HCC_AML_OP_PACK_U8X4_F32X4] = "PACK_U8X4_F32X4",
	[HCC_AML_OP_UNPACK_U8X4_F32X4] = "UNPACK_U8X4_F32X4",
	[HCC_AML_OP_PACK_S8X4_F32X4] = "PACK_S8X4_F32X4",
	[HCC_AML_OP_UNPACK_S8X4_F32X4] = "UNPACK_S8X4_F32X4",
	[HCC_AML_OP_DISCARD_FRAGMENT] = "DISCARD_FRAGMENT",
	[HCC_AML_OP_DDX] = "DDX",
	[HCC_AML_OP_DDY] = "DDY",
	[HCC_AML_OP_FWIDTH] = "FWIDTH",
	[HCC_AML_OP_DDX_FINE] = "DDX_FINE",
	[HCC_AML_OP_DDY_FINE] = "DDY_FINE",
	[HCC_AML_OP_FWIDTH_FINE] = "FWIDTH_FINE",
	[HCC_AML_OP_DDX_COARSE] = "DDX_COARSE",
	[HCC_AML_OP_DDY_COARSE] = "DDY_COARSE",
	[HCC_AML_OP_FWIDTH_COARSE] = "FWIDTH_COARSE",
	[HCC_AML_OP_QUAD_SWAP_X] = "QUAD_SWAP_X",
	[HCC_AML_OP_QUAD_SWAP_Y] = "QUAD_SWAP_Y",
	[HCC_AML_OP_QUAD_SWAP_DIAGONAL] = "QUAD_SWAP_DIAGONAL",
	[HCC_AML_OP_QUAD_READ_THREAD] = "QUAD_READ_THREAD",
	[HCC_AML_OP_QUAD_ANY] = "QUAD_ANY",
	[HCC_AML_OP_QUAD_ALL] = "QUAD_ALL",
	[HCC_AML_OP_WAVE_ACTIVE_ANY] = "WAVE_ACTIVE_ANY",
	[HCC_AML_OP_WAVE_ACTIVE_ALL] = "WAVE_ACTIVE_ALL",
	[HCC_AML_OP_WAVE_READ_THREAD] = "WAVE_READ_THREAD",
	[HCC_AML_OP_WAVE_ACTIVE_ALL_EQUAL] = "WAVE_ACTIVE_ALL_EQUAL",
	[HCC_AML_OP_WAVE_ACTIVE_MIN] = "WAVE_ACTIVE_MIN",
	[HCC_AML_OP_WAVE_ACTIVE_MAX] = "WAVE_ACTIVE_MAX",
	[HCC_AML_OP_WAVE_ACTIVE_SUM] = "WAVE_ACTIVE_SUM",
	[HCC_AML_OP_WAVE_ACTIVE_PREFIX_SUM] = "WAVE_ACTIVE_PREFIX_SUM",
	[HCC_AML_OP_WAVE_ACTIVE_PRODUCT] = "WAVE_ACTIVE_PRODUCT",
	[HCC_AML_OP_WAVE_ACTIVE_PREFIX_PRODUCT] = "WAVE_ACTIVE_PREFIX_PRODUCT",
	[HCC_AML_OP_WAVE_ACTIVE_COUNT_BITS] = "WAVE_ACTIVE_COUNT_BITS",
	[HCC_AML_OP_WAVE_ACTIVE_PREFIX_COUNT_BITS] = "WAVE_ACTIVE_PREFIX_COUNT_BITS",
	[HCC_AML_OP_WAVE_ACTIVE_BIT_AND] = "WAVE_ACTIVE_BIT_AND",
	[HCC_AML_OP_WAVE_ACTIVE_BIT_OR] = "WAVE_ACTIVE_BIT_OR",
	[HCC_AML_OP_WAVE_ACTIVE_BIT_XOR] = "WAVE_ACTIVE_BIT_XOR",
	[HCC_AML_OP_MEMORY_BARRIER_RESOURCE] = "MEMORY_BARRIER_RESOURCE",
	[HCC_AML_OP_MEMORY_BARRIER_DISPATCH] = "MEMORY_BARRIER_DISPATCH",
	[HCC_AML_OP_MEMORY_BARRIER_ALL] = "MEMORY_BARRIER_ALL",
	[HCC_AML_OP_CONTROL_BARRIER_RESOURCE] = "CONTROL_BARRIER_RESOURCE",
	[HCC_AML_OP_CONTROL_BARRIER_DISPATCH] = "CONTROL_BARRIER_DISPATCH",
	[HCC_AML_OP_CONTROL_BARRIER_ALL] = "CONTROL_BARRIER_ALL",
	[HCC_AML_OP_ATOMIC_LOAD] = "ATOMIC_LOAD",
	[HCC_AML_OP_ATOMIC_STORE] = "ATOMIC_STORE",
	[HCC_AML_OP_ATOMIC_EXCHANGE] = "ATOMIC_EXCHANGE",
	[HCC_AML_OP_ATOMIC_COMPARE_EXCHANGE] = "ATOMIC_COMPARE_EXCHANGE",
	[HCC_AML_OP_ATOMIC_ADD] = "ATOMIC_ADD",
	[HCC_AML_OP_ATOMIC_SUB] = "ATOMIC_SUB",
	[HCC_AML_OP_ATOMIC_MIN] = "ATOMIC_MIN",
	[HCC_AML_OP_ATOMIC_MAX] = "ATOMIC_MAX",
	[HCC_AML_OP_ATOMIC_BIT_AND] = "ATOMIC_BIT_AND",
	[HCC_AML_OP_ATOMIC_BIT_OR] = "ATOMIC_BIT_OR",
	[HCC_AML_OP_ATOMIC_BIT_XOR] = "ATOMIC_BIT_XOR",
	[HCC_AML_OP_LOAD_TEXTURE] = "LOAD_TEXTURE",
	[HCC_AML_OP_FETCH_TEXTURE] = "FETCH_TEXTURE",
	[HCC_AML_OP_SAMPLE_TEXTURE] = "SAMPLE_TEXTURE",
	[HCC_AML_OP_SAMPLE_MIP_BIAS_TEXTURE] = "SAMPLE_MIP_BIAS_TEXTURE",
	[HCC_AML_OP_SAMPLE_MIP_GRADIENT_TEXTURE] = "SAMPLE_MIP_GRADIENT_TEXTURE",
	[HCC_AML_OP_SAMPLE_MIP_LEVEL_TEXTURE] = "SAMPLE_MIP_LEVEL_TEXTURE",
	[HCC_AML_OP_GATHER_RED_TEXTURE] = "GATHER_RED_TEXTURE",
	[HCC_AML_OP_GATHER_GREEN_TEXTURE] = "GATHER_GREEN_TEXTURE",
	[HCC_AML_OP_GATHER_BLUE_TEXTURE] = "GATHER_BLUE_TEXTURE",
	[HCC_AML_OP_GATHER_ALPHA_TEXTURE] = "GATHER_ALPHA_TEXTURE",
	[HCC_AML_OP_STORE_TEXTURE] = "STORE_TEXTURE",
};

bool hcc_aml_op_code_has_return_value[HCC_AML_OP_COUNT] = {
	[HCC_AML_OP_NO_OP] = false,
	[HCC_AML_OP_PTR_STATIC_ALLOC] = true,
	[HCC_AML_OP_PTR_LOAD] = true,
	[HCC_AML_OP_PTR_STORE] = false,
	[HCC_AML_OP_PTR_ACCESS_CHAIN] = true,
	[HCC_AML_OP_PTR_ACCESS_CHAIN_IN_BOUNDS] = true,
	[HCC_AML_OP_COMPOSITE_INIT] = true,
	[HCC_AML_OP_COMPOSITE_ACCESS_CHAIN_GET] = true,
	[HCC_AML_OP_COMPOSITE_ACCESS_CHAIN_SET] = false,
	[HCC_AML_OP_BASIC_BLOCK] = true,
	[HCC_AML_OP_BRANCH] = false,
	[HCC_AML_OP_BRANCH_CONDITIONAL] = false,
	[HCC_AML_OP_SWITCH] = false,
	[HCC_AML_OP_ADD] = true,
	[HCC_AML_OP_SUBTRACT] = true,
	[HCC_AML_OP_MULTIPLY] = true,
	[HCC_AML_OP_DIVIDE] = true,
	[HCC_AML_OP_MODULO] = true,
	[HCC_AML_OP_BIT_AND] = true,
	[HCC_AML_OP_BIT_OR] = true,
	[HCC_AML_OP_BIT_XOR] = true,
	[HCC_AML_OP_BIT_SHIFT_LEFT] = true,
	[HCC_AML_OP_BIT_SHIFT_RIGHT] = true,
	[HCC_AML_OP_EQUAL] = true,
	[HCC_AML_OP_NOT_EQUAL] = true,
	[HCC_AML_OP_LESS_THAN] = true,
	[HCC_AML_OP_LESS_THAN_OR_EQUAL] = true,
	[HCC_AML_OP_GREATER_THAN] = true,
	[HCC_AML_OP_GREATER_THAN_OR_EQUAL] = true,
	[HCC_AML_OP_NEGATE] = true,
	[HCC_AML_OP_CONVERT] = true,
	[HCC_AML_OP_BITCAST] = true,
	[HCC_AML_OP_CALL] = true,
	[HCC_AML_OP_RETURN] = false,
	[HCC_AML_OP_UNREACHABLE] = false,
	[HCC_AML_OP_SELECT] = true,
	[HCC_AML_OP_SELECTION_MERGE] = false,
	[HCC_AML_OP_LOOP_MERGE] = false,
	[HCC_AML_OP_RESOURCE_DESCRIPTOR_LOAD] = true,
	[HCC_AML_OP_MIN] = true,
	[HCC_AML_OP_MAX] = true,
	[HCC_AML_OP_CLAMP] = true,
	[HCC_AML_OP_SIGN] = true,
	[HCC_AML_OP_ABS] = true,
	[HCC_AML_OP_FMA] = true,
	[HCC_AML_OP_FLOOR] = true,
	[HCC_AML_OP_CEIL] = true,
	[HCC_AML_OP_ROUND] = true,
	[HCC_AML_OP_TRUNC] = true,
	[HCC_AML_OP_FRACT] = true,
	[HCC_AML_OP_RADIANS] = true,
	[HCC_AML_OP_DEGREES] = true,
	[HCC_AML_OP_STEP] = true,
	[HCC_AML_OP_SMOOTHSTEP] = true,
	[HCC_AML_OP_SIN] = true,
	[HCC_AML_OP_COS] = true,
	[HCC_AML_OP_TAN] = true,
	[HCC_AML_OP_ASIN] = true,
	[HCC_AML_OP_ACOS] = true,
	[HCC_AML_OP_ATAN] = true,
	[HCC_AML_OP_SINH] = true,
	[HCC_AML_OP_COSH] = true,
	[HCC_AML_OP_TANH] = true,
	[HCC_AML_OP_ASINH] = true,
	[HCC_AML_OP_ACOSH] = true,
	[HCC_AML_OP_ATANH] = true,
	[HCC_AML_OP_ATAN2] = true,
	[HCC_AML_OP_POW] = true,
	[HCC_AML_OP_EXP] = true,
	[HCC_AML_OP_LOG] = true,
	[HCC_AML_OP_EXP2] = true,
	[HCC_AML_OP_LOG2] = true,
	[HCC_AML_OP_SQRT] = true,
	[HCC_AML_OP_RSQRT] = true,
	[HCC_AML_OP_ISINF] = true,
	[HCC_AML_OP_ISNAN] = true,
	[HCC_AML_OP_LERP] = true,
	[HCC_AML_OP_ANY] = true,
	[HCC_AML_OP_ALL] = true,
	[HCC_AML_OP_DOT] = true,
	[HCC_AML_OP_LEN] = true,
	[HCC_AML_OP_NORM] = true,
	[HCC_AML_OP_REFLECT] = true,
	[HCC_AML_OP_REFRACT] = true,
	[HCC_AML_OP_PACK_F16X2_F32X2] = true,
	[HCC_AML_OP_UNPACK_F16X2_F32X2] = true,
	[HCC_AML_OP_PACK_U16X2_F32X2] = true,
	[HCC_AML_OP_UNPACK_U16X2_F32X2] = true,
	[HCC_AML_OP_PACK_S16X2_F32X2] = true,
	[HCC_AML_OP_UNPACK_S16X2_F32X2] = true,
	[HCC_AML_OP_PACK_U8X4_F32X4] = true,
	[HCC_AML_OP_UNPACK_U8X4_F32X4] = true,
	[HCC_AML_OP_PACK_S8X4_F32X4] = true,
	[HCC_AML_OP_UNPACK_S8X4_F32X4] = true,
	[HCC_AML_OP_DISCARD_FRAGMENT] = false,
	[HCC_AML_OP_DDX] = true,
	[HCC_AML_OP_DDY] = true,
	[HCC_AML_OP_FWIDTH] = true,
	[HCC_AML_OP_DDX_FINE] = true,
	[HCC_AML_OP_DDY_FINE] = true,
	[HCC_AML_OP_FWIDTH_FINE] = true,
	[HCC_AML_OP_DDX_COARSE] = true,
	[HCC_AML_OP_DDY_COARSE] = true,
	[HCC_AML_OP_FWIDTH_COARSE] = true,
	[HCC_AML_OP_QUAD_SWAP_X] = true,
	[HCC_AML_OP_QUAD_SWAP_Y] = true,
	[HCC_AML_OP_QUAD_SWAP_DIAGONAL] = true,
	[HCC_AML_OP_QUAD_READ_THREAD] = true,
	[HCC_AML_OP_QUAD_ANY] = true,
	[HCC_AML_OP_QUAD_ALL] = true,
	[HCC_AML_OP_WAVE_ACTIVE_ANY] = true,
	[HCC_AML_OP_WAVE_ACTIVE_ALL] = true,
	[HCC_AML_OP_WAVE_READ_THREAD] = true,
	[HCC_AML_OP_WAVE_ACTIVE_ALL_EQUAL] = true,
	[HCC_AML_OP_WAVE_ACTIVE_MIN] = true,
	[HCC_AML_OP_WAVE_ACTIVE_MAX] = true,
	[HCC_AML_OP_WAVE_ACTIVE_SUM] = true,
	[HCC_AML_OP_WAVE_ACTIVE_PREFIX_SUM] = true,
	[HCC_AML_OP_WAVE_ACTIVE_PRODUCT] = true,
	[HCC_AML_OP_WAVE_ACTIVE_PREFIX_PRODUCT] = true,
	[HCC_AML_OP_WAVE_ACTIVE_COUNT_BITS] = true,
	[HCC_AML_OP_WAVE_ACTIVE_PREFIX_COUNT_BITS] = true,
	[HCC_AML_OP_WAVE_ACTIVE_BIT_AND] = true,
	[HCC_AML_OP_WAVE_ACTIVE_BIT_OR] = true,
	[HCC_AML_OP_WAVE_ACTIVE_BIT_XOR] = true,
	[HCC_AML_OP_MEMORY_BARRIER_RESOURCE] = false,
	[HCC_AML_OP_MEMORY_BARRIER_DISPATCH] = false,
	[HCC_AML_OP_MEMORY_BARRIER_ALL] = false,
	[HCC_AML_OP_CONTROL_BARRIER_RESOURCE] = false,
	[HCC_AML_OP_CONTROL_BARRIER_DISPATCH] = false,
	[HCC_AML_OP_CONTROL_BARRIER_ALL] = false,
	[HCC_AML_OP_ATOMIC_LOAD] = true,
	[HCC_AML_OP_ATOMIC_STORE] = false,
	[HCC_AML_OP_ATOMIC_EXCHANGE] = true,
	[HCC_AML_OP_ATOMIC_COMPARE_EXCHANGE] = true,
	[HCC_AML_OP_ATOMIC_ADD] = true,
	[HCC_AML_OP_ATOMIC_SUB] = true,
	[HCC_AML_OP_ATOMIC_MIN] = true,
	[HCC_AML_OP_ATOMIC_MAX] = true,
	[HCC_AML_OP_ATOMIC_BIT_AND] = true,
	[HCC_AML_OP_ATOMIC_BIT_OR] = true,
	[HCC_AML_OP_ATOMIC_BIT_XOR] = true,
	[HCC_AML_OP_LOAD_TEXTURE] = true,
	[HCC_AML_OP_SAMPLE_TEXTURE] = true,
	[HCC_AML_OP_SAMPLE_MIP_BIAS_TEXTURE] = true,
	[HCC_AML_OP_SAMPLE_MIP_GRADIENT_TEXTURE] = true,
	[HCC_AML_OP_SAMPLE_MIP_LEVEL_TEXTURE] = true,
	[HCC_AML_OP_GATHER_RED_TEXTURE] = true,
	[HCC_AML_OP_GATHER_GREEN_TEXTURE] = true,
	[HCC_AML_OP_GATHER_BLUE_TEXTURE] = true,
	[HCC_AML_OP_GATHER_ALPHA_TEXTURE] = true,
	[HCC_AML_OP_STORE_TEXTURE] = false,
};

// ===========================================
//
//
// AML Operand
//
//
// ===========================================

HccDataType hcc_aml_operand_data_type(HccCU* cu, const HccAMLFunction* function, HccAMLOperand operand) {
	switch (HCC_AML_OPERAND_TYPE(operand)) {
		case HCC_AML_OPERAND_VALUE: {
			HCC_DEBUG_ASSERT_ARRAY_BOUNDS(HCC_AML_OPERAND_AUX(operand), function->values_count);
			HccAMLValue* value = &function->values[HCC_AML_OPERAND_AUX(operand)];
			return value->data_type;
		};
		case HCC_AML_OPERAND_CONSTANT: {
			HccConstant constant = hcc_constant_table_get(cu, HccConstantId(HCC_AML_OPERAND_AUX(operand)));
			return constant.data_type;
		};
		case HCC_AML_OPERAND_BASIC_BLOCK: {
			return 0;
		};
		case HCC_AML_OPERAND_BASIC_BLOCK_PARAM: {
			HCC_DEBUG_ASSERT_ARRAY_BOUNDS(HCC_AML_OPERAND_AUX(operand), function->basic_block_params_count);
			HccAMLBasicBlockParam* basic_block_param = &function->basic_block_params[HCC_AML_OPERAND_AUX(operand)];
			return basic_block_param->data_type;
		};
		case HCC_DECL_GLOBAL_VARIABLE: {
			HccASTVariable* variable = hcc_ast_global_variable_get(cu, (HccDecl)operand);
			return variable->data_type;
		};
		case HCC_DECL_FUNCTION:
			return hcc_decl_function_data_type(cu, (HccDecl)operand);

		case HCC_DECL_ENUM_VALUE: {
			return hcc_data_type_lower_ast_to_aml(cu, HCC_DATA_TYPE_AST_BASIC_SINT);
		};
		case HCC_DECL_LOCAL_VARIABLE: {
			HCC_ABORT("we shouldn't have access to local variables from the AST in the AML");
		};
		default:
			return (HccDataType)operand;
	}
}

// ===========================================
//
//
// AML - Abstract Machine Language
//
//
// ===========================================

const HccAMLFunction* hcc_aml_function_get(HccCU* cu, HccDecl decl) {
	HCC_DEBUG_ASSERT(HCC_DECL_IS_FUNCTION(decl), "internal error: expected a function declaration");
	HCC_DEBUG_ASSERT(!HCC_DECL_IS_FORWARD_DECL(decl), "internal error: expected a function declaration that is not a forward declaration");
	return *hcc_stack_get(cu->aml.functions, HCC_DECL_AUX(decl));
}

uint32_t hcc_aml_function_words_count(HccAMLFunction* function) {
	return function->words_count;
}

HccAMLWord* hcc_aml_function_words(HccAMLFunction* function) {
	return function->words;
}

uint32_t hcc_aml_function_values_count(HccAMLFunction* function) {
	return function->values_count;
}

HccAMLValue* hcc_aml_function_values(HccAMLFunction* function) {
	return function->values;
}

uint32_t hcc_aml_function_basic_blocks_count(HccAMLFunction* function) {
	return function->basic_blocks_count;
}

HccAMLBasicBlock* hcc_aml_function_basic_blocks(HccAMLFunction* function) {
	return function->basic_blocks;
}

uint32_t hcc_aml_function_basic_block_params_count(HccAMLFunction* function) {
	return function->basic_block_params_count;
}

HccAMLBasicBlockParam* hcc_aml_function_basic_block_params(HccAMLFunction* function) {
	return function->basic_block_params;
}

uint32_t hcc_aml_function_basic_block_param_srcs_count(HccAMLFunction* function) {
	return function->basic_block_param_srcs_count;
}

HccAMLBasicBlockParamSrc* hcc_aml_function_basic_block_param_srcs(HccAMLFunction* function) {
	return function->basic_block_param_srcs;
}

void hcc_aml_init(HccCU* cu, HccCUSetup* setup) {
	hcc_aml_function_alctor_init(cu, &setup->aml.function_alctor);
	cu->aml.functions = hcc_stack_init(HccAtomic(HccAMLFunction*), HCC_ALLOC_TAG_AML_FUNCTIONS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->aml.locations = hcc_stack_init(HccLocation*, HCC_ALLOC_TAG_AML_LOCATIONS, setup->ast.expr_locations_grow_count, setup->ast.expr_locations_reserve_cap);
	cu->aml.call_graph_nodes = hcc_stack_init(HccAMLCallNode, HCC_ALLOC_TAG_AML_CALL_GRAPH_NODES, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->aml.function_call_node_lists = hcc_stack_init(HccAMLCallNode*, 	HCC_ALLOC_TAG_AML_FUNCTION_CALL_NODE_LISTS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->aml.optimize_functions[0] = hcc_stack_init(HccDecl, HCC_ALLOC_TAG_AML_OPIMIZE_FUNCTIONS, setup->functions_grow_count, setup->functions_reserve_cap);
	cu->aml.optimize_functions[1] = hcc_stack_init(HccDecl, HCC_ALLOC_TAG_AML_OPIMIZE_FUNCTIONS, setup->functions_grow_count, setup->functions_reserve_cap);
}

void hcc_aml_deinit(HccCU* cu) {
	hcc_aml_function_alctor_deinit(cu);
}

void hcc_aml_print_operand(HccCU* cu, const HccAMLFunction* function, HccAMLOperand operand, HccIIO* iio, bool is_definition) {
	const char* fmt;
	switch (HCC_AML_OPERAND_TYPE(operand)) {
		case HCC_AML_OPERAND_VALUE: {
			HccAMLValue* value = &function->values[HCC_AML_OPERAND_AUX(operand)];
			if (is_definition) {
				HccString data_type_name = hcc_data_type_string(cu, value->data_type);
				if (iio->ascii_colors_enabled) {
					fmt = "\x1b[94m%.*s \x1b[93m%%%u\x1b[0m";
				} else {
					fmt = "%.*s %%%u";
				}
				hcc_iio_write_fmt(iio, fmt, (int)data_type_name.size, data_type_name.data, HCC_AML_OPERAND_AUX(operand));
			} else {
				if (iio->ascii_colors_enabled) {
					fmt = "\x1b[93m%%%u\x1b[0m";
				} else {
					fmt = "%%%u";
				}
				hcc_iio_write_fmt(iio, fmt, HCC_AML_OPERAND_AUX(operand));
			}
			break;
		};
		case HCC_AML_OPERAND_CONSTANT: {
			hcc_constant_print(cu, HccConstantId(HCC_AML_OPERAND_AUX(operand)), iio);
			break;
		};
		case HCC_AML_OPERAND_BASIC_BLOCK: {
			if (iio->ascii_colors_enabled) {
				fmt = "\x1b[96m@%u\x1b[0m";
			} else {
				fmt = "@%u";
			}
			hcc_iio_write_fmt(iio, fmt, HCC_AML_OPERAND_AUX(operand));
			break;
		};
		case HCC_AML_OPERAND_BASIC_BLOCK_PARAM: {
			HccAMLBasicBlockParam* param = &function->basic_block_params[HCC_AML_OPERAND_AUX(operand)];
			if (is_definition) {
				HccString data_type_name = hcc_data_type_string(cu, param->data_type);
				if (iio->ascii_colors_enabled) {
					fmt = "\x1b[94m%.*s \x1b[93m^%u\x1b[0m";
				} else {
					fmt = "%.*s ^%u";
				}
				hcc_iio_write_fmt(iio, fmt, (int)data_type_name.size, data_type_name.data, HCC_AML_OPERAND_AUX(operand));
			} else {
				if (iio->ascii_colors_enabled) {
					fmt = "\x1b[93m^%u\x1b[0m";
				} else {
					fmt = "^%u";
				}
				hcc_iio_write_fmt(iio, fmt, HCC_AML_OPERAND_AUX(operand));
			}
			break;
		};
		case HCC_DECL_GLOBAL_VARIABLE: {
			if (iio->ascii_colors_enabled) {
				fmt = "\x1b[92m$%.*s\x1b[0m";
			} else {
				fmt = "$%.*s";
			}
			HccASTVariable* variable = hcc_ast_global_variable_get(cu, (HccDecl)operand);
			HccString identifier_string = hcc_string_table_get_or_empty(variable->identifier_string_id);
			hcc_iio_write_fmt(iio, fmt, (int)identifier_string.size, identifier_string.data);
			break;
		};
		case HCC_DECL_FUNCTION: {
			if (iio->ascii_colors_enabled) {
				fmt = "\x1b[92m$%.*s\x1b[0m";
			} else {
				fmt = "$%.*s";
			}
			HccASTFunction* function = hcc_ast_function_get(cu, (HccDecl)operand);
			HccString identifier_string = hcc_string_table_get_or_empty(function->identifier_string_id);
			hcc_iio_write_fmt(iio, fmt, (int)identifier_string.size, identifier_string.data);
			break;
		};
		case HCC_DECL_ENUM_VALUE: {
			HccEnumValue* enum_value = hcc_enum_value_get(cu, (HccDecl)operand);
			hcc_constant_print(cu, enum_value->constant_id, iio);
			break;
		};
		case HCC_DECL_LOCAL_VARIABLE: {
			HCC_ABORT("we shouldn't have access to local variables from the AST in the AML");
			break;
		};
		default: {
			if (iio->ascii_colors_enabled) {
				fmt = "\x1b[94m%.*s\x1b[0m";
			} else {
				fmt = "%.*s";
			}
			HccString data_type_name = hcc_data_type_string(cu, (HccDataType)operand);
			hcc_iio_write_fmt(iio, fmt, (int)data_type_name.size, data_type_name.data);
			break;
		};
	}
}

void hcc_aml_print(HccCU* cu, HccIIO* iio) {
	const char* fmt;
	uint32_t functions_count = hcc_stack_count(cu->aml.functions);
	for (uint32_t function_idx = HCC_FUNCTION_IDX_USER_START; function_idx < functions_count; function_idx += 1) {
		const HccAMLFunction* function = cu->aml.functions[function_idx];
		HccString name = hcc_string_table_get_or_empty(function->identifier_string_id);
		if (iio->ascii_colors_enabled) {
			fmt = "\x1b[94mFunction\x1b[0m(\x1b[93m#%u\x1b[0m): \x1b[1m%.*s\x1b[0m(";
		} else {
			fmt = "Function(#%u): %.*s(";
		}

		hcc_iio_write_fmt(iio, fmt, function_idx, (int)name.size, name.data);

		for (uint32_t param_idx = 0; param_idx < function->params_count; param_idx += 1) {
			hcc_aml_print_operand(cu, function, HCC_AML_OPERAND(VALUE, param_idx), iio, true);

			if (param_idx + 1 < function->params_count) {
				hcc_iio_write_fmt(iio, ", ");
			}
		}

		hcc_iio_write_fmt(iio, "):\n");

		for (uint32_t word_idx = 0; word_idx < function->words_count; ) {
			HccAMLInstr* instr = &function->words[word_idx];
			HccAMLOperand* operands = HCC_AML_INSTR_OPERANDS(instr);
			uint32_t operands_count = HCC_AML_INSTR_OPERANDS_COUNT(instr);
			HccAMLOp op = HCC_AML_INSTR_OP(instr);

			hcc_iio_write_fmt(iio, "\t");
			if (op != HCC_AML_OP_BASIC_BLOCK) {
				hcc_iio_write_fmt(iio, "\t");
			}

			bool has_return_value_register = hcc_aml_op_code_has_return_value[op];
			if (has_return_value_register) {
				hcc_aml_print_operand(cu, function, operands[0], iio, true);
				hcc_iio_write_fmt(iio, " = ");
			}

			if (iio->ascii_colors_enabled) {
				fmt = "\x1b[91m%s\x1b[0m(";
			} else {
				fmt = "%s(";
			}
			hcc_iio_write_fmt(iio, fmt, hcc_aml_op_code_strings[op]);

			if (op == HCC_AML_OP_BASIC_BLOCK) {
				HccAMLBasicBlock* bb = &function->basic_blocks[HCC_AML_OPERAND_AUX(operands[0])];
				for (uint32_t param_idx = bb->params_start_idx; param_idx < bb->params_start_idx + bb->params_count; param_idx += 1) {
					hcc_aml_print_operand(cu, function, HCC_AML_OPERAND(BASIC_BLOCK_PARAM, param_idx), iio, true);

					if (param_idx + 1 < bb->params_count) {
						hcc_iio_write_fmt(iio, ", ");
					}
				}
				hcc_iio_write_fmt(iio, "):\n");
			} else {
				for (uint32_t operand_idx = has_return_value_register; operand_idx < operands_count; operand_idx += 1) {
					hcc_aml_print_operand(cu, function, operands[operand_idx], iio, false);

					if (operand_idx + 1 < operands_count) {
						hcc_iio_write_fmt(iio, ", ");
					}
				}
				hcc_iio_write_fmt(iio, ");\n");
			}

			word_idx += HCC_AML_INSTR_WORDS_COUNT(instr);
		}
	}
}

HccLocation* hcc_aml_instr_location(HccCU* cu, HccAMLInstr* instr) {
	return *hcc_stack_get(cu->aml.locations, HCC_AML_INSTR_LOCATION_IDX(instr));
}

HccStack(HccDecl) hcc_aml_optimize_functions(HccCU* cu) {
	return cu->aml.optimize_functions[cu->aml.optimize_functions_idx];
}

void hcc_aml_next_optimize_functions_array(HccCU* cu) {
	cu->aml.optimize_functions_idx = (cu->aml.optimize_functions_idx + 1) % 2;
	hcc_stack_clear(cu->aml.optimize_functions[cu->aml.optimize_functions_idx]);
}

HccAMLOperand hcc_aml_basic_block_next(const HccAMLFunction* function, HccAMLOperand basic_block_operand) {
	HCC_DEBUG_ASSERT(HCC_AML_OPERAND_IS_BASIC_BLOCK(basic_block_operand), "expected a basic block");

	HccAMLBasicBlock* basic_block = &function->basic_blocks[HCC_AML_OPERAND_AUX(basic_block_operand)];
	if (basic_block->terminating_instr_word_idx + 1 >= function->words_count) {
		return 0;
	}

	HccAMLInstr* instr = &function->words[basic_block->terminating_instr_word_idx];
	instr = &instr[HCC_AML_INSTR_WORDS_COUNT(instr)];
	HccAMLOperand next_basic_block_operand = HCC_AML_INSTR_OPERANDS(instr)[0];
	HCC_DEBUG_ASSERT(HCC_AML_OPERAND_IS_BASIC_BLOCK(next_basic_block_operand), "expected the next instruction after the terminating instruction to be a basic block");
	return next_basic_block_operand;
}

HccAMLOperand hcc_aml_instr_switch_merge_basic_block_operand(const HccAMLFunction* function, HccAMLInstr* instr) {
	HccAMLOp op = HCC_AML_INSTR_OP(instr);
	HccAMLOperand* operands = HCC_AML_INSTR_OPERANDS(instr);
	uint32_t operands_count = HCC_AML_INSTR_OPERANDS_COUNT(instr);
	HCC_DEBUG_ASSERT(op == HCC_AML_OP_SWITCH, "must be a switch instruction");

	//
	// find the last basic block defined physically in the switch statement
	HccAMLOperand last_basic_block_operand = operands[1]; // default block
	last_basic_block_operand = HCC_MAX(last_basic_block_operand, operands[operands_count - 1]); // last case block
	HCC_DEBUG_ASSERT(HCC_AML_OPERAND_IS_BASIC_BLOCK(last_basic_block_operand), "expected the default block or last case block to be a basic block");

	//
	// the merge block for switch is the basic block that follows the last basic block in the switch case physically.
	// so lets move to that next block now.
	return hcc_aml_basic_block_next(function, last_basic_block_operand);
}

