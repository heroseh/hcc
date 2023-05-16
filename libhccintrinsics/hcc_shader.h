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

