
HccATAToken hcc_astgen_specifier_tokens[HCC_ASTGEN_SPECIFIER_COUNT] = {
	[HCC_ASTGEN_SPECIFIER_STATIC] =           HCC_ATA_TOKEN_KEYWORD_STATIC,
	[HCC_ASTGEN_SPECIFIER_EXTERN] =           HCC_ATA_TOKEN_KEYWORD_EXTERN,
	[HCC_ASTGEN_SPECIFIER_THREAD_LOCAL] =     HCC_ATA_TOKEN_KEYWORD_THREAD_LOCAL,
	[HCC_ASTGEN_SPECIFIER_INLINE] =           HCC_ATA_TOKEN_KEYWORD_INLINE,
	[HCC_ASTGEN_SPECIFIER_NO_RETURN] =        HCC_ATA_TOKEN_KEYWORD_NO_RETURN,
	[HCC_ASTGEN_SPECIFIER_RASTERIZER_STATE] = HCC_ATA_TOKEN_KEYWORD_RASTERIZER_STATE,
	[HCC_ASTGEN_SPECIFIER_FRAGMENT_STATE] =   HCC_ATA_TOKEN_KEYWORD_FRAGMENT_STATE,
	[HCC_ASTGEN_SPECIFIER_BUFFER_ELEMENT] =   HCC_ATA_TOKEN_KEYWORD_BUFFER_ELEMENT,
	[HCC_ASTGEN_SPECIFIER_RESOURCE_SET] =     HCC_ATA_TOKEN_KEYWORD_RESOURCE_SET,
	[HCC_ASTGEN_SPECIFIER_RESOURCE_TABLE] =   HCC_ATA_TOKEN_KEYWORD_RESOURCE_TABLE,
	[HCC_ASTGEN_SPECIFIER_RESOURCES] =        HCC_ATA_TOKEN_KEYWORD_RESOURCES,
	[HCC_ASTGEN_SPECIFIER_POSITION] =         HCC_ATA_TOKEN_KEYWORD_POSITION,
	[HCC_ASTGEN_SPECIFIER_NOINTERP] =         HCC_ATA_TOKEN_KEYWORD_NOINTERP,
	[HCC_ASTGEN_SPECIFIER_VERTEX] =           HCC_ATA_TOKEN_KEYWORD_VERTEX,
	[HCC_ASTGEN_SPECIFIER_FRAGMENT] =         HCC_ATA_TOKEN_KEYWORD_FRAGMENT,
};

void hcc_astgen_init(HccWorker* w, HccASTGenSetup* setup) {
	w->astgen.variable_stack_strings = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_STRINGS, setup->variable_stack_grow_count, setup->variable_stack_reserve_cap);
	w->astgen.variable_stack_var_indices = hcc_stack_init(uint32_t, HCC_ALLOC_TAG_ASTGEN_VARIABLE_STACK_VAR_INDICES, setup->variable_stack_grow_count, setup->variable_stack_reserve_cap);
	w->astgen.compound_type_find_fields = hcc_stack_init(HccFieldAccess, HCC_ALLOC_TAG_ASTGEN_COMPOUND_TYPE_FIND_FIELDS, setup->compound_fields_reserve_cap, setup->compound_fields_reserve_cap);
	w->astgen.compound_field_names = hcc_stack_init(HccStringId, HCC_ALLOC_TAG_ASTGEN_COMPOUND_FIELD_NAMES, setup->compound_fields_reserve_cap, setup->compound_fields_reserve_cap);
	w->astgen.compound_field_locations = hcc_stack_init(HccLocation*, HCC_ALLOC_TAG_ASTGEN_COMPOUND_FIELD_LOCATIONS, setup->compound_fields_reserve_cap, setup->compound_fields_reserve_cap);
	w->astgen.compound_fields = hcc_stack_init(HccCompoundField, HCC_ALLOC_TAG_ASTGEN_COMPOUND_FIELDS, setup->compound_fields_reserve_cap, setup->compound_fields_reserve_cap);
	w->astgen.function_params_and_variables = hcc_stack_init(HccASTVariable, HCC_ALLOC_TAG_ASTGEN_FUNCTION_PARAMS_AND_VARIABLES, setup->function_params_and_variables_reserve_cap, setup->function_params_and_variables_reserve_cap);
	w->astgen.enum_values = hcc_stack_init(HccEnumValue, HCC_ALLOC_TAG_ASTGEN_ENUM_VALUES, setup->enum_values_reserve_cap, setup->enum_values_reserve_cap);

	w->astgen.curly_initializer.nested = hcc_stack_init(HccASTGenCurlyInitializerNested, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED, setup->curly_initializer_nested_reserve_cap, setup->curly_initializer_nested_reserve_cap);
	w->astgen.curly_initializer.nested_curlys = hcc_stack_init(HccASTGenCurlyInitializerCurly, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_CURLYS, setup->curly_initializer_nested_curlys_reserve_cap, setup->curly_initializer_nested_curlys_reserve_cap);
	w->astgen.curly_initializer.nested_elmts = hcc_stack_init(HccASTGenCurlyInitializerElmt, HCC_ALLOC_TAG_ASTGEN_CURLY_INITIALIZER_NESTED_ELMTS, setup->curly_initializer_nested_elmts_reserve_cap, setup->curly_initializer_nested_elmts_reserve_cap);
}

void hcc_astgen_deinit(HccWorker* w) {
	hcc_stack_deinit(w->astgen.variable_stack_strings);
	hcc_stack_deinit(w->astgen.variable_stack_var_indices);
	hcc_stack_deinit(w->astgen.compound_type_find_fields);
	hcc_stack_deinit(w->astgen.compound_field_names);
	hcc_stack_deinit(w->astgen.compound_field_locations);
}

void hcc_astgen_reset(HccWorker* w) {
	w->astgen.specifier_flags = 0;
	hcc_stack_clear(w->astgen.curly_initializer.nested);
	hcc_stack_clear(w->astgen.curly_initializer.nested_curlys);
	hcc_stack_clear(w->astgen.curly_initializer.nested_elmts);
	hcc_stack_clear(w->astgen.compound_fields);
	hcc_stack_clear(w->astgen.function_params_and_variables);
	hcc_stack_clear(w->astgen.variable_stack_strings);
	hcc_stack_clear(w->astgen.variable_stack_var_indices);
	hcc_stack_clear(w->astgen.compound_type_find_fields);
	hcc_stack_clear(w->astgen.compound_field_names);
	hcc_stack_clear(w->astgen.compound_field_locations);
}

void hcc_astgen_error_1(HccWorker* w, HccErrorCode error_code, ...) {
	va_list va_args;
	va_start(va_args, error_code);
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_astgen_error_1_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_astgen_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_astgen_error_2_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_astgen_warn_1(HccWorker* w, HccWarnCode warn_code, ...) {
	va_list va_args;
	va_start(va_args, warn_code);
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	hcc_warn_pushv(hcc_worker_task(w), warn_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_astgen_warn_1_manual(HccWorker* w, HccWarnCode warn_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	hcc_warn_pushv(hcc_worker_task(w), warn_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_astgen_warn_2(HccWorker* w, HccWarnCode warn_code, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	hcc_warn_pushv(hcc_worker_task(w), warn_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_astgen_warn_2_manual(HccWorker* w, HccWarnCode warn_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_warn_pushv(hcc_worker_task(w), warn_code, location, other_location, va_args);
	va_end(va_args);
}

noreturn void hcc_astgen_bail_error_1(HccWorker* w, HccErrorCode error_code, ...) {
	va_list va_args;
	va_start(va_args, error_code);
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);

	hcc_bail(HCC_ERROR_MESSAGES, 0);
}

noreturn void hcc_astgen_bail_error_1_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);

	hcc_bail(HCC_ERROR_MESSAGES, 0);
}

noreturn void hcc_astgen_bail_error_1_merge_apply(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	HccLocation* other_location = hcc_ata_iter_location(w->astgen.token_iter);
	hcc_location_merge_apply(location, other_location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);

	hcc_bail(HCC_ERROR_MESSAGES, 0);
}

noreturn void hcc_astgen_bail_error_2(HccWorker* w, HccErrorCode error_code, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, other_location, va_args);
	va_end(va_args);

	hcc_bail(HCC_ERROR_MESSAGES, 0);
}

noreturn void hcc_astgen_bail_error_2_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, other_location, va_args);
	va_end(va_args);

	hcc_bail(HCC_ERROR_MESSAGES, 0);
}

void hcc_astgen_data_type_found(HccWorker* w, HccDataType data_type) {
	HCC_UNUSED(w);
	HCC_UNUSED(data_type);
	//*hcc_stack_push(w->astgen.ordered_data_types) = data_type;
}

void hcc_astgen_data_type_ensure_compound_type_default_kind(HccWorker* w, HccDataType data_type, HccErrorCode error_code) {
	HccDataType resolved_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, data_type);
	if (HCC_DATA_TYPE_IS_COMPOUND(resolved_data_type)) {
		HccCompoundDataType* d = hcc_compound_data_type_get(w->cu, resolved_data_type);
		if (d->kind != HCC_COMPOUND_DATA_TYPE_KIND_DEFAULT) {
			HccString data_type_name = hcc_data_type_string(w->cu, data_type);
			hcc_astgen_bail_error_1(w, error_code, (int)data_type_name.size, data_type_name.data);
		}
	}
}

void hcc_astgen_data_type_ensure_valid_variable(HccWorker* w, HccDataType data_type, HccErrorCode error_code) {
	HccDataType resolved_data_type = hcc_data_type_strip_all_pointers(w->cu, hcc_decl_resolve_and_strip_qualifiers(w->cu, data_type));
	if (HCC_DATA_TYPE_IS_COMPOUND(resolved_data_type)) {
		HccCompoundDataType* d = hcc_compound_data_type_get(w->cu, resolved_data_type);
		if (d->kind != HCC_COMPOUND_DATA_TYPE_KIND_DEFAULT) {
			HccErrorCode error_code;
			switch (d->kind) {
				case HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE:
				case HCC_COMPOUND_DATA_TYPE_KIND_FRAGMENT_STATE:
				case HCC_COMPOUND_DATA_TYPE_KIND_BUFFER_ELEMENT:
				case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_SET:
					goto ERROR;
				case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_TABLE:
				case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCES:
					break;
			}
		}
	}

	return;
ERROR: {}
	HccString data_type_name = hcc_data_type_string(w->cu, data_type);
	hcc_astgen_bail_error_1(w, error_code, (int)data_type_name.size, data_type_name.data);
}

void hcc_astgen_data_type_ensure_compound_type_has_no_resources(HccWorker* w, HccDataType data_type, HccErrorCode error_code) {
	HccDataType resolved_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, data_type);
	if (HCC_DATA_TYPE_IS_COMPOUND(resolved_data_type)) {
		HccCompoundDataType* d = hcc_compound_data_type_get(w->cu, resolved_data_type);
		if (d->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_RESOURCE) {
			HccString data_type_name = hcc_data_type_string(w->cu, data_type);
			hcc_astgen_bail_error_1(w, error_code, (int)data_type_name.size, data_type_name.data);
		}
	}
}

void hcc_astgen_data_type_ensure_has_no_resources(HccWorker* w, HccDataType data_type, HccErrorCode error_code) {
	if (hcc_data_type_has_resources(w->cu, data_type)) {
		HccString data_type_name = hcc_data_type_string(w->cu, data_type);
		hcc_astgen_bail_error_1(w, error_code, (int)data_type_name.size, data_type_name.data);
	}
}

void hcc_astgen_data_type_ensure_has_no_pointers(HccWorker* w, HccDataType data_type, HccErrorCode error_code) {
	HccDataType resolved_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, data_type);
	if (HCC_DATA_TYPE_IS_COMPOUND(resolved_data_type)) {
		HccCompoundDataType* d = hcc_compound_data_type_get(w->cu, resolved_data_type);
		if (d->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_POINTER) {
			goto ERROR;
		}
	}

	if (HCC_DATA_TYPE_IS_POINTER(resolved_data_type)) {
		goto ERROR;
	}

	return;
ERROR: {}
	HccString data_type_name = hcc_data_type_string(w->cu, data_type);
	hcc_astgen_bail_error_1(w, error_code, (int)data_type_name.size, data_type_name.data);
}

HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name(HccWorker* w, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id) {
	hcc_stack_clear(w->astgen.compound_type_find_fields);
	return hcc_astgen_compound_data_type_find_field_by_name_recursive(w, compound_data_type, identifier_string_id);
}

HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_checked(HccWorker* w, HccDataType data_type, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id) {
	HccCompoundField* field = hcc_astgen_compound_data_type_find_field_by_name(w, compound_data_type, identifier_string_id);
	if (field == NULL) {
		HccString data_type_name = hcc_data_type_string(w->cu, data_type);
		HccString identifier_string = hcc_string_table_get(identifier_string_id);
		hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_CANNOT_FIND_FIELD, compound_data_type->identifier_location, (int)identifier_string.size, identifier_string.data, (int)data_type_name.size, data_type_name.data);
	}
	return field;
}

HccCompoundField* hcc_astgen_compound_data_type_find_field_by_name_recursive(HccWorker* w, HccCompoundDataType* compound_data_type, HccStringId identifier_string_id) {
	HccFieldAccess* access = hcc_stack_push(w->astgen.compound_type_find_fields);
	for (uint32_t field_idx = 0; field_idx < compound_data_type->fields_count; field_idx += 1) {
		HccCompoundField* field = &compound_data_type->fields[field_idx];
		access->data_type = field->data_type;
		access->idx = field_idx;

		if (field->identifier_string_id.idx_plus_one == 0) {
			//
			// we have an anonymous compound data type as a field.
			// so nest down and search for the identifier in there too
			HccCompoundDataType* field_compound_data_type = hcc_compound_data_type_get(w->cu, field->data_type);
			HccCompoundField* nested_field = hcc_astgen_compound_data_type_find_field_by_name_recursive(w, field_compound_data_type, identifier_string_id);
			if (nested_field) {
				return nested_field;
			}
		}

		if (field->identifier_string_id.idx_plus_one == identifier_string_id.idx_plus_one) {
			// found a field so return success
			return field;
		}
	}

	//
	// no luck finding the field, so remove this result and return failure
	hcc_stack_pop(w->astgen.compound_type_find_fields);
	return NULL;
}

void hcc_astgen_static_variable_usage_found(HccWorker* w, HccDecl decl) {
	HCC_UNUSED(w);
	HCC_UNUSED(decl);
#if 0
	bool found = false;
	uint32_t end_idx = hcc_stack_count(w->astgen.function_used_static_variables);
	for (uint32_t idx = w->astgen.function->used_static_variables_start_idx; idx < end_idx; idx += 1) {
		if (w->astgen.function_used_static_variables[idx] == decl) {
			found = true;
			break;
		}
	}
	if (!found) {
		*hcc_stack_push(w->astgen.function_used_static_variables) = decl;
	}
#endif
}

void hcc_astgen_insert_global_declaration(HccWorker* w, HccStringId identifier_string_id, HccDecl decl, HccLocation* location) {
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(w->astgen.ast_file->global_declarations, &identifier_string_id);
	HccDeclEntry* entry = &w->astgen.ast_file->global_declarations[insert.idx];
	if (!insert.is_new) {
		HccString string = hcc_string_table_get(identifier_string_id);
		hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_INTERNAL, location, entry->location, (int)string.size, string.data);
	}
	entry->decl = decl;
	entry->location = location;
}

void hcc_astgen_eval_cast(HccWorker* w, HccASTExpr* expr, HccDataType dst_data_type) {
	HCC_DEBUG_ASSERT(expr->type == HCC_AST_EXPR_TYPE_CONSTANT, "internal error: expected to be evaluating a constant");
	HccConstant constant = hcc_constant_table_get(w->cu, expr->constant.id);

	HccBasic basic;
	if (HCC_DATA_TYPE_TYPE(expr->data_type) == HCC_DATA_TYPE_AST_BASIC) {
		HccASTBasicDataType basic_data_type = HCC_DATA_TYPE_AUX(expr->data_type);
		if (HCC_AST_BASIC_DATA_TYPE_IS_UINT(w->cu, basic_data_type)) {
			uint64_t uint;
			hcc_constant_as_uint(w->cu, constant, &uint);
			basic = hcc_basic_from_uint(w->cu, dst_data_type, uint);
		} else if (HCC_AST_BASIC_DATA_TYPE_IS_SINT(w->cu, basic_data_type)) {
			int64_t sint;
			hcc_constant_as_sint(w->cu, constant, &sint);
			basic = hcc_basic_from_sint(w->cu, dst_data_type, sint);
		} else if (HCC_AST_BASIC_DATA_TYPE_IS_FLOAT(basic_data_type)) {
			double float_;
			hcc_constant_as_float(w->cu, constant, &float_);
			basic = hcc_basic_from_float(w->cu, dst_data_type, float_);
		} else if (basic_data_type == HCC_AST_BASIC_DATA_TYPE_BOOL) {
			uint64_t uint;
			hcc_constant_as_uint(w->cu, constant, &uint);
			uint &= 0x1;
			basic = hcc_basic_from_uint(w->cu, dst_data_type, uint);
		} else {
			HCC_ABORT("unhandled data type '%u'", expr->data_type);
		}
	} else {
		HCC_ABORT("unhandled data type '%u'", expr->data_type);
	}

	expr->constant.id = hcc_constant_table_deduplicate_basic(w->cu, dst_data_type, &basic);
	expr->data_type = dst_data_type;
}

HccASTExpr* hcc_astgen_alloc_expr(HccWorker* w, HccASTExprType type) {
	HccASTExpr* expr = hcc_stack_push(w->cu->ast.exprs);
	expr->type = type;
	return expr;
}

HccHash64 hcc_astgen_hash_compound_data_type_field(HccCU* cu, HccDataType data_type, HccHash64 hash) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(cu, data_type);

	if (HCC_DATA_TYPE_IS_POINTER(data_type)) {
		// don't hash pointers as the same because they can be for forward declarations which will mess up the hash.
		data_type = HCC_DATA_TYPE_POINTER;
	}

	if (!HCC_DATA_TYPE_IS_COMPOUND(data_type)) {
		return hcc_hash_fnv_64(&data_type, sizeof(data_type), hash);
	}

	//
	// hash all of the compound fields data types but recursively scope in to nested compound data types.
	// this solves the problem where anonymous compound data types would give back a different hash
	HccCompoundDataType* cdt = hcc_compound_data_type_get(cu, data_type);
	hash = hcc_hash_fnv_64(&cdt->size, sizeof(cdt->size), hash);
	hash = hcc_hash_fnv_64(&cdt->align, sizeof(cdt->align), hash);
	hash = hcc_hash_fnv_64(&cdt->fields_count, sizeof(cdt->fields_count), hash);
	for (uint32_t field_idx = 0; field_idx < cdt->fields_count; field_idx += 1) {
		HccCompoundField* field = &cdt->fields[field_idx];
		hash = hcc_astgen_hash_compound_data_type_field(cu, field->data_type, hash);
	}

	return hash;
}

const char* hcc_astgen_type_specifier_string(HccASTGenTypeSpecifier specifier) {
	switch (specifier) {
		case HCC_ASTGEN_TYPE_SPECIFIER_VOID: return "void";
		case HCC_ASTGEN_TYPE_SPECIFIER_BOOL: return "_Bool";
		case HCC_ASTGEN_TYPE_SPECIFIER_CHAR: return "char";
		case HCC_ASTGEN_TYPE_SPECIFIER_SHORT: return "short";
		case HCC_ASTGEN_TYPE_SPECIFIER_INT: return "int";
		case HCC_ASTGEN_TYPE_SPECIFIER_LONG: return "long";
		case HCC_ASTGEN_TYPE_SPECIFIER_LONGLONG: return "long long";
		case HCC_ASTGEN_TYPE_SPECIFIER_FLOAT: return "float";
		case HCC_ASTGEN_TYPE_SPECIFIER_DOUBLE: return "double";
		case HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED: return "unsigned";
		case HCC_ASTGEN_TYPE_SPECIFIER_SIGNED: return "signed";
		case HCC_ASTGEN_TYPE_SPECIFIER_COMPLEX: return "_Complex";
		case HCC_ASTGEN_TYPE_SPECIFIER_ATOMIC: return "_Atomic";
		default: return NULL;
	}
}

void hcc_astgen_data_type_ensure_is_condition(HccWorker* w, HccDataType data_type) {
	data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, data_type);
	if (!hcc_data_type_is_condition(data_type)) {
		HccString data_type_name = hcc_data_type_string(w->cu, data_type);
		hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_CONDITION, (int)data_type_name.size, data_type_name.data);
	}
}

void hcc_astgen_compound_data_type_validate_field_names(HccWorker* w, HccDataType outer_data_type, HccCompoundDataType* compound_data_type) {
	for (uint32_t field_idx = 0; field_idx < compound_data_type->fields_count; field_idx += 1) {
		HccCompoundField* field = &compound_data_type->fields[field_idx];
		if (field->identifier_string_id.idx_plus_one == 0) {
			HccCompoundDataType* field_compound_data_type = hcc_compound_data_type_get(w->cu, field->data_type);
			hcc_astgen_compound_data_type_validate_field_names(w, outer_data_type, field_compound_data_type);
		} else {
			uint32_t found_id = 0;
			for (uint32_t idx = 0; idx < hcc_stack_count(w->astgen.compound_field_names); idx += 1) {
				if (w->astgen.compound_field_names[idx].idx_plus_one == field->identifier_string_id.idx_plus_one) {
					found_id = idx + 1;
					break;
				}
			}
			if (found_id) {
				HccString field_identifier_string = hcc_string_table_get(field->identifier_string_id);
				HccString data_type_name = hcc_data_type_string(w->cu, outer_data_type);
				hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_DUPLICATE_FIELD_IDENTIFIER, field->identifier_location, w->astgen.compound_field_locations[found_id - 1], (int)field_identifier_string.size, field_identifier_string.data, (int)data_type_name.size, data_type_name.data);
			} else {
				*hcc_stack_push(w->astgen.compound_field_names) = field->identifier_string_id;
				*hcc_stack_push(w->astgen.compound_field_locations) = field->identifier_location;
			}
		}
	}
}

void hcc_astgen_validate_specifiers(HccWorker* w, HccASTGenSpecifierFlags non_specifiers, HccErrorCode invalid_specifier_error_code) {
	while (w->astgen.specifier_flags & non_specifiers) {
		HccASTGenSpecifier specifier = hcc_leastsetbitidx32(w->astgen.specifier_flags & non_specifiers);
		HccATAToken token = hcc_astgen_specifier_tokens[specifier];
		hcc_astgen_error_1(w, invalid_specifier_error_code, hcc_ata_token_strings[token]);
		w->astgen.specifier_flags = HCC_LEAST_SET_BIT_REMOVE(w->astgen.specifier_flags);
	}
}

void hcc_astgen_ensure_semicolon(HccWorker* w) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_SEMICOLON) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_MISSING_SEMICOLON);
	}
	hcc_ata_iter_next(w->astgen.token_iter);
}

void hcc_astgen_ensure_not_unsupported_basic_type(HccWorker* w, HccLocation* location, HccDataType data_type) {
	if (w->astgen.function == NULL) {
		return;
	}

	data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);
	HccAMLScalarDataTypeMask mask = hcc_data_type_scalar_data_types_mask(w->cu, data_type) & ~w->cu->supported_scalar_data_types_mask;
	if (mask) {
		HccLocation* other_location = hcc_data_type_location(w->cu, data_type);
		HccString data_type_string = hcc_data_type_string(w->cu, data_type);
		HccString mask_string = hcc_aml_scalar_data_type_mask_string(mask);

		HccASTUnsupportedIntrinsicTypeUsed* used = hcc_stack_push_thread_safe(w->cu->ast.unsupported_intrinsic_type_used);
		used->location = location;
		used->data_type = data_type;
		used->next_id = w->astgen.function->unsupported_intrinsic_types_first_id;

		w->astgen.function->unsupported_intrinsic_types_first_id = (used - w->cu->ast.unsupported_intrinsic_type_used) + 1;
	}
}

bool hcc_astgen_data_type_check_compatible_assignment(HccWorker* w, HccDataType target_data_type, HccASTExpr** source_expr_mut) {
	HccASTExpr* source_expr = *source_expr_mut;
	HccDataType source_data_type = source_expr->data_type;
	if (HCC_DATA_TYPE_IS_CONST(target_data_type)) {
		return false;
	}

	target_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, target_data_type);
	source_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, source_data_type);
	if (target_data_type == source_data_type) {
		return true;
	}

	if (HCC_DATA_TYPE_IS_AST_BASIC(target_data_type) && HCC_DATA_TYPE_IS_AST_BASIC(source_data_type)) {
		hcc_astgen_generate_implicit_cast(w, target_data_type, source_expr_mut);
		return true;
	}

	return false;
}

void hcc_astgen_data_type_ensure_compatible_assignment(HccWorker* w, HccLocation* other_location, HccDataType target_data_type, HccASTExpr** source_expr_mut) {
	if (!hcc_astgen_data_type_check_compatible_assignment(w, target_data_type, source_expr_mut)) {
		HccString target_data_type_name = hcc_data_type_string(w->cu, target_data_type);
		HccString source_data_type_name = hcc_data_type_string(w->cu, (*source_expr_mut)->data_type);
		hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_TYPE_MISMATCH_IMPLICIT_CAST, other_location, (int)source_data_type_name.size, source_data_type_name.data, (int)target_data_type_name.size, target_data_type_name.data);
	}
}

