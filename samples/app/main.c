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

	int width, height;
	dm_screen_dims(&width, &height);
	width /= 2;
	height /= 2;

	DmWindow window = dm_window_open(width, height);

	gpu_init(window, width, height);

	GpuResourceId triangle_vertex_buffer_id = gpu_create_buffer(3 * sizeof(TriangleVertex));

	uint8_t bc_data[64];
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

				TriangleVertex* vertices = (TriangleVertex*)gpu_map_buffer(triangle_vertex_buffer_id);
				vertices[0].pos = f32x2(-0.5f, -0.5f);
				vertices[1].pos = f32x2(0.f, 0.5f);
				vertices[2].pos = f32x2(0.5f, -0.5f);
				break;
			};
			case APP_SAMPLE_ALT_2_5_D_RGB_COLOR_PICKER: {
				ColorPickerBC* bc = bundled_constants_ptr;
				if (init_sample) {
					time_ = 0.f;
				}
				bc->time_ = time_;
				break;
			};
			case APP_SAMPLE_BLOB_VACATION: {
				BlobBC* bc = bundled_constants_ptr;
				if (init_sample) {
					time_ = 0.f;
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

