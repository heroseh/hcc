#pragma once

#include <hcc_interop.h>

typedef uint16_t HccShader;
enum HccShader {
	HCC_SHADER_triangle_vs,
	HCC_SHADER_triangle_ps,
	HCC_SHADER_compute_square_cs,
	HCC_SHADER_texture_vs,
	HCC_SHADER_texture_ps,
	HCC_SHADER_color_picker_vs,
	HCC_SHADER_color_picker_ps,
	HCC_SHADER_blob_vacation_vs,
	HCC_SHADER_blob_vacation_ps,
	HCC_SHADER_voxel_raytracer_cs,
	HCC_SHADER_sdf2d_vs,
	HCC_SHADER_sdf2d_ps,
	HCC_SHADER_COUNT,
};

typedef uint16_t HccResourceStruct;
enum HccResourceStruct {
	HccResourceStruct_INVALID,
	HCC_RESOURCE_STRUCT_TriangleBC,
	HCC_RESOURCE_STRUCT_ComputeSquareBC,
	HCC_RESOURCE_STRUCT_TextureBC,
	HCC_RESOURCE_STRUCT_VoxelModel,
	HCC_RESOURCE_STRUCT_VoxelRaytracerBC,
	HCC_RESOURCE_STRUCT_SDFShape,
	HCC_RESOURCE_STRUCT_SDF2dBC,
};

const char* hcc_shader_names[] = {
	"triangle_vs",
	"triangle_ps",
	"compute_square_cs",
	"texture_vs",
	"texture_ps",
	"color_picker_vs",
	"color_picker_ps",
	"blob_vacation_vs",
	"blob_vacation_ps",
	"voxel_raytracer_cs",
	"sdf2d_vs",
	"sdf2d_ps",
};

