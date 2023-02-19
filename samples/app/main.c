#include "app.h"
#include "dm_backend_api.h"
#include "gpu_backend_api.h"
#include "platform_backend_api.h"

#ifdef __linux__
#include "platform_linux.c"
#include "dm_x11.c"
#include "gpu_vulkan.c"
#else
#error "unsupported platform"
#endif

int main(int argc, char** argv) {
	APP_UNUSED(argc);
	APP_UNUSED(argv);

	dm_init();

	int window_width, window_height;
	dm_screen_dims(&window_width, &window_height);
	window_width /= 2;
	window_height /= 2;

	DmWindow window = dm_window_open(window_width, window_height);
	float ar = (float)window_height / (float)window_width;

	gpu_init(window, window_width, window_height);

	GpuResourceId triangle_vertex_buffer_id = gpu_create_buffer(3 * sizeof(TriangleVertex));
	GpuResourceId logo_texture_id = gpu_create_texture(GPU_TEXTURE_TYPE_2D, APP_LOGO_WIDTH, APP_LOGO_HEIGHT, 1, 1, APP_LOGO_MIP_LEVELS);
	GpuResourceId clamp_linear_sampler_id = gpu_create_sampler();

	{
		stbi_set_flip_vertically_on_load(true);
		uint32_t logo_width = APP_LOGO_WIDTH;
		uint32_t logo_height = APP_LOGO_HEIGHT;
		uint8_t* dst_mip_pixels = gpu_map_resource(logo_texture_id);
		for (int mip = 0; mip < APP_LOGO_MIP_LEVELS; mip += 1) {
			char path[512];
			snprintf(path, sizeof(path), APP_LOGO_PATH_FMT, logo_width);

			int x, y, comp;
			uint8_t* src_mip_pixels = stbi_load(path, &x, &y, &comp, 4);
			APP_ASSERT(src_mip_pixels, "failed to load logo at %s", path);
			uint32_t row_size = logo_width * 4;
			uint32_t dst_offset = gpu_mip_offset(logo_texture_id, mip);
			uint32_t src_offset = 0;
			uint32_t row_pitch = gpu_row_pitch(logo_texture_id, mip);
			for (uint32_t y = 0; y < logo_height; y += 1) {
				memcpy(&dst_mip_pixels[dst_offset], &src_mip_pixels[src_offset], row_size);
				dst_offset += row_pitch;
				src_offset += row_size;
			}

			logo_width /= 2;
			logo_height /= 2;
		}
	}

	uint8_t bc_data[64];
	memset(bc_data, 0x00, sizeof(bc_data));
	void* bundled_constants_ptr = bc_data;

	AppSampleEnum next_sample_enum_to_init = 0;
	AppSampleEnum sample_enum = 0;
	float time_ = 0.f;
	bool init_sample = true;
	while (1) {
		if (next_sample_enum_to_init < APP_SAMPLE_COUNT) {
			gpu_init_sample(next_sample_enum_to_init);
			next_sample_enum_to_init += 1;
		}

		DmEvent event;
		while (dm_process_events(&event)) {
			switch (event.type) {
				case DM_EVENT_TYPE_WINDOW_CLOSED:
					exit(0);
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
				break;
			};
			case APP_SAMPLE_TEXTURE: {
				TextureBC* bc = bundled_constants_ptr;
				if (init_sample) {
					bc->texture = logo_texture_id;
					bc->sample_texture = logo_texture_id;
					bc->sampler = clamp_linear_sampler_id;
				}

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
				break;
			};
			case APP_SAMPLE_BLOB_VACATION: {
				BlobBC* bc = bundled_constants_ptr;
				if (init_sample) {
				}
				bc->time_ = time_;
				break;
			};
		}

		gpu_render_frame(sample_enum, bc_data, sizeof(bc_data));
		init_sample = false;
		time_ += 0.01f;
	}

	return 0;
}

