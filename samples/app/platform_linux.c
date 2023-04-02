#include <signal.h>

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

void platform_message_box(const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, va_args);
	va_end(va_args);

	char command[1024];
	snprintf(command, sizeof(command), "xmessage \"%s\"", buf);
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

