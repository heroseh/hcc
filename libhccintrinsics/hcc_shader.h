#pragma once
#ifndef _HCC_SHADER_H_
#define _HCC_SHADER_H_

// ===========================================
//
//
// Misc
//
//
// ===========================================

#if __STDC__ == 1 && __STDC_VERSION__ >= 201112L
#define HCC_HAS_C_GENERIC_SUPPORT
#endif

// ===========================================
//
//
// Metaprogramming
//
//
// ===========================================

#define HCC_CONCAT_(a,b) a ## b
#define HCC_CONCAT(a, b) HCC_CONCAT_(a, b)

#define HCC_PP_ARGS_COUNT(...)    HCC_PP_ARGS_COUNT_(__VA_ARGS__, HCC_PP_ARGS_COUNT_REV_SEQ())
#define HCC_PP_ARGS_COUNT_(...)   HCC_PP_ARGS_COUNT_INTERNAL(__VA_ARGS__)

#define HCC_PP_ARGS_COUNT_INTERNAL( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,  \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
	_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
	_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
	_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
	_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
	_61,_62,_63,N,...) N

#define HCC_PP_ARGS_COUNT_REV_SEQ() \
	63,62,61,60,                   \
	59,58,57,56,55,54,53,52,51,50, \
	49,48,47,46,45,44,43,42,41,40, \
	39,38,37,36,35,34,33,32,31,30, \
	29,28,27,26,25,24,23,22,21,20, \
	19,18,17,16,15,14,13,12,11,10, \
	9,8,7,6,5,4,3,2,1,0

#define HCC_PP_UNTUPLE(...) __VA_ARGS__
#define HCC_PP_CALL(f, ...) f(__VA_ARGS__)

#define HCC_PP_ARGS_FOREACH_1(f, oarg, next_arg)       HCC_PP_CALL(f, oarg, next_arg)
#define HCC_PP_ARGS_FOREACH_2(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_1(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_3(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_2(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_4(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_3(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_5(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_4(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_6(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_5(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_7(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_6(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_8(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_7(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_9(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_8(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_10(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_9(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_11(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_10(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_12(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_11(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_13(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_12(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_14(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_13(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_15(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_14(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_16(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) HCC_PP_ARGS_FOREACH_15(f, oarg, __VA_ARGS__)

#define HCC_PP_ARGS_FOREACH_(f, oarg, M, ...) M(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH(f, oarg, ...) HCC_PP_ARGS_FOREACH_(f, oarg, HCC_CONCAT(HCC_PP_ARGS_FOREACH_, HCC_PP_ARGS_COUNT(__VA_ARGS__)), __VA_ARGS__)

#define HCC_PP_ARGS_FOREACH_SUM_1(f, oarg, next_arg)       HCC_PP_CALL(f, oarg, next_arg)
#define HCC_PP_ARGS_FOREACH_SUM_2(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_1(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_3(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_2(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_4(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_3(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_5(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_4(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_6(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_5(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_7(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_6(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_8(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_7(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_9(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_8(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_10(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_9(f,  oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_11(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_10(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_12(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_11(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_13(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_12(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_14(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_13(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_15(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_14(f, oarg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM_16(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, next_arg) + HCC_PP_ARGS_FOREACH_SUM_15(f, oarg, __VA_ARGS__)

#define HCC_PP_ARGS_FOREACH_SUM_(f, outer_arg, M, ...) M(f, outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_SUM(f, outer_arg, ...) HCC_PP_ARGS_FOREACH_SUM_(f, outer_arg, HCC_CONCAT(HCC_PP_ARGS_FOREACH_SUM_, HCC_PP_ARGS_COUNT(__VA_ARGS__)), __VA_ARGS__)

// ===========================================
//
//
// Keywords
//
//
// ===========================================

