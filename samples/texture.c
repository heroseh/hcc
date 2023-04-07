
#include <stdbool.h>
#include <stdint.h>
#include <hmaths_types.h>
#include <hcc_shader.h>

typedef struct TextureVertex TextureVertex;
struct TextureVertex {
	f32x2 pos;
};

typedef struct TextureBC TextureBC;
struct TextureBC {
	HccRoTexture2D(f32x4)     texture;
	HccSampleTexture2D(f32x4) sample_texture;
	HccRoSampler              sampler;
	float                     time_;
	f32x2                     offset;
	f32x2                     scale;
	float                     ar;
};

#ifdef __HCC__
#include <hmaths.h>

typedef struct TextureRasterizerState TextureRasterizerState;
HCC_RASTERIZER_STATE struct TextureRasterizerState {
	HCC_INTERP f32x2 uv;
};

HCC_VERTEX void texture_vs(
	HccVertexSV const* const sv,
	HccVertexSVOut* const sv_out,
	TextureBC const* const bc,
	TextureRasterizerState* const state_out
) {
	sv_out->position = f32x4((sv->vertex_idx & 1) * 2.f - 1.f, (sv->vertex_idx / 2) * 2.f - 1.f, 0.f, 1.f);
	state_out->uv = f32x2((sv->vertex_idx & 1) * 2.f - 1.f, (sv->vertex_idx / 2) * 2.f - 1.f);
}

typedef struct TextureFragment TextureFragment;
HCC_FRAGMENT_STATE struct TextureFragment {
	f32x4 color;
};

HCC_FRAGMENT void texture_fs(
	HccFragmentSV const* const sv,
	HccFragmentSVOut* const sv_out,
	TextureBC const* const bc,
	TextureRasterizerState const* const state,
	TextureFragment* const frag_out
) {
	f32x2 uv_unorm = addsG(mulsG(state->uv, 0.5f), 0.5f);
	f32x2 uv = addsG(mulsG(addG(mulG(state->uv, bc->scale), bc->offset), 0.5f), 0.5f);
	uv = clampsG(uv, 0.f, 1.f);

	uint32_t mode = floorG(bc->time_) % 10;
	switch (mode) {
		case 0:
			frag_out->color = load_textureG(bc->texture, u32x2(uv.x * 1023, uv.y * 1023));
			break;
		case 1:
			frag_out->color = fetch_textureG(bc->sample_texture, u32x2(uv.x * 1023, uv.y * 1023));
			break;
		case 2:
			frag_out->color = sample_textureG(bc->sample_texture, bc->sampler, uv);
			break;
		case 3:
			frag_out->color = sample_mip_bias_textureG(bc->sample_texture, bc->sampler, uv, 3.f);
			break;
		case 4: {
			f32x2 ddx = f32x2(5.f / 1024.f, 0.f);
			f32x2 ddy = f32x2(0.f, 5.f / 1024.f);
			frag_out->color = sample_mip_gradient_textureG(bc->sample_texture, bc->sampler, uv, ddx, ddy);
			break;
		};
		case 5:
			frag_out->color = sample_mip_level_textureG(bc->sample_texture, bc->sampler, uv, 5.f);
			break;
		case 6:
			frag_out->color = gather_red_textureG(bc->sample_texture, bc->sampler, uv);
			break;
		case 7:
			frag_out->color = gather_green_textureG(bc->sample_texture, bc->sampler, uv);
			break;
		case 8:
			frag_out->color = gather_blue_textureG(bc->sample_texture, bc->sampler, uv);
			break;
		case 9:
			frag_out->color = gather_alpha_textureG(bc->sample_texture, bc->sampler, uv);
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
		num_pos.x < uv_unorm.x && uv_unorm.x < num_pos.x + num_size.x &&
		num_pos.y < uv_unorm.y && uv_unorm.y < num_pos.y + num_size.y
	) {
		uv = remapG(uv_unorm, num_pos, addG(num_pos, num_size), f32x2s(0.f), f32x2s(1.f));
		uv.x *= 8;
		uv.y *= 8;
		uint32_t x = (uint32_t)uv.x;
		uint32_t y = (uint32_t)uv.y;
		uint32_t pixel_idx = y * 8 + x;
		uint32_t num = mode;
		bool pixel = (bool)(font_1bit_8x8[num * 2 + (pixel_idx < 32)] & (1 << (pixel_idx % 32)));
		if (pixel) {
			frag_out->color = f32x4(0.7f, 0.9f, 0.3f, 1.f);
		}
	}
}

#endif // __HCC__
