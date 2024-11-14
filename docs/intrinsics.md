# HCC Intrinsics Docs

- [Bindless Resources](#bindless-resources)
- [Bundled Constants](#bundled-constants)
- [System Values](#system-values)
- [Rasterizer State](#rasterizer-state)
- [Pixel State](#pixel-state)
- [Global Variables](#global-variables)
- [bool](#bool)
- [8bit, 16bit, 64bit integer & float support](#8bit-16bit-64bit-integer--float-support)
- [Vector & Matrix Maths](#vector--matrix-maths)
- [Atomics](#atomics)
- [Pixel](#pixel)
- [Textures](#textures)
- [Texture Formats](#texture-formats)
- [Quad & Wave](#quad--wave)
- [Swizzling](#swizzling)
- [hprintf](#hprintf)
- [Libc](#libc)

## Bindless Resources

All Hcc\*Buffer, Hcc\*Texture & HccRoSampler resources are fully bindless in HCC. This means that each resource is a 32bit unsigned integer underneath and can be uploaded as is to the GPU via [Bundled Constants](#bundled-constants) or in the contents of a Hcc\*Buffer itself. This also allows you to pass them around in the shader with no limiations, including into function arguments, writing them out in to buffers or writing in [Rasterizer State](#rasterizer-state).

You can even explicitly cast between a `uint32_t` and **any** resource type. Here are some examples of how this is can be leveraged:
1. You can pack extra information into a `uint32_t` that is masked out before casting into a resource
2. The resource could be a constant integer that you simply cast into the resource
3. Your resource id is calculated on the GPU
```c
// 1. packing extra info
uint32_t texture_and_array_layer = buffer[0].texture_and_array_layer;
HccSampleTexture2DArray(f32x4) tex = (HccSampleTexture2DArray(f32x4))(texture_and_array_layer & 0xffffff);
uint32_t array_layer = (texture_and_array_layer >> 24) & 0xff;
f32x4 texel = sample_textureG(tex, sampler, u32x3(uv.x, uv.y, array_layer));

// 2. resource constant
HccSampleTexture2D(f32x4) noise_tex = (HccSampleTexture2D(f32x4))GAME_RES_ID_NOISE_TEXTURE;
f32x4 texel = sample_textureG(noise_tex, sampler, uv);

// 3. calculated resource id
HccSampleTexture2D(f32x4) tex = (HccSampleTexture2D(f32x4))(buffer[0].base_res_id + slot_idx);
f32x4 texel = sample_textureG(tex, sampler, uv);
```

Please refer to the [engine integration docs](integrating_into_your_engine.md#bindless-resources) to learn how Bindless Resources hook up CPU side.

Here is a list of all of the Resource Types available in HCC:
- HccRoSampler

- Buffer Types: where T is the element type
	- HccRoBuffer(T)
	- HccWoBuffer(T)
	- HccRwBuffer(T)

- Sample Texture Types: where T is the element type
	- HccSampleTexture1D(T)
	- HccSampleTexture1DArray(T)
	- HccSampleTexture2D(T)
	- HccSampleTexture2DArray(T)
	- HccSampleTextureCube(T)
	- HccSampleTextureCubeArray(T)
	- HccSampleTexture3D(T)

- Storage Texture Types: where FMT is the [texture format](#texture-formats)
	- HccRoTexture1D(FMT)
	- HccRoTexture1DArray(FMT)
	- HccRoTexture2D(FMT)
	- HccRoTexture2DArray(FMT)
	- HccRoTexture2DMS(FMT)
	- HccRoTexture2DMSArray(FMT)
	- HccRoTexture3D(FMT)
	- HccWoTexture1D(FMT)
	- HccWoTexture1DArray(FMT)
	- HccWoTexture2D(FMT)
	- HccWoTexture2DArray(FMT)
	- HccWoTexture2DMS(FMT)
	- HccWoTexture2DMSArray(FMT)
	- HccWoTexture3D(FMT)
	- HccRwTexture1D(FMT)
	- HccRwTexture1DArray(FMT)
	- HccRwTexture2D(FMT)
	- HccRwTexture2DArray(FMT)
	- HccRwTexture2DMS(FMT)
	- HccRwTexture2DMSArray(FMT)
	- HccRwTexture3D(FMT)

## Bundled Constants

Bundled Constants are the initial way you get data to your shader. They are just declared as a plain old data structure and are usual small and are limited by the target hardware, the size constrained is controlled via the CPU.

Please refer to the [engine integration docs](integrating_into_your_engine.md#bundled-constants) to learn how this works CPU side.

When writing GPU side code, Bundled Constants are passed into the vertex & pixel shader via the 3rd argument and for the compute shader it is the 2nd argument.
Below is a minimal code example of this, but missing the usage code inside of functions. Please see the [samples](release_package.md#sample-application) folder for a more in-depth example.
```c
#include <hcc_shader.h>

// defined in some CPU-GPU shared header somewhere
typedef struct Vertex Vertex;
struct Vertex {
    f32x2    pos;
    uint32_t color;
};

// defined in some CPU-GPU shared header somewhere
typedef struct BundledConstants BundledConstants;
struct BundledConstants {
    HccRoBuffer(Vertex) vertices;
    f32x4               tint;
};

HCC_VERTEX void vertex_shader(
    HccVertexSV const* const sv,
    HccVertexSVOut* const sv_out,
    BundledConstants const* const bc, // passed in here
    RS* const state_out
);
HCC_PIXEL void pixel_shader(
    HccPixelSV const* const sv,
    HccPixelSVOut* const sv_out,
    BundledConstants const* const bc, // passed in here
    RS const* const state,
    Pixel* const pixel_out
);
HCC_COMPUTE(8, 8, 1) void compute_shader(
    HccComputeSV const* const sv,
    BundledConstants const* const bc // passed in here
);
```

## System Values

Like other shading languages, HCC exposes built-in input and output variables via a construct know as 'System Values'.

Unlike other shading languages, you'll find the **input** System Values as the first parameter and potentially **output** System values as the second parameter in shader entry points. The parameter's type is a built-in structure to the compiler exposed in userland in the [hcc_shader.h](../libhccintrinsics/hcc_shader.h) file

Here are the shader entry point function prototypes:
```c
HCC_VERTEX void vertex_shader(
    HccVertexSV const* const sv,    // input system values
    HccVertexSVOut* const sv_out,   // output system values
    BC const* const bc,
    RS* const state_out
);
HCC_PIXEL void pixel_shader(
    HccPixelSV const* const sv,  // input system values
    HccPixelSVOut* const sv_out, // output system values
    BC const* const bc,
    RS const* const state,
    Pixel* const pixel_out
);
HCC_COMPUTE(8, 8, 1) void compute_shader(
    HccComputeSV const* const sv,   // input system values
    BC const* const bc
);
```

Please see the [samples](release_package.md#sample-application) folder for a more in-depth example.

For more info check out the documentation in [hcc_shader.h](../libhccintrinsics/hcc_shader.h) for:
- HccVertexSV
- HccVertexSVOut
- HccPixelSV
- HccPixelSVOut
- HccComputeSV

## Rasterizer State

Like other shading languages, HCC allows you to pass state from the vertex shader to the pixel shader that can be interpolated from state that is at the vertices to the pixel.

Unlike other shading languages, this is a custom structure that is passed in as the 4th parameter to the vertex & pixel shader entry points. You must declare this structure with a `HCC_RASTERIZER_STATE` specifier like so:
```c
typedef struct RasterizerState RasterizerState;
HCC_RASTERIZER_STATE struct RasterizerState {
    HCC_NOINTERP HccRoBuffer(uint32_t) buffer;
    HCC_INTERP   f32x2                 uv;
    HCC_INTERP   f32x4                 color;
};
```
Each field is limited to being an intrinsic data type (scalar or vector) or a resource (buffer, texture or sampler). Each field must be explicitly prefixed with either `HCC_INTERP` or `HCC_NOINTERP` to decide if you wish the value to be interpolated or not from the vertex shader to the pixel shader.

Then you may use this structure in the vertex & pixel entry points like so:
```c
HCC_VERTEX void vertex_shader(
    HccVertexSV const* const sv,
    HccVertexSVOut* const sv_out,
    BC const* const bc,
    RasterizerState* const state_out    // output per vertex here
);
HCC_PIXEL void pixel_shader(
    HccPixelSV const* const sv,
    HccPixelSVOut* const sv_out,
    BC const* const bc,
    RasterizerState const* const state, // input per pixel here
    Pixel* const pixel_out
);
```

Please see the [samples](release_package.md#sample-application) folder for a more in-depth example.

The benefit to this approach, is that you'll be able to:
- reuse the same structure in multiple shaders
- the structure will not go out of sync between vertex & pixel shaders
- help remove the limitation of declaring multiple shaders in a single file

## Pixel State

Like other shading languages, HCC allows you to output a pixel from the pixel shader to a single or multiple render targets.

Unlike other shading languages, this is a custom structure that is passed in as the 5th parameter to the pixel shader entry point. You must declare this structure with a `HCC_PIXEL_STATE` specifier like so:
```c
typedef struct Pixel Pixel;
HCC_PIXEL_STATE struct Pixel {
	f32x4 albedo;   // render target slot 0
	f32x4 emissive; // render target slot 1
};
```
Each field is limited to being a intrinsic data type (scalar or vector). Each field represents a render target and they are assigned their slot index internally using the index of the field. In this example `albedo` will be slot 0 and `emissive` will be slot 1.

Then you may use this structure in the pixel entry points like so:
```c
HCC_PIXEL void pixel_shader(
    HccPixelSV const* const sv,
    HccPixelSVOut* const sv_out,
    BC const* const bc,
    RS const* const state,
    Pixel* const pixel_out // output here per pixel
);
```

Please see the [samples](release_package.md#sample-application) folder for a more in-depth example.

The benefit to this approach, is that you'll be able to:
- reuse the same structure in multiple shaders
- help remove the limitation of declaring multiple shaders in a single file

## Global Variables

Like other shading languages, HCC allows you to declare global variables that can be used in your shaders. Unfortunately GPUs don't allow mutable `static` globals variables like C does, so `static` is not allowed in HCC unless it is `const`. Although we do not have support for mutable `static` (aka. A kind shared memory across all threads), we do have Hcc\*Buffer resources that can be accessed across dispatch groups in compute & across different shaders entirely.

Instead HCC supports the following:
### `static`
`static` must be `const` and is the same value across all invocations.

### `_Thread_local`
`_Thread_local` has the lifetime and scope of your invocation from the shader entry point. It can be explicit initialized or will be implicit initialized to zero.

### `HCC_DISPATCH_GROUP`
`HCC_DISPATCH_GROUP` has the lifetime and scope of your dispatch group and shared between all waves executing the dispatch group. However it cannot be initialized and memory is uninitialized, so careful!

## bool

C's builtin `bool` data type aka `_Bool` is implemented as an `int` by default since byte sized type support is optional. To have compatibility with CPU code, avoid putting `bool`s in `struct`s and just use them within as function return, parameters and variables.

## 8bit, 16bit, 64bit integer & float support

By default only bool, int32, uint32 & float are supported. If you try and use anything else it will error. This is because support for other sized integers and floats varies across hardware. But you can enable support for them.

Using the following command line arguments:
- [--enable-int8](command_line.md#--enable-int8)
- [--enable-int16](command_line.md#--enable-int16)
- [--enable-int64](command_line.md#--enable-int64)
- [--enable-float16](command_line.md#--enable-float16)
- [--enable-float64](command_line.md#--enable-float64)

## Vector & Matrix Maths

HCC comes with a maths library know as `hmaths`, you can find it in the `hmaths` folder in the release package. It comes with a wide range of helpful functions that operate on scalar, vector and matrix types. Even with C11 `_Generic` support to bring overloaded maths functions to C. For more information, please check out the documentation over in the source code.

Currently `hmaths` only works for GPU but that is only because i haven't had the time to implement the functionality on the CPU yet! But that is planned for a future release. If you would like to speed up the process, please get in touch if you would like to help with that.

The maths functions in `hmaths` will compile down to the instructions that exist in the SPIR-V instruction set.

Although it is very valuable to use `hmaths`, you do not have to use `hmaths` if you have your own maths library already. HCC has a couple of built-in types `__hcc_half_t` and `__hcc_vector_t` that you can use in your own maths library with `#if defined(__HCC__)`. The intrinsic functions that compile down to SPIR-V can be replicated in your maths library too but it might not matter that much if compilers that take the SPIR-V can vectorize your code in an optimization pass.

## Atomics

Like other shading languages, HCC allows for atomic operations (thread-safe writes) in buffers and [HCC_DISPATCH_GROUP](#hcc_dispatch_group) global variables. Control barriers and memory barriers can be used to synchronize and make atomic writes visible from changes in other dispatch groups.

You can find the functions and their documentation in [hcc_atomic_intrinsics.h](../libhccintrinsics/hcc_atomic_intrinsics.h)

## Pixel

Like other shading languages, HCC has special functions that can only be used in a pixel shader. Such as discarding a pixel with the `discard_pixel()` function and calculating the partial derivative using functions like `ddxG` and `ddyG`.

You can find the functions and their documentation in [hcc_pixel_intrinsics.h](../libhccintrinsics/hcc_pixel_intrinsics.h)

## Textures

Like other shading languages, HCC has special functions that read/write to/from texture resources.

You can find the functions and their documentation in [hcc_texture_intrinsics.h](../libhccintrinsics/hcc_texture_intrinsics.h)

There are Storage Textures `HccRoTexture*(FMT)`, `HccWoTexture*(FMT)`, `HccRwTexture*(FMT)`. These allow you to load, store and atomic load & store to a texture.
These take a [texture-format](#texture-formats) as the `FMT` type argument so the load/store data type can be inferred from it and so the shader code will
be compatible with a wider range of GPUs.

There are Sample Textures `HccSampleTexture*(T)`. These allow you to do all the usual sample operations and usually require an explicit `HccRoSampler` to be passed in along side.
These take a sample type as the `T` type argument so you can choose the return type of the sample functions.

This code samples a pixel and writes it back out to the same location.
```
HccRoSampler clamp_point_sampler = bc->clamp_point_sampler;
HccSampleTexture2D(f32x4) sample_texture = bc->sample_texture;
f32x4 color = sample_textureG(sample_texture, clamp_point_sampler, uv);

HccWoTexture2D(FMT_8_8_8_8_UNORM) storage_texture = bc->storage_texture;
u32x2 coord = u32x2(uv.x * bc->texture_dims.x, uv.y * bc->texture_dims.y);
store_textureG(storage_texture, coord, color);
```

## Texture Formats

Texture format are required for Storage Textures as not all GPU's support reading/writing without an explicit format.

The [Vulkan Environment for SPIR-V](https://docs.vulkan.org/spec/latest/appendices/spirvenv.html#spirvenv-image-formats) has a compatibility list.

Here that table adjusted with HCC's identifiers:

| HCC Formats | Vulkan Formats |
| ----------- | -------------- |
| FMT_8_8_8_8_UNORM | VK_FORMAT_R8G8B8A8_UNORM |
| FMT_8_8_8_8_SNORM | VK_FORMAT_R8G8B8A8_SNORM |
| FMT_8_8_8_8_UINT | VK_FORMAT_R8G8B8A8_UINT |
| FMT_8_8_8_8_SINT | VK_FORMAT_R8G8B8A8_SINT |
| FMT_8_8_UNORM | VK_FORMAT_R8G8_UNORM |
| FMT_8_8_SNORM | VK_FORMAT_R8G8_SNORM |
| FMT_8_8_UINT | VK_FORMAT_R8G8_UINT |
| FMT_8_8_SINT | VK_FORMAT_R8G8_SINT |
| FMT_8_UNORM | VK_FORMAT_R8_UNORM |
| FMT_8_SNORM | VK_FORMAT_R8_SNORM |
| FMT_8_UINT | VK_FORMAT_R8_UINT |
| FMT_8_SINT | VK_FORMAT_R8_SINT |
| FMT_16_16_16_16_FLOAT | VK_FORMAT_R16G16B16A16_SFLOAT |
| FMT_16_16_16_16_UNORM | VK_FORMAT_R16G16B16A16_UNORM |
| FMT_16_16_16_16_SNORM | VK_FORMAT_R16G16B16A16_SNORM |
| FMT_16_16_16_16_UINT | VK_FORMAT_R16G16B16A16_UINT |
| FMT_16_16_16_16_SINT | VK_FORMAT_R16G16B16A16_SINT |
| FMT_16_16_FLOAT | VK_FORMAT_R16G16_SFLOAT |
| FMT_16_16_UNORM | VK_FORMAT_R16G16_UNORM |
| FMT_16_16_SNORM | VK_FORMAT_R16G16_SNORM |
| FMT_16_16_UINT | VK_FORMAT_R16G16_UINT |
| FMT_16_16_SINT | VK_FORMAT_R16G16_SINT |
| FMT_16_FLOAT | VK_FORMAT_R16_SFLOAT |
| FMT_16_UNORM | VK_FORMAT_R16_UNORM |
| FMT_16_SNORM | VK_FORMAT_R16_SNORM |
| FMT_16_UINT | VK_FORMAT_R16_UINT |
| FMT_16_SINT | VK_FORMAT_R16_SINT |
| FMT_32_32_32_32_FLOAT | VK_FORMAT_R32G32B32A32_SFLOAT |
| FMT_32_32_32_32_UINT | VK_FORMAT_R32G32B32A32_UINT |
| FMT_32_32_32_32_SINT | VK_FORMAT_R32G32B32A32_SINT |
| FMT_32_32_FLOAT | VK_FORMAT_R32G32_SFLOAT |
| FMT_32_32_UINT | VK_FORMAT_R32G32_UINT |
| FMT_32_32_SINT | VK_FORMAT_R32G32_SINT |
| FMT_32_FLOAT | VK_FORMAT_R32_SFLOAT |
| FMT_32_UINT | VK_FORMAT_R32_UINT |
| FMT_32_SINT | VK_FORMAT_R32_SINT |
| FMT_64_UINT | VK_FORMAT_R64_UINT |
| FMT_64_SINT | VK_FORMAT_R64_SINT |
| FMT_11_11_10_FLOAT | VK_FORMAT_B10G11R11_UFLOAT_PACK32 |
| FMT_10_10_10_2_UNORM | VK_FORMAT_A2B10G10R10_UNORM_PACK32 |
| FMT_10_10_10_2_UINT | VK_FORMAT_A2B10G10R10_UINT_PACK32 |

## Quad & Wave

Like other shading languages, HCC has special functions that communicate and operate on groups of threads that execute in lockstep know as a `Quad` and `Wave`.

A `Quad` is a 2x2 block of 4 threads that are executing in lockstep and are part of a larger `Wave`.

A `Wave` is a group of threads executing in lockstep. This varies across hardware but is typically 32 or 64 threads.

You can find the functions and their documentation in [hcc_wave_intrinsics.h](../libhccintrinsics/hcc_wave_intrinsics.h)

## Swizzling

Like other shading languages, HCC supports swizzling even though it is not a feature in C.

eg:
```c
u32x4 vec = u32x4(1, 2, 3, 4);
u32x3 xyz = vec.xyz;
u32x3 rgb = vec.rgb;
u32x2 xx = vec.xx;
u32x2 zy = vec.zy;
vec.zy = u32x2(5, 6);
```

By default, HCC exposed ordered types of swizzling so it can be compatible with vectors declared in C and work with other C compilers.

like so:
```c
struct f32x4 {
	union {
		struct { float x; float y; float z; float w; };
		struct { float r; float g; float b; float a; };
		struct { f32x2 xy; f32x2 zw; };
		struct { f32x2 rg; f32x2 ba; };
		f32x3 xyz;
		struct { float _0; f32x3 yzw; };
		f32x3 rgb;
		struct { float _1; f32x3 gba; };
		struct { float _2; f32x2 yz; float _3; };
		struct { float _4; f32x2 gb; float _5; };
	};
};
```

You can also have the more powerful version of swizzling where the components can be unorderd and repeating. To do so use the [--enable-unordered-swizzling](command_line.md#--enable-unordered-swizzling) command line argument.

## hprintf

HCC comes with a way to print from a shader into a `HccWoBuffer(uint32_t)` in a very similar way to how `sprintf` works. Later this buffer can be read from the CPU side and displayed in a terminal.

Example of calling `hprintf` from a pixel shader:
```c
hprintf(bc->hprintf_buffer, "pixel_coord: %f, %f and test: %u, %u, %u, %u\n", splat2(sv->pixel_coord), splat4(test.vec));
```

Printing from a shader will copy the format string and the `uint32_t` words of the variable length arguments into the buffer at an insert index that is atomically calculated. So it will not overlap with any other shader invocations.

This can be used in any shader, across all threads. just be aware that with millions of threads, its easy to fill up a buffer. So use if statements to help minimize the amount of data you are outputting when you don't need it.

Please refer to the [engine integration docs](integrating_into_your_engine.md#hprintf) to learn how hprintf hooks up CPU side.

## Libc

Because HCC brings C to the GPU which is a rather unusual programming environment. It has it's own limited version of libc which you can find along side the compiler executable in the `libc` folder. You don't have to do anything special to use it when invoking the compile.