bool hcc_astgen_data_type_check_compatible_arithmetic(HccWorker* w, HccASTExpr** left_expr_mut, HccASTExpr** right_expr_mut) {
	HccASTExpr* left_expr = *left_expr_mut;
	HccASTExpr* right_expr = *right_expr_mut;

	HccDataType left_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, left_expr->data_type);
	HccDataType right_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, right_expr->data_type);
	if (left_data_type == right_data_type) {
		return true;
	}

	if (HCC_DATA_TYPE_IS_AST_BASIC(left_data_type) && HCC_DATA_TYPE_IS_AST_BASIC(right_data_type)) {
		HccASTBasicDataType left_basic_data_type = HCC_DATA_TYPE_AUX(left_data_type);
		HccASTBasicDataType right_basic_data_type = HCC_DATA_TYPE_AUX(right_data_type);

		bool left_is_float = HCC_AST_BASIC_DATA_TYPE_IS_FLOAT(left_basic_data_type);
		bool right_is_float = HCC_AST_BASIC_DATA_TYPE_IS_FLOAT(right_basic_data_type);
		uint8_t left_rank = hcc_ast_data_type_basic_type_ranks[left_basic_data_type];
		uint8_t right_rank = hcc_ast_data_type_basic_type_ranks[right_basic_data_type];
		if (left_is_float || right_is_float) {
			//
			// if one of operands is a float, then cast lower ranked operand
			// into the type of the higher ranked operand
			if (left_rank < right_rank) {
				hcc_astgen_generate_implicit_cast(w, right_data_type, left_expr_mut);
			} else if (left_rank > right_rank) {
				hcc_astgen_generate_implicit_cast(w, left_data_type, right_expr_mut);
			}
		} else {
			//
			// both operands are integers
			//

			{
				//
				// promote each operand to an int if it has a lower rank
				//

				uint8_t int_rank = hcc_ast_data_type_basic_type_ranks[HCC_AST_BASIC_DATA_TYPE_SINT];
				if (left_rank < int_rank) {
					hcc_astgen_generate_implicit_cast(w, HCC_DATA_TYPE_AST_BASIC_SINT, left_expr_mut);
					left_basic_data_type = HCC_AST_BASIC_DATA_TYPE_SINT;
					left_data_type = HCC_DATA_TYPE_AST_BASIC_SINT;
					left_rank = int_rank;
				}

				if (right_rank < int_rank) {
					hcc_astgen_generate_implicit_cast(w, HCC_DATA_TYPE_AST_BASIC_SINT, right_expr_mut);
					right_basic_data_type = HCC_AST_BASIC_DATA_TYPE_SINT;
					right_data_type = HCC_DATA_TYPE_AST_BASIC_SINT;
					right_rank = int_rank;
				}
			}

			if (left_basic_data_type != right_basic_data_type) {
				bool left_is_unsigned = HCC_AST_BASIC_DATA_TYPE_IS_UINT(w->cu, left_basic_data_type);
				bool right_is_unsigned = HCC_AST_BASIC_DATA_TYPE_IS_UINT(w->cu, right_basic_data_type);
				if (left_is_unsigned == right_is_unsigned) {
					//
					// both types are either unsigned or signed,
					// so cast the type with the lower rank to the higher ranked type.
					//

					if (left_rank < right_rank) {
						hcc_astgen_generate_implicit_cast(w, right_data_type, left_expr_mut);
					} else {
						hcc_astgen_generate_implicit_cast(w, left_data_type, right_expr_mut);
					}

					return true;
				}

				//
				// from here one of the operands is unsigned and the other is signed.
				//

				//
				// if unsigned operand has a higher or equal rank to the signed type,
				// then convert the other operand into the unsigned data type.
				//
				if (right_is_unsigned && left_rank <= right_rank) {
					hcc_astgen_generate_implicit_cast(w, right_data_type, left_expr_mut);
					return true;
				} else if (left_is_unsigned && left_rank >= right_rank) {
					hcc_astgen_generate_implicit_cast(w, left_data_type, right_expr_mut);
					return true;
				}

				//
				// check to see if the unsigned type can fit in the other signed type.
				// if it can, convert it into that.
				//
				if (left_is_unsigned && w->cu->dtt.basic_type_size_and_aligns[left_basic_data_type] < w->cu->dtt.basic_type_size_and_aligns[right_basic_data_type]) {
					hcc_astgen_generate_implicit_cast(w, right_data_type, left_expr_mut);
					return true;
				} else if (right_is_unsigned && w->cu->dtt.basic_type_size_and_aligns[right_basic_data_type] < w->cu->dtt.basic_type_size_and_aligns[left_basic_data_type]) {
					hcc_astgen_generate_implicit_cast(w, left_data_type, right_expr_mut);
					return true;
				}

				//
				// all of the other conversion have failed, so convert both expressions to
				// the unsigned version of the signed data type.
				//
				HccDataType unsigned_data_type;
				if (left_is_unsigned) {
					unsigned_data_type = hcc_data_type_signed_to_unsigned(w->cu, right_data_type);
				} else {
					unsigned_data_type = hcc_data_type_signed_to_unsigned(w->cu, left_data_type);
				}
				hcc_astgen_generate_implicit_cast(w, unsigned_data_type, left_expr_mut);
				hcc_astgen_generate_implicit_cast(w, unsigned_data_type, right_expr_mut);
			}
		}

		return true;
	}

	return false;
}

void hcc_astgen_data_type_ensure_compatible_arithmetic(HccWorker* w, HccLocation* other_location, HccASTExpr** left_expr_mut, HccASTExpr** right_expr_mut, HccATAToken operator_token) {
	if (!hcc_astgen_data_type_check_compatible_arithmetic(w, left_expr_mut, right_expr_mut)) {
		HccString left_data_type_name = hcc_data_type_string(w->cu, (*left_expr_mut)->data_type);
		HccString right_data_type_name = hcc_data_type_string(w->cu, (*right_expr_mut)->data_type);
		hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_UNSUPPORTED_BINARY_OPERATOR, other_location, hcc_ata_token_strings[operator_token], (int)right_data_type_name.size, right_data_type_name.data, (int)left_data_type_name.size, left_data_type_name.data);
	}
}

void hcc_astgen_ensure_function_args_count(HccWorker* w, HccDecl function_decl, uint32_t args_count) {
	uint32_t params_count = hcc_decl_function_params_count(w->cu, function_decl);
	if (args_count < params_count) {
		HccString string = hcc_string_table_get(hcc_decl_identifier_string_id(w->cu, function_decl));
		hcc_astgen_error_2(w, HCC_ERROR_CODE_NOT_ENOUGH_FUNCTION_ARGS, hcc_decl_location(w->cu, function_decl), params_count, args_count, (int)string.size, string.data);
	} else if (args_count > params_count) {
		HccString string = hcc_string_table_get(hcc_decl_identifier_string_id(w->cu, function_decl));
		hcc_astgen_error_2(w, HCC_ERROR_CODE_TOO_MANY_FUNCTION_ARGS, hcc_decl_location(w->cu, function_decl), params_count, args_count, (int)string.size, string.data);
	}
}

HccDataType hcc_astgen_deduplicate_array_data_type(HccWorker* w, HccDataType element_data_type, HccConstantId element_count_constant_id) {
	HccCU* cu = w->cu;
	element_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, element_data_type);
	uint64_t key = (uint64_t)element_data_type | ((uint64_t)element_count_constant_id.idx_plus_one << 32);
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->dtt.arrays_dedup_hash_table, &key);
	HccDataTypeDedupEntry* entry = &cu->dtt.arrays_dedup_hash_table[insert.idx];
	if (!insert.is_new) {
		uint32_t id;
		while ((id = atomic_load(&entry->id)) == 0) {
			HCC_CPU_RELAX();
		}

		return HCC_DATA_TYPE(ARRAY, id - 1);
	}

	HccArrayDataType* d = hcc_stack_push_thread_safe(cu->dtt.arrays);
	d->element_data_type = element_data_type;
	d->element_count_constant_id = element_count_constant_id;

	if (HCC_DATA_TYPE_IS_COMPOUND(element_data_type)) {
		HccCompoundDataType* field_compound_data_type = hcc_compound_data_type_get(w->cu, element_data_type);
		if (field_compound_data_type->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_RESOURCE) {
			d->has_resource = true;
		}
		if (field_compound_data_type->flags & HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_POINTER) {
			d->has_pointer = true;
		}
	} else if (HCC_DATA_TYPE_IS_RESOURCE(element_data_type)) {
		d->has_resource = true;
	} else if (HCC_DATA_TYPE_IS_POINTER(element_data_type)) {
		d->has_pointer = true;
	} else if (HCC_DATA_TYPE_IS_ARRAY(element_data_type)) {
		HccArrayDataType* other_d = hcc_array_data_type_get(w->cu, element_data_type);
		d->has_resource |= other_d->has_resource;
		d->has_pointer |= other_d->has_pointer;
	}

	uint32_t array_data_types_idx = d - cu->dtt.arrays;
	atomic_store(&entry->id, array_data_types_idx + 1);
	HccDataType data_type = HCC_DATA_TYPE(ARRAY, array_data_types_idx);
	hcc_astgen_data_type_found(w, data_type);
	return data_type;
}

HccDataType hcc_astgen_deduplicate_pointer_data_type(HccWorker* w, HccDataType element_data_type) {
	HccCU* cu = w->cu;
	element_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, element_data_type);
	uint64_t key = element_data_type;
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->dtt.pointers_dedup_hash_table, &key);
	HccDataTypeDedupEntry* entry = &cu->dtt.pointers_dedup_hash_table[insert.idx];
	if (!insert.is_new) {
		uint32_t id;
		while ((id = atomic_load(&entry->id)) == 0) {
			HCC_CPU_RELAX();
		}

		return HCC_DATA_TYPE(POINTER, id - 1);
	}

	HccPointerDataType* d = hcc_stack_push_thread_safe(cu->dtt.pointers);
	d->element_data_type = element_data_type;

	uint32_t pointer_idx = d - cu->dtt.pointers;
	atomic_store(&entry->id, pointer_idx + 1);
	HccDataType data_type = HCC_DATA_TYPE(POINTER, pointer_idx);
	return data_type;
}

HccDataType hcc_astgen_deduplicate_constbuffer_data_type(HccWorker* w, HccDataType element_data_type) {
	HccCU* cu = w->cu;
	element_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, element_data_type);
	uint64_t key = (uint64_t)element_data_type | ((uint64_t)HCC_RESOURCE_DATA_TYPE_CONSTBUFFER << 32);
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->dtt.buffers_dedup_hash_table, &key);
	HccDataTypeDedupEntry* entry = &cu->dtt.buffers_dedup_hash_table[insert.idx];
	if (!insert.is_new) {
		uint32_t id;
		while ((id = atomic_load(&entry->id)) == 0) {
			HCC_CPU_RELAX();
		}

		return HCC_DATA_TYPE(RESOURCE, HCC_RESOURCE_DATA_TYPE_CONSTBUFFER(id - 1));
	}

	HccBufferDataType* d = hcc_stack_push_thread_safe(cu->dtt.buffers);
	d->element_data_type = element_data_type;

	uint32_t buffer_idx = d - cu->dtt.buffers;
	atomic_store(&entry->id, buffer_idx + 1);
	HccDataType data_type = HCC_DATA_TYPE(RESOURCE, HCC_RESOURCE_DATA_TYPE_CONSTBUFFER(buffer_idx));
	return data_type;
}

HccDataType hcc_astgen_deduplicate_buffer_data_type(HccWorker* w, HccDataType element_data_type) {
	HccCU* cu = w->cu;
	element_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, element_data_type);
	uint64_t key = (uint64_t)element_data_type | ((uint64_t)HCC_RESOURCE_DATA_TYPE_BUFFER << 32);
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->dtt.buffers_dedup_hash_table, &key);
	HccDataTypeDedupEntry* entry = &cu->dtt.buffers_dedup_hash_table[insert.idx];
	if (!insert.is_new) {
		uint32_t id;
		while ((id = atomic_load(&entry->id)) == 0) {
			HCC_CPU_RELAX();
		}

		return HCC_DATA_TYPE(RESOURCE, HCC_RESOURCE_DATA_TYPE_BUFFER(id - 1));
	}

	HccBufferDataType* d = hcc_stack_push_thread_safe(cu->dtt.buffers);
	d->element_data_type = element_data_type;

	uint32_t buffer_idx = d - cu->dtt.buffers;
	atomic_store(&entry->id, buffer_idx + 1);
	HccDataType data_type = HCC_DATA_TYPE(RESOURCE, HCC_RESOURCE_DATA_TYPE_BUFFER(buffer_idx));
	return data_type;
}

void _hcc_astgen_ensure_no_unused_specifiers(HccWorker* w, char* what) {
	if (w->astgen.specifier_flags) {
		HccATAToken keyword_token;
		if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_STATIC) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_STATIC;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_EXTERN) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_EXTERN;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_THREAD_LOCAL) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_THREAD_LOCAL;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_INLINE) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_INLINE;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_NO_RETURN) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_NO_RETURN;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_RASTERIZER_STATE) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_RASTERIZER_STATE;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_BUFFER_ELEMENT) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_BUFFER_ELEMENT;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_SET) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_RESOURCE_SET;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_TABLE) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_RESOURCE_TABLE;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCES) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_RESOURCES;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT_STATE) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_FRAGMENT_STATE;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_POSITION) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_POSITION;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_NOINTERP) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_NOINTERP;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_VERTEX) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_VERTEX;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT) {
			keyword_token = HCC_ATA_TOKEN_KEYWORD_FRAGMENT;
		}
		hcc_astgen_error_1(w, HCC_ERROR_CODE_UNUSED_SPECIFIER, hcc_ata_token_strings[keyword_token], what, hcc_ata_token_strings[hcc_ata_iter_peek(w->astgen.token_iter)]);
	}
}

void hcc_astgen_ensure_no_unused_specifiers_data_type(HccWorker* w) {
	_hcc_astgen_ensure_no_unused_specifiers(w, "a data type");
}

void hcc_astgen_ensure_no_unused_specifiers_identifier(HccWorker* w) {
	_hcc_astgen_ensure_no_unused_specifiers(w, "an identifier");
}

void hcc_astgen_variable_stack_open(HccWorker* w) {
	if (hcc_stack_count(w->astgen.variable_stack_strings) == 0) {
		w->astgen.next_var_idx = 0;
	}
	hcc_stack_push(w->astgen.variable_stack_strings)->idx_plus_one = 0;
	*hcc_stack_push(w->astgen.variable_stack_var_indices) = UINT32_MAX;
}

void hcc_astgen_variable_stack_close(HccWorker* w) {
	while (hcc_stack_count(w->astgen.variable_stack_strings)) {
		bool end = hcc_stack_get_last(w->astgen.variable_stack_strings)->idx_plus_one == 0;
		hcc_stack_pop(w->astgen.variable_stack_strings);
		hcc_stack_pop(w->astgen.variable_stack_var_indices);
		if (end) {
			break;
		}
	}
}

uint32_t hcc_astgen_variable_stack_add(HccWorker* w, HccStringId string_id) {
	uint32_t var_idx = w->astgen.next_var_idx;
	*hcc_stack_push(w->astgen.variable_stack_strings) = string_id;
	*hcc_stack_push(w->astgen.variable_stack_var_indices) = var_idx;
	w->astgen.next_var_idx += 1;
	return var_idx;
}

uint32_t hcc_astgen_variable_stack_find(HccWorker* w, HccStringId string_id) {
	HCC_DEBUG_ASSERT(string_id.idx_plus_one, "string id is null");
	for (uint32_t idx = hcc_stack_count(w->astgen.variable_stack_strings); idx-- > 0;) {
		if (hcc_stack_get(w->astgen.variable_stack_strings, idx)->idx_plus_one == string_id.idx_plus_one) {
			return *hcc_stack_get(w->astgen.variable_stack_var_indices, idx) + 1;
		}
	}
	return 0;
}

HccATAToken hcc_astgen_curly_initializer_start(HccWorker* w, HccDataType data_type, HccDataType resolved_data_type, HccASTExpr* first_expr) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
	gen->elmt_data_type = data_type;
	gen->resolved_elmt_data_type = resolved_data_type;

	if (gen->first_initializer_expr) {
		HccASTGenCurlyInitializerNested* nested = hcc_stack_push(gen->nested);
		nested->first_initializer_expr = gen->first_initializer_expr;
		nested->prev_initializer_expr = gen->prev_initializer_expr;
		nested->nested_elmts_start_idx = gen->nested_elmts_start_idx;
	}

	//
	// start the initializer expression linked list with
	// a designated initializer to zero the whole initial composite type.
	gen->first_initializer_expr = first_expr;
	gen->prev_initializer_expr = first_expr;
	gen->nested_elmts_start_idx = hcc_stack_count(gen->nested_elmts);
	HccASTExpr* initializer_expr = hcc_astgen_curly_initializer_generate_designated_initializer(w, hcc_ata_iter_location(w->astgen.token_iter));
	initializer_expr->designated_initializer.value_expr = NULL;

	return hcc_astgen_curly_initializer_open(w);
}

HccATAToken hcc_astgen_curly_initializer_open(HccWorker* w) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
	if (!HCC_DATA_TYPE_IS_COMPOSITE(gen->resolved_elmt_data_type)) {
		if (gen->elmt_data_type == HCC_DATA_TYPE_AST_BASIC_VOID) {
			// this is only happens for the first opening initializer when it is not used for an assignment.
			hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_CURLY_EXPR);
		} else {
			HccString data_type_name = hcc_data_type_string(w->cu, gen->elmt_data_type);
			hcc_astgen_warn_1(w, HCC_WARN_CODE_CURLY_INITIALIZER_ON_SCALAR, (int)data_type_name.size, data_type_name.data);
		}
	}

	*hcc_stack_push(gen->nested_curlys) = (HccASTGenCurlyInitializerCurly) {
		.nested_elmts_start_idx = hcc_stack_count(gen->nested_elmts) + 1,
		.found_designator = false,
	};

	hcc_astgen_curly_initializer_tunnel_in(w);
	return hcc_ata_iter_next(w->astgen.token_iter);
}

HccATAToken hcc_astgen_curly_initializer_close(HccWorker* w, bool is_finished) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;

	//
	// restore the elements back to where the parent curly initializer was inserted.
	// so we start searching from the parent curly initializer.
	uint32_t new_elmts_count = hcc_stack_get_last(gen->nested_curlys)->nested_elmts_start_idx - 1;
	hcc_stack_resize(gen->nested_elmts, new_elmts_count);
	hcc_stack_pop(gen->nested_curlys);

	if (is_finished) {
		return 0;
	}

	HccASTGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
	hcc_astgen_curly_initializer_set_composite(w, nested_elmt->data_type, nested_elmt->resolved_data_type);

	return hcc_ata_iter_next(w->astgen.token_iter);
}

bool hcc_astgen_curly_initializer_next_elmt(HccWorker* w, HccDataType resolved_target_data_type) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;

	//
	// tunnel in and out the composite data types until we reach the next non composite data type
	while (1) {
		HccASTGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
		HccASTGenCurlyInitializerCurly* curly = hcc_stack_get_last(gen->nested_curlys);
		bool had_explicit_designator_for_union_field = nested_elmt->had_explicit_designator_for_union_field;

		nested_elmt->elmt_idx += 1;
		nested_elmt->had_explicit_designator_for_union_field = false;

		if (!had_explicit_designator_for_union_field && nested_elmt->elmt_idx >= gen->elmts_end_idx) {
			//
			// the end of this initializer/data_type has been reached.
			// it could be either a composite or non composite data type.
			//

			if (curly->nested_elmts_start_idx == hcc_stack_count(gen->nested_elmts)) {
				//
				// here we have reached the end of the initializers for the explicit curly braces { }.
				// so we have a initializer that is targeting nothing.
				HccString data_type_name = hcc_data_type_string(w->cu, gen->composite_data_type);
				hcc_astgen_warn_1(w, HCC_WARN_CODE_UNUSED_INITIALIZER_REACHED_END, (int)data_type_name.size, data_type_name.data);
				return false;
			} else {
				//
				// otherwise we are tunneled inside a nested composite data type.
				// which means one of it's parents has explicit curly braces.
				// so we just tunnel back out of our nested composite data type.
				hcc_astgen_curly_initializer_tunnel_out(w);
			}
		} else {
			if (HCC_DATA_TYPE_IS_COMPOUND(gen->resolved_composite_data_type)) {
				//
				// this is a compound data type so fetch the next element data type
				gen->elmt_data_type = gen->compound_fields[nested_elmt->elmt_idx].data_type;
				gen->resolved_elmt_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, gen->elmt_data_type);
			}

			if (resolved_target_data_type == gen->resolved_elmt_data_type) {
				return true;
			}

			if (resolved_target_data_type == HCC_DATA_TYPE_AST_BASIC_VOID) {
				return true;
			}

			if (HCC_DATA_TYPE_IS_COMPOSITE(gen->resolved_elmt_data_type)) {
				hcc_astgen_curly_initializer_tunnel_in(w);
			} else {
				//
				// we found the non composite data type
				return true;
			}
		}
	}
}

HccATAToken hcc_astgen_curly_initializer_next_elmt_with_designator(HccWorker* w) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	HCC_DEBUG_ASSERT(token == HCC_ATA_TOKEN_FULL_STOP || token == HCC_ATA_TOKEN_SQUARE_OPEN, "internal error: expected '.' or '['");
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
	HccASTGenCurlyInitializerCurly* curly = hcc_stack_get_last(gen->nested_curlys);
	curly->found_designator = true;

	//
	// remove all of the excess nested elements so we are back on
	// the open curly braces that we are in.
	hcc_stack_resize(gen->nested_elmts, curly->nested_elmts_start_idx);
	HccASTGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
	hcc_astgen_curly_initializer_set_composite(w, nested_elmt->data_type, nested_elmt->resolved_data_type);

	//
	// parse and process the chain of designators for the composite types
	// eg: .field[0].another
	while (1) {
		switch (token) {
			case HCC_ATA_TOKEN_FULL_STOP:
				if (HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					HccString data_type_name = hcc_data_type_string(w->cu, gen->composite_data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_FIELD_DESIGNATOR_ON_ARRAY_TYPE, (int)data_type_name.size, data_type_name.data);
				}

				token = hcc_ata_iter_next(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_IDENT) {
					HccString data_type_name = hcc_data_type_string(w->cu, gen->composite_data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_DESIGNATOR, (int)data_type_name.size, data_type_name.data);
				}

				HccStringId identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
				hcc_astgen_compound_data_type_find_field_by_name_checked(w, gen->composite_data_type, gen->compound_data_type, identifier_string_id);
				for (uint32_t i = 0; i < hcc_stack_count(w->astgen.compound_type_find_fields); i += 1) {
					HccFieldAccess* field = hcc_stack_get(w->astgen.compound_type_find_fields, i);
					hcc_stack_get_last(gen->nested_elmts)->elmt_idx = field->idx;
					hcc_astgen_curly_initializer_nested_elmt_push(w, field->data_type, hcc_decl_resolve_and_keep_qualifiers(w->cu, field->data_type));
				}

				if (hcc_stack_count(w->astgen.compound_type_find_fields) > 1) {
					gen->composite_data_type = hcc_stack_get_back(w->astgen.compound_type_find_fields, 1)->data_type;
					gen->resolved_composite_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, gen->composite_data_type);
					gen->compound_data_type = hcc_compound_data_type_get(w->cu, gen->resolved_composite_data_type);
					gen->compound_fields = gen->compound_data_type->fields;
				}

				gen->elmt_data_type = hcc_stack_get_last(w->astgen.compound_type_find_fields)->data_type;
				gen->resolved_elmt_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, gen->elmt_data_type);
				token = hcc_ata_iter_next(w->astgen.token_iter);
				break;
			case HCC_ATA_TOKEN_SQUARE_OPEN:
				if (!HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					HccString data_type_name = hcc_data_type_string(w->cu, gen->composite_data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ARRAY_DESIGNATOR_ON_COMPOUND_TYPE, (int)data_type_name.size, data_type_name.data);
				}

				token = hcc_ata_iter_next(w->astgen.token_iter);
				HccASTExpr* expr = hcc_astgen_generate_expr(w, 0);
				if (expr->type != HCC_AST_EXPR_TYPE_CONSTANT || !HCC_DATA_TYPE_IS_AST_BASIC(expr->data_type) || !HCC_AST_BASIC_DATA_TYPE_IS_INT(HCC_DATA_TYPE_AUX(expr->data_type))) {
					HccString data_type_name = hcc_data_type_string(w->cu, gen->composite_data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_INTEGER_FOR_ARRAY_IDX, (int)data_type_name.size, data_type_name.data);
				}

				HccConstant constant = hcc_constant_table_get(w->cu, expr->constant.id);
				uint64_t elmt_idx;
				if (!hcc_constant_as_uint(w->cu, constant, &elmt_idx) || elmt_idx >= gen->elmts_end_idx) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ARRAY_INDEX_OUT_OF_BOUNDS, gen->elmts_end_idx);
				}

				token = hcc_ata_iter_peek(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_SQUARE_CLOSE) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ARRAY_DESIGNATOR_EXPECTED_SQUARE_BRACE_CLOSE);
				}
				token = hcc_ata_iter_next(w->astgen.token_iter);

				hcc_stack_get_last(gen->nested_elmts)->elmt_idx = elmt_idx;
				hcc_astgen_curly_initializer_nested_elmt_push(w, gen->elmt_data_type, gen->resolved_elmt_data_type);
				break;
			case HCC_ATA_TOKEN_EQUAL:
				goto END;
			default: {
				//
				// we reach here when we have looped at least once and we do not have a '=', '.' or '[' with a compatible composite data type
				HccErrorCode error_code;
				if (HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					error_code = HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_ARRAY_DESIGNATOR;
				} else if (HCC_DATA_TYPE_IS_COMPOUND(gen->resolved_composite_data_type)) {
					error_code = HCC_ERROR_CODE_EXPECTED_ASSIGN_OR_FIELD_DESIGNATOR;
				} else {
					HccString data_type_name = hcc_data_type_string(w->cu, gen->composite_data_type);
					HCC_UNREACHABLE("we only handle array and compound types here right? but we got '%.*s'", (int)data_type_name.size, data_type_name.data);
				}
				hcc_astgen_bail_error_1(w, error_code);
			};
		}

		if (token == HCC_ATA_TOKEN_EQUAL) {
			// we reach here after processing a array or field designator that is followed by a '='
			goto END;
		} else if (!HCC_DATA_TYPE_IS_COMPOSITE(gen->resolved_elmt_data_type)) {
			HccString data_type_name = hcc_data_type_string(w->cu, gen->elmt_data_type);
			hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_ASSIGN, (int)data_type_name.size, data_type_name.data);
		}

		hcc_astgen_curly_initializer_set_composite(w, gen->elmt_data_type, gen->resolved_elmt_data_type);
	}