#ifdef __HCC_GPU__
#define HCC_VERTEX __hcc_vertex
#define HCC_PIXEL __hcc_pixel
#define HCC_COMPUTE(x, y, z) __hcc_compute(x, y, z)
#define HCC_MESHTASK __hcc_meshtask
#define HCC_MESH __hcc_mesh
#define HCC_RASTERIZER_STATE __hcc_rasterizer_state
#define HCC_NOINTERP __hcc_nointerp
#define HCC_PIXEL_STATE __hcc_pixel_state
#define HCC_INTERP __hcc_interp
#define HCC_DISPATCH_GROUP __hcc_dispatch_group
#else // !__HCC_GPU__
#define HCC_VERTEX
#define HCC_PIXEL
#define HCC_COMPUTE(x, y, z)
#define HCC_MESHTASK
#define HCC_MESH
#define HCC_RASTERIZER_STATE
#define HCC_NOINTERP
#define HCC_PIXEL_STATE
#define HCC_INTERP
#define HCC_DISPATCH_GROUP static
#endif // !__HCC_GPU__

// ===========================================
//
//
// Resources - (Buffers, Textures, Samplers)
//
//
// ===========================================

#ifndef __HCC_GPU__
#define HccRoSampler uint32_t
#define HccRoBuffer(T) uint32_t
#define HccRoTexture1D(T) uint32_t
#define HccRoTexture1DArray(T) uint32_t
#define HccRoTexture2D(T) uint32_t
#define HccRoTexture2DArray(T) uint32_t
#define HccRoTexture2DMS(T) uint32_t
#define HccRoTexture2DMSArray(T) uint32_t
#define HccRoTexture3D(T) uint32_t
#define HccWoBuffer(T) uint32_t
#define HccWoTexture1D(T) uint32_t
#define HccWoTexture1DArray(T) uint32_t
#define HccWoTexture2D(T) uint32_t
#define HccWoTexture2DArray(T) uint32_t
#define HccWoTexture2DMS(T) uint32_t
#define HccWoTexture2DMSArray(T) uint32_t
#define HccWoTexture3D(T) uint32_t
#define HccRwBuffer(T) uint32_t
#define HccRwTexture1D(T) uint32_t
#define HccRwTexture1DArray(T) uint32_t
#define HccRwTexture2D(T) uint32_t
#define HccRwTexture2DArray(T) uint32_t
#define HccRwTexture2DMS(T) uint32_t
#define HccRwTexture2DMSArray(T) uint32_t
#define HccRwTexture3D(T) uint32_t
#define HccSampleTexture1D(T) uint32_t
#define HccSampleTexture1DArray(T) uint32_t
#define HccSampleTexture2D(T) uint32_t
#define HccSampleTexture2DArray(T) uint32_t
#define HccSampleTextureCube(T) uint32_t
#define HccSampleTextureCubeArray(T) uint32_t
#define HccSampleTexture3D(T) uint32_t
#endif // !__HCC_GPU__

typedef HccRoBuffer(uint32_t) HccRoByteBuffer;
typedef HccRwBuffer(uint32_t) HccRwByteBuffer;
typedef HccWoBuffer(uint32_t) HccWoByteBuffer;

