#pragma once
#ifndef _HCC_STD_CORE_H_
#define _HCC_STD_CORE_H_

#include "math_types.h"

// ===========================================
//
//
// Options
//
//
// ===========================================

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

#define HCC_PP_ARGS_FOREACH_1(f, oarg, next_arg)       HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg)
#define HCC_PP_ARGS_FOREACH_2(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_1(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_3(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_2(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_4(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_3(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_5(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_4(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_6(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_5(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_7(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_6(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_8(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_7(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_9(f, oarg, next_arg, ...)  HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_8(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_10(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_9(f,  outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_11(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_10(f, outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_12(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_11(f, outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_13(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_12(f, outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_14(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_13(f, outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_15(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_14(f, outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH_16(f, oarg, next_arg, ...) HCC_PP_CALL(f, oarg, HCC_PP_UNTUPLE next_arg) HCC_PP_ARGS_FOREACH_15(f, outer_arg, __VA_ARGS__)

#define HCC_PP_ARGS_FOREACH_(f, outer_arg, M, ...) M(f, outer_arg, __VA_ARGS__)
#define HCC_PP_ARGS_FOREACH(f, outer_arg, ...) HCC_PP_ARGS_FOREACH_(f, outer_arg, HCC_CONCAT(HCC_PP_ARGS_FOREACH_, HCC_PP_ARGS_COUNT(__VA_ARGS__)), __VA_ARGS__)

// ===========================================
//
//
// Rasterizer State
//
//
// ===========================================

#ifdef __HCC_GPU__
#define HCC_RASTERIZER_STATE __hcc_rasterizer_state
#define HCC_POSITION __hcc_position
#define HCC_NOINTERP __hcc_nointerp
#define HCC_INTERP

#define _HCC_DEFINE_RASTERIZER_STATE_FIELD(StructName, KIND, DataType, name) \
	HCC_##KIND DataType name;

#define HCC_DEFINE_RASTERIZER_STATE(Name, ...) \
	typedef struct Name Name; \
	HCC_RASTERIZER_STATE struct Name { \
		HCC_PP_ARGS_FOREACH(_HCC_DEFINE_RASTERIZER_STATE_FIELD, Name, __VA_ARGS__) \
	}\

#else // !__HCC_GPU__

#define HCC_POSITION
#define HCC_NOINTERP
#define HCC_INTERP
#define HCC_RASTERIZER_STATE

typedef uint8_t HccRasterizerStateFieldKind;
enum {
	HCC_RASTERIZER_STATE_FIELD_KIND_POSITION,
	HCC_RASTERIZER_STATE_FIELD_KIND_INTERP,
	HCC_RASTERIZER_STATE_FIELD_KIND_NOINTERP,
};
typedef struct HccRasterizerStateField HccRasterizerStateField;
struct HccRasterizerStateField {
	const char*                 name;
	HccRasterizerStateFieldKind kind;
	uint16_t                    offset;
};

#define _HCC_DEFINE_METADATA_ENTRY(StructName, KIND, DataType, name) \
	{ .name = #name, .kind = HCC_RASTERIZER_STATE_FIELD_KIND_##KIND, .offset = offsetof(StructName, name), },

#define _HCC_DEFINE_STRUCT_FIELD(StructName, KIND, DataType, name) \
	DataType Name;

#define HCC_DEFINE_RASTERIZER_STATE_METADATA(Name, ...) \
	HccStateField hcc_rasterizer_state_fields_##Name[] = { \
		HCC_PP_ARGS_FOREACH(_HCC_DEFINE_METADATA_ENTRY, Name, __VA_ARGS__) \
	}

#define HCC_DEFINE_RASTERIZER_STATE_TYPE(Name, ...) \
	typedef struct Name Name; \
	struct Name { \
		HCC_PP_ARGS_FOREACH(HCC_DEFINE_STRUCT_FIELD, Name, __VA_ARGS__) \
	}

#define HCC_DEFINE_RASTERIZER_STATE(Name, ...) \
	HCC_DEFINE_RASTERIZER_STATE_METADATA(Name, __VA_ARGS__) \
	HCC_DEFINE_RASTERIZER_STATE_TYPE(Name, __VA_ARGS__)

#endif // !__HCC_GPU__

// ===========================================
//
//
// Fragment State
//
//
// ===========================================

#ifdef __HCC_GPU__
#define HCC_FRAGMENT_STATE __hcc_fragment_state
#else
#define HCC_FRAGMENT_STATE
#endif

