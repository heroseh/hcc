#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
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

void platform_message_box(const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, va_args);
	va_end(va_args);

	char command[1024];
	if (system("command -v zenity") == 0) {
		snprintf(command, sizeof(command), "zenity --error --text \"%s\"", buf);
	} else if (system("command -v gxmessage") == 0) {
		snprintf(command, sizeof(command), "gxmessage \"%s\"", buf);
	} else if (system("command -v xmessage") == 0) {
		snprintf(command, sizeof(command), "xmessage \"%s\"", buf);
	} else if (system("command -v notify-send") == 0) {
		snprintf(command, sizeof(command), "notify-send Error \"%s\"", buf);
	} else {
		printf("%s\n", buf);
		return;
	}
	system(command);
}

void segfault_handler(int signum, siginfo_t* info, void* data) {
	APP_UNUSED(signum);
	APP_UNUSED(info);
	APP_UNUSED(data);

	FILE* f = fopen("segfault_log.txt", "w");
	char* stacktrace = b_stacktrace_get_string();
	fprintf(f, "Stacktrace:\n%s\n\n", stacktrace);
	fflush(f);
	platform_message_box("Segfault Detected.\nThe error has been logged to the segfault_log.txt file.\nPlease report this error and the log file on the HCC github issue tracker");
	abort();
}

void platform_register_segfault_handler(void) {
	struct sigaction sa = {0};
	sigemptyset(&sa.sa_mask);
	sa.sa_flags     = SA_NODEFER;
	sa.sa_sigaction = segfault_handler;
	sigaction(SIGSEGV, &sa, NULL);
}


