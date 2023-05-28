#include "hcc_internal.h"

// ===========================================
//
//
// Preprocessor Generator
//
//
// ===========================================

void hcc_ppgen_init(HccWorker* w, HccPPGenSetup* setup) {
	hcc_code_file_init(&w->atagen.ppgen.concat_buffer_code_file, hcc_string_lit("<concat-buffer>"), true);
	w->atagen.ppgen.expand_stack = hcc_stack_init(HccPPExpand, HCC_ALLOC_TAG_PPGEN_EXPAND_STACK, setup->expand_stack_grow_count, setup->expand_stack_reserve_cap);
	w->atagen.ppgen.expand_macro_idx_stack = hcc_stack_init(uint32_t, HCC_ALLOC_TAG_PPGEN_EXPAND_MACRO_IDX_STACK, setup->expand_stack_grow_count, setup->expand_stack_reserve_cap);
	w->atagen.ppgen.stringify_buffer = hcc_stack_init(char, HCC_ALLOC_TAG_PPGEN_STRINGIFY_BUFFER, setup->stringify_buffer_grow_count, setup->stringify_buffer_reserve_cap);
	w->atagen.ppgen.if_stack = hcc_stack_init(HccPPGenIf, HCC_ALLOC_TAG_PPGEN_IF_STACK, setup->if_stack_grow_count, setup->if_stack_reserve_cap);
	w->atagen.ppgen.macro_declarations = hcc_hash_table_init(HccPPGenMacroDeclEntry, HCC_ALLOC_TAG_PPGEN_MACRO_DECLARATIONS, hcc_u32_key_cmp, hcc_u32_key_hash, setup->macro_declarations_cap);
	w->atagen.ppgen.macro_args_stack = hcc_stack_init(HccPPMacroArg, HCC_ALLOC_TAG_PPGEN_MACRO_ARGS_STACK, setup->macro_args_stack_grow_count, setup->macro_args_stack_reserve_cap);
}

void hcc_ppgen_deinit(HccWorker* w) {
	hcc_code_file_deinit(&w->atagen.ppgen.concat_buffer_code_file);
	hcc_stack_deinit(w->atagen.ppgen.expand_stack);
	hcc_stack_deinit(w->atagen.ppgen.expand_macro_idx_stack);
	hcc_stack_deinit(w->atagen.ppgen.stringify_buffer);
	hcc_stack_deinit(w->atagen.ppgen.if_stack);
	hcc_hash_table_deinit(w->atagen.ppgen.macro_declarations);
	hcc_stack_deinit(w->atagen.ppgen.macro_args_stack);
}

void hcc_ppgen_reset(HccWorker* w) {
	hcc_stack_clear(w->atagen.ppgen.expand_stack);
	hcc_stack_clear(w->atagen.ppgen.expand_macro_idx_stack);
	hcc_stack_clear(w->atagen.ppgen.stringify_buffer);
	hcc_stack_clear(w->atagen.ppgen.if_stack);
	hcc_hash_table_clear(w->atagen.ppgen.macro_declarations);
	hcc_stack_clear(w->atagen.ppgen.macro_args_stack);

	HccOptions* options = hcc_worker_cu(w)->options;
	HccTargetArch target_arch = hcc_options_get_u32(options, HCC_OPTION_KEY_TARGET_ARCH);
	HccTargetOS target_os = hcc_options_get_u32(options, HCC_OPTION_KEY_TARGET_OS);
	for (HccPPPredefinedMacro m = 0; m < HCC_PP_PREDEFINED_MACRO_COUNT; m += 1) {
		if (m == HCC_PP_PREDEFINED_MACRO___HCC_LINUX__ && target_os != HCC_TARGET_OS_LINUX) {
			continue;
		}
		if (m == HCC_PP_PREDEFINED_MACRO___HCC_WINDOWS__ && target_os != HCC_TARGET_OS_WINDOWS) {
			continue;
		}
		if (m == HCC_PP_PREDEFINED_MACRO___HCC_X86_64__ && target_arch != HCC_TARGET_ARCH_X86_64) {
			continue;
		}

		HccStringId identifier_string_id = { HCC_STRING_ID_PREDEFINED_MACROS_START + m };
		HccHashTableInsert insert = hcc_hash_table_find_insert_idx(w->atagen.ppgen.macro_declarations, &identifier_string_id);
		HCC_DEBUG_ASSERT(insert.is_new, "internal error: predefined macro has already been initialized");
		w->atagen.ppgen.macro_declarations[insert.idx].macro_idx = UINT32_MAX;
	}
}

HccPPIfSpan* hcc_ppgen_if_span_get(HccWorker* w, uint32_t pp_if_span_id) {
	HCC_DEBUG_ASSERT(
		w->atagen.we_are_mutator_of_code_file || (atomic_load(&w->atagen.location.code_file->flags) & HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS),
		"we can only get the pp if span if we are the mutator of the code file or the code file has been atagen'd before by the mutator of the code file"
	);
	HCC_DEBUG_ASSERT_NON_ZERO(pp_if_span_id);
	HccCodeFile* code_file = w->atagen.location.code_file;
	return hcc_stack_get(code_file->pp_if_spans, pp_if_span_id - 1);
}

uint32_t hcc_ppgen_if_span_id(HccWorker* w, HccPPIfSpan* if_span) {
	HccCodeFile* code_file = w->atagen.location.code_file;
	HCC_DEBUG_ASSERT(
		w->atagen.we_are_mutator_of_code_file || (atomic_load(&w->atagen.location.code_file->flags) & HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS),
		"we can only get the pp if span if we are the mutator of the code file or the code file has been atagen'd before by the mutator of the code file"
	);
	HCC_DEBUG_ASSERT(code_file->pp_if_spans <= if_span && if_span <= hcc_stack_get_last(code_file->pp_if_spans), "if_span is out of the array bounds");
	uint32_t id = (if_span - hcc_stack_get_first(code_file->pp_if_spans)) + 1;
	return id;
}

HccPPIfSpan* hcc_ppgen_if_span_push(HccWorker* w, HccPPDirective directive) {
	HccCodeFile* code_file = w->atagen.location.code_file;
	w->atagen.pp_if_span_id += 1;

	HccPPIfSpan* pp_if_span = NULL;
	if (!(atomic_load(&code_file->flags) & HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS)) {
		if (w->atagen.we_are_mutator_of_code_file) {
#if HCC_DEBUG_CODE_IF_SPAN
			printf("#%s at line %u\n", hcc_pp_directive_strings[directive], w->atagen.location.line_start);
#endif // HCC_DEBUG_CODE_IF_SPAN
			pp_if_span = hcc_stack_push(code_file->pp_if_spans);
			pp_if_span->directive = directive;
			pp_if_span->location = w->atagen.location;
			pp_if_span->location.display_line = hcc_atagen_display_line(w);
			pp_if_span->first_id = hcc_ppgen_if_span_id(w, pp_if_span);
			pp_if_span->has_else = false;
			pp_if_span->prev_id = 0;
			pp_if_span->next_id = 0;
			pp_if_span->last_id = 0;
		}
	} else {
#if HCC_DEBUG_CODE_IF_SPAN
		printf("refound #%s at line %u\n", hcc_pp_directive_strings[directive], w->atagen.location.line_start);
#endif // HCC_DEBUG_CODE_IF_SPAN
		pp_if_span = hcc_ppgen_if_span_get(w, w->atagen.pp_if_span_id);

		HccLocation location = w->atagen.location;
		location.display_line = pp_if_span->location.display_line;
		HCC_DEBUG_ASSERT(
			pp_if_span->directive == directive &&
			HCC_CMP_ELMT(&pp_if_span->location, &location),
			"internal error: preprocessor if mismatch from first time this was included"
		);
	}

	return pp_if_span;
}

void hcc_ppgen_if_found_if(HccWorker* w, HccPPDirective directive) {
	uint32_t start_span_id = 0;
	HccPPIfSpan* pp_if_span = hcc_ppgen_if_span_push(w, directive);
	if (pp_if_span) {
		start_span_id = hcc_ppgen_if_span_id(w, pp_if_span);
	}

	*hcc_stack_push(w->atagen.ppgen.if_stack) = (HccPPGenIf) {
		.location = w->atagen.location,
		.directive = directive,
		.start_span_id = start_span_id,
		.has_else = false,
	};
}

HccPPIfSpan* hcc_ppgen_if_found_if_counterpart(HccWorker* w, HccPPDirective directive) {
	HccCodeFile* code_file = w->atagen.location.code_file;
	HccPPIfSpan* counterpart = hcc_ppgen_if_span_push(w, directive);

	if (w->atagen.we_are_mutator_of_code_file) {
		if (!(atomic_load(&code_file->flags) & HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS)) {
			uint32_t counterpart_id = hcc_ppgen_if_span_id(w, counterpart);
			HccPPIfSpan* pp_if_span = hcc_ppgen_if_span_get(w, hcc_stack_get_last(w->atagen.ppgen.if_stack)->start_span_id);
			counterpart->first_id = hcc_ppgen_if_span_id(w, pp_if_span);
			counterpart->prev_id = pp_if_span->last_id;
			if (counterpart->prev_id) {
				HccPPIfSpan* prev = hcc_ppgen_if_span_get(w, counterpart->prev_id);
				prev->next_id = counterpart_id;
			} else {
				pp_if_span->next_id = counterpart_id;
			}
			pp_if_span->last_id = counterpart_id;
		}
	}

	return counterpart;
}

void hcc_ppgen_if_found_endif(HccWorker* w) {
	HccPPIfSpan* pp_if_span = hcc_ppgen_if_found_if_counterpart(w, HCC_PP_DIRECTIVE_ENDIF);
	if (pp_if_span) {
		pp_if_span->next_id = 0;
	}
	hcc_stack_pop(w->atagen.ppgen.if_stack);
}

void hcc_ppgen_if_found_else(HccWorker* w, HccPPDirective directive) {
	HccPPGenIf* pp_if = hcc_stack_get_last(w->atagen.ppgen.if_stack);
	pp_if->has_else = true;

	hcc_ppgen_if_found_if_counterpart(w, directive);
}

void hcc_ppgen_if_ensure_first_else(HccWorker* w, HccPPDirective directive) {
	HccPPGenIf* pp_if = hcc_stack_get_last(w->atagen.ppgen.if_stack);

	HccCodeFile* code_file = w->atagen.location.code_file;
	if (!(atomic_load(&code_file->flags) & HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS)) {
		if (pp_if->has_else) {
			hcc_atagen_bail_error_2(w, HCC_ERROR_CODE_PP_ELSEIF_CANNOT_FOLLOW_ELSE, &w->atagen.location, &pp_if->location, hcc_pp_directive_strings[directive]);
		}
	}
}

void hcc_ppgen_if_ensure_one_is_open(HccWorker* w, HccPPDirective directive) {
	if (hcc_stack_count(w->atagen.ppgen.if_stack) == 0) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_PP_ENDIF_BEFORE_IF, hcc_pp_directive_strings[directive]);
	}
}

void hcc_ppgen_eval_binary_op(HccWorker* w, uint32_t* token_idx_mut, HccASTBinaryOp* binary_op_type_out, uint32_t* precedence_out) {
	HccATAToken token = *hcc_stack_get(w->atagen.ast_file->token_bag.tokens, *token_idx_mut);
	switch (token) {
		case HCC_ATA_TOKEN_ASTERISK:              *binary_op_type_out = HCC_AST_BINARY_OP_MULTIPLY;              *precedence_out = 3;  break;
		case HCC_ATA_TOKEN_FORWARD_SLASH:         *binary_op_type_out = HCC_AST_BINARY_OP_DIVIDE;                *precedence_out = 3;  break;
		case HCC_ATA_TOKEN_PERCENT:               *binary_op_type_out = HCC_AST_BINARY_OP_MODULO;                *precedence_out = 3;  break;
		case HCC_ATA_TOKEN_PLUS:                  *binary_op_type_out = HCC_AST_BINARY_OP_ADD;                   *precedence_out = 4;  break;
		case HCC_ATA_TOKEN_MINUS:                 *binary_op_type_out = HCC_AST_BINARY_OP_SUBTRACT;              *precedence_out = 4;  break;
		case HCC_ATA_TOKEN_BIT_SHIFT_LEFT:        *binary_op_type_out = HCC_AST_BINARY_OP_BIT_SHIFT_LEFT;        *precedence_out = 5;  break;
		case HCC_ATA_TOKEN_BIT_SHIFT_RIGHT:       *binary_op_type_out = HCC_AST_BINARY_OP_BIT_SHIFT_RIGHT;       *precedence_out = 5;  break;
		case HCC_ATA_TOKEN_LESS_THAN:             *binary_op_type_out = HCC_AST_BINARY_OP_LESS_THAN;             *precedence_out = 6;  break;
		case HCC_ATA_TOKEN_LESS_THAN_OR_EQUAL:    *binary_op_type_out = HCC_AST_BINARY_OP_LESS_THAN_OR_EQUAL;    *precedence_out = 6;  break;
		case HCC_ATA_TOKEN_GREATER_THAN:          *binary_op_type_out = HCC_AST_BINARY_OP_GREATER_THAN;          *precedence_out = 6;  break;
		case HCC_ATA_TOKEN_GREATER_THAN_OR_EQUAL: *binary_op_type_out = HCC_AST_BINARY_OP_GREATER_THAN_OR_EQUAL; *precedence_out = 6;  break;
		case HCC_ATA_TOKEN_LOGICAL_EQUAL:         *binary_op_type_out = HCC_AST_BINARY_OP_EQUAL;                 *precedence_out = 7;  break;
		case HCC_ATA_TOKEN_LOGICAL_NOT_EQUAL:     *binary_op_type_out = HCC_AST_BINARY_OP_NOT_EQUAL;             *precedence_out = 7;  break;
		case HCC_ATA_TOKEN_AMPERSAND:             *binary_op_type_out = HCC_AST_BINARY_OP_BIT_AND;               *precedence_out = 8;  break;
		case HCC_ATA_TOKEN_CARET:                 *binary_op_type_out = HCC_AST_BINARY_OP_BIT_XOR;               *precedence_out = 9;  break;
		case HCC_ATA_TOKEN_PIPE:                  *binary_op_type_out = HCC_AST_BINARY_OP_BIT_OR;                *precedence_out = 10; break;
		case HCC_ATA_TOKEN_LOGICAL_AND:           *binary_op_type_out = HCC_AST_BINARY_OP_LOGICAL_AND;           *precedence_out = 11; break;
		case HCC_ATA_TOKEN_LOGICAL_OR:            *binary_op_type_out = HCC_AST_BINARY_OP_LOGICAL_OR;            *precedence_out = 12; break;
		case HCC_ATA_TOKEN_QUESTION_MARK:         *binary_op_type_out = HCC_AST_BINARY_OP_TERNARY;               *precedence_out = 13; break;
		case HCC_ATA_TOKEN_COMMA:                 *binary_op_type_out = HCC_AST_BINARY_OP_COMMA;                 *precedence_out = 15; break;

		case HCC_ATA_TOKEN_PARENTHESIS_CLOSE:
		case HCC_ATA_TOKEN_COLON:
			*binary_op_type_out = HCC_AST_BINARY_OP_ASSIGN;
			*precedence_out = 0;
			break;

		default:
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PP_BINARY_OP, hcc_ata_token_strings[token]);
			break;
	}
}

HccPPEval hcc_ppgen_eval_unary_expr(HccWorker* w, uint32_t* token_idx_mut, uint32_t* token_value_idx_mut) {
	HccATAToken token = *hcc_stack_get(w->atagen.ast_file->token_bag.tokens, *token_idx_mut);
	*token_idx_mut += 1;

	HccPPEval eval;
	HccASTUnaryOp unary_op;
	switch (token) {
		case HCC_ATA_TOKEN_LIT_UINT:
		case HCC_ATA_TOKEN_LIT_ULONG:
		case HCC_ATA_TOKEN_LIT_ULONGLONG: {
			HccConstantId constant_id = hcc_stack_get(w->atagen.ast_file->token_bag.values, *token_value_idx_mut)->constant_id;
			*token_value_idx_mut += 2; // skip the constant and it's associated string id

			HccConstant constant = hcc_constant_table_get(w->cu, constant_id);
			uint64_t u64;
			HCC_DEBUG_ASSERT(hcc_constant_as_uint(w->cu, constant, &u64), "internal error: expected to be a unsigned int");

			eval.data_type = HCC_DATA_TYPE_AML_INTRINSIC_U64;
			eval.basic.u64 = u64;
			break;
		};

		case HCC_ATA_TOKEN_LIT_SINT:
		case HCC_ATA_TOKEN_LIT_SLONG:
		case HCC_ATA_TOKEN_LIT_SLONGLONG: {
			HccConstantId constant_id = hcc_stack_get(w->atagen.ast_file->token_bag.values, *token_value_idx_mut)->constant_id;
			*token_value_idx_mut += 2; // skip the constant and it's associated string id

			HccConstant constant = hcc_constant_table_get(w->cu, constant_id);
			int64_t s64;
			HCC_DEBUG_ASSERT(hcc_constant_as_sint(w->cu, constant, &s64), "internal error: expected to be a signed int");

			eval.data_type = HCC_DATA_TYPE_AML_INTRINSIC_S64;
			eval.basic.s64 = s64;
			break;
		};

		case HCC_ATA_TOKEN_PARENTHESIS_OPEN:
			eval = hcc_ppgen_eval_expr(w, 0, token_idx_mut, token_value_idx_mut);
			token = *hcc_stack_get(w->atagen.ast_file->token_bag.tokens, *token_idx_mut);
			if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
				hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE);
			}
			*token_idx_mut += 1;
			break;

		case HCC_ATA_TOKEN_TILDE: unary_op = HCC_AST_UNARY_OP_BIT_NOT; goto UNARY;
		case HCC_ATA_TOKEN_EXCLAMATION_MARK: unary_op = HCC_AST_UNARY_OP_LOGICAL_NOT; goto UNARY;
		case HCC_ATA_TOKEN_PLUS: unary_op = HCC_AST_UNARY_OP_PLUS; goto UNARY;
		case HCC_ATA_TOKEN_MINUS: unary_op = HCC_AST_UNARY_OP_NEGATE; goto UNARY;
