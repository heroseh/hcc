#include "hcc_internal.h"

// ===========================================
//
//
// Preprocessor
//
//
// ===========================================

const char* hcc_pp_predefined_macro_identifier_strings[HCC_PP_PREDEFINED_MACRO_COUNT] = {
	[HCC_PP_PREDEFINED_MACRO___FILE__] = "__FILE__",
	[HCC_PP_PREDEFINED_MACRO___LINE__] = "__LINE__",
	[HCC_PP_PREDEFINED_MACRO___STDC__] = "__STDC__",
	[HCC_PP_PREDEFINED_MACRO___STDC_VERSION__] = "__STDC_VERSION__",
	[HCC_PP_PREDEFINED_MACRO___COUNTER__] = "__COUNTER__",
	[HCC_PP_PREDEFINED_MACRO___HCC__] = "__HCC__",
	[HCC_PP_PREDEFINED_MACRO___HCC_GPU__] = "__HCC_GPU__",
	[HCC_PP_PREDEFINED_MACRO___HCC_X86_64__] = "__HCC_X86_64__",
	[HCC_PP_PREDEFINED_MACRO___HCC_AARCH64__] = "__HCC_AARCH64__",
	[HCC_PP_PREDEFINED_MACRO___HCC_LINUX__] = "__HCC_LINUX__",
	[HCC_PP_PREDEFINED_MACRO___HCC_WINDOWS__] = "__HCC_WINDOWS__",
};

const char* hcc_pp_directive_enum_strings[HCC_PP_DIRECTIVE_COUNT] = {
	[HCC_PP_DIRECTIVE_DEFINE] = "HCC_PP_DIRECTIVE_DEFINE",
	[HCC_PP_DIRECTIVE_UNDEF] = "HCC_PP_DIRECTIVE_UNDEF",
	[HCC_PP_DIRECTIVE_INCLUDE] = "HCC_PP_DIRECTIVE_INCLUDE",
	[HCC_PP_DIRECTIVE_IF] = "HCC_PP_DIRECTIVE_IF",
	[HCC_PP_DIRECTIVE_IFDEF] = "HCC_PP_DIRECTIVE_IFDEF",
	[HCC_PP_DIRECTIVE_IFNDEF] = "HCC_PP_DIRECTIVE_IFNDEF",
	[HCC_PP_DIRECTIVE_ELSE] = "HCC_PP_DIRECTIVE_ELSE",
	[HCC_PP_DIRECTIVE_ELIF] = "HCC_PP_DIRECTIVE_ELIF",
	[HCC_PP_DIRECTIVE_ELIFDEF] = "HCC_PP_DIRECTIVE_ELIFDEF",
	[HCC_PP_DIRECTIVE_ELIFNDEF] = "HCC_PP_DIRECTIVE_ELIFNDEF",
	[HCC_PP_DIRECTIVE_ENDIF] = "HCC_PP_DIRECTIVE_ENDIF",
	[HCC_PP_DIRECTIVE_LINE] = "HCC_PP_DIRECTIVE_LINE",
	[HCC_PP_DIRECTIVE_ERROR] = "HCC_PP_DIRECTIVE_ERROR",
	[HCC_PP_DIRECTIVE_WARNING] = "HCC_PP_DIRECTIVE_WARNING",
	[HCC_PP_DIRECTIVE_PRAGMA] = "HCC_PP_DIRECTIVE_PRAGMA",
};

const char* hcc_pp_directive_strings[HCC_PP_DIRECTIVE_COUNT] = {
	[HCC_PP_DIRECTIVE_DEFINE] = "define",
	[HCC_PP_DIRECTIVE_UNDEF] = "undef",
	[HCC_PP_DIRECTIVE_INCLUDE] = "include",
	[HCC_PP_DIRECTIVE_IF] = "if",
	[HCC_PP_DIRECTIVE_IFDEF] = "ifdef",
	[HCC_PP_DIRECTIVE_IFNDEF] = "ifndef",
	[HCC_PP_DIRECTIVE_ELSE] = "else",
	[HCC_PP_DIRECTIVE_ELIF] = "elif",
	[HCC_PP_DIRECTIVE_ELIFDEF] = "elifdef",
	[HCC_PP_DIRECTIVE_ELIFNDEF] = "elifndef",
	[HCC_PP_DIRECTIVE_ENDIF] = "endif",
	[HCC_PP_DIRECTIVE_LINE] = "line",
	[HCC_PP_DIRECTIVE_ERROR] = "error",
	[HCC_PP_DIRECTIVE_WARNING] = "warning",
	[HCC_PP_DIRECTIVE_PRAGMA] = "pragma",
};

