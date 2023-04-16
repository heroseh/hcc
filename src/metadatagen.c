#include "hcc_internal.h"

void hcc_metadatagen_generate_c_resources(HccCU* cu, HccIIO* iio, HccCompoundDataType* dt, const char* base_name, uint32_t base_offset, uint32_t* resources_count_mut) {
	for (uint32_t field_idx = 0; field_idx < dt->fields_count; field_idx += 1) {
		HccCompoundField* field = &dt->fields[field_idx];
		HccString field_name = field->identifier_string_id.idx_plus_one ? hcc_string_table_get(field->identifier_string_id) : hcc_string_lit("");
		HccDataType field_data_type = hcc_decl_resolve_and_strip_qualifiers(cu, field->data_type);
		if (HCC_DATA_TYPE_IS_STRUCT(field_data_type)) {
			char name[512];
			snprintf(name, sizeof(name), "%s%.*s%s", base_name, (int)field_name.size, field_name.data, field_name.size ? "." : "");
			hcc_metadatagen_generate_c_resources(cu, iio, hcc_compound_data_type_get(cu, field_data_type), name, base_offset + field->byte_offset, resources_count_mut);
		} else if (HCC_DATA_TYPE_IS_RESOURCE(field_data_type)) {
			HccResourceDataType resource_data_type = HCC_DATA_TYPE_AUX(field_data_type);
			HccResourceType resource_type;
			switch (HCC_RESOURCE_DATA_TYPE_TYPE(resource_data_type)) {
				case HCC_RESOURCE_DATA_TYPE_BUFFER:
					resource_type = HCC_RESOURCE_TYPE_BUFFER;
					break;
				case HCC_RESOURCE_DATA_TYPE_TEXTURE:
					switch (HCC_RESOURCE_DATA_TYPE_TEXTURE_DIM(resource_data_type)) {
						case HCC_TEXTURE_DIM_1D:
							resource_type = HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_ARRAY(resource_data_type) ? HCC_RESOURCE_TYPE_TEXTURE_1D_ARRAY : HCC_RESOURCE_TYPE_TEXTURE_1D;
							break;
						case HCC_TEXTURE_DIM_2D:
							switch (HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_MS(resource_data_type) * 2 + HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_ARRAY(resource_data_type)) {
								case 0: resource_type = HCC_RESOURCE_TYPE_TEXTURE_2D; break;
								case 1: resource_type = HCC_RESOURCE_TYPE_TEXTURE_2D_ARRAY; break;
								case 2: resource_type = HCC_RESOURCE_TYPE_TEXTURE_2D_MS; break;
								case 3: resource_type = HCC_RESOURCE_TYPE_TEXTURE_2D_MS_ARRAY; break;
							}
							break;
						case HCC_TEXTURE_DIM_3D:
							resource_type = HCC_RESOURCE_TYPE_TEXTURE_3D;
							break;
						case HCC_TEXTURE_DIM_CUBE:
							resource_type = HCC_RESOURCE_DATA_TYPE_TEXTURE_IS_ARRAY(resource_data_type) ? HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE : HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE_ARRAY;
							break;
					}
					break;
				case HCC_RESOURCE_DATA_TYPE_SAMPLER:
					resource_type = HCC_RESOURCE_TYPE_SAMPLER;
					break;
			}
			hcc_iio_write_fmt(iio, "\t{\n");
			hcc_iio_write_fmt(iio, "\t\t/* .name = */        \"%s%.*s\",\n", base_name, (int)field_name.size, field_name.data);
			hcc_iio_write_fmt(iio, "\t\t/* .offset = */      %u,\n", base_offset + field->byte_offset);
			hcc_iio_write_fmt(iio, "\t\t/* .access_mode = */ %s,\n", hcc_resource_access_mode_strings[HCC_RESOURCE_DATA_TYPE_ACCESS_MODE(resource_data_type)]);
			hcc_iio_write_fmt(iio, "\t\t/* .type = */        %s,\n", hcc_resource_type_strings[resource_type]);
			hcc_iio_write_fmt(iio, "\t},\n");

			*resources_count_mut += 1;
		}
	}
}

