#include <stdio.h>

typedef enum DataType DataType;
enum DataType {
	DATA_TYPE_BOOL,
	DATA_TYPE_HALF,
	DATA_TYPE_FLOAT,
	DATA_TYPE_DOUBLE,
	DATA_TYPE_S8,
	DATA_TYPE_S16,
	DATA_TYPE_S32,
	DATA_TYPE_S64,
	DATA_TYPE_U8,
	DATA_TYPE_U16,
	DATA_TYPE_U32,
	DATA_TYPE_U64,

	DATA_TYPE_COUNT,
};

typedef enum Vector Vector;
enum Vector {
	VECTOR_1,
	VECTOR_2,
	VECTOR_3,
	VECTOR_4,

	VECTOR_COUNT,
};

typedef enum Texture Texture;
enum {
	RO_TEXTURE_1D,
	RO_TEXTURE_1D_ARRAY,
	RO_TEXTURE_2D,
	RO_TEXTURE_2D_ARRAY,
	RO_TEXTURE_2D_MS,
	RO_TEXTURE_2D_MS_ARRAY,
	RO_TEXTURE_CUBE,
	RO_TEXTURE_CUBE_ARRAY,
	RO_TEXTURE_CUBE_MS,
	RO_TEXTURE_CUBE_MS_ARRAY,
	RO_TEXTURE_3D,
	RW_TEXTURE_1D,
	RW_TEXTURE_1D_ARRAY,
	RW_TEXTURE_2D,
	RW_TEXTURE_2D_ARRAY,
	RW_TEXTURE_2D_MS,
	RW_TEXTURE_2D_MS_ARRAY,
	RW_TEXTURE_CUBE,
	RW_TEXTURE_CUBE_ARRAY,
	RW_TEXTURE_CUBE_MS,
	RW_TEXTURE_CUBE_MS_ARRAY,
	RW_TEXTURE_3D,

	TEXTURE_CUBE_COUNT,
};

static const char* data_type_identifiers[DATA_TYPE_COUNT] = {
	[DATA_TYPE_BOOL] = "bool",
	[DATA_TYPE_HALF] = "half",
	[DATA_TYPE_FLOAT] = "float",
	[DATA_TYPE_DOUBLE] = "double",
	[DATA_TYPE_S8] = "int8_t",
	[DATA_TYPE_S16] = "int16_t",
	[DATA_TYPE_S32] = "int32_t",
	[DATA_TYPE_S64] = "int64_t",
	[DATA_TYPE_U8] = "uint8_t",
	[DATA_TYPE_U16] = "uint16_t",
	[DATA_TYPE_U32] = "uint32_t",
	[DATA_TYPE_U64] = "uint64_t",
};

static const char* data_type_suffixes[DATA_TYPE_COUNT] = {
	[DATA_TYPE_BOOL] = "bool",
	[DATA_TYPE_HALF] = "f16",
	[DATA_TYPE_FLOAT] = "f32",
	[DATA_TYPE_DOUBLE] = "f64",
	[DATA_TYPE_S8] = "s8",
	[DATA_TYPE_S16] = "s16",
	[DATA_TYPE_S32] = "s32",
	[DATA_TYPE_S64] = "s64",
	[DATA_TYPE_U8] = "u8",
	[DATA_TYPE_U16] = "u16",
	[DATA_TYPE_U32] = "u32",
	[DATA_TYPE_U64] = "u64",
};

static const char* vector_suffixes[] = {
	[VECTOR_1] = "",
	[VECTOR_2] = "x2",
	[VECTOR_3] = "x3",
	[VECTOR_4] = "x4",
};

