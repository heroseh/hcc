
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
Buffer(const T)      // storage buffer / structured buffer / byte address buffer - big data buffer allow indexing on type T
Buffer(T)            // storage buffer / structured buffer / byte address buffer - big data buffer allow indexing on type T
Texture1D
Texture1DArray
Texture2D
Texture2DArray
Texture2DMS
Texture2DMSArray
TextureCube
TextureCubeArray
TextureCubeMS
TextureCubeMSArray
Texture3D
Texture1D(const T)
Texture1DArray(const T)
Texture2D(const T)
Texture2DArray(const T)
Texture2DMS(const T)
Texture2DMSArray(const T)
TextureCube(const T)
TextureCubeArray(const T)
TextureCubeMS(const T)
TextureCubeMSArray(const T)
Texture3D(const T)
Texture1D(T)
Texture1DArray(T)
Texture2D(T)
Texture2DArray(T)
Texture2DMS(T)
Texture2DMSArray(T)
TextureCube(T)
TextureCubeArray(T)
TextureCubeMS(T)
TextureCubeMSArray(T)
Texture3D(T)
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

RoBuffer(T)      // storage buffer / structured buffer - allow indexing on type T
RwBuffer(T)      // storage buffer / structured buffer - allow indexing on type T
RoAnyBuffer      // byte addres buffer - allow loading any type T at a byte offset
RwAnyBuffer      // byte addres buffer - allow loading any type T at a byte offset
RoTexture1D
RoTexture1DArray
RoTexture2D
RoTexture2DArray
RoTexture2DMS
RoTexture2DMSArray
RoTextureCube
RoTextureCubeArray
RoTextureCubeMS
RoTextureCubeMSArray
RoTexture3D
RwTexture1D(T)
RwTexture1DArray(T)
RwTexture2D(T)
RwTexture2DArray(T)
RwTexture2DMS(T)
RwTexture2DMSArray(T)
RwTextureCube(T)
RwTextureCubeArray(T)
RwTextureCubeMS(T)
RwTextureCubeMSArray(T)
RwTexture3D(T)
RoSampler

/////////////////////////////////////////////////////////////////
//
// Texture functions
//
// texture functions context aware print formating
// $tt - texture type (snake case)
// $tT - texture type (camel case)
// $ti - texture index type
// $tc - texture coord type
// $dx - data type suffix
// $di - data type identifer
// $vi - vector type identifier
//

// foreach ro/rw texture type
//     foreach data type
//         foreach vector type
$vi load_$tt_$vx($tT texture, $ti idx);

// foreach ro/rw texture type
//     foreach data type
//         foreach vector type
$vi sample_$tt_$vx($tT texture, RoSampler sampler, $tc coord);

// foreach ro/rw texture type
//     foreach data type
//         foreach vector type
$vi sample_mip_bias_$tt_$vx($tT texture, RoSampler sampler, $tc coord, float mip_bias);

// foreach ro/rw texture type
//     foreach data type
//         foreach vector type
$vi sample_gradient_$tt_$vx($tT texture, RoSampler sampler, $tc coord, float ddx, float ddy);

// foreach ro/rw texture type
//     foreach data type
//         foreach vector type
$vi sample_level_$tt_$vx($tT texture, RoSampler sampler, $tc coord, float lod);

// foreach ro/rw texture type
//     foreach scalar data type
$vi gather_red_$tt_$dx($tT texture, RoSampler sampler, $tc coord);

// foreach ro/rw texture type
//     foreach scalar data type
$vi gather_green_$tt_$dx($tT texture, RoSampler sampler, $tc coord);

// foreach ro/rw texture type
//     foreach scalar data type
$vi gather_blue_$tt_$dx($tT texture, RoSampler sampler, $tc coord);

// foreach ro/rw texture type
//     foreach scalar data type
$vi gather_alpha_$tt_$dx($tT texture, RoSampler sampler, $tc coord);

// foreach RW texture type
//     foreach data type
//         foreach vector type
void store_$tt_$dx($tT($di) texture, $ti idx, $di value);

//
// Texture functions
//
/////////////////////////////////////////////////////////////////

struct Vertex {
	f32x4 position;
	f32x2 uv;
};

