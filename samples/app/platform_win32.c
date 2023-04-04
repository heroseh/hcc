#include <signal.h>

bool platform_file_read_all(const char* path, void** data_out, uintptr_t* size_out) {
	FILE* f;
	if (fopen_s(&f, path, "rb")) {
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

	MessageBoxA(NULL, buf, NULL, 0);
}

void segfault_handler(int signum) {
	APP_UNUSED(signum);

	FILE* f;
	fopen_s(&f, "segfault_log.txt", "w");
	char* stacktrace = b_stacktrace_get_string();
	fprintf(f, "Stacktrace:\n%s\n\n", stacktrace);
	fflush(f);
	platform_message_box("Segfault Detected.\nThe error has been logged to the segfault_log.txt file.\nPlease report this error and the log file on the HCC github issue tracker");
	abort();
}

void platform_register_segfault_handler(void) {
	signal(SIGSEGV, &segfault_handler);
}
