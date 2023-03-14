#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/inotify.h>

#define WATCH_BUFFER_SIZE 8192
typedef struct WatchedDirectoryLinux WatchedDirectoryLinux;
struct WatchedDirectoryLinux {
	int fd;
	int wd;
	int buffer_pos;
	int buffer_size;
	char buffer[WATCH_BUFFER_SIZE];
};

bool platform_file_read_all(const char* path, void** data_out, uintptr_t* size_out) {
	FILE* f = fopen(path, "rb");
	if (!f) {
		return false;
	}

	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	void* data = malloc(file_size);
	long read_size = fread(data, 1, file_size, f);
	if (read_size != file_size) {
		return false;
	}

	*data_out = data;
	*size_out = file_size;
	return true;
}

WatchedDirectory* platform_watch_directory(const char* path) {
	WatchedDirectoryLinux* w = malloc(sizeof(WatchedDirectoryLinux));
	w->fd = inotify_init();
	w->wd = inotify_add_watch(w->fd, path, IN_MODIFY);
	w->buffer_pos = 0;
	w->buffer_size = 0;

	fcntl(w->fd, F_SETFL, fcntl(w->fd, F_GETFL) | O_NONBLOCK);

	return (WatchedDirectory*)w;
}

bool platform_watch_directory_next_event(WatchedDirectory* handle, WatchedDirectoryEvent* e_out) {
	WatchedDirectoryLinux* w = (WatchedDirectoryLinux*)handle;

	if (w->buffer_pos >= w->buffer_size) {
		w->buffer_pos = 0;
		w->buffer_size = 0;

		w->buffer_size = read(w->fd, w->buffer, WATCH_BUFFER_SIZE);
		if (w->buffer_size == -1) {
			w->buffer_size = 0;
		}
	}

	if (w->buffer_pos >= w->buffer_size) {
		return false;
	}

	struct inotify_event* event = (struct inotify_event*)&w->buffer[w->buffer_pos];
	w->buffer_pos += sizeof(struct inotify_event) + event->len;

	if (event->mask & IN_MODIFY) {
		e_out->type = WATCHED_DIRECTORY_EVENT_TYPE_CHANGED;
		e_out->is_dir = event->mask & IN_ISDIR;
		e_out->file_name = event->name;
		return true;
	}

	e_out->type = WATCHED_DIRECTORY_EVENT_TYPE_UNKNOWN;
	e_out->is_dir = event->mask & IN_ISDIR;
	e_out->file_name = event->name;
	return true;
}