static const char* texture_names_snake_case[TEXTURE_CUBE_COUNT] = {
	[RO_TEXTURE_1D] = "ro_texture_1d",
	[RO_TEXTURE_1D_ARRAY] = "ro_texture_1d_array",
	[RO_TEXTURE_2D] = "ro_texture_2d",
	[RO_TEXTURE_2D_ARRAY] = "ro_texture_2d_array",
	[RO_TEXTURE_2D_MS] = "ro_texture_2d_ms",
	[RO_TEXTURE_2D_MS_ARRAY] = "ro_texture_2d_ms_array",
	[RO_TEXTURE_CUBE] = "ro_texture_cube",
	[RO_TEXTURE_CUBE_ARRAY] = "ro_texture_cube_array",
	[RO_TEXTURE_CUBE_MS] = "ro_texture_cube_ms",
	[RO_TEXTURE_CUBE_MS_ARRAY] = "ro_texture_cube_ms_array",
	[RO_TEXTURE_3D] = "ro_texture_3d",
	[RW_TEXTURE_1D] = "ro_texture_1d",
	[RW_TEXTURE_1D_ARRAY] = "ro_texture_1d_array",
	[RW_TEXTURE_2D] = "ro_texture_2d",
	[RW_TEXTURE_2D_ARRAY] = "ro_texture_2d_array",
	[RW_TEXTURE_2D_MS] = "ro_texture_2d_ms",
	[RW_TEXTURE_2D_MS_ARRAY] = "ro_texture_2d_ms_array",
	[RW_TEXTURE_CUBE] = "ro_texture_cube",
	[RW_TEXTURE_CUBE_ARRAY] = "ro_texture_cube_array",
	[RW_TEXTURE_CUBE_MS] = "ro_texture_cube_ms",
	[RW_TEXTURE_CUBE_MS_ARRAY] = "ro_texture_cube_ms_array",
	[RW_TEXTURE_3D] = "ro_texture_3d",
};

static const char* texture_names_title_case[TEXTURE_CUBE_COUNT] = {
	[RO_TEXTURE_1D] = "RoTexture1D",
	[RO_TEXTURE_1D_ARRAY] = "RoTexture1DArray",
	[RO_TEXTURE_2D] = "RoTexture2D",
	[RO_TEXTURE_2D_ARRAY] = "RoTexture2DArray",
	[RO_TEXTURE_2D_MS] = "RoTexture2DMS",
	[RO_TEXTURE_2D_MS_ARRAY] = "RoTexture2DMSArray",
	[RO_TEXTURE_CUBE] = "RoTextureCube",
	[RO_TEXTURE_CUBE_ARRAY] = "RoTextureCubeArray",
	[RO_TEXTURE_CUBE_MS] = "RoTextureCubeMS",
	[RO_TEXTURE_CUBE_MS_ARRAY] = "RoTextureCubeMSArray",
	[RO_TEXTURE_3D] = "RoTexture3D",
	[RW_TEXTURE_1D] = "RwTexture1D",
	[RW_TEXTURE_1D_ARRAY] = "RwTexture1DArray",
	[RW_TEXTURE_2D] = "RwTexture2D",
	[RW_TEXTURE_2D_ARRAY] = "RwTexture2DArray",
	[RW_TEXTURE_2D_MS] = "RwTexture2DMS",
	[RW_TEXTURE_2D_MS_ARRAY] = "RwTexture2DMSArray",
	[RW_TEXTURE_CUBE] = "RwTextureCube",
	[RW_TEXTURE_CUBE_ARRAY] = "RwTextureCubeArray",
	[RW_TEXTURE_CUBE_MS] = "RwTextureCubeMS",
	[RW_TEXTURE_CUBE_MS_ARRAY] = "RwTextureCubeMSArray",
	[RW_TEXTURE_3D] = "RwTexture3D",
};

