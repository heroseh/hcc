#pragma once
#ifndef _HCC_INTEROP_H_
#define _HCC_INTEROP_H_


//
// TODO: hcc will generate an enum and array of HccResourceStructInfo for all of the structures that contain resources for reflection and sync code
// TODO: hcc will generate an enum and array of HccShaderInfo
//

#include <stdint.h>

#ifndef HCC_INTEROP_ASSERT
#define HCC_INTEROP_ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, __VA_ARGS__); exit(1); }
#endif

#define HCC_INTEROP_ARRAY_COUNT(arr) (sizeof(arr) / sizeof(*(arr)))
#define HCC_INTEROP_UNUSED(expr) (void)(expr)

typedef uint8_t HccShaderStage;
enum HccShaderStage {
	HCC_SHADER_STAGE_NONE,
	HCC_SHADER_STAGE_VERTEX,
	HCC_SHADER_STAGE_PIXEL,
	HCC_SHADER_STAGE_COMPUTE,
	HCC_SHADER_STAGE_MESH_TASK,
	HCC_SHADER_STAGE_MESH,

	HCC_SHADER_STAGE_COUNT,
};

typedef struct HccShaderInfo HccShaderInfo;
struct HccShaderInfo {
	const char*    name;
	HccShaderStage stage;
	uint32_t       bundled_constants_size;
	uint32_t       dispatch_group_size_x;
	uint32_t       dispatch_group_size_y;
	uint32_t       dispatch_group_size_z;
};

typedef uint8_t HccResourceAccessMode;
enum {
	HCC_RESOURCE_ACCESS_MODE_READ_ONLY,
	HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY,
	HCC_RESOURCE_ACCESS_MODE_READ_WRITE,
	HCC_RESOURCE_ACCESS_MODE_SAMPLE,

	HCC_RESOURCE_ACCESS_MODE_COUNT,
};

typedef uint8_t HccResourceType;
enum HccResourceType {
	HCC_RESOURCE_TYPE_BUFFER,
	HCC_RESOURCE_TYPE_TEXTURE_1D,
	HCC_RESOURCE_TYPE_TEXTURE_1D_ARRAY,
	HCC_RESOURCE_TYPE_TEXTURE_2D,
	HCC_RESOURCE_TYPE_TEXTURE_2D_MS,
	HCC_RESOURCE_TYPE_TEXTURE_2D_ARRAY,
	HCC_RESOURCE_TYPE_TEXTURE_2D_MS_ARRAY,
	HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE,
	HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE_ARRAY,
	HCC_RESOURCE_TYPE_TEXTURE_3D,
	HCC_RESOURCE_TYPE_SAMPLER,

	HCC_RESOURCE_TYPE_COUNT,
};

typedef struct HccResourceInfo HccResourceInfo;
struct HccResourceInfo {
	const char*           name;
	uint32_t              offset;
	HccResourceAccessMode access_mode;
	HccResourceType       type;
};

typedef struct HccResourceStructInfo HccResourceStructInfo;
struct HccResourceStructInfo {
	const char*      name;
	HccResourceInfo* resources;
	uint32_t         resources_count;
	uint32_t         size;
	uint32_t         align;
};

typedef struct HccMetadata HccMetadata;
struct HccMetadata {
	HccShaderInfo*         shaders;
	HccResourceStructInfo* resource_structs;
	uint32_t               shaders_count;
	uint32_t               resource_structs_count;
	uint32_t               bundled_constants_size_max;
	uint32_t               resource_descriptors_max; // from hcc --max-descriptors argument
};

extern const char* hcc_resource_access_mode_strings[HCC_RESOURCE_ACCESS_MODE_COUNT];
extern const char* hcc_shader_stage_strings[HCC_SHADER_STAGE_COUNT];
extern const char* hcc_resource_type_strings[HCC_RESOURCE_TYPE_COUNT];

void hcc_print_hprintf_buffer(const uint32_t* print_buffer);

#endif

