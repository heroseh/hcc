
#include "hcc_internal.h"

void hcc_astlink_init(HccWorker* w, HccASTLinkSetup* setup) {
	HCC_UNUSED(w);
	HCC_UNUSED(setup);

}

void hcc_astlink_deinit(HccWorker* w) {
	HCC_UNUSED(w);
}

void hcc_astlink_reset(HccWorker* w) {
	w->astlink.ast_file = NULL;
}

void hcc_astlink_error_1_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, ...) {
	va_list va_args;
	va_start(va_args, location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, NULL, va_args);
	va_end(va_args);
}

void hcc_astlink_error_2_manual(HccWorker* w, HccErrorCode error_code, HccLocation* location, HccLocation* other_location, ...) {
	va_list va_args;
	va_start(va_args, other_location);
	hcc_error_pushv(hcc_worker_task(w), error_code, location, other_location, va_args);
	va_end(va_args);
}

void hcc_astlink_link_file(HccWorker* w) {
	w->astlink.ast_file = w->job.arg;
	HccASTFile* ast_file = w->job.arg;
	HccCU* cu = w->cu;

	//
	// loop over all of the forward declared global variables and functions for this HccASTFile
	// and add the resolved definition to the HccASTFile.global_declarations hash table.
	uint32_t decls_to_link_count = hcc_stack_count(ast_file->forward_declarations_to_link);
	for (uint32_t decls_to_link_idx = 0; decls_to_link_idx < decls_to_link_count; decls_to_link_idx += 1) {
		HccDecl decl = ast_file->forward_declarations_to_link[decls_to_link_idx];
		HccASTForwardDecl* forward_decl = hcc_ast_forward_decl_get(cu, decl);
		uintptr_t found_idx = hcc_hash_table_find_idx(ast_file->global_declarations, &forward_decl->identifier_string_id);
		HCC_DEBUG_ASSERT(found_idx != UINTPTR_MAX, "forward declaration should at least have a hash table entry");

		HccDeclEntry* ast_file_decl_entry = &ast_file->global_declarations[found_idx];
		if (ast_file_decl_entry->decl != decl) {
			//
			// the declaration was later on defined inside the file,
			// so the linking work and all of the error checking would have been done in astgen.c
			continue;
		}

		HccDecl found_decl = 0;
		bool is_intrinsic =
			HCC_STRING_ID_INTRINSIC_FUNCTIONS_START <= forward_decl->identifier_string_id.idx_plus_one &&
			forward_decl->identifier_string_id.idx_plus_one < HCC_STRING_ID_INTRINSIC_FUNCTIONS_END;
		if (is_intrinsic) {
			uint32_t function_idx = forward_decl->identifier_string_id.idx_plus_one - HCC_STRING_ID_INTRINSIC_FUNCTIONS_START;
			found_decl = HCC_DECL(FUNCTION, function_idx);
			goto LINK_DECL_END;
		}

		found_idx = hcc_hash_table_find_idx(cu->global_declarations, &forward_decl->identifier_string_id);
		if (found_idx != UINTPTR_MAX) {
			//
			// try and find the definition for the forward declaration in the compiliation unit
			//

			//
			// cast away the HccAtomic as this hash table is read-only during the phase of the compiler.
			HccDeclEntryAtomicLink** link_ptr = (HccDeclEntryAtomicLink**)&cu->global_declarations[found_idx].link;
			while (1) {
				HccDeclEntryAtomicLink* link = *link_ptr;
				if (link == NULL) {
					break;
				}

				if (HCC_DECL_TYPE(decl) == HCC_DECL_TYPE(link->decl)) {
					switch (HCC_DECL_TYPE(decl)) {
						case HCC_DECL_FUNCTION: {
							HccASTFunction* function = hcc_ast_function_get(cu, link->decl);

							HccDataType other_return_data_type = function->return_data_type;
							HccASTVariable* other_params = function->params_and_variables;
							uint32_t other_params_count = function->params_count;

							HccDataType return_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, forward_decl->function.return_data_type);
							other_return_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, other_return_data_type);

							bool is_error = false;
							is_error |= return_data_type != other_return_data_type;
							is_error |= forward_decl->function.params_count != other_params_count;
							if (!is_error) {
								for (uint32_t param_idx = 0; param_idx < forward_decl->function.params_count; param_idx += 1) {
									HccDataType param_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, forward_decl->function.params[param_idx].data_type);
									HccDataType other_param_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, other_params[param_idx].data_type);
									if (param_data_type != other_param_data_type) {
										is_error = true;
										break;
									}
								}
							}

							if (is_error) {
								HccString identifier_string = hcc_string_table_get(forward_decl->identifier_string_id);
								HccLocation* other_location = hcc_decl_location(cu, link->decl);
								hcc_astlink_error_2_manual(w, HCC_ERROR_CODE_LINK_FUNCTION_PROTOTYPE_MISMATCH, forward_decl->identifier_location, other_location, (int)identifier_string.size, identifier_string.data);
								goto NEXT_FORWARD_DECL;
							}

							found_decl = link->decl;
							goto LINK_DECL_END;
						};
						case HCC_DECL_GLOBAL_VARIABLE: {
							HccASTVariable* variable = hcc_ast_global_variable_get(cu, link->decl);

							HccDataType data_type = hcc_decl_resolve_and_strip_qualifiers(cu, forward_decl->variable.data_type);
							HccDataType other_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, variable->data_type);

							bool is_error = data_type != other_data_type;
							if (is_error) {
								HccString identifier_string = hcc_string_table_get(forward_decl->identifier_string_id);
								HccLocation* other_location = hcc_decl_location(cu, link->decl);
								hcc_astlink_error_2_manual(w, HCC_ERROR_CODE_LINK_GLOBAL_VARIABLE_MISMATCH, forward_decl->identifier_location, other_location, (int)identifier_string.size, identifier_string.data);
								goto NEXT_FORWARD_DECL;
							}

							found_decl = link->decl;
							goto LINK_DECL_END;
						};
					}
				}

				link_ptr = (HccDeclEntryAtomicLink**)&link->next;
			}
		}

LINK_DECL_END:
		if (found_decl == 0) {
			//
			// FUTURE: if we target a proper OS in future, we would skip this error if we have legacy kind of linking.
			HccString identifier_string = hcc_string_table_get(forward_decl->identifier_string_id);
			hcc_astlink_error_1_manual(w, HCC_ERROR_CODE_LINK_DECLARATION_UNDEFINED, forward_decl->identifier_location, (int)identifier_string.size, identifier_string.data);
			goto NEXT_FORWARD_DECL;
		}

		//
		// success, we can now update the global declaration entry inside the HccASTFile with a concrete definitions
		ast_file_decl_entry->decl = found_decl;

NEXT_FORWARD_DECL: {}
	}
}

