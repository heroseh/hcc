
typedef struct WatchedFile WatchedFile;

bool platform_file_read_all(const char* path, void** data_out, uintptr_t* size_out);
bool platform_file_exists(const char* path);
void platform_open_console(void);
WatchedFile* platform_watch_file(const char* path);
bool platform_watch_file_check_if_changed(WatchedFile* handle);
