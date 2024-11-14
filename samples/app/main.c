#include "app.h"
#include "dm_backend_api.h"
#include "gpu_backend_api.h"
#include "platform_backend_api.h"

#include <hmaths.c>
#include <hcc_interop.c>

#ifdef __linux__
#include "platform_linux.c"
#include "dm_x11.c"
#include "gpu_vulkan.c"
#elif defined(_WIN32)
#include <windows.h>
#include "platform_win32.c"
#include "dm_win32.c"
#include "gpu_vulkan.c"
#else
#error "unsupported platform
#endif

int main(int argc, char** argv) {
	APP_UNUSED(argc);
	APP_UNUSED(argv);

	platform_register_segfault_handler();
	dm_init();

	int window_width, window_height;
	dm_screen_dims(&window_width, &window_height);
	window_width /= 2;
	window_height /= 2;

	DmWindow window = dm_window_open(window_width, window_height);
	float ar = (float)window_height / (float)window_width;

	gpu_init(window, window_width, window_height);

	GpuResourceId backbuffer_texture_id = gpu_backbuffer();
	GpuResourceId staging_buffer = gpu_create_staging_buffer();
	GpuResourceId triangle_vertex_buffer_id = gpu_create_buffer(3 * sizeof(TriangleVertex));
	GpuResourceId logo_texture_id = gpu_create_texture(GPU_TEXTURE_TYPE_2D, GPU_FORMAT_8_8_8_8_UNORM, APP_LOGO_WIDTH, APP_LOGO_HEIGHT, 1, 1, APP_LOGO_MIP_LEVELS);
	GpuResourceId logo_voxel_texture_id = gpu_create_texture(GPU_TEXTURE_TYPE_3D, GPU_FORMAT_8_8_8_8_UNORM, APP_LOGO_VOXEL_WIDTH, APP_LOGO_VOXEL_HEIGHT, APP_LOGO_VOXEL_DEPTH, 1, 1);
	GpuResourceId gray_texture_id = gpu_create_texture(GPU_TEXTURE_TYPE_2D, GPU_FORMAT_32_FLOAT, APP_LOGO_WIDTH, APP_LOGO_HEIGHT, 1, 1, 1);
	GpuResourceId red_green_texture_id = gpu_create_texture(GPU_TEXTURE_TYPE_2D, GPU_FORMAT_16_16_UINT, APP_LOGO_WIDTH, APP_LOGO_HEIGHT, 1, 1, 1);
#if APP_ALL_SAMPLES
	GpuResourceId shapes_buffer_id = gpu_create_buffer(1 * sizeof(SDFShape));
	GpuResourceId voxel_model_buffer_id = gpu_create_buffer(1 * sizeof(VoxelModel));
#endif
	GpuResourceId hprintf_buffer_id0 = gpu_create_buffer(1024 * sizeof(uint32_t));
	GpuResourceId hprintf_buffer_id1 = gpu_create_buffer(1024 * sizeof(uint32_t));
	GpuResourceId clamp_linear_sampler_id = gpu_create_sampler();

	uint32_t* hprintf_buffer0 = gpu_map_resource(hprintf_buffer_id0);
	uint32_t* hprintf_buffer1 = gpu_map_resource(hprintf_buffer_id1);
	hprintf_buffer0[HPRINTF_BUFFER_CURSOR_IDX] = HPRINTF_BUFFER_CURSOR_START_IDX; // here is where the cursor is stored that the gpu will atomically increment
	hprintf_buffer0[HPRINTF_BUFFER_CAPACITY_IDX] = 1024; // store the maximum of words for the print buffer here for the shaders
	hprintf_buffer1[HPRINTF_BUFFER_CURSOR_IDX] = HPRINTF_BUFFER_CURSOR_START_IDX; // here is where the cursor is stored that the gpu will atomically increment
	hprintf_buffer1[HPRINTF_BUFFER_CAPACITY_IDX] = 1024; // store the maximum of words for the print buffer here for the shaders

#if APP_ALL_SAMPLES
	//
	// put logo and all of it's mip levels into the logo texture
	{
		stbi_set_flip_vertically_on_load(true);
		uint32_t logo_width = APP_LOGO_WIDTH;
		uint32_t logo_height = APP_LOGO_HEIGHT;
		for (int mip = 0; mip < APP_LOGO_MIP_LEVELS; mip += 1) {
			char path[512];
			snprintf(path, sizeof(path), APP_LOGO_PATH_FMT, logo_width);

			int x, y, comp;
			uint8_t* src_mip_pixels = stbi_load(path, &x, &y, &comp, 4);
			APP_ASSERT(src_mip_pixels, "failed to load logo at %s", path);

			uint8_t* dst_mip_pixels = gpu_stage_upload(logo_texture_id, x, y, 1, 4, mip);

			uint32_t row_size = logo_width * 4;
			uint32_t dst_offset = 0;
			uint32_t src_offset = 0;
			for (uint32_t y = 0; y < logo_height; y += 1) {
				memcpy(&dst_mip_pixels[dst_offset], &src_mip_pixels[src_offset], row_size);
				dst_offset += row_size;
				src_offset += row_size;
			}

			logo_width /= 2;
			logo_height /= 2;
		}
	}

	//
	// load the 256 logo as a 3d voxel texture
	{
		stbi_set_flip_vertically_on_load(false);
		int x, y, comp;
		uint8_t* src_pixels = stbi_load(APP_LOGO_VOXEL_PATH, &x, &y, &comp, 4);
		APP_ASSERT(src_pixels, "failed to load logo voxel data at %s", APP_LOGO_VOXEL_PATH);
		uint8_t* dst_pixels = gpu_stage_upload(logo_voxel_texture_id, x, y, APP_LOGO_VOXEL_DEPTH, 4, 0);
		uint32_t row_size = APP_LOGO_VOXEL_WIDTH * 4;
		uint32_t dst_offset = 0;
		uint32_t src_offset = 0;
		for (uint32_t z = 0; z < APP_LOGO_VOXEL_DEPTH; z += 1) {
			src_offset = 0;
			for (uint32_t y = 0; y < APP_LOGO_VOXEL_HEIGHT; y += 1) {
				memcpy(&dst_pixels[dst_offset], &src_pixels[src_offset], row_size);
				dst_offset += row_size;
				src_offset += row_size;
			}
		}
	}

	{
		float* dst_pixels = gpu_stage_upload(gray_texture_id, APP_LOGO_WIDTH, APP_LOGO_HEIGHT, 1, sizeof(float), 0);
		for (uint32_t y = 0; y < APP_LOGO_HEIGHT; y += 1) {
			for (uint32_t x = 0; x < APP_LOGO_WIDTH; x += 1) {
				dst_pixels[y * APP_LOGO_WIDTH + x] = 0.5f;
			}
		}
	}

	{
		u16x2* dst_pixels = gpu_stage_upload(red_green_texture_id, APP_LOGO_WIDTH, APP_LOGO_HEIGHT, 1, sizeof(float), 0);
		for (uint32_t y = 0; y < APP_LOGO_HEIGHT; y += 1) {
			for (uint32_t x = 0; x < APP_LOGO_WIDTH; x += 1) {
				dst_pixels[y * APP_LOGO_WIDTH + x].x = (uint16_t)(x * ((float)65536 / APP_LOGO_WIDTH));
				dst_pixels[y * APP_LOGO_WIDTH + x].y = (uint16_t)(y * ((float)65536 / APP_LOGO_HEIGHT));
			}
		}
	}

	{ // CPU side code setting up buffer data
		VoxelModel* models = gpu_map_resource(voxel_model_buffer_id);
		models[0].position = f32x3(0.f, 0.f, 1024.f);
		models[0].half_size = f32x3(APP_LOGO_VOXEL_WIDTH / 2, APP_LOGO_VOXEL_HEIGHT / 2, APP_LOGO_VOXEL_DEPTH / 2);
		models[0].color = logo_voxel_texture_id;
	}

	{ // CPU side code setting up buffer data
		SDFShape* shape = gpu_map_resource(shapes_buffer_id);
		shape->texture = logo_texture_id;
		shape->type = SDF_SHAPE_TYPE_BOX;
		shape->radius = 1;
		shape->width = 3;
		shape->height = 2;
		shape->color = 0xff227788;
	}
#endif

	gpu_stage_uploads_flush();

	uint8_t bc_data[64];
	memset(bc_data, 0x00, sizeof(bc_data)); // we zero the memory here to a avoid a driver crash if we forget to set a bindless index at all!
	void* bundled_constants_ptr = bc_data;

	AppSampleEnum next_sample_enum_to_init = 0;
	AppSampleEnum sample_enum = 0;
	float time_ = 0.f;
	bool init_sample = true;
	while (1) {
		uint32_t active_frame_idx = gpu.frame_idx % APP_FRAMES_IN_FLIGHT;

		if (next_sample_enum_to_init < APP_SAMPLE_COUNT) {
			gpu_init_sample(next_sample_enum_to_init);
			next_sample_enum_to_init += 1;
		}

		DmEvent event;
		while (dm_process_events(&event)) {
			switch (event.type) {
				case DM_EVENT_TYPE_WINDOW_CLOSED:
					exit(0);
				case DM_EVENT_TYPE_WINDOW_RESIZED:
					window_width = event.window_width;
					window_height = event.window_height;
					break;
				case DM_EVENT_TYPE_KEY_PRESSED:
					switch (event.key) {
						case '[':
							if (sample_enum == 0) {
								sample_enum = APP_SAMPLE_COUNT;
							}
							sample_enum -= 1;
							init_sample = true;
							break;
						case ']':
							sample_enum += 1;
							if (sample_enum == APP_SAMPLE_COUNT) {
								sample_enum = 0;
							}
							init_sample = true;
							break;
					}

					printf("key pressed %c\n", event.key);
					break;
				case DM_EVENT_TYPE_KEY_RELEASED:
					printf("key released %c\n", event.key);
					break;
			}
		}

		if (init_sample) {
			time_ = 0.f;
		}

		switch (sample_enum) {
			case APP_SAMPLE_TRIANGLE: {
				TriangleBC* bc = bundled_constants_ptr;
				if (init_sample) {
					bc->vertices = triangle_vertex_buffer_id;
					bc->tint = f32x4(1.f, 1.f, 1.f, 1.f);
				} else {
					switch (rand() % 6) {
						case 0: bc->tint.x -= 0.05f; break;
						case 1: bc->tint.y -= 0.05f; break;
						case 2: bc->tint.z -= 0.05f; break;
						case 3: bc->tint.x += 0.05f; break;
						case 4: bc->tint.y += 0.05f; break;
						case 5: bc->tint.z += 0.05f; break;
					}
				}

				TriangleVertex* vertices = (TriangleVertex*)gpu_map_resource(triangle_vertex_buffer_id);
				vertices[0].pos = f32x2(-0.5f, -0.5f);
				vertices[1].pos = f32x2(0.f, 0.5f);
				vertices[2].pos = f32x2(0.5f, -0.5f);
				bc->hprintf_buffer = active_frame_idx ? hprintf_buffer_id0 : hprintf_buffer_id1;
				break;
			};
#if APP_ALL_SAMPLES
			case APP_SAMPLE_COMPUTE_SQUARE: {
				ComputeSquareBC* bc = bundled_constants_ptr;
				if (init_sample) {
					bc->output = backbuffer_texture_id;
				}
				break;
			};
			case APP_SAMPLE_TEXTURE: {
				TextureBC* bc = bundled_constants_ptr;
				if (init_sample) {
					bc->gray_texture = gray_texture_id;
					bc->red_green_texture = red_green_texture_id;
					bc->logo_texture = logo_texture_id;
					bc->sample_logo_texture = logo_texture_id;
					bc->sampler = clamp_linear_sampler_id;
				}

				bc->hprintf_buffer = active_frame_idx ? hprintf_buffer_id0 : hprintf_buffer_id1;
				bc->offset.x = sinf(time_ * 4.12f) * 0.1f;
				bc->offset.y = sinf(time_ * 3.33f) * 0.1f;
				bc->scale.x = (cosf(time_ * 2.f) * 0.5f + 0.5f) * 0.25f + 1.5f;
				bc->scale.y = bc->scale.x * ar;
				bc->time_ = time_;
				bc->ar = ar;
				break;
			};
			case APP_SAMPLE_ALT_2_5_D_RGB_COLOR_PICKER: {
				ColorPickerBC* bc = bundled_constants_ptr;
				if (init_sample) {
				}
				bc->time_ = time_;
				bc->screen_width = window_width;
				bc->screen_height = window_height;
				break;
			};
			case APP_SAMPLE_BLOB_VACATION: {
				BlobVacationBC* bc = bundled_constants_ptr;
				if (init_sample) {
				}
				bc->time_ = time_;
				bc->screen_width = window_width;
				bc->screen_height = window_height;
				break;
			};
			case APP_SAMPLE_VOXEL_RAYTRACER: {
				VoxelRaytracerBC* bc = bundled_constants_ptr;
				if (init_sample) {
					bc->models = voxel_model_buffer_id;
					bc->output = backbuffer_texture_id;
				}
				app_samples[sample_enum].compute.dispatch_group_size_x = window_width / 8;
				app_samples[sample_enum].compute.dispatch_group_size_y = window_height / 8;
				bc->time_ = time_;
				bc->screen_width = window_width;
				bc->screen_height = window_height;
				bc->hprintf_buffer = active_frame_idx ? hprintf_buffer_id0 : hprintf_buffer_id1;
				break;
			};
			case APP_SAMPLE_SDF_2D: {
				SDF2dBC* bc = bundled_constants_ptr;
				bc->shapes = shapes_buffer_id;
				bc->sampler = clamp_linear_sampler_id;
				bc->time_ = time_;
				bc->ar = ar;
				break;
			};
#endif
		}

		gpu_render_frame(sample_enum, bc_data, window_width, window_height);

		uint32_t* hprintf_buffer = active_frame_idx ? hprintf_buffer1 : hprintf_buffer0;
		gpu_invalidate_caches(active_frame_idx ? hprintf_buffer_id1 : hprintf_buffer_id0);
		printf("hprintf_buffer contains %u words with a capacity of %u\n", hprintf_buffer[0], hprintf_buffer[1]);
		hcc_print_hprintf_buffer(hprintf_buffer);

		// reset for next time it is used
		hprintf_buffer[HPRINTF_BUFFER_CURSOR_IDX] = HPRINTF_BUFFER_CURSOR_START_IDX; // here is where the cursor is stored that the gpu will atomically increment

		init_sample = false;
		time_ += 0.01f;
	}

	return 0;
}