UNARY:
		{
			eval = hcc_ppgen_eval_expr(w, 0, token_idx_mut, token_value_idx_mut);
			switch (unary_op) {
				case HCC_ATA_TOKEN_TILDE:            eval.basic.u64 = ~eval.basic.u64; break;
				case HCC_ATA_TOKEN_EXCLAMATION_MARK: eval.basic.u64 = !eval.basic.u64; break;
				case HCC_ATA_TOKEN_PLUS:                                   break;
				case HCC_ATA_TOKEN_MINUS:            eval.basic.u64 = -eval.basic.u64; break;
				default: HCC_UNREACHABLE();
			}
			break;
		};

		default:
			if (HCC_ATA_TOKEN_IS_KEYWORD(token) || token == HCC_ATA_TOKEN_IDENT) {
				//
				// the spec states that any identifier that is not evaluated during macro expansion
				// gets substituded for a 0.
				//
				eval.data_type = HCC_DATA_TYPE_AML_INTRINSIC_S64;
				eval.basic.s64 = 0;

				if (token == HCC_ATA_TOKEN_IDENT) {
					*token_value_idx_mut += 1; // skip the identifier's string id
				}
			} else {
				*token_idx_mut -= 1;
				hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PP_UNARY_EXPR, hcc_ata_token_strings[token]);
			}
			break;
	}

	return eval;
}

HccPPEval hcc_ppgen_eval_expr(HccWorker* w, uint32_t min_precedence, uint32_t* token_idx_mut, uint32_t* token_value_idx_mut) {
	HccPPEval left_eval = hcc_ppgen_eval_unary_expr(w, token_idx_mut, token_value_idx_mut);

	while (*token_idx_mut < hcc_stack_count(w->atagen.ast_file->token_bag.tokens)) {
		HccASTBinaryOp binary_op;
		uint32_t precedence;
		hcc_ppgen_eval_binary_op(w, token_idx_mut, &binary_op, &precedence);
		if (binary_op == HCC_AST_BINARY_OP_ASSIGN || (min_precedence && min_precedence <= precedence)) {
			return left_eval;
		}
		*token_idx_mut += 1;

		HccPPEval eval;
		if (binary_op == HCC_AST_BINARY_OP_TERNARY) {
			HccPPEval true_eval = hcc_ppgen_eval_expr(w, 0, token_idx_mut, token_value_idx_mut);
			HccATAToken token = *hcc_stack_get(w->atagen.ast_file->token_bag.tokens, *token_idx_mut);
			if (token != HCC_ATA_TOKEN_COLON) {
				hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_COLON_FOR_TERNARY_OP);
			}
			*token_idx_mut += 1;
			HccPPEval false_eval = hcc_ppgen_eval_expr(w, 0, token_idx_mut, token_value_idx_mut);
			eval = left_eval.basic.u64 ? true_eval : false_eval;
		} else {
			HccPPEval right_eval = hcc_ppgen_eval_expr(w, precedence, token_idx_mut, token_value_idx_mut);
			eval.data_type = left_eval.data_type;
			eval.basic = hcc_basic_eval_binary(w->cu, binary_op, left_eval.data_type, left_eval.basic, right_eval.basic);
		}

		left_eval = eval;
	}

	return left_eval;
}

void hcc_ppgen_ensure_end_of_directive(HccWorker* w, HccErrorCode error_code, HccPPDirective directive) {
	char byte = w->atagen.code[w->atagen.location.code_end_idx];
	char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
	if (
		byte != '\r' &&
		byte != '\n' &&
		!(byte == '/' && next_byte == '/') &&
		!(byte == '/' && next_byte == '*')
	) {
		if (w->atagen.location.code_end_idx < w->atagen.code_size) {
			hcc_atagen_bail_error_1(w, error_code, hcc_pp_directive_strings[directive]);
		}
	}
}

void hcc_ppgen_parse_define(HccWorker* w) {
	//
	// skip the whitespace after the #define
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	//
	// parse the identifier for the macro
	HccString ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER);
	HccStringId identifier_string_id;
	hcc_string_table_deduplicate(ident_string.data, ident_string.size, &identifier_string_id);
	hcc_atagen_advance_column(w, ident_string.size);
	HccLocation* ident_location = hcc_atagen_make_location(w);

	uint32_t params_start_idx = hcc_stack_count(w->atagen.ast_file->macro_params);
	uint32_t params_count = 0;
	bool is_function = false;
	bool has_va_args = false;
	if (w->atagen.code[w->atagen.location.code_end_idx] == '(') {
		//
		// we found a open parenthesis right after the identifier
		// so this is a function-like macro.
		// let's collect the parameter names inside of the pair of parenthesis.
		//
		is_function = true;
		hcc_atagen_advance_column(w, 1);
		hcc_atagen_consume_whitespace(w);

		if (w->atagen.code[w->atagen.location.code_end_idx] != ')') {
			//
			// function-like has parameters
			//
			while (1) { // for each parameter
				params_count += 1;
				HccStringId param_string_id;
				if (
					w->atagen.code[w->atagen.location.code_end_idx] == '.' &&
					w->atagen.code[w->atagen.location.code_end_idx + 1] == '.' &&
					w->atagen.code[w->atagen.location.code_end_idx + 2] == '.'
				) {
					//
					// we found a ellipse '...' aka. the va arg
					//
					hcc_atagen_advance_column(w, 3);
					has_va_args = true;
					param_string_id.idx_plus_one = HCC_STRING_ID___VA_ARGS__;
				} else {
					//
					// parse the parameter identifier and ensure that it is unique
					//
					HccString ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_PARAM_IDENTIFIER);
					hcc_string_table_deduplicate(ident_string.data, ident_string.size, &param_string_id);
					hcc_atagen_advance_column(w, ident_string.size);
					if (param_string_id.idx_plus_one == HCC_STRING_ID___VA_ARGS__) {
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_VA_ARGS_IN_MACRO_PARAMETER);
					}

					for (uint32_t idx = params_start_idx; idx < hcc_stack_count(w->atagen.ast_file->macro_params); idx += 1) {
						if (hcc_stack_get(w->atagen.ast_file->macro_params, idx)->idx_plus_one == param_string_id.idx_plus_one) {
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_DUPLICATE_MACRO_PARAM_IDENTIFIER, (int)ident_string.size, ident_string.data);
						}
					}
				}

				//
				// push the parameter onto the end of the array
				*hcc_stack_push(w->atagen.ast_file->macro_params) = param_string_id;
				hcc_atagen_consume_whitespace(w);

				char byte = w->atagen.code[w->atagen.location.code_end_idx];
				if (byte == ')') {
					//
					// we found the closing parenthesis so we are finished here
					break;
				}

				if (byte != ',') {
					//
					// huh, we couldn't find a ')' or a ',' after the identifier
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_MACRO_PARAM_DELIMITER);
				}

				if (has_va_args) {
					//
					// a ',' was found after the va arg '...'
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MACRO_PARAM_VA_ARG_NOT_LAST);
				}

				hcc_atagen_advance_column(w, 1); // skip ','
				hcc_atagen_consume_whitespace(w);
			}
		}

		hcc_atagen_advance_column(w, 1); // skip ')'
	}

	hcc_atagen_consume_whitespace(w);

	uint32_t tokens_start_idx = hcc_stack_count(w->atagen.ast_file->macro_token_bag.tokens);
	uint32_t token_values_start_idx = hcc_stack_count(w->atagen.ast_file->macro_token_bag.values);
	w->atagen.macro_is_function = is_function;
	w->atagen.macro_has_va_arg = has_va_args;
	w->atagen.macro_param_string_ids = hcc_stack_get_or_null(w->atagen.ast_file->macro_params, params_start_idx);
	w->atagen.macro_params_count = params_count;
	hcc_atagen_run(w, &w->atagen.ast_file->macro_token_bag, HCC_ATAGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST);
	uint32_t tokens_end_idx = hcc_stack_count(w->atagen.ast_file->macro_token_bag.tokens);
	uint32_t token_values_end_idx = hcc_stack_count(w->atagen.ast_file->macro_token_bag.values);
	uint32_t tokens_count = tokens_end_idx - tokens_start_idx;
	uint32_t token_values_count = token_values_end_idx - token_values_start_idx;

	if (tokens_count && w->atagen.ast_file->macro_token_bag.tokens[tokens_end_idx - 1] == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
		tokens_end_idx -= 1;
		tokens_count -= 1;
	}

	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(w->atagen.ppgen.macro_declarations, &identifier_string_id);
	HccPPGenMacroDeclEntry* entry = &w->atagen.ppgen.macro_declarations[insert.idx];
	if (!insert.is_new) {
		HccPPMacro* existing_macro = hcc_stack_get(w->atagen.ast_file->macros, entry->macro_idx);
		HccATAToken* macro_tokens = hcc_stack_get_or_null(w->atagen.ast_file->macro_token_bag.tokens, tokens_start_idx);
		HccATAValue* macro_token_values = hcc_stack_get_or_null(w->atagen.ast_file->macro_token_bag.values, token_values_start_idx);
		HccATAToken* existing_macro_tokens = hcc_stack_get_or_null(w->atagen.ast_file->macro_token_bag.tokens, existing_macro->token_cursor.tokens_start_idx);
		HccATAValue* existing_macro_token_values = hcc_stack_get_or_null(w->atagen.ast_file->macro_token_bag.values, existing_macro->token_cursor.token_value_idx);
		HccStringId* param_string_ids = hcc_stack_get_or_null(w->atagen.ast_file->macro_params, params_start_idx);
		if (
			is_function != existing_macro->is_function ||
			(is_function && (
				params_count != existing_macro->params_count ||
				!HCC_CMP_ELMT_MANY(param_string_ids, existing_macro->params, params_count)
			)) ||
			tokens_count != hcc_ata_token_cursor_tokens_count(&existing_macro->token_cursor) ||
			!HCC_CMP_ELMT_MANY(macro_tokens, existing_macro_tokens, tokens_count) ||
			!HCC_CMP_ELMT_MANY(macro_token_values, existing_macro_token_values, token_values_count)
		) {
			hcc_atagen_bail_error_2(w, HCC_ERROR_CODE_MACRO_ALREADY_DEFINED, ident_location, existing_macro->location, (int)ident_string.size, ident_string.data);
		}

		//
		// we have found an identical redefinition of a macro,
		// so just removed the parameters and tokens that where parsed and return.
		hcc_stack_resize(w->atagen.ast_file->macro_params, params_start_idx);
		hcc_stack_resize(w->atagen.ast_file->macro_token_bag.tokens, tokens_start_idx);
		hcc_stack_resize(w->atagen.ast_file->macro_token_bag.locations, tokens_start_idx);
		hcc_stack_resize(w->atagen.ast_file->macro_token_bag.values, token_values_start_idx);
		return;
	}
	entry->macro_idx = hcc_stack_count(w->atagen.ast_file->macros);

	HccPPMacro* macro = hcc_stack_push(w->atagen.ast_file->macros);
	macro->identifier_string = hcc_string_table_get(identifier_string_id);
	macro->identifier_string_id = identifier_string_id;
	macro->location = ident_location;
	macro->token_cursor.tokens_start_idx = tokens_start_idx;
	macro->token_cursor.tokens_end_idx = tokens_end_idx;
	macro->token_cursor.token_value_idx = token_values_start_idx;
	macro->token_cursor.token_idx = tokens_start_idx;
	macro->params = hcc_stack_get_or_null(w->atagen.ast_file->macro_params, params_start_idx);
	macro->params_count = params_count;
	macro->is_function = is_function;
	macro->has_va_args = has_va_args;
}

void hcc_ppgen_parse_undef(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	//
	// parse the identifier for the macro
	HccString ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER);
	HccStringId identifier_string_id;
	hcc_string_table_deduplicate(ident_string.data, ident_string.size, &identifier_string_id);
	hcc_atagen_advance_column(w, ident_string.size);
	hcc_atagen_consume_whitespace(w);

	//
	// remove the macro from the hash table. we do not need to error if the macro is not defined.
	hcc_hash_table_remove(w->atagen.ppgen.macro_declarations, &identifier_string_id);

	hcc_ppgen_ensure_end_of_directive(w, HCC_ERROR_CODE_TOO_MANY_UNDEF_OPERANDS, HCC_PP_DIRECTIVE_UNDEF);
}

void hcc_ppgen_parse_include(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	//
	// run the tokenizer to get the single operand and expand any macros
	HccATATokenBag* token_bag = &w->atagen.ast_file->token_bag;
	uint32_t tokens_start_idx = hcc_stack_count(token_bag->tokens);
	uint32_t token_values_start_idx = hcc_stack_count(token_bag->values);
	uint32_t token_location_indices_start_idx = hcc_stack_count(token_bag->locations);
	hcc_atagen_run(w, token_bag, HCC_ATAGEN_RUN_MODE_PP_INCLUDE_OPERAND);

	//
	// error if no operands found
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_INCLUDE_OPERAND);
	}

	//
	// error if more that 1 operands found
	if (tokens_start_idx + 1 != hcc_stack_count(token_bag->tokens)) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_TOO_MANY_INCLUDE_OPERANDS);
	}

	HccATAToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx);
	if (token != HCC_ATA_TOKEN_STRING && token != HCC_ATA_TOKEN_INCLUDE_PATH_SYSTEM) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_INCLUDE_OPERAND);
	}

	HccStringId path_string_id = hcc_stack_get(token_bag->values, token_values_start_idx)->string_id;
	HccString path_string = hcc_string_table_get(path_string_id);
	if (path_string.size <= 1) { // <= as it has a null terminator
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INCLUDE_PATH_IS_EMPTY);
	}

	bool search_the_include_paths = false;
	switch (token) {
		case HCC_ATA_TOKEN_STRING:
			//
			// if the path is relative then glue the directory of the current file to the start of the include path.
			// this is to prevent finding a file relative from in the current working directory
			if (hcc_path_is_relative(path_string.data)) {
				HccCodeFile* code_file = w->atagen.location.code_file;
				HccString check_path = hcc_path_replace_file_name(code_file->path_string, path_string);
				if (!hcc_path_is_file(check_path.data)) {
					search_the_include_paths = true;
					break;
				}
				path_string = check_path;
			}

			//
			// the spec states the #include "" get 'upgraded' to a #include <>
			// if the file does not exist.
			//
			break;
		case HCC_ATA_TOKEN_INCLUDE_PATH_SYSTEM:
			search_the_include_paths = hcc_path_is_relative(path_string.data);
			break;
		default:
			HCC_UNREACHABLE("internal error: the token should have been checked above to ensure we never reach here");
	}

	if (search_the_include_paths) {
		//
		// we have a #include <> so search through all of the system library directories
		// and the user defined ones using the -I argument.
		//
		uint32_t idx;

		HccStack(HccString) include_path_strings = hcc_worker_task(w)->include_path_strings;
		uint32_t count = hcc_stack_count(include_path_strings);
		for (idx = 0; idx < count; idx += 1) {
			//
			// build {include_path}/{path} in the string buffer
			//
			hcc_stack_clear(w->string_buffer);

			HccString include_dir_path = *hcc_stack_get(include_path_strings, idx);
			HCC_DEBUG_ASSERT(include_dir_path.size, "internal error: include directory path cannot be zero sized");

			hcc_stack_push_string(w->string_buffer, include_dir_path);
			if (include_dir_path.data[include_dir_path.size - 1] != '/') {
				*hcc_stack_push(w->string_buffer) = '/';
			}
			hcc_stack_push_string(w->string_buffer, path_string);
			*hcc_stack_push(w->string_buffer) = '\0';

			if (hcc_path_is_file(w->string_buffer)) {
				path_string = hcc_string(w->string_buffer, hcc_stack_count(w->string_buffer));
				break;
			}
		}

		if (idx == count) {
			w->atagen.location = *HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(token_bag->locations, tokens_start_idx));
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INCLUDE_PATH_DOES_NOT_EXIST);
		}
	}

	path_string = hcc_path_canonicalize(path_string.data);
	if (path_string.size == 0) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_FAILED_TO_OPEN_FILE_FOR_READ, path_string.data);
	}

	HccCodeFile* code_file;
	HccResult result = hcc_code_file_find_or_insert(path_string, &code_file);
	if (!HCC_IS_SUCCESS(result)) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_FAILED_TO_OPEN_FILE_FOR_READ, path_string.data);
	}
	bool we_are_mutator_of_code_file = result.code == HCC_SUCCESS_IS_NEW;

	hcc_string_table_deduplicate(path_string.data, path_string.size, &path_string_id);
	hcc_ast_file_found_included_file(w->atagen.ast_file, path_string_id);
	if (!hcc_ast_file_has_been_pragma_onced(w->atagen.ast_file, path_string_id)) {
		hcc_atagen_paused_file_push(w);
		hcc_atagen_location_setup_new_file(w, code_file);
		w->atagen.we_are_mutator_of_code_file = we_are_mutator_of_code_file;
	}

	//
	// remove the added token for when we evaluated the include operand
	// using the call to hcc_atagen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->values, token_values_start_idx);
	hcc_stack_resize(token_bag->locations, token_location_indices_start_idx);
}