END: {}
	token = hcc_ata_iter_next(w->astgen.token_iter);

	//
	// pop off the last nested element since we don't
	// want to tunnel into the last designator in the chain.
	hcc_stack_pop(gen->nested_elmts);

	if (token != HCC_ATA_TOKEN_CURLY_OPEN) {
		// take away one as the next call to hcc_astgen_curly_initializer_next_elmt
		// will increment it past where the designator has specified.
		nested_elmt = hcc_stack_get_last(gen->nested_elmts);
		nested_elmt->elmt_idx -= 1;
		nested_elmt->had_explicit_designator_for_union_field = HCC_DATA_TYPE_IS_UNION(gen->resolved_composite_data_type);
	}
	return token;
}

void hcc_astgen_curly_initializer_nested_elmt_push(HccWorker* w, HccDataType data_type, HccDataType resolved_data_type) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
	*hcc_stack_push(gen->nested_elmts) = (HccASTGenCurlyInitializerElmt) {
		.elmt_idx = -1, // start on -1 so that it'll be 0 after the first call to hcc_astgen_curly_initializer_next_elmt
		.data_type =  data_type,
		.resolved_data_type = resolved_data_type,
	};
}

void hcc_astgen_curly_initializer_tunnel_in(HccWorker* w) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
	hcc_astgen_curly_initializer_nested_elmt_push(w, gen->elmt_data_type, gen->resolved_elmt_data_type);
	hcc_astgen_curly_initializer_set_composite(w, gen->elmt_data_type, gen->resolved_elmt_data_type);
}

void hcc_astgen_curly_initializer_tunnel_out(HccWorker* w) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
	hcc_stack_pop(gen->nested_elmts);
	HccASTGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
	hcc_astgen_curly_initializer_set_composite(w, nested_elmt->data_type, nested_elmt->resolved_data_type);
}

void hcc_astgen_curly_initializer_set_composite(HccWorker* w, HccDataType data_type, HccDataType resolved_data_type) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;

	gen->composite_data_type = data_type;
	gen->resolved_composite_data_type = resolved_data_type;
	if (HCC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
		gen->array_data_type = hcc_array_data_type_get(w->cu, gen->resolved_composite_data_type);

		HccConstant constant = hcc_constant_table_get(w->cu, gen->array_data_type->element_count_constant_id);
		uint64_t cap;
		hcc_constant_as_uint(w->cu, constant, &cap);

		gen->elmt_data_type = gen->array_data_type->element_data_type;
		gen->resolved_elmt_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, gen->elmt_data_type);
		gen->elmts_end_idx = cap;
	} else if (HCC_DATA_TYPE_IS_COMPOUND(gen->resolved_composite_data_type)) {
		gen->compound_data_type = hcc_compound_data_type_get(w->cu, gen->resolved_composite_data_type);
		gen->compound_fields = gen->compound_data_type->fields;
		gen->elmt_data_type = gen->compound_fields[0].data_type;
		gen->resolved_elmt_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, gen->elmt_data_type);

		gen->elmts_end_idx = HCC_DATA_TYPE_IS_UNION(gen->resolved_composite_data_type) ? 1 : gen->compound_data_type->fields_count;
	} else {
		//
		// non composite data type that has explicit curly braces { }.
		gen->elmt_data_type = data_type;
		gen->resolved_elmt_data_type = resolved_data_type;
		gen->elmts_end_idx = 1;
	}
}

HccASTExpr* hcc_astgen_curly_initializer_generate_designated_initializer(HccWorker* w, HccLocation* location) {
	HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
	uint32_t elmt_indices_start_idx = gen->nested_elmts_start_idx;
	uint32_t elmt_indices_end_idx = hcc_stack_count(gen->nested_elmts);
	uint32_t elmt_indices_count = elmt_indices_end_idx - elmt_indices_start_idx;

	//
	// copy the element indices out into the persistant array
	uint32_t dst_elmt_indices_start_idx = hcc_stack_count(w->cu->ast.designated_initializer_elmt_indices);
	uint64_t* elmt_indices = hcc_stack_push_many(w->cu->ast.designated_initializer_elmt_indices, elmt_indices_count);
	for (uint32_t idx = 0; idx < elmt_indices_count; idx += 1) {
		elmt_indices[idx] = hcc_stack_get(gen->nested_elmts, elmt_indices_start_idx + idx)->elmt_idx;
	}

	//
	// create the expression node and reference the auxillary data
	HccASTExpr* initializer_expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_DESIGNATED_INITIALIZER);
	initializer_expr->is_stmt = true;
	initializer_expr->next_stmt = NULL;
	initializer_expr->designated_initializer.elmt_indices_start_idx = dst_elmt_indices_start_idx;
	initializer_expr->designated_initializer.elmts_count = elmt_indices_count;
	initializer_expr->location = location;

	//
	// append to the link list of designated initializers
	if (gen->prev_initializer_expr) {
		gen->prev_initializer_expr->next_stmt = initializer_expr;
	} else {
		gen->first_initializer_expr = initializer_expr;
	}
	gen->prev_initializer_expr = initializer_expr;

	return initializer_expr;
}

HccATAToken hcc_astgen_generate_specifiers(HccWorker* w) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	while (1) {
		HccASTGenSpecifierFlags flag = 0;
		switch (token) {
			case HCC_ATA_TOKEN_KEYWORD_STATIC:           flag = HCC_ASTGEN_SPECIFIER_FLAGS_STATIC;           break;
			case HCC_ATA_TOKEN_KEYWORD_EXTERN:           flag = HCC_ASTGEN_SPECIFIER_FLAGS_EXTERN;           break;
			case HCC_ATA_TOKEN_KEYWORD_THREAD_LOCAL:     flag = HCC_ASTGEN_SPECIFIER_FLAGS_THREAD_LOCAL;     break;
			case HCC_ATA_TOKEN_KEYWORD_INLINE:           flag = HCC_ASTGEN_SPECIFIER_FLAGS_INLINE;           break;
			case HCC_ATA_TOKEN_KEYWORD_NO_RETURN:        flag = HCC_ASTGEN_SPECIFIER_FLAGS_NO_RETURN;        break;
			case HCC_ATA_TOKEN_KEYWORD_RASTERIZER_STATE: flag = HCC_ASTGEN_SPECIFIER_FLAGS_RASTERIZER_STATE; break;
			case HCC_ATA_TOKEN_KEYWORD_BUFFER_ELEMENT:   flag = HCC_ASTGEN_SPECIFIER_FLAGS_BUFFER_ELEMENT;   break;
			case HCC_ATA_TOKEN_KEYWORD_RESOURCE_TABLE:   flag = HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_TABLE;   break;
			case HCC_ATA_TOKEN_KEYWORD_RESOURCES:        flag = HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCES;        break;
			case HCC_ATA_TOKEN_KEYWORD_FRAGMENT_STATE:   flag = HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT_STATE;   break;
			case HCC_ATA_TOKEN_KEYWORD_POSITION:         flag = HCC_ASTGEN_SPECIFIER_FLAGS_POSITION;         break;
			case HCC_ATA_TOKEN_KEYWORD_NOINTERP:         flag = HCC_ASTGEN_SPECIFIER_FLAGS_NOINTERP;         break;
			case HCC_ATA_TOKEN_KEYWORD_VERTEX:           flag = HCC_ASTGEN_SPECIFIER_FLAGS_VERTEX;           break;
			case HCC_ATA_TOKEN_KEYWORD_FRAGMENT:         flag = HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT;         break;
			case HCC_ATA_TOKEN_KEYWORD_AUTO: break;
			case HCC_ATA_TOKEN_KEYWORD_VOLATILE:
			case HCC_ATA_TOKEN_KEYWORD_RESOURCE_SET:
				flag = HCC_ASTGEN_SPECIFIER_FLAGS_RESOURCE_SET;

				token = hcc_ata_iter_next(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_RESOURCE_SET_SLOT);
				}
				token = hcc_ata_iter_next(w->astgen.token_iter);

				uint32_t max_resource_set_slot = hcc_options_get_u32(w->cu->options, HCC_OPTION_KEY_RESOURCE_SET_SLOT_MAX);
				switch (token) {
					case HCC_ATA_TOKEN_LIT_UINT:
					case HCC_ATA_TOKEN_LIT_ULONG:
					case HCC_ATA_TOKEN_LIT_ULONGLONG:
					case HCC_ATA_TOKEN_LIT_SINT:
					case HCC_ATA_TOKEN_LIT_SLONG:
					case HCC_ATA_TOKEN_LIT_SLONGLONG:
						token = hcc_ata_iter_next(w->astgen.token_iter);
						HccConstantId constant_id = hcc_ata_iter_next_value(w->astgen.token_iter).constant_id;
						HccConstant constant = hcc_constant_table_get(w->cu, constant_id);

						//
						// skip the associated HccStringId kept around to turn the
						// literal back into the exact string it was parsed from.
						hcc_ata_iter_next_value(w->astgen.token_iter);
						bool out_of_bounds = false;
						uint64_t resource_set_slot;
						if (hcc_constant_as_uint(w->cu, constant, &resource_set_slot)) {
							out_of_bounds = resource_set_slot < max_resource_set_slot;
						} else {
							out_of_bounds = true;
						}
						if (out_of_bounds) {
							hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_RESOURCE_SET_SLOT_OUT_OF_BOUNDS, max_resource_set_slot, resource_set_slot);
						}
						w->astgen.resource_set_slot = resource_set_slot;
						break;
					default:
						hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_RESOURCE_SET_MUST_BE_A_UINT, max_resource_set_slot);
						break;

				}
				if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_RESOURCE_SET_SLOT);
				}
				token = hcc_ata_iter_next(w->astgen.token_iter);
				break;
			default: return token;
		}

		if (w->astgen.specifier_flags & flag) {
			hcc_astgen_error_1(w, HCC_ERROR_CODE_SPECIFIER_ALREADY_BEEN_USED, hcc_ata_token_strings[token]);
		}
		w->astgen.specifier_flags |= flag;
		token = hcc_ata_iter_next(w->astgen.token_iter);
	}
}

HccDataType hcc_astgen_generate_enum_data_type(HccWorker* w) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	HCC_DEBUG_ASSERT(token == HCC_ATA_TOKEN_KEYWORD_ENUM, "internal error: expected 'enum' but got '%s'", hcc_ata_token_strings[token]);
	token = hcc_ata_iter_next(w->astgen.token_iter);

	HccStringId identifier_string_id = {0};
	HccEnumDataType enum_data_type;
	HccDeclEntry* table_entry = NULL;
	bool is_new = false;
	if (token == HCC_ATA_TOKEN_IDENT) {
		identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
		token = hcc_ata_iter_next(w->astgen.token_iter);

		HccHashTableInsert insert = hcc_hash_table_find_insert_idx(w->astgen.ast_file->enum_declarations, &identifier_string_id);
		table_entry = &w->astgen.ast_file->enum_declarations[insert.idx];
		if (insert.is_new) {
			goto MAKE_NEW;
		}
	} else {
MAKE_NEW: {}
		HCC_ZERO_ELMT(&enum_data_type);
		enum_data_type.identifier_location = hcc_ata_iter_location(w->astgen.token_iter);
		enum_data_type.identifier_string_id = identifier_string_id;
		is_new = true;
	}

	if (token != HCC_ATA_TOKEN_CURLY_OPEN) {
		if (!is_new) {
			return table_entry->decl;
		}

		if (identifier_string_id.idx_plus_one) {
			HccString identifier_string = hcc_string_table_get(identifier_string_id);
			if (token == HCC_ATA_TOKEN_SEMICOLON) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ENUM_GOT_SEMICOLON, (int)identifier_string.size, identifier_string.data, (int)identifier_string.size, identifier_string.data);
			} else {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ENUM, (int)identifier_string.size, identifier_string.data, hcc_ata_token_strings[token]);
			}
		}
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_UNNAMED_ENUM, hcc_ata_token_strings[token]);
	}

	if (!is_new) {
		HccDataType data_type = table_entry->decl;
		HccString data_type_name = hcc_data_type_string(w->cu, data_type);
		hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REIMPLEMENTATION_DATA_TYPE, enum_data_type.identifier_location, hcc_enum_data_type_get(w->cu, data_type)->identifier_location, (int)data_type_name.size, data_type_name.data);
	}

	token = hcc_ata_iter_next(w->astgen.token_iter);
	if (token == HCC_ATA_TOKEN_CURLY_CLOSE) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EMPTY_ENUM);
	}

	HccCU* cu = w->cu;
	enum_data_type.values = hcc_stack_get_next_push(w->astgen.enum_values);
	int64_t next_value = 0;
	while (token != HCC_ATA_TOKEN_CURLY_CLOSE) {
		HccEnumValue* enum_value = hcc_stack_push(w->astgen.enum_values);

		if (token != HCC_ATA_TOKEN_IDENT) {
			hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_ENUM_VALUE);
		}

		if (next_value > INT32_MAX) {
			hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ENUM_VALUE_OVERFLOW);
		}

		HccStringId value_identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
		enum_value->identifier_location = hcc_ata_iter_location(w->astgen.token_iter);
		enum_value->identifier_string_id = value_identifier_string_id;
		enum_data_type.value_identifiers_hash = hcc_hash_fnv_64(&value_identifier_string_id, sizeof(value_identifier_string_id), enum_data_type.value_identifiers_hash);

		token = hcc_ata_iter_next(w->astgen.token_iter);
		bool has_explicit_value = token == HCC_ATA_TOKEN_EQUAL;
		if (has_explicit_value) {
			token = hcc_ata_iter_next(w->astgen.token_iter);

			HccASTExpr* expr = hcc_astgen_generate_expr_no_comma_operator(w, 0);
			if (expr->type != HCC_AST_EXPR_TYPE_CONSTANT || !HCC_DATA_TYPE_IS_AST_BASIC(expr->data_type) || !HCC_AST_BASIC_DATA_TYPE_IS_INT(HCC_DATA_TYPE_AUX(expr->data_type))) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ENUM_VALUE_INVALID_FORMAT);
			}

			HccConstant constant = hcc_constant_table_get(w->cu, expr->constant.id);

			int32_t value;
			if (!hcc_constant_as_sint32(w->cu, constant, &value)) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ENUM_VALUE_INVALID_FORMAT);
			}

			next_value = value;
			token = hcc_ata_iter_peek(w->astgen.token_iter);
		}

		HccBasic v = hcc_basic_from_sint(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, next_value);

		HccConstantId value_constant_id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AST_BASIC_SINT, &v);
		enum_data_type.values_hash = hcc_hash_fnv_64(&value_constant_id, sizeof(value_constant_id), enum_data_type.values_hash);

		enum_value->constant_id = value_constant_id;
		next_value += 1;
		enum_data_type.values_count += 1;

		if (token == HCC_ATA_TOKEN_COMMA) {
			token = hcc_ata_iter_next(w->astgen.token_iter);
		} else if (token != HCC_ATA_TOKEN_CURLY_CLOSE) {
			HccErrorCode error_code = has_explicit_value
				? HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR_WITH_EXPLICIT_VALUE
				: HCC_ERROR_CODE_ENUM_VALUE_INVALID_TERMINATOR;
			hcc_astgen_bail_error_1(w, error_code);
		}
	}

	token = hcc_ata_iter_next(w->astgen.token_iter);

	//
	// add the enum to the compilation unit if an identical version doesn't exist there already
	HccEnumDataType* real_enum_data_type = &enum_data_type;
	HccDataType data_type = 0;
	if (table_entry) {
		HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->enum_declarations, &identifier_string_id);
		HccDeclEntryAtomic* entry = &cu->enum_declarations[insert.idx];

		HccAtomic(HccDeclEntryAtomicLink*)* link_ptr = &entry->link;
		while (1) {
			HccDeclEntryAtomicLink* link = atomic_load(link_ptr);
			if (link == NULL && atomic_compare_exchange_strong(link_ptr, &link, HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL)) {
				//
				// our thread claimed the atomic link which means this enum is unique with this identifier.
				// so now lets setup the enum and atomic link.
				HccEnumValue* dst_enum_values = hcc_stack_push_many_thread_safe(cu->dtt.enum_values, enum_data_type.values_count);
				HCC_COPY_ELMT_MANY(dst_enum_values, enum_data_type.values, enum_data_type.values_count);
				enum_data_type.values = dst_enum_values;

				HccEnumDataType* dst_enum_data_type = hcc_stack_push_thread_safe(cu->dtt.enums);
				*dst_enum_data_type = enum_data_type;

				uint32_t enum_idx = dst_enum_data_type - cu->dtt.enums;
				data_type = HCC_DATA_TYPE(ENUM, enum_idx);

				HccDeclEntryAtomicLink* alloced_link = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccDeclEntryAtomicLink, &w->arena_alctor);
				alloced_link->decl = data_type;
				atomic_store(link_ptr, alloced_link);

				real_enum_data_type = dst_enum_data_type;
				break;
			}

			//
			// wait if any other thread is setting up the atomic link
			while (link == HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL) {
				HCC_CPU_RELAX();
				link = atomic_load(link_ptr);
			}

			HccEnumDataType* other_enum_data_type = hcc_enum_data_type_get(cu, link->decl);
			if (
				enum_data_type.values_count == other_enum_data_type->values_count
				&& enum_data_type.value_identifiers_hash == other_enum_data_type->value_identifiers_hash
				&& enum_data_type.values_hash == other_enum_data_type->values_hash
			) {
				//
				// we have found a matching enum with the same identifier, so lets use this one instead.
				real_enum_data_type = other_enum_data_type;
				data_type = link->decl;
				break;
			}

			link_ptr = &link->next;
		}
	}

	//
	// add all of the enum values to the global declaration table local to this ast file
	uint32_t global_value_start_idx =  real_enum_data_type->values - cu->dtt.enum_values;
	for (uint32_t idx = 0; idx < real_enum_data_type->values_count; idx += 1) {
		HccDecl decl = HCC_DECL(ENUM_VALUE, global_value_start_idx + idx);
		HccEnumValue* enum_value = &real_enum_data_type->values[idx];
		hcc_astgen_insert_global_declaration(w, enum_value->identifier_string_id, decl, enum_value->identifier_location);
	}

	if (table_entry) {
		table_entry->decl = data_type;
	}

	hcc_astgen_data_type_found(w, data_type);
	hcc_stack_pop_many(w->astgen.enum_values, enum_data_type.values_count);
	return data_type ? data_type : HCC_DATA_TYPE_AST_BASIC_SINT;
}