uint32_t hcc_pp_directive_hashes[] = {
	[HCC_PP_DIRECTIVE_DEFINE] = 0x6a9f8552,
	[HCC_PP_DIRECTIVE_UNDEF] = 0x75191b51,
	[HCC_PP_DIRECTIVE_INCLUDE] = 0x75597a67,
	[HCC_PP_DIRECTIVE_IF] = 0x39386e06,
	[HCC_PP_DIRECTIVE_IFDEF] = 0x546cc0ed,
	[HCC_PP_DIRECTIVE_IFNDEF] = 0xf1e59c9f,
	[HCC_PP_DIRECTIVE_ELSE] = 0xbdbf5bf0,
	[HCC_PP_DIRECTIVE_ELIF] = 0xc0a5c8c3,
	[HCC_PP_DIRECTIVE_ELIFDEF] = 0x7aeee312,
	[HCC_PP_DIRECTIVE_ELIFNDEF] = 0xdc679c86,
	[HCC_PP_DIRECTIVE_ENDIF] = 0x1c12ad4d,
	[HCC_PP_DIRECTIVE_LINE] = 0x17db1627,
	[HCC_PP_DIRECTIVE_ERROR] = 0x21918751,
	[HCC_PP_DIRECTIVE_WARNING] = 0x792112ef,
	[HCC_PP_DIRECTIVE_PRAGMA] = 0x19fa4625,
};

HccStringId hcc_pp_macro_get_identifier_string_id(HccPPMacro* macro) {
	return macro->identifier_string_id;
}

HccString hcc_pp_macro_get_identifier_string(HccPPMacro* macro) {
	return macro->identifier_string;
}

HccLocation* hcc_pp_macro_get_location(HccPPMacro* macro) {
	return macro->location;
}

HccStringId* hcc_pp_macro_get_params(HccPPMacro* macro, uint32_t* params_count_out) {
	*params_count_out = macro->params_count;
	return macro->params;
}

// ===========================================
//
//
// ATA Token
//
//
// ===========================================

