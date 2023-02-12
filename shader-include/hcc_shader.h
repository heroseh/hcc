#pragma once
#ifndef _HCC_SHADER_H_
#define _HCC_SHADER_H_

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

#ifndef __HCC_GPU__
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

#endif // _HCC_SHADER_H_