bool hcc_ppgen_parse_if(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	//
	// run the tokenizer to get the #if condition as a list of tokens
	HccATATokenBag* token_bag = &w->atagen.ast_file->token_bag;
	uint32_t tokens_start_idx = hcc_stack_count(token_bag->tokens);
	uint32_t token_values_start_idx = hcc_stack_count(token_bag->values);
	uint32_t token_location_indices_start_idx = hcc_stack_count(token_bag->locations);
	hcc_atagen_run(w, token_bag, HCC_ATAGEN_RUN_MODE_PP_IF_OPERAND);

	//
	// error if no tokens found
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_CONDITION_HAS_NO_PP_TOKENS);
	}

	//
	// evaluate the tokens and compute a boolean value
	uint32_t token_idx = tokens_start_idx;
	uint32_t token_value_idx = token_values_start_idx;
	bool is_true = !!hcc_ppgen_eval_expr(w, 0, &token_idx, &token_value_idx).basic.u64;

	HCC_DEBUG_ASSERT(token_idx == hcc_stack_count(token_bag->tokens), "internal error: preprocessor expression has not been fully evaluated");

	//
	// remove the added expression tokens for the #if operand that were
	// generated by the call to hcc_atagen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->values, token_values_start_idx);
	hcc_stack_resize(token_bag->locations, token_location_indices_start_idx);

	return is_true;
}

void hcc_ppgen_parse_defined(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);

	bool has_parenthesis = w->atagen.code[w->atagen.location.code_end_idx] == '(';
	if (has_parenthesis) {
		hcc_atagen_advance_column(w, 1); // skip '('
		hcc_atagen_consume_whitespace(w);
	}

	HccString macro_ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_PP_IF_DEFINED);
	HccStringId macro_string_id;
	hcc_string_table_deduplicate(macro_ident_string.data, macro_ident_string.size, &macro_string_id);

	bool does_macro_exist = hcc_hash_table_find_idx(w->atagen.ppgen.macro_declarations, &macro_string_id) != UINTPTR_MAX;
	hcc_atagen_advance_column(w, macro_ident_string.size);

	if (has_parenthesis) {
		hcc_atagen_consume_whitespace(w);
		if (w->atagen.code[w->atagen.location.code_end_idx] != ')') {
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_DEFINED);
		}
		hcc_atagen_advance_column(w, 1); // skip ')'
	}

	HccATAValue token_value;
	token_value.constant_id = does_macro_exist ? hcc_constant_table_deduplicate_one(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT) : hcc_constant_table_deduplicate_zero(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT);

	hcc_atagen_token_add(w, HCC_ATA_TOKEN_LIT_SINT);
	hcc_atagen_token_value_add(w, token_value);

	hcc_string_table_deduplicate_c_string(does_macro_exist ? "1" : "0", &token_value.string_id);
	hcc_atagen_token_value_add(w, token_value);
}

bool hcc_ppgen_parse_ifdef(HccWorker* w, HccPPDirective directive) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	HccString ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_INVALID_TOKEN_MACRO_IDENTIFIER);
	HccStringId identifier_string_id;
	hcc_string_table_deduplicate(ident_string.data, ident_string.size, &identifier_string_id);
	hcc_atagen_advance_column(w, ident_string.size);
	hcc_atagen_consume_whitespace(w);

	bool is_true = hcc_hash_table_find_idx(w->atagen.ppgen.macro_declarations, &identifier_string_id) != UINTPTR_MAX;
	hcc_ppgen_ensure_end_of_directive(w, HCC_ERROR_CODE_TOO_MANY_IFDEF_OPERANDS, directive);
	return is_true;
}

void hcc_ppgen_parse_line(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	//
	// run the tokenizer to get the #line operands as a list of tokens
	HccATATokenBag* token_bag = &w->atagen.ast_file->token_bag;
	uint32_t tokens_start_idx = hcc_stack_count(token_bag->tokens);
	uint32_t token_values_start_idx = hcc_stack_count(token_bag->values);
	uint32_t token_location_indices_start_idx = hcc_stack_count(token_bag->locations);
	hcc_atagen_run(w, token_bag, HCC_ATAGEN_RUN_MODE_PP_OPERAND);

	//
	// error if no tokens found
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS);
	}

	//
	// get the custom line number from the token
	int64_t custom_line;
	{
		HccATAToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx);
		if (token != HCC_ATA_TOKEN_LIT_SINT) {
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS);
		}
		HccConstantId constant_id = hcc_stack_get(token_bag->values, token_values_start_idx)->constant_id;
		HccConstant constant = hcc_constant_table_get(w->cu, constant_id);

		HCC_DEBUG_ASSERT(hcc_constant_as_sint(w->cu, constant, &custom_line), "internal error: expected to be a signed int");
		if (custom_line < 0 || custom_line > INT32_MAX) {
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_PP_LINE_MUST_BE_MORE_THAN_ZERO, INT32_MAX);
		}
	}

	//
	// get the custom path from the next token if there is one
	HccString custom_path = {0};
	if (tokens_start_idx + 1 < hcc_stack_count(token_bag->tokens)) {
		HccATAToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx + 1);
		if (token != HCC_ATA_TOKEN_STRING) {
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PP_LINE_OPERANDS);
		}
		HccStringId string_id = hcc_stack_get(token_bag->values, token_values_start_idx + 2)->string_id;
		custom_path = hcc_string_table_get(string_id);
	}

	//
	// error if too many operands
	if (tokens_start_idx + 2 < hcc_stack_count(token_bag->tokens)) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_TOO_MANY_PP_LINE_OPERANDS);
	}

	//
	// store the custom line and custom path in the code file we are currently parsing
	w->atagen.custom_line_dst = custom_line;
	w->atagen.custom_line_src = w->atagen.location.line_start;
	if (custom_path.data) {
		w->atagen.location.display_path = custom_path;
	}

	//
	// remove the added operand tokens that were
	// generated by the call to hcc_atagen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->values, token_values_start_idx);
	hcc_stack_resize(token_bag->locations, token_location_indices_start_idx);
}

void hcc_ppgen_parse_error(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	HccString message = hcc_string((char*)&w->atagen.code[w->atagen.location.code_end_idx], 0);
	hcc_atagen_consume_until_any_byte(w, "\n");
	message.size = (char*)&w->atagen.code[w->atagen.location.code_end_idx] - message.data;

	hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_PP_ERROR, (int)message.size, message.data);
}

void hcc_ppgen_parse_warning(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	HccString message = hcc_string((char*)&w->atagen.code[w->atagen.location.code_end_idx], 0);
	hcc_atagen_consume_until_any_byte(w, "\n");
	message.size = (char*)&w->atagen.code[w->atagen.location.code_end_idx] - message.data;

	//
	// make a copy of the location for this warning since we overrite this when we continue tokenizing
	HccLocation* location = hcc_atagen_make_location(w);
	location->display_line = hcc_atagen_display_line(w);

	hcc_warn_push(hcc_worker_task(w), HCC_WARN_CODE_PP_WARNING, location, NULL, (int)message.size, message.data);
}

void hcc_ppgen_parse_pragma(HccWorker* w) {
	hcc_atagen_consume_whitespace(w);
	w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
	w->atagen.location.column_start = w->atagen.location.column_end;

	//
	// check if this is STDC which is a thing that the spec states should exist
	if (
		w->atagen.code[w->atagen.location.code_end_idx + 0] == 'S' &&
		w->atagen.code[w->atagen.location.code_end_idx + 1] == 'T' &&
		w->atagen.code[w->atagen.location.code_end_idx + 2] == 'D' &&
		w->atagen.code[w->atagen.location.code_end_idx + 3] == 'C'
	) {
		HCC_ABORT("TODO: implement #pragma STDC support");
		return;
	}

	//
	// run the tokenizer to get the #pragma operands as a list of tokens
	HccATATokenBag* token_bag = &w->atagen.ast_file->token_bag;
	uint32_t tokens_start_idx = hcc_stack_count(token_bag->tokens);
	uint32_t token_values_start_idx = hcc_stack_count(token_bag->values);
	uint32_t token_location_indices_start_idx = hcc_stack_count(token_bag->locations);
	hcc_atagen_run(w, token_bag, HCC_ATAGEN_RUN_MODE_PP_OPERAND);

	//
	// if no tokens exist then we do nothing
	if (tokens_start_idx == hcc_stack_count(token_bag->tokens)) {
		return;
	}

	HccATAToken token = *hcc_stack_get(token_bag->tokens, tokens_start_idx);
	if (token != HCC_ATA_TOKEN_IDENT) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PP_PRAGMA_OPERAND);
	}

	uint32_t expected_tokens_count = hcc_stack_count(token_bag->tokens);
	HccStringId ident_string_id = hcc_stack_get(token_bag->values, token_values_start_idx)->string_id;
	switch (ident_string_id.idx_plus_one) {
		case HCC_STRING_ID_ONCE: {
			if (hcc_stack_count(w->atagen.paused_file_stack) == 0) {
				hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_PP_PRAGMA_OPERAND_USED_IN_MAIN_FILE);
			}

			HccCodeFile* code_file = w->atagen.location.code_file;
			HccStringId path_string_id;
			hcc_string_table_deduplicate(code_file->path_string.data, code_file->path_string.size, &path_string_id);
			hcc_ast_file_set_pragma_onced(w->atagen.ast_file, path_string_id);
			expected_tokens_count = tokens_start_idx + 1;
			break;
		};
	}

	if (expected_tokens_count != hcc_stack_count(token_bag->tokens)) {
		HccString ident_string = hcc_string_table_get(ident_string_id);
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_TOO_MANY_PP_PRAGMA_OPERANDS, (int)ident_string.size, ident_string.data);
	}

	//
	// remove the added operand tokens that were
	// generated by the call to hcc_atagen_run.
	hcc_stack_resize(token_bag->tokens, tokens_start_idx);
	hcc_stack_resize(token_bag->values, token_values_start_idx);
	hcc_stack_resize(token_bag->locations, token_location_indices_start_idx);
}

HccPPDirective hcc_ppgen_parse_directive_header(HccWorker* w) {
	hcc_atagen_advance_column(w, 1); // skip '#'
	hcc_atagen_consume_whitespace(w);

	char byte = w->atagen.code[w->atagen.location.code_end_idx];
	switch (byte) {
		case '\r':
		case '\n':
		case '\0':
			return HCC_PP_DIRECTIVE_COUNT;
	}

	HccString ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_INVALID_TOKEN_PREPROCESSOR_DIRECTIVE);
	hcc_atagen_advance_column(w, ident_string.size);

	HccPPDirective directive = hcc_string_to_enum_hashed_find(ident_string, hcc_pp_directive_hashes, HCC_PP_DIRECTIVE_COUNT);
	if (directive == HCC_PP_DIRECTIVE_COUNT) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PREPROCESSOR_DIRECTIVE, (int)ident_string.size, ident_string.data);
	}

	return directive;
}

void hcc_ppgen_parse_directive(HccWorker* w) {
	HccPPDirective directive = hcc_ppgen_parse_directive_header(w);
	if (directive == HCC_PP_DIRECTIVE_COUNT) {
		// we only found a '#' on this line, so return
		return;
	}

#if HCC_DEBUG_CODE_PREPROCESSOR
	uint32_t debug_indent_level = hcc_stack_count(w->atagen.ppgen.if_stack);
	uint32_t debug_line = w->atagen.location.line_end - 1;
	if (directive == HCC_PP_DIRECTIVE_ENDIF) {
		debug_indent_level -= 1;
	}
#endif

	bool is_skipping_code = false;
	bool is_skipping_until_endif = false;
	switch (directive) {
		case HCC_PP_DIRECTIVE_DEFINE:
			hcc_ppgen_parse_define(w);
			break;
		case HCC_PP_DIRECTIVE_UNDEF:
			hcc_ppgen_parse_undef(w);
			break;
		case HCC_PP_DIRECTIVE_INCLUDE:
			hcc_ppgen_parse_include(w);
			break;
		case HCC_PP_DIRECTIVE_LINE:
			hcc_ppgen_parse_line(w);
			break;
		case HCC_PP_DIRECTIVE_ERROR:
			hcc_ppgen_parse_error(w);
			break;
		case HCC_PP_DIRECTIVE_WARNING:
			hcc_ppgen_parse_warning(w);
			break;
		case HCC_PP_DIRECTIVE_PRAGMA:
			hcc_ppgen_parse_pragma(w);
			break;
		case HCC_PP_DIRECTIVE_IF: {
			hcc_ppgen_if_found_if(w, directive);
			is_skipping_code = !hcc_ppgen_parse_if(w);
			break;
		};
		case HCC_PP_DIRECTIVE_IFDEF:
		case HCC_PP_DIRECTIVE_IFNDEF: {
			hcc_ppgen_if_found_if(w, directive);
			is_skipping_code = hcc_ppgen_parse_ifdef(w, directive) != (directive == HCC_PP_DIRECTIVE_IFDEF);
			break;
		};
		case HCC_PP_DIRECTIVE_ENDIF:
			hcc_ppgen_if_ensure_one_is_open(w, directive);
			hcc_ppgen_if_found_endif(w);
			break;
		case HCC_PP_DIRECTIVE_ELSE:
		case HCC_PP_DIRECTIVE_ELIF:
		case HCC_PP_DIRECTIVE_ELIFDEF:
		case HCC_PP_DIRECTIVE_ELIFNDEF:
			hcc_ppgen_if_ensure_one_is_open(w, directive);
			hcc_ppgen_if_ensure_first_else(w, directive);
			if (directive == HCC_PP_DIRECTIVE_ELSE) {
				hcc_ppgen_if_found_else(w, directive);
			} else {
				hcc_ppgen_if_found_if_counterpart(w, directive);
			}

			//
			// we where just in a true block and processing the code as normal.
			// now we have come across some else directive, we just skip until the #endif.
			is_skipping_code = true;
			is_skipping_until_endif = true;
			break;
	}

#if HCC_DEBUG_CODE_PREPROCESSOR
	printf("+(%u)#%-7s at line %u\n", debug_indent_level, hcc_pp_directive_strings[directive], debug_line);
#endif

	if (is_skipping_code) {
		hcc_ppgen_skip_false_conditional(w, is_skipping_until_endif);
	}
}