const char* hcc_ata_token_strings[HCC_ATA_TOKEN_COUNT] = {
	[HCC_ATA_TOKEN_EOF] = "end of file",
	[HCC_ATA_TOKEN_IDENT] = "identifier",
	[HCC_ATA_TOKEN_STRING] = "string",
	[HCC_ATA_TOKEN_INCLUDE_PATH_SYSTEM] = "<>",
	[HCC_ATA_TOKEN_BACK_SLASH] = "\\",
	[HCC_ATA_TOKEN_HASH] = "#",
	[HCC_ATA_TOKEN_DOUBLE_HASH] = "##",
	[HCC_ATA_TOKEN_MACRO_WHITESPACE] = " ",
	[HCC_ATA_TOKEN_MACRO_PARAM] = "macro param",
	[HCC_ATA_TOKEN_MACRO_STRINGIFY] = "macro stringify",
	[HCC_ATA_TOKEN_MACRO_STRINGIFY_WHITESPACE] = "macro stringify whitespace",
	[HCC_ATA_TOKEN_MACRO_CONCAT] = "##",
	[HCC_ATA_TOKEN_MACRO_CONCAT_WHITESPACE] = " ##",
	[HCC_ATA_TOKEN_CURLY_OPEN] = "{",
	[HCC_ATA_TOKEN_CURLY_CLOSE] = "}",
	[HCC_ATA_TOKEN_PARENTHESIS_OPEN] = "(",
	[HCC_ATA_TOKEN_PARENTHESIS_CLOSE] = ")",
	[HCC_ATA_TOKEN_SQUARE_OPEN] = "[",
	[HCC_ATA_TOKEN_SQUARE_CLOSE] = "]",
	[HCC_ATA_TOKEN_FULL_STOP] = ".",
	[HCC_ATA_TOKEN_COMMA] = ",",
	[HCC_ATA_TOKEN_SEMICOLON] = ";",
	[HCC_ATA_TOKEN_COLON] = ":",
	[HCC_ATA_TOKEN_PLUS] = "+",
	[HCC_ATA_TOKEN_MINUS] = "-",
	[HCC_ATA_TOKEN_FORWARD_SLASH] = "/",
	[HCC_ATA_TOKEN_ASTERISK] = "*",
	[HCC_ATA_TOKEN_PERCENT] = "%",
	[HCC_ATA_TOKEN_AMPERSAND] = "&",
	[HCC_ATA_TOKEN_PIPE] = "|",
	[HCC_ATA_TOKEN_CARET] = "^",
	[HCC_ATA_TOKEN_EXCLAMATION_MARK] = "!",
	[HCC_ATA_TOKEN_QUESTION_MARK] = "?",
	[HCC_ATA_TOKEN_TILDE] = "~",
	[HCC_ATA_TOKEN_EQUAL] = "=",
	[HCC_ATA_TOKEN_LESS_THAN] = "<",
	[HCC_ATA_TOKEN_GREATER_THAN] = ">",
	[HCC_ATA_TOKEN_LOGICAL_AND] = "&&",
	[HCC_ATA_TOKEN_LOGICAL_OR] = "||",
	[HCC_ATA_TOKEN_LOGICAL_EQUAL] = "==",
	[HCC_ATA_TOKEN_LOGICAL_NOT_EQUAL] = "!=",
	[HCC_ATA_TOKEN_LESS_THAN_OR_EQUAL] = "<=",
	[HCC_ATA_TOKEN_GREATER_THAN_OR_EQUAL] = ">=",
	[HCC_ATA_TOKEN_BIT_SHIFT_LEFT] = "<<",
	[HCC_ATA_TOKEN_BIT_SHIFT_RIGHT] = ">>",
	[HCC_ATA_TOKEN_ADD_ASSIGN] = "+=",
	[HCC_ATA_TOKEN_SUBTRACT_ASSIGN] = "-=",
	[HCC_ATA_TOKEN_MULTIPLY_ASSIGN] = "*=",
	[HCC_ATA_TOKEN_DIVIDE_ASSIGN] = "/=",
	[HCC_ATA_TOKEN_MODULO_ASSIGN] = "%=",
	[HCC_ATA_TOKEN_BIT_SHIFT_LEFT_ASSIGN] = "<<=",
	[HCC_ATA_TOKEN_BIT_SHIFT_RIGHT_ASSIGN] = ">>=",
	[HCC_ATA_TOKEN_BIT_AND_ASSIGN] = "&=",
	[HCC_ATA_TOKEN_BIT_XOR_ASSIGN] = "^=",
	[HCC_ATA_TOKEN_BIT_OR_ASSIGN] = "|=",
	[HCC_ATA_TOKEN_INCREMENT] = "++",
	[HCC_ATA_TOKEN_DECREMENT] = "--",
	[HCC_ATA_TOKEN_ARROW_RIGHT] = "->",
	[HCC_ATA_TOKEN_LIT_UINT] = "unsigned int",
	[HCC_ATA_TOKEN_LIT_ULONG] = "unsigned long",
	[HCC_ATA_TOKEN_LIT_ULONGLONG] = "unsigned long long",
	[HCC_ATA_TOKEN_LIT_SINT] = "int",
	[HCC_ATA_TOKEN_LIT_SLONG] = "long",
	[HCC_ATA_TOKEN_LIT_SLONGLONG] = "long long",
	[HCC_ATA_TOKEN_LIT_FLOAT] = "float",
	[HCC_ATA_TOKEN_LIT_DOUBLE] = "double",
	[HCC_ATA_TOKEN_KEYWORD_VOID] = "void",
	[HCC_ATA_TOKEN_KEYWORD_BOOL] = "_Bool",
	[HCC_ATA_TOKEN_KEYWORD_CHAR] = "char",
	[HCC_ATA_TOKEN_KEYWORD_SHORT] = "short",
	[HCC_ATA_TOKEN_KEYWORD_INT] = "int",
	[HCC_ATA_TOKEN_KEYWORD_LONG] = "long",
	[HCC_ATA_TOKEN_KEYWORD_FLOAT] = "float",
	[HCC_ATA_TOKEN_KEYWORD_DOUBLE] = "double",
	[HCC_ATA_TOKEN_KEYWORD_UNSIGNED] = "unsigned",
	[HCC_ATA_TOKEN_KEYWORD_SIGNED] = "signed",
	[HCC_ATA_TOKEN_KEYWORD_COMPLEX] = "_Complex",
	[HCC_ATA_TOKEN_KEYWORD_ATOMIC] = "_Atomic",
	[HCC_ATA_TOKEN_KEYWORD_RETURN] = "return",
	[HCC_ATA_TOKEN_KEYWORD_IF] = "if",
	[HCC_ATA_TOKEN_KEYWORD_ELSE] = "else",
	[HCC_ATA_TOKEN_KEYWORD_DO] = "do",
	[HCC_ATA_TOKEN_KEYWORD_WHILE] = "while",
	[HCC_ATA_TOKEN_KEYWORD_FOR] = "for",
	[HCC_ATA_TOKEN_KEYWORD_SWITCH] = "switch",
	[HCC_ATA_TOKEN_KEYWORD_CASE] = "case",
	[HCC_ATA_TOKEN_KEYWORD_DEFAULT] = "default",
	[HCC_ATA_TOKEN_KEYWORD_BREAK] = "break",
	[HCC_ATA_TOKEN_KEYWORD_CONTINUE] = "continue",
	[HCC_ATA_TOKEN_KEYWORD_TRUE] = "true",
	[HCC_ATA_TOKEN_KEYWORD_FALSE] = "false",
	[HCC_ATA_TOKEN_KEYWORD_VERTEX] = "__hcc_vertex",
	[HCC_ATA_TOKEN_KEYWORD_PIXEL] = "__hcc_pixel",
	[HCC_ATA_TOKEN_KEYWORD_COMPUTE] = "__hcc_compute",
	[HCC_ATA_TOKEN_KEYWORD_MESHTASK] = "__hcc_meshtask",
	[HCC_ATA_TOKEN_KEYWORD_MESH] = "__hcc_mesh",
	[HCC_ATA_TOKEN_KEYWORD_ENUM] = "enum",
	[HCC_ATA_TOKEN_KEYWORD_STRUCT] = "struct",
	[HCC_ATA_TOKEN_KEYWORD_UNION] = "union",
	[HCC_ATA_TOKEN_KEYWORD_TYPEDEF] = "typedef",
	[HCC_ATA_TOKEN_KEYWORD_STATIC] = "static",
	[HCC_ATA_TOKEN_KEYWORD_CONST] = "const",
	[HCC_ATA_TOKEN_KEYWORD_MUTONLY] = "mutonly",
	[HCC_ATA_TOKEN_KEYWORD_AUTO] = "auto",
	[HCC_ATA_TOKEN_KEYWORD_REGISTER] = "register",
	[HCC_ATA_TOKEN_KEYWORD_VOLATILE] = "volatile",
	[HCC_ATA_TOKEN_KEYWORD_EXTERN] = "extern",
	[HCC_ATA_TOKEN_KEYWORD_THREAD_LOCAL] = "_Thread_local",
	[HCC_ATA_TOKEN_KEYWORD_DISPATCH_GROUP] = "__hcc_dispatch_group",
	[HCC_ATA_TOKEN_KEYWORD_INLINE] = "inline",
	[HCC_ATA_TOKEN_KEYWORD_NO_RETURN] = "_Noreturn",
	[HCC_ATA_TOKEN_KEYWORD_SIZEOF] = "sizeof",
	[HCC_ATA_TOKEN_KEYWORD_ALIGNOF] = "_Alignof",
	[HCC_ATA_TOKEN_KEYWORD_ALIGNAS] = "_Alignas",
	[HCC_ATA_TOKEN_KEYWORD_STATIC_ASSERT] = "_Static_assert",
	[HCC_ATA_TOKEN_KEYWORD_GENERIC] = "_Generic",
	[HCC_ATA_TOKEN_KEYWORD_RESTRICT] = "restrict",
	[HCC_ATA_TOKEN_KEYWORD_HALF_T] = "__hcc_half_t",
	[HCC_ATA_TOKEN_KEYWORD_VECTOR_T] = "__hcc_vector_t",
	[HCC_ATA_TOKEN_KEYWORD_RASTERIZER_STATE] = "__hcc_rasterizer_state",
	[HCC_ATA_TOKEN_KEYWORD_PIXEL_STATE] = "__hcc_pixel_state",
	[HCC_ATA_TOKEN_KEYWORD_INTERP] = "__hcc_interp",
	[HCC_ATA_TOKEN_KEYWORD_NOINTERP] = "__hcc_nointerp",
	[HCC_ATA_TOKEN_KEYWORD_ROSAMPLER] = "HccRoSampler",
	[HCC_ATA_TOKEN_KEYWORD_ROBUFFER] = "HccRoBuffer",
	[HCC_ATA_TOKEN_KEYWORD_ROTEXTURE1D] = "HccRoTexture1D",
	[HCC_ATA_TOKEN_KEYWORD_ROTEXTURE1DARRAY] = "HccRoTexture1DArray",
	[HCC_ATA_TOKEN_KEYWORD_ROTEXTURE2D] = "HccRoTexture2D",
	[HCC_ATA_TOKEN_KEYWORD_ROTEXTURE2DARRAY] = "HccRoTexture2DArray",
	[HCC_ATA_TOKEN_KEYWORD_ROTEXTURE2DMS] = "HccRoTexture2DMS",
	[HCC_ATA_TOKEN_KEYWORD_ROTEXTURE2DMSARRAY] = "HccRoTexture2DMSArray",
	[HCC_ATA_TOKEN_KEYWORD_ROTEXTURE3D] = "HccRoTexture3D",
	[HCC_ATA_TOKEN_KEYWORD_RWBUFFER] = "HccRwBuffer",
	[HCC_ATA_TOKEN_KEYWORD_RWTEXTURE1D] = "HccRwTexture1D",
	[HCC_ATA_TOKEN_KEYWORD_RWTEXTURE1DARRAY] = "HccRwTexture1DArray",
	[HCC_ATA_TOKEN_KEYWORD_RWTEXTURE2D] = "HccRwTexture2D",
	[HCC_ATA_TOKEN_KEYWORD_RWTEXTURE2DARRAY] = "HccRwTexture2DArray",
	[HCC_ATA_TOKEN_KEYWORD_RWTEXTURE2DMS] = "HccRwTexture2DMS",
	[HCC_ATA_TOKEN_KEYWORD_RWTEXTURE2DMSARRAY] = "HccRwTexture2DMSArray",
	[HCC_ATA_TOKEN_KEYWORD_RWTEXTURE3D] = "HccRwTexture3D",
	[HCC_ATA_TOKEN_KEYWORD_WOBUFFER] = "HccWoBuffer",
	[HCC_ATA_TOKEN_KEYWORD_WOTEXTURE1D] = "HccWoTexture1D",
	[HCC_ATA_TOKEN_KEYWORD_WOTEXTURE1DARRAY] = "HccWoTexture1DArray",
	[HCC_ATA_TOKEN_KEYWORD_WOTEXTURE2D] = "HccWoTexture2D",
	[HCC_ATA_TOKEN_KEYWORD_WOTEXTURE2DARRAY] = "HccWoTexture2DArray",
	[HCC_ATA_TOKEN_KEYWORD_WOTEXTURE2DMS] = "HccWoTexture2DMS",
	[HCC_ATA_TOKEN_KEYWORD_WOTEXTURE2DMSARRAY] = "HccWoTexture2DMSArray",
	[HCC_ATA_TOKEN_KEYWORD_WOTEXTURE3D] = "HccWoTexture3D",
	[HCC_ATA_TOKEN_KEYWORD_SAMPLETEXTURE1D] = "HccSampleTexture1D",
	[HCC_ATA_TOKEN_KEYWORD_SAMPLETEXTURE1DARRAY] = "HccSampleTexture1DArray",
	[HCC_ATA_TOKEN_KEYWORD_SAMPLETEXTURE2D] = "HccSampleTexture2D",
	[HCC_ATA_TOKEN_KEYWORD_SAMPLETEXTURE2DARRAY] = "HccSampleTexture2DArray",
	[HCC_ATA_TOKEN_KEYWORD_SAMPLETEXTURECUBE] = "HccSampleTextureCube",
	[HCC_ATA_TOKEN_KEYWORD_SAMPLETEXTURECUBEARRAY] = "HccSampleTextureCubeArray",
	[HCC_ATA_TOKEN_KEYWORD_SAMPLETEXTURE3D] = "HccSampleTexture3D",
};

