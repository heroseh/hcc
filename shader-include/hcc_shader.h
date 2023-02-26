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
#define HCC_FRAGMENT __hcc_fragment
#define HCC_COMPUTE(x, y, z) __hcc_compute(x, y, z)
#define HCC_MESHTASK __hcc_meshtask
#define HCC_MESH __hcc_mesh
#define HCC_RASTERIZER_STATE __hcc_rasterizer_state
#define HCC_NOINTERP __hcc_nointerp
#define HCC_FRAGMENT_STATE __hcc_fragment_state
#define HCC_INTERP
#else // !__HCC_GPU__
#define HCC_VERTEX
#define HCC_FRAGMENT
#define HCC_COMPUTE(x, y, z)
#define HCC_MESHTASK
#define HCC_MESH
#define HCC_RASTERIZER_STATE
#define HCC_NOINTERP
#define HCC_FRAGMENT_STATE
#define HCC_INTERP
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

// ===========================================
//
//
// Shader System Values
//
//
// ===========================================

typedef struct HccVertexSV HccVertexSV;
struct HccVertexSV {
	int32_t vertex_idx;
	int32_t instance_idx;
};

typedef struct HccVertexSVOut HccVertexSVOut;
struct HccVertexSVOut {
	f32x4    position;
};

typedef struct HccFragmentSV HccFragmentSV;
struct HccFragmentSV {
	f32x4 frag_coord;
};

typedef struct HccFragmentSVOut HccFragmentSVOut;
struct HccFragmentSVOut {
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
#include "hcc_fragment_intrinsics.h"
#include "hcc_wave_intrinsics.h"
#include "hcc_atomic_intrinsics.h"

#endif // _HCC_SHADER_H_

