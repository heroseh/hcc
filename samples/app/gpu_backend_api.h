#pragma once

typedef uint32_t GpuResourceId;

void gpu_init(DmWindow window, uint32_t window_width, uint32_t window_height);
void gpu_init_sample(AppSampleEnum sample_enum);
void gpu_render_frame(AppSampleEnum sample_enum, void* bc, uint32_t bc_size);
GpuResourceId gpu_create_buffer(uint32_t size);
void* gpu_map_buffer(GpuResourceId res_id);
