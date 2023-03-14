#include "app.h"
#include "dm_backend_api.h"
#include "gpu_backend_api.h"
#include "platform_backend_api.h"
#include <time.h>

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

bool recompile_shader(void) {
	printf("\033c"); // reset the terminal
	fflush(stdout);

	const char* hcc_path;
	if (platform_file_exists("../hcc")) { // release package
		hcc_path = "../hcc";
	} else { // repo
		hcc_path = "../build/hcc";
	}

	char buf[1024];
	snprintf(buf, sizeof(buf), "%s -O -fi shader.c -fo shader.spirv -fomc shader-metadata.h", hcc_path);
	if (system(buf) == 0) {
		time_t rawtime;
		struct tm* timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		if (gpu_reload_shaders()) {
			printf("SUCCESS: loaded shader.spirv at %s\n", asctime(timeinfo));
			fflush(stdout);
			return true;
		} else {
			printf("ERROR: loading shader.spirv at %s\nthis is due to a vulkan error, please send this in!\n", asctime(timeinfo));
			fflush(stdout);
		}
	}
	return false;
}

int main(int argc, char** argv) {
	APP_UNUSED(argc);
	APP_UNUSED(argv);

	platform_open_console();

	dm_init();

	int window_width, window_height;
	dm_screen_dims(&window_width, &window_height);
	window_width /= 2;
	window_height /= 2;

	DmWindow window = dm_window_open(window_width, window_height);
	float ar = (float)window_height / (float)window_width;

	gpu_init(window, window_width, window_height);
	recompile_shader();

	WatchedDirectory* wd = platform_watch_directory("./");

	GpuResourceId backbuffer_texture_id = gpu_create_backbuffer();

	ShaderBC bc;
	memset(&bc, 0x00, sizeof(bc)); // we zero the memory here to a avoid a driver crash if we forget to set a bindless index at all!


	bc.time_ = 0.f;
	bc.screen_width = window_width;
	bc.screen_height = window_height;
	while (1) {
		DmEvent event;
		while (dm_process_events(&event)) {
			switch (event.type) {
				case DM_EVENT_TYPE_WINDOW_CLOSED:
					exit(0);
				case DM_EVENT_TYPE_KEY_PRESSED:
					printf("key pressed %c\n", event.key);
					break;
				case DM_EVENT_TYPE_KEY_RELEASED:
					printf("key released %c\n", event.key);
					break;
			}
		}

		WatchedDirectoryEvent wevent;
		while (platform_watch_directory_next_event(wd, &wevent)) {
			switch (wevent.type) {
				case WATCHED_DIRECTORY_EVENT_TYPE_UNKNOWN:
					break;
				case WATCHED_DIRECTORY_EVENT_TYPE_CHANGED:
					if (strcmp(wevent.file_name, "shader.c") == 0) {
						recompile_shader();
					}
					break;
			}
		}

		gpu_render_frame(&bc);
		bc.time_ += 0.01f;
	}

	return 0;
}

