#include "hcc_interop_vulkan.h"

VkDescriptorType hcc_interop_vulkan_descriptor_binding_descriptor_type[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT] = {
	[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_STORAGE_BUFFER] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_SAMPLED_IMAGE] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_STORAGE_IMAGE] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_SAMPLER] = VK_DESCRIPTOR_TYPE_SAMPLER,
};

void hcc_interop_vulkan_init(HccInteropVulkan* interop, HccInteropVulkanSetup const* setup) {
	VkResult vk_result;
	interop->device = setup->device;
	interop->descriptor_sets_count = setup->descriptor_sets_count;
	interop->descriptor_sets = setup->descriptor_sets;
	interop->shader_stages = setup->shader_stages;

	{
		VkDescriptorBindingFlags bindless_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
		VkDescriptorSetLayoutBinding bindings[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT];
		VkDescriptorBindingFlags bindless_flags_array[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT];

		for (uint32_t binding = 0; binding < HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT; binding += 1) {
			bindless_flags_array[binding] = bindless_flags;

			VkDescriptorSetLayoutBinding* b = &bindings[binding];
			b->binding = binding;
			b->descriptorType = hcc_interop_vulkan_descriptor_binding_descriptor_type[binding];
			b->descriptorCount = setup->resource_descriptors_max;
			b->stageFlags = interop->shader_stages;
			b->pImmutableSamplers = 0;
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindless_create_info;
		bindless_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		bindless_create_info.pNext = NULL;
		bindless_create_info.bindingCount = HCC_INTEROP_ARRAY_COUNT(bindings);
		bindless_create_info.pBindingFlags = bindless_flags_array;

		VkDescriptorSetLayoutCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &bindless_create_info,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = HCC_INTEROP_ARRAY_COUNT(bindings),
			.pBindings = bindings,
		};

		HCC_INTEROP_VK_ASSERT(vkCreateDescriptorSetLayout(interop->device, &create_info, NULL, &interop->descriptor_set_layout));
	}

	{
		VkDescriptorPoolSize pool_sizes[HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT];
		for (uint32_t binding = 0; binding < HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_COUNT; binding += 1) {
			VkDescriptorPoolSize* p = &pool_sizes[binding];
			p->type = hcc_interop_vulkan_descriptor_binding_descriptor_type[binding];
			p->descriptorCount = setup->resource_descriptors_max * interop->descriptor_sets_count;
		}

		VkDescriptorPoolCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = interop->descriptor_sets_count,
			.poolSizeCount = HCC_INTEROP_ARRAY_COUNT(pool_sizes),
			.pPoolSizes = pool_sizes,
		};

		HCC_INTEROP_VK_ASSERT(vkCreateDescriptorPool(interop->device, &create_info, NULL, &interop->descriptor_pool));
	}

	for (uint32_t idx = 0; idx < interop->descriptor_sets_count; idx += 1) {
		VkDescriptorSetAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = NULL,
			.descriptorPool = interop->descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &interop->descriptor_set_layout
		};

		HCC_INTEROP_VK_ASSERT(vkAllocateDescriptorSets(interop->device, &alloc_info, &interop->descriptor_sets[idx]));
	}

	{
		VkPushConstantRange push_constant_range = {
			.stageFlags = interop->shader_stages,
			.offset = 0,
			.size = setup->bundled_constants_size_max,
		};

		VkPipelineLayoutCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.setLayoutCount = 1,
			.pSetLayouts = &interop->descriptor_set_layout,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &push_constant_range,
		};

		HCC_INTEROP_VK_ASSERT(vkCreatePipelineLayout(interop->device, &create_info, NULL, &interop->pipeline_layout));
	}
}

void hcc_interop_vulkan_descriptor_write_make(HccInteropVulkan* interop, VkWriteDescriptorSet* w, uint32_t descriptor_set_idx, HccInteropVulkanDescriptorBinding binding, uint32_t resource_idx) {
	w->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	w->pNext = NULL,
	w->dstSet = interop->descriptor_sets[descriptor_set_idx],
	w->dstBinding = binding,
	w->dstArrayElement = resource_idx,
	w->descriptorCount = 1,
	w->descriptorType = hcc_interop_vulkan_descriptor_binding_descriptor_type[binding],
	w->pImageInfo = NULL;
	w->pBufferInfo = NULL;
	w->pTexelBufferView = NULL;
}