static const char* texture_index_type[TEXTURE_CUBE_COUNT] = {
	[RO_TEXTURE_1D] = "uint32_t",
	[RO_TEXTURE_1D_ARRAY] = "u32x2",
	[RO_TEXTURE_2D] = "u32x2",
	[RO_TEXTURE_2D_ARRAY] = "u32x3",
	[RO_TEXTURE_2D_MS] = "u32x2",
	[RO_TEXTURE_2D_MS_ARRAY] = "u32x3",
	[RO_TEXTURE_CUBE] = NULL,
	[RO_TEXTURE_CUBE_ARRAY] = NULL,
	[RO_TEXTURE_CUBE_MS] = NULL,
	[RO_TEXTURE_CUBE_MS_ARRAY] = NULL,
	[RO_TEXTURE_3D] = "u32x3",
	[RW_TEXTURE_1D] = "uint32_t",
	[RW_TEXTURE_1D_ARRAY] = "u32x2",
	[RW_TEXTURE_2D] = "u32x2",
	[RW_TEXTURE_2D_ARRAY] = "u32x3",
	[RW_TEXTURE_2D_MS] = "u32x2",
	[RW_TEXTURE_2D_MS_ARRAY] = "u32x3",
	[RW_TEXTURE_CUBE] = NULL,
	[RW_TEXTURE_CUBE_ARRAY] = NULL,
	[RW_TEXTURE_CUBE_MS] = NULL,
	[RW_TEXTURE_CUBE_MS_ARRAY] = NULL,
	[RW_TEXTURE_3D] = "u32x3",
};

static const char* texture_coord_type[TEXTURE_CUBE_COUNT] = {
	[RO_TEXTURE_1D] = "float",
	[RO_TEXTURE_1D_ARRAY] = "f32x2",
	[RO_TEXTURE_2D] = "f32x2",
	[RO_TEXTURE_2D_ARRAY] = "f32x3",
	[RO_TEXTURE_2D_MS] = "f32x2",
	[RO_TEXTURE_2D_MS_ARRAY] = "f32x3",
	[RO_TEXTURE_CUBE] = "f32x3",
	[RO_TEXTURE_CUBE_ARRAY] = "f32x4",
	[RO_TEXTURE_CUBE_MS] = "f32x3",
	[RO_TEXTURE_CUBE_MS_ARRAY] = "f32x3",
	[RO_TEXTURE_3D] = "f32x3",
	[RW_TEXTURE_1D] = "uint32_t",
	[RW_TEXTURE_1D_ARRAY] = "f32x2",
	[RW_TEXTURE_2D] = "f32x2",
	[RW_TEXTURE_2D_ARRAY] = "f32x3",
	[RW_TEXTURE_2D_MS] = "f32x2",
	[RW_TEXTURE_2D_MS_ARRAY] = "f32x3",
	[RW_TEXTURE_CUBE] = "f32x3",
	[RW_TEXTURE_CUBE_ARRAY] = "f32x4",
	[RW_TEXTURE_CUBE_MS] = "f32x3",
	[RW_TEXTURE_CUBE_MS_ARRAY] = "f32x3",
	[RW_TEXTURE_3D] = "f32x3",
};

struct Ctx {
	FILE* f;
	DataType data_type;
	Vector vector;
	Texture texture;
};