#ifndef __HCC__
#define RoBuffer(T) uint32_t
#define RoAnyBuffer uint32_t
#define RoTexture1D uint32_t
#define RoTexture1DArray uint32_t
#define RoTexture2D uint32_t
#define RoTexture2DArray uint32_t
#define RoTexture2DMS uint32_t
#define RoTexture2DMSArray uint32_t
#define RoTextureCube uint32_t
#define RoTextureCubeArray uint32_t
#define RoTextureCubeMS uint32_t
#define RoTextureCubeMSArray uint32_t
#define RoTexture3D uint32_t
#define RoSampler uint32_t
#define RwBuffer(T) uint32_t
#define RwAnyBuffer uint32_t
#define RwTexture1D(T) uint32_t
#define RwTexture1DArray(T) uint32_t
#define RwTexture2D(T) uint32_t
#define RwTexture2DArray(T) uint32_t
#define RwTexture2DMS(T) uint32_t
#define RwTexture2DMSArray(T) uint32_t
#define RwTextureCube(T) uint32_t
#define RwTextureCubeArray(T) uint32_t
#define RwTextureCubeMS(T) uint32_t
#define RwTextureCubeMSArray(T) uint32_t
#define RwTexture3D(T) uint32_t
#endif

//
// bundled constants are vulkan's push constants and dx12's root constants.
// you will tell the compiler your maximum size of bundled constants you want.
// you can upload resources or and other constant data that you want.
// each resource is 4 bytes as it is just an index.
// in future when resources are no longer bindless indices and move to pointers
// or passing direct descriptors, this size may change
struct TriangleBC {
	RoBuffer(Vertex) vertices;
	RwBuffer(Vertex) vertices_out;
	RoAnyBuffer any_buffer;
	uint32_t num_vertices;
};

HCC_DEFINE_RASTERIZER_STATE(
	RasterizerState,
	(POSITION, f32x4, position),
	(INTERP,   f32x4, color)
);

#ifdef __HCC__
__hcc_generic_t load_any_buffer(__hcc_type T, RoAnyBuffer any_buffer, uint32_t byte_offset);
void store_any_buffer(RoAnyBuffer any_buffer, uint32_t byte_offset, __hcc_generic_t value);
#else
#define load_any_buffer(T, any_buffer, byte_offset) (*(const T*)(const char*)any_buffer + byte_offset)
#define store_any_buffer(any_buffer, byte_offset, value) ((*(T*)(char*)any_buffer + byte_offset) = value)
#endif

vertex void vs(HccVertexSV const sv, TriangleBC const* const bc, RasterizerState* const state_out) {
	RoBuffer(Vertex) vertices = bc->vertices;
	const Vertex* vertex = &vertices[sv.vertex_idx];        // fetch pointer but is logical so must be known at compile time when it is dereferenced
	Vertex vertex = vertices[sv.vertex_idx];                // load full struct from buffer directly
	Vertex vertex = *vertex;                                // dereference logical pointer
	f32x4 position = vertex->position;                      // field access using logical pointer
	f32x4 position = vertices[sv.vertex_idx].position;      // field access from buffer directly
	float position_x = vertices[sv.vertex_idx].position.x;  // deeper field access from buffer directly
	const Vertex* vertex = vertices;                        // also implicitly casts into pointer

	RwBuffer(Vertex) vertices_out = bc->vertices_out;

	//
	// physical pointer support adds:
	const Vertex* vertices_ptr = &vertices[sv.vertex_idx];  // fetching pointer but this one can be passed around and used like a regular C pointer
	const Vertex* vertex = &vertices_ptr[sv.vertex_idx];    // array index using pointer into buffer and can be passed around
	f32x4 position = *(f32x4*)vertices_ptr;                 // cast pointer and dereference as a different type

	//
	// this is basically HLSL's ByteAddressBuffer but will be removed once most "modern" GPU's support physical pointers.
	// this can be used to load any type T that only consists of 4 byte scalar types from a byte offset that is 4 byte aligned.
	RoAnyBuffer any_buffer = bc->any_buffer;
	Vertex vertex = load_any_buffer(Vertex, any_buffer, sv.vertex_idx * sizeof(Vertex));
	store_any_buffer(Vertex, any_buffer, sv.vertex_idx * sizeof(Vertex), vertex);

	f32x2 vertices[3] = {
		f32x2(-0.5f, -0.5f),
		f32x2(0.f, 0.5f),
		f32x2(0.5f, -0.5f),
	};

	f32x4 colors[3] = {
		f32x4(1.f, 0.f, 0.f, 1.f),
		f32x4(0.f, 1.f, 0.f, 1.f),
		f32x4(0.f, 0.f, 1.f, 1.f),
	};

	state_out->position = f32x4(vertices[sv.vertex_idx].x, vertices[sv.vertex_idx].y, 0.f, 1.f);
	state_out->color = colors[sv.vertex_idx];
}

HCC_DEFINE_FRAGMENT_STATE(
	Fragment,
	(f32x4, color)
);

fragment void fs(HccFragmentSV const sv, RasterizerState const* const state, Fragment* const frag_out) {
	frag_out->color = state->color;
}

