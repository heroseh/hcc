
#include "../libc-gpu/math.h"
#include "../libhccstd/core.h"
#include "../libhccstd/math.h"

#if 0

/////////////////////////////////////////
//
// OpenGL
//
/////////////////////////////////////////
- binding slots eg. 0 - 1024
- you bind slots indivially
- each shader stage has max number of bound textures or buffers
layout(location = 0) uniform texture u_texture; // pesudo-GLSL code

/////////////////////////////////////////
//
// Vulkan - Binding Approach
//
/////////////////////////////////////////
- binding sets eg. 0 - 1024
- binding slots eg. 0 - 1024
- you bind a 'set' of 'slots'
- each shader stage has max number of bound textures or buffers
layout(set = 0, location = 0) uniform buffer { ... } u_frame_constants; // pesudo-GLSL code
layout(set = 1, location = 0) uniform texture u_texture; // pesudo-GLSL code

/////////////////////////////////////////
//
// Vulkan - Bindless
//
/////////////////////////////////////////
- binding sets eg. 0 - 1024
- binding slots eg. 0 - 1024
- you bind a 'set' of 'slots'
- each shader stage has max number of bound textures or buffers
layout(set = 0, location = 0) uniform buffer { ... } u_frame_constants; // pesudo-GLSL code
layout(set = 1, location = 0) uniform bytebuffer u_all_buffers; // pesudo-GLSL code
layout(set = 1, location = 1) uniform texture u_all_textures[1024]; // pesudo-GLSL code
layout(push_constants) push_constants {
	uint texture_idx;
};


ConstBuffer(T)       // uniform buffer/constant buffer - 16K or 64K no indexing on type T
ROBuffer(T)          // storage buffer / structured buffer / byte address buffer - big data buffer allow indexing on type T
RWBuffer(T)          // storage buffer / structured buffer / byte address buffer - big data buffer allow indexing on type T
ROTexture1D
ROTexture1DArray
ROTexture2D
ROTexture2DArray
ROTexture2DMS
ROTexture2DMSArray
ROTextureCube
ROTextureCubeArray
ROTextureCubeMS
ROTextureCubeMSArray
ROTexture3D
RWTexture1D(T)
RWTexture1DArray(T)
RWTexture2D(T)
RWTexture2DArray(T)
RWTexture2DMS(T)
RWTexture2DMSArray(T)
RWTextureCube(T)
RWTextureCubeArray(T)
RWTextureCubeMS(T)
RWTextureCubeMSArray(T)
RWTexture3D(T)
SamplerState

HCC_DEFINE_BUFFER_ELEMENT(FrameConstants,
	uint32_t frame_idx;
	float    time_ms;
};

HCC_DEFINE_BUFFER_ELEMENT(SceneConstants,
	mat4x4 proj_from_view;
	mat4x4 proj_from_world;
});

HCC_DEFINE_RESOURCE_SET(FrameResourceSet, 0,
	ConstBuffer(FrameConstants) constants;
);

HCC_DEFINE_RESOURCE_SET(SceneResourceSet, 1,
	ConstBuffer(SceneConstants) constants;
);

HCC_DEFINE_RESOURCE_TABLE(AnotherSharedResourceTable,
	ROByteBuffer all_the_data;
);

HCC_DEFINE_RESOURCE_TABLE(SharedResourceTable, // local resource sets can be shared between multiple draw calls
	ROTexture2D(vec4)                 all_the_colours;
	ROTexture2D(float)                noise;
	const AnotherSharedResourceTable* another_shared_resources; // but you pay a cost in another layer of indirection
	uint32_t                          loose_constant;
);

HCC_DEFINE_RESOURCES(BillboardResources,
	const FrameResourceSet*    frame;
	const SceneResourceSet*    scene;
	const SharedResourceTable* shared;  // this is a uint32_t byte_offset in bindless and not supported in the binding approaches
	ConstBuffer                const_buffer;  // this is not supported in bindless, will produce a binding which we can optional error on.
	ROBuffer                   buffers[4];    // if using bindless these will be uint32_t buffer_indices[4] in push constants
	uint32_t                   loose_constant; // these map to push constants, if you overflow them it will error.
);

#endif

HCC_DEFINE_RASTERIZER_STATE(
	JohnboardRasterizerState,
	(POSITION, vec4f32, position),
	(INTERP,   vec4f32, color),
	(INTERP,   vec2f32, snorm)
);

vertex JohnboardRasterizerState johnboard_shader_vertex(const HccVertexInput input, const JohnboardResources resources) {
	JohnboardRasterizerState state;
	vec2f32 unorm = v2f32(input.vertex_idx & 1, input.vertex_idx / 2);
	vec2f32 snorm = subsv2f32(mulsv2f32(unorm, 2.f), 1.f);
	state.position = v4f32(snorm.x, snorm.y, 0.25f, 1.f);
	state.color = v4f32(unorm.x, unorm.y, 0.f, 1.f);
	state.snorm = snorm;
	return state;
}

HCC_DEFINE_FRAGMENT_STATE(
	JohnboardFragment,
	(vec4f32, color)
);

float circle_distance(vec2f32 pt, float radius) {
	return lenv2f32(pt) - radius;
}

fragment JohnboardFragment johnboard_shader_fragment(
	const HccFragmentInput input,
	const JohnboardRasterizerState state,
	const JohnboardResources resources
) {
	float distance = circle_distance(state.snorm, 0.75f);

	JohnboardFragment frag;
	frag.color = distance < 0.f ? state.color : ZEROV4F32;
	return frag;
}

