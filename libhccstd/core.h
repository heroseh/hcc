#pragma once
#ifndef _HCC_STD_CORE_H_
#define _HCC_STD_CORE_H_

#include "math_types.h"

// ===========================================
//
//
// Config
//
//
// ===========================================

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
#define HCC_COMPUTE __hcc_compute
#define HCC_MESHTASK __hcc_meshtask
#define HCC_MESH __hcc_mesh
#define HCC_RASTERIZER_STATE __hcc_rasterizer_state
#define HCC_NOINTERP __hcc_nointerp
#define HCC_FRAGMENT_STATE __hcc_fragment_state
#define HCC_INTERP
#else // !__HCC_GPU__
#define HCC_VERTEX
#define HCC_FRAGMENT
#define HCC_COMPUTE
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

#ifdef __HCC_GPU__
#define HccRoSampler __hcc_sampler
#define HccRoBuffer(T) __hcc_ro_buffer_t(T)
#define HccRoAnyBuffer __hcc_ro_any_buffer_t
#define HccRoTexture1D(T) __hcc_ro_texture_1d_t(T)
#define HccRoTexture1DArray(T) __hcc_ro_texture_1d_array_t(T)
#define HccRoTexture2D(T) __hcc_ro_texture_2d_t(T)
#define HccRoTexture2DArray(T) __hcc_ro_texture_2d_array_t(T)
#define HccRoTexture2DMS(T) __hcc_ro_texture_2d_t(T)
#define HccRoTexture2DMSArray(T) __hcc_ro_texture_2d_array_t(T)
#define HccRoTextureCube(T) __hcc_ro_texture_cube_t(T)
#define HccRoTextureCubeArray(T) __hcc_ro_texture_cube_array_t(T)
#define HccRoTextureCubeMS(T) __hcc_ro_texture_cube_ms_t(T)
#define HccRoTextureCubeMSArray(T) __hcc_ro_texture_cube_ms_array_t(T)
#define HccRoTexture3D(T) __hcc_ro_texture_3d_t(T)
#define HccRwBuffer(T) __hcc_rw_buffer_t(T)
#define HccRwAnyBuffer __hcc_rw_any_buffer_t
#define HccRwTexture1D(T) __hcc_rw_texture_1d_t(T)
#define HccRwTexture1DArray(T) __hcc_rw_texture_1d_array_t(T)
#define HccRwTexture2D(T) __hcc_rw_texture_2d_t(T)
#define HccRwTexture2DArray(T) __hcc_rw_texture_2d_array_t(T)
#define HccRwTexture2DMS(T) __hcc_rw_texture_2d_t(T)
#define HccRwTexture2DMSArray(T) __hcc_rw_texture_2d_array_t(T)
#define HccRwTextureCube(T) __hcc_rw_texture_cube_t(T)
#define HccRwTextureCubeArray(T) __hcc_rw_texture_cube_array_t(T)
#define HccRwTextureCubeMS(T) __hcc_rw_texture_cube_ms_t(T)
#define HccRwTextureCubeMSArray(T) __hcc_ro_texture_cube_ms_array_t(T)
#define HccRwTexture3D(T) __hcc_rw_texture_3d_t(T)
#else // !__HCC_GPU__
#define HccRoSampler uint32_t
#define HccRoBuffer(T) uint32_t
#define HccRoAnyBuffer uint32_t
#define HccRoTexture1D(T) uint32_t
#define HccRoTexture1DArray(T) uint32_t
#define HccRoTexture2D(T) uint32_t
#define HccRoTexture2DArray(T) uint32_t
#define HccRoTexture2DMS(T) uint32_t
#define HccRoTexture2DMSArray(T) uint32_t
#define HccRoTextureCube(T) uint32_t
#define HccRoTextureCubeArray(T) uint32_t
#define HccRoTextureCubeMS(T) uint32_t
#define HccRoTextureCubeMSArray(T) uint32_t
#define HccRoTexture3D(T) uint32_t
#define HccRwBuffer(T) uint32_t
#define HccRwAnyBuffer uint32_t
#define HccRwTexture1D(T) uint32_t
#define HccRwTexture1DArray(T) uint32_t
#define HccRwTexture2D(T) uint32_t
#define HccRwTexture2DArray(T) uint32_t
#define HccRwTexture2DMS(T) uint32_t
#define HccRwTexture2DMSArray(T) uint32_t
#define HccRwTextureCube(T) uint32_t
#define HccRwTextureCubeArray(T) uint32_t
#define HccRwTextureCubeMS(T) uint32_t
#define HccRwTextureCubeMSArray(T) uint32_t
#define HccRwTexture3D(T) uint32_t
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

#endif // _HCC_STD_CORE_H_

