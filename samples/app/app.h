#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//
// include the metadata produced by HCC about the shaders
#include "../shaders-metadata.h"

//
// include the samples here so we have access to their structures
#include "../shaders.c"

#define APP_NAME "Hcc Samples"

#define APP_ABORT(...) APP_ASSERT(false, __VA_ARGS__)
#define APP_ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define APP_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
#define APP_UNUSED(expr) (void)(expr)

#define APP_FRAMES_IN_FLIGHT 2

#define APP_SHADERS_PATH "samples/shaders.spirv"

#define APP_LOGO_MIP_LEVELS 6
#define APP_LOGO_WIDTH 1024
#define APP_LOGO_HEIGHT 1024
#define APP_LOGO_PATH_FMT "logo/logo-%u.png"
#define APP_INT_ROUND_UP_ALIGN(i, align) (((i) + ((align) - 1)) & ~((align) - 1))

#define APP_LOGO_VOXEL_PATH "samples/voxel-raytracer-assets/logo-256.png"
#define APP_LOGO_VOXEL_WIDTH  256
#define APP_LOGO_VOXEL_HEIGHT 256
#define APP_LOGO_VOXEL_DEPTH  32

#define APP_CLAMP(v, min, max) (((v) > (max)) ? (max) : ((v) < (min)) ? (min) : (v))
#define for_range(idx, start, end) for (unsigned int idx = start; idx < end; idx += 1)

typedef int AppSampleEnum;
enum AppSampleEnum {
	APP_SAMPLE_TRIANGLE,
	APP_SAMPLE_COMPUTE_SQUARE,
	APP_SAMPLE_TEXTURE,
	APP_SAMPLE_ALT_2_5_D_RGB_COLOR_PICKER,
	APP_SAMPLE_BLOB_VACATION,
	APP_SAMPLE_VOXEL_RAYTRACER,

	APP_SAMPLE_COUNT,
};

typedef int AppShaderType;
enum AppShaderType {
	APP_SHADER_TYPE_GRAPHICS,
	APP_SHADER_TYPE_COMPUTE,

	APP_SHADER_TYPE_COUNT,
};

typedef int AppTopology;
enum AppTopology {
	APP_TOPOLOGY_TRIANGLE_LIST,
	APP_TOPOLOGY_TRIANGLE_STRIP,
};

typedef struct AppSample AppSample;
struct AppSample {
	const char*   shader_name;
	AppShaderType shader_type;

	struct {
		HccShader   shader_vs;
		HccShader   shader_fs;
		AppTopology topology;
		uint32_t    vertices_count;
	} graphics;
	struct {
		HccShader shader_cs;
		uint32_t  dispatch_group_size_x;
		uint32_t  dispatch_group_size_y;
		uint32_t  dispatch_group_size_z;
	} compute;
};

static AppSample app_samples[APP_SAMPLE_COUNT] = {
	[APP_SAMPLE_TRIANGLE] = {
		.shader_name = "triangle",
		.shader_type = APP_SHADER_TYPE_GRAPHICS,
		.graphics = {
			.shader_vs = HCC_SHADER_triangle_vs,
			.shader_fs = HCC_SHADER_triangle_fs,
			.topology = APP_TOPOLOGY_TRIANGLE_LIST,
			.vertices_count = 3,
		},
	},
	[APP_SAMPLE_COMPUTE_SQUARE] = {
		.shader_name = "compute-square",
		.shader_type = APP_SHADER_TYPE_COMPUTE,
		.compute = {
			.shader_cs = HCC_SHADER_compute_square_cs,
			.dispatch_group_size_x = 64,
			.dispatch_group_size_y = 64,
			.dispatch_group_size_z = 1,
		},
	},
	[APP_SAMPLE_TEXTURE] = {
		.shader_name = "texture",
		.shader_type = APP_SHADER_TYPE_GRAPHICS,
		.graphics = {
			.shader_vs = HCC_SHADER_texture_vs,
			.shader_fs = HCC_SHADER_texture_fs,
			.topology = APP_TOPOLOGY_TRIANGLE_STRIP,
			.vertices_count = 4,
		},
	},
	[APP_SAMPLE_ALT_2_5_D_RGB_COLOR_PICKER] = {
		.shader_name = "alt-2.5d-rgb-color-picker",
		.shader_type = APP_SHADER_TYPE_GRAPHICS,
		.graphics = {
			.shader_vs = HCC_SHADER_color_picker_vs,
			.shader_fs = HCC_SHADER_color_picker_fs,
			.topology = APP_TOPOLOGY_TRIANGLE_STRIP,
			.vertices_count = 4,
		},
	},
	[APP_SAMPLE_BLOB_VACATION] = {
		.shader_name = "blob-vacation",
		.shader_type = APP_SHADER_TYPE_GRAPHICS,
		.graphics = {
			.shader_vs = HCC_SHADER_blob_vacation_vs,
			.shader_fs = HCC_SHADER_blob_vacation_fs,
			.topology = APP_TOPOLOGY_TRIANGLE_STRIP,
			.vertices_count = 4,
		},
	},
	[APP_SAMPLE_VOXEL_RAYTRACER] = {
		.shader_name = "voxel-raytracer",
		.shader_type = APP_SHADER_TYPE_COMPUTE,
		.compute = {
			.shader_cs = HCC_SHADER_voxel_raytracer_cs,
			.dispatch_group_size_x = 1,
			.dispatch_group_size_y = 1,
			.dispatch_group_size_z = 1,
		},
	},
};

