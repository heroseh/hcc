
#include <stdbool.h>
#include <stdint.h>
#include <hmaths_types.h>
#include <hcc_shader.h>

typedef struct TriangleVertex TriangleVertex;
struct TriangleVertex {
	f32x2 pos;
};

typedef struct TriangleBC TriangleBC;
struct TriangleBC {
	f32x4 tint;
	HccRoBuffer(TriangleVertex) vertices;
};

#ifdef __HCC__
#include <hmaths.h>

typedef struct TriangleRasterizerState TriangleRasterizerState;
HCC_RASTERIZER_STATE struct TriangleRasterizerState {
	HCC_INTERP f32x4 color;
};

HCC_VERTEX void triangle_vs(
	HccVertexSV const* const sv,
	HccVertexSVOut* const sv_out,
	TriangleBC const* const bc,
	TriangleRasterizerState* const state_out
) {
	HccRoBuffer(TriangleVertex) vertices = bc->vertices;

	TriangleVertex vertex = bc->vertices[sv->vertex_idx];

	f32x4 colors[3] = {
		f32x4(1.f, 0.f, 0.f, 1.f),
		f32x4(0.f, 1.f, 0.f, 1.f),
		f32x4(0.f, 0.f, 1.f, 1.f),
	};

	sv_out->position = f32x4(vertex.pos.x, vertices[sv->vertex_idx].pos.y, 0.f, 1.f);
	state_out->color = mulG(colors[sv->vertex_idx], bc->tint);
}

typedef struct TrianglePixel TrianglePixel;
HCC_PIXEL_STATE struct TrianglePixel {
	f32x4 color;
};

HCC_PIXEL void triangle_ps(
	HccPixelSV const* const sv,
	HccPixelSVOut* const sv_out,
	TriangleBC const* const bc,
	TriangleRasterizerState const* const state,
	TrianglePixel* const pixel_out
) {
	pixel_out->color = state->color;
}

#endif // __HCC__
