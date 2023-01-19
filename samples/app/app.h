#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define APP_NAME "Hcc Samples"

// these name can be what ever you want, we just want to make it consistant and easy for this sample codebase
#define APP_SHADER_ENTRY_POINT_VERTEX "vs"
#define APP_SHADER_ENTRY_POINT_FRAGMENT "fs"
#define APP_SHADER_ENTRY_POINT_COMPUTE "cs"

#define APP_ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define APP_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
#define APP_UNUSED(expr) (void)(expr)

#define APP_FRAMES_IN_FLIGHT 2

#define APP_CLAMP(v, min, max) (((v) > (max)) ? (max) : ((v) < (min)) ? (min) : (v))
#define for_range(idx, start, end) for (unsigned int idx = start; idx < end; idx += 1)

typedef int AppSampleEnum;
enum AppSampleEnum {
	APP_SAMPLE_TRIANGLE,

	APP_SAMPLE_COUNT,
};

typedef int AppShaderType;
enum AppShaderType {
	APP_SHADER_TYPE_GRAPHICS,
	APP_SHADER_TYPE_COMPUTE,

	APP_SHADER_TYPE_COUNT,
};

typedef struct AppSample AppSample;
struct AppSample {
	const char*   shader_name;
	AppShaderType shader_type;

	struct {
		uint32_t vertices_count;
	} graphics;
	struct {
		uint32_t dispatch_size_x;
		uint32_t dispatch_size_y;
		uint32_t dispatch_size_z;
	} compute;
};

static AppSample app_samples[APP_SAMPLE_COUNT] = {
	[APP_SAMPLE_TRIANGLE] = {
		.shader_name = "triangle",
		.graphics.vertices_count = 3,
	},
};

