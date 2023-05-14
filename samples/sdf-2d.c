
#include <stdbool.h>
#include <stdint.h>
#include <hmaths_types.h>
#include <hcc_shader.h>

typedef enum SDFShapeType {
	SDF_SHAPE_TYPE_CIRCLE,
	SDF_SHAPE_TYPE_BOX,
} SDFShapeType;

typedef struct SDFShape SDFShape;
struct SDFShape {
	HccSampleTexture2D(f32x4) texture;
	SDFShapeType              type  : 9;
	uint32_t                  radius: 9;
	uint32_t                  width : 15;
	uint32_t                  height: 9;
	uint32_t                  color;
};

typedef struct SDF2dBC SDF2dBC;
struct SDF2dBC {
	HccRoBuffer(SDFShape) shapes;
	HccRoSampler          sampler;
	float                 time_;
	float                 ar;
};

#ifdef __HCC__
#include <hmaths.h>

const float RADIUS_MULTIPLIER = 0.1f;
const float WIDTH_HEIGHT_MULTIPLIER = 0.1f;

static const SDFShape g_sdf_shape = {
	.type = SDF_SHAPE_TYPE_CIRCLE,
	.radius = 3,
	.color = 0xff7722cc,
};

float sdf_circle(f32x2 p, float r) {
	return lenG(p) - r;
}

float sdf_box(f32x2 p, f32x2 b, float r) {
	f32x2 q = addsG(subG(absG(p), b), r);
	return minG(maxG(q.x,q.y),0.f) + lenG(maxsG(q,0.f)) - r;
}

typedef struct SDF2dRasterizerState SDF2dRasterizerState;
HCC_RASTERIZER_STATE struct SDF2dRasterizerState {
	HCC_INTERP f32x2 uv;
};

HCC_VERTEX void sdf2d_vs(
	HccVertexSV const* const sv,
	HccVertexSVOut* const sv_out,
	SDF2dBC const* const bc,
	SDF2dRasterizerState* const state_out
) {
	sv_out->position = f32x4((sv->vertex_idx & 1) * 2.f - 1.f, (sv->vertex_idx / 2) * 2.f - 1.f, 0.f, 1.f);
	state_out->uv = f32x2(sv->vertex_idx & 1, sv->vertex_idx / 2);
}

typedef struct SDF2dPixel SDF2dPixel;
HCC_PIXEL_STATE struct SDF2dPixel {
	f32x4 color;
};

HCC_PIXEL void sdf2d_ps(
	HccPixelSV const* const sv,
	HccPixelSVOut* const sv_out,
	SDF2dBC const* const bc,
	SDF2dRasterizerState const* const state,
	SDF2dPixel* const pixel_out
) {
	SDFShape shape;
	uint32_t shape_idx = floorG(bc->time_) % 4;
	if (shape_idx == 0) {
		shape = bc->shapes[shape_idx];
	} else if (shape_idx == 1) {
		shape = g_sdf_shape;
	} else if (shape_idx == 2) {
		shape = (SDFShape) {
			.type = SDF_SHAPE_TYPE_BOX,
			.radius = shape_idx == 2,
			.width = 3,
			.height = 3,
			.color = 0xff339911,
		};
	} else if (shape_idx == 3) {
		shape = (SDFShape){0};
		shape.type = SDF_SHAPE_TYPE_BOX;
		shape.radius = 1;
		shape.width = 4;
		shape.height = 4;
		shape.height += (1 << 9); // overflow height so it ends up on the same number
		shape.color = 0xff993311;
	}

	f32x4 color = f32x4s(1.f);
	if ((uint32_t)shape.texture) {
		color = sample_textureG(shape.texture, bc->sampler, state->uv);
	}
	pixel_out->color = mulG(color, unpack_u8x4_f32x4(shape.color));

	f32x2 shape_pos = f32x2s(0.5f);
	switch (shape.type) {
		case SDF_SHAPE_TYPE_CIRCLE:
			if (sdf_circle(subG(state->uv, shape_pos), shape.radius * RADIUS_MULTIPLIER) > 0.f) {
				pixel_out->color = f32x4(0.f, 0.f, 0.f, 1.f);
			}
			break;

		case SDF_SHAPE_TYPE_BOX:
			if (sdf_box(subG(state->uv, shape_pos), f32x2(shape.width * WIDTH_HEIGHT_MULTIPLIER, shape.height * WIDTH_HEIGHT_MULTIPLIER), shape.radius * RADIUS_MULTIPLIER) > 0.f) {
				pixel_out->color = f32x4(0.f, 0.f, 0.f, 1.f);
			}
			break;
	}

	uint32_t font_1bit_8x8[20] = {
		0x3E63737B, 0x6F673E00, // (0)
		0x0C0E0C0C, 0x0C0C3F00, // (1)
		0x1E33301C, 0x06333F00, // (2)
		0x1E33301C, 0x30331E00, // (3)
		0x383C3633, 0x7F307800, // (4)
		0x3F031F30, 0x30331E00, // (5)
		0x1C06031F, 0x33331E00, // (6)
		0x3F333018, 0x0C0C0C00, // (7)
		0x1E33331E, 0x33331E00, // (8)
		0x1E33333E, 0x30180E00, // (9)
	};

	f32x2 num_pos = f32x2(0.8f, 0.1f);
	f32x2 num_size = f32x2(0.1f, 0.1f / bc->ar);
	if (
		num_pos.x < state->uv.x && state->uv.x < num_pos.x + num_size.x &&
		num_pos.y < state->uv.y && state->uv.y < num_pos.y + num_size.y
	) {
		f32x2 uv = remapG(state->uv, num_pos, addG(num_pos, num_size), f32x2s(0.f), f32x2s(1.f));
		uv.x *= 8;
		uv.y *= 8;
		uint32_t x = (uint32_t)uv.x;
		uint32_t y = (uint32_t)uv.y;
		uint32_t pixel_idx = y * 8 + x;
		uint32_t num = shape_idx;
		bool pixel = (bool)(font_1bit_8x8[num * 2 + (pixel_idx < 32)] & (1 << (pixel_idx % 32)));
		if (pixel) {
			pixel_out->color = f32x4(0.7f, 0.9f, 0.3f, 1.f);
		}
	}
}

#endif // __HCC__
