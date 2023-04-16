#pragma once

#include <hmaths_types.h>
#include <hcc_shader.h>

typedef struct ShaderBC ShaderBC;
struct ShaderBC {
	float    time_;
	uint32_t screen_width;
	uint32_t screen_height;
};

typedef struct Pixel Pixel;
HCC_PIXEL_STATE struct Pixel {
	f32x4 color;
};

