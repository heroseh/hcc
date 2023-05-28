#include "hcc_interop.h"

#include <string.h>
#include <stdio.h>

#include <hcc_shader.h>
#include <hmaths_scalar.h>

const char* hcc_resource_access_mode_strings[HCC_RESOURCE_ACCESS_MODE_COUNT] = {
	[HCC_RESOURCE_ACCESS_MODE_READ_ONLY] = "HCC_RESOURCE_ACCESS_MODE_READ_ONLY",
	[HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY] = "HCC_RESOURCE_ACCESS_MODE_WRITE_ONLY",
	[HCC_RESOURCE_ACCESS_MODE_READ_WRITE] = "HCC_RESOURCE_ACCESS_MODE_READ_WRITE",
	[HCC_RESOURCE_ACCESS_MODE_SAMPLE] = "HCC_RESOURCE_ACCESS_MODE_SAMPLE",
};

const char* hcc_shader_stage_strings[HCC_SHADER_STAGE_COUNT] = {
	[HCC_SHADER_STAGE_NONE] = "HCC_SHADER_STAGE_NONE",
	[HCC_SHADER_STAGE_VERTEX] = "HCC_SHADER_STAGE_VERTEX",
	[HCC_SHADER_STAGE_PIXEL] = "HCC_SHADER_STAGE_PIXEL",
	[HCC_SHADER_STAGE_COMPUTE] = "HCC_SHADER_STAGE_COMPUTE",
	[HCC_SHADER_STAGE_MESH_TASK] = "HCC_SHADER_STAGE_MESH_TASK",
	[HCC_SHADER_STAGE_MESH] = "HCC_SHADER_STAGE_MESH",
};

const char* hcc_resource_type_strings[HCC_RESOURCE_TYPE_COUNT] = {
	[HCC_RESOURCE_TYPE_BUFFER] = "HCC_RESOURCE_TYPE_BUFFER",
	[HCC_RESOURCE_TYPE_TEXTURE_1D] = "HCC_RESOURCE_TYPE_TEXTURE_1D",
	[HCC_RESOURCE_TYPE_TEXTURE_1D_ARRAY] = "HCC_RESOURCE_TYPE_TEXTURE_1D_ARRAY",
	[HCC_RESOURCE_TYPE_TEXTURE_2D] = "HCC_RESOURCE_TYPE_TEXTURE_2D",
	[HCC_RESOURCE_TYPE_TEXTURE_2D_MS] = "HCC_RESOURCE_TYPE_TEXTURE_2D_MS",
	[HCC_RESOURCE_TYPE_TEXTURE_2D_ARRAY] = "HCC_RESOURCE_TYPE_TEXTURE_2D_ARRAY",
	[HCC_RESOURCE_TYPE_TEXTURE_2D_MS_ARRAY] = "HCC_RESOURCE_TYPE_TEXTURE_2D_MS_ARRAY",
	[HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE] = "HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE",
	[HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE_ARRAY] = "HCC_RESOURCE_TYPE_TEXTURE_2D_CUBE_ARRAY",
	[HCC_RESOURCE_TYPE_TEXTURE_3D] = "HCC_RESOURCE_TYPE_TEXTURE_3D",
	[HCC_RESOURCE_TYPE_SAMPLER] = "HCC_RESOURCE_TYPE_SAMPLER",
};

void hcc_print_hprintf_buffer(const uint32_t* print_buffer) {
	uint32_t buffer_size = print_buffer[0]; // get last frames final cursor
	uint32_t cursor = 2; // start after the cursor index & buffer size words
	while (cursor < buffer_size) {
		const char* fmt = (char*)&print_buffer[cursor];
		uint32_t fmt_size = strlen(fmt);
		cursor += HPRINT_STRING_SIZE(fmt);

		uint32_t fmt_idx = 0;
		while (fmt_idx < fmt_size) {
			char ch = fmt[fmt_idx];
			fmt_idx += 1;
			if (ch != '%') {
				putchar(ch);
				continue;
			}

			ch = fmt[fmt_idx];
			fmt_idx += 1;
			if (ch == 'z') {
				ch = fmt[fmt_idx];
				switch (ch) {
					case 'u':
						printf("%zu", (uint64_t)print_buffer[cursor] | ((uint64_t)print_buffer[cursor + 1] << 32));
						break;
					case 'd':
						printf("%zd", (uint64_t)print_buffer[cursor] | ((uint64_t)print_buffer[cursor + 1] << 32));
						break;
					case 'x':
						printf("%zx", (uint64_t)print_buffer[cursor] | ((uint64_t)print_buffer[cursor + 1] << 32));
						break;
				}
				cursor += 2;
			} else {
				switch (ch) {
					case 'u':
						printf("%u", print_buffer[cursor]);
						break;
					case 'd':
						printf("%d", print_buffer[cursor]);
						break;
					case 'x':
						printf("%x", print_buffer[cursor]);
						break;
					case 'f':
						printf("%f", bitsto_f32(print_buffer[cursor]));
						break;
				}
				cursor += 1;
			}
		}
	}
}