HccShaderInfo hcc_shader_infos[] = {
	{
		/* .name = */                   "triangle_vs",
		/* .stage = */                  HCC_SHADER_STAGE_VERTEX,
		/* .bundled_constants_size = */ 24,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "triangle_ps",
		/* .stage = */                  HCC_SHADER_STAGE_PIXEL,
		/* .bundled_constants_size = */ 24,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "compute_square_cs",
		/* .stage = */                  HCC_SHADER_STAGE_COMPUTE,
		/* .bundled_constants_size = */ 4,
		/* .dispatch_group_size_x = */  8,
		/* .dispatch_group_size_y = */  8,
		/* .dispatch_group_size_z = */  1,
	},
	{
		/* .name = */                   "texture_vs",
		/* .stage = */                  HCC_SHADER_STAGE_VERTEX,
		/* .bundled_constants_size = */ 48,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "texture_ps",
		/* .stage = */                  HCC_SHADER_STAGE_PIXEL,
		/* .bundled_constants_size = */ 48,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "color_picker_vs",
		/* .stage = */                  HCC_SHADER_STAGE_VERTEX,
		/* .bundled_constants_size = */ 12,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "color_picker_ps",
		/* .stage = */                  HCC_SHADER_STAGE_PIXEL,
		/* .bundled_constants_size = */ 12,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "blob_vacation_vs",
		/* .stage = */                  HCC_SHADER_STAGE_VERTEX,
		/* .bundled_constants_size = */ 12,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "blob_vacation_ps",
		/* .stage = */                  HCC_SHADER_STAGE_PIXEL,
		/* .bundled_constants_size = */ 12,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "voxel_raytracer_cs",
		/* .stage = */                  HCC_SHADER_STAGE_COMPUTE,
		/* .bundled_constants_size = */ 24,
		/* .dispatch_group_size_x = */  8,
		/* .dispatch_group_size_y = */  8,
		/* .dispatch_group_size_z = */  1,
	},
	{
		/* .name = */                   "sdf2d_vs",
		/* .stage = */                  HCC_SHADER_STAGE_VERTEX,
		/* .bundled_constants_size = */ 16,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "sdf2d_ps",
		/* .stage = */                  HCC_SHADER_STAGE_PIXEL,
		/* .bundled_constants_size = */ 16,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
};

HccResourceInfo TriangleBC_resources[] = {
	{
		/* .name = */        "hprintf_buffer",
		/* .offset = */      16,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_WRITE,
		/* .type = */        HCC_RESOURCE_TYPE_BUFFER,
	},
	{
		/* .name = */        "vertices",
		/* .offset = */      20,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_BUFFER,
	},
};

HccResourceInfo ComputeSquareBC_resources[] = {
	{
		/* .name = */        "output",
		/* .offset = */      0,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_WRITE,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_2D,
	},
};

HccResourceInfo TextureBC_resources[] = {
	{
		/* .name = */        "gray_texture",
		/* .offset = */      0,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_2D,
	},
	{
		/* .name = */        "red_green_texture",
		/* .offset = */      4,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_2D,
	},
	{
		/* .name = */        "logo_texture",
		/* .offset = */      8,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_2D,
	},
	{
		/* .name = */        "sample_logo_texture",
		/* .offset = */      12,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_SAMPLE,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_2D,
	},
	{
		/* .name = */        "hprintf_buffer",
		/* .offset = */      16,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_WRITE,
		/* .type = */        HCC_RESOURCE_TYPE_BUFFER,
	},
	{
		/* .name = */        "sampler",
		/* .offset = */      20,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_SAMPLER,
	},
};

HccResourceInfo VoxelModel_resources[] = {
	{
		/* .name = */        "color",
		/* .offset = */      24,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_3D,
	},
};

HccResourceInfo VoxelRaytracerBC_resources[] = {
	{
		/* .name = */        "models",
		/* .offset = */      0,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_BUFFER,
	},
	{
		/* .name = */        "output",
		/* .offset = */      4,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_2D,
	},
	{
		/* .name = */        "hprintf_buffer",
		/* .offset = */      8,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_BUFFER,
	},
};

HccResourceInfo SDFShape_resources[] = {
	{
		/* .name = */        "texture",
		/* .offset = */      0,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_SAMPLE,
		/* .type = */        HCC_RESOURCE_TYPE_TEXTURE_2D,
	},
};

HccResourceInfo SDF2dBC_resources[] = {
	{
		/* .name = */        "shapes",
		/* .offset = */      0,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_BUFFER,
	},
	{
		/* .name = */        "sampler",
		/* .offset = */      4,
		/* .access_mode = */ HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
		/* .type = */        HCC_RESOURCE_TYPE_SAMPLER,
	},
};

HccResourceStructInfo hcc_resource_structs[] = {
	{0},
	{
		/* .name = */            "TriangleBC",
		/* .resources = */       TriangleBC_resources,
		/* .resources_count = */ 2,
		/* .size = */            24,
		/* .align = */           4,
	},
	{
		/* .name = */            "ComputeSquareBC",
		/* .resources = */       ComputeSquareBC_resources,
		/* .resources_count = */ 1,
		/* .size = */            4,
		/* .align = */           4,
	},
	{
		/* .name = */            "TextureBC",
		/* .resources = */       TextureBC_resources,
		/* .resources_count = */ 6,
		/* .size = */            48,
		/* .align = */           4,
	},
	{
		/* .name = */            "VoxelModel",
		/* .resources = */       VoxelModel_resources,
		/* .resources_count = */ 1,
		/* .size = */            28,
		/* .align = */           4,
	},
	{
		/* .name = */            "VoxelRaytracerBC",
		/* .resources = */       VoxelRaytracerBC_resources,
		/* .resources_count = */ 3,
		/* .size = */            24,
		/* .align = */           4,
	},
	{
		/* .name = */            "SDFShape",
		/* .resources = */       SDFShape_resources,
		/* .resources_count = */ 1,
		/* .size = */            16,
		/* .align = */           4,
	},
	{
		/* .name = */            "SDF2dBC",
		/* .resources = */       SDF2dBC_resources,
		/* .resources_count = */ 2,
		/* .size = */            16,
		/* .align = */           4,
	},
};

HccMetadata hcc_metadata = {
	/* .shaders = */                    hcc_shader_infos,
	/* .resource_structs = */           hcc_resource_structs,
	/* .shaders_count = */              12,
	/* .resource_structs_count = */     7,
	/* .bundled_constants_size_max = */ 48,
	/* .resource_descriptors_max = */   1024,
};