void hcc_ppgen_skip_false_conditional(HccWorker* w, bool is_skipping_until_endif) {
	bool first_non_white_space_char = false;
	uint32_t nested_level = hcc_stack_count(w->atagen.ppgen.if_stack);
	HccCodeFile* code_file = w->atagen.location.code_file;
	bool is_inside_nested_comment = false;
	bool is_inside_single_line_comment = false;
	while (w->atagen.location.code_end_idx < w->atagen.code_size) {
		if (!(atomic_load(&code_file->flags) & HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS)) {
			//
			// TODO test a SIMD optimized version of this
			//
			while (w->atagen.location.code_end_idx < w->atagen.code_size) {
				char byte = w->atagen.code[w->atagen.location.code_end_idx];
				if (byte == '\n') break;
				if (byte == '#') break;
				if (byte == '/' && w->atagen.code[w->atagen.location.code_end_idx + 1] == '/') break;
				if (byte == '/' && w->atagen.code[w->atagen.location.code_end_idx + 1] == '*') break;
				if (byte == '*' && w->atagen.code[w->atagen.location.code_end_idx + 1] == '/') break;
				first_non_white_space_char = false;
				hcc_atagen_advance_column(w, 1);
			}
		} else {
			HccPPIfSpan* pp_if_span = hcc_ppgen_if_span_get(w, w->atagen.pp_if_span_id);
			if (is_skipping_until_endif) {
				HccPPIfSpan* first_pp_if_span = hcc_ppgen_if_span_get(w, pp_if_span->first_id);
				HccPPIfSpan* last_pp_if_span = hcc_ppgen_if_span_get(w, first_pp_if_span->last_id);
				w->atagen.location = last_pp_if_span->location;
				w->atagen.pp_if_span_id = first_pp_if_span->last_id - 1;
			} else {
				HccPPIfSpan* next_pp_if_span = hcc_ppgen_if_span_get(w, pp_if_span->next_id);
				w->atagen.location = next_pp_if_span->location;
				w->atagen.pp_if_span_id = pp_if_span->next_id - 1;
			}

			//
			// put cursor back to the start of the line
			w->atagen.location.code_end_idx = w->atagen.location.code_start_idx;
			w->atagen.location.column_start = 1;
			w->atagen.location.column_end = 1;
			first_non_white_space_char = true;
		}

		if (w->atagen.location.code_end_idx >= w->atagen.code_size) {
			return;
		}

		char byte = w->atagen.code[w->atagen.location.code_end_idx];
		switch (byte) {
			case '\n':
				is_inside_single_line_comment = false;
				hcc_atagen_advance_newline(w);
				w->atagen.location.line_start = w->atagen.location.line_end - 1;
				w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
				w->atagen.location.column_start = 1;
				first_non_white_space_char = true;
				break;
			case '#': {
				if (!first_non_white_space_char) {
					w->atagen.location.column_end += 1;
					if (!is_inside_single_line_comment && !is_inside_nested_comment) {
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_PP_DIRECTIVE_NOT_FIRST_ON_LINE);
					}
					hcc_atagen_advance_column(w, 1); // skip '#'
					break;
				}

				HccPPDirective directive = hcc_ppgen_parse_directive_header(w);
				if (directive == HCC_PP_DIRECTIVE_COUNT) {
					// we only found a '#' on this line, so continue;
					continue;
				}

				bool is_finished = false;
				switch (directive) {
					case HCC_PP_DIRECTIVE_DEFINE:
					case HCC_PP_DIRECTIVE_UNDEF:
					case HCC_PP_DIRECTIVE_INCLUDE:
					case HCC_PP_DIRECTIVE_LINE:
					case HCC_PP_DIRECTIVE_ERROR:
					case HCC_PP_DIRECTIVE_WARNING:
					case HCC_PP_DIRECTIVE_PRAGMA:
						//
						// consume until the new line and continue so the operands
						// of these defines do not mess with the skipping code.
						hcc_atagen_consume_until_any_byte(w, "\n");
						break;
					case HCC_PP_DIRECTIVE_IF:
					case HCC_PP_DIRECTIVE_IFDEF:
					case HCC_PP_DIRECTIVE_IFNDEF: {
						//
						// push any if on the stack so we can keep track of when reach
						// the original if block's #endif or #else
						//
						hcc_ppgen_if_found_if(w, directive);
						break;
					};
					case HCC_PP_DIRECTIVE_ENDIF:
						if (hcc_stack_count(w->atagen.ppgen.if_stack) == nested_level) {
							is_finished = true;
						}
						hcc_ppgen_if_found_endif(w);
						break;
					case HCC_PP_DIRECTIVE_ELSE:
					case HCC_PP_DIRECTIVE_ELIF:
					case HCC_PP_DIRECTIVE_ELIFDEF:
					case HCC_PP_DIRECTIVE_ELIFNDEF:
						hcc_ppgen_if_ensure_first_else(w, directive);
						if (directive == HCC_PP_DIRECTIVE_ELSE) {
							hcc_ppgen_if_found_else(w, directive);
						} else {
							hcc_ppgen_if_found_if_counterpart(w, directive);
						}

						if (!is_skipping_until_endif && hcc_stack_count(w->atagen.ppgen.if_stack) == nested_level) {
							switch (directive) {
								case HCC_PP_DIRECTIVE_ELSE:
									is_finished = true;
									break;
								case HCC_PP_DIRECTIVE_ELIF:
									is_finished = hcc_ppgen_parse_if(w);
									break;
								case HCC_PP_DIRECTIVE_ELIFDEF:
								case HCC_PP_DIRECTIVE_ELIFNDEF:
									is_finished = hcc_ppgen_parse_ifdef(w, directive) == (directive == HCC_PP_DIRECTIVE_ELIFDEF);
									break;
							}
						}
						break;
				}

#if HCC_DEBUG_CODE_PREPROCESSOR
				char* plus_or_minus = is_skipping_code ? "-" : "+";
				printf("%s(%u)#%-7s at line %u\n", plus_or_minus, debug_indent_level, hcc_pp_directive_strings[directive], debug_line);
#endif

				if (is_finished) {
					return;
				}

				first_non_white_space_char = false;
				break;
			};
			case '/': {
				hcc_atagen_advance_column(w, 1);
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx];
				switch (next_byte) {
					case '/':
						hcc_atagen_advance_column(w, 1);
						if (!is_inside_single_line_comment && !is_inside_nested_comment) {
							is_inside_single_line_comment = true;
						}
						break;
					case '*':
						hcc_atagen_advance_column(w, 1);
						if (!is_inside_single_line_comment && !is_inside_nested_comment) {
							is_inside_nested_comment = true;
						}
						break;
				}
				break;
			};
			case '*': {
				hcc_atagen_advance_column(w, 1);
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx];
				switch (next_byte) {
					case '/':
						hcc_atagen_advance_column(w, 1);
						is_inside_nested_comment = false;
						break;
				}
				break;
			};
		}
	}
}

void hcc_ppgen_copy_expand_predefined_macro(HccWorker* w, HccPPPredefinedMacro predefined_macro) {
	switch (predefined_macro) {
		case HCC_PP_PREDEFINED_MACRO___FILE__: {
			HccATAValue value;
			if (hcc_atagen_is_last_token_string(w->atagen.dst_token_bag)) {
				hcc_atagen_token_merge_append_string(w, w->atagen.dst_token_bag, hcc_string(w->atagen.location.code_file->path_string.data, w->atagen.location.code_file->path_string.size));
			} else {
				hcc_string_table_deduplicate(w->atagen.location.code_file->path_string.data, w->atagen.location.code_file->path_string.size, &value.string_id);
				hcc_atagen_token_add(w, HCC_ATA_TOKEN_STRING);
				hcc_atagen_token_value_add(w, value);
			}
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___LINE__: {
			HccBasic line_num = hcc_basic_from_sint(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, w->atagen.location.line_end - 1);
			HccATAValue token_value = {
				.constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, &line_num),
			};
			hcc_atagen_token_add(w, HCC_ATA_TOKEN_LIT_SINT);
			hcc_atagen_token_value_add(w, token_value);

			char buf[128];
			snprintf(buf, sizeof(buf), "%u", w->atagen.location.line_end - 1);
			hcc_string_table_deduplicate_c_string(buf, &token_value.string_id);
			hcc_atagen_token_value_add(w, token_value);
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___STDC__: {
			HccATAValue token_value = {
				.constant_id = hcc_constant_table_deduplicate_one(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT),
			};
			hcc_atagen_token_add(w, HCC_ATA_TOKEN_LIT_SINT);
			hcc_atagen_token_value_add(w, token_value);

			hcc_string_table_deduplicate_lit("1", &token_value.string_id);
			hcc_atagen_token_value_add(w, token_value);
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___STDC_VERSION__: {
			HccBasic line_num = hcc_basic_from_sint(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, 201112L);
			HccATAValue token_value = {
				.constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, &line_num),
			};
			hcc_atagen_token_add(w, HCC_ATA_TOKEN_LIT_SINT);
			hcc_atagen_token_value_add(w, token_value);

			hcc_string_table_deduplicate_lit("201112L", &token_value.string_id);
			hcc_atagen_token_value_add(w, token_value);
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___COUNTER__: {
			HccBasic counter = hcc_basic_from_sint(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, w->atagen.__counter__);
			HccATAValue token_value = {
				.constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, &counter),
			};
			hcc_atagen_token_add(w, HCC_ATA_TOKEN_LIT_SINT);
			hcc_atagen_token_value_add(w, token_value);

			char buf[128];
			snprintf(buf, sizeof(buf), "%u", w->atagen.__counter__);
			hcc_string_table_deduplicate_c_string(buf, &token_value.string_id);
			hcc_atagen_token_value_add(w, token_value);

			w->atagen.__counter__ += 1;
			break;
		};
		case HCC_PP_PREDEFINED_MACRO___HCC__:
		case HCC_PP_PREDEFINED_MACRO___HCC_GPU__:
		case HCC_PP_PREDEFINED_MACRO___HCC_X86_64__:
		case HCC_PP_PREDEFINED_MACRO___HCC_LINUX__:
		case HCC_PP_PREDEFINED_MACRO___HCC_WINDOWS__:
			return;
	}
}

void hcc_ppgen_copy_expand_macro_begin(HccWorker* w, HccPPMacro* macro, HccLocation* macro_callsite_location) {
	HccPPExpand args_expand;
	HccATATokenBag* args_src_bag = &w->atagen.ast_file->macro_token_bag;
	if (macro->is_function) {
		HCC_DEBUG_ASSERT(w->atagen.code[w->atagen.location.code_end_idx] == '(', "internal error: expected to be on a '(' but got '%w'", w->atagen.code[w->atagen.location.code_end_idx]);
		//
		// our arguments are currently in string form.
		// tokenize them before we call into the expand code.
		//
		args_expand.macro = NULL;
		args_expand.cursor.tokens_start_idx = hcc_stack_count(args_src_bag->tokens);
		args_expand.cursor.token_idx = hcc_stack_count(args_src_bag->tokens);
		args_expand.cursor.token_value_idx = hcc_stack_count(args_src_bag->values);

		hcc_atagen_run(w, args_src_bag, HCC_ATAGEN_RUN_MODE_PP_MACRO_ARGS);
		args_expand.cursor.tokens_end_idx = hcc_stack_count(args_src_bag->tokens);
	}

	HccATATokenBag* dst_bag = &w->atagen.ast_file->token_bag;
	HccATATokenBag* alt_dst_bag = &w->atagen.ast_file->macro_token_bag;
	hcc_ppgen_copy_expand_macro(w, macro, macro_callsite_location, macro_callsite_location, &args_expand, args_src_bag, dst_bag, alt_dst_bag, HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION);
}

bool hcc_ppgen_is_callable_macro(HccWorker* w, HccStringId ident_string_id, uint32_t* macro_idx_out) {
	uintptr_t found_idx = hcc_hash_table_find_idx(w->atagen.ppgen.macro_declarations, &ident_string_id.idx_plus_one);
	if (found_idx == UINTPTR_MAX) {
		return false;
	}

	uint32_t macro_idx = w->atagen.ppgen.macro_declarations[found_idx].macro_idx;
	for (uint32_t idx = 0; idx < hcc_stack_count(w->atagen.ppgen.expand_macro_idx_stack); idx += 1) {
		if (*hcc_stack_get(w->atagen.ppgen.expand_macro_idx_stack, idx) == macro_idx) {
			return false;
		}
	}

	if (macro_idx_out) {
		*macro_idx_out = macro_idx;
	}

	return true;
}

HccPPExpand* hcc_ppgen_expand_push_macro(HccWorker* w, HccPPMacro* macro) {
	HccPPExpand* expand = hcc_stack_push(w->atagen.ppgen.expand_stack);
	*hcc_stack_push(w->atagen.ppgen.expand_macro_idx_stack) = macro - w->atagen.ast_file->macros;
	expand->cursor = macro->token_cursor;
	expand->macro = macro;
	return expand;
}

HccPPExpand* hcc_ppgen_expand_push_macro_arg(HccWorker* w, uint32_t param_idx, uint32_t args_start_idx, HccLocation** callsite_location_out) {
	HccPPMacroArg* arg = hcc_stack_get(w->atagen.ppgen.macro_args_stack, args_start_idx + param_idx);
	if (callsite_location_out) *callsite_location_out = arg->callsite_location;

	HccPPExpand* expand = hcc_stack_push(w->atagen.ppgen.expand_stack);
	*hcc_stack_push(w->atagen.ppgen.expand_macro_idx_stack) = -1;
	expand->macro = NULL;
	expand->cursor = arg->cursor;
	return expand;
}

void hcc_ppgen_expand_pop(HccWorker* w, HccPPExpand* expected_expand) {
	HccPPExpand* popped_ptr = hcc_stack_get_last(w->atagen.ppgen.expand_stack);
	HCC_DEBUG_ASSERT(popped_ptr == expected_expand, "internal error: we expected to pop %p but it was actually %p", expected_expand, popped_ptr);
	hcc_stack_pop(w->atagen.ppgen.expand_stack);
	hcc_stack_pop(w->atagen.ppgen.expand_macro_idx_stack);
}

void hcc_ppgen_copy_expand_range(HccWorker* w, HccPPExpand* expand, HccATATokenBag* dst_bag, HccATATokenBag* src_bag, HccATATokenBag* alt_dst_bag, HccLocation* parent_or_child_location, HccLocation* grandparent_location, HccPPExpandFlags flags, HccPPMacro* expand_macro) {
	//
	// copy each token from src_bag into the dst_bag, while looking out for macros to expand into the dst_bag
	//
	while (expand->cursor.token_idx < expand->cursor.tokens_end_idx) {
		HccATAToken token = *hcc_stack_get(src_bag->tokens, expand->cursor.token_idx);
		if (token == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
			if ((flags & HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION) || *hcc_stack_get_last(dst_bag->tokens) == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
				expand->cursor.token_idx += 1;
				continue;
			}
		}

		HccLocation* token_location = *hcc_stack_get(src_bag->locations, expand->cursor.token_idx);
		bool is_preexpanded_macro_arg = HCC_PP_TOKEN_IS_PREEXPANDED_MACRO_ARG(token_location);
		token_location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(token_location);

		HccLocation* location = hcc_worker_alloc_location(w);
		{
			if ((flags & HCC_PP_EXPAND_FLAGS_IS_ARGS) || !is_preexpanded_macro_arg) {
				//
				// location needs a parent as we are either are:
				//     - copying the contents of argument tokens
				//     - copying a macro and are not on any of the preexpanded macro argument tokens
				//
				HccLocation* parent_location;
				HccLocation* child_location;
				HccPPMacro* macro;
				if ((flags & HCC_PP_EXPAND_FLAGS_IS_ARGS)) {
					// for expanded macro argument tokens:
					// the child is the macro parameter location for all of the argument tokens.
					// the parent is the original argument token location at the callsite
					bool is_same = parent_location == grandparent_location;
					parent_location = hcc_worker_alloc_location(w);
					*parent_location = *token_location;
					if (!is_same && grandparent_location) {
						hcc_ppgen_attach_to_most_parent(w, parent_location, grandparent_location);
					}
					if (!token_location->macro) {
						parent_location->macro = expand->macro;
					}
					child_location = parent_or_child_location;
					macro = expand_macro;
				} else {
					// for expanded macro tokens:
					// the child is the original macro token location
					// the parent is the whole macro span at the callsite
					parent_location = parent_or_child_location;
					child_location = token_location;
					macro = expand_macro;
				}

				*location = *child_location;
				location->parent_location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(parent_location);
				location->macro = macro;
			} else {
				//
				// we are on a argument that has been preexpanded before
				// so there is no need to setup any parent location or macro links
				*location = *token_location;
			}
		}

		if (token == HCC_ATA_TOKEN_IDENT) {
			HccStringId ident_string_id = hcc_stack_get(src_bag->values, expand->cursor.token_value_idx)->string_id;

			uint32_t macro_idx;
			if (hcc_ppgen_is_callable_macro(w, ident_string_id, &macro_idx)) {
				//
				// we found a callable macro, lets expand it into the dst_bag
				//
				HccPPMacro* macro = hcc_stack_get(w->atagen.ast_file->macros, macro_idx);
				bool can_expand = true;
				if (macro->is_function) {
					HccATAToken* next_token = hcc_stack_get_or_null(src_bag->tokens, expand->cursor.token_idx + 1);
					HccATAToken* next_next_token = hcc_stack_get_or_null(src_bag->tokens, expand->cursor.token_idx + 2);
					can_expand = next_token &&
						(*next_token == HCC_ATA_TOKEN_PARENTHESIS_OPEN ||
							(*next_token == HCC_ATA_TOKEN_MACRO_WHITESPACE &&
								next_next_token && *next_next_token == HCC_ATA_TOKEN_PARENTHESIS_OPEN));
				}

				if (can_expand) {
					expand->cursor.token_idx += 1; // skip the identifier token
					expand->cursor.token_value_idx += 1;

					HccLocation* callsite_location = location;
					HccPPExpandFlags expand_flags = flags;
					if (expand_flags & HCC_PP_EXPAND_FLAGS_IS_ARGS) {
						expand_flags |= HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS;
						expand_flags &= ~HCC_PP_EXPAND_FLAGS_IS_ARGS;
						callsite_location = location->parent_location;
					}
					hcc_ppgen_copy_expand_macro(w, macro, callsite_location, location, expand, src_bag, dst_bag, alt_dst_bag, expand_flags);
					continue;
				}
			}
		}

		if ((flags & HCC_PP_EXPAND_FLAGS_DEST_IS_ORIGINAL_LOCATION)) {
			HccATAToken bracket = token - HCC_ATA_TOKEN_BRACKET_START;
			if (bracket < HCC_ATA_TOKEN_BRACKET_COUNT) {
				if (bracket % 2 == 0) {
					hcc_atagen_bracket_open(w, token, location);
				} else {
					hcc_atagen_bracket_close(w, token, location);
				}
			}
		}

		//
		// copy the token/location/value
		//
		if (token == HCC_ATA_TOKEN_STRING && hcc_atagen_is_last_token_string(dst_bag)) {
			HccString append_string = hcc_string_table_get(hcc_stack_get(src_bag->values, expand->cursor.token_value_idx)->string_id);
			hcc_atagen_token_merge_append_string(w, dst_bag, append_string);
			expand->cursor.token_idx += 1;
			expand->cursor.token_value_idx += 1;
		} else {
			*hcc_stack_push(dst_bag->tokens) = token;

			if (flags & (HCC_PP_EXPAND_FLAGS_IS_ARGS | HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS)) {
				location = HCC_PP_TOKEN_SET_PREEXPANDED_MACRO_ARG(location);
			}
			*hcc_stack_push(dst_bag->locations) = location;

			expand->cursor.token_idx += 1;

			uint32_t num_values = hcc_ata_token_num_values(token);
			for (uint32_t value_idx = 0; value_idx < num_values; value_idx += 1) {
				*hcc_stack_push(dst_bag->values) =
					*hcc_stack_get(src_bag->values, expand->cursor.token_value_idx);
				expand->cursor.token_value_idx += 1;
			}
		}
	}
}

void hcc_ppgen_copy_expand_macro(HccWorker* w, HccPPMacro* macro, HccLocation* macro_callsite_location, HccLocation* parent_location, HccPPExpand* arg_expand, HccATATokenBag* args_src_bag, HccATATokenBag* dst_bag, HccATATokenBag* alt_dst_bag, HccPPExpandFlags flags) {
	uint32_t restore_to_tokens_count = hcc_stack_count(alt_dst_bag->tokens);
	uint32_t restore_to_token_values_count = hcc_stack_count(alt_dst_bag->values);
	uint32_t restore_to_expand_args_count = hcc_stack_count(w->atagen.ppgen.macro_args_stack);
	uint32_t args_start_idx = UINT32_MAX;
	HccATATokenBag* src_bag = &w->atagen.ast_file->macro_token_bag;

	if (macro->is_function) {
		args_start_idx = hcc_ppgen_process_macro_args(w, macro, arg_expand, args_src_bag, parent_location);
		HccLocation* final_location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(args_src_bag->locations, arg_expand->cursor.tokens_end_idx - 1));
		hcc_location_merge_apply(macro_callsite_location, final_location);
	}

	HccPPExpand* macro_expand;
	if (macro->is_function) {
		//
		// copy a contents of the macro and the expanded argument tokens into the alt_dst_bag
		// this is so that the expanded tokens do not go where the macro needs to be expanded in the dst_bag
		//
		uint32_t tokens_start_idx = hcc_stack_count(alt_dst_bag->tokens);
		uint32_t token_values_start_idx = hcc_stack_count(alt_dst_bag->values);
		HccATATokenCursor cursor = macro->token_cursor;
		while (cursor.token_idx < cursor.tokens_end_idx) {
			HccATAToken token = *hcc_stack_get(src_bag->tokens, cursor.token_idx);
			switch (token) {
				case HCC_ATA_TOKEN_MACRO_PARAM: {
					HccLocation* location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(src_bag->locations, cursor.token_idx));
					uint32_t param_idx = hcc_stack_get(src_bag->values, cursor.token_value_idx)->macro_param_idx;

					HccPPExpand* param_arg_expand = hcc_ppgen_expand_push_macro_arg(w, param_idx, args_start_idx, NULL);
					param_arg_expand->macro = arg_expand->macro;

					HccLocation* grandparent_location = (flags & HCC_PP_EXPAND_FLAGS_DEST_IS_ARGS)
						? parent_location
						: parent_location->parent_location;
					hcc_ppgen_copy_expand_range(w, param_arg_expand, alt_dst_bag, args_src_bag, dst_bag, location, grandparent_location, HCC_PP_EXPAND_FLAGS_IS_ARGS, macro);
					hcc_ppgen_expand_pop(w, param_arg_expand);

					cursor.token_idx += 1;
					cursor.token_value_idx += 1;
					continue;
				};

				case HCC_ATA_TOKEN_MACRO_STRINGIFY:
				case HCC_ATA_TOKEN_MACRO_STRINGIFY_WHITESPACE: {
					uint32_t param_idx = hcc_stack_get(src_bag->values, cursor.token_value_idx)->macro_param_idx;

					HccLocation* callsite_location;
					HccPPExpand* param_arg_expand = hcc_ppgen_expand_push_macro_arg(w, param_idx, args_start_idx, &callsite_location);

					HccStringId string_id = hcc_ata_token_bag_stringify_range(args_src_bag, &param_arg_expand->cursor, NULL, w->atagen.ppgen.stringify_buffer, hcc_stack_cap(w->atagen.ppgen.stringify_buffer), NULL);
					hcc_ppgen_expand_pop(w, param_arg_expand);

					if (token == HCC_ATA_TOKEN_STRING && hcc_atagen_is_last_token_string(alt_dst_bag)) {
						HccString append_string = hcc_string_table_get(string_id);
						hcc_atagen_token_merge_append_string(w, alt_dst_bag, append_string);
					} else {
						//
						// push on the location
						HccLocation* dst_location = hcc_worker_alloc_location(w);
						*dst_location = *HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(src_bag->locations, cursor.token_idx));
						dst_location->parent_location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(callsite_location);
						dst_location->macro = macro;

						*hcc_stack_push(alt_dst_bag->locations) = HCC_PP_TOKEN_SET_PREEXPANDED_MACRO_ARG(dst_location);
						*hcc_stack_push(alt_dst_bag->tokens) = HCC_ATA_TOKEN_STRING;
						hcc_stack_push(alt_dst_bag->values)->string_id = string_id;
					}

					cursor.token_idx += 1;
					cursor.token_value_idx += 1;
					continue;
				};

				case HCC_ATA_TOKEN_MACRO_CONCAT:
				case HCC_ATA_TOKEN_MACRO_CONCAT_WHITESPACE: {
					cursor.token_idx += 1;

					hcc_stack_clear(w->atagen.ppgen.stringify_buffer);

					HccLocation* before = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(src_bag->locations, cursor.token_idx));
					uint32_t stringify_buffer_size = 0;
					HccATAToken before_token = hcc_ata_token_bag_stringify_single_or_macro_param(w, src_bag, &cursor, args_start_idx, args_src_bag, false, w->atagen.ppgen.stringify_buffer, hcc_stack_cap(w->atagen.ppgen.stringify_buffer), &stringify_buffer_size);
					if (*hcc_stack_get(src_bag->tokens, cursor.token_idx) == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
						cursor.token_idx += 1;
					}

					HccLocation* after = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(src_bag->locations, cursor.token_idx));
					HccATAToken after_token = hcc_ata_token_bag_stringify_single_or_macro_param(w, src_bag, &cursor, args_start_idx, args_src_bag, true, w->atagen.ppgen.stringify_buffer + stringify_buffer_size, hcc_stack_cap(w->atagen.ppgen.stringify_buffer) - stringify_buffer_size, &stringify_buffer_size);
					HccLocation* dst_location = hcc_worker_alloc_location(w);
					*dst_location = *before;
					hcc_location_merge_apply(dst_location, after);
					dst_location->parent_location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(parent_location);
					dst_location->macro = macro;

					HccCodeFile* code_file = &w->atagen.ppgen.concat_buffer_code_file;
					hcc_stack_clear(code_file->line_code_start_indices);
					*hcc_stack_push(code_file->line_code_start_indices) = 0;
					*hcc_stack_push(code_file->line_code_start_indices) = 0;
					code_file->code = hcc_string(w->atagen.ppgen.stringify_buffer, stringify_buffer_size);

					hcc_atagen_paused_file_push(w);
					hcc_atagen_location_setup_new_file(w, code_file);
					w->atagen.we_are_mutator_of_code_file = true;
					w->atagen.location.parent_location = dst_location;

					if (before_token != HCC_ATA_TOKEN_COUNT && after_token != HCC_ATA_TOKEN_COUNT) {
						if (!hcc_ata_token_concat_is_okay(before_token, after_token)) {
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_PP_CONCAT_OPERANDS, hcc_ata_token_strings[before_token], hcc_ata_token_strings[after_token]);
						}
					}

					uint32_t token_location_start_idx = hcc_stack_count(alt_dst_bag->locations);
					hcc_atagen_run(w, alt_dst_bag, HCC_ATAGEN_RUN_MODE_PP_CONCAT);

					//
					// hcc_atagen_run puts the locations in the token bag but we don't care about them.
					// what we really want is the location that spans the concatination operands and links back to their parents.
					// so just link to it as a stable pointer.
					for (uint32_t idx = token_location_start_idx; idx < hcc_stack_count(alt_dst_bag->locations); idx += 1) {
						*hcc_stack_get(alt_dst_bag->locations, idx) = dst_location;
					}
					continue;
				};
				case HCC_ATA_TOKEN_MACRO_WHITESPACE:
					if (*hcc_stack_get_last(alt_dst_bag->tokens) == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
						cursor.token_idx += 1;
						continue;
					}
					break;
			}

			//
			// copy the token/location/value
			//
			if (token == HCC_ATA_TOKEN_STRING && hcc_atagen_is_last_token_string(alt_dst_bag)) {
				HccString append_string = hcc_string_table_get(hcc_stack_get(src_bag->values, cursor.token_value_idx)->string_id);
				hcc_atagen_token_merge_append_string(w, alt_dst_bag, append_string);
				cursor.token_idx += 1;
				cursor.token_value_idx += 1;
			} else {
				*hcc_stack_push(alt_dst_bag->tokens) = token;
				*hcc_stack_push(alt_dst_bag->locations) = *hcc_stack_get(src_bag->locations, cursor.token_idx);
				cursor.token_idx += 1;

				uint32_t num_values = hcc_ata_token_num_values(token);
				for (uint32_t value_idx = 0; value_idx < num_values; value_idx += 1) {
					*hcc_stack_push(alt_dst_bag->values) =
						*hcc_stack_get(src_bag->values, cursor.token_value_idx);
					cursor.token_value_idx += 1;
				}
			}
		}

		//
		// setup the copy expand after so that this same macro can be expanded as a macro argument to itself
		macro_expand = hcc_ppgen_expand_push_macro(w, macro);
		macro_expand->cursor.token_idx = tokens_start_idx;
		macro_expand->cursor.tokens_end_idx = hcc_stack_count(alt_dst_bag->tokens);
		macro_expand->cursor.token_value_idx = token_values_start_idx;
		src_bag = alt_dst_bag;
	} else {
		//
		// just use the macros' tokens directly since this is a non function macro
		//
		macro_expand = hcc_ppgen_expand_push_macro(w, macro);
	}


	//
	// expand the macro tokens into the dst_bag
	//
	hcc_ppgen_copy_expand_range(w, macro_expand, dst_bag, src_bag, alt_dst_bag, parent_location, NULL, flags, macro);
	hcc_ppgen_expand_pop(w, macro_expand);

	//
	// remove all of the temporary tokens from the alt_dst_bag
	// that where generated for the expansion of the macro.
	//
	hcc_stack_resize(alt_dst_bag->tokens, restore_to_tokens_count);
	hcc_stack_resize(alt_dst_bag->locations, restore_to_tokens_count);
	hcc_stack_resize(alt_dst_bag->values, restore_to_token_values_count);
	hcc_stack_resize(w->atagen.ppgen.macro_args_stack, restore_to_expand_args_count);
}

