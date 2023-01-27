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

	AppSampleEnum next_sample_enum_to_init = 0;
	AppSampleEnum sample_enum = 0;
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
							break;
						case ']':
							sample_enum += 1;
							if (sample_enum == APP_SAMPLE_COUNT) {
								sample_enum = 0;
							}
							break;
					}

					printf("key pressed %c\n", event.key);
					break;
				case DM_EVENT_TYPE_KEY_RELEASED:
					printf("key released %c\n", event.key);
					break;
			}
		}

		gpu_render_frame(sample_enum);
	}

	return 0;
}

