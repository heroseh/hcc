
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
		return false;
	}

	*data_out = data;
	*size_out = file_size;
	return true;
}