HccDataType hcc_astgen_generate_compound_data_type(HccWorker* w) {
	HccLocation* compound_data_type_location = hcc_ata_iter_location(w->astgen.token_iter);
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	bool is_union = false;
	switch (token) {
		case HCC_ATA_TOKEN_KEYWORD_STRUCT: break;
		case HCC_ATA_TOKEN_KEYWORD_UNION:
			is_union = true;
			break;
		default:
			HCC_UNREACHABLE("internal error: expected 'struct' or 'union' but got '%s'", hcc_ata_token_strings[token]);
	}
	token = hcc_ata_iter_next(w->astgen.token_iter);

	HccDataType data_type = 0;
	HccStringId identifier_string_id = {0};
	HccCompoundDataType compound_data_type;
	uint32_t intrinsic_id = 0;
	HccDeclEntry* table_entry = NULL; // will be NULL when is anonymous
	HccCU* cu = w->cu;
	if (token == HCC_ATA_TOKEN_IDENT) {
		compound_data_type_location = hcc_ata_iter_location(w->astgen.token_iter);
		identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
		token = hcc_ata_iter_next(w->astgen.token_iter);

		HccHashTable(HccDeclEntry) declarations;
		if (is_union) {
			declarations = w->astgen.ast_file->union_declarations;
		} else {
			declarations = w->astgen.ast_file->struct_declarations;
		}

		HccHashTableInsert insert = hcc_hash_table_find_insert_idx(declarations, &identifier_string_id);
		table_entry = &declarations[insert.idx];
		if (!insert.is_new) {
			data_type = table_entry->decl;
		}
	}

	if (token != HCC_ATA_TOKEN_CURLY_OPEN) {
		if (identifier_string_id.idx_plus_one) {
			if (data_type == 0) {
				HccASTForwardDecl* forward_decl = hcc_stack_push_thread_safe(cu->ast.forward_declarations);
				forward_decl->ast_file = w->astgen.ast_file;
				forward_decl->identifier_location = compound_data_type_location;
				forward_decl->identifier_string_id = identifier_string_id;

				uint32_t forward_decl_idx = forward_decl - cu->ast.forward_declarations;
				data_type = is_union ? HCC_DATA_TYPE_FORWARD_DECL(UNION, forward_decl_idx) : HCC_DATA_TYPE_FORWARD_DECL(STRUCT, forward_decl_idx);

				table_entry->decl = data_type;
				table_entry->location = compound_data_type_location;
			}
			return data_type;
		}
		hcc_astgen_bail_error_1(w, is_union ? HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ANON_UNION_TYPE : HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_ANON_STRUCT_TYPE);
	}

	{
		HCC_ZERO_ELMT(&compound_data_type);
		compound_data_type.identifier_location = compound_data_type_location;
		compound_data_type.identifier_string_id = identifier_string_id;
	}

	//
	// process the specifiers
	{
		hcc_astgen_validate_specifiers(w, HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_STRUCT_SPECIFIERS, HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT);
		if (w->astgen.specifier_flags) {
			//
			// ensure only one specifier is enabled
			if (!HCC_IS_POWER_OF_TWO_OR_ZERO(w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS)) {
				hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT, hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_RASTERIZER_STATE], hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_FRAGMENT_STATE]);
			}

			uint32_t idx = hcc_leastsetbitidx32(w->astgen.specifier_flags);
			if (is_union) {
				hcc_astgen_error_1(w, HCC_ERROR_CODE_NOT_AVAILABLE_FOR_UNION, hcc_ata_token_strings[hcc_astgen_specifier_tokens[idx]]);
			}


			switch (idx) {
				case HCC_ASTGEN_SPECIFIER_RASTERIZER_STATE:
					compound_data_type.kind = HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE;
					break;
				case HCC_ASTGEN_SPECIFIER_FRAGMENT_STATE:
					compound_data_type.kind = HCC_COMPOUND_DATA_TYPE_KIND_FRAGMENT_STATE;
					break;
				case HCC_ASTGEN_SPECIFIER_BUFFER_ELEMENT:
					compound_data_type.kind = HCC_COMPOUND_DATA_TYPE_KIND_BUFFER_ELEMENT;
					break;
				case HCC_ASTGEN_SPECIFIER_RESOURCE_SET:
					compound_data_type.kind = HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_SET;
					compound_data_type.resource_set_slot = w->astgen.resource_set_slot;
					break;
				case HCC_ASTGEN_SPECIFIER_RESOURCE_TABLE:
					if (hcc_options_get_u32(cu->options, HCC_OPTION_KEY_TARGET_RESOURCE_MODEL) == HCC_TARGET_RESOURCE_MODEL_BINDING) {
						hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_RESOURCE_TABLE_RESOURCE_MODEL);
					}
					compound_data_type.kind = HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_TABLE;
					break;
				case HCC_ASTGEN_SPECIFIER_RESOURCES:
					compound_data_type.kind = HCC_COMPOUND_DATA_TYPE_KIND_RESOURCES;
					break;
			}
		}

		w->astgen.specifier_flags &= ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_SPECIFIERS;
	}

	if (table_entry && data_type && !HCC_DATA_TYPE_IS_FORWARD_DECL(data_type)) {
		HccString data_type_name = hcc_data_type_string(w->cu, data_type);
		hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REIMPLEMENTATION_DATA_TYPE, compound_data_type.identifier_location, table_entry->location, (int)data_type_name.size, data_type_name.data);
	}

	token = hcc_ata_iter_next(w->astgen.token_iter);

	HccCompoundField* fields = hcc_stack_get_next_push(w->astgen.compound_fields);
	compound_data_type.fields = fields;
	uint32_t field_idx = 0;
	bool found_position = false;
	uint32_t resource_constants_size = 0;
	while (token != HCC_ATA_TOKEN_CURLY_CLOSE) { // for each field
		HccCompoundField* compound_field = hcc_stack_push(w->astgen.compound_fields);
		compound_data_type.fields_count += 1;
		token = hcc_astgen_generate_specifiers(w);

		//
		// process the compound field specifiers
		//
		{
			if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_STRUCT_FIELD_SPECIFIERS) {
				HccASTGenSpecifier specifier = hcc_leastsetbitidx32(w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_STRUCT_FIELD_SPECIFIERS);
				HccATAToken token = hcc_astgen_specifier_tokens[specifier];
				hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_STRUCT_FIELD, hcc_ata_token_strings[token]);
			}

			if (w->astgen.specifier_flags) {
				if (compound_data_type.kind != HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE) {
					w->astgen.token_iter->token_idx -= 1;
					hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_MISSING_RASTERIZER_STATE_SPECIFIER, compound_data_type_location, hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_RASTERIZER_STATE], hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_POSITION], hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_NOINTERP]);
				}

				//
				// ensure only one specifier is enabled
				if (!HCC_IS_POWER_OF_TWO_OR_ZERO(w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS)) {
					hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_SPECIFIER_CONFIG_FOR_STRUCT_FIELD, hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_POSITION], hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_NOINTERP]);
				}

				switch (hcc_leastsetbitidx32(w->astgen.specifier_flags)) {
					case HCC_ASTGEN_SPECIFIER_POSITION:
						if (found_position) {
							hcc_astgen_error_1(w, HCC_ERROR_CODE_POSITION_ALREADY_SPECIFIED, hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_POSITION]);
						}
						found_position = true;
						compound_field->rasterizer_state_field_kind = HCC_RASTERIZER_STATE_FIELD_KIND_POSITION;
						break;
					case HCC_ASTGEN_SPECIFIER_NOINTERP: compound_field->rasterizer_state_field_kind = HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP; break;
				}

				w->astgen.specifier_flags &= ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_STRUCT_FIELD_SPECIFIERS;
			}
		}

		uint64_t alignas_align = 0;
		if (token == HCC_ATA_TOKEN_KEYWORD_ALIGNAS) {
			if (compound_data_type.kind != HCC_COMPOUND_DATA_TYPE_KIND_DEFAULT) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ALIGNAS_ON_SPECIAL_COMPOUND_DATA_TYPE);
			}

			token = hcc_ata_iter_next(w->astgen.token_iter);
			if (token != HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_ALIGNAS);
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);

			switch (token) {
				case HCC_ATA_TOKEN_LIT_SINT:
				case HCC_ATA_TOKEN_LIT_UINT:
				case HCC_ATA_TOKEN_LIT_SLONG:
				case HCC_ATA_TOKEN_LIT_ULONG: {
					HccConstantId constant_id = hcc_ata_iter_next_value(w->astgen.token_iter).constant_id;
					HccConstant constant = hcc_constant_table_get(w->cu, constant_id);

					//
					// skip the associated HccStringId kept around to turn the
					// literal back into the exact string it was parsed from.
					hcc_ata_iter_next_value(w->astgen.token_iter);
					if (!hcc_constant_as_uint(w->cu, constant, &alignas_align)) {
						hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_ALIGNAS_INT_CONSTANT);
					}
					token = hcc_ata_iter_next(w->astgen.token_iter);
					break;
				};
				default: {
					HccDataType alignas_data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_INVALID_ALIGNAS_OPERAND, true);
					alignas_data_type = hcc_astgen_generate_array_data_type_if_exists(w, alignas_data_type);
					uint64_t unused_size;
					hcc_data_type_size_align(w->cu, alignas_data_type, &unused_size, &alignas_align);
					token = hcc_ata_iter_peek(w->astgen.token_iter);
					break;
				};
			}

			if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_ALIGNAS);
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);
		}

		bool requires_name;
		switch (token) {
			case HCC_ATA_TOKEN_KEYWORD_STRUCT:
			case HCC_ATA_TOKEN_KEYWORD_UNION: {
				compound_field->data_type = hcc_astgen_generate_compound_data_type(w);
				requires_name = false;
				break;
			};
			case HCC_ATA_TOKEN_KEYWORD_ENUM: {
				compound_field->data_type = hcc_astgen_generate_enum_data_type(w);
				requires_name = true;
				break;
			};
			default: {
				compound_field->data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_COMPOUND_FIELD_INVALID_TERMINATOR, true);
				requires_name = true;
				break;
			};
		}

		compound_data_type.field_data_types_hash = hcc_astgen_hash_compound_data_type_field(cu, compound_field->data_type, compound_data_type.field_data_types_hash);

		HccDataType data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, compound_field->data_type);
		if (HCC_DATA_TYPE_IS_AST_BASIC(data_type)) {
			HccDataType aml_intrinsic_data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
			HCC_AML_SCALAR_DATA_TYPE_MASK_SET(&compound_data_type.has_scalar_data_types_mask, HCC_AML_INTRINSIC_DATA_TYPE_SCALAR(HCC_DATA_TYPE_AUX(aml_intrinsic_data_type)));
		} else if (HCC_DATA_TYPE_IS_COMPOUND(data_type)) {
			HccCompoundDataType* field_compound_data_type = hcc_compound_data_type_get(w->cu, data_type);
			compound_data_type.has_scalar_data_types_mask |= field_compound_data_type->has_scalar_data_types_mask;
			compound_data_type.flags |= (field_compound_data_type->flags & (HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_RESOURCE | HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_POINTER));
		} else if (HCC_DATA_TYPE_IS_RESOURCE(data_type)) {
			compound_data_type.flags |= HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_RESOURCE;
		} else if (HCC_DATA_TYPE_IS_POINTER(data_type)) {
			compound_data_type.flags |= HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_POINTER;
		} else if (HCC_DATA_TYPE_IS_ARRAY(data_type)) {
			HccArrayDataType* d = hcc_array_data_type_get(w->cu, data_type);
			compound_data_type.flags |= d->has_resource ? HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_RESOURCE : 0;
			compound_data_type.flags |= d->has_pointer ? HCC_COMPOUND_DATA_TYPE_FLAGS_HAS_POINTER : 0;
		} else if (data_type == HCC_DATA_TYPE_HALF) {
			HCC_AML_SCALAR_DATA_TYPE_MASK_SET(&compound_data_type.has_scalar_data_types_mask, HCC_AML_INTRINSIC_DATA_TYPE_F16);
		}

		//
		// validate the compound data type field given what the kind of compound data type this is.
		bool is_resource_constant = false;
		switch (compound_data_type.kind) {
			case HCC_COMPOUND_DATA_TYPE_KIND_DEFAULT:
				hcc_astgen_data_type_ensure_valid_variable(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_COMPOUND_DATA_TYPE);
				break;
			case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_TABLE:
				if (hcc_data_type_is_resource_set_pointer(w->cu, data_type)) {
					HccString data_type_name = hcc_data_type_string(w->cu, compound_field->data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCE_TABLE, (int)data_type_name.size, data_type_name.data);
				}

				hcc_astgen_data_type_ensure_compound_type_has_no_resources(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCE_TABLE);
				hcc_astgen_data_type_ensure_compound_type_default_kind(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCE_TABLE);
				if (!hcc_data_type_is_resource_table_pointer(w->cu, data_type)) {
					hcc_astgen_data_type_ensure_has_no_pointers(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT);
				}
				break;
			case HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE: {
				HccDataType lowered_data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
				if (!HCC_DATA_TYPE_IS_AML_INTRINSIC(lowered_data_type)) {
					HccString data_type_name = hcc_data_type_string(w->cu, compound_field->data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_DATA_TYPE_RASTERIZER_STATE, (int)data_type_name.size, data_type_name.data);
				}

				if (
					compound_field->rasterizer_state_field_kind == HCC_RASTERIZER_STATE_FIELD_KIND_POSITION &&
					data_type != HCC_DATA_TYPE_AML(HCC_AML_INTRINSIC_DATA_TYPE_F32X4)
				) {
					HccString expected_data_type_name = hcc_data_type_string(w->cu, HCC_DATA_TYPE_AML(HCC_AML_INTRINSIC_DATA_TYPE_F32X4));
					HccString data_type_name = hcc_data_type_string(w->cu, compound_field->data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_POSITION_MUST_BE_VEC4_F32, hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_POSITION], (int)expected_data_type_name.size, expected_data_type_name.data, (int)data_type_name.size, data_type_name.data);
				}
				hcc_astgen_data_type_ensure_has_no_pointers(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT);
				break;
			};
			case HCC_COMPOUND_DATA_TYPE_KIND_FRAGMENT_STATE: {
				HccDataType lowered_data_type = hcc_data_type_lower_ast_to_aml(cu, data_type);
				if (!HCC_DATA_TYPE_IS_AML_INTRINSIC(lowered_data_type) || HCC_AML_INTRINSIC_DATA_TYPE_ROWS(HCC_DATA_TYPE_AUX(lowered_data_type)) > 1) {
					HccString data_type_name = hcc_data_type_string(w->cu, compound_field->data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_DATA_TYPE_FRAGMENT_STATE, (int)data_type_name.size, data_type_name.data);
				}
				hcc_astgen_data_type_ensure_has_no_pointers(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT);
				break;
			};
			case HCC_COMPOUND_DATA_TYPE_KIND_BUFFER_ELEMENT:
				hcc_astgen_data_type_ensure_compound_type_default_kind(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT);
				hcc_astgen_data_type_ensure_has_no_resources(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT);
				hcc_astgen_data_type_ensure_has_no_pointers(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT);
				break;
			case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_SET:
				if (!HCC_DATA_TYPE_IS_RESOURCE(data_type)) {
					HccString data_type_name = hcc_data_type_string(w->cu, compound_field->data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCE_SET, (int)data_type_name.size, data_type_name.data);
				}
				break;
			case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCES: {
				HccErrorCode error_code;
				switch (hcc_options_get_u32(cu->options, HCC_OPTION_KEY_TARGET_RESOURCE_MODEL)) {
					case HCC_TARGET_RESOURCE_MODEL_BINDING:
						error_code = HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCES_BINDING;
						if (hcc_data_type_is_resource_table_pointer(w->cu, data_type)) {
							hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_RESOURCE_TABLE_RESOURCE_MODEL);
						}
						break;
					case HCC_TARGET_RESOURCE_MODEL_BINDING_AND_BINDLESS:
						error_code = HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_RESOURCES_BINDING_AND_BINDLESS;
						if (HCC_DATA_TYPE_IS_RESOURCE(data_type) && HCC_RESOURCE_DATA_TYPE_TYPE(HCC_DATA_TYPE_AUX(data_type)) == HCC_RESOURCE_DATA_TYPE_CONSTBUFFER) {
							hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_CONST_BUFFER_IN_RESOURCES_USING_BINDLESS);
						}
						break;
				}
				hcc_astgen_data_type_ensure_compound_type_default_kind(w, compound_field->data_type, error_code);
				hcc_astgen_data_type_ensure_compound_type_has_no_resources(w, compound_field->data_type, error_code);

				HccCompoundDataType* rs_d = hcc_data_type_get_resource_set(w->cu, data_type);
				if (rs_d) {
					//
					// ensure the resource set slot is unique
					for (uint32_t other_field_idx = 0; other_field_idx < field_idx; other_field_idx += 1) {
						HccCompoundField* prev_field = &fields[other_field_idx];
						HccDataType prev_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, prev_field->data_type);
						HccCompoundDataType* prev_rs_d = hcc_data_type_get_resource_set(w->cu, prev_data_type);
						if (!prev_rs_d) {
							continue;
						}

						if (rs_d->resource_set_slot == prev_rs_d->resource_set_slot) {
							HccString data_type_name = hcc_data_type_string(w->cu, compound_field->data_type);
							HccString other_data_type_name = hcc_data_type_string(w->cu, prev_field->data_type);
							hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_MATCHING_RESOURCE_SLOTS_IN_RESOURCES, prev_field->identifier_location, (int)data_type_name.size, data_type_name.data, (int)other_data_type_name.size, other_data_type_name.data, rs_d->resource_set_slot);
						}
					}
				} else if (!hcc_data_type_has_resources(w->cu, data_type)) {
					hcc_astgen_data_type_ensure_has_no_pointers(w, compound_field->data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_BUFFER_ELEMENT);
					is_resource_constant = true;
				}
				break;
			};
		}

		token = hcc_ata_iter_peek(w->astgen.token_iter);
		if (token != HCC_ATA_TOKEN_IDENT) {
			if (requires_name) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_COMPOUND_FIELD_MISSING_NAME);
			}

			if (HCC_DATA_TYPE_IS_COMPOUND(data_type) && !HCC_DATA_TYPE_IS_FORWARD_DECL(data_type)) {
				compound_data_type.field_identifiers_hash ^= hcc_compound_data_type_get(cu, data_type)->field_identifiers_hash;
			}

			compound_field->identifier_location = NULL;
			compound_field->identifier_string_id.idx_plus_one = 0;
		} else {
			HccStringId field_identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
			compound_field->identifier_location = hcc_ata_iter_location(w->astgen.token_iter);
			compound_field->identifier_string_id = field_identifier_string_id;

			compound_data_type.field_identifiers_hash = hcc_hash_fnv_64(&field_identifier_string_id, sizeof(field_identifier_string_id), compound_data_type.field_identifiers_hash);

			token = hcc_ata_iter_next(w->astgen.token_iter);
			compound_field->data_type = hcc_astgen_generate_array_data_type_if_exists(w, compound_field->data_type);
		}
		hcc_astgen_ensure_semicolon(w);
		token = hcc_ata_iter_peek(w->astgen.token_iter);

		//
		// TODO: in future this will be a platform specific problem
		uint64_t size;
		uint64_t align;
		hcc_data_type_size_align(w->cu, compound_field->data_type, &size, &align);

		if (alignas_align) {
			if (alignas_align < align) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ALIGNAS_REDUCES_ALIGNMENT, alignas_align, align);
			}
			align = alignas_align;
		}

		if (is_union) {
			if (compound_data_type.size < size) {
				compound_data_type.largest_sized_field_idx = field_idx;
				compound_data_type.size = size;
			}
		} else {
			compound_data_type.size = HCC_INT_ROUND_UP_ALIGN(compound_data_type.size, align) + size;
		}
		compound_data_type.align = HCC_MAX(compound_data_type.align, align);

		if (is_resource_constant) {
			resource_constants_size = HCC_INT_ROUND_UP_ALIGN(resource_constants_size, align) + size;
			resource_constants_size = HCC_MAX(resource_constants_size, align);
		}

		field_idx += 1;
	}

	hcc_stack_clear(w->astgen.compound_field_names);
	hcc_stack_clear(w->astgen.compound_field_locations);
	hcc_astgen_compound_data_type_validate_field_names(w, data_type, &compound_data_type);
	uint32_t max_resource_constants_size = hcc_options_get_u32(cu->options, HCC_OPTION_KEY_RESOURCE_CONSTANTS_MAX_SIZE);
	if (resource_constants_size > max_resource_constants_size) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_OVERFLOW_RESOURCE_CONSTANTS_SIZE, max_resource_constants_size, resource_constants_size);
	}

	if (!is_union) {
		compound_data_type.size = HCC_INT_ROUND_UP_ALIGN(compound_data_type.size, compound_data_type.align);
	}

	if (compound_data_type.kind == HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE && !found_position) {
		hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_POSITION_NOT_SPECIFIED, compound_data_type_location, hcc_ata_token_strings[HCC_ATA_TOKEN_KEYWORD_POSITION]);
	}

	if (table_entry && data_type && HCC_DATA_TYPE_IS_FORWARD_DECL(data_type)) {
		table_entry->location = compound_data_type.identifier_location;
	}

	//
	// add the struct or union to the compilation unit if an identical version doesn't exist there already
	HccHashTable(HccDeclEntryAtomic) cu_declarations = is_union ? cu->union_declarations : cu->struct_declarations;
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu_declarations, &identifier_string_id);
	HccDeclEntryAtomic* entry = &cu_declarations[insert.idx];

	HccAtomic(HccDeclEntryAtomicLink*)* link_ptr = &entry->link;
	while (1) {
		HccDeclEntryAtomicLink* link = atomic_load(link_ptr);
		if (link == NULL && atomic_compare_exchange_strong(link_ptr, &link, HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL)) {
			//
			// our thread claimed the atomic link which means this compound data type is unique with this identifier.
			// so now lets setup the enum and atomic link.
			HccCompoundField* new_fields = hcc_stack_push_many_thread_safe(cu->dtt.compound_fields, compound_data_type.fields_count);
			HCC_COPY_ELMT_MANY(new_fields, compound_data_type.fields, compound_data_type.fields_count);
			compound_data_type.fields = new_fields;

			HccCompoundDataType* dst_compound_data_type;
			uint32_t compound_idx;
			if (
				identifier_string_id.idx_plus_one &&
				HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_START <= identifier_string_id.idx_plus_one &&
				identifier_string_id.idx_plus_one < HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_END
			) {
				compound_idx = identifier_string_id.idx_plus_one - HCC_STRING_ID_INTRINSIC_COMPOUND_DATA_TYPES_START;
				dst_compound_data_type = hcc_stack_get(cu->dtt.compounds, compound_idx);
			} else {
				dst_compound_data_type = hcc_stack_push_thread_safe(cu->dtt.compounds);
				compound_idx = dst_compound_data_type - cu->dtt.compounds;
			}
			*dst_compound_data_type = compound_data_type;

			data_type = is_union ? HCC_DATA_TYPE(UNION, compound_idx) : HCC_DATA_TYPE(STRUCT, compound_idx);

			HccDeclEntryAtomicLink* alloced_link = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccDeclEntryAtomicLink, &w->arena_alctor);
			alloced_link->decl = data_type;
			atomic_store(link_ptr, alloced_link);
			break;
		}

		//
		// wait if any other thread is setting up the atomic link
		while (link == HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL) {
			HCC_CPU_RELAX();
			link = atomic_load(link_ptr);
		}

		HccCompoundDataType* other_compound_data_type = hcc_compound_data_type_get(cu, link->decl);
		if (
			other_compound_data_type->fields_count == compound_data_type.fields_count
			&& other_compound_data_type->field_data_types_hash == compound_data_type.field_data_types_hash
			&& other_compound_data_type->field_identifiers_hash == compound_data_type.field_identifiers_hash
			&& other_compound_data_type->size == compound_data_type.size
			&& other_compound_data_type->align == compound_data_type.align
			&& other_compound_data_type->flags == compound_data_type.flags
		) {
			//
			// we have found a matching compound data type with the same identifier, so lets use this one instead.
			data_type = link->decl;
			break;
		}

		link_ptr = &link->next;
	}

	if (table_entry) {
		table_entry->decl = data_type;
	}

	hcc_stack_pop_many(w->astgen.compound_fields, compound_data_type.fields_count);
	token = hcc_ata_iter_next(w->astgen.token_iter);
	hcc_astgen_data_type_found(w, data_type);
	return data_type;
}

HccATAToken hcc_astgen_generate_type_specifiers(HccWorker* w, HccLocation* location, HccASTGenTypeSpecifier* type_specifiers_mut) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	while (1) {
		HccASTGenTypeSpecifier found_specifier;
		switch (token) {
			case HCC_ATA_TOKEN_KEYWORD_VOID: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_VOID; break;
			case HCC_ATA_TOKEN_KEYWORD_BOOL: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_BOOL; break;
			case HCC_ATA_TOKEN_KEYWORD_CHAR: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_CHAR; break;
			case HCC_ATA_TOKEN_KEYWORD_SHORT: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_SHORT; break;
			case HCC_ATA_TOKEN_KEYWORD_INT: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_INT; break;
			case HCC_ATA_TOKEN_KEYWORD_LONG: found_specifier = *type_specifiers_mut & HCC_ASTGEN_TYPE_SPECIFIER_LONG ? HCC_ASTGEN_TYPE_SPECIFIER_LONGLONG : HCC_ASTGEN_TYPE_SPECIFIER_LONG; break;
			case HCC_ATA_TOKEN_KEYWORD_FLOAT: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_FLOAT; break;
			case HCC_ATA_TOKEN_KEYWORD_DOUBLE: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_DOUBLE; break;
			case HCC_ATA_TOKEN_KEYWORD_UNSIGNED: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED; break;
			case HCC_ATA_TOKEN_KEYWORD_SIGNED: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_SIGNED; break;
			case HCC_ATA_TOKEN_KEYWORD_COMPLEX: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_COMPLEX; break;
			case HCC_ATA_TOKEN_KEYWORD_ATOMIC: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_ATOMIC; break;
			case HCC_ATA_TOKEN_KEYWORD_CONST: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_CONST; break;
			case HCC_ATA_TOKEN_KEYWORD_VOLATILE: found_specifier = HCC_ASTGEN_TYPE_SPECIFIER_VOLATILE; break;
			default: return token;
		}

		if (*type_specifiers_mut & found_specifier) {
			hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_DUPLICATE_TYPE_SPECIFIER, location, hcc_astgen_type_specifier_string(found_specifier));
		}

		*type_specifiers_mut |= found_specifier;
		token = hcc_ata_iter_next(w->astgen.token_iter);
	}
}