bool hcc_ata_token_has_value(HccATAToken token) {
	return hcc_ata_token_num_values(token);
}

uint32_t hcc_ata_token_num_values(HccATAToken token) {
	switch (token) {
		case HCC_ATA_TOKEN_IDENT:
		case HCC_ATA_TOKEN_STRING:
		case HCC_ATA_TOKEN_INCLUDE_PATH_SYSTEM:
		case HCC_ATA_TOKEN_MACRO_PARAM:
		case HCC_ATA_TOKEN_MACRO_STRINGIFY:
		case HCC_ATA_TOKEN_MACRO_STRINGIFY_WHITESPACE:
			return 1;
		case HCC_ATA_TOKEN_LIT_UINT:
		case HCC_ATA_TOKEN_LIT_ULONG:
		case HCC_ATA_TOKEN_LIT_ULONGLONG:
		case HCC_ATA_TOKEN_LIT_SINT:
		case HCC_ATA_TOKEN_LIT_SLONG:
		case HCC_ATA_TOKEN_LIT_SLONGLONG:
		case HCC_ATA_TOKEN_LIT_FLOAT:
		case HCC_ATA_TOKEN_LIT_DOUBLE:
			return 2;
		default:
			return 0;
	}
}

bool hcc_ata_token_concat_is_okay(HccATAToken before, HccATAToken after) {
	return (
		(HCC_ATA_TOKEN_IS_LIT_NUMBER(before) &&
			(
				HCC_ATA_TOKEN_IS_LIT_NUMBER(after) ||
				after == HCC_ATA_TOKEN_IDENT
			)
		) ||
		(before == HCC_ATA_TOKEN_IDENT &&
			(
				after == HCC_ATA_TOKEN_IDENT ||
				HCC_ATA_TOKEN_IS_LIT_NUMBER(after)
			)
		) ||
		(before == HCC_ATA_TOKEN_EQUAL && after == HCC_ATA_TOKEN_EQUAL) ||
		(before == HCC_ATA_TOKEN_EXCLAMATION_MARK && after == HCC_ATA_TOKEN_EQUAL) ||
		(before == HCC_ATA_TOKEN_PLUS &&
			(
				after == HCC_ATA_TOKEN_PLUS ||
				after == HCC_ATA_TOKEN_EQUAL
			)
		) ||
		(before == HCC_ATA_TOKEN_MINUS &&
			(
				after == HCC_ATA_TOKEN_MINUS ||
				after == HCC_ATA_TOKEN_EQUAL ||
				after == HCC_ATA_TOKEN_GREATER_THAN
			)
		) ||
		(before == HCC_ATA_TOKEN_ASTERISK && after == HCC_ATA_TOKEN_EQUAL) ||
		(before == HCC_ATA_TOKEN_FORWARD_SLASH && after == HCC_ATA_TOKEN_EQUAL) ||
		(before == HCC_ATA_TOKEN_PERCENT && after == HCC_ATA_TOKEN_EQUAL) ||
		(before == HCC_ATA_TOKEN_AMPERSAND &&
			(
				after == HCC_ATA_TOKEN_AMPERSAND ||
				after == HCC_ATA_TOKEN_EQUAL
			)
		) ||
		(before == HCC_ATA_TOKEN_PIPE &&
			(
				after == HCC_ATA_TOKEN_PIPE ||
				after == HCC_ATA_TOKEN_EQUAL
			)
		) ||
		(before == HCC_ATA_TOKEN_CARET &&
			(
				after == HCC_ATA_TOKEN_CARET ||
				after == HCC_ATA_TOKEN_EQUAL
			)
		) ||
		(before == HCC_ATA_TOKEN_LESS_THAN &&
			(
				after == HCC_ATA_TOKEN_LESS_THAN ||
				after == HCC_ATA_TOKEN_LESS_THAN_OR_EQUAL ||
				after == HCC_ATA_TOKEN_EQUAL
			)
		) ||
		(before == HCC_ATA_TOKEN_GREATER_THAN &&
			(
				after == HCC_ATA_TOKEN_GREATER_THAN ||
				after == HCC_ATA_TOKEN_GREATER_THAN_OR_EQUAL ||
				after == HCC_ATA_TOKEN_EQUAL
			)
		) ||
		(before == HCC_ATA_TOKEN_EQUAL &&
			(
				after == HCC_ATA_TOKEN_GREATER_THAN ||
				after == HCC_ATA_TOKEN_GREATER_THAN_OR_EQUAL ||
				after == HCC_ATA_TOKEN_EQUAL
			)
		) ||
		(before == HCC_ATA_TOKEN_HASH && after == HCC_ATA_TOKEN_HASH)
	);
}