void print_entry(const char* string) {
	const char* prev_special = string, *next_special = string;
	while ((next_special = strchr(next_special, '$'))) {
		unsigned size = next_special - prev_special;
		fprintf(ctx.f,"%.*s", size, prev_special);
		next_special += 1; // skip '$'
		switch (*next_special) {
		case 'd':
			next_special += 1; // skip 'd'
			switch (*next_special) {
			case 'i':
				fprintf(ctx.f,"%s", data_type_identifiers[ctx.data_type]);
				break;
			case 'I':
				fprintf(ctx.f,"%-8s", data_type_identifiers[ctx.data_type]);
				break;
			case 'x':
				fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
				break;
			case 'X':
				fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
				break;
			case 'b':
				fprintf(ctx.f,"%u", data_type_sizes[ctx.data_type] * 8);
				break;
			case 'm':
				switch (ctx.data_type) {
					case DATA_TYPE_S8: fprintf(ctx.f,"0x80"); break;
					case DATA_TYPE_S16: fprintf(ctx.f,"0x8000"); break;
					case DATA_TYPE_S32: fprintf(ctx.f,"0x800000"); break;
					case DATA_TYPE_S64: fprintf(ctx.f,"0x80000000"); break;
					default: break;
				}
				break;
			}
			break;
		case 'v':
			next_special += 1; // skip 'v'
			switch (*next_special) {
				case 'a':
					fprintf(ctx.f,"%u", vector_align_comps[ctx.vector] * data_type_sizes[ctx.data_type]);
					break;
				case 'b':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type + 4]);
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					break;
				case 'c':
					fprintf(ctx.f,"%u", vector_comps[ctx.vector]);
					break;
				case '2':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes[VECTOR_2]);
					break;
				case '3':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes[VECTOR_3]);
					break;
				case 's':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					break;
				case 'S':
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					fprintf(ctx.f,"%s", vector_suffixes_cap[ctx.vector]);
					break;
				case 'x':
					fprintf(ctx.f,"%s", vector_suffixes[ctx.vector]);
					break;
				case 'z':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%-4s", vector_suffixes[ctx.vector]);
					break;
				case 'Z':
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					fprintf(ctx.f,"%-4s", vector_suffixes_cap[ctx.vector]);
					break;
			}
			break;
		case 'm':
			next_special += 1; // skip 'm'
			switch (*next_special) {
				case 'c':
					fprintf(ctx.f,"%u", matrix_columns_count[ctx.matrix]);
					break;
				case 'r':
					fprintf(ctx.f,"%u", matrix_rows_count[ctx.matrix]);
					break;
				case 's':
					fprintf(ctx.f,"%-2u", matrix_columns_count[ctx.matrix] * matrix_rows_count[ctx.matrix]);
					break;
				case 'S':
					fprintf(ctx.f,"%-2u", 4 * matrix_rows_count[ctx.matrix]);
					break;
				case 'v':
					fprintf(ctx.f,"%s%s", data_type_suffixes[ctx.data_type], vector_suffixes[ctx.matrix / 3]);
					break;
				case 'V':
					fprintf(ctx.f,"%s%s", data_type_suffixes[ctx.data_type], vector_suffixes[ctx.matrix % 3]);
					break;
				case 'x':
					fprintf(ctx.f,"%s", data_type_suffixes[ctx.data_type]);
					fprintf(ctx.f,"%s", matrix_suffixes[ctx.matrix]);
					break;
				case 'X':
					fprintf(ctx.f,"%s", data_type_suffixes_cap[ctx.data_type]);
					fprintf(ctx.f,"%s", matrix_suffixes_cap[ctx.matrix]);
					break;
			}
			break;
		}

		next_special += 1;
		prev_special = next_special;
	}
	fprintf(ctx.f,"%s", prev_special);
}

FILE* open_file_write(const char* path) {
	FILE* f = fopen(path, "w");
	if (f == NULL) {
		fprintf(stderr, "failed to open file at '%s'\n", path);
		exit(1);
	}
	return f;
}

void print_file_header() {
	fprintf(ctx.f,
		"// !?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!\n"
		"// !?!?!? WARNING CONTRIBUTOR ?!?!?!\n"
		"// !?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!\n"
		"// this file is generated by tools/std_math_gen.c\n"
		"// please edit that file and regenerate this one if you want to make edits\n"
		"\n"
	);
}

void print_header_file_header(const char* guard) {
	print_file_header();
	fprintf(ctx.f,
		"#pragma once\n"
		"#ifndef %s\n"
		"#define %s\n"
		"\n", guard, guard
	);
}

void print_header_file_footer(const char* guard) {
	fprintf(ctx.f, "\n#endif // %s\n" , guard);
}

int main(int argc, char** argv) {
	ctx.f = open_file_write("libhccstd/texture.h");
	print_header_file_header("_HCC_TEXTURE_H_");

	print_header_file_footer("_HCC_TEXTURE_H_");
}