#define _HCC_DEFINE_FRAGMENT_STATE_FIELD(StructName, DataType, name) \
	DataType name;

#define HCC_DEFINE_FRAGMENT_STATE(Name, ...) \
	typedef struct Name Name; \
	HCC_FRAGMENT_STATE struct Name { \
		HCC_PP_ARGS_FOREACH(_HCC_DEFINE_FRAGMENT_STATE_FIELD, Name, __VA_ARGS__) \
	}\

// ===========================================
//
//
// Resources - (Buffers, Textures, Samplers)
//
//
// ===========================================


#ifdef __HCC_GPU__

#define HCC_BUFFER_ELEMENT __hcc_buffer_element
#define HCC_RESOURCE_SET   __hcc_resource_set
#define HCC_RESOURCE_TABLE __hcc_resource_table
#define HCC_RESOURCES      __hcc_resources

#else // !__HCC_GPU__

typedef uint8_t HccSamplerFilter;
enum {
	HCC_SAMPLER_FILTER_NEAREST,
	HCC_SAMPLER_FILTER_LINEAR,
};

typedef uint8_t HccSamplerMipmapMode;
enum {
	HCC_SAMPLER_MIPMAP_MODE_NEAREST,
	HCC_SAMPLER_MIPMAP_MODE_LINEAR,
};

typedef uint8_t HccSamplerAddressMode;
enum {
    HCC_SAMPLER_ADDRESS_MODE_REPEAT,
    HCC_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    HCC_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    HCC_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    HCC_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

typedef uint8_t HccSamplerCompareOp;
enum {
    HCC_SAMPLER_COMPARE_OP_NEVER,
    HCC_SAMPLER_COMPARE_OP_LESS,
    HCC_SAMPLER_COMPARE_OP_EQUAL,
    HCC_SAMPLER_COMPARE_OP_LESS_OR_EQUAL,
    HCC_SAMPLER_COMPARE_OP_GREATER,
    HCC_SAMPLER_COMPARE_OP_NOT_EQUAL,
    HCC_SAMPLER_COMPARE_OP_GREATER_OR_EQUAL,
    HCC_SAMPLER_COMPARE_OP_ALWAYS,
};

typedef uint8_t HccSamplerBorderColor;
enum {
    HCC_SAMPLER_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
    HCC_SAMPLER_BORDER_COLOR_INT_TRANSPARENT_BLACK,
    HCC_SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
    HCC_SAMPLER_BORDER_COLOR_INT_OPAQUE_BLACK,
    HCC_SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
    HCC_SAMPLER_BORDER_COLOR_INT_OPAQUE_WHITE,
};

typedef struct HccSamplerState HccSamplerState;
struct HccSamplerState {
	HccSamplerFilter      mag_filter;
	HccSamplerFilter      min_filter;
	HccSamplerMipmapMode  mipmap_mode;
	HccSamplerAddressMode address_modeU;
	HccSamplerAddressMode address_modeV;
	HccSamplerAddressMode address_modeW;
	float                 mip_lod_bias;
	bool                  anisotropy_enable;
	float                 max_anisotropy;
	bool                  compare_enable;
	HccSamplerCompareOp   compare_op;
	float                 min_lod;
	float                 max_lod;
	HccSamplerBorderColor border_color;
};

typedef uint16_t HccTextureFormat;
enum {
	HCC_TEXTURE_FORMAT_R8G8B8A8_UNORM,
};

#define HCC_DEFINE_TEXTURE_TYPE(Name) \
	typedef struct Name Name; \
	struct Name { \
		void*            data; \
		HccTextureFormat format; \
	};

HCC_DEFINE_TEXTURE_TYPE(HccROTexelBuffer);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexelBuffer);
HCC_DEFINE_TEXTURE_TYPE(HccROTexture1D);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexture1D);
HCC_DEFINE_TEXTURE_TYPE(HccROTexture1DArray);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexture1DArray);
HCC_DEFINE_TEXTURE_TYPE(HccROTexture2D);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexture2D);
HCC_DEFINE_TEXTURE_TYPE(HccROTexture2DArray);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexture2DArray);
HCC_DEFINE_TEXTURE_TYPE(HccROTexture2DMS);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexture2DMS);
HCC_DEFINE_TEXTURE_TYPE(HccROTexture2DMSArray);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexture2DMSArray);
HCC_DEFINE_TEXTURE_TYPE(HccROTexture3D);
HCC_DEFINE_TEXTURE_TYPE(HccRWTexture3D);
HCC_DEFINE_TEXTURE_TYPE(HccROTextureCube);
HCC_DEFINE_TEXTURE_TYPE(HccRWTextureCube);
HCC_DEFINE_TEXTURE_TYPE(HccROTextureCubeArray);
HCC_DEFINE_TEXTURE_TYPE(HccRWTextureCubeArray);

