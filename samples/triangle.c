
#include <stdbool.h>
#include <stdint.h>
#include <hmaths/types.h>
#include <hcc_shader.h>

typedef struct TriangleVertex TriangleVertex;
struct TriangleVertex {
	f32x2 pos;
};

typedef struct TriangleBC TriangleBC;
struct TriangleBC {
	HccRoBuffer(TriangleVertex) vertices;
	f32x4 tint;
};

#ifdef __HCC__
#include <hmaths/maths.h>

typedef struct RasterizerState RasterizerState;
HCC_RASTERIZER_STATE struct RasterizerState {
	HCC_INTERP f32x4 color;
};

HCC_VERTEX void vs(HccVertexSV const* const sv, HccVertexSVOut* const sv_out, TriangleBC const* const bc, RasterizerState* const state_out) {
	HccRoBuffer(TriangleVertex) vertices = bc->vertices;

	TriangleVertex vertex = bc->vertices[sv->vertex_idx];

	f32x4 colors[3] = {
		f32x4(1.f, 0.f, 0.f, 1.f),
		f32x4(0.f, 1.f, 0.f, 1.f),
		f32x4(0.f, 0.f, 1.f, 1.f),
	};

	sv_out->position = f32x4(vertex.pos.x, vertices[sv->vertex_idx].pos.y, 0.f, 1.f);
	state_out->color = mul_f32x4(colors[sv->vertex_idx], bc->tint);
}

typedef struct Fragment Fragment;
HCC_FRAGMENT_STATE struct Fragment {
	f32x4 color;
};

HCC_FRAGMENT void fs(HccFragmentSV const* const sv, HccFragmentSVOut* const sv_out, TriangleBC const* const bc, RasterizerState const* const state, Fragment* const frag_out) {
	frag_out->color = state->color;
}

#endif // __HCC__