// ===========================================
//
//
// ATA Iterator
//
//
// ===========================================

HccATAIter* hcc_ata_iter_start(HccASTFile* file) {
	HccATAIter* iter = &file->iter;
	HCC_ASSERT(iter->tokens == NULL, "hcc_ata_iter_finish must be called before starting the next token iterator");
	iter->tokens = file->token_bag.tokens;
	iter->locations = file->token_bag.locations;
	iter->values = file->token_bag.values;
	iter->token_idx = 0;
	iter->value_idx = 0;
	iter->tokens_count = hcc_stack_count(file->token_bag.tokens);
	iter->values_count = hcc_stack_count(file->token_bag.values);
	return iter;
}

void hcc_ata_iter_finish(HccASTFile* file, HccATAIter* iter) {
	HCC_ASSERT(&file->iter == iter, "cannot finish the iterator from another file");
	HCC_ASSERT(iter->tokens, "hcc_ata_iter_start must be called before finishing the token iterator");
	iter->tokens = NULL;
}

HccATAToken hcc_ata_iter_next(HccATAIter* iter) {
	if (iter->token_idx >= iter->tokens_count) {
		return HCC_ATA_TOKEN_EOF;
	}

	iter->token_idx += 1;
	HccATAToken token = iter->tokens[iter->token_idx];
	return token;
}