uint32_t hcc_ppgen_process_macro_args(HccWorker* w, HccPPMacro* macro, HccPPExpand* expand, HccATATokenBag* src_bag, HccLocation* parent_location) {
	HCC_DEBUG_ASSERT(
		*hcc_stack_get(src_bag->tokens, expand->cursor.token_idx) == HCC_ATA_TOKEN_PARENTHESIS_OPEN ||
			(*hcc_stack_get(src_bag->tokens, expand->cursor.token_idx) == HCC_ATA_TOKEN_MACRO_WHITESPACE &&
				*hcc_stack_get(src_bag->tokens, expand->cursor.token_idx + 1) == HCC_ATA_TOKEN_PARENTHESIS_OPEN),
		"internal error: expected to be on a '(' for the macro function arguments"
	);
	expand->cursor.token_idx += *hcc_stack_get(src_bag->tokens, expand->cursor.token_idx) == HCC_ATA_TOKEN_MACRO_WHITESPACE ? 2 : 1;

	//
	// scan the src_bag tokens and keep a record of the token ranges for each macro argument in
	// the w->atagen.ppgen.macro_args_stack array
	//
	uint32_t args_start_idx = hcc_stack_count(w->atagen.ppgen.macro_args_stack);
	HccPPMacroArg* arg = hcc_ppgen_push_macro_arg(w, expand, src_bag, parent_location);
	uint32_t nested_parenthesis = 0;
	bool reached_va_args = macro->has_va_args && macro->params_count == 1;
	while (1) {
		HccATAToken token = *hcc_stack_get(src_bag->tokens, expand->cursor.token_idx);
		HccLocation* location = *hcc_stack_get(src_bag->locations, expand->cursor.token_idx);

		switch (token) {
			case HCC_ATA_TOKEN_COMMA:
				if (nested_parenthesis == 0 && !reached_va_args) {
					//
					// we found a ',' outside of nested_parenthesis,
					// so lets finalize the current argument and start the next one.
					expand->cursor.token_idx += 1;
					hcc_ppgen_finalize_macro_arg(arg, expand, src_bag);

					//
					// start the next argument
					//
					arg = hcc_ppgen_push_macro_arg(w, expand, src_bag, parent_location);

					uint32_t args_count = hcc_stack_count(w->atagen.ppgen.macro_args_stack) - args_start_idx;
					if (macro->has_va_args && args_count == macro->params_count) {
						//
						// we have reached the variable argument, so just group all
						// of the arguments that follow into this last argument.
						reached_va_args = true;
					}
					continue;
				}
				break;

			case HCC_ATA_TOKEN_PARENTHESIS_OPEN:
				nested_parenthesis += 1;
				expand->cursor.token_idx += 1;
				continue;

			case HCC_ATA_TOKEN_PARENTHESIS_CLOSE:
				expand->cursor.token_idx += 1;
				if (nested_parenthesis == 0) {
					goto BREAK;
				}
				nested_parenthesis -= 1;
				continue;
		}

		expand->cursor.token_idx += 1;
		expand->cursor.token_value_idx += hcc_ata_token_num_values(token);
	}
BREAK: {}
	hcc_ppgen_finalize_macro_arg(arg, expand, src_bag);

	uint32_t args_count = hcc_stack_count(w->atagen.ppgen.macro_args_stack) - args_start_idx;
	if (args_count == 1 && arg->cursor.tokens_start_idx == arg->cursor.tokens_end_idx) {
		args_count = 0;
	}

	if (args_count < macro->params_count) {
		// this is not a error that breaks a rule in the C spec.
		// TODO: see if we want ot have a compiler option to enable this.
		// hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_NOT_ENOUGH_MACRO_ARGUMENTS, macro->params_count, args_count);

		uint32_t missing_count = macro->params_count - args_count;
		HccPPMacroArg* args = hcc_stack_push_many(w->atagen.ppgen.macro_args_stack, missing_count);
		HCC_ZERO_ELMT_MANY(args, missing_count);
	} else if (args_count > macro->params_count) {
		hcc_atagen_bail_error_2(w, HCC_ERROR_CODE_TOO_MANY_MACRO_ARGUMENTS, parent_location, macro->location, macro->params_count, args_count);
	}
	return args_start_idx;
}

HccPPMacroArg* hcc_ppgen_push_macro_arg(HccWorker* w, HccPPExpand* expand, HccATATokenBag* src_bag, HccLocation* parent_location) {
	HccPPMacroArg* arg = hcc_stack_push(w->atagen.ppgen.macro_args_stack);
	arg->cursor.tokens_start_idx = expand->cursor.token_idx;
	arg->cursor.token_idx = expand->cursor.token_idx;
	arg->cursor.token_value_idx = expand->cursor.token_value_idx;
	arg->callsite_location = hcc_worker_alloc_location(w);
	HccLocation* src_location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(src_bag->locations, expand->cursor.token_idx));
	*arg->callsite_location = *src_location;
	bool is_same = parent_location == src_location;
	if (!is_same && parent_location) {
		hcc_ppgen_attach_to_most_parent(w, arg->callsite_location, parent_location);
	}
	arg->callsite_location->macro = expand->macro;
	return arg;
}

void hcc_ppgen_finalize_macro_arg(HccPPMacroArg* arg, HccPPExpand* expand, HccATATokenBag* src_bag) {
	arg->cursor.tokens_end_idx = expand->cursor.token_idx - 1;
	HccLocation* location = HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(*hcc_stack_get(src_bag->locations, expand->cursor.token_idx - 2));
	hcc_location_merge_apply(arg->callsite_location, location);
	if (*hcc_stack_get(src_bag->tokens, arg->cursor.tokens_start_idx) == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
		arg->cursor.tokens_start_idx += 1;
		arg->cursor.token_idx += 1;
	}

	if (*hcc_stack_get(src_bag->tokens, arg->cursor.tokens_end_idx - 1) == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
		arg->cursor.tokens_end_idx -= 1;
	}
}

void hcc_ppgen_attach_to_most_parent(HccWorker* w, HccLocation* location, HccLocation* parent_location) {
	HccLocation* a = location;
	while (a) {
		HccLocation* b = parent_location;
		while (b) {
			if (a == b) {
				return;
			}

			b = b->parent_location;
		}
		a = a->parent_location;
	}

	HccLocation* prev_pl = location;
	HccLocation* pl = location;
	while (pl->parent_location) {
		prev_pl = pl;
		pl = pl->parent_location;
	}

	if (prev_pl->parent_location) {
		pl = hcc_worker_alloc_location(w);
		*pl = *prev_pl->parent_location;
		prev_pl->parent_location = pl;
	}
	pl->parent_location = parent_location;
}