HccDataType hcc_astgen_generate_data_type(HccWorker* w, HccErrorCode error_code, bool want_concrete_type) {
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	HccASTGenTypeSpecifier type_specifiers = 0;
	HccATAToken token = hcc_astgen_generate_type_specifiers(w, location, &type_specifiers);

	HccDataType data_type = 0;
	bool was_enum = false;
	if (!(type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_TYPES)) {
		bool is_tex_array;
		bool is_tex_ms;
		HccTextureDim tex_dim;
		switch (token) {
			case HCC_ATA_TOKEN_KEYWORD_STRUCT:
			case HCC_ATA_TOKEN_KEYWORD_UNION: {
				data_type = hcc_astgen_generate_compound_data_type(w);
				hcc_astgen_generate_type_specifiers(w, location, &type_specifiers);
				break;
			};
			case HCC_ATA_TOKEN_KEYWORD_ENUM:
				data_type = hcc_astgen_generate_enum_data_type(w);
				hcc_astgen_generate_type_specifiers(w, location, &type_specifiers);
				was_enum = true;
				break;
			case HCC_ATA_TOKEN_IDENT: {
				HccStringId identifier_string_id = hcc_ata_iter_peek_value(w->astgen.token_iter).string_id;
				uintptr_t found_idx = hcc_hash_table_find_idx(w->astgen.ast_file->global_declarations, &identifier_string_id);
				if (found_idx != UINTPTR_MAX) {
					HccDeclEntry* entry = &w->astgen.ast_file->global_declarations[found_idx];
					HccDecl decl = entry->decl;
					if (HCC_DECL_IS_DATA_TYPE(decl)) {
						data_type = decl;
						hcc_astgen_generate_type_specifiers(w, location, &type_specifiers);
						hcc_ata_iter_next_value(w->astgen.token_iter);
						token = hcc_ata_iter_next(w->astgen.token_iter);
					}
				}
				break;
			};
			case HCC_ATA_TOKEN_KEYWORD_CONSTBUFFER:
			case HCC_ATA_TOKEN_KEYWORD_BUFFER: {
				if (hcc_ata_iter_next(w->astgen.token_iter) != HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_RESOURCE_TYPE_GENERIC, hcc_ata_token_strings[token]);
				}

				HccDataType generic_data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_EXPECTED_TYPE_NAME, false);
				HccDataType resolved_generic_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, generic_data_type);
				token = hcc_ata_iter_peek(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_RESOURCE_TYPE_GENERIC);
				}

				bool is_error = !HCC_DATA_TYPE_IS_COMPOUND(resolved_generic_data_type);
				if (!is_error) {
					HccCompoundDataType* cdt = hcc_compound_data_type_get(w->cu, resolved_generic_data_type);
					is_error = cdt->kind != HCC_COMPOUND_DATA_TYPE_KIND_BUFFER_ELEMENT;
				}
				if (is_error) {
					HccString data_type_name = hcc_data_type_string(w->cu, generic_data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_BUFFER_ELEMENT_TYPE, (int)data_type_name.size, data_type_name.data);
				}
				data_type = hcc_astgen_deduplicate_buffer_data_type(w, resolved_generic_data_type);
				break;
			};
			case HCC_ATA_TOKEN_KEYWORD_TEXTURE1D: tex_dim = HCC_TEXTURE_DIM_1D; is_tex_array = false; is_tex_ms = false; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURE1DARRAY: tex_dim = HCC_TEXTURE_DIM_1D; is_tex_array = true; is_tex_ms = false; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURE2D: tex_dim = HCC_TEXTURE_DIM_2D; is_tex_array = false; is_tex_ms = false; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURE2DARRAY: tex_dim = HCC_TEXTURE_DIM_2D; is_tex_array = true; is_tex_ms = false; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURE2DMS: tex_dim = HCC_TEXTURE_DIM_2D; is_tex_array = false; is_tex_ms = true; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURE2DMSARRAY: tex_dim = HCC_TEXTURE_DIM_2D; is_tex_array = true; is_tex_ms = true; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURECUBE: tex_dim = HCC_TEXTURE_DIM_CUBE; is_tex_array = false; is_tex_ms = false; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURECUBEARRAY: tex_dim = HCC_TEXTURE_DIM_CUBE; is_tex_array = true; is_tex_ms = false; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURECUBEMS: tex_dim = HCC_TEXTURE_DIM_CUBE; is_tex_array = false; is_tex_ms = true; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURECUBEMSARRAY: tex_dim = HCC_TEXTURE_DIM_CUBE; is_tex_array = true; is_tex_ms = true; goto TEXTURE;
			case HCC_ATA_TOKEN_KEYWORD_TEXTURE3D: tex_dim = HCC_TEXTURE_DIM_3D; is_tex_array = false; is_tex_ms = false; goto TEXTURE;
			{
TEXTURE:{}
				HccAMLIntrinsicDataType intrinsic_type = HCC_AML_INTRINSIC_DATA_TYPE_VOID;
				if (hcc_ata_iter_next(w->astgen.token_iter) == HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
					HccDataType generic_data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_EXPECTED_TYPE_NAME, false);
					HccDataType resolved_generic_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, generic_data_type);
					token = hcc_ata_iter_peek(w->astgen.token_iter);
					if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
						hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_RESOURCE_TYPE_GENERIC);
					}


					HccDataType lowered_data_type = hcc_data_type_lower_ast_to_aml(w->cu, data_type);
					if (!HCC_DATA_TYPE_IS_AML_INTRINSIC(lowered_data_type)) {
						HccString data_type_name = hcc_data_type_string(w->cu, generic_data_type);
						hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_TEXEL_TYPE, (int)data_type_name.size, data_type_name.data);
					}

					intrinsic_type = HCC_DATA_TYPE_AUX(lowered_data_type);
				}
				HccResourceDataType resource_data_type = HCC_RESOURCE_DATA_TYPE_TEXTURE(tex_dim, intrinsic_type, is_tex_array, is_tex_ms);
				data_type = HCC_DATA_TYPE(RESOURCE, resource_data_type);
				break;
			}
			case HCC_ATA_TOKEN_KEYWORD_SAMPLERSTATE:
				data_type = HCC_DATA_TYPE(RESOURCE, HCC_RESOURCE_DATA_TYPE_SAMPLER_STATE);
				break;
			default:
				break;
		}
	}

	switch (HCC_DATA_TYPE_TYPE(data_type)) {
		case HCC_DATA_TYPE_STRUCT:
		case HCC_DATA_TYPE_UNION:
		case HCC_DATA_TYPE_TYPEDEF:
		case HCC_DATA_TYPE_ENUM:
NON_NUM_TYPE: {}
			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_ATOMIC) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ATOMIC_UNSUPPORTED_AT_THIS_TIME);
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
				hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, location);
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_COMPLEX) {
				hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_COMPLEX_ON_NON_FLOAT_TYPE, location);
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_TYPES) {
				hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_MULTIPLE_TYPES_SPECIFIED, location);
			}
			break;
		default: {
			if (HCC_DATA_TYPE_IS_RESOURCE(data_type) || was_enum) {
				goto NON_NUM_TYPE;
			}

			if (type_specifiers == 0) {
				if (error_code != HCC_ERROR_CODE_NONE) {
					token = hcc_ata_iter_peek(w->astgen.token_iter);
					hcc_astgen_bail_error_1(w, error_code, hcc_ata_token_strings[token]);
				}
				return HCC_DATA_TYPE_INVALID;
			}

			uint32_t num_types = hcc_onebitscount32((type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_TYPES) & ~HCC_ASTGEN_TYPE_SPECIFIER_INT);
			if (
				num_types > 1 &&
				!(num_types == 2 && type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_LONG_DOUBLE)
			) {
				hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_MULTIPLE_TYPES_SPECIFIED, location);
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_FLOAT_TYPES) {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_COMPLEX) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_COMPLEX_UNSUPPORTED_AT_THIS_TIME);
				}

				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
					hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, location);
				}

				if ((type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_LONG_DOUBLE) == HCC_ASTGEN_TYPE_SPECIFIER_LONG_DOUBLE) {
					hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_LONG_DOUBLE_IS_UNSUPPORTED, location);
				}

				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_FLOAT) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_FLOAT);
				} else if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_DOUBLE) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_DOUBLE);
				} else {
					HCC_ABORT("wat?");
				}
				break;
			} else {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_COMPLEX) {
					hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_COMPLEX_ON_NON_FLOAT_TYPE, location);
				}
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_VOID) {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
					hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, location);
				}

				data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_VOID);
				break;
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_BOOL) {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
					hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, location);
				}

				data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_BOOL);
				break;
			}

			if ((type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED) == HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
				hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_UNSIGNED_AND_SIGNED, location);
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_CHAR) {
				if (type_specifiers == HCC_ASTGEN_TYPE_SPECIFIER_CHAR) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_CHAR);
				} else if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_UCHAR);
				} else {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_SCHAR);
				}

				break;
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_SHORT) {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_USHORT);
				} else {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_SSHORT);
				}

				break;
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_LONG) {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_ULONG);
				} else {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_SLONG);
				}

				break;
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_LONGLONG) {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_ULONGLONG);
				} else {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_SLONGLONG);
				}

				break;
			}

			if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_INT) {
				if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED) {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_UINT);
				} else {
					data_type = HCC_DATA_TYPE(AST_BASIC, HCC_AST_BASIC_DATA_TYPE_SINT);
				}

				break;
			}

			break;
		};
	}

	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_CONST) {
		data_type = HCC_DATA_TYPE_CONST(data_type);
	}

	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_VOLATILE) {
		data_type = HCC_DATA_TYPE_VOLATILE(data_type);
	}

	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_ATOMIC) {
		data_type = HCC_DATA_TYPE_ATOMIC(data_type);
	}

	hcc_astgen_ensure_not_unsupported_basic_type(w, location, data_type);

	if (token != HCC_ATA_TOKEN_ASTERISK) {
		if (want_concrete_type) {
			HccDataType resolved_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, data_type);
			bool is_concrete_impl = true;
			switch (HCC_DATA_TYPE_TYPE(resolved_data_type)) {
				case HCC_DATA_TYPE_STRUCT:
				case HCC_DATA_TYPE_UNION: {
					is_concrete_impl = !HCC_DATA_TYPE_IS_FORWARD_DECL(resolved_data_type);
					break;
				};
			}

			if (!is_concrete_impl) {
				HccString data_type_name = hcc_data_type_string(w->cu, data_type);
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INCOMPLETE_TYPE_USED_BY_VALUE, (int)data_type_name.size, data_type_name.data);
			}
		}

		return data_type;
	}

	return hcc_astgen_generate_pointer_data_type_if_exists(w, data_type);
}

HccDataType hcc_astgen_generate_pointer_data_type_if_exists(HccWorker* w, HccDataType element_data_type) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_ASTERISK) {
		return element_data_type;
	}
	hcc_ata_iter_next(w->astgen.token_iter);

	if (HCC_DATA_TYPE_IS_POINTER(element_data_type)) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ONLY_SINGLE_POINTERS_ARE_SUPPORTED);
	}

	bool TODO_POINTER_TYPE_FOR_ALL_THE_OTHER_THINGS = false;

	HccDataType resolved_element_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, element_data_type);
	if (HCC_DATA_TYPE_IS_COMPOUND(resolved_element_data_type)) {
		HccCompoundDataType* d = hcc_compound_data_type_get(w->cu, resolved_element_data_type);
		switch (d->kind) {
			case HCC_COMPOUND_DATA_TYPE_KIND_RASTERIZER_STATE:
			case HCC_COMPOUND_DATA_TYPE_KIND_FRAGMENT_STATE:
				TODO_POINTER_TYPE_FOR_ALL_THE_OTHER_THINGS = true;
				break;

			case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCES: {
				HccString data_type_name = hcc_data_type_string(w->cu, element_data_type);
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_POINTER_DATA_TYPE, (int)data_type_name.size, data_type_name.data);
			};
			case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_SET:
			case HCC_COMPOUND_DATA_TYPE_KIND_RESOURCE_TABLE:
				TODO_POINTER_TYPE_FOR_ALL_THE_OTHER_THINGS = true;
				if (!HCC_DATA_TYPE_IS_CONST(element_data_type)) {

				}

				hcc_fallthrough;
			case HCC_COMPOUND_DATA_TYPE_KIND_BUFFER_ELEMENT:
				break;
		}
	}
	HCC_ASSERT(TODO_POINTER_TYPE_FOR_ALL_THE_OTHER_THINGS, "pointer type has only been implemented for ResourceSet and ResourceTable atm");

	HccDataType data_type = hcc_astgen_deduplicate_pointer_data_type(w, element_data_type);
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	HccASTGenTypeSpecifier type_specifiers;
	token = hcc_astgen_generate_type_specifiers(w, location, &type_specifiers);
	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_CONST) {
		data_type = HCC_DATA_TYPE_CONST(data_type);
	}
	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_VOLATILE) {
		data_type = HCC_DATA_TYPE_VOLATILE(data_type);
	}
	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_ATOMIC) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ATOMIC_UNSUPPORTED_AT_THIS_TIME);
	}

	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_UNSIGNED_SIGNED) {
		hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_UNSIGNED_OR_SIGNED_ON_NON_INT_TYPE, location);
	}

	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_COMPLEX) {
		hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_COMPLEX_ON_NON_FLOAT_TYPE, location);
	}

	if (type_specifiers & HCC_ASTGEN_TYPE_SPECIFIER_TYPES) {
		hcc_astgen_bail_error_1_merge_apply(w, HCC_ERROR_CODE_MULTIPLE_TYPES_SPECIFIED, location);
	}

	return hcc_astgen_generate_pointer_data_type_if_exists(w, data_type);
}

HccDataType hcc_astgen_generate_array_data_type_if_exists(HccWorker* w, HccDataType element_data_type) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_SQUARE_OPEN) {
		return element_data_type;
	}

	token = hcc_ata_iter_next(w->astgen.token_iter);
	if (token == HCC_ATA_TOKEN_SQUARE_CLOSE) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_ARRAY_SIZE);
	}

	HccASTExpr* size_expr = hcc_astgen_generate_expr(w, 0);
	if (size_expr->type != HCC_AST_EXPR_TYPE_CONSTANT || !HCC_DATA_TYPE_IS_AST_BASIC(size_expr->data_type) || !HCC_AST_BASIC_DATA_TYPE_IS_INT(HCC_DATA_TYPE_AUX(size_expr->data_type))) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_INTEGER_CONSTANT_ARRAY_SIZE);
	}

	HccConstant constant = hcc_constant_table_get(w->cu, size_expr->constant.id);
	uint64_t size;
	if (!hcc_constant_as_uint(w->cu, constant, &size)) {
		hcc_astgen_error_1(w, HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_NEGATIVE);
	}
	if (size == 0) {
		hcc_astgen_error_1(w, HCC_ERROR_CODE_ARRAY_SIZE_CANNOT_BE_ZERO);
	}

	token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_SQUARE_CLOSE) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ARRAY_DECL_EXPECTED_SQUARE_BRACE_CLOSE);
	}
	token = hcc_ata_iter_next(w->astgen.token_iter);

	HccDataType data_type = hcc_astgen_deduplicate_array_data_type(w, element_data_type, size_expr->constant.id);
	data_type = hcc_astgen_generate_array_data_type_if_exists(w, data_type);
	return data_type;
}

HccDataType hcc_astgen_generate_typedef(HccWorker* w) {
	HCC_DEBUG_ASSERT(hcc_ata_iter_peek(w->astgen.token_iter) == HCC_ATA_TOKEN_KEYWORD_TYPEDEF, "internal error: expected a typedef token");
	hcc_ata_iter_next(w->astgen.token_iter);

	HccDataType aliased_data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_EXPECTED_TYPE_NAME, false);

	return hcc_astgen_generate_typedef_with_data_type(w, aliased_data_type);
}

HccDataType hcc_astgen_generate_typedef_with_data_type(HccWorker* w, HccDataType aliased_data_type) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_IDENT) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_TYPEDEF, hcc_ata_token_strings[token]);
	}
	HccLocation* identifier_location = hcc_ata_iter_location(w->astgen.token_iter);
	HccStringId identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
	hcc_ata_iter_next(w->astgen.token_iter);
	aliased_data_type = hcc_astgen_generate_array_data_type_if_exists(w, aliased_data_type);

	hcc_astgen_validate_specifiers(w, HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_TYPEDEF_SPECIFIERS, HCC_ERROR_CODE_INVALID_SPECIFIER_FOR_TYPEDEF);

	uint32_t intrinsic_id = 0;
	HccCU* cu = w->cu;

	HccDataType* insert_value_ptr;
	HccTypedef* typedef_ = NULL;
	HccDataType data_type;
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(w->astgen.ast_file->global_declarations, &identifier_string_id.idx_plus_one);
	HccDeclEntry* table_entry = &w->astgen.ast_file->global_declarations[insert.idx];
	if (insert.is_new) {
		//
		// add the typedef to the compilation unit if an identical version doesn't exist there already
		HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->global_declarations, &identifier_string_id);
		HccDeclEntryAtomic* entry = &cu->global_declarations[insert.idx];

		HccAtomic(HccDeclEntryAtomicLink*)* link_ptr = &entry->link;
		while (1) {
			HccDeclEntryAtomicLink* link = atomic_load(link_ptr);
			if (link == NULL && atomic_compare_exchange_strong(link_ptr, &link, HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL)) {
				//
				// our thread claimed the atomic link which means this typedef is unique with this identifier.
				// so now lets setup the typedef and atomic link.
				typedef_ = hcc_stack_push_thread_safe(cu->dtt.typedefs);

				typedef_->identifier_location = identifier_location;
				typedef_->identifier_string_id = identifier_string_id;
				typedef_->aliased_data_type = aliased_data_type;

				uint32_t typedef_idx = typedef_ - cu->dtt.typedefs;
				data_type = HCC_DATA_TYPE(TYPEDEF, typedef_idx);

				HccDeclEntryAtomicLink* alloced_link = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccDeclEntryAtomicLink, &w->arena_alctor);
				alloced_link->decl = data_type;
				atomic_store(link_ptr, alloced_link);
				break;
			}

			//
			// wait if any other thread is setting up the atomic link
			while (link == HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL) {
				HCC_CPU_RELAX();
				link = atomic_load(link_ptr);
			}

			if (HCC_DATA_TYPE_IS_TYPEDEF(link->decl) && hcc_typedef_get(cu, link->decl)->aliased_data_type == aliased_data_type) {
				//
				// we have found a matching typedef with the same identifier, so lets use this one instead.
				data_type = link->decl;
				break;
			}

			link_ptr = &link->next;
		}

		table_entry->decl = data_type;
		table_entry->location = identifier_location;
		hcc_astgen_data_type_found(w, data_type);
	} else {
		data_type = table_entry->decl;
		if (!HCC_DATA_TYPE_IS_TYPEDEF(data_type) || hcc_typedef_get(cu, data_type)->aliased_data_type != aliased_data_type) {
			HccString data_type_name = hcc_string_table_get(identifier_string_id);
			HccLocation* other_location = hcc_data_type_location(cu, data_type);
			hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_INTERNAL, identifier_location, other_location, (int)data_type_name.size, data_type_name.data);
		}
	}

	hcc_astgen_ensure_semicolon(w);
	return data_type;
}

void hcc_astgen_generate_implicit_cast(HccWorker* w, HccDataType dst_data_type, HccASTExpr** expr_mut) {
	HccASTExpr* expr = *expr_mut;
	if (hcc_decl_resolve_and_strip_qualifiers(w->cu, expr->data_type) == hcc_decl_resolve_and_strip_qualifiers(w->cu, dst_data_type)) {
		return;
	}

	if (expr->type == HCC_AST_EXPR_TYPE_CONSTANT) {
		hcc_astgen_eval_cast(w, expr, dst_data_type);
		return;
	}

	HccASTExpr* cast_expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_CAST);
	cast_expr->cast_.expr = expr;
	cast_expr->data_type = dst_data_type;
	cast_expr->location = hcc_ata_iter_location(w->astgen.token_iter);
	*expr_mut = cast_expr;
}

HccASTExpr* hcc_astgen_generate_unary_op(HccWorker* w, HccASTExpr* inner_expr, HccASTUnaryOp unary_op, HccATAToken operator_token, HccLocation* location) {
	HccDataType resolved_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, inner_expr->data_type);

	HccDataType unary_expr_data_type;
	if (unary_op == HCC_AST_UNARY_OP_DEREF || unary_op == HCC_AST_UNARY_OP_ADDRESS_OF) {
		if (!HCC_DATA_TYPE_IS_POINTER(resolved_data_type)) {
			goto ERROR;
		}

		unary_expr_data_type = hcc_data_type_strip_pointer(w->cu, inner_expr->data_type);
	} else if (HCC_DATA_TYPE_IS_AST_BASIC(resolved_data_type)) {
		if (unary_op == HCC_AST_UNARY_OP_LOGICAL_NOT) {
			unary_expr_data_type = HCC_DATA_TYPE_AST_BASIC_BOOL;
		} else {
			HccASTBasicDataType resolved_basic_data_type = HCC_DATA_TYPE_AUX(resolved_data_type);
			if (HCC_AST_BASIC_DATA_TYPE_IS_INT(resolved_basic_data_type)) {
				uint8_t rank = hcc_ast_data_type_basic_type_ranks[resolved_basic_data_type];
				uint8_t int_rank = hcc_ast_data_type_basic_type_ranks[HCC_AST_BASIC_DATA_TYPE_SINT];
				if (rank < int_rank) {
					hcc_astgen_generate_implicit_cast(w, HCC_DATA_TYPE_AST_BASIC_SINT, &inner_expr);
				}
			}

			unary_expr_data_type = inner_expr->data_type;
		}
	} else {
		goto ERROR;
	}

	HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_UNARY_OP);
	expr->unary.op = unary_op;
	expr->unary.expr = inner_expr;
	expr->data_type = unary_expr_data_type;
	expr->location = location;

	return expr;

ERROR: {}
	HccString data_type_name = hcc_data_type_string(w->cu, inner_expr->data_type);
	hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_UNARY_OPERATOR_NOT_SUPPORTED, hcc_ata_token_strings[operator_token], (int)data_type_name.size, data_type_name.data);
}

HccASTExpr* hcc_astgen_generate_unary_expr(HccWorker* w) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	HccASTUnaryOp unary_op;
	switch (token) {
		case HCC_ATA_TOKEN_KEYWORD_TRUE:
		case HCC_ATA_TOKEN_KEYWORD_FALSE:
		case HCC_ATA_TOKEN_LIT_SINT:
		case HCC_ATA_TOKEN_LIT_SLONG:
		case HCC_ATA_TOKEN_LIT_SLONGLONG:
		case HCC_ATA_TOKEN_LIT_UINT:
		case HCC_ATA_TOKEN_LIT_ULONG:
		case HCC_ATA_TOKEN_LIT_ULONGLONG:
		case HCC_ATA_TOKEN_LIT_FLOAT:
		case HCC_ATA_TOKEN_LIT_DOUBLE: {
			HccDataType data_type;
			HccConstantId constant_id;
			switch (token) {
				case HCC_ATA_TOKEN_KEYWORD_TRUE:
					data_type = HCC_DATA_TYPE_AST_BASIC_BOOL;
					constant_id = w->cu->dtt.basic_type_one_constant_ids[HCC_AST_BASIC_DATA_TYPE_BOOL];
					break;
				case HCC_ATA_TOKEN_KEYWORD_FALSE:
					data_type = HCC_DATA_TYPE_AST_BASIC_BOOL;
					constant_id = w->cu->dtt.basic_type_zero_constant_ids[HCC_AST_BASIC_DATA_TYPE_BOOL];
					break;
				case HCC_ATA_TOKEN_LIT_SINT: data_type = HCC_DATA_TYPE_AST_BASIC_SINT; break;
				case HCC_ATA_TOKEN_LIT_SLONG: data_type = HCC_DATA_TYPE_AST_BASIC_SLONG; break;
				case HCC_ATA_TOKEN_LIT_SLONGLONG: data_type = HCC_DATA_TYPE_AST_BASIC_SLONGLONG; break;
				case HCC_ATA_TOKEN_LIT_UINT: data_type = HCC_DATA_TYPE_AST_BASIC_UINT; break;
				case HCC_ATA_TOKEN_LIT_ULONG: data_type = HCC_DATA_TYPE_AST_BASIC_ULONG; break;
				case HCC_ATA_TOKEN_LIT_ULONGLONG: data_type = HCC_DATA_TYPE_AST_BASIC_ULONGLONG; break;
				case HCC_ATA_TOKEN_LIT_FLOAT: data_type = HCC_DATA_TYPE_AST_BASIC_FLOAT; break;
				case HCC_ATA_TOKEN_LIT_DOUBLE: data_type = HCC_DATA_TYPE_AST_BASIC_DOUBLE; break;
			}
			if (data_type != HCC_DATA_TYPE_AST_BASIC_BOOL) {
				constant_id = hcc_ata_iter_next_value(w->astgen.token_iter).constant_id;

				//
				// skip the associated HccStringId kept around to turn the
				// literal back into the exact string it was parsed from.
				hcc_ata_iter_next_value(w->astgen.token_iter);
			}

			HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_CONSTANT);
			expr->constant.id = constant_id;
			expr->data_type = data_type;
			expr->location = hcc_ata_iter_location(w->astgen.token_iter);
			hcc_ata_iter_next(w->astgen.token_iter);
			return expr;
		};
		case HCC_ATA_TOKEN_IDENT: {
			HccATAValue identifier_value = hcc_ata_iter_next_value(w->astgen.token_iter);
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
			hcc_ata_iter_next(w->astgen.token_iter);

			uint32_t existing_variable_id = hcc_astgen_variable_stack_find(w, identifier_value.string_id);
			if (existing_variable_id) {
				HccASTVariable* variable = &w->astgen.function_params_and_variables[existing_variable_id - 1];

				HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_LOCAL_VARIABLE);
				expr->variable.decl = HCC_DECL(LOCAL_VARIABLE, existing_variable_id - 1);
				expr->data_type = variable->data_type;
				expr->location = location;

				if (variable->storage_duration != HCC_AST_STORAGE_DURATION_AUTOMATIC) {
					hcc_astgen_static_variable_usage_found(w, expr->variable.decl);
				}
				return expr;
			}

			HccDecl decl;
			uintptr_t found_idx = hcc_hash_table_find_idx(w->astgen.ast_file->global_declarations, &identifier_value.string_id);
			if (found_idx != UINTPTR_MAX) {
				HccDeclEntry* entry = &w->astgen.ast_file->global_declarations[found_idx];
				HccDecl decl = entry->decl;
				if (HCC_DECL_IS_DATA_TYPE(decl)) {
					HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_DATA_TYPE);
					expr->data_type = decl;
					expr->location = location;
					return expr;
				} else if (HCC_DECL_IS_FUNCTION(decl)) {
					HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_FUNCTION);
					expr->function.decl = decl;
					expr->location = location;
					return expr;
				} else if (HCC_DECL_IS_ENUM_VALUE(decl)) {
					HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_CONSTANT);
					HccEnumValue* enum_value = hcc_enum_value_get(w->cu, decl);
					expr->constant.id = enum_value->constant_id;
					expr->data_type = HCC_DATA_TYPE_AST_BASIC_SINT;
					expr->location = location;
					return expr;
				} else if (HCC_DECL_IS_GLOBAL_VARIABLE(decl)) {
					HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_GLOBAL_VARIABLE);
					expr->variable.decl = decl;
					expr->data_type = hcc_decl_return_data_type(w->cu, decl);
					expr->location = location;

					hcc_astgen_static_variable_usage_found(w, decl);
					return expr;
				} else {
					HCC_UNREACHABLE("unhandled decl type here %u", decl);
				}
			}

			HccString string = hcc_string_table_get(identifier_value.string_id);
			hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_UNDECLARED_IDENTIFIER, (int)string.size, string.data);
		};
		case HCC_ATA_TOKEN_TILDE: unary_op = HCC_AST_UNARY_OP_BIT_NOT; goto UNARY;
		case HCC_ATA_TOKEN_EXCLAMATION_MARK: unary_op = HCC_AST_UNARY_OP_LOGICAL_NOT; goto UNARY;
		case HCC_ATA_TOKEN_PLUS: unary_op = HCC_AST_UNARY_OP_PLUS; goto UNARY;
		case HCC_ATA_TOKEN_MINUS: unary_op = HCC_AST_UNARY_OP_NEGATE; goto UNARY;
		case HCC_ATA_TOKEN_INCREMENT: unary_op = HCC_AST_UNARY_OP_PRE_INCREMENT; goto UNARY;
		case HCC_ATA_TOKEN_DECREMENT: unary_op = HCC_AST_UNARY_OP_PRE_DECREMENT; goto UNARY;
		case HCC_ATA_TOKEN_ASTERISK: unary_op = HCC_AST_UNARY_OP_DEREF; goto UNARY;
		case HCC_ATA_TOKEN_AMPERSAND: unary_op = HCC_AST_UNARY_OP_ADDRESS_OF; goto UNARY;
