#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define B_STACKTRACE_IMPL
#include "b_stacktrace.h"

#include "../shader-metadata.h"

#include "shared.h"

#define APP_NAME "Hcc Playground"

#define APP_ABORT(...) APP_ASSERT(false, __VA_ARGS__)
#define APP_ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define APP_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
#define APP_UNUSED(expr) (void)(expr)

#define APP_FRAMES_IN_FLIGHT 2

#define APP_SHADERS_PATH "shader.spirv"

#define APP_CLAMP(v, min, max) (((v) > (max)) ? (max) : ((v) < (min)) ? (min) : (v))
#define for_range(idx, start, end) for (unsigned int idx = start; idx < end; idx += 1)