#define ConstBuffer(T)        T
#define ROElementBuffer(T)    const T*
#define RWElementBuffer(T)    T*
#define ROByteBuffer          const uint8_t*
#define RWByteBuffer          uint8_t*
#define ROTexelBuffer(T)      HccTexelBuffer
#define RWTexelBuffer(T)      HccTexelBuffer
#define ROTexture1D(T)        HccTexture1D
#define RWTexture1D(T)        HccTexture1D
#define ROTexture1DArray(T)   HccTexture1DArray
#define RWTexture1DArray(T)   HccTexture1DArray
#define ROTexture2D(T)        HccTexture2D
#define RWTexture2D(T)        HccTexture2D
#define ROTexture2DArray(T)   HccTexture2DArray
#define RWTexture2DArray(T)   HccTexture2DArray
#define ROTexture2DMS(T)      HccTexture2DMS
#define RWTexture2DMS(T)      HccTexture2DMS
#define ROTexture2DMSArray(T) HccTexture2DMSArray
#define RWTexture2DMSArray(T) HccTexture2DMSArray
#define ROTexture3D(T)        HccTexture3D
#define RWTexture3D(T)        HccTexture3D
#define ROTextureCube(T)      HccTextureCube
#define RWTextureCube(T)      HccTextureCube
#define ROTextureCubeArray(T) HccTextureCubeArray
#define RWTextureCubeArray(T) HccTextureCubeArray
#define RWTextureCubeArray(T) HccTextureCubeArray
#define SamplerState          HccSamplerState

#endif // !__HCC_GPU__

//
// this structure cannot contain any resources.
// this structure cannot be used anywhere other than as a type of ConstBuffer, ROElementBuffer, RWElementBuffer
#define HCC_DEFINE_BUFFER_ELEMENT(Name, ...) \
	typedef struct Name Name; \
	HCC_BUFFER_ELEMENT struct Name { __VA__ARGS__ }

//
// in OpenGL this structure is inlined into all of the 'Resources' types that it is used in.
// in Vulkan this maps to a Descriptor Set.
// this structure cannot contain ResourceTable pointer.
// this structure cannot be used anywhere other than in the user-defined Resources structure.
#define HCC_DEFINE_RESOURCE_SET(Name, slot, ...) \
	typedef struct Name Name; \
	HCC_RESOURCE_SET(slot) struct Name { __VA__ARGS__ }

//
// this is only supported when using the 'bindless' mode in the compiler.
// each field can be a resource or a constant.
// these are allowed to be passed in as function parameters as they are the same data structure on all platforms.
// this structure can contain ResourceTable pointer.
// this structure cannot contain ConstBuffer.
// essentially the resource table is pulled from an internal ROByteBuffer.
// all resources will be a uint32_t in this ResourceTable
#define HCC_DEFINE_RESOURCE_TABLE(Name, ...) \
	typedef struct Name Name; \
	HCC_RESOURCE_TABLE struct Name { __VA__ARGS__ }

//
// this is the structure that you have passed into your shader entry point.
// this structure contains ResourceSet pointer, resources and constants
// this structure cannot be used anywhere other than as the shader entry point parameter.
//
// in Vulkan:
//     - in 'binding' mode:
//         - this maps to a Descriptor Set.
//     - in 'bindless' mode:
//         - this structure can contain ResourceTable pointer which will be a uint32_t constant.
//         - this structure cannot contain ConstBuffer.
//     - constants map to 'push constants' and will error when you overflow the limit specified at compile time.
//
// in OpenGL:
//     - constants map to 'uniform constants' and will error when you overflow the limit specified at compile time.
//
#define HCC_DEFINE_RESOURCES(Name, ...) \
	typedef struct Name Name; \
	HCC_RESOURCES struct Name { __VA__ARGS__ }

// ===========================================
//
//
// Shader Input
//
//
// ===========================================

typedef struct HccVertexInput HccVertexInput;
struct HccVertexInput {
	int32_t vertex_idx;
	int32_t instance_idx;
};

typedef struct HccFragmentInput HccFragmentInput;
struct HccFragmentInput {
	f32x4 frag_coord;
};

#endif // _HCC_STD_CORE_H_

