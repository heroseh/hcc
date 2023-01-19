#include <libc-gpu/stdbool.h>
#include <libc-gpu/stdint.h>
#include <libhccstd/core.h>
#include <libhccstd/math.h>

HCC_DEFINE_RASTERIZER_STATE(
	RasterizerState,
	(POSITION, f32x4, position),
	(INTERP,   f32x4, color)
);

vertex void vs(HccVertexSV const sv, RasterizerState* const state_out) {
	f32x2 vertices[3] = {
		f32x2(-0.5f, -0.5f),
		f32x2(0.f, 0.5f),
		f32x2(0.5f, -0.5f),
	};

	f32x4 colors[3] = {
		f32x4(1.f, 0.f, 0.f, 1.f),
		f32x4(0.f, 1.f, 0.f, 1.f),
		f32x4(0.f, 0.f, 1.f, 1.f),
	};

	state_out->position = f32x4(vertices[sv.vertex_idx].x, vertices[sv.vertex_idx].y, 0.f, 1.f);
	state_out->color = colors[sv.vertex_idx];
}

HCC_DEFINE_FRAGMENT_STATE(
	Fragment,
	(f32x4, color)
);

fragment void fs(HccFragmentSV const sv, RasterizerState const* const state, Fragment* const frag_out) {
	frag_out->color = state->color;
}

