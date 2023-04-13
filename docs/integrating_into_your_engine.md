# Integrating Into Your Engine Docs

## Interop

HCC comes with an interop layer that helps integrate HCC into your engine. Please find this in the `interop` folder of the release package.

Currently the interop layer helps:
- define metadata structures and enums that are output by the compiler via the [-fomc](command_line.md#--fomc-pathh) argument
- Vulkan
	- setup VkDescriptorSetLayout
	- setup VkDescriptorPool
	- setup VkDescriptorSet's
	- setup VkPipelineLayout

## Bindless Resources

In HCC all resources are fully bindless. This means that each resource is a 32bit unsigned integer underneath and can be uploaded as is to the GPU via [Bundled Constants](#bundled-constants) or in the contents of a buffer itself.

Please refer to the [intrinsics docs](intrinsics.md#bindless-resources) to learn how Bindless Resources are used GPU side.

### Vulkan

For Vulkan we require that the descriptor set's are setup in a particular way. You can find an easy to use interop layer that sets up your descriptors for you in [interop/hcc_interop_vulkan.h](../interop/hcc_interop_vulkan.h). You can find usage of the interop layer in the [samples](../samples/app/gpu_vulkan.c) and [playground](../playground/app/gpu_vulkan.c) app source code.

HCC requires that `descriptor set 0` is fully bindless and laid out like so:
```
binding = 0, type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptors_count = <max-descriptors>
binding = 1, type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  descriptors_count = <max-descriptors>
binding = 2, type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  descriptors_count = <max-descriptors>
binding = 3, type = VK_DESCRIPTOR_TYPE_SAMPLER,        descriptors_count = <max-descriptors>
```

\<max-descriptors\> must be compiled into the shaders and is configurable by the hcc command line using the [--max-descriptors](command_line.md#--max-descriptors-num) argument. This number is also output into the shader metadata using the [-fomc](command_line.md#--fomc-pathh) argument. You'll find it in the `HccMetadata.resource_descriptors_max` field declared in [interop/hcc_interop.h](../interop/hcc_interop.h).

| Hcc Resource Type              | Vulkan Descriptor Type            |
| ------------------------------ | --------------------------------- |
| HccRoSampler                   | VK_DESCRIPTOR_TYPE_SAMPLER        |
| HccRoBuffer(T)                 | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER |
| HccWoBuffer(T)                 | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER |
| HccRwBuffer(T)                 | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER |
| HccRoTexture1D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRoTexture1DArray(T)         | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRoTexture2D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRoTexture2DArray(T)         | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRoTexture2DMS(T)            | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRoTexture2DMSArray(T)       | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRoTexture3D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccWoTexture1D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccWoTexture1DArray(T)         | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccWoTexture2D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccWoTexture2DArray(T)         | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccWoTexture2DMS(T)            | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccWoTexture2DMSArray(T)       | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccWoTexture3D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRwTexture1D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRwTexture1DArray(T)         | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRwTexture2D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRwTexture2DArray(T)         | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRwTexture2DMS(T)            | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRwTexture2DMSArray(T)       | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccRwTexture3D(T)              | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  |
| HccSampleTexture1D(T)          | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE  |
| HccSampleTexture1DArray(T)     | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE  |
| HccSampleTexture2D(T)          | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE  |
| HccSampleTexture2DArray(T)     | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE  |
| HccSampleTextureCube(T)        | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE  |
| HccSampleTextureCubeArray(T)   | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE  |
| HccSampleTexture3D(T)          | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE  |

## Bundled Constants

Bundled constants map to Vulkan's Push Constants, DX12's Root Constants & Metal's Set\*Bytes().
This is the initial way you get data to your shader.
The structure is easily shared between the CPU & GPU, you just simply upload the structure's data directly into the command buffer using your graphics API.

Here are the functions that are used to upload bundled constants to shaders:
- Vulkan
	- [vkCmdPushConstants](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCmdPushConstants.html)
- DX12
	- [ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstants](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-setgraphicsroot32bitconstants)
	- [ID3D12GraphicsCommandList::SetComputeRoot32BitConstants](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-setcomputeroot32bitconstants)
- Metal
	- [MTLRenderCommandEncoder/setVertexBytes](https://developer.apple.com/documentation/metal/mtlrendercommandencoder/1515846-setvertexbytes)
	- [MTLRenderCommandEncoder/setFragmentBytes](https://developer.apple.com/documentation/metal/mtlrendercommandencoder/1516192-setfragmentbytes)
	- [MTLComputeCommandEncoder/setBytes](https://developer.apple.com/documentation/metal/mtlcomputecommandencoder/1443159-setbytes)

Please refer to the [intrinsics docs](intrinsics.md#bundled-constants) to learn how Bundled Constants are used GPU side.

Below is a minimal code sample of how this could be done CPU:
```c
#include <hcc_shader.h>

// defined in some CPU-GPU shared header somewhere
typedef struct Vertex Vertex;
struct Vertex {
    f32x2    pos;
    uint32_t color;
};

// defined in some CPU-GPU shared header somewhere
typedef struct TriangleBC TriangleBC;
struct TriangleBC {
    HccRoBuffer(Vertex) vertices;
    f32x4               tint;
};

void gpu_vk_draw( // emits draw command into command buffer
	VkPipeline vk_pipeline,
	uint32_t vertices_count,
	void* bundled_constants,
	uint32_t bundled_constants_size,
) {
	VkCommandBuffer vk_command_buffer = g_gpu.command_buffer;

	vkCmdBindPipeline(
		vk_command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		vk_pipeline);

	vkCmdPushConstants(
		vk_command_buffer,
		g_gpu.pipeline_layout,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		0, bundled_constants_size, bundled_constants);

	vkCmdDraw(vk_command_buffer, vertices_count, 1, 0, 0);
}

void gpu_render() { // renders scene
	VkPipeline pipeline = get_pipeline(SHADER_TRIANGLE);
	TriangleBC bc = {
		.vertices = get_vertex_buffer(),
		.tint = f32x4(1.f, 0.8f, 1.f, 1.f)
	};
	gpu_vk_draw(pipeline, 3, &bc, sizeof(bc));
}
```