HccATAToken hcc_ata_iter_peek(HccATAIter* iter) {
	if (iter->token_idx >= iter->tokens_count) {
		return HCC_ATA_TOKEN_EOF;
	}

	return iter->tokens[iter->token_idx];
}

HccATAToken hcc_ata_iter_peek_ahead(HccATAIter* iter, uint32_t by) {
	if (iter->token_idx + by >= iter->tokens_count) {
		return HCC_ATA_TOKEN_EOF;
	}

	return iter->tokens[iter->token_idx + by];
}

HccLocation* hcc_ata_iter_location(HccATAIter* iter) {
	return HCC_PP_TOKEN_STRIP_PREEXPANDED_MACRO_ARG(iter->locations[iter->token_idx]);
}

HccATAValue hcc_ata_iter_next_value(HccATAIter* iter) {
	HCC_DEBUG_ASSERT(hcc_ata_token_has_value(iter->tokens[iter->token_idx]), "cannot get the next value as the current token does not have a value");
	HccATAValue value = iter->values[iter->value_idx];
	iter->value_idx += 1;
	return value;
}

HccATAValue hcc_ata_iter_peek_value(HccATAIter* iter) {
	HCC_DEBUG_ASSERT(hcc_ata_token_has_value(iter->tokens[iter->token_idx]), "cannot get the next value as the current token does not have a value");
	return iter->values[iter->value_idx];
}

// ===========================================
//
//
// ATA Token Cursor
//
//
// ===========================================

uint32_t hcc_ata_token_cursor_tokens_count(HccATATokenCursor* cursor) {
	return cursor->tokens_end_idx - cursor->tokens_start_idx;
}

// ===========================================
//
//
// ATA Token Bag
//
//
// ===========================================

void hcc_ata_token_bag_init(HccATATokenBag* bag, uint32_t tokens_grow_count, uint32_t tokens_reserve_cap, uint32_t values_grow_count, uint32_t values_reserve_cap) {
	bag->tokens = hcc_stack_init(HccATAToken, HCC_ALLOC_TAG_ATA_TOKEN_BAG_TOKENS, tokens_grow_count, tokens_reserve_cap);
	bag->locations = hcc_stack_init(HccLocation*, HCC_ALLOC_TAG_ATA_TOKEN_BAG_LOCATIONS, tokens_grow_count, tokens_reserve_cap);
	bag->values = hcc_stack_init(HccATAValue, HCC_ALLOC_TAG_ATA_TOKEN_BAG_VALUES, values_grow_count, values_reserve_cap);
}