// ===========================================
//
//
// ATA Generator
//
//
// ===========================================

void hcc_atagen_init(HccWorker* w, HccATAGenSetup* setup) {
	hcc_ppgen_init(w, &setup->ppgen);
	w->atagen.paused_file_stack = hcc_stack_init(HccATAPausedFile, HCC_ALLOC_TAG_ATAGEN_PAUSED_FILE_STACK, setup->paused_file_stack_grow_count, setup->paused_file_stack_reserve_cap);
	w->atagen.open_bracket_stack = hcc_stack_init(HccATAOpenBracket, HCC_ALLOC_TAG_ATAGEN_OPEN_BRACKET_STACK, setup->open_bracket_stack_grow_count, setup->open_bracket_stack_reserve_cap);
}

void hcc_atagen_deinit(HccWorker* w) {
	hcc_ppgen_deinit(w);
	hcc_stack_deinit(w->atagen.paused_file_stack);
	hcc_stack_deinit(w->atagen.open_bracket_stack);
}

void hcc_atagen_reset(HccWorker* w) {
	hcc_ppgen_reset(w);
	hcc_stack_clear(w->atagen.paused_file_stack);
	hcc_stack_clear(w->atagen.open_bracket_stack);
}

void hcc_atagen_generate(HccWorker* w) {
	HccTaskInputLocation* il = w->job.arg;

	hcc_ast_add_file(w->cu, il->file_path, &w->atagen.ast_file);
	HccResult result = hcc_code_file_find_or_insert(il->file_path, &w->atagen.location.code_file);
	if (!HCC_IS_SUCCESS(result)) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_FAILED_TO_OPEN_FILE_FOR_READ, il->file_path.data);
	}
	w->atagen.we_are_mutator_of_code_file = result.code == HCC_SUCCESS_IS_NEW;

	hcc_atagen_location_setup_new_file(w, w->atagen.location.code_file);

	hcc_atagen_run(w, &w->atagen.ast_file->token_bag, HCC_ATAGEN_RUN_MODE_CODE);

	if (w->atagen.we_are_mutator_of_code_file) {
		atomic_fetch_or(&w->atagen.location.code_file->flags, HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS);
	}
}

HccLocation* hcc_atagen_make_location(HccWorker* w) {
	HccLocation* l = hcc_worker_alloc_location(w);
	*l = w->atagen.location;
	l->display_line = hcc_atagen_display_line(w);
	return l;
}

void hcc_atagen_advance_column(HccWorker* w, uint32_t by) {
	w->atagen.location.column_end += by;
	w->atagen.location.code_end_idx += by;
}

void hcc_atagen_advance_newline(HccWorker* w) {
	w->atagen.location.line_end += 1;
	w->atagen.location.column_start = 1;
	w->atagen.location.column_end = 1;
	w->atagen.location.code_end_idx += 1;

	if (w->atagen.we_are_mutator_of_code_file) {
		uint32_t* dst = hcc_stack_push(w->atagen.location.code_file->line_code_start_indices);
		*dst = w->atagen.location.code_end_idx;
	}
}

uint32_t hcc_atagen_display_line(HccWorker* w) {
	uint32_t line_num = w->atagen.location.line_start;
	return w->atagen.custom_line_dst ? w->atagen.custom_line_dst + (line_num - w->atagen.custom_line_src) : line_num;
}

void hcc_atagen_token_add(HccWorker* w, HccATAToken token) {
	hcc_ata_token_bag_push_token(w->atagen.dst_token_bag, token, hcc_atagen_make_location(w));
}

void hcc_atagen_token_value_add(HccWorker* w, HccATAValue value) {
	hcc_ata_token_bag_push_value(w->atagen.dst_token_bag, value);
}

void hcc_atagen_count_extra_newlines(HccWorker* w) {
	if (!w->atagen.we_are_mutator_of_code_file) {
		return;
	}

	HccLocation* location = &w->atagen.location;
	HccString code = location->code_file->code;
	uint32_t idx = hcc_stack_count(w->atagen.paused_file_stack);
	while (1) {
		uint32_t lines_count = 3;
		while (location->code_end_idx < code.size) {
			char byte = code.data[location->code_end_idx];
			location->code_end_idx += 1;
			if (byte == '\n') {
				uint32_t* dst = hcc_stack_push(location->code_file->line_code_start_indices);
				*dst = location->code_end_idx;

				lines_count -= 1;
				if (lines_count == 0) {
					break;
				}
			}
		}

		if (idx == 0) {
			break;
		}

		idx -= 1;
		location = &hcc_stack_get(w->atagen.paused_file_stack, idx)->location;
		code = location->code_file->code;
	}
}

_Noreturn void hcc_atagen_bail_error_1(HccWorker* w, HccErrorCode error_code, ...) {
	hcc_atagen_count_extra_newlines(w);

	va_list va_args;
	va_start(va_args, error_code);
	w->atagen.location.display_line = hcc_atagen_display_line(w);
	hcc_error_pushv(hcc_worker_task(w), error_code, &w->atagen.location, NULL, va_args);
	va_end(va_args);

	//
	// TODO see if you can recover from an error at the atagen stage.
	// I don't think it is worth it as i don't want to pump out any incorrect errors.
	hcc_bail(HCC_ERROR_MESSAGES, 0);
}

_Noreturn void hcc_atagen_bail_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* token_location, HccLocation* other_token_location, ...) {
	hcc_atagen_count_extra_newlines(w);

	va_list va_args;
	va_start(va_args, other_token_location);
	w->atagen.location.display_line = hcc_atagen_display_line(w);
	hcc_error_pushv(hcc_worker_task(w), error_code, token_location, other_token_location, va_args);
	va_end(va_args);

	//
	// TODO see if you can recover from an error at the atagen stage.
	// I don't think it is worth it as i don't want to pump out any incorrect errors.
	hcc_bail(HCC_ERROR_MESSAGES, 0);
}

void hcc_atagen_paused_file_push(HccWorker* w) {
	HccATAPausedFile* paused_file = hcc_stack_push(w->atagen.paused_file_stack);
	paused_file->we_are_mutator_of_code_file = w->atagen.we_are_mutator_of_code_file;
	paused_file->location = w->atagen.location;
	paused_file->if_stack_count = hcc_stack_count(w->atagen.ppgen.if_stack);
	paused_file->pp_if_span_id = w->atagen.pp_if_span_id;
}

void hcc_atagen_paused_file_pop(HccWorker* w) {
	if (w->atagen.we_are_mutator_of_code_file) {
		atomic_fetch_or(&w->atagen.location.code_file->flags, HCC_CODE_FILE_FLAGS_COMPLETED_MUTATOR_PASS);
	}

	HccATAPausedFile* paused_file = hcc_stack_get_last(w->atagen.paused_file_stack);
	if (paused_file->if_stack_count != hcc_stack_count(w->atagen.ppgen.if_stack)) {
		HccPPGenIf* pp_if = hcc_stack_get_last(w->atagen.ppgen.if_stack);
		w->atagen.location = pp_if->location;
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_PP_IF_UNTERMINATED, hcc_pp_directive_strings[pp_if->directive]);
	}

	w->atagen.we_are_mutator_of_code_file = paused_file->we_are_mutator_of_code_file;
	w->atagen.location = paused_file->location;
	w->atagen.code = paused_file->location.code_file->code.data;
	w->atagen.code_size = paused_file->location.code_file->code.size;
	w->atagen.pp_if_span_id = paused_file->pp_if_span_id;
	hcc_stack_pop(w->atagen.paused_file_stack);
}

void hcc_atagen_location_setup_new_file(HccWorker* w, HccCodeFile* code_file) {
	HccLocation* location = &w->atagen.location;
	location->code_file = code_file;
	location->parent_location = NULL;
	location->macro = NULL;
	location->code_start_idx = 0;
	location->code_end_idx = 0;
	location->line_start = 1;
	location->line_end = 2;
	location->column_start = 1;
	location->column_end = 1;

	w->atagen.code = code_file->code.data;
	w->atagen.code_size = code_file->code.size;
	w->atagen.pp_if_span_id = 0;
}

bool hcc_atagen_is_last_token_string(HccATATokenBag* bag) {
	return hcc_stack_count(bag->tokens) > 0 && *hcc_stack_get_last(bag->tokens) == HCC_ATA_TOKEN_STRING;
}

void hcc_atagen_token_merge_append_string(HccWorker* w, HccATATokenBag* bag, HccString append_string) {
	HCC_DEBUG_ASSERT(hcc_stack_count(bag->tokens) > 0 && *hcc_stack_get_last(bag->tokens) == HCC_ATA_TOKEN_STRING, "expected string token");

	HccString prepend_string = hcc_string_table_get(hcc_stack_get_last(bag->values)->string_id);

	uint32_t stringify_buffer_start_idx = hcc_stack_count(w->string_buffer);
	hcc_stack_push_string(w->string_buffer, prepend_string);
	hcc_stack_pop(w->string_buffer); // remove \0
	hcc_stack_push_string(w->string_buffer, append_string);
	HccStringId string_id;
	hcc_string_table_deduplicate(&w->string_buffer[stringify_buffer_start_idx], hcc_stack_count(w->string_buffer) - stringify_buffer_start_idx, &string_id);
	hcc_stack_resize(w->string_buffer, stringify_buffer_start_idx);

	hcc_stack_get_last(bag->values)->string_id = string_id;
}

bool hcc_atagen_consume_backslash(HccWorker* w) {
	hcc_atagen_advance_column(w, 1);
	char byte = w->atagen.code[w->atagen.location.code_end_idx];
	bool found_newline = false;
	if (byte == '\r') {
		hcc_atagen_advance_column(w, 1);
		byte = w->atagen.code[w->atagen.location.code_end_idx];
		found_newline = true;
	}
	if (byte == '\n'){
		hcc_atagen_advance_column(w, 1);
		byte = w->atagen.code[w->atagen.location.code_end_idx];
		found_newline = true;
	}
	if (found_newline) {
		w->atagen.location.line_end += 1;
		w->atagen.location.column_start = 1;
		w->atagen.location.column_end = 1;
		if (w->atagen.we_are_mutator_of_code_file) {
			uint32_t* dst = hcc_stack_push(w->atagen.location.code_file->line_code_start_indices);
			*dst = w->atagen.location.code_end_idx;
		}
	}
	return found_newline;
}

void hcc_atagen_consume_whitespace(HccWorker* w) {
	while (w->atagen.location.code_end_idx < w->atagen.code_size) {
		char byte = w->atagen.code[w->atagen.location.code_end_idx];
		if (byte != ' ' && byte != '\t') {
			char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
			if (byte != '\\' || !hcc_atagen_consume_backslash(w)) {
				break;
			}
		} else {
			hcc_atagen_advance_column(w, 1);
		}
	}
}

void hcc_atagen_consume_whitespace_and_newlines(HccWorker* w) {
	while (w->atagen.location.code_end_idx < w->atagen.code_size) {
		char byte = w->atagen.code[w->atagen.location.code_end_idx];
		if (byte != ' ' && byte != '\t' && byte != '\r' && byte != '\n' && byte != '\\') {
			break;
		} else {
			if (byte == '\n') {
				hcc_atagen_advance_newline(w);
			} else {
				hcc_atagen_advance_column(w, 1);
			}
		}
	}
}


void hcc_atagen_consume_until_any_byte(HccWorker* w, char* terminator_bytes) {
	while (w->atagen.location.code_end_idx < w->atagen.code_size) {
		char byte = w->atagen.code[w->atagen.location.code_end_idx];
		if (byte == '\\') {
			hcc_atagen_consume_backslash(w);
			continue;
		}

		if (strchr(terminator_bytes, byte)) {
			break;
		}

		if (byte == '\r' || byte == '\n') {
			hcc_atagen_advance_newline(w);
		} else {
			hcc_atagen_advance_column(w, 1);
		}
	}
}

HccString hcc_atagen_parse_ident_from_string(HccWorker* w, HccString string, HccErrorCode error_code) {
	char byte = string.data[0];
	if (!hcc_ascii_is_alpha(byte) && byte != '_') {
		HCC_DEBUG_ASSERT(error_code != HCC_ERROR_CODE_NONE, "internal error: expected no error to happen when parsing this identifier");
		w->atagen.location.column_end += 1;
		hcc_atagen_bail_error_1(w, error_code, byte);
	}

	HccString ident_string = hcc_string(string.data, 0);
	while (ident_string.size < string.size) {
		char ident_byte = ident_string.data[ident_string.size];
		if (
			!hcc_ascii_is_alpha(ident_byte) &&
			!hcc_ascii_is_digit(ident_byte) &&
			ident_byte != '_'
		) {
			break;
		}
		ident_string.size += 1;
	}

	return ident_string;
}

HccString hcc_atagen_parse_ident(HccWorker* w, HccErrorCode error_code) {
	HccString code = hcc_string((char*)&w->atagen.code[w->atagen.location.code_end_idx], w->atagen.code_size - w->atagen.location.code_end_idx);
	return hcc_atagen_parse_ident_from_string(w, code, error_code);
}

uint32_t hcc_atagen_parse_num_literals(HccWorker* w, char* num_string, uint32_t token_size, uint32_t remaining_size, HccATAToken* token_mut) {
	HccATAToken token = *token_mut;
	token_size -= 1; // move back onto the character
	while (token_size < remaining_size) {
		char ch = num_string[token_size];
		token_size += 1;
		switch (ch) {
			case 'u':
			case 'U':
				switch (token) {
					case HCC_ATA_TOKEN_LIT_SINT: token = HCC_ATA_TOKEN_LIT_UINT; break;
					case HCC_ATA_TOKEN_LIT_SLONG: token = HCC_ATA_TOKEN_LIT_ULONG; break;
					case HCC_ATA_TOKEN_LIT_SLONGLONG: token = HCC_ATA_TOKEN_LIT_ULONGLONG; break;
					case HCC_ATA_TOKEN_LIT_UINT:
					case HCC_ATA_TOKEN_LIT_ULONG:
					case HCC_ATA_TOKEN_LIT_ULONGLONG:
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_U_SUFFIX_ALREADY_USED);
						break;
					case HCC_ATA_TOKEN_LIT_FLOAT:
					case HCC_ATA_TOKEN_LIT_DOUBLE:
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_U_SUFFIX_ON_FLOAT);
						break;
				}
				break;
			case 'l':
			case 'L':
				switch (token) {
					case HCC_ATA_TOKEN_LIT_FLOAT:
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_L_SUFFIX_ON_FLOAT);
					case HCC_ATA_TOKEN_LIT_DOUBLE:
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_LONG_DOUBLE_IS_UNSUPPORTED);
					case HCC_ATA_TOKEN_LIT_SINT:
						token = HCC_ATA_TOKEN_LIT_SLONG;
						break;
					case HCC_ATA_TOKEN_LIT_SLONG:
						token = HCC_ATA_TOKEN_LIT_SLONGLONG;
						break;
					case HCC_ATA_TOKEN_LIT_UINT:
						token = HCC_ATA_TOKEN_LIT_ULONG;
						break;
					case HCC_ATA_TOKEN_LIT_ULONG:
						token = HCC_ATA_TOKEN_LIT_ULONGLONG;
						break;
				}
				break;
			case 'f':
			case 'F':
				if (token != HCC_ATA_TOKEN_LIT_DOUBLE) {
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_FLOAT_SUFFIX_MUST_FOLLOW_DECIMAL_PLACE);
				}
				token = HCC_ATA_TOKEN_LIT_FLOAT;
				break;
			default:
				token_size -= 1;
				goto END;
		}
	}

END:
	*token_mut = token;
	return token_size;
}

