#pragma once
#ifndef _HCC_INTEROP_VULKAN_H_
#define _HCC_INTEROP_VULKAN_H_

#include "hcc_interop.h"

#ifndef HCC_INTEROP_VK_ASSERT
#define HCC_INTEROP_VK_ASSERT(expr) HCC_INTEROP_ASSERT((vk_result = (expr)) >= 0, "vulkan result error '%u' for '%s' at %s:%u", vk_result, #expr, __FILE__, __LINE__)
#endif

typedef uint8_t HccInteropVulkanDescriptorBinding;
enum HccInteropVulkanDescriptorBinding {
	HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_STORAGE_BUFFER, // HccRoBuffer or HccRwBuffer or HccWoBuffer
	HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_SAMPLED_IMAGE,  // HccSampledTexture
	HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_STORAGE_IMAGE,  // HccRoTexture, HccRwTexture, HccWoTexture
	HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_SAMPLER,        // HccSampler

	HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT,
};

typedef struct HccInteropVulkanSetup HccInteropVulkanSetup;
struct HccInteropVulkanSetup {
	VkDevice           device;
	VkDescriptorSet*   descriptor_sets;
	uint32_t           descriptor_sets_count;
	uint32_t           resource_descriptors_max;
	uint32_t           bundled_constants_size_max;
	VkShaderStageFlags shader_stages;
};

typedef struct HccInteropVulkan HccInteropVulkan;
struct HccInteropVulkan {
	VkDevice              device;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorPool      descriptor_pool;
	VkPipelineLayout      pipeline_layout;
	VkDescriptorSet*      descriptor_sets;
	uint32_t              descriptor_sets_count;
	VkShaderStageFlags    shader_stages;
};

extern VkDescriptorType hcc_interop_vulkan_descriptor_binding_descriptor_type[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT];

void hcc_interop_vulkan_init(HccInteropVulkan* interop, HccInteropVulkanSetup const* setup);
void hcc_interop_vulkan_descriptor_write_make(HccInteropVulkan* interop, VkWriteDescriptorSet* w, uint32_t descriptor_set_idx, HccInteropVulkanDescriptorBinding binding, uint32_t resource_idx);

#endif // _HCC_INTEROP_VULKAN_H_