void hcc_ata_token_bag_deinit(HccATATokenBag* bag) {
	hcc_stack_deinit(bag->tokens);
	hcc_stack_deinit(bag->locations);
	hcc_stack_deinit(bag->values);
}

void hcc_ata_token_bag_reset(HccATATokenBag* bag) {
	hcc_stack_clear(bag->tokens);
	hcc_stack_clear(bag->locations);
	hcc_stack_clear(bag->values);
}

void hcc_ata_token_bag_push_token(HccATATokenBag* bag, HccATAToken token, HccLocation* location) {
	*hcc_stack_push(bag->tokens) = token;
	*hcc_stack_push(bag->locations) = location;
}

void hcc_ata_token_bag_push_value(HccATATokenBag* bag, HccATAValue value) {
	*hcc_stack_push(bag->values) = value;
}

bool hcc_ata_token_bag_pop_token(HccATATokenBag* bag) {
	if (hcc_stack_count(bag->tokens) == 0) {
		return false;
	}

	if (hcc_ata_token_has_value(*hcc_stack_get_last(bag->tokens))) {
		hcc_stack_pop(bag->values);
	}

	hcc_stack_pop(bag->tokens);
	hcc_stack_pop(bag->locations);
	return true;
}

uint32_t hcc_ata_token_bag_stringify_single(HccATATokenBag* bag, HccATATokenCursor* cursor, HccPPMacro* macro, char* outbuf, uint32_t outbufsize) {
	uint32_t outbufidx = 0;
	HccATAToken token = *hcc_stack_get(bag->tokens, cursor->token_idx);
	cursor->token_idx += 1;
	switch (token) {
		case HCC_ATA_TOKEN_IDENT: {
			HccStringId string_id = hcc_stack_get(bag->values, cursor->token_value_idx)->string_id;
			cursor->token_value_idx += 1;
			HccString string = hcc_string_table_get(string_id);
			uint32_t size = HCC_MIN(string.size, outbufsize - outbufidx);
			memcpy(outbuf + outbufidx, string.data, size);
			outbufidx += size;
			break;
		};

		//
		// these cases are only reachable for the debug printing code in hcc_tokengen_print
		case HCC_ATA_TOKEN_MACRO_PARAM:
		case HCC_ATA_TOKEN_MACRO_STRINGIFY:
		case HCC_ATA_TOKEN_MACRO_STRINGIFY_WHITESPACE: {
			uint32_t param_idx = hcc_stack_get(bag->values, cursor->token_value_idx)->macro_param_idx;
			HccStringId string_id = macro->params[param_idx];
			cursor->token_value_idx += 1;
			if (token == HCC_ATA_TOKEN_MACRO_STRINGIFY) {
				if (outbufidx < outbufsize) {
					outbuf[outbufidx] = '#';
					outbufidx += 1;
				}
			} else if (token == HCC_ATA_TOKEN_MACRO_STRINGIFY_WHITESPACE) {
				uint32_t size = HCC_MIN(2, outbufsize - outbufidx);
				memcpy(outbuf + outbufidx, "# ", 2);
				outbufidx += size;
			}
			HccString string = hcc_string_table_get(string_id);
			uint32_t size = HCC_MIN(string.size, outbufsize - outbufidx);
			memcpy(outbuf + outbufidx, string.data, size);
			outbufidx += size;
			break;
		};

		case HCC_ATA_TOKEN_LIT_UINT:
		case HCC_ATA_TOKEN_LIT_ULONG:
		case HCC_ATA_TOKEN_LIT_ULONGLONG:
		case HCC_ATA_TOKEN_LIT_SINT:
		case HCC_ATA_TOKEN_LIT_SLONG:
		case HCC_ATA_TOKEN_LIT_SLONGLONG:
		case HCC_ATA_TOKEN_LIT_FLOAT:
		case HCC_ATA_TOKEN_LIT_DOUBLE: {
			cursor->token_value_idx += 1;
			HccStringId string_id = bag->values[cursor->token_value_idx].string_id;
			cursor->token_value_idx += 1;

			HccString string = hcc_string_table_get(string_id);
			uint32_t size = HCC_MIN(string.size, outbufsize - outbufidx);
			memcpy(outbuf + outbufidx, string.data, size);
			outbufidx += size;
			break;
		};

		case HCC_ATA_TOKEN_STRING: {
			HccStringId string_id = bag->values[cursor->token_value_idx].string_id;
			cursor->token_value_idx += 1;

			HccString string = hcc_string_table_get(string_id);

			if (outbufidx < outbufsize) {
				outbuf[outbufidx] = '"';
				outbufidx += 1;
			}
			for (uint32_t idx = 0; idx < string.size; idx += 1) {
				char ch = string.data[idx];
				switch (ch) {
					//
					// TODO complete the escape codes
					case '\\':
						if (outbufidx < outbufsize) { outbuf[outbufidx] = '\\'; outbufidx += 1; }
						if (outbufidx < outbufsize) { outbuf[outbufidx] = '\\'; outbufidx += 1; }
						break;
					case '\r':
						if (outbufidx < outbufsize) { outbuf[outbufidx] = '\\'; outbufidx += 1; }
						if (outbufidx < outbufsize) { outbuf[outbufidx] = '\r'; outbufidx += 1; }
						break;
					case '\n':
						if (outbufidx < outbufsize) { outbuf[outbufidx] = '\\'; outbufidx += 1; }
						if (outbufidx < outbufsize) { outbuf[outbufidx] = '\n'; outbufidx += 1; }
						break;
					default:
						if (outbufidx < outbufsize) {
							outbuf[outbufidx] =  ch;
							outbufidx += 1;
						}
						break;
				}
			}
			if (outbufidx < outbufsize) {
				outbuf[outbufidx] = '"';
				outbufidx += 1;
			}

			break;
		};
		case HCC_ATA_TOKEN_MACRO_CONCAT:
		case HCC_ATA_TOKEN_MACRO_CONCAT_WHITESPACE: {
			//
			// tokengen will reorder macro concat so comes before the two operands.
			// so `ident ## ifier` becomes `## ident ifier`
			// so when stringifing, put it back in the order it was in string form.
			outbufidx += hcc_ata_token_bag_stringify_single(bag, cursor, macro, outbuf + outbufidx, outbufsize - outbufidx);
			HccString string = hcc_string_c((char*)hcc_ata_token_strings[token]);
			uint32_t size = HCC_MIN(string.size, outbufsize - outbufidx);
			memcpy(outbuf + outbufidx, string.data, size);
			outbufidx += size;
			if (*hcc_stack_get(bag->tokens, cursor->token_idx) == HCC_ATA_TOKEN_MACRO_WHITESPACE) {
				if (outbufidx < outbufsize) {
					outbuf[outbufidx] = ' ';
					outbufidx += 1;
				}
				cursor->token_idx += 1;
			}
			outbufidx += hcc_ata_token_bag_stringify_single(bag, cursor, macro, outbuf + outbufidx, outbufsize - outbufidx);
			break;
		};

		default: {
			HccString string = hcc_string_c((char*)hcc_ata_token_strings[token]);
			uint32_t size = HCC_MIN(string.size, outbufsize - outbufidx);
			memcpy(outbuf + outbufidx, string.data, size);
			outbufidx += size;
			break;
		};
	}

	return outbufidx;
}