uint32_t hcc_atagen_parse_num(HccWorker* w, HccATAToken* token_out) {
	char* num_string = (char*)&w->atagen.code[w->atagen.location.code_end_idx];
	uint32_t remaining_size = w->atagen.code_size - w->atagen.location.code_end_idx;
	uint32_t token_size = 0;

	//
	// parse the radix prefix if there is a 0x or 0
	HccATAToken token = HCC_ATA_TOKEN_LIT_SINT;
	uint8_t radix = 10;
	if (num_string[0] == '0') {
		switch (num_string[1]) {
			case 'x':
			case 'X':
				radix = 16;
				token_size += 2;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				radix = 8;
				token_size += 2;
				break;
		}
	}

	uint64_t u64 = 0;
	double f64 = 0.0;
	double pow_10 = 10.0;
	while (token_size < remaining_size) {
		char digit = num_string[token_size];
		token_size += 1;

		switch (digit) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			{
				if (radix == 8 && digit > 7) {
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_OCTAL_DIGIT);
				}
				uint32_t int_digit = digit - '0';
				switch (token) {
					case HCC_ATA_TOKEN_LIT_SINT:
					case HCC_ATA_TOKEN_LIT_SLONG:
					case HCC_ATA_TOKEN_LIT_SLONGLONG:
					case HCC_ATA_TOKEN_LIT_UINT:
					case HCC_ATA_TOKEN_LIT_ULONG:
					case HCC_ATA_TOKEN_LIT_ULONGLONG:
						if (
							!hcc_i64_checked_mul(u64, radix, &u64)        ||
							!hcc_u64_checked_add(u64, int_digit, &u64)
						) {
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MAX_UINT_OVERFLOW);
						}
						break;
					case HCC_ATA_TOKEN_LIT_FLOAT:
					case HCC_ATA_TOKEN_LIT_DOUBLE:
						f64 += (double)(int_digit) / pow_10;
						if (isinf(f64)) {
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MAX_FLOAT_OVERFLOW);
						}
						pow_10 *= 10.0;
						break;
				}
				break;
			};
			case 'u':
			case 'U':
			case 'l':
			case 'L':
				token_size = hcc_atagen_parse_num_literals(w, num_string, token_size, remaining_size, &token);
				break;

			case '.':
				if (token == HCC_ATA_TOKEN_LIT_DOUBLE) {
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_FLOAT_HAS_DOUBLE_FULL_STOP);
				}
				if (radix != 10) {
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_FLOAT_MUST_BE_DECIMAL);
				}

				token = HCC_ATA_TOKEN_LIT_DOUBLE;
				f64 = (double)u64;
				if ((uint64_t)f64 != u64) {
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MAX_FLOAT_OVERFLOW);
				}
				break;

			default: {
				if (radix == 16 && ((digit >= 'a' && digit <= 'f') || (digit >= 'A' && digit <= 'F'))) {
					uint32_t int_digit = 10 + (digit >= 'a' ? (digit - 'a') : (digit - 'A'));
					HCC_DEBUG_ASSERT(token != HCC_ATA_TOKEN_LIT_FLOAT && token != HCC_ATA_TOKEN_LIT_DOUBLE, "internal error: expected to be dealing with integer literals here");
					if (
						!hcc_i64_checked_mul(u64, radix, &u64)        ||
						!hcc_u64_checked_add(u64, int_digit, &u64)
					) {
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MAX_UINT_OVERFLOW);
					}
					break;
				} else if (digit == 'f' || digit == 'F') {
					token_size = hcc_atagen_parse_num_literals(w, num_string, token_size, remaining_size, &token);
				} else if ((digit >= 'a' && digit <= 'z') || (digit >= 'A' && digit <= 'Z')) {
					switch (token) {
						case HCC_ATA_TOKEN_LIT_SINT:
						case HCC_ATA_TOKEN_LIT_SLONG:
						case HCC_ATA_TOKEN_LIT_SLONGLONG:
						case HCC_ATA_TOKEN_LIT_UINT:
						case HCC_ATA_TOKEN_LIT_ULONG:
						case HCC_ATA_TOKEN_LIT_ULONGLONG:
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_INTEGER_LITERALS);
						case HCC_ATA_TOKEN_LIT_FLOAT:
						case HCC_ATA_TOKEN_LIT_DOUBLE:
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_FLOAT_LITERALS);
					}
				} else {
					token_size -= 1;
					goto NUM_END;
				}
				break;
			};
		}
	}
NUM_END: {}

	//
	// perform literal type upgrades if they exceed their current type
	HccCU* cu = w->cu;
	switch (token) {
		case HCC_ATA_TOKEN_LIT_UINT:
			if (u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_UINT]) {
				break;
			}
			token = HCC_ATA_TOKEN_LIT_ULONG;
			hcc_fallthrough;
		case HCC_ATA_TOKEN_LIT_ULONG:
			if (u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_ULONG]) {
				break;
			}
			token = HCC_ATA_TOKEN_LIT_ULONGLONG;
			break;
		case HCC_ATA_TOKEN_LIT_SINT:
			if (u64 > (uint64_t)INT64_MAX) {
				hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MAX_SINT_OVERFLOW);
			}
			if (u64 > cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_SINT]) {
				if (radix != 10 && u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_UINT]) {
					token = HCC_ATA_TOKEN_LIT_UINT;
				} else if (u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_ULONG]) {
					token = HCC_ATA_TOKEN_LIT_SLONG;
				} else if (radix != 10 && u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_ULONG]) {
					token = HCC_ATA_TOKEN_LIT_ULONG;
				} else if (u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_SLONGLONG]) {
					token = HCC_ATA_TOKEN_LIT_SLONGLONG;
				} else if (radix != 10 && u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_ULONGLONG]) {
					token = HCC_ATA_TOKEN_LIT_ULONGLONG;
				} else {
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MAX_SINT_OVERFLOW_DECIMAL);
				}
			}
			break;
		case HCC_ATA_TOKEN_LIT_SLONG:
			if (u64 > cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_ULONG]) {
				if (radix != 10 && u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_ULONG]) {
					token = HCC_ATA_TOKEN_LIT_ULONG;
				} else if (u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_SLONGLONG]) {
					token = HCC_ATA_TOKEN_LIT_SLONGLONG;
				} else if (radix != 10 && u64 <= cu->dtt.basic_type_int_maxes[HCC_AST_BASIC_DATA_TYPE_ULONGLONG]) {
					token = HCC_ATA_TOKEN_LIT_ULONGLONG;
				} else {
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MAX_SINT_OVERFLOW_DECIMAL);
				}
			}
			break;
	}

	if (token_out == NULL) {
		return token_size;
	}

	HccASTBasicDataType basic_data_type;
	switch (token) {
		case HCC_ATA_TOKEN_LIT_SINT: basic_data_type = HCC_AST_BASIC_DATA_TYPE_SINT; break;
		case HCC_ATA_TOKEN_LIT_SLONG: basic_data_type = HCC_AST_BASIC_DATA_TYPE_SLONG; break;
		case HCC_ATA_TOKEN_LIT_SLONGLONG: basic_data_type = HCC_AST_BASIC_DATA_TYPE_SLONGLONG; break;
		case HCC_ATA_TOKEN_LIT_UINT: basic_data_type = HCC_AST_BASIC_DATA_TYPE_UINT; break;
		case HCC_ATA_TOKEN_LIT_ULONG: basic_data_type = HCC_AST_BASIC_DATA_TYPE_ULONG; break;
		case HCC_ATA_TOKEN_LIT_ULONGLONG: basic_data_type = HCC_AST_BASIC_DATA_TYPE_ULONGLONG; break;
		case HCC_ATA_TOKEN_LIT_FLOAT: basic_data_type = HCC_AST_BASIC_DATA_TYPE_FLOAT; break;
		case HCC_ATA_TOKEN_LIT_DOUBLE: basic_data_type = HCC_AST_BASIC_DATA_TYPE_DOUBLE; break;
	}

	HccDataType data_type = HCC_DATA_TYPE(AST_BASIC, basic_data_type);
	HccBasic basic;
	switch (token) {
		case HCC_ATA_TOKEN_LIT_SINT:
		case HCC_ATA_TOKEN_LIT_SLONG:
		case HCC_ATA_TOKEN_LIT_SLONGLONG: basic = hcc_basic_from_sint(w->cu, data_type, u64); break;
		case HCC_ATA_TOKEN_LIT_UINT:
		case HCC_ATA_TOKEN_LIT_ULONG:
		case HCC_ATA_TOKEN_LIT_ULONGLONG: basic = hcc_basic_from_uint(w->cu, data_type, u64); break;
		case HCC_ATA_TOKEN_LIT_FLOAT:
		case HCC_ATA_TOKEN_LIT_DOUBLE: basic = hcc_basic_from_float(w->cu, data_type, f64); break;
	}

	HccATAValue token_value;
	token_value.constant_id = hcc_constant_table_deduplicate_basic(w->cu, data_type, &basic);
	hcc_atagen_token_value_add(w, token_value);

	//
	// push on an addition string id of the literial so we can get the exact string representation.
	// this is needed for macro argument stringify and macro concatination.
	// TODO: we should probably just append these to the end of an array so it'll be faster but consume more RAM
	hcc_string_table_deduplicate(num_string, token_size, &token_value.string_id);
	hcc_atagen_token_value_add(w, token_value);

	*token_out = token;
	return token_size;
}

void hcc_atagen_parse_string(HccWorker* w, char terminator_byte, bool ignore_escape_sequences_except_double_quotes) {
	w->atagen.location.code_end_idx += 1;
	w->atagen.location.column_end += 1;

	uint32_t stringify_buffer_start_idx = hcc_stack_count(w->string_buffer);
	bool is_pp = w->atagen.run_mode == HCC_ATAGEN_RUN_MODE_PP_INCLUDE_OPERAND || ignore_escape_sequences_except_double_quotes;
	if (is_pp) {
		bool ended_with_terminator = false;
		while (w->atagen.location.code_end_idx < w->atagen.code_size) {
			char byte = w->atagen.code[w->atagen.location.code_end_idx];
			w->atagen.location.column_end += 1;
			w->atagen.location.code_end_idx += 1;

			if (byte == '\\') {
				byte = w->atagen.code[w->atagen.location.code_end_idx];
				switch (byte) {
					case '\"':
						if (ignore_escape_sequences_except_double_quotes) {
							hcc_stack_push_char(w->string_buffer, '\\');
						}
						w->atagen.location.column_end += 1;
						w->atagen.location.code_end_idx += 1;
						hcc_stack_push_char(w->string_buffer, '"');
						break;
					default:
						if (ignore_escape_sequences_except_double_quotes) {
							hcc_stack_push_char(w->string_buffer, '\\');
						} else {
							hcc_stack_push_char(w->string_buffer, '/'); // convert \ to / for windows paths
						}
						break;
				}
			} else if (byte == '\r' || byte == '\n') {
				break;
			} else if (byte == terminator_byte) {
				ended_with_terminator = true;
				break;
			} else {
				hcc_stack_push_char(w->string_buffer, byte);
			}
		}

		if (!ended_with_terminator) {
			w->atagen.location.column_end += 1;
			w->atagen.location.code_end_idx -= 1;
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_UNCLOSED_STRING_LITERAL, terminator_byte);
		}

		if (ignore_escape_sequences_except_double_quotes) {
			return;
		}
	} else {
		bool ended_with_terminator = false;
		while (w->atagen.location.code_end_idx < w->atagen.code_size) {
			char byte = w->atagen.code[w->atagen.location.code_end_idx];
			w->atagen.location.column_end += 1;
			w->atagen.location.code_end_idx += 1;

			if (byte == '\\') {
				byte = w->atagen.code[w->atagen.location.code_end_idx];
				switch (byte) {
					case 'r':
						hcc_stack_push_char(w->string_buffer, '\r');
						w->atagen.location.column_end += 1;
						w->atagen.location.code_end_idx += 1;
						break;
					case 'n':
						hcc_stack_push_char(w->string_buffer, '\n');
						w->atagen.location.column_end += 1;
						w->atagen.location.code_end_idx += 1;
						break;
					case '\\':
					case '"':
					case '\'':
						hcc_stack_push_char(w->string_buffer, byte);
						w->atagen.location.column_end += 1;
						w->atagen.location.code_end_idx += 1;
						break;
					default:
						break;
				}
			} else if (byte == '\r' || byte == '\n') {
				break;
			} else if (byte == terminator_byte) {
				ended_with_terminator = true;
				break;
			} else {
				hcc_stack_push_char(w->string_buffer, byte);
			}
		}

		if (!ended_with_terminator) {
			w->atagen.location.column_end += 1;
			w->atagen.location.code_end_idx -= 1;
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_UNCLOSED_STRING_LITERAL, terminator_byte);
		}
	}

	hcc_stack_push_char(w->string_buffer, '\0');

	uint32_t string_size = hcc_stack_count(w->string_buffer) - stringify_buffer_start_idx;

	if (!is_pp && hcc_atagen_is_last_token_string(w->atagen.dst_token_bag)) {
		hcc_atagen_token_merge_append_string(w, w->atagen.dst_token_bag, hcc_string(hcc_stack_get(w->string_buffer, stringify_buffer_start_idx), string_size));
	} else {
		HccATAValue token_value;
		hcc_string_table_deduplicate(hcc_stack_get(w->string_buffer, stringify_buffer_start_idx), string_size, &token_value.string_id);
		hcc_atagen_token_add(w, terminator_byte == '>' ? HCC_ATA_TOKEN_INCLUDE_PATH_SYSTEM : HCC_ATA_TOKEN_STRING);
		hcc_atagen_token_value_add(w, token_value);
	}

	hcc_stack_resize(w->string_buffer, stringify_buffer_start_idx);
}

uint32_t hcc_atagen_find_macro_param(HccWorker* w, HccStringId ident_string_id) {
	uint32_t param_idx;
	for (param_idx = 0; param_idx < w->atagen.macro_params_count; param_idx += 1) {
		if (w->atagen.macro_param_string_ids[param_idx].idx_plus_one == ident_string_id.idx_plus_one) {
			break;
		}
	}

	return param_idx;
}

void hcc_atagen_consume_hash_for_define_replacement_list(HccWorker* w) {
	if (w->atagen.code[w->atagen.location.code_end_idx + 1] == '#') {
		if (w->atagen.macro_tokens_start_idx == hcc_stack_count(w->atagen.ast_file->macro_token_bag.tokens)) {
			hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_MACRO_STARTS_WITH_CONCAT);
		}

		HccATAToken temp_token = *hcc_stack_get_last(w->atagen.ast_file->macro_token_bag.tokens);
		HccLocation* temp_token_location = *hcc_stack_get_last(w->atagen.ast_file->macro_token_bag.locations);
		bool has_left_whitespace = false;
		if (temp_token == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
			hcc_stack_pop(w->atagen.ast_file->macro_token_bag.tokens);
			hcc_stack_pop(w->atagen.ast_file->macro_token_bag.locations);
			temp_token = *hcc_stack_get_last(w->atagen.ast_file->macro_token_bag.tokens);
			temp_token_location = *hcc_stack_get_last(w->atagen.ast_file->macro_token_bag.locations);
			has_left_whitespace = true;
		}

		hcc_stack_pop(w->atagen.ast_file->macro_token_bag.tokens);
		hcc_stack_pop(w->atagen.ast_file->macro_token_bag.locations);

		hcc_atagen_advance_column(w, 2); // skip the '##'
		hcc_atagen_token_add(w, has_left_whitespace ? HCC_ATA_TOKEN_MACRO_CONCAT_WHITESPACE : HCC_ATA_TOKEN_MACRO_CONCAT);

		*hcc_stack_push(w->atagen.ast_file->macro_token_bag.tokens) = temp_token;
		*hcc_stack_push(w->atagen.ast_file->macro_token_bag.locations) = temp_token_location;
	} else {
		if (w->atagen.macro_is_function) {
			hcc_atagen_advance_column(w, 1); // skip the '#'

			uint32_t before_ident_start_idx = w->atagen.location.code_end_idx;
			hcc_atagen_consume_whitespace(w);
			bool whitespace_after_hash = before_ident_start_idx != w->atagen.location.code_end_idx;
			HccString ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_STRINGIFY_MUST_BE_MACRO_PARAM);
			HccStringId ident_string_id;
			hcc_string_table_deduplicate((char*)ident_string.data, ident_string.size, &ident_string_id);

			uint32_t param_idx;
			if (ident_string_id.idx_plus_one == HCC_STRING_ID___VA_ARGS__) {
				if (!w->atagen.macro_has_va_arg) {
					w->atagen.location.column_end += ident_string.size;
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_USE_OF_VA_ARGS);
				}
				param_idx = w->atagen.macro_params_count - 1;
			} else {
				param_idx = hcc_atagen_find_macro_param(w, ident_string_id);
				if (param_idx == w->atagen.macro_params_count) {
					w->atagen.location.column_end += ident_string.size;
					hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_STRINGIFY_MUST_BE_MACRO_PARAM);
				}
			}

			hcc_atagen_advance_column(w, ident_string.size);
			hcc_atagen_token_add(w, whitespace_after_hash ? HCC_ATA_TOKEN_MACRO_STRINGIFY_WHITESPACE : HCC_ATA_TOKEN_MACRO_STRINGIFY);
			hcc_atagen_token_value_add(w, ((HccATAValue) { .macro_param_idx = param_idx }));
		} else {
			hcc_atagen_advance_column(w, 1); // skip the '#'
			hcc_atagen_token_add(w, HCC_ATA_TOKEN_HASH);
		}
	}
}

bool hcc_atagen_is_first_non_whitespace_on_line(HccWorker* w) {
	for (uint32_t idx = w->atagen.location.code_end_idx; idx-- > 0; ) {
		switch (w->atagen.code[idx]) {
			case '\r':
			case '\n':
				return true;
			case ' ':
			case '\t':
				break;
			default:
				return false;
		}
	}

	return true;
}

void hcc_atagen_bracket_open(HccWorker* w, HccATAToken token, HccLocation* location) {
	HccATAOpenBracket* ob = hcc_stack_push(w->atagen.open_bracket_stack);
	HccATAToken close_token = token + 1; // the close token is defined right away after the open in the HccATAToken enum
	ob->close_token = close_token;
	ob->open_token_location = location;
}

