#pragma once
#define _HCC_STD_H_

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

#ifdef __HCC_GPU__
#define HCC_INTRINSIC __hcc_intrinsic

#define HCC_STATE_STRUCT __hcc_state_struct
#define HCC_POSITION __hcc_position
#define HCC_NOINTERP __hcc_nointerp
#define HCC_INTERP

#define _HCC_DEFINE_STATE_STRUCT_FIELD(StructName, KIND, DataType, name) \
	HCC_##KIND DataType name;

#define HCC_DEFINE_STATE(Name, ...) \
	typedef struct Name Name; \
	struct Name { \
		HCC_PP_ARGS_FOREACH(_HCC_DEFINE_STATE_STRUCT_FIELD, Name, __VA_ARGS__) \
	}\

#else

#define HCC_POSITION
#define HCC_NOINTERP
#define HCC_INTERP
#define HCC_STATE_STRUCT

typedef U8 HccStateFieldKind;
enum {
	HCC_STATE_FIELD_KIND_POSITION,
	HCC_STATE_FIELD_KIND_INTERP,
	HCC_STATE_FIELD_KIND_NOINTERP,
};
typedef struct HccStateField HccStateField;
struct HccStateField {
	const char* name;
	HccStateFieldKind kind;
	U16 offset;
};

#define _HCC_DEFINE_METADATA_ENTRY(StructName, KIND, DataType, name) \
	{ .name = #name, .kind = HCC_STATE_FIELD_KIND_##KIND, .offset = offsetof(StructName, name), },

#define _HCC_DEFINE_STRUCT_FIELD(StructName, KIND, DataType, name) \
	DataType Name;

#define HCC_DEFINE_STATE_METADATA(Name, ...) \
	HccStateField hcc_state_fields_##Name[] = { \
		HCC_PP_ARGS_FOREACH(_HCC_DEFINE_METADATA_ENTRY, Name, __VA_ARGS__) \
	}

#define HCC_DEFINE_STATE_STRUCT(Name, ...) \
	typedef struct Name Name; \
	struct Name { \
		HCC_PP_ARGS_FOREACH(HCC_DEFINE_STRUCT_FIELD, Name, __VA_ARGS__) \
	}

#define HCC_DEFINE_STATE(Name, ...) \
	HCC_DEFINE_STATE_METADATA(Name, __VA_ARGS__) \
	HCC_DEFINE_STATE_STRUCT(Name, __VA_ARGS__)

#endif

HCC_INTRINSIC typedef struct HccVertexInput HccVertexInput;
HCC_INTRINSIC struct HccVertexInput {
	int32_t vertex_idx;
	int32_t instance_idx;
};

HCC_INTRINSIC typedef struct HccFragmentInput HccFragmentInput;
HCC_INTRINSIC struct HccFragmentInput {
	vec2_t frag_coord;
};

