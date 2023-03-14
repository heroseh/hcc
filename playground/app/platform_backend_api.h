
typedef struct WatchedDirectory WatchedDirectory;

typedef uint8_t WatchedDirectoryEventType;
enum WatchedDirectoryEventType {
	WATCHED_DIRECTORY_EVENT_TYPE_UNKNOWN,
	WATCHED_DIRECTORY_EVENT_TYPE_CHANGED,
};

typedef struct WatchedDirectoryEvent WatchedDirectoryEvent;
struct WatchedDirectoryEvent {
	WatchedDirectoryEventType type;
	bool                      is_dir;
	const char*               file_name;
};

bool platform_file_read_all(const char* path, void** data_out, uintptr_t* size_out);
bool platform_file_exists(const char* path);
void platform_open_console(void);
WatchedDirectory* platform_watch_directory(const char* path);
bool platform_watch_directory_next_event(WatchedDirectory* handle, WatchedDirectoryEvent* e_out);