UNARY:
		{
			HccATAToken operator_token = token;
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
			hcc_ata_iter_next(w->astgen.token_iter);
			uint8_t precedence = hcc_ast_unary_op_precedence[unary_op];

			HccASTExpr* inner_expr = hcc_astgen_generate_expr(w, precedence);
			return hcc_astgen_generate_unary_op(w, inner_expr, unary_op, operator_token, location);
		};
		case HCC_ATA_TOKEN_PARENTHESIS_OPEN: {
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
			hcc_ata_iter_next(w->astgen.token_iter);
			HccASTExpr* expr = hcc_astgen_generate_expr(w, 0);
			HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
			if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_EXPR);
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);

			if (expr->type == HCC_AST_EXPR_TYPE_DATA_TYPE) {
				if (token == HCC_ATA_TOKEN_CURLY_OPEN) {
					//
					// found compound literal
					w->astgen.assign_data_type = expr->data_type;
					return hcc_astgen_generate_unary_expr(w);
				} else {
					HccASTExpr* right_expr = hcc_astgen_generate_expr(w, 2);
					HccCanCast can_cast = hcc_data_type_can_cast(w->cu, expr->data_type, right_expr->data_type);
					switch (can_cast) {
						case HCC_CAN_CAST_YES:
							if (right_expr->type == HCC_AST_EXPR_TYPE_CONSTANT) {
								hcc_astgen_eval_cast(w, right_expr, expr->data_type);
								return right_expr;
							}

							HccASTExpr* cast_expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_CAST);
							cast_expr->cast_.expr = cast_expr;
							cast_expr->data_type = expr->data_type;
							cast_expr->location = location;
							return cast_expr;
						case HCC_CAN_CAST_NO_DIFFERENT_TYPES: {
							HccString target_data_type_name = hcc_data_type_string(w->cu, expr->data_type);
							HccString source_data_type_name = hcc_data_type_string(w->cu, right_expr->data_type);
							hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_CAST, (int)source_data_type_name.size, source_data_type_name.data, (int)target_data_type_name.size, target_data_type_name.data);
						};
						case HCC_CAN_CAST_NO_SAME_TYPES:
							// do nothing
							break;
					}

					return right_expr;
				}
			}

			return expr;
		};
		case HCC_ATA_TOKEN_CURLY_OPEN: {
			HccDataType assign_data_type = w->astgen.assign_data_type;
			HccDataType resolved_assign_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, assign_data_type);
			w->astgen.assign_data_type = HCC_DATA_TYPE_AST_BASIC_VOID;
			HccASTGenCurlyInitializer* gen = &w->astgen.curly_initializer;
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);

			HccASTExpr* curly_initializer_expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_CURLY_INITIALIZER);
			curly_initializer_expr->data_type = assign_data_type;

			HccASTExpr* variable_expr;
			HccASTVariable* variable;
			{
				variable = hcc_stack_push(w->astgen.function_params_and_variables);
				variable->identifier_string_id.idx_plus_one = 0;
				variable->identifier_location = NULL;
				variable->data_type = assign_data_type;

				uint32_t variable_idx = w->astgen.next_var_idx;
				w->astgen.next_var_idx += 1;

				variable_expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_LOCAL_VARIABLE);
				variable_expr->variable.decl = HCC_DECL(LOCAL_VARIABLE, variable_idx);
				variable_expr->next_stmt = NULL;
				variable_expr->location = location;
			}

			uint32_t nested_curlys_start_idx = hcc_stack_count(gen->nested_curlys);
			token = hcc_astgen_curly_initializer_start(w, assign_data_type, resolved_assign_data_type, variable_expr);

			while (1) {
				if (token == HCC_ATA_TOKEN_CURLY_OPEN) {
					//
					// we have just found a another curly initializer, so nest down into the next element type
					//
					hcc_astgen_curly_initializer_next_elmt(w, HCC_DATA_TYPE_AST_BASIC_VOID);
					token = hcc_astgen_curly_initializer_open(w);
					continue;
				}

				location = hcc_ata_iter_location(w->astgen.token_iter);
				if (token == HCC_ATA_TOKEN_FULL_STOP || token == HCC_ATA_TOKEN_SQUARE_OPEN) {
					token = hcc_astgen_curly_initializer_next_elmt_with_designator(w);
					if (token == HCC_ATA_TOKEN_CURLY_OPEN) {
						token = hcc_astgen_curly_initializer_open(w);
						continue;
					}
				} else if (hcc_stack_get_last(gen->nested_curlys)->found_designator) {
					hcc_astgen_warn_1(w, HCC_WARN_CODE_NO_DESIGNATOR_AFTER_DESIGNATOR);
				}

				HccASTExpr* value_expr = hcc_astgen_generate_expr_no_comma_operator(w, 0);
				HccDataType resolved_value_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, value_expr->data_type);
				bool emit_elmt_initializer = hcc_astgen_curly_initializer_next_elmt(w, resolved_value_data_type);
				if (emit_elmt_initializer) {
					HccLocation* other_location = NULL;
					hcc_astgen_data_type_ensure_compatible_assignment(w, other_location, w->astgen.curly_initializer.elmt_data_type, &value_expr);
					HccASTExpr* initializer_expr = hcc_astgen_curly_initializer_generate_designated_initializer(w, location);
					initializer_expr->designated_initializer.value_expr = value_expr;
				}

				token = hcc_ata_iter_peek(w->astgen.token_iter);

				//
				// loop to close curly braces and/or go to the next element.
				// loop until we find the final curly close or when we find a ',' that is _not_ followed by a '}'
				while (1) {
					bool found_one = false;
					if (token == HCC_ATA_TOKEN_CURLY_CLOSE) {
						bool is_finished = hcc_stack_count(gen->nested_curlys) == nested_curlys_start_idx + 1;
						token = hcc_astgen_curly_initializer_close(w, is_finished);
						if (is_finished) {
							goto CURLY_INITIALIZER_FINISH;
						}
						found_one = true;
					}

					if (token == HCC_ATA_TOKEN_COMMA) {
						token = hcc_ata_iter_next(w->astgen.token_iter);
						if (token != HCC_ATA_TOKEN_CURLY_CLOSE) {
							break;
						}
						found_one = true;
					}

					if (!found_one) {
						hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_CURLY_INITIALIZER_LIST_END);
					}
				}
			}
CURLY_INITIALIZER_FINISH: {}
			token = hcc_ata_iter_next(w->astgen.token_iter);
			curly_initializer_expr->curly_initializer.first_expr = gen->first_initializer_expr;

			//
			// if we have nested into another curly initializer expression
			// then restore the parent curly initializers state in the HccASTGenCurlyInitializer structure.
			if (nested_curlys_start_idx) {
				HccASTGenCurlyInitializerElmt* nested_elmt = hcc_stack_get_last(gen->nested_elmts);
				hcc_astgen_curly_initializer_set_composite(w, nested_elmt->data_type, nested_elmt->resolved_data_type);

				HccASTGenCurlyInitializerNested* nested = hcc_stack_get_last(gen->nested);
				gen->prev_initializer_expr = nested->prev_initializer_expr;
				gen->first_initializer_expr = nested->first_initializer_expr;
				gen->nested_elmts_start_idx = nested->nested_elmts_start_idx;
				hcc_stack_pop(gen->nested);
			}

			return curly_initializer_expr;
		};
		case HCC_ATA_TOKEN_KEYWORD_SIZEOF:
		case HCC_ATA_TOKEN_KEYWORD_ALIGNOF:
		{
			bool is_sizeof = token == HCC_ATA_TOKEN_KEYWORD_SIZEOF;
			token = hcc_ata_iter_next(w->astgen.token_iter);
			bool has_parenthesis = token == HCC_ATA_TOKEN_PARENTHESIS_OPEN;
			if (has_parenthesis) {
				token = hcc_ata_iter_next(w->astgen.token_iter);
			}
			HccASTExpr* expr = hcc_astgen_generate_unary_expr(w);
			if (has_parenthesis) {
				token = hcc_ata_iter_peek(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_EXPR);
				}
				token = hcc_ata_iter_next(w->astgen.token_iter);
			} else if (expr->type == HCC_AST_EXPR_TYPE_DATA_TYPE) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_SIZEALIGNOF_TYPE_OPERAND_NOT_WRAPPED, hcc_ata_token_strings[token]);
			}

			uint64_t size;
			uint64_t align;
			hcc_data_type_size_align(w->cu, expr->data_type, &size, &align);

			// TODO the actual type of sizeof must be a size_t aka uintptr_t
			// but GPU likes 32 bit by default so what do we do?
			HccBasic TODO_intptr_support_plz = hcc_basic_from_uint(w->cu, HCC_DATA_TYPE_AST_BASIC_UINT, is_sizeof ? size : align);

			expr->type = HCC_AST_EXPR_TYPE_CONSTANT;
			expr->constant.id = hcc_constant_table_deduplicate_basic(w->cu, HCC_DATA_TYPE_AST_BASIC_UINT, &TODO_intptr_support_plz);
			expr->data_type = HCC_DATA_TYPE_AST_BASIC_UINT;
			return expr;
		};

		case HCC_ATA_TOKEN_KEYWORD_VOID:
		case HCC_ATA_TOKEN_KEYWORD_BOOL:
		case HCC_ATA_TOKEN_KEYWORD_CHAR:
		case HCC_ATA_TOKEN_KEYWORD_SHORT:
		case HCC_ATA_TOKEN_KEYWORD_INT:
		case HCC_ATA_TOKEN_KEYWORD_LONG:
		case HCC_ATA_TOKEN_KEYWORD_FLOAT:
		case HCC_ATA_TOKEN_KEYWORD_DOUBLE:
		case HCC_ATA_TOKEN_KEYWORD_UNSIGNED:
		case HCC_ATA_TOKEN_KEYWORD_SIGNED:
		case HCC_ATA_TOKEN_KEYWORD_STRUCT:
		case HCC_ATA_TOKEN_KEYWORD_UNION:
		default: {
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
			HccDataType data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_EXPECTED_EXPR, true);
			data_type = hcc_astgen_generate_array_data_type_if_exists(w, data_type);
			HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_DATA_TYPE);
			expr->data_type = data_type;
			expr->location = location;
			return expr;
		};
	}
}

void hcc_astgen_generate_binary_op(HccWorker* w, HccASTBinaryOp* binary_op_out, uint32_t* precedence_out, bool* is_assignment_out) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	*is_assignment_out = false;
	switch (token) {
		case HCC_ATA_TOKEN_FULL_STOP:
			*binary_op_out = HCC_AST_BINARY_OP_FIELD_ACCESS;
			*precedence_out = 1;
			break;
		case HCC_ATA_TOKEN_ARROW_RIGHT:
			*binary_op_out = HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT;
			*precedence_out = 1;
			break;
		case HCC_ATA_TOKEN_PARENTHESIS_OPEN:
			*binary_op_out = HCC_AST_BINARY_OP_CALL;
			*precedence_out = 1;
			break;
		case HCC_ATA_TOKEN_SQUARE_OPEN:
			*binary_op_out = HCC_AST_BINARY_OP_ARRAY_SUBSCRIPT;
			*precedence_out = 1;
			break;
		case HCC_ATA_TOKEN_ASTERISK:
			*binary_op_out = HCC_AST_BINARY_OP_MULTIPLY;
			*precedence_out = 3;
			break;
		case HCC_ATA_TOKEN_FORWARD_SLASH:
			*binary_op_out = HCC_AST_BINARY_OP_DIVIDE;
			*precedence_out = 3;
			break;
		case HCC_ATA_TOKEN_PERCENT:
			*binary_op_out = HCC_AST_BINARY_OP_MODULO;
			*precedence_out = 3;
			break;
		case HCC_ATA_TOKEN_PLUS:
			*binary_op_out = HCC_AST_BINARY_OP_ADD;
			*precedence_out = 4;
			break;
		case HCC_ATA_TOKEN_MINUS:
			*binary_op_out = HCC_AST_BINARY_OP_SUBTRACT;
			*precedence_out = 4;
			break;
		case HCC_ATA_TOKEN_BIT_SHIFT_LEFT:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_SHIFT_LEFT;
			*precedence_out = 5;
			break;
		case HCC_ATA_TOKEN_BIT_SHIFT_RIGHT:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_SHIFT_RIGHT;
			*precedence_out = 5;
			break;
		case HCC_ATA_TOKEN_LESS_THAN:
			*binary_op_out = HCC_AST_BINARY_OP_LESS_THAN;
			*precedence_out = 6;
			break;
		case HCC_ATA_TOKEN_LESS_THAN_OR_EQUAL:
			*binary_op_out = HCC_AST_BINARY_OP_LESS_THAN_OR_EQUAL;
			*precedence_out = 6;
			break;
		case HCC_ATA_TOKEN_GREATER_THAN:
			*binary_op_out = HCC_AST_BINARY_OP_GREATER_THAN;
			*precedence_out = 6;
			break;
		case HCC_ATA_TOKEN_GREATER_THAN_OR_EQUAL:
			*binary_op_out = HCC_AST_BINARY_OP_GREATER_THAN_OR_EQUAL;
			*precedence_out = 6;
			break;
		case HCC_ATA_TOKEN_LOGICAL_EQUAL:
			*binary_op_out = HCC_AST_BINARY_OP_EQUAL;
			*precedence_out = 7;
			break;
		case HCC_ATA_TOKEN_LOGICAL_NOT_EQUAL:
			*binary_op_out = HCC_AST_BINARY_OP_NOT_EQUAL;
			*precedence_out = 7;
			break;
		case HCC_ATA_TOKEN_AMPERSAND:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_AND;
			*precedence_out = 8;
			break;
		case HCC_ATA_TOKEN_CARET:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_XOR;
			*precedence_out = 9;
			break;
		case HCC_ATA_TOKEN_PIPE:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_OR;
			*precedence_out = 10;
			break;
		case HCC_ATA_TOKEN_LOGICAL_AND:
			*binary_op_out = HCC_AST_BINARY_OP_LOGICAL_AND;
			*precedence_out = 11;
			break;
		case HCC_ATA_TOKEN_LOGICAL_OR:
			*binary_op_out = HCC_AST_BINARY_OP_LOGICAL_OR;
			*precedence_out = 12;
			break;
		case HCC_ATA_TOKEN_QUESTION_MARK:
			*binary_op_out = HCC_AST_BINARY_OP_TERNARY;
			*precedence_out = 13;
			break;
		case HCC_ATA_TOKEN_EQUAL:
			*binary_op_out = HCC_AST_BINARY_OP_ASSIGN;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_ADD_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_ADD;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_SUBTRACT_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_SUBTRACT;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_MULTIPLY_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_MULTIPLY;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_DIVIDE_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_DIVIDE;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_MODULO_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_MODULO;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_BIT_SHIFT_LEFT_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_SHIFT_LEFT;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_BIT_SHIFT_RIGHT_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_SHIFT_RIGHT;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_BIT_AND_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_AND;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_BIT_XOR_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_XOR;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_BIT_OR_ASSIGN:
			*binary_op_out = HCC_AST_BINARY_OP_BIT_OR;
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HCC_ATA_TOKEN_COMMA:
			*binary_op_out = HCC_AST_BINARY_OP_COMMA;
			*precedence_out = 15;
			*is_assignment_out = true;
			break;
		default:
			*binary_op_out = HCC_AST_BINARY_OP_COUNT;
			*precedence_out = 0;
			break;
	}
}

HccASTExpr* hcc_astgen_generate_call_expr(HccWorker* w, HccASTExpr* function_expr) {
	HCC_DEBUG_ASSERT(function_expr->type == HCC_AST_EXPR_TYPE_FUNCTION, "TODO: function pointer support");
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	HccDecl function_decl = function_expr->function.decl;
	HccDataType return_data_type = hcc_decl_return_data_type(w->cu, function_decl);
	HccASTVariable* params_array = hcc_decl_function_params(w->cu, function_decl);
	uint32_t params_count = hcc_decl_function_params_count(w->cu, function_decl);

	if (hcc_decl_function_shader_stage(w->cu, function_decl) != HCC_AST_FUNCTION_SHADER_STAGE_NONE) {
		hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_CANNOT_CALL_SHADER_FUNCTION, hcc_decl_location(w->cu, function_decl));
	}

	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token == HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
		hcc_astgen_ensure_function_args_count(w, function_decl, 0);
		token = hcc_ata_iter_next(w->astgen.token_iter);

		HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
		expr->binary.op = HCC_AST_BINARY_OP_CALL;
		expr->binary.left_expr = function_expr;
		expr->binary.right_expr = NULL;
		expr->data_type = return_data_type;
		expr->location = location;
		return expr;
	}

	HccASTExpr* first_arg_expr = NULL;
	HccASTExpr* prev_arg_expr = NULL;
	uint32_t args_count = 0;
	token = hcc_ata_iter_peek(w->astgen.token_iter);
	while (1) {
		HccASTExpr* arg_expr = hcc_astgen_generate_expr_no_comma_operator(w, 0);
		if (args_count < params_count) {
			HccASTVariable* param = &params_array[args_count];
			HccDataType param_data_type = HCC_DATA_TYPE_STRIP_CONST(param->data_type);
			hcc_astgen_data_type_ensure_compatible_assignment(w, param->identifier_location, param_data_type, &arg_expr);
		}
		arg_expr->is_stmt = true;
		arg_expr->next_stmt = NULL;

		if (prev_arg_expr) {
			prev_arg_expr->next_stmt = arg_expr;
		} else {
			first_arg_expr = arg_expr;
		}
		args_count += 1;

		token = hcc_ata_iter_peek(w->astgen.token_iter);
		if (token != HCC_ATA_TOKEN_COMMA) {
			if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_FUNCTION_ARG_DELIMITER);
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);
			break;
		}
		token = hcc_ata_iter_next(w->astgen.token_iter);
		prev_arg_expr = arg_expr;
	}

	hcc_astgen_ensure_function_args_count(w, function_decl, args_count);

	HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
	expr->binary.op = HCC_AST_BINARY_OP_CALL;
	expr->binary.left_expr = function_expr;
	expr->binary.right_expr = first_arg_expr;
	expr->data_type = return_data_type;
	expr->location = location;
	return expr;
}

HccASTExpr* hcc_astgen_generate_array_subscript_expr(HccWorker* w, HccASTExpr* array_expr) {
	HccASTExpr* index_expr = hcc_astgen_generate_expr(w, 0);
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_SQUARE_CLOSE) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ARRAY_SUBSCRIPT_EXPECTED_SQUARE_BRACE_CLOSE);
	}
	hcc_ata_iter_next(w->astgen.token_iter);

	HccArrayDataType* d = hcc_array_data_type_get(w->cu, array_expr->data_type);

	HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
	expr->binary.op = HCC_AST_BINARY_OP_ARRAY_SUBSCRIPT;
	expr->binary.left_expr = array_expr;
	expr->binary.right_expr = index_expr;
	expr->data_type = d->element_data_type | (array_expr->data_type & HCC_DATA_TYPE_QUALIFIERS_MASK);
	expr->location = hcc_ata_iter_location(w->astgen.token_iter);
	return expr;
}

HccASTExpr* hcc_astgen_generate_field_access_expr(HccWorker* w, HccASTExpr* left_expr, bool is_indirect) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_IDENT) {
		HccString left_data_type_name = hcc_data_type_string(w->cu, left_expr->data_type);
		HccLocation* location = hcc_data_type_location(w->cu, left_expr->data_type);
		hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FIELD_ACCESS, location, (int)left_data_type_name.size, left_data_type_name.data);
	}

	HccDataType data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, left_expr->data_type);
	if (is_indirect) {
		data_type = hcc_data_type_strip_pointer(w->cu, left_expr->data_type);
	}

	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	HccCompoundDataType* compound_data_type = hcc_compound_data_type_get(w->cu, data_type);

	HccStringId identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
	hcc_astgen_compound_data_type_find_field_by_name_checked(w, data_type, compound_data_type, identifier_string_id);

	hcc_ata_iter_next(w->astgen.token_iter);

	HccDataType qualifier_mask = data_type & HCC_DATA_TYPE_QUALIFIERS_MASK;

	//
	// create the single access or the chain of field accesses to get through anonymous fields
	//
	uint32_t fields_count = hcc_stack_count(w->astgen.compound_type_find_fields);
	for (uint32_t i = 0; i < fields_count; i += 1) {
		HccFieldAccess* access = hcc_stack_get(w->astgen.compound_type_find_fields, i);
		HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
		expr->binary.op = is_indirect ? HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT : HCC_AST_BINARY_OP_FIELD_ACCESS;
		expr->binary.left_expr = left_expr; // link to the previous expression
		expr->binary.field_idx = access->idx;
		expr->data_type = access->data_type | qualifier_mask;
		expr->location = location;
		left_expr = expr;
	}

	return left_expr;
}

HccASTExpr* hcc_astgen_generate_ternary_expr(HccWorker* w, HccASTExpr* cond_expr) {
	hcc_astgen_data_type_ensure_is_condition(w, cond_expr->data_type);

	HccASTExpr* true_expr = hcc_astgen_generate_expr(w, 0);

	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_COLON) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_MISSING_COLON_TERNARY_OP);
	}
	HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
	token = hcc_ata_iter_next(w->astgen.token_iter);

	HccASTExpr* false_expr = hcc_astgen_generate_expr(w, 0);

	HccLocation* other_location = NULL;
	if (!hcc_astgen_data_type_check_compatible_arithmetic(w, &true_expr, &false_expr)) {
		HccString true_data_type_name = hcc_data_type_string(w->cu, true_expr->data_type);
		HccString false_data_type_name = hcc_data_type_string(w->cu, false_expr->data_type);
		hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_TYPE_MISMATCH, other_location, (int)false_data_type_name.size, false_data_type_name.data, (int)true_data_type_name.size, true_data_type_name.data);
	}

	if (cond_expr->type == HCC_AST_EXPR_TYPE_CONSTANT && true_expr->type == HCC_AST_EXPR_TYPE_CONSTANT && false_expr->type == HCC_AST_EXPR_TYPE_CONSTANT) {
		HccConstant constant = hcc_constant_table_get(w->cu, cond_expr->constant.id);
		uint64_t cond;
		if (!hcc_constant_as_uint(w->cu, constant, &cond)) {
			HCC_ABORT("TODO: ternary expr constant evaluation unable to turn into a uint possible because it's a pointer type");
		}

		return cond ? true_expr : false_expr;
	} else {
		HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
		HccASTExpr* results_expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);

		expr->binary.op = HCC_AST_BINARY_OP_TERNARY;
		expr->binary.left_expr = cond_expr;
		expr->binary.right_expr = results_expr;
		expr->data_type = HCC_DATA_TYPE_STRIP_CONST(true_expr->data_type);
		expr->location = location;

		results_expr->binary.op = HCC_AST_BINARY_OP_TERNARY_RESULTS;
		results_expr->binary.left_expr = true_expr;
		results_expr->binary.right_expr = false_expr;
		results_expr->data_type = HCC_DATA_TYPE_STRIP_CONST(true_expr->data_type);
		results_expr->location = location;
		return expr;
	}
}

HccASTExpr* hcc_astgen_generate_expr_(HccWorker* w, uint32_t min_precedence, bool no_comma_operator) {
	HccLocation* callee_location = hcc_ata_iter_location(w->astgen.token_iter);
	HccASTExpr* left_expr = hcc_astgen_generate_unary_expr(w);
	if (left_expr->type == HCC_AST_EXPR_TYPE_DATA_TYPE) {
		goto RETURN;
	}

	while (1) {
		HccASTBinaryOp binary_op;
		uint32_t precedence;
		bool is_assign;
		HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
		HccATAToken operator_token = hcc_ata_iter_peek(w->astgen.token_iter);
		if (no_comma_operator && operator_token == HCC_ATA_TOKEN_COMMA) {
			goto RETURN;
		}

		switch (operator_token) {
			case HCC_ATA_TOKEN_INCREMENT:
				left_expr = hcc_astgen_generate_unary_op(w, left_expr, HCC_AST_UNARY_OP_POST_INCREMENT, operator_token, hcc_ata_iter_location(w->astgen.token_iter));
				hcc_ata_iter_next(w->astgen.token_iter);
				continue;
			case HCC_ATA_TOKEN_DECREMENT:
				left_expr = hcc_astgen_generate_unary_op(w, left_expr, HCC_AST_UNARY_OP_POST_DECREMENT, operator_token, hcc_ata_iter_location(w->astgen.token_iter));
				hcc_ata_iter_next(w->astgen.token_iter);
				continue;
		}

		hcc_astgen_generate_binary_op(w, &binary_op, &precedence, &is_assign);
		if (binary_op == HCC_AST_BINARY_OP_COUNT || (min_precedence && min_precedence <= precedence)) {
			goto RETURN;
		}
		hcc_ata_iter_next(w->astgen.token_iter);
		HccDataType resolved_left_expr_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, left_expr->data_type);

		if (binary_op == HCC_AST_BINARY_OP_CALL) {
			if (left_expr->type != HCC_AST_EXPR_TYPE_FUNCTION) { // TODO add function pointer support
				hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_PARENTHISES_USED_ON_NON_FUNCTION, callee_location);
			}

			left_expr = hcc_astgen_generate_call_expr(w, left_expr);
		} else if (binary_op == HCC_AST_BINARY_OP_ARRAY_SUBSCRIPT) {
			if (!HCC_DATA_TYPE_IS_ARRAY(resolved_left_expr_data_type)) { // TODO add pointer support
				HccString left_data_type_name = hcc_data_type_string(w->cu, left_expr->data_type);
				hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_SQUARE_BRACE_USED_ON_NON_ARRAY_DATA_TYPE, callee_location, (int)left_data_type_name.size, left_data_type_name.data);
			}

			left_expr = hcc_astgen_generate_array_subscript_expr(w, left_expr);
		} else if (binary_op == HCC_AST_BINARY_OP_FIELD_ACCESS) {
			if (!HCC_DATA_TYPE_IS_COMPOUND(resolved_left_expr_data_type)) {
				HccString left_data_type_name = hcc_data_type_string(w->cu, left_expr->data_type);
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_FULL_STOP_USED_ON_NON_COMPOUND_DATA_TYPE, (int)left_data_type_name.size, left_data_type_name.data);
			}

			left_expr = hcc_astgen_generate_field_access_expr(w, left_expr, false);
		} else if (binary_op == HCC_AST_BINARY_OP_FIELD_ACCESS_INDIRECT) {
			if (!HCC_DATA_TYPE_IS_POINTER(resolved_left_expr_data_type) && !HCC_DATA_TYPE_IS_COMPOUND(hcc_data_type_strip_pointer(w->cu, resolved_left_expr_data_type))) {
				HccString left_data_type_name = hcc_data_type_string(w->cu, left_expr->data_type);
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_ARROW_RIGHT_USED_ON_NON_COMPOUND_DATA_TYPE_POINTER, (int)left_data_type_name.size, left_data_type_name.data);
			}

			left_expr = hcc_astgen_generate_field_access_expr(w, left_expr, true);
		} else if (binary_op == HCC_AST_BINARY_OP_TERNARY) {
			left_expr = hcc_astgen_generate_ternary_expr(w, left_expr);
		} else {
			HccASTExpr* right_expr = hcc_astgen_generate_expr(w, precedence);

			HccLocation* other_location = NULL;
			if (is_assign) {
				hcc_astgen_data_type_ensure_compatible_assignment(w, other_location, resolved_left_expr_data_type, &right_expr);
			} else {
				hcc_astgen_data_type_ensure_compatible_arithmetic(w, other_location, &left_expr, &right_expr, operator_token);
			}

			HccDataType data_type;
			if (HCC_AST_BINARY_OP_EQUAL <= binary_op && binary_op <= HCC_AST_BINARY_OP_LOGICAL_OR) {
				data_type = HCC_DATA_TYPE_AST_BASIC_BOOL;
			} else {
				data_type = left_expr->data_type; // TODO make implicit conversions explicit in the AST and make the error above work correctly
			}
			data_type = HCC_DATA_TYPE_STRIP_CONST(data_type);

			if (left_expr->type == HCC_AST_EXPR_TYPE_CONSTANT && right_expr->type == HCC_AST_EXPR_TYPE_CONSTANT) {
				HccDataType resolved_right_expr_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, right_expr->data_type);
				HCC_DEBUG_ASSERT(
					HCC_DATA_TYPE_IS_AST_BASIC(resolved_left_expr_data_type) && HCC_DATA_TYPE_IS_AST_BASIC(resolved_right_expr_data_type),
					"this constant folding code assumes that both expressions are basic types"
				);

				HccBasicEval left_eval;
				HccConstant left_constant = hcc_constant_table_get(w->cu, left_expr->constant.id);
				if (HCC_AST_BASIC_DATA_TYPE_IS_SINT(w->cu, HCC_DATA_TYPE_AUX(resolved_left_expr_data_type))) {
					hcc_constant_as_sint(w->cu, left_constant, &left_eval.s64);
					left_eval.is_signed = true;
				} else {
					hcc_constant_as_uint(w->cu, left_constant, &left_eval.u64);
				}

				HccBasicEval right_eval;
				HccConstant right_constant = hcc_constant_table_get(w->cu, right_expr->constant.id);
				if (HCC_AST_BASIC_DATA_TYPE_IS_SINT(w->cu, HCC_DATA_TYPE_AUX(resolved_right_expr_data_type))) {
					hcc_constant_as_sint(w->cu, right_constant, &right_eval.s64);
					right_eval.is_signed = true;
				} else {
					hcc_constant_as_uint(w->cu, right_constant, &right_eval.u64);
				}

				HccBasicEval eval = hcc_basic_eval(binary_op, left_eval, right_eval);
				HccBasic basic = eval.is_signed ? hcc_basic_from_sint(w->cu, data_type, eval.s64) : hcc_basic_from_uint(w->cu, data_type, eval.u64);
				left_expr->constant.id = hcc_constant_table_deduplicate_basic(w->cu, data_type, &basic);
			} else {
				if (is_assign && HCC_DATA_TYPE_IS_CONST(data_type)) {
					HccString left_data_type_name = hcc_data_type_string(w->cu, data_type);
					hcc_astgen_error_1(w, HCC_ERROR_CODE_CANNOT_ASSIGN_TO_CONST, (int)left_data_type_name.size, left_data_type_name.data);
				}

				HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
				expr->binary.op = binary_op;
				expr->binary.left_expr = left_expr;
				expr->binary.right_expr = right_expr;
				expr->binary.is_assign = is_assign;
				expr->data_type = data_type;
				expr->location = location;
				left_expr = expr;
			}
		}
	}

