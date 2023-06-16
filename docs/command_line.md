# Command Line Docs

The **hcc** compiler has a very simple command line interface (CLI) and behaves differently to other C compilers.

- [-fi \<path\>.c](#-fi-pathc)
- [-fo \<path\>.spirv](#-fo-pathspirv)
- [-fomc \<path\>.h](#-fomc-pathh)
- [-I \<path\>](#-i-path)
- [-O](#-o)
- [--hlsl-packing](#--hlsl-packing)
- [--hlsl \<path\>](#--hlsl-path)
- [--msl \<path\>](#--msl-path)
- [--max-descriptors](#--max-descriptors-num)
- [--max-bc-size](#--max-bc-size-num)
- [--enable-int8](#--enable-int8)
- [--enable-int16](#--enable-int16)
- [--enable-int64](#--enable-int64)
- [--enable-float16](#--enable-float16)
- [--enable-float64](#--enable-float64)
- [--enable-unordered-swizzling](#--enable-unordered-swizzling)
- [--debug-time](#--debug-time)
- [--debug-ata](#--debug-ata)
- [--debug-ast](#--debug-ast)
- [--debug-aml](#--debug-aml)

# Differences with other C Compilers:
- Multiple input files compiled into a single output binary [More Info](#-fi-pathc)
- Optimization is handled by spirv-opt ([More Info](#-o))
	- Note: the compiler is architected in a way to support doing optimizations internally but is something that has to be worked on
- Optionally transpiles to [HLSL](#--hlsl-path) and [MSL](#--msl-path) using spirv-cross
- Type-safe aware linking
	- Linking is done at the AST level so will error if function prototypes or global variable data type for a symbol do not match

# Command Line Options

## -fi \<path\>.c
Use this flag to add another input file to the compilation, **-fi** must be followed by a path that has a **.c** file extension. You may have as many of these input files as you like.

```
hcc -fi libengine_shader_shared.c -fi game_shader_shared.c -fi game_shaders.c ...
```

## -fo \<path\>.spirv
Use this flag to specify the output file of the compilation, **-fo** must be followed by a path that has a **.spirv** file extension.

```
hcc -fi game_shaders.c -fo game_shaders.spirv
```

## -fomc \<path\>.h
Use this flag to specify the output file for a shader metadata C header file, **-fomc** must be followed by a path that has a **.h** file extension.

```
hcc -fi game_shaders.c -fo game_shaders.spirv -fomc game_shaders_metadata.h
```

The shader metadata includes the following information:
- HccShader enumeration that has a enum value for each shader compiled with hcc
- array of struct HccResourceStruct. Which is reflection information for all Bundled Constants structures that are passed into all shader entry points.
	- this can be used:
		- to automate sync code
		- in higher level authoring tools to set shader inputs

This can be expanded on in the future, please let me know if you have any ideas!

## -I \<path\>
Use this flag to add a directory that will be searched for header files when using the `#include <...>` directive in your code files, **-I** must be followed by a path that is a directory

```
hcc -I libengine/include -fi game_shaders.c -fo game_shaders.spirv
```

## -O
Use this flag to enable optimizations on your SPIR-V code. spirv-opt will be used at the end of compilation, so make sure you have spirv-opt installed. This does not have to be done now, you can manually use spirv-opt on the SPIR-V created by the compiler later.

```
hcc -fi game_shaders.c -fo game_shaders.spirv -O
```

## --hlsl-packing
Use this flag to enable errors for when HLSL packing has been violated for Bundled Constants. Use this when you want to ensure that your shaders will port over to HLSL nicely when you later export them with the `--hlsl` option. HCC by default has scalar alignment everywhere and this is not compatible with HLSL Constant Buffer's at this time.

See the [HLSL Packing Rules](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules)

```
hcc -fi game_shaders.c -fo game_shaders.spirv --hlsl-packing
```

## --hlsl \<path\>
Use this flag to specify a directory to output the HLSL code designed for use with DX12 that is transpiled from the output SPIR-V file. spirv-cross is used at the end of compilation, so make sure you have spirv-cross installed. You can then use another compiler to compile HLSL into DXIL. Hopefully in the future we will output directly to DXIL and make it 1 compiler step instead of 3 and hopefully output a single file instead. If you would like to help make that happen, please open up a conversion and lets chat!

```
hcc -fi game_shaders.c -fo game_shaders.spirv --hlsl game_shaders_hlsl
```

## --msl \<path\>
Use this flag to specify a directory to output the MSL code designed for use with Metal that is transpiled from the output SPIR-V file. spirv-cross is used at the end of compilation, so make sure you have spirv-cross installed. You can then use the MSL files directly in Metal. Hopefully in the future we will output directly to MSL and make it 1 compiler step instead of 2. If you would like to help make that happen, please open up a conversion and lets chat!

```
hcc -fi game_shaders.c -fo game_shaders.spirv --msl game_shaders_msl
```

## --max-descriptors \<num\>
Use this flags to specify the maximum number of resource descriptor that can be used across all shaders. this must be compiled into your shaders & must match the CPU side when you setup your graphics API. see the [engine integration docs](integrating_into_your_engine.md#bindless-resources)

## --max-bc-size \<num\>
Use this flags to specify the maximum size of bundled constants that are passed into every shader. this must match the CPU side when you setup your graphics API. see the [engine integration docs](integrating_into_your_engine.md#bundled-constants)

## --enable-int8
Adds support for 8bit integer types that can be used fully throughout your code and will place the necessary SPIR-V capability feature flags in the final binary.

## --enable-int16
Adds support for 16bit integer types that can be used fully throughout your code and will place the necessary SPIR-V capability feature flags in the final binary.

## --enable-int64
Adds support for 64bit integer types that can be used fully throughout your code and will place the necessary SPIR-V capability feature flags in the final binary.

## --enable-float16
Adds support for 16bit float type `__hcc_half_t` that can be used fully throughout your code and will place the necessary SPIR-V capability feature flags in the final binary.

## --enable-float64
Adds support for 64bit float type `double` that can be used fully throughout your code and will place the necessary SPIR-V capability feature flags in the final binary.

## --enable-unordered-swizzling
allows for vector swizzling x, y, z, w, r, g, b, a out of order or repeat eg. .zyx or .xx or .bga or .yyzz, warning: this is not compatible with standard C

## --debug-time
Use this flag to show a detailed view of how long each stage of the compiler took to compile your shaders. This will be useful information to help see where the problems are in compilation for developers of HCC but also in your build pipeline.

```
hcc -fi game_shaders.c -fo game_shaders.spirv --debug-time
```

## --debug-ata
Use this flag to show a detailed view of the Abstract Token Array generation that the compiler generated and uses in compilation. This will be useful for developers of HCC to help debug issues with the compiler.

WARNING: if you use this flag compiler will stop after the ATA stage and will **not** fully complete the compilation of your file.

```
hcc -fi game_shaders.c -fo game_shaders.spirv --debug-ata > hcc_ata_log.txt
```

## --debug-ast
Use this flag to show a detailed view of the Abstract Syntax Tree generation that the compiler generated and uses in compilation. This will be useful for developers of HCC to help debug issues with the compiler.

WARNING: if you use this flag compiler will stop after the AST stage and will **not** fully complete the compilation of your file.

```
hcc -fi game_shaders.c -fo game_shaders.spirv --debug-ast > hcc_ast_log.txt
```

## --debug-aml
Use this flag to show a detailed view of the Abstract Machine Language generation that the compiler generated and uses in compilation. This will be useful for developers of HCC to help debug issues with the compiler.

WARNING: if you use this flag compiler will stop after the AML stage and will **not** fully complete the compilation of your file.

```
hcc -fi game_shaders.c -fo game_shaders.spirv --debug-aml > hcc_aml.txt
```