void hcc_metadatagen_generate_c(HccCU* cu, HccIIO* iio) {
	HccString shader_enum_name = hcc_options_get_string(cu->options, HCC_OPTION_KEY_SHADER_ENUM_NAME);
	HccString shader_enum_prefix = hcc_options_get_string(cu->options, HCC_OPTION_KEY_SHADER_ENUM_PREFIX);
	HccString shader_infos_name = hcc_options_get_string(cu->options, HCC_OPTION_KEY_SHADER_INFOS_NAME);
	HccString resource_structs_enum_name = hcc_options_get_string(cu->options, HCC_OPTION_KEY_RESOURCE_STRUCTS_ENUM_NAME);
	HccString resource_structs_enum_prefix = hcc_options_get_string(cu->options, HCC_OPTION_KEY_RESOURCE_STRUCTS_ENUM_PREFIX);
	uint32_t resource_descriptors_max = hcc_options_get_u32(cu->options, HCC_OPTION_KEY_RESOURCE_DESCRIPTORS_MAX);

	hcc_iio_write_fmt(iio, "#pragma once\n\n");
	hcc_iio_write_fmt(iio, "#include <hcc_interop.h>\n\n");

	hcc_iio_write_fmt(iio, "typedef uint16_t %.*s;\n", (int)shader_enum_name.size, shader_enum_name.data);
	hcc_iio_write_fmt(iio, "enum %.*s {\n", (int)shader_enum_name.size, shader_enum_name.data);
	for (uint32_t shader_idx = 0; shader_idx < hcc_stack_count(cu->shader_function_decls); shader_idx += 1) {
		HccDecl decl = cu->shader_function_decls[shader_idx];
		HccASTFunction* function = hcc_ast_function_get(cu, decl);
		HccString function_name = hcc_string_table_get(function->identifier_string_id);

		hcc_iio_write_fmt(iio, "\t%.*s%.*s,\n", (int)shader_enum_prefix.size, shader_enum_prefix.data, (int)function_name.size, function_name.data);
	}
	hcc_iio_write_fmt(iio, "};\n\n");

	hcc_iio_write_fmt(iio, "typedef uint16_t %.*s;\n", (int)resource_structs_enum_name.size, resource_structs_enum_name.data);
	hcc_iio_write_fmt(iio, "enum %.*s {\n", (int)resource_structs_enum_name.size, resource_structs_enum_name.data);
	hcc_iio_write_fmt(iio, "\t%.*s_INVALID,\n", (int)resource_structs_enum_name.size, resource_structs_enum_name.data);
	for (uint32_t struct_idx = 0; struct_idx < hcc_stack_count(cu->resource_structs); struct_idx += 1) {
		HccDataType data_type = cu->resource_structs[struct_idx];
		HccCompoundDataType* dt = hcc_compound_data_type_get(cu, data_type);

		char name_buf[1024];
		HccString dt_name;
		if (dt->identifier_string_id.idx_plus_one == 0) {
			snprintf(name_buf, sizeof(name_buf), "__ANON_%u", HCC_DATA_TYPE_AUX(data_type));
			dt_name = hcc_string_c(name_buf);
		} else {
			dt_name = hcc_string_table_get(dt->identifier_string_id);
		}

		hcc_iio_write_fmt(iio, "\t%.*s%.*s,\n", (int)resource_structs_enum_prefix.size, resource_structs_enum_prefix.data, (int)dt_name.size, dt_name.data);
	}
	hcc_iio_write_fmt(iio, "};\n\n");

	uint32_t bundled_constants_size_max = 0;
	hcc_iio_write_fmt(iio, "HccShaderInfo %.*s[] = {\n", (int)shader_infos_name.size, shader_infos_name.data);
	for (uint32_t shader_idx = 0; shader_idx < hcc_stack_count(cu->shader_function_decls); shader_idx += 1) {
		HccDecl decl = cu->shader_function_decls[shader_idx];
		HccASTFunction* function = hcc_ast_function_get(cu, decl);
		HccString function_name = hcc_string_table_get(function->identifier_string_id);

		uint32_t param_idx;
		switch (function->shader_stage) {
			case HCC_SHADER_STAGE_VERTEX:
				param_idx = HCC_VERTEX_SHADER_PARAM_BC;
				break;
			case HCC_SHADER_STAGE_PIXEL:
				param_idx = HCC_PIXEL_SHADER_PARAM_BC;
				break;
			case HCC_SHADER_STAGE_COMPUTE:
				param_idx = HCC_COMPUTE_SHADER_PARAM_BC;
				break;
			HCC_ABORT("unhandled shader type %u", function->shader_stage);
		}
		HccDataType bc_data_type = hcc_data_type_strip_pointer(cu, hcc_decl_resolve_and_strip_qualifiers(cu, function->params_and_variables[param_idx].data_type));

		uint64_t bundled_constants_size;
		uint64_t bundled_constants_align;
		hcc_data_type_size_align(cu, bc_data_type, &bundled_constants_size, &bundled_constants_align);

		hcc_iio_write_fmt(iio, "\t{\n");
		hcc_iio_write_fmt(iio, "\t\t/* .name = */                   \"%.*s\",\n", (int)function_name.size, function_name.data);
		hcc_iio_write_fmt(iio, "\t\t/* .stage = */                  %s,\n", hcc_shader_stage_strings[function->shader_stage]);
		hcc_iio_write_fmt(iio, "\t\t/* .bundled_constants_size = */ %u,\n", (uint32_t)bundled_constants_size);
		hcc_iio_write_fmt(iio, "\t\t/* .dispatch_group_size_x = */  %u,\n", function->compute_dispatch_group_size_x);
		hcc_iio_write_fmt(iio, "\t\t/* .dispatch_group_size_y = */  %u,\n", function->compute_dispatch_group_size_y);
		hcc_iio_write_fmt(iio, "\t\t/* .dispatch_group_size_z = */  %u,\n", function->compute_dispatch_group_size_z);
		hcc_iio_write_fmt(iio, "\t},\n");

		if (bundled_constants_size_max < bundled_constants_size) {
			bundled_constants_size_max = bundled_constants_size;
		}
	}
	hcc_iio_write_fmt(iio, "};\n\n");

	for (uint32_t struct_idx = 0; struct_idx < hcc_stack_count(cu->resource_structs); struct_idx += 1) {
		HccDataType data_type = cu->resource_structs[struct_idx];
		HccCompoundDataType* dt = hcc_compound_data_type_get(cu, data_type);

		char name_buf[1024];
		HccString dt_name;
		if (dt->identifier_string_id.idx_plus_one == 0) {
			snprintf(name_buf, sizeof(name_buf), "__ANON_%u", HCC_DATA_TYPE_AUX(data_type));
			dt_name = hcc_string_c(name_buf);
		} else {
			dt_name = hcc_string_table_get(dt->identifier_string_id);
		}

		hcc_iio_write_fmt(iio, "HccResourceInfo %.*s_resources[] = {\n", (int)dt_name.size, dt_name.data);
		uint32_t resources_count = 0;
		hcc_metadatagen_generate_c_resources(cu, iio, dt, "", 0, &resources_count);
		hcc_iio_write_fmt(iio, "};\n\n");
	}

	hcc_iio_write_fmt(iio, "HccResourceStructInfo hcc_resource_structs[] = {\n");
	hcc_iio_write_fmt(iio, "\t{0},\n");
	for (uint32_t struct_idx = 0; struct_idx < hcc_stack_count(cu->resource_structs); struct_idx += 1) {
		HccDataType data_type = cu->resource_structs[struct_idx];
		HccCompoundDataType* dt = hcc_compound_data_type_get(cu, data_type);

		char name_buf[1024];
		HccString dt_name;
		if (dt->identifier_string_id.idx_plus_one == 0) {
			snprintf(name_buf, sizeof(name_buf), "__ANON_%u", HCC_DATA_TYPE_AUX(data_type));
			dt_name = hcc_string_c(name_buf);
		} else {
			dt_name = hcc_string_table_get(dt->identifier_string_id);
		}

		uint32_t resources_count = 0;
		hcc_metadatagen_generate_c_resources(cu, NULL, dt, "", 0, &resources_count);

		uint64_t size;
		uint64_t align;
		hcc_data_type_size_align(cu, data_type, &size, &align);

		hcc_iio_write_fmt(iio, "\t{\n");
		hcc_iio_write_fmt(iio, "\t\t/* .name = */            \"%.*s\",\n", (int)dt_name.size, dt_name.data);
		hcc_iio_write_fmt(iio, "\t\t/* .resources = */       %.*s_resources,\n", (int)dt_name.size, dt_name.data);
		hcc_iio_write_fmt(iio, "\t\t/* .resources_count = */ %u,\n", resources_count);
		hcc_iio_write_fmt(iio, "\t\t/* .size = */            %u,\n", (uint32_t)size);
		hcc_iio_write_fmt(iio, "\t\t/* .align = */           %u,\n", (uint32_t)align);
		hcc_iio_write_fmt(iio, "\t},\n");
	}
	hcc_iio_write_fmt(iio, "};\n\n");

	hcc_iio_write_fmt(iio, "HccMetadata hcc_metadata = {\n");
	hcc_iio_write_fmt(iio, "\t/* .shaders = */                    %.*s,\n", (int)shader_infos_name.size, shader_infos_name.data);
	hcc_iio_write_fmt(iio, "\t/* .resource_structs = */           hcc_resource_structs,\n");
	hcc_iio_write_fmt(iio, "\t/* .shaders_count = */              %u,\n", (uint32_t)hcc_stack_count(cu->shader_function_decls));
	hcc_iio_write_fmt(iio, "\t/* .resource_structs_count = */     %u,\n", (uint32_t)hcc_stack_count(cu->resource_structs));
	hcc_iio_write_fmt(iio, "\t/* .bundled_constants_size_max = */ %u,\n", bundled_constants_size_max);
	hcc_iio_write_fmt(iio, "\t/* .resource_descriptors_max = */   %u,\n", resource_descriptors_max);
	hcc_iio_write_fmt(iio, "};\n\n");
}

void hcc_metadatagen_generate_json(HccCU* cu, HccIIO* iio) {
	HCC_UNUSED(cu);
	HCC_UNUSED(iio);
}

