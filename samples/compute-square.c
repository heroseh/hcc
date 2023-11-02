
#include <stdbool.h>
#include <stdint.h>
#include <hmaths_types.h>
#include <hcc_shader.h>

typedef struct ComputeSquareBC ComputeSquareBC;
struct ComputeSquareBC {
	HccRwTexture2D(uint32_t) output;
};

#ifdef __HCC__
#include <hmaths.h>

HCC_COMPUTE(8, 8, 1)
void compute_square_cs(HccComputeSV const* const sv, ComputeSquareBC const* const bc) {
	u32x2 coord = u32x2(sv->dispatch_idx.x, sv->dispatch_idx.y);
	f32x2 rg = f32x2((float)sv->dispatch_idx.x / 511.f, (float)sv->dispatch_idx.y / 511.f);
	f32x4 color = f32x4(rg.x, rg.y, 0.f, 1.f);
	color.bgra = color;
	store_textureG(bc->output, coord, pack_u8x4_f32x4(color));
}

#endif // __HCC__