RETURN: {}
	return left_expr;
}

HccASTExpr* hcc_astgen_generate_expr(HccWorker* w, uint32_t min_precedence) {
	return hcc_astgen_generate_expr_(w, min_precedence, false);
}

HccASTExpr* hcc_astgen_generate_expr_no_comma_operator(HccWorker* w, uint32_t min_precedence) {
	return hcc_astgen_generate_expr_(w, min_precedence, true);
}

HccASTExpr* hcc_astgen_generate_cond_expr(HccWorker* w) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_CONDITION_EXPR);
	}
	token = hcc_ata_iter_next(w->astgen.token_iter);

	HccASTExpr* cond_expr = hcc_astgen_generate_expr(w, 0);
	hcc_astgen_data_type_ensure_is_condition(w, cond_expr->data_type);

	token = hcc_ata_iter_peek(w->astgen.token_iter);
	if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_CONDITION_EXPR);
	}
	token = hcc_ata_iter_next(w->astgen.token_iter);
	return cond_expr;
}

HccDecl hcc_astgen_generate_variable_decl(HccWorker* w, bool is_global, HccDataType element_data_type, HccDataType* data_type_mut, HccASTExpr** init_expr_out) {
	HCC_UNUSED(element_data_type);
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	HCC_DEBUG_ASSERT(token == HCC_ATA_TOKEN_IDENT, "internal error: expected '%s' at the start of generating a variable", hcc_ata_token_strings[HCC_ATA_TOKEN_IDENT]);
	HccStringId identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
	HccLocation* identifier_location = hcc_ata_iter_location(w->astgen.token_iter);

	token = hcc_ata_iter_next(w->astgen.token_iter);
	if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_VARIABLE_SPECIFIERS) {
		HccASTGenSpecifier specifier = hcc_leastsetbitidx32(w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_VARIABLE_SPECIFIERS);
		HccATAToken token = hcc_astgen_specifier_tokens[specifier];
		hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_SPECIFIER_VARIABLE_DECL, hcc_ata_token_strings[token]);
	}

	uint32_t existing_variable_id = hcc_astgen_variable_stack_find(w, identifier_string_id);
	if (existing_variable_id) { // TODO: support shadowing but also warn or error about it
		HccLocation* other_location = NULL;
		HccString string = hcc_string_table_get(identifier_string_id);
		hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_LOCAL, other_location, (int)string.size, string.data);
	}

	bool found_static = w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_STATIC;
	bool found_extern = w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_EXTERN;
	bool found_thread_local = w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_THREAD_LOCAL;

	if (found_static && found_extern) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_STATIC_AND_EXTERN);
	}

	if (!is_global && found_thread_local) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_THREAD_LOCAL_MUST_BE_GLOBAL);
	}

	HccASTVariable variable;
	variable.ast_file = w->astgen.ast_file;
	variable.identifier_string_id = identifier_string_id;
	variable.identifier_location = identifier_location;
	variable.data_type = hcc_astgen_generate_array_data_type_if_exists(w, *data_type_mut);
	variable.initializer_constant_id.idx_plus_one = 0;
	if (is_global) {
		variable.storage_duration = found_thread_local ? HCC_AST_STORAGE_DURATION_THREAD : HCC_AST_STORAGE_DURATION_STATIC;
	} else {
		variable.storage_duration = found_static ? HCC_AST_STORAGE_DURATION_STATIC : HCC_AST_STORAGE_DURATION_AUTOMATIC;
	}

	*data_type_mut = variable.data_type;
	token = hcc_ata_iter_peek(w->astgen.token_iter);
	hcc_astgen_data_type_ensure_valid_variable(w, variable.data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_VARIABLE);

	switch (token) {
		case HCC_ATA_TOKEN_SEMICOLON:
			if (init_expr_out) *init_expr_out = NULL;
			break;
		case HCC_ATA_TOKEN_EQUAL: {
			hcc_ata_iter_next(w->astgen.token_iter);

			w->astgen.assign_data_type = variable.data_type;
			HccASTExpr* init_expr = hcc_astgen_generate_expr_no_comma_operator(w, 0);
			HccLocation* other_location = NULL;
			HccDataType variable_data_type = HCC_DATA_TYPE_STRIP_CONST(variable.data_type);
			hcc_astgen_data_type_ensure_compatible_assignment(w, other_location, variable_data_type, &init_expr);
			w->astgen.assign_data_type = HCC_DATA_TYPE_AST_BASIC_VOID;

			if (variable.storage_duration != HCC_AST_STORAGE_DURATION_AUTOMATIC) {
				if (init_expr->type != HCC_AST_EXPR_TYPE_CONSTANT) {
					hcc_astgen_error_1(w, HCC_ERROR_CODE_STATIC_VARIABLE_INITIALIZER_MUST_BE_CONSTANT);
				}
				variable.initializer_constant_id = init_expr->constant.id;
				if (init_expr_out) *init_expr_out = NULL;
			} else {
				if (init_expr_out) *init_expr_out = init_expr;
			}

			break;
		};
		case HCC_ATA_TOKEN_COMMA: break;
		default:
			hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_VARIABLE_DECL_TERMINATOR);
	}

	HccCU* cu = w->cu;
	HccDecl decl = 0;
	if (is_global) {
		variable.linkage = found_static ? HCC_AST_LINKAGE_INTERNAL : HCC_AST_LINKAGE_EXTERNAL;
		variable.linkage_has_been_explicitly_declared = found_static | found_extern;

		bool is_definition = !found_extern | variable.initializer_constant_id.idx_plus_one;
		if (found_static) {
			//
			// found static global variable so just add it to the global variable array for the AST
			HccASTVariable* dst_variable = hcc_stack_push_thread_safe(cu->ast.global_variables);
			*dst_variable = variable;

			uint32_t variable_idx = dst_variable - cu->ast.global_variables;
			decl = HCC_DECL(GLOBAL_VARIABLE, variable_idx);
		} else if (is_definition) {
			//
			// try to add the global variable definition to the compilation unit as it has external linkage
			HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->global_declarations, &identifier_string_id);
			HccDeclEntryAtomic* entry = &cu->global_declarations[insert.idx];

			HccAtomic(HccDeclEntryAtomicLink*)* link_ptr = &entry->link;
			while (1) {
				HccDeclEntryAtomicLink* link = atomic_load(link_ptr);
				if (link == NULL && atomic_compare_exchange_strong(link_ptr, &link, HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL)) {
					//
					// our thread claimed the atomic link which means adding this global variable to the compilation unit is okay.
					// so now lets setup the global variable and atomic link.
					HccASTVariable* dst_variable = hcc_stack_push_thread_safe(cu->ast.global_variables);
					*dst_variable = variable;

					uint32_t variable_idx = dst_variable - cu->ast.global_variables;
					decl = HCC_DECL(GLOBAL_VARIABLE, variable_idx);

					HccDeclEntryAtomicLink* alloced_link = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccDeclEntryAtomicLink, &w->arena_alctor);
					alloced_link->decl = decl;
					atomic_store(link_ptr, alloced_link);
					break;
				}

				//
				// wait if any other thread is setting up the atomic link
				while (link == HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL) {
					HCC_CPU_RELAX();
					link = atomic_load(link_ptr);
				}

				if (HCC_DECL_IS_GLOBAL_VARIABLE(link->decl)) {
					HccASTVariable* other_variable = hcc_ast_global_variable_get(cu, link->decl);
					if (variable.ast_file == other_variable->ast_file && !found_extern && !variable.initializer_constant_id.idx_plus_one) {
						//
						// tentative definition
						HccDataType data_type = hcc_decl_resolve_and_strip_qualifiers(cu, variable.data_type);
						HccDataType other_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, other_variable->data_type);
						if (data_type == other_data_type) {
							break;
						}
					}
					HccString identifier_string = hcc_string_table_get(identifier_string_id);
					hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_EXTERNAL, variable.identifier_location, other_variable->identifier_location, (int)identifier_string.size, identifier_string.data);
				} else if (HCC_DECL_IS_FUNCTION(link->decl)) {
					HccASTFunction* other_function = hcc_ast_function_get(cu, link->decl);
					HccString identifier_string = hcc_string_table_get(identifier_string_id);
					hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_EXTERNAL, variable.identifier_location, other_function->identifier_location, (int)identifier_string.size, identifier_string.data);
				}

				link_ptr = &link->next;
			}
		}

		//
		// insert the global variable in to this file's global declarations and perform the crazy C error checking
		HccDataType* insert_value_ptr;
		HccHashTableInsert insert = hcc_hash_table_find_insert_idx(w->astgen.ast_file->global_declarations, &identifier_string_id.idx_plus_one);
		HccDeclEntry* table_entry = &w->astgen.ast_file->global_declarations[insert.idx];
		if (insert.is_new) {
			if (decl == 0) {
				//
				// for the case when this is the first declaration in the file and is 'extern T variable;' with no initialization
				HccASTForwardDecl* forward_decl = hcc_stack_push_thread_safe(cu->ast.forward_declarations);
				forward_decl->ast_file = w->astgen.ast_file;
				forward_decl->identifier_location = variable.identifier_location;
				forward_decl->identifier_string_id = variable.identifier_string_id;
				forward_decl->variable.data_type = variable.data_type;

				uint32_t forward_decl_idx = forward_decl - cu->ast.forward_declarations;
				decl = HCC_DECL_FORWARD_DECL(GLOBAL_VARIABLE, forward_decl_idx);
				*hcc_stack_push(w->astgen.ast_file->forward_declarations_to_link) = decl;
			}

			table_entry->decl = decl;
			table_entry->location = identifier_location;
		} else {
			bool is_error = true;
			HccDecl found_decl = hcc_decl_resolve_and_strip_qualifiers(cu, table_entry->decl); // try to resolve a possible forward decl
			if (HCC_DECL_IS_GLOBAL_VARIABLE(found_decl)) {
				is_error = false;
				HccDataType data_type = hcc_decl_resolve_and_strip_qualifiers(cu, variable.data_type);
				if (HCC_DECL_IS_FORWARD_DECL(found_decl)) {
					//
					// we have a 'extern T variable;' declaration
					HccASTForwardDecl* forward_decl = hcc_ast_forward_decl_get(cu, found_decl);
					HccDataType other_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, forward_decl->variable.data_type);
					is_error |= found_static;
					is_error |= data_type != other_data_type;
				} else {
					//
					// we have a definition, lets see if the C rules allow for this type of redeclaring
					HccASTVariable* other_variable = hcc_ast_global_variable_get(cu, found_decl);
					HccDataType other_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, other_variable->data_type);
					is_error |= other_variable->linkage == HCC_AST_LINKAGE_INTERNAL && !found_extern;
					is_error |= other_variable->linkage == HCC_AST_LINKAGE_EXTERNAL && found_static;
					is_error |= other_variable->initializer_constant_id.idx_plus_one && variable.initializer_constant_id.idx_plus_one;
					is_error |= data_type != other_data_type;
				}
			}

			if (is_error) {
				HccString identifier_string = hcc_string_table_get(identifier_string_id);
				HccLocation* other_location = hcc_decl_location(cu, found_decl);
				hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_INTERNAL, variable.identifier_location, other_location, (int)identifier_string.size, identifier_string.data);
			}

			if (decl) {
				if (
					HCC_DECL_IS_GLOBAL_VARIABLE(found_decl) &&
					HCC_DECL_IS_FORWARD_DECL(found_decl)
				) {
					// we have a definition that can overwrite an 'extern T variable;' declaration
					table_entry->decl = decl;
				}
			} else {
				// we found another 'extern T variable;' so just reference the same decl as before.
				decl = found_decl;
			}
		}
	} else {
		variable.linkage = HCC_AST_LINKAGE_INTERNAL;
		uint32_t variable_idx = hcc_astgen_variable_stack_add(w, identifier_string_id);
		HccASTVariable* dst_variable = hcc_stack_push(w->astgen.function_params_and_variables);
		*dst_variable = variable;
		decl = HCC_DECL(LOCAL_VARIABLE, variable_idx);
	}

	w->astgen.specifier_flags &= ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_VARIABLE_SPECIFIERS;
	return decl;
}

HccASTExpr* hcc_astgen_generate_variable_decl_stmt(HccWorker* w, HccDataType data_type) {
	HccASTExpr* prev_expr = NULL;
	HccASTExpr* init_expr = NULL;
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);

	HccDataType element_data_type = hcc_data_type_strip_all_pointers(w->cu, data_type);
	while (1) {
		HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
		HccDecl decl = hcc_astgen_generate_variable_decl(w, false, element_data_type, &data_type, &init_expr);
		if (init_expr) {
			HccASTExpr* left_expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_LOCAL_VARIABLE);
			left_expr->variable.decl = decl;
			left_expr->data_type = data_type;
			left_expr->location = location;

			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
			stmt->binary.op = HCC_AST_BINARY_OP_ASSIGN;
			stmt->binary.is_assign = true;
			stmt->binary.left_expr = left_expr;
			stmt->binary.right_expr = init_expr;
			stmt->location = location;

			if (prev_expr) {
				HccASTExpr* expr = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_BINARY_OP);
				expr->binary.op = HCC_AST_BINARY_OP_COMMA;
				expr->binary.is_assign = false;
				expr->binary.left_expr = prev_expr;
				expr->binary.right_expr = stmt;
				expr->location = hcc_ata_iter_location(w->astgen.token_iter);
				prev_expr = expr;
			} else {
				prev_expr = stmt;
			}
		}

		token = hcc_ata_iter_peek(w->astgen.token_iter);
		if (token != HCC_ATA_TOKEN_COMMA) {
			break;
		}
		data_type = hcc_astgen_generate_pointer_data_type_if_exists(w, element_data_type);
		token = hcc_ata_iter_next(w->astgen.token_iter);
	}

	return prev_expr;
}

HccASTExpr* hcc_astgen_generate_stmt(HccWorker* w) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	switch (token) {
		case HCC_ATA_TOKEN_CURLY_OPEN: {
			hcc_astgen_variable_stack_open(w);

			HccASTExpr* prev_stmt = NULL;
			HccASTExpr* stmt_block = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_BLOCK);
			HccASTExpr* prev_stmt_block = w->astgen.stmt_block;

			stmt_block->is_stmt = true;
			stmt_block->location = hcc_ata_iter_location(w->astgen.token_iter);

			w->astgen.stmt_block = stmt_block;

			token = hcc_ata_iter_next(w->astgen.token_iter);
			while (token != HCC_ATA_TOKEN_CURLY_CLOSE) {
				HccASTExpr* stmt = hcc_astgen_generate_stmt(w);
				if (stmt == NULL) {
					continue;
				}
				stmt->is_stmt = true;
				stmt->next_stmt = NULL;

				if (prev_stmt) {
					prev_stmt->next_stmt = stmt;
				} else {
					stmt_block->stmt_block.first_stmt = stmt;
				}

				token = hcc_ata_iter_peek(w->astgen.token_iter);
				prev_stmt = stmt;
			}

			hcc_astgen_variable_stack_close(w);
			token = hcc_ata_iter_next(w->astgen.token_iter);
			w->astgen.stmt_block = prev_stmt_block;
			return stmt_block;
		};
		case HCC_ATA_TOKEN_KEYWORD_RETURN: {
			hcc_ata_iter_next(w->astgen.token_iter);
			HccASTExpr* expr = hcc_astgen_generate_expr(w, 0);

			hcc_astgen_data_type_ensure_compatible_assignment(w, w->astgen.function->return_data_type_location, w->astgen.function->return_data_type, &expr);

			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_RETURN);
			stmt->return_.expr = expr;
			stmt->location = hcc_ata_iter_location(w->astgen.token_iter);
			hcc_astgen_ensure_semicolon(w);

			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_IF: {
			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_IF);
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);

			hcc_ata_iter_next(w->astgen.token_iter);
			HccASTExpr* cond_expr = hcc_astgen_generate_cond_expr(w);

			HccASTExpr* true_stmt = hcc_astgen_generate_stmt(w);
			true_stmt->is_stmt = true;

			token = hcc_ata_iter_peek(w->astgen.token_iter);
			HccASTExpr* false_stmt = NULL;
			if (token == HCC_ATA_TOKEN_KEYWORD_ELSE) {
				token = hcc_ata_iter_next(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_KEYWORD_IF && token != HCC_ATA_TOKEN_CURLY_OPEN) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_ELSE);
				}
				false_stmt = hcc_astgen_generate_stmt(w);
				false_stmt->is_stmt = true;
			}

			stmt->type = HCC_AST_EXPR_TYPE_STMT_IF;
			stmt->if_.cond_expr = cond_expr;
			stmt->if_.true_stmt = true_stmt;
			stmt->if_.false_stmt = false_stmt;
			stmt->location = location;
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_SWITCH: {
			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_SWITCH);
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);
			token = hcc_ata_iter_next(w->astgen.token_iter);

			HccASTExpr* cond_expr;
			{
				if (token != HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_CONDITION_EXPR);
				}
				token = hcc_ata_iter_next(w->astgen.token_iter);

				cond_expr = hcc_astgen_generate_expr(w, 0);
				HccDataType resolved_data_type = hcc_decl_resolve_and_strip_qualifiers(w->cu, cond_expr->data_type);
				if (!HCC_DATA_TYPE_IS_AST_BASIC(resolved_data_type) || !HCC_AST_BASIC_DATA_TYPE_IS_INT(HCC_DATA_TYPE_AUX(resolved_data_type))) {
					HccString data_type_name = hcc_data_type_string(w->cu, cond_expr->data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_SWITCH_CONDITION_TYPE, (int)data_type_name.size, data_type_name.data);
				}

				token = hcc_ata_iter_peek(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_CONDITION_EXPR);
				}
				token = hcc_ata_iter_next(w->astgen.token_iter);
			}
			stmt->switch_.cond_expr = cond_expr;

			if (token != HCC_ATA_TOKEN_CURLY_OPEN) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_CURLY_OPEN_SWITCH_STATEMENT);
			}

			HccASTGenSwitchState* switch_state = &w->astgen.switch_state;
			HccASTGenSwitchState prev_switch_state = *switch_state;

			switch_state->switch_stmt = stmt;
			switch_state->first_switch_case = NULL;
			switch_state->prev_switch_case = NULL;
			switch_state->default_switch_case = NULL;
			switch_state->switch_condition_type = cond_expr->data_type;
			switch_state->case_stmts_count = 0;

			HccASTExpr* block_stmt = hcc_astgen_generate_stmt(w);
			block_stmt->is_stmt = true;

			stmt->switch_.first_case_expr = switch_state->first_switch_case;
			stmt->switch_.block_expr = block_stmt;
			stmt->next_stmt = switch_state->default_switch_case;
			stmt->location = location;

			*switch_state = prev_switch_state;
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_DO: {
			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_WHILE);
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);

			hcc_ata_iter_next(w->astgen.token_iter);

			bool prev_is_in_loop = w->astgen.is_in_loop;
			w->astgen.is_in_loop = true;
			HccASTExpr* loop_stmt = hcc_astgen_generate_stmt(w);
			loop_stmt->is_stmt = true;
			w->astgen.is_in_loop = prev_is_in_loop;

			token = hcc_ata_iter_peek(w->astgen.token_iter);
			if (token != HCC_ATA_TOKEN_KEYWORD_WHILE) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_WHILE_CONDITION_FOR_DO_WHILE);
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);

			HccASTExpr* cond_expr = hcc_astgen_generate_cond_expr(w);

			stmt->while_.cond_expr = cond_expr;
			stmt->while_.loop_stmt = loop_stmt;
			stmt->location = location;

			hcc_astgen_ensure_semicolon(w);
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_WHILE: {
			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_WHILE);
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);

			hcc_ata_iter_next(w->astgen.token_iter);

			HccASTExpr* cond_expr = hcc_astgen_generate_cond_expr(w);

			bool prev_is_in_loop = w->astgen.is_in_loop;
			w->astgen.is_in_loop = true;
			HccASTExpr* loop_stmt = hcc_astgen_generate_stmt(w);
			loop_stmt->is_stmt = true;
			w->astgen.is_in_loop = prev_is_in_loop;

			stmt->while_.cond_expr = cond_expr;
			stmt->while_.loop_stmt = loop_stmt;
			stmt->location = location;
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_FOR: {
			hcc_astgen_variable_stack_open(w);

			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_FOR);
			HccLocation* location = hcc_ata_iter_location(w->astgen.token_iter);

			token = hcc_ata_iter_next(w->astgen.token_iter);

			if (token != HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_OPEN_FOR);
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);

			HccASTExpr* init_expr = hcc_astgen_generate_expr(w, 0);
			if (init_expr->type == HCC_AST_EXPR_TYPE_DATA_TYPE) {
				token = hcc_ata_iter_peek(w->astgen.token_iter);
				if (token != HCC_ATA_TOKEN_IDENT) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FOR_VARIABLE_DECL);
				}
				init_expr = hcc_astgen_generate_variable_decl_stmt(w, init_expr->data_type);
			}
			hcc_astgen_ensure_semicolon(w);

			HccASTExpr* cond_expr = hcc_astgen_generate_expr(w, 0);
			hcc_astgen_data_type_ensure_is_condition(w, cond_expr->data_type);
			hcc_astgen_ensure_semicolon(w);

			HccASTExpr* inc_expr = hcc_astgen_generate_expr(w, 0);

			token = hcc_ata_iter_peek(w->astgen.token_iter);
			if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_PARENTHESIS_CLOSE_FOR);
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);

			bool prev_is_in_loop = w->astgen.is_in_loop;
			w->astgen.is_in_loop = true;
			HccASTExpr* loop_stmt = hcc_astgen_generate_stmt(w);
			loop_stmt->is_stmt = true;
			w->astgen.is_in_loop = prev_is_in_loop;

			stmt->for_.init_expr = init_expr;
			stmt->for_.cond_expr = cond_expr;
			stmt->for_.inc_expr = inc_expr;
			stmt->for_.loop_stmt = loop_stmt;
			stmt->location = location;

			hcc_astgen_variable_stack_close(w);
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_CASE: {
			HccASTGenSwitchState* switch_state = &w->astgen.switch_state;
			if (switch_state->switch_condition_type == HCC_DATA_TYPE_AST_BASIC_VOID) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_CASE_STATEMENT_OUTSIDE_OF_SWITCH);
			}

			token = hcc_ata_iter_next(w->astgen.token_iter);

			HccASTExpr* expr = hcc_astgen_generate_expr(w, 0);
			if (expr->type != HCC_AST_EXPR_TYPE_CONSTANT) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_SWITCH_CASE_VALUE_MUST_BE_A_CONSTANT);
			}
			HccLocation* other_location = NULL; // TODO: the switch condition expr
			hcc_astgen_data_type_ensure_compatible_assignment(w, other_location, switch_state->switch_condition_type, &expr);

			HccConstantId constant_id = expr->constant.id;

			expr->type = HCC_AST_EXPR_TYPE_STMT_CASE;
			expr->is_stmt = true;
			expr->next_stmt = NULL;
			expr->case_.constant_id = constant_id;
			expr->case_.next_case_stmt = NULL;

			token = hcc_ata_iter_peek(w->astgen.token_iter);
			if (token != HCC_ATA_TOKEN_COLON) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_CASE);
			}
			hcc_ata_iter_next(w->astgen.token_iter);

			//
			// TODO: add this constant to a linear array with a location in a parallel array and check to see
			// if this constant has already been used in the switch case

			if (switch_state->prev_switch_case) {
				switch_state->prev_switch_case->case_.next_case_stmt = expr;
			} else {
				switch_state->first_switch_case = expr;
			}

			switch_state->case_stmts_count += 1;
			switch_state->prev_switch_case = expr;
			return expr;
		};
		case HCC_ATA_TOKEN_KEYWORD_DEFAULT: {
			HccASTGenSwitchState* switch_state = &w->astgen.switch_state;
			if (switch_state->switch_condition_type == HCC_DATA_TYPE_AST_BASIC_VOID) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_DEFAULT_STATMENT_OUTSIDE_OF_SWITCH);
			}
			if (switch_state->default_switch_case) {
				hcc_astgen_error_1(w, HCC_ERROR_CODE_DEFAULT_STATEMENT_ALREADY_DECLARED);
			}

			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_DEFAULT);
			stmt->location = hcc_ata_iter_location(w->astgen.token_iter);
			stmt->is_stmt = true;

			token = hcc_ata_iter_next(w->astgen.token_iter);
			if (token != HCC_ATA_TOKEN_COLON) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_COLON_SWITCH_DEFAULT);
			}
			hcc_ata_iter_next(w->astgen.token_iter);

			switch_state->default_switch_case = stmt;
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_BREAK: {
			if (w->astgen.switch_state.switch_condition_type == HCC_DATA_TYPE_AST_BASIC_VOID && !w->astgen.is_in_loop) {
				hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_BREAK_STATEMENT_USAGE);
			}
			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_BREAK);
			stmt->is_stmt = true;
			stmt->location = hcc_ata_iter_location(w->astgen.token_iter);
			hcc_ata_iter_next(w->astgen.token_iter);
			hcc_astgen_ensure_semicolon(w);
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_CONTINUE: {
			if (w->astgen.switch_state.switch_condition_type == HCC_DATA_TYPE_AST_BASIC_VOID && !w->astgen.is_in_loop) {
				hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_CONTINUE_STATEMENT_USAGE);
			}
			HccASTExpr* stmt = hcc_astgen_alloc_expr(w, HCC_AST_EXPR_TYPE_STMT_CONTINUE);
			stmt->is_stmt = true;
			stmt->location = hcc_ata_iter_location(w->astgen.token_iter);
			hcc_ata_iter_next(w->astgen.token_iter);
			hcc_astgen_ensure_semicolon(w);
			return stmt;
		};
		case HCC_ATA_TOKEN_KEYWORD_TYPEDEF:
			hcc_astgen_generate_typedef(w);
			return NULL;
		case HCC_ATA_TOKEN_SEMICOLON:
			hcc_ata_iter_next(w->astgen.token_iter);
			return NULL;
		default: {
			hcc_astgen_generate_specifiers(w);
			HccASTExpr* expr = hcc_astgen_generate_expr(w, 0);
			if (expr->type == HCC_AST_EXPR_TYPE_DATA_TYPE) {
				token = hcc_ata_iter_peek(w->astgen.token_iter);
				token = hcc_astgen_generate_specifiers(w);

				if (token == HCC_ATA_TOKEN_IDENT) {
					expr = hcc_astgen_generate_variable_decl_stmt(w, expr->data_type);
				} else if (token == HCC_ATA_TOKEN_KEYWORD_TYPEDEF) {
					hcc_astgen_generate_typedef_with_data_type(w, expr->data_type);
				} else {
					hcc_astgen_ensure_no_unused_specifiers_identifier(w);
					expr = NULL;
				}
			} else {
				hcc_astgen_ensure_no_unused_specifiers_data_type(w);
			}

			hcc_astgen_ensure_semicolon(w);
			return expr;
		};
	}
}

