#pragma once

#include <hcc_interop.h>

typedef uint16_t HccShader;
enum HccShader {
	HCC_SHADER_vertex,
	HCC_SHADER_fragment,
};

typedef uint16_t HccResourceStruct;
enum HccResourceStruct {
	HccResourceStruct_INVALID,
};

HccShaderInfo hcc_shader_infos[] = {
	{
		/* .name = */                   "vertex",
		/* .stage = */                  HCC_SHADER_STAGE_VERTEX,
		/* .bundled_constants_size = */ 12,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
	{
		/* .name = */                   "fragment",
		/* .stage = */                  HCC_SHADER_STAGE_FRAGMENT,
		/* .bundled_constants_size = */ 12,
		/* .dispatch_group_size_x = */  0,
		/* .dispatch_group_size_y = */  0,
		/* .dispatch_group_size_z = */  0,
	},
};

HccResourceStructInfo hcc_resource_structs[] = {
	{0},
};

HccMetadata hcc_metadata = {
	/* .shaders = */                    hcc_shader_infos,
	/* .resource_structs = */           hcc_resource_structs,
	/* .shaders_count = */              2,
	/* .resource_structs_count = */     0,
	/* .bundled_constants_size_max = */ 12,
	/* .resource_descriptors_max = */   1024,
};