void hcc_atagen_bracket_close(HccWorker* w, HccATAToken token, HccLocation* location) {
	if (hcc_stack_count(w->atagen.open_bracket_stack) == 0) {
		hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_NO_BRACKETS_OPEN, hcc_ata_token_strings[token]);
	}

	HccATAOpenBracket* ob = hcc_stack_get_last(w->atagen.open_bracket_stack);
	hcc_stack_pop(w->atagen.open_bracket_stack);
	if (ob->close_token != token) {
		hcc_atagen_advance_column(w, 1);
		hcc_atagen_bail_error_2(w, HCC_ERROR_CODE_INVALID_CLOSE_BRACKET_PAIR, location, ob->open_token_location, hcc_ata_token_strings[ob->close_token], hcc_ata_token_strings[token]);
	}
}

void hcc_atagen_run(HccWorker* w, HccATATokenBag* dst_token_bag, HccATAGenRunMode run_mode) {
	HccATAGenRunMode old_run_mode = w->atagen.run_mode;
	HccATATokenBag* old_dst_token_bag = w->atagen.dst_token_bag;
	w->atagen.run_mode = run_mode;
	w->atagen.dst_token_bag = dst_token_bag;

	uint32_t macro_args_nested_parenthesis_count = 0;
	while (1) {
		//
		// if we have reached the end of this file, then return
		// to the parent file or end the token generation.
		if (w->atagen.location.code_end_idx >= w->atagen.code_size) {
			if (hcc_stack_count(w->atagen.paused_file_stack) == 0) {
				goto RETURN;
			}

			hcc_atagen_paused_file_pop(w);
			if (run_mode == HCC_ATAGEN_RUN_MODE_PP_CONCAT) {
				goto RETURN;
			}
			continue;
		}

		char byte = w->atagen.code[w->atagen.location.code_end_idx];

		w->atagen.location.code_start_idx = w->atagen.location.code_end_idx;
		w->atagen.location.line_start += w->atagen.location.line_end - w->atagen.location.line_start - 1;
		w->atagen.location.column_start = w->atagen.location.column_end;

		HccATAToken token = HCC_ATA_TOKEN_COUNT;
		HccATAToken close_token;
		uint32_t token_size = 1;
		switch (byte) {
			case ' ':
			case '\t':
				hcc_atagen_consume_whitespace(w);
				switch (run_mode) {
					case HCC_ATAGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST:
					case HCC_ATAGEN_RUN_MODE_PP_MACRO_ARGS:
					case HCC_ATAGEN_RUN_MODE_PP_CONCAT:
						if (*hcc_stack_get_last(dst_token_bag->tokens) != HCC_ATA_TOKEN_MACRO_WHITESPACE) {
							hcc_atagen_token_add(w, HCC_ATA_TOKEN_MACRO_WHITESPACE);
						}
						break;
				}
				continue;
			case '\r':
				w->atagen.location.code_start_idx += 1;
				w->atagen.location.code_end_idx += 1;
				continue;
			case '\n':
				switch (run_mode) {
					case HCC_ATAGEN_RUN_MODE_PP_INCLUDE_OPERAND:
					case HCC_ATAGEN_RUN_MODE_PP_IF_OPERAND:
					case HCC_ATAGEN_RUN_MODE_PP_OPERAND:
					case HCC_ATAGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST:
						goto RETURN;
					case HCC_ATAGEN_RUN_MODE_PP_MACRO_ARGS:
					case HCC_ATAGEN_RUN_MODE_PP_CONCAT:
						if (*hcc_stack_get_last(dst_token_bag->tokens) != HCC_ATA_TOKEN_MACRO_WHITESPACE) {
							hcc_atagen_token_add(w, HCC_ATA_TOKEN_MACRO_WHITESPACE);
						}
						break;
				}

				hcc_atagen_advance_newline(w);
				continue;
			case '.': token = HCC_ATA_TOKEN_FULL_STOP; break;
			case ',': token = HCC_ATA_TOKEN_COMMA; break;
			case ';': token = HCC_ATA_TOKEN_SEMICOLON; break;
			case ':': token = HCC_ATA_TOKEN_COLON; break;
			case '~': token = HCC_ATA_TOKEN_TILDE; break;
			case '?': token = HCC_ATA_TOKEN_QUESTION_MARK; break;
			case '+': {
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_ADD_ASSIGN;
				} else if (next_byte == '+') {
					token_size = 2;
					token = HCC_ATA_TOKEN_INCREMENT;
				} else {
					token = HCC_ATA_TOKEN_PLUS;
				}
				break;
			};
			case '-': {
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_SUBTRACT_ASSIGN;
				} else if (next_byte == '-') {
					token_size = 2;
					token = HCC_ATA_TOKEN_DECREMENT;
				} else if (next_byte == '>') {
					token_size = 2;
					token = HCC_ATA_TOKEN_ARROW_RIGHT;
				} else {
					token = HCC_ATA_TOKEN_MINUS;
				}
				break;
			};
			case '/': {
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
				if (next_byte == '/') {
					w->atagen.location.code_end_idx += 2;
					while (w->atagen.location.code_end_idx < w->atagen.code_size) {
						char b = w->atagen.code[w->atagen.location.code_end_idx];
						if (b == '\n') {
							break;
						}
						w->atagen.location.code_end_idx += 1;
					}

					token_size = w->atagen.location.code_end_idx - w->atagen.location.code_start_idx;
					w->atagen.location.column_start += token_size;
					w->atagen.location.column_end += token_size;
					continue;
				} else if (next_byte == '*') {
					hcc_atagen_advance_column(w, 2);
					while (w->atagen.location.code_end_idx < w->atagen.code_size) {
						char b = w->atagen.code[w->atagen.location.code_end_idx];
						if (b == '\n') {
							hcc_atagen_advance_newline(w);
						} else {
							hcc_atagen_advance_column(w, 1);
							if (b == '*') {
								b = w->atagen.code[w->atagen.location.code_end_idx];
								if (b == '/') { // no need to check in bounds see _HCC_TOKENIZER_LOOK_HEAD_SIZE
									hcc_atagen_advance_column(w, 1);
									break;
								}
							}
						}
					}

					w->atagen.location.column_start = w->atagen.location.column_end;
					continue;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_DIVIDE_ASSIGN;
				} else {
					token = HCC_ATA_TOKEN_FORWARD_SLASH;
				}
				break;
			};
			case '*':
				if (w->atagen.code[w->atagen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_MULTIPLY_ASSIGN;
				} else {
					token = HCC_ATA_TOKEN_ASTERISK;
				}
				break;
			case '%':
				if (w->atagen.code[w->atagen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_MODULO_ASSIGN;
				} else {
					token = HCC_ATA_TOKEN_PERCENT;
				}
				break;
			case '&': {
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
				if (next_byte == '&') {
					token_size = 2;
					token = HCC_ATA_TOKEN_LOGICAL_AND;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_BIT_AND_ASSIGN;
				} else {
					token = HCC_ATA_TOKEN_AMPERSAND;
				}
				break;
			};
			case '|': {
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
				if (next_byte == '|') {
					token_size = 2;
					token = HCC_ATA_TOKEN_LOGICAL_OR;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_BIT_OR_ASSIGN;
				} else {
					token = HCC_ATA_TOKEN_PIPE;
				}
				break;
			};
			case '^':
				if (w->atagen.code[w->atagen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_BIT_XOR_ASSIGN;
				} else {
					token = HCC_ATA_TOKEN_CARET;
				}
				break;
			case '!':
				if (w->atagen.code[w->atagen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_LOGICAL_NOT_EQUAL;
				} else {
					token = HCC_ATA_TOKEN_EXCLAMATION_MARK;
				}
				break;
			case '=':
				if (w->atagen.code[w->atagen.location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_LOGICAL_EQUAL;
				} else {
					token = HCC_ATA_TOKEN_EQUAL;
				}
				break;
			case '<': {
				if (run_mode == HCC_ATAGEN_RUN_MODE_PP_INCLUDE_OPERAND) {
					hcc_atagen_parse_string(w, '>', false);
					continue;
				}
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_LESS_THAN_OR_EQUAL;
				} else if (next_byte == '<') {
					if (w->atagen.code[w->atagen.location.code_end_idx + 2] == '=') {
						token_size = 3;
						token = HCC_ATA_TOKEN_BIT_SHIFT_LEFT_ASSIGN;
					} else {
						token_size = 2;
						token = HCC_ATA_TOKEN_BIT_SHIFT_LEFT;
					}
				} else {
					token = HCC_ATA_TOKEN_LESS_THAN;
				}
				break;
			};
			case '>': {
				char next_byte = w->atagen.code[w->atagen.location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HCC_ATA_TOKEN_GREATER_THAN_OR_EQUAL;
				} else if (next_byte == '>') {
					if (w->atagen.code[w->atagen.location.code_end_idx + 2] == '=') {
						token_size = 3;
						token = HCC_ATA_TOKEN_BIT_SHIFT_RIGHT_ASSIGN;
					} else {
						token_size = 2;
						token = HCC_ATA_TOKEN_BIT_SHIFT_RIGHT;
					}
				} else {
					token = HCC_ATA_TOKEN_GREATER_THAN;
				}
				break;
			};
			case '(':
				macro_args_nested_parenthesis_count += 1;
				token = HCC_ATA_TOKEN_PARENTHESIS_OPEN;
				goto OPEN_BRACKET;
			case '{': token = HCC_ATA_TOKEN_CURLY_OPEN; goto OPEN_BRACKET;
			case '[': token = HCC_ATA_TOKEN_SQUARE_OPEN; goto OPEN_BRACKET;
OPEN_BRACKET: {}
				if (run_mode == HCC_ATAGEN_RUN_MODE_CODE) {
					hcc_atagen_advance_column(w, token_size);
					hcc_atagen_token_add(w, token);
					HccLocation* location = *hcc_stack_get_last(w->atagen.dst_token_bag->locations);
					hcc_atagen_bracket_open(w, token, location);
					continue;
				}
				break;

			case ')':
				macro_args_nested_parenthesis_count -= 1;
				token = HCC_ATA_TOKEN_PARENTHESIS_CLOSE;
				if (run_mode == HCC_ATAGEN_RUN_MODE_PP_MACRO_ARGS && macro_args_nested_parenthesis_count == 0) {
					hcc_atagen_advance_column(w, 1);
					hcc_atagen_token_add(w, token);
					goto RETURN;
				}
				goto CLOSE_BRACKET;
			case '}': token = HCC_ATA_TOKEN_CURLY_CLOSE; goto CLOSE_BRACKET;
			case ']': token = HCC_ATA_TOKEN_SQUARE_CLOSE; goto CLOSE_BRACKET;
CLOSE_BRACKET:
				if (run_mode == HCC_ATAGEN_RUN_MODE_CODE) {
					hcc_atagen_bracket_close(w, token, &w->atagen.location);
				}
				break;

			case '"':
				hcc_atagen_parse_string(w, '"', false);
				continue;

			case '\\':
				if (!hcc_atagen_consume_backslash(w)) {
					hcc_atagen_token_add(w, HCC_ATA_TOKEN_BACK_SLASH);
				}
				continue;

			case '#':
				switch (run_mode) {
					case HCC_ATAGEN_RUN_MODE_PP_INCLUDE_OPERAND:
					case HCC_ATAGEN_RUN_MODE_PP_IF_OPERAND:
					case HCC_ATAGEN_RUN_MODE_PP_OPERAND:
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_TOKEN_HASH_IN_PP_OPERAND);
					case HCC_ATAGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST:
						hcc_atagen_consume_hash_for_define_replacement_list(w);
						continue;
					case HCC_ATAGEN_RUN_MODE_PP_CONCAT:
						if (w->atagen.code[w->atagen.location.code_end_idx + 1] == '#') {
							hcc_atagen_advance_column(w, 2); // skip the '##'
							hcc_atagen_token_add(w, HCC_ATA_TOKEN_DOUBLE_HASH);
							continue;
						} else {
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_TOKEN, '#');
						}
						break;
					case HCC_ATAGEN_RUN_MODE_PP_MACRO_ARGS:
						hcc_atagen_advance_column(w, 1);
						hcc_atagen_token_add(w, HCC_ATA_TOKEN_HASH);
						continue;
					case HCC_ATAGEN_RUN_MODE_CODE:
						if (hcc_atagen_is_first_non_whitespace_on_line(w)) {
							hcc_ppgen_parse_directive(w);
							continue;
						} else {
							hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_PP_DIRECTIVE_NOT_FIRST_ON_LINE);
						}
						break;
				}
				break;

			default: {
				if ('0' <= byte && byte <= '9') {
					token_size = hcc_atagen_parse_num(w, &token);
					break;
				}

				HccString ident_string = hcc_atagen_parse_ident(w, HCC_ERROR_CODE_INVALID_TOKEN);
				hcc_atagen_advance_column(w, ident_string.size);

				HccStringId ident_string_id;
				hcc_string_table_deduplicate((char*)ident_string.data, ident_string.size, &ident_string_id);
				if (run_mode == HCC_ATAGEN_RUN_MODE_PP_IF_OPERAND && ident_string_id.idx_plus_one == HCC_STRING_ID_DEFINED) {
					hcc_ppgen_parse_defined(w);
					continue;
				}

				if (ident_string_id.idx_plus_one == HCC_STRING_ID___VA_ARGS__) {
					if (run_mode != HCC_ATAGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST || !w->atagen.macro_has_va_arg) {
						w->atagen.location.column_end += ident_string.size;
						hcc_atagen_bail_error_1(w, HCC_ERROR_CODE_INVALID_USE_OF_VA_ARGS);
					}

					hcc_atagen_token_add(w, HCC_ATA_TOKEN_MACRO_PARAM);
					hcc_atagen_token_value_add(w, ((HccATAValue){ .macro_param_idx = w->atagen.macro_params_count - 1 }));
					continue;
				}

				if (run_mode == HCC_ATAGEN_RUN_MODE_PP_DEFINE_REPLACEMENT_LIST) {
					uint32_t param_idx = hcc_atagen_find_macro_param(w, ident_string_id);
					if (param_idx < w->atagen.macro_params_count) {
						hcc_atagen_token_add(w, HCC_ATA_TOKEN_MACRO_PARAM);
						hcc_atagen_token_value_add(w, ((HccATAValue){ .macro_param_idx = param_idx }));
						continue;
					}
				}

				if (run_mode == HCC_ATAGEN_RUN_MODE_CODE || run_mode == HCC_ATAGEN_RUN_MODE_PP_OPERAND || run_mode == HCC_ATAGEN_RUN_MODE_PP_IF_OPERAND || run_mode == HCC_ATAGEN_RUN_MODE_PP_INCLUDE_OPERAND) {
					uintptr_t found_idx = hcc_hash_table_find_idx(w->atagen.ppgen.macro_declarations, &ident_string_id);
					if (found_idx != UINTPTR_MAX) {
						if (HCC_STRING_ID_PREDEFINED_MACROS_START <= ident_string_id.idx_plus_one && ident_string_id.idx_plus_one < HCC_STRING_ID_PREDEFINED_MACROS_END) {
							HccPPPredefinedMacro m = ident_string_id.idx_plus_one - HCC_STRING_ID_PREDEFINED_MACROS_START;
							hcc_ppgen_copy_expand_predefined_macro(w, m);
							continue;
						} else {
							uint32_t macro_idx = w->atagen.ppgen.macro_declarations[found_idx].macro_idx;
							HccPPMacro* macro = hcc_stack_get(w->atagen.ast_file->macros, macro_idx);
							bool can_expand = true;

							HccLocation macro_callsite_location = w->atagen.location;
							if (macro->is_function) {
								HccLocation backup_location = w->atagen.location;
								uint32_t backup_lines_count = hcc_stack_count(w->atagen.location.code_file->line_code_start_indices);

								hcc_atagen_consume_whitespace_and_newlines(w);
								if (w->atagen.code[w->atagen.location.code_end_idx] != '(') {
									//
									// the identifier is a macro function but we don't actually call it.
									// so return back and just process this as a regular identifier.
									w->atagen.location = backup_location;
									if (w->atagen.we_are_mutator_of_code_file) {
										hcc_stack_resize(w->atagen.location.code_file->line_code_start_indices, backup_lines_count);
									}
									can_expand = false;
								}
							}

							if (can_expand) {
								HccLocation* l = hcc_worker_alloc_location(w);
								*l = macro_callsite_location;
								hcc_ppgen_copy_expand_macro_begin(w, macro, l);
								continue;
							}
						}
					}
				}

				if (HCC_STRING_ID_KEYWORDS_START <= ident_string_id.idx_plus_one && ident_string_id.idx_plus_one < HCC_STRING_ID_KEYWORDS_END) {
					token = HCC_ATA_TOKEN_KEYWORDS_START + (ident_string_id.idx_plus_one - HCC_STRING_ID_KEYWORDS_START);
				} else {
					token = HCC_ATA_TOKEN_IDENT;
					HccATAValue token_value;
					token_value.string_id = ident_string_id;
					hcc_atagen_token_value_add(w, token_value);
				}

				hcc_atagen_token_add(w, token);
				continue;
			};
		}

		hcc_atagen_advance_column(w, token_size);
		hcc_atagen_token_add(w, token);
	}

RETURN: {}
	if (run_mode == HCC_ATAGEN_RUN_MODE_CODE && hcc_stack_count(w->atagen.paused_file_stack) == 0) {
		hcc_atagen_token_add(w, HCC_ATA_TOKEN_EOF);
	}
	w->atagen.run_mode = old_run_mode;
	w->atagen.dst_token_bag = old_dst_token_bag;
}

