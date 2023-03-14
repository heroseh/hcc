
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
	sv_out->position = f32x4((sv->vertex_idx & 1) * 2.f - 1.f, (sv->vertex_idx / 2) * 2.f - 1.f, 0.f, 1.f);
	state_out->uv = f32x2((sv->vertex_idx & 1) * 2.f - 1.f, (sv->vertex_idx / 2) * 2.f - 1.f);
}

HCC_FRAGMENT void fragment(
	HccFragmentSV const* const sv,
	HccFragmentSVOut* const sv_out,
	ShaderBC const* const bc,
	RasterizerState const* const state,
	Fragment* const frag_out
) {
	frag_out->color = f32x4(0.0f, 0.7f, 0.3f, 1.f);
}