void hcc_astgen_generate_function(HccWorker* w, HccDataType return_data_type, HccLocation* return_data_type_location) {
	HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
	HCC_DEBUG_ASSERT(token == HCC_ATA_TOKEN_IDENT, "internal error: expected '%s' at the start of generating a function", hcc_ata_token_strings[HCC_ATA_TOKEN_IDENT]);
	HccStringId identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
	HccLocation* identifier_location = hcc_ata_iter_location(w->astgen.token_iter);
	HccLocation* params_location = hcc_ata_iter_location(w->astgen.token_iter);

	bool found_static = false;
	bool found_extern = false;
	HccASTFunctionShaderStage shader_stage = HCC_AST_FUNCTION_SHADER_STAGE_NONE;
	HccASTFunctionFlags flags = 0;
	{
		if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_FUNCTION_SPECIFIERS) {
			HccASTGenSpecifier specifier = hcc_leastsetbitidx32(w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_NON_FUNCTION_SPECIFIERS);
			HccATAToken token = hcc_astgen_specifier_tokens[specifier];
			hcc_astgen_error_1(w, HCC_ERROR_CODE_INVALID_SPECIFIER_FUNCTION_DECL, hcc_ata_token_strings[token]);
		}

		if (!HCC_IS_POWER_OF_TWO_OR_ZERO(w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_ALL_SHADER_STAGES)) {
			hcc_astgen_error_1(w, HCC_ERROR_CODE_MULTIPLE_SHADER_STAGES_ON_FUNCTION);
		}

		if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_VERTEX) {
			shader_stage = HCC_AST_FUNCTION_SHADER_STAGE_VERTEX;
		} else if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_FRAGMENT) {
			shader_stage = HCC_AST_FUNCTION_SHADER_STAGE_FRAGMENT;
		}

		found_static |= w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_STATIC;
		found_extern |= w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_EXTERN;

		if (w->astgen.specifier_flags & HCC_ASTGEN_SPECIFIER_FLAGS_INLINE) {
			flags |= HCC_AST_FUNCTION_FLAGS_INLINE;
		}

		w->astgen.specifier_flags &= ~HCC_ASTGEN_SPECIFIER_FLAGS_ALL_FUNCTION_SPECIFIERS;
	}

	if (found_static && found_extern) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_STATIC_AND_EXTERN);
	}

	HccCU* cu = w->cu;
	HccASTFunction function = {0};
	w->astgen.function = &function;
	function.identifier_location = identifier_location;
	function.shader_stage = shader_stage;
	function.flags = flags;
	function.linkage = found_static ? HCC_AST_LINKAGE_INTERNAL : HCC_AST_LINKAGE_EXTERNAL;

	token = hcc_ata_iter_next(w->astgen.token_iter);

	function.identifier_string_id = identifier_string_id;
	function.return_data_type = return_data_type;
	function.return_data_type_location = return_data_type_location;

	hcc_astgen_variable_stack_open(w);

	token = hcc_ata_iter_next(w->astgen.token_iter);
	uint32_t param_idx = 0;
	if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
		//
		// parse the parameters into a temporary buffer
		function.params_and_variables = w->astgen.function_params_and_variables;
		while (1) {
			HccASTVariable* param = hcc_stack_push(w->astgen.function_params_and_variables);
			function.params_count += 1;

			HccDataType param_data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_EXPECTED_TYPE_NAME, true);
			if (shader_stage == HCC_AST_FUNCTION_SHADER_STAGE_NONE) {
				hcc_astgen_data_type_ensure_valid_variable(w, param_data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_FUNCTION_PARAM);
			}
			if (!(flags & HCC_AST_FUNCTION_FLAGS_INLINE)) {
				hcc_astgen_data_type_ensure_has_no_resources(w, param_data_type, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_FUNCTION_PARAM_INLINE);
				if (HCC_DATA_TYPE_IS_ARRAY(param_data_type)) {
					HccString data_type_name = hcc_data_type_string(w->cu, param_data_type);
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_INVALID_DATA_TYPE_FOR_FUNCTION_PARAM_INLINE, (int)data_type_name.size, data_type_name.data);
				}
			}

			token = hcc_ata_iter_peek(w->astgen.token_iter);
			if (token != HCC_ATA_TOKEN_IDENT) {
				hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_EXPECTED_IDENTIFIER_FUNCTION_PARAM);
			}
			HccStringId param_identifier_string_id = hcc_ata_iter_next_value(w->astgen.token_iter).string_id;
			HccLocation* param_identifier_location = hcc_ata_iter_location(w->astgen.token_iter);

			uint32_t existing_variable_id = hcc_astgen_variable_stack_find(w, param_identifier_string_id);
			if (existing_variable_id) {
				HccLocation* other_location = NULL; // TODO: location of existing variable
				HccString string = hcc_string_table_get(param_identifier_string_id);
				hcc_astgen_bail_error_2(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_FUNCTION_PARAM, other_location, (int)string.size, string.data);
			}
			hcc_astgen_variable_stack_add(w, param_identifier_string_id);
			token = hcc_ata_iter_next(w->astgen.token_iter);

			param->data_type = param_data_type;
			param->identifier_string_id = param_identifier_string_id;
			param->identifier_location = param_identifier_location;
			param->linkage = HCC_AST_LINKAGE_INTERNAL;
			param->storage_duration = HCC_AST_STORAGE_DURATION_AUTOMATIC;

			if (token != HCC_ATA_TOKEN_COMMA) {
				if (token != HCC_ATA_TOKEN_PARENTHESIS_CLOSE) {
					hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_FUNCTION_INVALID_TERMINATOR);
				}
				break;
			}
			token = hcc_ata_iter_next(w->astgen.token_iter);

			param_idx += 1;
		}
	}
	token = hcc_ata_iter_next(w->astgen.token_iter);

	//
	// validate the function prototype for shader stage entry points if this function is one
	switch (function.shader_stage) {
		case HCC_AST_FUNCTION_SHADER_STAGE_VERTEX: {
			if (function.params_count != 2) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_VERTEX, params_location);
			}
			HccASTVariable* param;
			HccDataType param_data_type;

			//
			// return data type
			if (hcc_decl_resolve_and_keep_qualifiers(w->cu, return_data_type) != HCC_DATA_TYPE_AST_BASIC_VOID) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_VERTEX, return_data_type_location);
			}

			//
			// param[0]: HccVertexInput
			param = hcc_stack_get(w->astgen.function_params_and_variables, 0);
			if (hcc_decl_resolve_and_keep_qualifiers(w->cu, param->data_type) != HCC_DATA_TYPE_CONST(HCC_DATA_TYPE(STRUCT, HCC_COMPOUND_DATA_TYPE_IDX_HCC_VERTEX_INPUT))) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_VERTEX, param->identifier_location);
			}

			//
			// param[1]: HCC_DEFINE_RASTERIZER_STATE
			param = hcc_stack_get(w->astgen.function_params_and_variables, 1);
			param_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, param->data_type);
			if (!HCC_DATA_TYPE_IS_POINTER(param_data_type) || HCC_DATA_TYPE_IS_CONST((param_data_type = hcc_data_type_strip_pointer(w->cu, param_data_type))) || !hcc_data_type_is_rasterizer_state(w->cu, param_data_type)) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_VERTEX, param->identifier_location);
			}
			break;
		};
		case HCC_AST_FUNCTION_SHADER_STAGE_FRAGMENT: {
			if (function.params_count != 3) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_FRAGMENT, params_location);
			}
			HccASTVariable* param;
			HccDataType param_data_type;

			//
			// return data type
			if (hcc_decl_resolve_and_keep_qualifiers(w->cu, return_data_type) != HCC_DATA_TYPE_AST_BASIC_VOID) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_FRAGMENT, return_data_type_location);
			}

			//
			// param[0]: HccFragmentInput
			param = hcc_stack_get(w->astgen.function_params_and_variables, 0);
			if (hcc_decl_resolve_and_keep_qualifiers(w->cu, param->data_type) != HCC_DATA_TYPE_CONST(HCC_DATA_TYPE(STRUCT, HCC_COMPOUND_DATA_TYPE_IDX_HCC_FRAGMENT_INPUT))) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_FRAGMENT, param->identifier_location);
			}

			//
			// param[1]: HCC_DEFINE_RASTERIZER_STATE
			param = hcc_stack_get(w->astgen.function_params_and_variables, 1);
			param_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, param->data_type);
			if (!HCC_DATA_TYPE_IS_POINTER(param_data_type) || !HCC_DATA_TYPE_IS_CONST((param_data_type = hcc_data_type_strip_pointer(w->cu, param_data_type))) || !hcc_data_type_is_rasterizer_state(w->cu, param_data_type)) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_FRAGMENT, param->identifier_location);
			}

			//
			// param[2]: HCC_DEFINE_FRAGMENT_STATE
			param = hcc_stack_get(w->astgen.function_params_and_variables, 2);
			param_data_type = hcc_decl_resolve_and_keep_qualifiers(w->cu, param->data_type);
			if (!HCC_DATA_TYPE_IS_POINTER(param_data_type) || HCC_DATA_TYPE_IS_CONST((param_data_type = hcc_data_type_strip_pointer(w->cu, param_data_type))) || !hcc_data_type_is_fragment_state(w->cu, param_data_type)) {
				hcc_astgen_bail_error_1_manual(w, HCC_ERROR_CODE_SHADER_PROTOTYPE_INVALID_FRAGMENT, param->identifier_location);
			}

			break;
		};
	}

	if (token == HCC_ATA_TOKEN_SEMICOLON) {
		hcc_astgen_variable_stack_close(w);
		hcc_ata_iter_next(w->astgen.token_iter);
		w->astgen.function = NULL;
		goto END;
	} else if (token != HCC_ATA_TOKEN_CURLY_OPEN) {
		hcc_astgen_bail_error_1(w, HCC_ERROR_CODE_UNEXPECTED_TOKEN_FUNCTION_PROTOTYPE_END, hcc_ata_token_strings[token]);
	}

	function.block_expr = NULL;
	if (token == HCC_ATA_TOKEN_CURLY_OPEN) {
		function.block_expr = hcc_astgen_generate_stmt(w);
	}

END: {}
	function.variables_count = w->astgen.next_var_idx;
	hcc_astgen_variable_stack_close(w);

	bool is_intrinsic =
		HCC_STRING_ID_INTRINSIC_FUNCTIONS_START <= identifier_string_id.idx_plus_one &&
		identifier_string_id.idx_plus_one < HCC_STRING_ID_INTRINSIC_FUNCTIONS_END;

	HccDecl decl = 0;
	bool is_definition = w->astgen.function != NULL;
	if (found_static && !is_intrinsic) {
		//
		// found static global variable so just add it to the global variable array for the AST
		HccASTVariable* params_and_variables = hcc_stack_push_many_thread_safe(cu->ast.function_params_and_variables, function.variables_count);
		HCC_COPY_ELMT_MANY(params_and_variables, w->astgen.function_params_and_variables, function.variables_count);
		function.params_and_variables = params_and_variables;

		HccASTFunction* dst_function = hcc_stack_push_thread_safe(cu->ast.functions);
		*dst_function = function;

		uint32_t function_idx = dst_function - cu->ast.functions;
		decl = HCC_DECL(FUNCTION, function_idx);
	} else if (is_definition || is_intrinsic) {
		//
		// try to add the global function definition to the compilation unit as it has external linkage
		HccHashTableInsert insert = hcc_hash_table_find_insert_idx(cu->global_declarations, &identifier_string_id);
		HccDeclEntryAtomic* entry = &cu->global_declarations[insert.idx];

		HccAtomic(HccDeclEntryAtomicLink*)* link_ptr = &entry->link;
		while (1) {
			HccDeclEntryAtomicLink* link = atomic_load(link_ptr);
			if (link == NULL && atomic_compare_exchange_strong(link_ptr, &link, HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL)) {
				//
				// our thread claimed the atomic link which means adding this function to the compilation unit is okay.
				// so now lets setup the function and atomic link.
				HccASTVariable* params_and_variables = hcc_stack_push_many_thread_safe(cu->ast.function_params_and_variables, function.variables_count);
				HCC_COPY_ELMT_MANY(params_and_variables, w->astgen.function_params_and_variables, function.variables_count);
				function.params_and_variables = params_and_variables;

				HccASTFunction* dst_function;
				uint32_t function_idx;
				if (is_intrinsic) {
					function_idx = identifier_string_id.idx_plus_one - HCC_STRING_ID_INTRINSIC_FUNCTIONS_START;
					dst_function = hcc_stack_get(cu->ast.functions, function_idx);
				} else {
					dst_function = hcc_stack_push_thread_safe(cu->ast.functions);
					function_idx = dst_function - cu->ast.functions;
				}
				*dst_function = function;

				decl = HCC_DECL(FUNCTION, function_idx);

				HccDeclEntryAtomicLink* alloced_link = HCC_ARENA_ALCTOR_ALLOC_ELMT(HccDeclEntryAtomicLink, &w->arena_alctor);
				alloced_link->decl = decl;
				atomic_store(link_ptr, alloced_link);
				break;
			}

			//
			// wait if any other thread is setting up the atomic link
			while (link == HCC_DECL_ENTRY_ATOMIC_LINK_SENTINAL) {
				HCC_CPU_RELAX();
				link = atomic_load(link_ptr);
			}

			if (HCC_DECL_IS_GLOBAL_VARIABLE(link->decl)) {
				HccASTVariable* other_variable = hcc_ast_global_variable_get(cu, link->decl);
				HccString identifier_string = hcc_string_table_get(identifier_string_id);
				hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_EXTERNAL, function.identifier_location, other_variable->identifier_location, (int)identifier_string.size, identifier_string.data);
			} else if (HCC_DECL_IS_FUNCTION(link->decl)) {
				if (is_intrinsic) {
					decl = link->decl;
					break;
				}

				HccASTFunction* other_function = hcc_ast_function_get(cu, link->decl);
				HccString identifier_string = hcc_string_table_get(identifier_string_id);
				hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_EXTERNAL, function.identifier_location, other_function->identifier_location, (int)identifier_string.size, identifier_string.data);
			}

			link_ptr = &link->next;
		}
	}

	//
	// insert the function in to this file's global declarations and perform the crazy C error checking
	HccDataType* insert_value_ptr;
	HccHashTableInsert insert = hcc_hash_table_find_insert_idx(w->astgen.ast_file->global_declarations, &identifier_string_id.idx_plus_one);
	HccDeclEntry* table_entry = &w->astgen.ast_file->global_declarations[insert.idx];
	if (insert.is_new) {
		if (decl == 0) {
			//
			// for the case when this is the first declaration in the HccASTFile and is 'T function();' with no body
			HccASTForwardDecl* forward_decl = hcc_stack_push_thread_safe(cu->ast.forward_declarations);
			forward_decl->ast_file = w->astgen.ast_file;
			forward_decl->identifier_location = function.identifier_location;
			forward_decl->identifier_string_id = function.identifier_string_id;
			forward_decl->function.return_data_type = function.return_data_type;

			HccASTVariable* params_and_variables = hcc_stack_push_many_thread_safe(cu->ast.function_params_and_variables, function.variables_count);
			HCC_COPY_ELMT_MANY(params_and_variables, w->astgen.function_params_and_variables, function.variables_count);
			forward_decl->function.params = params_and_variables;
			forward_decl->function.params_count = function.params_count;

			uint32_t forward_decl_idx = forward_decl - cu->ast.forward_declarations;
			decl = HCC_DECL_FORWARD_DECL(FUNCTION, forward_decl_idx);
			*hcc_stack_push(w->astgen.ast_file->forward_declarations_to_link) = decl;
		}

		table_entry->decl = decl;
		table_entry->location = identifier_location;
	} else {
		bool is_error = true;
		HccDecl found_decl = hcc_decl_resolve_and_strip_qualifiers(cu, table_entry->decl); // try to resolve a possible forward decl
		bool current_or_found_is_a_forward_declaration = HCC_DECL_IS_FORWARD_DECL(found_decl) || !is_definition;
		if (HCC_DECL_IS_FUNCTION(found_decl) && current_or_found_is_a_forward_declaration) {
			HccDataType other_return_data_type;
			HccASTVariable* other_params;
			uint32_t other_params_count;
			if (HCC_DECL_IS_FORWARD_DECL(found_decl)) {
				//
				// we have a 'T function();' forward declaration
				HccASTForwardDecl* forward_decl = hcc_ast_forward_decl_get(cu, found_decl);
				other_return_data_type = forward_decl->function.return_data_type;
				other_params = forward_decl->function.params;
				other_params_count = forward_decl->function.params_count;
			} else {
				//
				// we have a function definition 'T function() { ... }'
				HccASTFunction* other_function = hcc_ast_function_get(cu, found_decl);
				other_return_data_type = other_function->return_data_type;
				other_params = other_function->params_and_variables;
				other_params_count = other_function->params_count;
			}

			HccDataType return_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, function.return_data_type);
			other_return_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, other_return_data_type);

			is_error = false;
			is_error |= return_data_type != other_return_data_type;
			is_error |= function.params_count != other_params_count;
			if (!is_error) {
				for (uint32_t param_idx = 0; param_idx < function.params_count; param_idx += 1) {
					HccDataType param_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, function.params_and_variables[param_idx].data_type);
					HccDataType other_param_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, other_params[param_idx].data_type);
					if (param_data_type != other_param_data_type) {
						is_error = true;
						break;
					}
				}
			}
		}

		if (is_error) {
			HccString identifier_string = hcc_string_table_get(identifier_string_id);
			HccLocation* other_location = hcc_decl_location(cu, found_decl);
			hcc_astgen_bail_error_2_manual(w, HCC_ERROR_CODE_REDEFINITION_IDENTIFIER_INTERNAL, function.identifier_location, other_location, (int)identifier_string.size, identifier_string.data);
		}

		if (decl) {
			if (
				HCC_DECL_IS_FUNCTION(found_decl) &&
				HCC_DECL_IS_FORWARD_DECL(found_decl)
			) {
				// we have a function definition 'T function() { ... }' that can overwrite an 'T function();' declaration
				table_entry->decl = decl;
			}
		} else {
			// we found another 'T function();' so just reference the same decl as before.
			decl = found_decl;
		}
	}

	hcc_stack_pop_many(w->astgen.function_params_and_variables, function.variables_count);
	w->astgen.function = NULL;
}

void hcc_astgen_generate(HccWorker* w) {
	w->astgen.ast_file = w->job.arg;
	w->astgen.token_iter = hcc_ata_iter_start(w->astgen.ast_file);

	while (1) {
		HccATAToken token = hcc_ata_iter_peek(w->astgen.token_iter);
		token = hcc_astgen_generate_specifiers(w);

		switch (token) {
			case HCC_ATA_TOKEN_EOF:
				goto END_OF_FILE;
			case HCC_ATA_TOKEN_KEYWORD_TYPEDEF:
				hcc_astgen_generate_typedef(w);
				break;
			default: {
				HccDataType data_type = hcc_astgen_generate_data_type(w, HCC_ERROR_CODE_UNEXPECTED_TOKEN, true);
				HccLocation* data_type_location = hcc_ata_iter_location(w->astgen.token_iter);
				token = hcc_astgen_generate_specifiers(w);
				bool ensure_semi_colon = true;
				if (token == HCC_ATA_TOKEN_IDENT) {
					if (hcc_ata_iter_peek_ahead(w->astgen.token_iter, 1) == HCC_ATA_TOKEN_PARENTHESIS_OPEN) {
						hcc_astgen_generate_function(w, data_type, data_type_location);
						ensure_semi_colon = false;
					} else {
						HccDataType element_data_type = hcc_data_type_strip_all_pointers(w->cu, data_type);
						while (1) {
							hcc_astgen_generate_variable_decl(w, true, element_data_type, &data_type, NULL);
							token = hcc_ata_iter_peek(w->astgen.token_iter);
							if (token != HCC_ATA_TOKEN_COMMA) {
								break;
							}
							data_type = hcc_astgen_generate_pointer_data_type_if_exists(w, element_data_type);
							token = hcc_ata_iter_next(w->astgen.token_iter);
						}
					}
				} else if (token == HCC_ATA_TOKEN_KEYWORD_TYPEDEF) {
					hcc_astgen_generate_typedef_with_data_type(w, data_type);
				} else {
					hcc_astgen_ensure_no_unused_specifiers_identifier(w);
				}
				if (ensure_semi_colon) {
					hcc_astgen_ensure_semicolon(w);
				}
				break;
			};
		}
	}

END_OF_FILE:{}
	hcc_ata_iter_finish(w->astgen.ast_file, w->astgen.token_iter);
}