HccATAToken hcc_ata_token_bag_stringify_single_or_macro_param(HccWorker* w, HccATATokenBag* bag, HccATATokenCursor* cursor, uint32_t args_start_idx, HccATATokenBag* args_src_bag, bool false_before_true_after, char* outbuf, uint32_t outbufsize, uint32_t* outbufidx_out) {
	if (cursor->token_idx >= cursor->tokens_end_idx) {
		return HCC_ATA_TOKEN_COUNT;
	}

	HccATAToken token = *hcc_stack_get(bag->tokens, cursor->token_idx);
	if (token == HCC_ATA_TOKEN_MACRO_PARAM) {
		cursor->token_idx += 1;
		uint32_t param_idx = hcc_stack_get(bag->values, cursor->token_value_idx)->macro_param_idx;
		cursor->token_value_idx += 1;
		HccPPMacroArg* arg = hcc_stack_get(w->atagen.ppgen.macro_args_stack, args_start_idx + param_idx);
		HccATATokenCursor arg_cursor = arg->cursor;
		if (arg_cursor.tokens_start_idx == arg_cursor.tokens_end_idx) {
			return HCC_ATA_TOKEN_COUNT;
		}

		HccATAToken return_token = *hcc_stack_get(args_src_bag->tokens, false_before_true_after ? arg_cursor.tokens_end_idx - 1 : arg_cursor.tokens_start_idx);
		hcc_ata_token_bag_stringify_range(args_src_bag, &arg_cursor, NULL, outbuf, outbufsize, outbufidx_out);
		return return_token;
	}

	*outbufidx_out += hcc_ata_token_bag_stringify_single(bag, cursor, NULL, outbuf, outbufsize);
	return token;
}

HccStringId hcc_ata_token_bag_stringify_range(HccATATokenBag* bag, HccATATokenCursor* cursor, HccPPMacro* macro, char* outbuf, uint32_t outbufsize, uint32_t* outbufidx_out) {
	uint32_t outbufidx = 0;
	while (cursor->token_idx < cursor->tokens_end_idx) {
		outbufidx += hcc_ata_token_bag_stringify_single(bag, cursor, macro, outbuf + outbufidx, outbufsize - outbufidx);
	}

	if (outbufidx_out) {
		*outbufidx_out += outbufidx;
		return HccStringId(0);
	} else {
		HccStringId string_id;
		hcc_string_table_deduplicate(outbuf, outbufidx, &string_id);
		return string_id;
	}
}
