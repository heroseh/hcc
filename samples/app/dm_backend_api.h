#pragma once

typedef int DmEventType;
enum DmEventType {
	DM_EVENT_TYPE_KEY_PRESSED,
	DM_EVENT_TYPE_KEY_RELEASED,
	DM_EVENT_TYPE_WINDOW_CLOSED,
};

typedef struct DmEvent DmEvent;
struct DmEvent {
	DmEventType type;
	union {
		char key;
	};
	void* handle;
};


typedef struct DmWindow DmWindow;
struct DmWindow {
	void* instance;
	void* handle;
};

void dm_init();
void dm_screen_dims(int* width_out, int* height_out);
DmWindow dm_window_open(int width, int height);
bool dm_process_events(DmEvent* e);

