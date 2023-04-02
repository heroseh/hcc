
bool platform_file_read_all(const char* path, void** data_out, uintptr_t* size_out);
void platform_message_box(const char* fmt, ...);
void platform_register_segfault_handler(void);