//
// loads a value from 'byte_buffer' at 'byte_idx'.
// 'byte_idx' is rounded down to the nearest 4 bytes due to the current limitation of 4 byte scalars types only being supported
//
uint32_t load_byte_buffer_u32(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
int32_t load_byte_buffer_s32(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
float load_byte_buffer_f32(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
u32x2 load_byte_buffer_u32x2(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
s32x2 load_byte_buffer_s32x2(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
f32x2 load_byte_buffer_f32x2(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
u32x3 load_byte_buffer_u32x3(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
s32x3 load_byte_buffer_s32x3(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
f32x3 load_byte_buffer_f32x3(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
u32x4 load_byte_buffer_u32x4(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
s32x4 load_byte_buffer_s32x4(HccRoByteBuffer byte_buffer, uint32_t byte_idx);
f32x4 load_byte_buffer_f32x4(HccRoByteBuffer byte_buffer, uint32_t byte_idx);

//
// stores a value in 'byte_buffer' at 'byte_idx'.
// 'byte_idx' is rounded down to the nearest 4 bytes due to the current limitation of 4 byte scalars types only being supported
//
void store_byte_buffer_u32(HccWoByteBuffer byte_buffer, uint32_t byte_idx, uint32_t value);
void store_byte_buffer_s32(HccWoByteBuffer byte_buffer, uint32_t byte_idx, int32_t value);
void store_byte_buffer_f32(HccWoByteBuffer byte_buffer, uint32_t byte_idx, float value);
void store_byte_buffer_u32x2(HccWoByteBuffer byte_buffer, uint32_t byte_idx, u32x2 value);
void store_byte_buffer_s32x2(HccWoByteBuffer byte_buffer, uint32_t byte_idx, s32x2 value);
void store_byte_buffer_f32x2(HccWoByteBuffer byte_buffer, uint32_t byte_idx, f32x2 value);
void store_byte_buffer_u32x3(HccWoByteBuffer byte_buffer, uint32_t byte_idx, u32x3 value);
void store_byte_buffer_s32x3(HccWoByteBuffer byte_buffer, uint32_t byte_idx, s32x3 value);
void store_byte_buffer_f32x3(HccWoByteBuffer byte_buffer, uint32_t byte_idx, f32x3 value);
void store_byte_buffer_u32x4(HccWoByteBuffer byte_buffer, uint32_t byte_idx, u32x4 value);
void store_byte_buffer_s32x4(HccWoByteBuffer byte_buffer, uint32_t byte_idx, s32x4 value);
void store_byte_buffer_f32x4(HccWoByteBuffer byte_buffer, uint32_t byte_idx, f32x4 value);

#ifdef HCC_HAS_C_GENERIC_SUPPORT
#define store_byte_bufferG(byte_buffer, byte_idx, value) \
	_Generic((value), \
		uint32_t: store_byte_buffer_u32, \
		int32_t: store_byte_buffer_s32, \
		float: store_byte_buffer_f32, \
		u32x2: store_byte_buffer_u32x2, \
		s32x2: store_byte_buffer_s32x2, \
		f32x2: store_byte_buffer_f32x2, \
		u32x3: store_byte_buffer_u32x3, \
		s32x3: store_byte_buffer_s32x3, \
		f32x3: store_byte_buffer_f32x3, \
		u32x4: store_byte_buffer_u32x4, \
		s32x4: store_byte_buffer_s32x4, \
		f32x4: store_byte_buffer_f32x4 \
	)(byte_buffer, byte_idx, value)
#endif // HCC_HAS_C_GENERIC_SUPPORT

// ===========================================
//
//
// Printing From Shaders
//
//
// ===========================================

// the string passed into hprint_string is rounded up to the nearest uint32_t, the remaining bytes will be zero
#ifdef __HCC__
#define HPRINT_STRING_SIZE(string) ((sizeof(string) + 3) & ~3)
#else
#define HPRINT_STRING_SIZE(string) ((strlen(string) + 1 + 3) & ~3)
#endif

#define HPRINTF_BUFFER_CURSOR_IDX   0 // cursor in the hprintf buffer is atomically incremented by shaders to get the insert index
#define HPRINTF_BUFFER_CAPACITY_IDX 1 // capacity in the hprintf buffer is fetched to avoid overflow

#define HPRINTF_BUFFER_CURSOR_START_IDX 2 // the cursor starts at 2 as 0 and 1 is used for cursor and capacity respectively

//
// prints a 'string' into 'buffer' at 'idx'
//
// 'string' must be a string literal and the size is rounded up to the nearest uint32_t, the remaining bytes will be zero
//
void hprint_string(HccWoBuffer(uint32_t) buffer, uint32_t idx, const char* string);

#define HPRINT_INTERNAL_ELMT_WORD_COUNT(buffer, elmt) \
	_Generic((elmt), \
		bool: 1, half: 1, float: 1, double: 2, int8_t: 1, int16_t: 1, int32_t: 1, int64_t: 1, uint8_t: 1, uint16_t: 1, uint32_t: 1, uint64_t: 2 \
	)

#define HPRINT_INTERNAL_PRINT_ELMT(buffer, elmt) \
	_Generic((elmt), \
		bool: (buffer[_hprintf_idx] = elmt, _hprintf_idx += 1), \
		half: (buffer[_hprintf_idx] = bitsfrom_f16(elmt), _hprintf_idx += 1), \
		float: (buffer[_hprintf_idx] = bitsfrom_f32(elmt), _hprintf_idx += 1), \
		double: (buffer[_hprintf_idx] = (bitsfrom_f64(elmt) >> 0) & 0xffffffff, buffer[_hprintf_idx + 1] = (bitsfrom_f64(elmt) >> 32) & 0xffffffff, _hprintf_idx += 2), \
		int8_t: (buffer[_hprintf_idx] = elmt, _hprintf_idx += 1), \
		int16_t: (buffer[_hprintf_idx] = elmt, _hprintf_idx += 1), \
		int32_t: (buffer[_hprintf_idx] = elmt, _hprintf_idx += 1), \
		int64_t: (buffer[_hprintf_idx] = ((uint64_t)(elmt) >> 0) & 0xffffffff, buffer[_hprintf_idx + 1] = ((uint64_t)(elmt) >> 32) & 0xffffffff, _hprintf_idx += 2), \
		uint8_t: (buffer[_hprintf_idx] = elmt, _hprintf_idx += 1), \
		uint16_t: (buffer[_hprintf_idx] = elmt, _hprintf_idx += 1), \
		uint32_t: (buffer[_hprintf_idx] = elmt, _hprintf_idx += 1), \
		uint64_t: (buffer[_hprintf_idx] = ((elmt) >> 0) & 0xffffffff, buffer[_hprintf_idx + 2] = ((elmt) >> 32) & 0xffffffff, _hprintf_idx += 2) \
	);

#define HPRINT_INTERNAL_SUM(buffer, ...) \
	HCC_PP_ARGS_FOREACH_SUM(HPRINT_INTERNAL_ELMT_WORD_COUNT, buffer, __VA_ARGS__) \

#define HPRINT_INTERNAL_PRINT(buffer, ...) \
	HCC_PP_ARGS_FOREACH(HPRINT_INTERNAL_PRINT_ELMT, buffer, __VA_ARGS__) \

//
// prints to HccWoBuffer(uint32_t) 'buffer' similar to how sprintf prints to a buffer.
//
// please read the docs in docs/intrinsics.md#hprintf
//
#define hprintf(buffer, fmt, ...) \
	{ \
		HccRwBuffer(uint32_t) _b = buffer; \
		uint32_t _hprintf_buffer_capacity = _b[HPRINTF_BUFFER_CAPACITY_IDX]; \
		uint32_t _hprintf_words_count = HPRINT_INTERNAL_SUM(_b, __VA_ARGS__) + HPRINT_STRING_SIZE(fmt); \
		uint32_t _hprintf_idx = atomic_add_u32(&_b[HPRINTF_BUFFER_CURSOR_IDX], _hprintf_words_count); \
		if (_hprintf_idx + _hprintf_words_count <= _hprintf_buffer_capacity) { \
			hprint_string(_b, _hprintf_idx, fmt); \
			_hprintf_idx += HPRINT_STRING_SIZE(fmt); \
			HPRINT_INTERNAL_PRINT(_b, __VA_ARGS__) \
		} else { \
			atomic_sub_u32(&_b[HPRINTF_BUFFER_CURSOR_IDX], _hprintf_words_count); \
		} \
	}

// ===========================================
//
//
// Shader System Values
//
//
// ===========================================

typedef struct HccVertexSV HccVertexSV;
struct HccVertexSV { // these are incomplete, please file an issue on the specific system values you need
	uint32_t vertex_idx;
	uint32_t instance_idx;
};

typedef struct HccVertexSVOut HccVertexSVOut;
struct HccVertexSVOut { // these are incomplete, please file an issue on the specific system values you need
	f32x4    position;
};

typedef struct HccPixelSV HccPixelSV;
struct HccPixelSV { // these are incomplete, please file an issue on the specific system values you need
	f32x4 pixel_coord;
};

typedef struct HccPixelSVOut HccPixelSVOut;
struct HccPixelSVOut { // these are incomplete, please file an issue on the specific system values you need
	float depth;
};

typedef struct HccComputeSV HccComputeSV;
struct HccComputeSV {
	u32x3    dispatch_idx;            // the overall dispatch index for this invocation, will be dispatch_group_idx * HCC_COMPUTE(x, y, z) + dispatch_local_idx
	u32x3    dispatch_group_idx;      // the current dispatch group this invocation is from, will be 0..Dispatch(x, y, z)
	u32x3    dispatch_local_idx;      // the local idx within the dispatch group for invocation, will be 0..HCC_COMPUTE(x, y, z)
	uint32_t dispatch_local_flat_idx; // the local idx within the dispatch group for invocation as a flat index
};

#include "hcc_texture_intrinsics.h"
#include "hcc_pixel_intrinsics.h"
#include "hcc_wave_intrinsics.h"
#include "hcc_atomic_intrinsics.h"

#endif // _HCC_SHADER_H_

