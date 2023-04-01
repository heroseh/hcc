#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

typedef struct WatchedFileLinux WatchedFileLinux;
struct WatchedFileLinux {
	const char* path;
	struct timespec time;
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
		fclose(f);
		return false;
	}

	*data_out = data;
	*size_out = file_size;
	fclose(f);
	return true;
}

bool platform_file_exists(const char* path) {
	return access(path, F_OK) == 0;
}

void platform_open_console(void) {
	int fds[2];
	if(pipe(fds) == -1) {
		abort();
	}

	int child_pid = fork();
	if(child_pid == -1) {
		abort();
	}

	if(child_pid == 0) {
		close(fds[1]);
		char f[PATH_MAX + 1];
		sprintf(f, "/dev/fd/%d", fds[0]);
		execlp("xterm", "xterm", "-bg", "black", "-fg", "white", "-fa", "Monospace", "-fs", "12", "-e", "cat", f, NULL);
		abort();
	}

	sleep(1);
	close(fds[0]);
	dup2(fds[1], fileno(stdout));
	setbuf(stdout, NULL);
}

WatchedFile* platform_watch_file(const char* path) {
	WatchedFileLinux* w = malloc(sizeof(WatchedFileLinux));
	w->path = path;
	w->time.tv_sec = 0;
	w->time.tv_nsec = 0;

	struct stat s;
	if (stat(w->path, &s) != -1) {
		w->time = s.st_mtim;
	}

	return (WatchedFile*)w;
}

bool platform_watch_file_check_if_changed(WatchedFile* handle) {
	WatchedFileLinux* w = (WatchedFileLinux*)handle;

	struct stat s;
	if (stat(w->path, &s) == -1) {
		return false;
	}

	bool has_changed = s.st_mtim.tv_sec != w->time.tv_sec || s.st_mtim.tv_nsec != w->time.tv_nsec;

	w->time = s.st_mtim;

	return has_changed;
}

