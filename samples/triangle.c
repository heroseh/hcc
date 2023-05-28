
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
	HccRwByteBuffer hprintf_buffer;
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

typedef struct Test Test;
struct Test {
	u32x4 vec;
};

HCC_PIXEL void triangle_ps(
	HccPixelSV const* const sv,
	HccPixelSVOut* const sv_out,
	TriangleBC const* const bc,
	TriangleRasterizerState const* const state,
	TrianglePixel* const pixel_out
) {
	u32x4 vec = u32x4(1, 2, 3, 4);
	u32x3 xyz = vec.xyz;
	u32x2 xy = vec.xy;
	uint32_t x = vec.x;

	bool success = true;
	success &= xyz.x == 1 && xyz.y == 2 && xyz.z == 3 && xy.x == 1 && xy.y == 2 && x == 1;

	u32x3 zxy = vec.zxy;
	u32x2 zy = vec.zy;
	uint32_t y = vec.y;
	success &= zxy.x == 3 && zxy.y == 1 && zxy.z == 2 && zy.x == 3 && zy.y == 2 && y == 2;

	u32x3 gba = vec.gba;
	success &= gba.x == 2 && gba.y == 3 && gba.z == 4;

	u32x2 xx = vec.xx;
	u32x3 yyy = vec.yyy;
	success &= xx.x == 1 && xx.y == 1 && yyy.x == 2 && yyy.y == 2 && yyy.z == 2;

	u32x2 xxyy = vec.xx.yy;
	u32x3 wat = vec.bga.xzy;
	success &= xxyy.x == 1 && xxyy.y == 1 && wat.x == 3 && wat.y == 4 && wat.z == 2;

	vec.zy = u32x2(5, 6);
	success &= vec.x == 1 && vec.y == 6 && vec.z == 5 && vec.w == 4;

	vec.wxy.yxz = u32x3(7, 8, 9);
	success &= vec.x == 7 && vec.y == 9 && vec.z == 5 && vec.w == 8;

	Test test = {
		.vec.zy.yx = u32x2(1, 2),
		.vec.x = 3,
	};

	success &= test.vec.x == 3 && test.vec.y == 1 && test.vec.z == 2 && test.vec.w == 0;

	hprintf(bc->hprintf_buffer, "pixel_coord: %f, %f and test: %u, %u, %u, %u\n", splat2(sv->pixel_coord), splat4(test.vec));

	f32x4 color = f32x4(1.f, 0.f, 0.f, 1.f);
	color.b = success ? 1.f : 0.f;
	pixel_out->color = color;
}

#endif // __HCC__
