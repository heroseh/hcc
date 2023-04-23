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

	const char* hcc_path;
#ifdef _WIN32
	if (platform_file_exists("..\\hcc.exe")) { // release package
		hcc_path = "..\\hcc.exe";
	} else { // repo
		hcc_path = "..\\build\\hcc.exe";
	}
#else
	if (platform_file_exists("../hcc")) { // release package
		hcc_path = "../hcc";
	} else { // repo
		hcc_path = "../build/hcc";
	}
#endif

	char buf[1024];
	snprintf(buf, sizeof(buf), "%s -fi shader.c -fo shader.spirv -fomc shader-metadata.h", hcc_path);
	if (system(buf) == 0) {
		time_t rawtime;
		struct tm* timeinfo;
		time(&rawtime);

#ifdef _WIN32
		char timebuf[1024];
		timeinfo = malloc(sizeof(struct tm));
		localtime_s(timeinfo, &rawtime);
		asctime_s(timebuf, sizeof(timebuf), timeinfo);
#else
		timeinfo = localtime(&rawtime);
		char* timebuf = asctime(timeinfo);
#endif

		if (gpu_reload_shaders()) {
			printf("SUCCESS: loaded shader.spirv at %s\n1. open shader.c\n2. make edits\n3. save the file\n4. watch this window for success and errors messages\n5. read the docs and samples and enjoy :)\n", timebuf);
			return true;
		} else {
			printf("ERROR: loading shader.spirv at %s\nthis is due to a vulkan error, please send this in!\n", timebuf);
		}
	}
	return false;
}

int main(int argc, char** argv) {
	APP_UNUSED(argc);
	APP_UNUSED(argv);

	platform_open_console();
	platform_register_segfault_handler();
	dm_init();

	int window_width, window_height;
	dm_screen_dims(&window_width, &window_height);
	window_width /= 2;
	window_height /= 2;

	DmWindow window = dm_window_open(window_width, window_height);
	float ar = (float)window_height / (float)window_width;

	gpu_init(window, window_width, window_height);
	recompile_shader();

	WatchedFile* wf = platform_watch_file("./shader.c");

	GpuResourceId backbuffer_texture_id = gpu_create_backbuffer();

	ShaderBC bc;
	memset(&bc, 0x00, sizeof(bc)); // we zero the memory here to a avoid a driver crash if we forget to set a bindless index at all!


	bc.time_ = 0.f;
	while (1) {
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
					printf("key pressed %c\n", event.key);
					break;
				case DM_EVENT_TYPE_KEY_RELEASED:
					printf("key released %c\n", event.key);
					break;
			}
		}

		if (platform_watch_file_check_if_changed(wf)) {
			recompile_shader();
		}

		bc.screen_width = window_width;
		bc.screen_height = window_height;
		gpu_render_frame(&bc, window_width, window_height);
		bc.time_ += 0.01f;
	}

	return 0;
}

