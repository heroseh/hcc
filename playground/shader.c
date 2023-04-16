
#include <stdbool.h>
#include <stdint.h>
#include <hmaths_types.h>
#include <hcc_shader.h>
#include <hmaths.h>

#include "app/shared.h"

typedef struct RasterizerState RasterizerState;
HCC_RASTERIZER_STATE struct RasterizerState {
	HCC_INTERP f32x2 uv;
};

HCC_VERTEX void vertex(
	HccVertexSV const* const sv,
	HccVertexSVOut* const sv_out,
	ShaderBC const* const bc,
	RasterizerState* const state_out
) {
	f32x2 unorm = f32x2(sv->vertex_idx & 1, sv->vertex_idx / 2);
	f32x2 snorm = subsG(mulsG(unorm, 2.f), 1.f);

	sv_out->position = f32x4(snorm.x, snorm.y, 0.f, 1.f);
	state_out->uv = unorm;
}

HCC_PIXEL void pixel(
	HccPixelSV const* const sv,
	HccPixelSVOut* const sv_out,
	ShaderBC const* const bc,
	RasterizerState const* const state,
	Pixel* const pixel_out
) {
	pixel_out->color = f32x4(state->uv.x, state->uv.y, 0.f, 1.f);
}

