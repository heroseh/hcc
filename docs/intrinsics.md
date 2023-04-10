# HCC Intrinsics Docs

- [Bindless Resources](#bindless-resources)
- [Bundled Constants](#bundled-constants)
- [System Values](#system-values)
- [Rasterizer State](#rasterizer-state)
- [Fragment State](#fragment-state)
- [Global Variables](#global-variables)
- [8bit, 16bit, 64bit integer & float support](#8bit-16bit-64bit-integer--float-support)
- [Vector & Matrix Maths](#vector--matrix-maths)
- [Atomics](#atomics)
- [Fragment](#fragment)
- [Textures](#textures)
- [Quad & Wave](#quad--wave)
- [Libc](#libc)

## Bindless Resources

All Hcc\*Buffer, Hcc\*Texture & HccRoSampler resources are fully bindless in HCC. This means that each resource is a 32bit unsigned integer underneath and can be uploaded as is to the GPU via [Bundled Constants](#bundled-constants) or in the contents of a Hcc\*Buffer itself. This also allows you to pass them around in the shader with no limiations, including into function arguments, writing them out in to buffers or writing in [Rasterizer State](#rasterizer-state).

Please refer to the [engine integration docs](integrating_into_your_engine.md#bindless-resources) to learn how Bindless Resources hook up CPU side.

Here is a list of all of the Resource Types available in HCC:
- HccRoSampler

- Buffer Types: where T is the element type
	- HccRoBuffer(T)
	- HccWoBuffer(T)
	- HccRwBuffer(T)

- Texture Types: where T is the element type
	- HccRoTexture1D(T)
	- HccRoTexture1DArray(T)
	- HccRoTexture2D(T)
	- HccRoTexture2DArray(T)
	- HccRoTexture2DMS(T)
	- HccRoTexture2DMSArray(T)
	- HccRoTexture3D(T)
	- HccWoTexture1D(T)
	- HccWoTexture1DArray(T)
	- HccWoTexture2D(T)
	- HccWoTexture2DArray(T)
	- HccWoTexture2DMS(T)
	- HccWoTexture2DMSArray(T)
	- HccWoTexture3D(T)
	- HccRwTexture1D(T)
	- HccRwTexture1DArray(T)
	- HccRwTexture2D(T)
	- HccRwTexture2DArray(T)
	- HccRwTexture2DMS(T)
	- HccRwTexture2DMSArray(T)
	- HccRwTexture3D(T)
	- HccSampleTexture1D(T)
	- HccSampleTexture1DArray(T)
	- HccSampleTexture2D(T)
	- HccSampleTexture2DArray(T)
	- HccSampleTextureCube(T)
	- HccSampleTextureCubeArray(T)
	- HccSampleTexture3D(T)

## Bundled Constants

Bundled Constants are the initial way you get data to your shader. They are just declared as a plain old data structure and are usual small and are limited by the target hardware, the size constrained is controlled via the CPU.

Please refer to the [engine integration docs](integrating_into_your_engine.md#bundled-constants) to learn how this works CPU side.

When writing GPU side code, Bundled Constants are passed into the vertex & fragment shader via the 3rd argument and for the compute shader it is the 2nd argument.
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
HCC_FRAGMENT void fragment_shader(
    HccFragmentSV const* const sv,
    HccFragmentSVOut* const sv_out,
    BundledConstants const* const bc, // passed in here
    RS const* const state,
    Frag* const frag_out
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
HCC_FRAGMENT void fragment_shader(
    HccFragmentSV const* const sv,  // input system values
    HccFragmentSVOut* const sv_out, // output system values
    BC const* const bc,
    RS const* const state,
    Frag* const frag_out
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
- HccFragmentSV
- HccFragmentSVOut
- HccComputeSV

## Rasterizer State

Like other shading languages, HCC allows you to pass state from the vertex shader to the fragment shader that can be interpolated from state that is at the vertices to the fragment.

Unlike other shading languages, this is a custom structure that is passed in as the 4th parameter to the vertex & fragment shader entry points. You must declare this structure with a `HCC_RASTERIZER_STATE` specifier like so:
```c
typedef struct RasterizerState RasterizerState;
HCC_RASTERIZER_STATE struct RasterizerState {
    HCC_NOINTERP HccRoBuffer(uint32_t) buffer;
    HCC_INTERP   f32x2                 uv;
    HCC_INTERP   f32x4                 color;
};
```
Each field is limited to being an intrinsic data type (scalar or vector) or a resource (buffer, texture or sampler). Each field must be explicitly prefixed with either `HCC_INTERP` or `HCC_NOINTERP` to decide if you wish the value to be interpolated or not from the vertex shader to the fragment shader.

Then you may use this structure in the vertex & fragment entry points like so:
```c
HCC_VERTEX void vertex_shader(
    HccVertexSV const* const sv,
    HccVertexSVOut* const sv_out,
    BC const* const bc,
    RasterizerState* const state_out    // output per vertex here
);
HCC_FRAGMENT void fragment_shader(
    HccFragmentSV const* const sv,
    HccFragmentSVOut* const sv_out,
    BC const* const bc,
    RasterizerState const* const state, // input per fragment here
    Frag* const frag_out
);
```

Please see the [samples](release_package.md#sample-application) folder for a more in-depth example.

The benefit to this approach, is that you'll be able to:
- reuse the same structure in multiple shaders
- the structure will not go out of sync between vertex & fragment shaders
- help remove the limitation of declaring multiple shaders in a single file

## Fragment State

Like other shading languages, HCC allows you to output a fragment from the fragment shader to a single or multiple render targets.

Unlike other shading languages, this is a custom structure that is passed in as the 5th parameter to the fragment shader entry point. You must declare this structure with a `HCC_FRAGMENT_STATE` specifier like so:
```c
typedef struct Fragment Fragment;
HCC_FRAGMENT_STATE struct Fragment {
	f32x4 albedo;   // render target slot 0
	f32x4 emissive; // render target slot 1
};
```
Each field is limited to being a intrinsic data type (scalar or vector). Each field represents a render target and they are assigned their slot index internally using the index of the field. In this example `albedo` will be slot 0 and `emissive` will be slot 1.

Then you may use this structure in the fragment entry points like so:
```c
HCC_FRAGMENT void fragment_shader(
    HccFragmentSV const* const sv,
    HccFragmentSVOut* const sv_out,
    BC const* const bc,
    RS const* const state,
    Fragment* const frag_out // output here per fragment
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

## Fragment

Like other shading languages, HCC has special functions that can only be used in a fragment shader. Such as discarding a pixel with the `discard_fragment()` function and calculating the partial derivative using functions like `ddxG` and `ddyG`.

You can find the functions and their documentation in [hcc_fragment_intrinsics.h](../libhccintrinsics/hcc_fragment_intrinsics.h)

## Textures

Like other shading languages, HCC has special functions that read/write to/from texture resources.

You can find the functions and their documentation in [hcc_texture_intrinsics.h](../libhccintrinsics/hcc_texture_intrinsics.h)

## Quad & Wave

Like other shading languages, HCC has special functions that communicate and operate on groups of threads that execute in lockstep know as a `Quad` and `Wave`.

A `Quad` is a 2x2 block of 4 threads that are executing in lockstep and are part of a larger `Wave`.

A `Wave` is a group of threads executing in lockstep. This varies across hardware but is typically 32 or 64 threads.

You can find the functions and their documentation in [hcc_wave_intrinsics.h](../libhccintrinsics/hcc_wave_intrinsics.h)

## Libc

Because HCC brings C to the GPU which is a rather unusual programming environment. It has it's own limited version of libc which you can find along side the compiler executable in the `libc` folder. You don't have to do anything special to use it when invoking the compile.

