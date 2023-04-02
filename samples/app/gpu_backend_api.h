#pragma once

typedef uint32_t GpuResourceId;

typedef uint8_t GpuTextureType;
enum GpuTextureType {
	GPU_TEXTURE_TYPE_1D,
	GPU_TEXTURE_TYPE_1D_ARRAY,
	GPU_TEXTURE_TYPE_2D,
	GPU_TEXTURE_TYPE_2D_ARRAY,
	GPU_TEXTURE_TYPE_CUBE,
	GPU_TEXTURE_TYPE_CUBE_ARRAY,
	GPU_TEXTURE_TYPE_3D,
};

void gpu_init(DmWindow window, uint32_t window_width, uint32_t window_height);
void gpu_init_sample(AppSampleEnum sample_enum);
void gpu_render_frame(AppSampleEnum sample_enum, void* bc, uint32_t window_width, uint32_t window_height);
GpuResourceId gpu_create_backbuffer(void);
GpuResourceId gpu_create_staging_buffer(void);
GpuResourceId gpu_create_buffer(uint32_t size);
GpuResourceId gpu_create_texture(GpuTextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t array_layers, uint32_t mip_levels);
GpuResourceId gpu_create_sampler(void);
void* gpu_map_resource(GpuResourceId res_id);
uint32_t gpu_mip_offset(GpuResourceId res_id, uint32_t mip);
uint32_t gpu_row_pitch(GpuResourceId res_id, uint32_t mip);
uint32_t gpu_depth_pitch(GpuResourceId res_id, uint32_t mip);
void* gpu_stage_upload(GpuResourceId res_id, uint32_t width, uint32_t height, uint32_t depth, uint32_t bpp, uint32_t mip);
void gpu_stage_uploads_flush(void);
