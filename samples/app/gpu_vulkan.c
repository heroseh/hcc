#include <vulkan/vulkan_core.h>

#define APP_VK_ASSERT(expr) APP_ASSERT((vk_result = (expr)) >= VK_SUCCESS, "vulkan error '%s' returned from: %s at %s:%u", app_vk_result_string(vk_result), #expr, __FILE__, __LINE__)

#ifdef __linux__
#include <vulkan/vulkan_xlib.h>
#elif defined(_WIN32)
#include <vulkan/vulkan_win32.h>
#else
#error "unsupported platform"
#endif

#include <hcc_interop_vulkan.h>
#include <hcc_interop_vulkan.c>

// each platform should have format that it wants the swapchain image to be
#ifdef __linux__
#define GPU_VK_SURFACE_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#elif defined(_WIN32)
#define GPU_VK_SURFACE_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#else
#error "unsupported platform"
#endif

#define GPU_VK_DEBUG 0

typedef struct GpuVkResource GpuVkResource;
struct GpuVkResource {
	VkBuffer       buffer;
	VkImage        image;
	VkImageView    image_view;
	VkSampler      sampler;
	VkDeviceMemory device_memory;
	uint32_t       device_memory_size;
	void*          device_memory_mapped;
};

#define GPU_VK_STAGED_UPLOADS_CAP 16

typedef struct GpuVkStagedUpload GpuVkStagedUpload;
struct GpuVkStagedUpload {
	VkImage  image;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t mip_level;
	uint32_t buffer_offset;
};

typedef struct GpuVk GpuVk;
struct GpuVk {
	AppSampleEnum         sample_enum;
	uint32_t              queue_family_idx;
	uint32_t              swapchain_width;
	uint32_t              swapchain_height;

	VkInstance            instance;
	VkPhysicalDevice      physical_device;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkDevice              device;
	VkQueue               queue;
	VkSurfaceKHR          surface;
	VkSwapchainKHR        swapchain;
	VkImage*              swapchain_images;
	VkImageView*          swapchain_image_views;
	uint32_t              swapchain_images_count;
	VkImage               depth_image;
	VkImageView           depth_image_view;
	VkDeviceMemory        depth_image_memory;
	VkShaderStageFlags    push_constants_stage_flags;
	VkShaderModule        shader_module;
	GpuResourceId         staging_buffer;
	uint32_t              staging_buffer_size;
	uint32_t              staging_buffer_cap;
	GpuVkStagedUpload     staged_uploads[GPU_VK_STAGED_UPLOADS_CAP];
	uint32_t              staged_uploads_count;

	uint32_t              next_resource_id;
	GpuVkResource         resources[128];

	VkCommandPool         command_pool;
	VkCommandBuffer       command_buffers[APP_FRAMES_IN_FLIGHT];
	VkFence               fences[APP_FRAMES_IN_FLIGHT];
	VkSemaphore           swapchain_image_ready_semaphore;
	VkSemaphore           swapchain_present_ready_semaphore;
	uint32_t              frame_idx;

	VkDescriptorSet       descriptor_sets[APP_FRAMES_IN_FLIGHT];
	HccInteropVulkan      interop;
};

typedef struct GpuVkSample GpuVkSample;
struct GpuVkSample {
	VkPipeline            pipeline;
};

static GpuVkSample gpu_samples[APP_SAMPLE_COUNT];

GpuVk gpu;

const char* app_vk_result_string(VkResult result) {
	switch (result) {
		case VK_SUCCESS: return "VK_SUCCESS";
		case VK_NOT_READY: return "VK_NOT_READY";
		case VK_TIMEOUT: return "VK_TIMEOUT";
		case VK_EVENT_SET: return "VK_EVENT_SET";
		case VK_EVENT_RESET: return "VK_EVENT_RESET";
		case VK_INCOMPLETE: return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
		case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
		case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
		default: return "??????";
	}
}

void gpu_vk_recreate_swapchain_and_friends(uint32_t window_width, uint32_t window_height) {
	VkResult vk_result;
	{
		if (gpu.swapchain_images) {
			vkDeviceWaitIdle(gpu.device);
			for (uint32_t image_idx = 0; image_idx < gpu.swapchain_images_count; image_idx += 1) {
				vkDestroyImageView(gpu.device, gpu.swapchain_image_views[image_idx], NULL);
			}

			free(gpu.swapchain_images);
			free(gpu.swapchain_image_views);
			gpu.swapchain_images = NULL;
			gpu.swapchain_image_views = NULL;
		}

		// Check the surface capabilities and formats
		VkSurfaceCapabilitiesKHR surface_capabilities;
		APP_VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.physical_device, gpu.surface, &surface_capabilities));

		VkExtent2D swapchain_extent = surface_capabilities.currentExtent;
		if (swapchain_extent.width == 0xFFFFFFFF) {
			swapchain_extent.width = APP_CLAMP(window_width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
			swapchain_extent.height = APP_CLAMP(window_height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		}

		gpu.swapchain_width = swapchain_extent.width;
		gpu.swapchain_height = swapchain_extent.height;

		VkSwapchainCreateInfoKHR swapchain_create_info = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = NULL,
			.surface = gpu.surface,
			.minImageCount = surface_capabilities.minImageCount,
			.imageFormat = GPU_VK_SURFACE_FORMAT,
			.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			.imageExtent = swapchain_extent,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.preTransform = surface_capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.imageArrayLayers = 1,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = NULL,
			// don't use FIFO for a game, it's just the option that is always
			// supported without checking for support for the other modes.
			.presentMode = VK_PRESENT_MODE_FIFO_KHR,
			.oldSwapchain = gpu.swapchain,
			.clipped = true,
		};
		APP_VK_ASSERT(vkCreateSwapchainKHR(gpu.device, &swapchain_create_info, NULL, &gpu.swapchain));

		APP_VK_ASSERT(vkGetSwapchainImagesKHR(gpu.device, gpu.swapchain, &gpu.swapchain_images_count, NULL));

		gpu.swapchain_images = (VkImage*)malloc(gpu.swapchain_images_count * sizeof(VkImage));
		APP_ASSERT(gpu.swapchain_images, "oom");

		gpu.swapchain_image_views = (VkImageView*)malloc(gpu.swapchain_images_count * sizeof(VkImageView));
		APP_ASSERT(gpu.swapchain_image_views, "oom");

		APP_VK_ASSERT(vkGetSwapchainImagesKHR(gpu.device, gpu.swapchain, &gpu.swapchain_images_count, gpu.swapchain_images));

		for (uint32_t image_idx = 0; image_idx < gpu.swapchain_images_count; image_idx += 1) {
			VkImageViewCreateInfo image_view_create_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = NULL,
				.format = GPU_VK_SURFACE_FORMAT,
				.components = {
					.r = VK_COMPONENT_SWIZZLE_R,
					.g = VK_COMPONENT_SWIZZLE_G,
					.b = VK_COMPONENT_SWIZZLE_B,
					.a = VK_COMPONENT_SWIZZLE_A,
				},
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.image = gpu.swapchain_images[image_idx],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.flags = 0,
			};

			APP_VK_ASSERT(vkCreateImageView(gpu.device, &image_view_create_info, NULL, &gpu.swapchain_image_views[image_idx]));
		}
	}

	{
		if (gpu.depth_image) {
			vkDestroyImageView(gpu.device, gpu.depth_image_view, NULL);
			vkDestroyImage(gpu.device, gpu.depth_image, NULL);
			vkFreeMemory(gpu.device, gpu.depth_image_memory, NULL);
		}

		VkImageCreateInfo image_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
			.extent = {
				.width = gpu.swapchain_width,
				.height = gpu.swapchain_height,
				.depth = 1,
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = NULL,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		APP_VK_ASSERT(vkCreateImage(gpu.device, &image_create_info, NULL, &gpu.depth_image));

		VkMemoryRequirements memory_req;
		vkGetImageMemoryRequirements(gpu.device, gpu.depth_image, &memory_req);

		uint32_t memory_type_idx = 0;
		while (!(memory_req.memoryTypeBits & (1 << memory_type_idx))) {
			memory_type_idx += 1;
		}

		VkMemoryAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = NULL,
			.allocationSize = memory_req.size,
			.memoryTypeIndex = memory_type_idx,
		};

		VkDeviceMemory depth_device_memory;
		APP_VK_ASSERT(vkAllocateMemory(gpu.device, &alloc_info, NULL, &depth_device_memory));

		APP_VK_ASSERT(vkBindImageMemory(gpu.device, gpu.depth_image, depth_device_memory, 0));

		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.format = VK_FORMAT_D32_SFLOAT,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.image = gpu.depth_image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.flags = 0,
		};

		APP_VK_ASSERT(vkCreateImageView(gpu.device, &image_view_create_info, NULL, &gpu.depth_image_view));
	}

	{
		if (gpu.swapchain_image_ready_semaphore) {
			vkDestroySemaphore(gpu.device, gpu.swapchain_image_ready_semaphore, NULL);
			vkDestroySemaphore(gpu.device, gpu.swapchain_present_ready_semaphore, NULL);
		}

		VkSemaphoreCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
		};
		APP_VK_ASSERT(vkCreateSemaphore(gpu.device, &create_info, NULL, &gpu.swapchain_image_ready_semaphore));
		APP_VK_ASSERT(vkCreateSemaphore(gpu.device, &create_info, NULL, &gpu.swapchain_present_ready_semaphore));
	}
}

VkBool32 gpu_vk_handle_validation_error(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	void*                                            pUserData
) {
	APP_UNUSED(messageSeverity);
	APP_UNUSED(messageTypes);
	APP_UNUSED(pUserData);

	char* stacktrace = b_stacktrace_get_string();
	FILE* f;
#if defined(_WIN32)
	fopen_s(&f, "vk_validation_log.txt", "w");
#else
	f = fopen("vk_validation_log.txt", "w");
#endif
	fprintf(f, "Error Name: %s\n\n%s\n\nStacktrace:\n%s\n\n", pCallbackData->pMessageIdName, pCallbackData->pMessage, stacktrace);
	fflush(f);

	platform_message_box("Vulkan Validation Error Detected.\nThe error has been logged to the vk_validation_log.txt file.\nPlease report this error and the log file on the HCC github issue tracker");

#if defined(_WIN32)
	DebugBreak();
#else
	raise(SIGINT);
#endif

	return VK_FALSE;
}

void gpu_init(DmWindow window, uint32_t window_width, uint32_t window_height) {
	VkResult vk_result;

	//
	// create instance
	//
	{
		VkLayerProperties layer_props[512];
		uint32_t layer_props_count = APP_ARRAY_COUNT(layer_props);
		APP_VK_ASSERT(vkEnumerateInstanceLayerProperties(&layer_props_count, layer_props));
		bool has_khronos_validation = false;
		for (uint32_t idx = 0; idx < layer_props_count; idx += 1) {
			if (strcmp(layer_props[idx].layerName, "VK_LAYER_KHRONOS_validation") == 0) {
				has_khronos_validation = true;
			}
		}

		VkExtensionProperties extension_props[512];
		uint32_t extension_props_count = APP_ARRAY_COUNT(extension_props);
		APP_VK_ASSERT(vkEnumerateInstanceExtensionProperties(NULL, &extension_props_count, extension_props));
		bool has_debug_utils = false;
		for (uint32_t idx = 0; idx < extension_props_count; idx += 1) {
			if (strcmp(extension_props[idx].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
				has_debug_utils = true;
			}
		}

		static const char* layers[2];
		uint32_t layers_count = 0;
		if (has_khronos_validation) {
			layers[layers_count] = "VK_LAYER_KHRONOS_validation";
			layers_count += 1;
		};

#if GPU_VK_DEBUG
		layers[layers_count] = "VK_LAYER_LUNARG_api_dump";
		layers_count += 1;
#endif

		uint32_t extensions_count = 2;
		const char* extensions[3] = {
			"VK_KHR_surface",
#ifdef __linux__
			"VK_KHR_xlib_surface",
#elif defined(_WIN32)
			"VK_KHR_win32_surface",
#else
#error "unsupported platform"
#endif
		};
		if (has_debug_utils) {
			extensions[extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			extensions_count += 1;
		}

		VkApplicationInfo app = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = NULL,
			.pApplicationName = APP_NAME,
			.applicationVersion = 0,
			.pEngineName = "none",
			.engineVersion = 0,
			.apiVersion = VK_API_VERSION_1_3,
		};
		VkInstanceCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = NULL,
			.pApplicationInfo = &app,
			.enabledLayerCount = layers_count,
			.ppEnabledLayerNames = layers,
			.enabledExtensionCount = extensions_count,
			.ppEnabledExtensionNames = extensions,
		};

		APP_VK_ASSERT(vkCreateInstance(&create_info, NULL, &gpu.instance));
	}

	{
		VkDebugUtilsMessengerCreateInfoEXT create_info = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.pNext = NULL,
			.flags = 0,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
			.pfnUserCallback = gpu_vk_handle_validation_error,
			.pUserData = NULL,
		};
		VkDebugUtilsMessengerEXT messenger;
		PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(gpu.instance, "vkCreateDebugUtilsMessengerEXT");
		APP_VK_ASSERT(fn(gpu.instance, &create_info, NULL, &messenger));
	}

	{
#define PHYSICAL_DEVICES_CAP 128
		uint32_t physical_devices_count = PHYSICAL_DEVICES_CAP;
		VkPhysicalDevice physical_devices[PHYSICAL_DEVICES_CAP];
		APP_VK_ASSERT(vkEnumeratePhysicalDevices(gpu.instance, &physical_devices_count, physical_devices));

		gpu.physical_device = physical_devices[0];

		vkGetPhysicalDeviceMemoryProperties(gpu.physical_device, &gpu.memory_properties);
	}

	{
#define QUEUE_FAMILIES_CAP 128
		uint32_t queue_families_count = QUEUE_FAMILIES_CAP;
		VkQueueFamilyProperties queue_families[QUEUE_FAMILIES_CAP];
		vkGetPhysicalDeviceQueueFamilyProperties(gpu.physical_device, &queue_families_count, queue_families);

		gpu.queue_family_idx = UINT32_MAX;
		for_range(queue_family_idx, 0, queue_families_count) {
			if (queue_families[queue_family_idx].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
				gpu.queue_family_idx = queue_family_idx;
				break;
			}
		}

		APP_ASSERT(gpu.queue_family_idx != UINT32_MAX, "could not find graphics and compute queue");

		float queue_priorities[1] = {0.0};
		VkDeviceQueueCreateInfo queue = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.queueFamilyIndex = gpu.queue_family_idx,
			.queueCount = 1,
			.pQueuePriorities = queue_priorities
		};

		VkPhysicalDeviceVulkan12Features features_1_1 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = NULL,
		};
		VkPhysicalDeviceVulkan12Features features_1_2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &features_1_1,
			.vulkanMemoryModel = VK_TRUE,
			.vulkanMemoryModelDeviceScope = VK_TRUE,
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
			.shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
			.shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
			.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
			.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
			.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
			.descriptorBindingPartiallyBound = VK_TRUE,
			.scalarBlockLayout = VK_TRUE,
		};
		VkPhysicalDeviceVulkan13Features features_1_3 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &features_1_2,
			.dynamicRendering = VK_TRUE,
			.synchronization2 = VK_TRUE,
			.shaderDemoteToHelperInvocation = VK_TRUE,
			.maintenance4 = VK_TRUE,
		};
		VkPhysicalDeviceFeatures2 features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features_1_3,
		};

		static const char* extensions[] = {
			"VK_KHR_swapchain",
		};

		VkDeviceCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queue,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = NULL,
			.enabledExtensionCount = APP_ARRAY_COUNT(extensions),
			.ppEnabledExtensionNames = extensions,
			.pEnabledFeatures = NULL,
		};

		APP_VK_ASSERT(vkCreateDevice(gpu.physical_device, &create_info, NULL, &gpu.device));

		vkGetDeviceQueue(gpu.device, gpu.queue_family_idx, 0, &gpu.queue);
	}

	VkCommandPoolCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.queueFamilyIndex = gpu.queue_family_idx,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};
	APP_VK_ASSERT(vkCreateCommandPool(gpu.device, &create_info, NULL, &gpu.command_pool));

	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = gpu.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = APP_FRAMES_IN_FLIGHT,
	};
	APP_VK_ASSERT(vkAllocateCommandBuffers(gpu.device, &alloc_info, gpu.command_buffers));

	for_range(idx, 0, APP_FRAMES_IN_FLIGHT) {
		VkFenceCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		APP_VK_ASSERT(vkCreateFence(gpu.device, &create_info, NULL, &gpu.fences[idx]));
	}

	{
#ifdef __linux__
		VkXlibSurfaceCreateInfoKHR create_info = {
			.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
			.pNext = NULL,
			.flags = 0,
			.dpy = window.instance,
			.window = (Window)(uintptr_t)window.handle,
		};

		APP_VK_ASSERT(vkCreateXlibSurfaceKHR(gpu.instance, &create_info, NULL, &gpu.surface));

		VisualID visual_id = XVisualIDFromVisual(DefaultVisual(window.instance, DefaultScreen(window.instance)));

		APP_ASSERT(vkGetPhysicalDeviceXlibPresentationSupportKHR(gpu.physical_device, gpu.queue_family_idx, window.instance, visual_id), "hmm the main queue should have presentation support, haven't seen a device that doesn't!");
#elif defined(_WIN32)
		VkWin32SurfaceCreateInfoKHR create_info = {
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.pNext = NULL,
			.flags = 0,
			.hinstance = window.instance,
			.hwnd = (HWND)(uintptr_t)window.handle,
		};

		APP_VK_ASSERT(vkCreateWin32SurfaceKHR(gpu.instance, &create_info, NULL, &gpu.surface));

		APP_ASSERT(vkGetPhysicalDeviceWin32PresentationSupportKHR(gpu.physical_device, gpu.queue_family_idx), "hmm the main queue should have presentation support, haven't seen a device that doesn't!");
#else
#error "unsupported platform"
#endif
	}

	gpu.push_constants_stage_flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

	{
		void* code;
		uintptr_t code_size;
		APP_ASSERT(platform_file_read_all(APP_SHADERS_PATH, &code, &code_size), "failed to read shader file from disk: %s", APP_SHADERS_PATH);

		VkShaderModuleCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = NULL,
			.codeSize = code_size,
			.pCode = code,
			.flags = 0,
		};

		APP_VK_ASSERT(vkCreateShaderModule(gpu.device, &create_info, NULL, &gpu.shader_module));
	}

	HccInteropVulkanSetup interop_setup = {
		.device = gpu.device,
		.descriptor_sets = gpu.descriptor_sets,
		.descriptor_sets_count = APP_ARRAY_COUNT(gpu.descriptor_sets),
		.shader_stages = gpu.push_constants_stage_flags,
		.resource_descriptors_max = hcc_metadata.resource_descriptors_max,
		.bundled_constants_size_max = hcc_metadata.bundled_constants_size_max,
	};
	hcc_interop_vulkan_init(&gpu.interop, &interop_setup);

	gpu_vk_recreate_swapchain_and_friends(window_width, window_height);
}

void gpu_init_sample(AppSampleEnum sample_enum) {
	VkResult vk_result;

	AppSample* sample = &app_samples[sample_enum];
	GpuVkSample* gpu_sample = &gpu_samples[sample_enum];

	switch (sample->shader_type) {
		case APP_SHADER_TYPE_GRAPHICS: {
			VkPipelineShaderStageCreateInfo shader_stages[] = {
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = NULL,
					.flags = 0,
					.stage = VK_SHADER_STAGE_VERTEX_BIT,
					.module = gpu.shader_module,
					.pName = hcc_shader_infos[sample->graphics.shader_vs].name,
					.pSpecializationInfo = NULL,
				},
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = NULL,
					.flags = 0,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.module = gpu.shader_module,
					.pName = hcc_shader_infos[sample->graphics.shader_fs].name,
					.pSpecializationInfo = NULL,
				},
			};

			VkPipelineVertexInputStateCreateInfo vertex_input_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.vertexBindingDescriptionCount = 0,
				.pVertexBindingDescriptions = NULL,
				.vertexAttributeDescriptionCount = 0,
				.pVertexAttributeDescriptions = NULL,
			};

			VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.topology = 0,
				.primitiveRestartEnable = false,
			};
			switch (sample->graphics.topology) {
				case APP_TOPOLOGY_TRIANGLE_LIST: input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
				case APP_TOPOLOGY_TRIANGLE_STRIP: input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
				default: APP_ABORT("unhandled topology");
			}

			VkPipelineTessellationStateCreateInfo tessellation_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.patchControlPoints = 0,
			};

			VkPipelineViewportStateCreateInfo viewport_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.viewportCount = 1,
				.pViewports = NULL,
				.scissorCount = 1,
				.pScissors = NULL
			};

			VkPipelineRasterizationStateCreateInfo rasterization_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.depthClampEnable = false,
				.rasterizerDiscardEnable = false,
				.polygonMode = VK_POLYGON_MODE_FILL,
				.cullMode = VK_CULL_MODE_NONE,
				.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
				.depthBiasEnable = false,
				.depthBiasConstantFactor = 0.f,
				.depthBiasClamp = 0.f,
				.depthBiasSlopeFactor = 0.f,
				.lineWidth = 1.f,
			};

			VkPipelineMultisampleStateCreateInfo multisample_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = false,
				.minSampleShading = 0.f,
				.pSampleMask = NULL,
				.alphaToCoverageEnable = false,
				.alphaToOneEnable = false,
			};

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.depthTestEnable = true,
				.depthWriteEnable = true,
				.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
				.depthBoundsTestEnable = false,
				.stencilTestEnable = false,
				.front = {0},
				.back = {0},
				.minDepthBounds = 0.f,
				.maxDepthBounds = 1.f,
			};

			VkPipelineColorBlendAttachmentState color_blend_attachment = {
				.blendEnable = true,
				.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			};

			VkPipelineColorBlendStateCreateInfo color_blend_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.logicOpEnable = false,
				.logicOp = 0,
				.attachmentCount = 1,
				.pAttachments = &color_blend_attachment,
				.blendConstants = {0},
			};

			VkDynamicState vk_dynamic_states[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
			};

			VkPipelineDynamicStateCreateInfo dynamic_state = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.dynamicStateCount = APP_ARRAY_COUNT(vk_dynamic_states),
				.pDynamicStates = vk_dynamic_states,
			};

			VkFormat image_format = GPU_VK_SURFACE_FORMAT;
			VkPipelineRenderingCreateInfo rendering_create_info = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.pNext = NULL,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &image_format,
				.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
				.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
			};

			VkGraphicsPipelineCreateInfo create_info = {
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = &rendering_create_info,
				.flags = 0,
				.stageCount = APP_ARRAY_COUNT(shader_stages),
				.pStages = shader_stages,
				.pVertexInputState = &vertex_input_state,
				.pInputAssemblyState = &input_assembly_state,
				.pTessellationState = &tessellation_state,
				.pViewportState = &viewport_state,
				.pRasterizationState = &rasterization_state,
				.pMultisampleState = &multisample_state,
				.pDepthStencilState = &depth_stencil_state,
				.pColorBlendState = &color_blend_state,
				.pDynamicState = &dynamic_state,
				.layout = gpu.interop.pipeline_layout,
				.renderPass = VK_NULL_HANDLE,
				.subpass = 0,
				.basePipelineHandle = NULL,
				.basePipelineIndex = 0,
			};

			APP_VK_ASSERT(vkCreateGraphicsPipelines(gpu.device, VK_NULL_HANDLE, 1, &create_info, NULL, &gpu_sample->pipeline));
			break;
		};
		case APP_SHADER_TYPE_COMPUTE: {
			VkComputePipelineCreateInfo create_info = {
				.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.stage = {
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = NULL,
					.flags = 0,
					.stage = VK_SHADER_STAGE_COMPUTE_BIT,
					.module = gpu.shader_module,
					.pName = hcc_shader_infos[sample->compute.shader_cs].name,
					.pSpecializationInfo = NULL,
				},
				.layout = gpu.interop.pipeline_layout,
				.basePipelineHandle = NULL,
				.basePipelineIndex = 0,
			};

			APP_VK_ASSERT(vkCreateComputePipelines(gpu.device, VK_NULL_HANDLE, 1, &create_info, NULL, &gpu_sample->pipeline));
			break;
		};
	}
}

void gpu_render_frame(AppSampleEnum sample_enum, void* bc, uint32_t window_width, uint32_t window_height) {
	VkResult vk_result;

	AppSample* sample = &app_samples[sample_enum];
	GpuVkSample* gpu_sample = &gpu_samples[sample_enum];

	uint32_t active_frame_idx = gpu.frame_idx % APP_FRAMES_IN_FLIGHT;

	//
	// wait for two frames ago to be finished on the GPU so we can start using it's stuff!
	APP_VK_ASSERT(vkWaitForFences(gpu.device, 1, &gpu.fences[active_frame_idx], true, UINT64_MAX));
	APP_VK_ASSERT(vkResetFences(gpu.device, 1, &gpu.fences[active_frame_idx]));

	uint32_t swapchain_image_idx;
	while (1) {
		vk_result = vkAcquireNextImageKHR(gpu.device, gpu.swapchain, UINT64_MAX, gpu.swapchain_image_ready_semaphore, VK_NULL_HANDLE, &swapchain_image_idx);
		bool yes = false;
		switch (vk_result) {
			case VK_ERROR_OUT_OF_DATE_KHR:
			case VK_SUBOPTIMAL_KHR:
				gpu_vk_recreate_swapchain_and_friends(window_width, window_height);
				break;
			default:
				APP_VK_ASSERT(vk_result);
				yes = true;
				break;
		}
		if (yes) {
			break;
		}
	}
	VkImage swapchain_image = gpu.swapchain_images[swapchain_image_idx];
	VkImageView swapchain_image_view = gpu.swapchain_image_views[swapchain_image_idx];

	{
		GpuVkResource* res = &gpu.resources[0];
		res->image = swapchain_image;
		res->image_view = swapchain_image_view;
	}

	for (uint32_t res_idx = 0; res_idx < gpu.next_resource_id; res_idx += 1) {
		GpuVkResource* res = &gpu.resources[res_idx];

		if (res->buffer && res->device_memory) {
			VkMappedMemoryRange range = {
				.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				.pNext = NULL,
				.memory = res->device_memory,
				.offset = 0,
				.size = res->device_memory_size,
			};

			APP_VK_ASSERT(vkFlushMappedMemoryRanges(gpu.device, 1, &range));
		}

		if (res->buffer) {
			VkDescriptorBufferInfo buffer_info = {
				.buffer = res->buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE,
			};

			VkWriteDescriptorSet vk_descriptor_write;
			hcc_interop_vulkan_descriptor_write_make(&gpu.interop, &vk_descriptor_write, active_frame_idx, HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_STORAGE_BUFFER, res_idx);
			vk_descriptor_write.pBufferInfo = &buffer_info;
			vkUpdateDescriptorSets(gpu.device, 1, &vk_descriptor_write, 0, NULL); // this is super lazy! you should batch you descriptor write into a single call!
		} else if (res->image) {
			if (res->image != swapchain_image) {
				VkDescriptorImageInfo image_info = {
					.sampler = VK_NULL_HANDLE,
					.imageView = res->image_view,
					.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
				};

				VkWriteDescriptorSet vk_descriptor_write;
				hcc_interop_vulkan_descriptor_write_make(&gpu.interop, &vk_descriptor_write, active_frame_idx, HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_SAMPLED_IMAGE, res_idx);
				vk_descriptor_write.pImageInfo = &image_info;
				vkUpdateDescriptorSets(gpu.device, 1, &vk_descriptor_write, 0, NULL); // this is super lazy! you should batch you descriptor write into a single call!
			}

			{
				VkDescriptorImageInfo image_info = {
					.sampler = VK_NULL_HANDLE,
					.imageView = res->image_view,
					.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
				};

				VkWriteDescriptorSet vk_descriptor_write;
				hcc_interop_vulkan_descriptor_write_make(&gpu.interop, &vk_descriptor_write, active_frame_idx, HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_STORAGE_IMAGE, res_idx);
				vk_descriptor_write.pImageInfo = &image_info;
				vkUpdateDescriptorSets(gpu.device, 1, &vk_descriptor_write, 0, NULL); // this is super lazy! you should batch you descriptor write into a single call!
			}
		} else if (res->sampler) {
			VkDescriptorImageInfo image_info = {
				.sampler = res->sampler,
				.imageView = VK_NULL_HANDLE,
				.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			};

			VkWriteDescriptorSet vk_descriptor_write;
			hcc_interop_vulkan_descriptor_write_make(&gpu.interop, &vk_descriptor_write, active_frame_idx, HCC_INTEROP_VULKAN_DESCRIPTOR_BINDING_SAMPLER, res_idx);
			vk_descriptor_write.pImageInfo = &image_info;
			vkUpdateDescriptorSets(gpu.device, 1, &vk_descriptor_write, 0, NULL); // this is super lazy! you should batch you descriptor write into a single call!
		}

	}

	VkCommandBuffer vk_command_buffer = gpu.command_buffers[active_frame_idx];
	APP_VK_ASSERT(vkResetCommandBuffer(vk_command_buffer, 0));
	{
		VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = NULL,
		};

		APP_VK_ASSERT(vkBeginCommandBuffer(vk_command_buffer, &begin_info));
	}

	vkCmdPushConstants(vk_command_buffer, gpu.interop.pipeline_layout, gpu.push_constants_stage_flags, 0, hcc_metadata.bundled_constants_size_max, bc);

	switch (sample->shader_type) {
		case APP_SHADER_TYPE_GRAPHICS: {
			{
				VkImageMemoryBarrier2 image_barriers[] = {
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.pNext = NULL,
						.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
						.srcAccessMask = VK_ACCESS_2_NONE,
						.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
						.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.srcQueueFamilyIndex = gpu.queue_family_idx,
						.dstQueueFamilyIndex = gpu.queue_family_idx,
						.image = swapchain_image,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						},
					},
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.pNext = NULL,
						.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
						.srcAccessMask = VK_ACCESS_2_NONE,
						.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
						.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						.srcQueueFamilyIndex = gpu.queue_family_idx,
						.dstQueueFamilyIndex = gpu.queue_family_idx,
						.image = gpu.depth_image,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						},
					}
				};

				VkDependencyInfo dependency_info = {
					.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
					.pNext = NULL,
					.dependencyFlags = 0,
					.memoryBarrierCount = 0,
					.pMemoryBarriers = NULL,
					.bufferMemoryBarrierCount = 0,
					.pBufferMemoryBarriers = NULL,
					.imageMemoryBarrierCount = APP_ARRAY_COUNT(image_barriers),
					.pImageMemoryBarriers = image_barriers,
				};

				vkCmdPipelineBarrier2(vk_command_buffer, &dependency_info);
			}

			{
				VkRenderingAttachmentInfo color_attachment_info = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.pNext = NULL,
					.imageView = swapchain_image_view,
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.resolveImageView = VK_NULL_HANDLE,
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.clearValue = {0},
				};

				VkRenderingAttachmentInfo depth_attachment_info = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.pNext = NULL,
					.imageView = gpu.depth_image_view,
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.resolveImageView = VK_NULL_HANDLE,
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.clearValue = {0},
				};

				VkRenderingInfo rendering_info = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
					.pNext = NULL,
					.flags = 0,
					.renderArea = {
						.offset.x = 0,
						.offset.y = 0,
						.extent.width = gpu.swapchain_width,
						.extent.height = gpu.swapchain_height,
					},
					.layerCount = 1,
					.viewMask = 0,
					.colorAttachmentCount = 1,
					.pColorAttachments = &color_attachment_info,
					.pDepthAttachment = &depth_attachment_info,
					.pStencilAttachment = NULL,
				};

				vkCmdBeginRendering(vk_command_buffer, &rendering_info);
			}

			VkViewport viewport = {
				.x = 0,
				.y = gpu.swapchain_height,
				.width = gpu.swapchain_width,
				.height = -(float)gpu.swapchain_height,
				.minDepth = 0.f,
				.maxDepth = 1.f,
			};

			VkRect2D scissor = {
				.offset = { .x = 0, .y = 0 },
				.extent = { .width = gpu.swapchain_width, .height = gpu.swapchain_height },
			};

			vkCmdSetViewport(vk_command_buffer, 0, 1, &viewport);
			vkCmdSetScissor(vk_command_buffer, 0, 1, &scissor);
			vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu_sample->pipeline);
			vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu.interop.pipeline_layout, 0, 1, &gpu.interop.descriptor_sets[active_frame_idx], 0, NULL);
			vkCmdDraw(vk_command_buffer, sample->graphics.vertices_count, 1, 0, 0);
			vkCmdEndRendering(vk_command_buffer);

			{
				VkImageMemoryBarrier2 image_barriers[] = {
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.pNext = NULL,
						.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
						.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
						.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
						.dstAccessMask = VK_ACCESS_2_NONE,
						.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						.srcQueueFamilyIndex = gpu.queue_family_idx,
						.dstQueueFamilyIndex = gpu.queue_family_idx,
						.image = swapchain_image,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						},
					}
				};

				VkDependencyInfo dependency_info = {
					.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
					.pNext = NULL,
					.dependencyFlags = 0,
					.memoryBarrierCount = 0,
					.pMemoryBarriers = NULL,
					.bufferMemoryBarrierCount = 0,
					.pBufferMemoryBarriers = NULL,
					.imageMemoryBarrierCount = APP_ARRAY_COUNT(image_barriers),
					.pImageMemoryBarriers = image_barriers,
				};

				vkCmdPipelineBarrier2(vk_command_buffer, &dependency_info);
			}
			break;
		};
		case APP_SHADER_TYPE_COMPUTE: {
			{
				VkImageMemoryBarrier2 image_barriers[] = {
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.pNext = NULL,
						.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
						.srcAccessMask = VK_ACCESS_2_NONE,
						.dstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT,
						.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = VK_IMAGE_LAYOUT_GENERAL,
						.srcQueueFamilyIndex = gpu.queue_family_idx,
						.dstQueueFamilyIndex = gpu.queue_family_idx,
						.image = swapchain_image,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						},
					}
				};

				VkDependencyInfo dependency_info = {
					.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
					.pNext = NULL,
					.dependencyFlags = 0,
					.memoryBarrierCount = 0,
					.pMemoryBarriers = NULL,
					.bufferMemoryBarrierCount = 0,
					.pBufferMemoryBarriers = NULL,
					.imageMemoryBarrierCount = APP_ARRAY_COUNT(image_barriers),
					.pImageMemoryBarriers = image_barriers,
				};

				vkCmdPipelineBarrier2(vk_command_buffer, &dependency_info);
			}

			VkImageSubresourceRange subresource_range = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			};
			VkClearColorValue clear_color = {0};
			vkCmdClearColorImage(vk_command_buffer, swapchain_image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &subresource_range);

			{
				VkImageMemoryBarrier2 image_barriers[] = {
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.pNext = NULL,
						.srcStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT,
						.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
						.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
						.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = VK_IMAGE_LAYOUT_GENERAL,
						.srcQueueFamilyIndex = gpu.queue_family_idx,
						.dstQueueFamilyIndex = gpu.queue_family_idx,
						.image = swapchain_image,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						},
					}
				};

				VkDependencyInfo dependency_info = {
					.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
					.pNext = NULL,
					.dependencyFlags = 0,
					.memoryBarrierCount = 0,
					.pMemoryBarriers = NULL,
					.bufferMemoryBarrierCount = 0,
					.pBufferMemoryBarriers = NULL,
					.imageMemoryBarrierCount = APP_ARRAY_COUNT(image_barriers),
					.pImageMemoryBarriers = image_barriers,
				};

				vkCmdPipelineBarrier2(vk_command_buffer, &dependency_info);
			}


			vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpu_sample->pipeline);
			vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gpu.interop.pipeline_layout, 0, 1, &gpu.interop.descriptor_sets[active_frame_idx], 0, NULL);
			vkCmdDispatch(vk_command_buffer, sample->compute.dispatch_group_size_x, sample->compute.dispatch_group_size_y, sample->compute.dispatch_group_size_z);

			{
				VkImageMemoryBarrier2 image_barriers[] = {
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.pNext = NULL,
						.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
						.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
						.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
						.dstAccessMask = VK_ACCESS_2_NONE,
						.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
						.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						.srcQueueFamilyIndex = gpu.queue_family_idx,
						.dstQueueFamilyIndex = gpu.queue_family_idx,
						.image = swapchain_image,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						},
					}
				};

				VkDependencyInfo dependency_info = {
					.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
					.pNext = NULL,
					.dependencyFlags = 0,
					.memoryBarrierCount = 0,
					.pMemoryBarriers = NULL,
					.bufferMemoryBarrierCount = 0,
					.pBufferMemoryBarriers = NULL,
					.imageMemoryBarrierCount = APP_ARRAY_COUNT(image_barriers),
					.pImageMemoryBarriers = image_barriers,
				};

				vkCmdPipelineBarrier2(vk_command_buffer, &dependency_info);
			}
			break;
		};
	}

	APP_VK_ASSERT(vkEndCommandBuffer(vk_command_buffer));

	{
		VkSemaphoreSubmitInfo wait_semaphore_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.pNext = NULL,
			.semaphore = gpu.swapchain_image_ready_semaphore,
			.value = 0,
			.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			.deviceIndex = 0,
		};

		VkSemaphoreSubmitInfo signal_semaphore_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.pNext = NULL,
			.semaphore = gpu.swapchain_present_ready_semaphore,
			.value = 0,
			.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.deviceIndex = 0,
		};

		VkCommandBufferSubmitInfo command_buffer_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.pNext = NULL,
			.commandBuffer = vk_command_buffer,
			.deviceMask = 0,
		};

		VkSubmitInfo2 vk_submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = 0,
			.flags = 0,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &wait_semaphore_info,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &command_buffer_info,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signal_semaphore_info,
		};

		APP_VK_ASSERT(vkQueueSubmit2(gpu.queue, 1, &vk_submit_info, gpu.fences[active_frame_idx]));
	}

	{
		VkPresentInfoKHR present_info = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &gpu.swapchain_present_ready_semaphore,
			.swapchainCount = 1,
			.pSwapchains = &gpu.swapchain,
			.pImageIndices = &swapchain_image_idx,
			.pResults = NULL,
		};

		vk_result = vkQueuePresentKHR(gpu.queue, &present_info);
		switch (vk_result) {
			case VK_ERROR_OUT_OF_DATE_KHR:
			case VK_SUBOPTIMAL_KHR:
				break;
			default:
				APP_VK_ASSERT(vk_result);
				break;
		}
	}
	gpu.frame_idx += 1;
}

GpuResourceId gpu_create_backbuffer(void) {
	GpuResourceId res_id = gpu.next_resource_id;
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resources full");
	gpu.next_resource_id += 1;
	GpuVkResource* res = &gpu.resources[res_id];
	memset(res, 0x00, sizeof(*res));
	return res_id;
}

GpuResourceId gpu_create_staging_buffer(void) {
	gpu.staging_buffer_cap = 32 * 1024 * 1024;
	gpu.staging_buffer = gpu_create_buffer(gpu.staging_buffer_cap);
	gpu.staging_buffer_size = 0;
	return gpu.staging_buffer;
}

GpuResourceId gpu_create_buffer(uint32_t size) {
	VkResult vk_result;
	GpuResourceId res_id = gpu.next_resource_id;
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resources full");
	gpu.next_resource_id += 1;

	GpuVkResource* res = &gpu.resources[res_id];

	VkBufferCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = size,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
	};

	APP_VK_ASSERT(vkCreateBuffer(gpu.device, &create_info, NULL, &res->buffer));

	VkMemoryRequirements mem_req;
	vkGetBufferMemoryRequirements(gpu.device, res->buffer, &mem_req);
	res->device_memory_size = mem_req.size;

	uint32_t memory_type_idx = 0;
	while (1) {
		if (mem_req.memoryTypeBits & (1 << memory_type_idx)) {
			if (gpu.memory_properties.memoryTypes[memory_type_idx].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && !(gpu.memory_properties.memoryTypes[memory_type_idx].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
				break;
			}
		}
		memory_type_idx += 1;
	}
	APP_ASSERT(memory_type_idx < VK_MAX_MEMORY_TYPES, "failed to find memory type");

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = memory_type_idx,
	};
	APP_VK_ASSERT(vkAllocateMemory(gpu.device, &alloc_info, NULL, &res->device_memory));
	APP_VK_ASSERT(vkBindBufferMemory(gpu.device, res->buffer, res->device_memory, 0));
	APP_VK_ASSERT(vkMapMemory(gpu.device, res->device_memory, 0, mem_req.size, 0, &res->device_memory_mapped));

	return res_id;
}

GpuResourceId gpu_create_texture(GpuTextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t array_layers, uint32_t mip_levels) {
	VkResult vk_result;
	GpuResourceId res_id = gpu.next_resource_id;
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resources full");
	gpu.next_resource_id += 1;

	GpuVkResource* res = &gpu.resources[res_id];

	VkImageType image_type;
	VkImageViewType image_view_type;
	switch (type) {
		case GPU_TEXTURE_TYPE_1D:
			image_type = VK_IMAGE_TYPE_1D;
			image_view_type = VK_IMAGE_VIEW_TYPE_1D;
			break;
		case GPU_TEXTURE_TYPE_1D_ARRAY:
			image_type = VK_IMAGE_TYPE_1D;
			image_view_type = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			break;
		case GPU_TEXTURE_TYPE_2D:
			image_type = VK_IMAGE_TYPE_2D;
			image_view_type = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case GPU_TEXTURE_TYPE_2D_ARRAY:
			image_type = VK_IMAGE_TYPE_2D;
			image_view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			break;
		case GPU_TEXTURE_TYPE_CUBE:
			image_type = VK_IMAGE_TYPE_2D;
			image_view_type = VK_IMAGE_VIEW_TYPE_CUBE;
			break;
		case GPU_TEXTURE_TYPE_CUBE_ARRAY:
			image_type = VK_IMAGE_TYPE_2D;
			image_view_type = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			break;
		case GPU_TEXTURE_TYPE_3D:
			image_type = VK_IMAGE_TYPE_3D;
			image_view_type = VK_IMAGE_VIEW_TYPE_3D;
			break;
	}

	VkImageCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.imageType = image_type,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.extent.width = width,
		.extent.height = height,
		.extent.depth = depth,
		.mipLevels = mip_levels,
		.arrayLayers = array_layers,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	APP_VK_ASSERT(vkCreateImage(gpu.device, &create_info, NULL, &res->image));

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(gpu.device, res->image, &mem_req);
	res->device_memory_size = mem_req.size;

	uint32_t memory_type_idx = 0;
	while (1) {
		if (mem_req.memoryTypeBits & (1 << memory_type_idx)) {
			if (gpu.memory_properties.memoryTypes[memory_type_idx].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
				break;
			}
		}
		memory_type_idx += 1;
	}
	APP_ASSERT(memory_type_idx < VK_MAX_MEMORY_TYPES, "failed to find memory type");

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = memory_type_idx,
	};
	APP_VK_ASSERT(vkAllocateMemory(gpu.device, &alloc_info, NULL, &res->device_memory));
	APP_VK_ASSERT(vkBindImageMemory(gpu.device, res->image, res->device_memory, 0));

	VkImageViewCreateInfo view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = res->image,
		.viewType = image_view_type,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.components.r = VK_COMPONENT_SWIZZLE_R,
		.components.g = VK_COMPONENT_SWIZZLE_G,
		.components.b = VK_COMPONENT_SWIZZLE_B,
		.components.a = VK_COMPONENT_SWIZZLE_A,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = mip_levels,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = array_layers,
	};
	APP_VK_ASSERT(vkCreateImageView(gpu.device, &view_create_info, NULL, &res->image_view));
	return res_id;
}

GpuResourceId gpu_create_sampler(void) {
	VkResult vk_result;
	GpuResourceId res_id = gpu.next_resource_id;
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resources full");
	gpu.next_resource_id += 1;

	GpuVkResource* res = &gpu.resources[res_id];

	VkSamplerCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = 0.f,
		.anisotropyEnable = false,
		.maxAnisotropy = 0.f,
		.compareEnable = false,
		.compareOp = VK_COMPARE_OP_NEVER,
		.minLod = 0.f,
		.maxLod = 64.f,
		.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
		.unnormalizedCoordinates = false,
	};
	APP_VK_ASSERT(vkCreateSampler(gpu.device, &create_info, NULL, &res->sampler));
	return res_id;
}

void* gpu_map_resource(GpuResourceId res_id) {
	VkResult vk_result;
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resource id out of bounds");
	GpuVkResource* res = &gpu.resources[res_id];
	return res->device_memory_mapped;
}

uint32_t gpu_mip_offset(GpuResourceId res_id, uint32_t mip) {
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resource id out of bounds");
	GpuVkResource* res = &gpu.resources[res_id];

	VkImageSubresource sub_resource = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = mip,
		.arrayLayer = 0,
	};

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(gpu.device, res->image, &sub_resource, &layout);
	return layout.offset;
}

uint32_t gpu_row_pitch(GpuResourceId res_id, uint32_t mip) {
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resource id out of bounds");
	GpuVkResource* res = &gpu.resources[res_id];

	VkImageSubresource sub_resource = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = mip,
		.arrayLayer = 0,
	};

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(gpu.device, res->image, &sub_resource, &layout);
	return layout.rowPitch;
}

uint32_t gpu_depth_pitch(GpuResourceId res_id, uint32_t mip) {
	APP_ASSERT(res_id < APP_ARRAY_COUNT(gpu.resources), "resource id out of bounds");
	GpuVkResource* res = &gpu.resources[res_id];

	VkImageSubresource sub_resource = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = mip,
		.arrayLayer = 0,
	};

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(gpu.device, res->image, &sub_resource, &layout);
	return layout.depthPitch;
}

void* gpu_stage_upload(GpuResourceId res_id, uint32_t width, uint32_t height, uint32_t depth, uint32_t bpp, uint32_t mip) {
	GpuVkResource* res = &gpu.resources[res_id];
	APP_ASSERT(res->image, "only handled images for now");
	APP_ASSERT(gpu.staged_uploads_count < GPU_VK_STAGED_UPLOADS_CAP, "staged uploads full");

	uint32_t size = width * height * depth * bpp;
	GpuVkResource* staging_buffer = &gpu.resources[gpu.staging_buffer];
	GpuVkStagedUpload* upload = &gpu.staged_uploads[gpu.staged_uploads_count];
	upload->image = res->image;
	upload->width = width;
	upload->height = height;
	upload->depth = depth;
	upload->mip_level = mip;
	gpu.staged_uploads_count += 1;

	uint32_t offset = gpu.staging_buffer_size;
	upload->buffer_offset = offset;
	gpu.staging_buffer_size += size;
	return (uint8_t*)staging_buffer->device_memory_mapped + offset;
}


void gpu_stage_uploads_flush(void) {
	VkResult vk_result;
	VkCommandPool vk_command_pool;

	VkCommandPoolCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.queueFamilyIndex = gpu.queue_family_idx,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};
	APP_VK_ASSERT(vkCreateCommandPool(gpu.device, &create_info, NULL, &vk_command_pool));

	VkCommandBuffer vk_command_buffer;
	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = vk_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = APP_FRAMES_IN_FLIGHT,
	};
	APP_VK_ASSERT(vkAllocateCommandBuffers(gpu.device, &alloc_info, &vk_command_buffer));

	{
		VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = NULL,
		};

		APP_VK_ASSERT(vkBeginCommandBuffer(vk_command_buffer, &begin_info));
	}

	GpuVkResource* staging_buffer = &gpu.resources[gpu.staging_buffer];
	for (uint32_t idx = 0; idx < gpu.staged_uploads_count; idx += 1) {
		GpuVkStagedUpload* upload = &gpu.staged_uploads[idx];

		{
			VkImageMemoryBarrier2 image_barriers[] = {
				{
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.pNext = NULL,
					.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
					.srcAccessMask = VK_ACCESS_2_NONE,
					.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
					.dstAccessMask = VK_ACCESS_2_NONE,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.srcQueueFamilyIndex = gpu.queue_family_idx,
					.dstQueueFamilyIndex = gpu.queue_family_idx,
					.image = upload->image,
					.subresourceRange = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = upload->mip_level,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					},
				},
			};

			VkDependencyInfo dependency_info = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = NULL,
				.dependencyFlags = 0,
				.memoryBarrierCount = 0,
				.pMemoryBarriers = NULL,
				.bufferMemoryBarrierCount = 0,
				.pBufferMemoryBarriers = NULL,
				.imageMemoryBarrierCount = APP_ARRAY_COUNT(image_barriers),
				.pImageMemoryBarriers = image_barriers,
			};

			vkCmdPipelineBarrier2(vk_command_buffer, &dependency_info);
		}

		VkBufferImageCopy copy_region = {
			.bufferOffset = upload->buffer_offset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = upload->mip_level,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = {0},
			.imageExtent.width = upload->width,
			.imageExtent.height = upload->height,
			.imageExtent.depth = upload->depth,
		};

		vkCmdCopyBufferToImage(
			vk_command_buffer,
			staging_buffer->buffer,
			upload->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copy_region);

		{
			VkImageMemoryBarrier2 image_barriers[] = {
				{
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.pNext = NULL,
					.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
					.srcAccessMask = VK_ACCESS_2_NONE,
					.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
					.dstAccessMask = VK_ACCESS_2_NONE,
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					.srcQueueFamilyIndex = gpu.queue_family_idx,
					.dstQueueFamilyIndex = gpu.queue_family_idx,
					.image = upload->image,
					.subresourceRange = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = upload->mip_level,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					},
				},
			};

			VkDependencyInfo dependency_info = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = NULL,
				.dependencyFlags = 0,
				.memoryBarrierCount = 0,
				.pMemoryBarriers = NULL,
				.bufferMemoryBarrierCount = 0,
				.pBufferMemoryBarriers = NULL,
				.imageMemoryBarrierCount = APP_ARRAY_COUNT(image_barriers),
				.pImageMemoryBarriers = image_barriers,
			};

			vkCmdPipelineBarrier2(vk_command_buffer, &dependency_info);
		}
	}

	APP_VK_ASSERT(vkEndCommandBuffer(vk_command_buffer));

	VkCommandBufferSubmitInfo command_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = NULL,
		.commandBuffer = vk_command_buffer,
		.deviceMask = 0,
	};

	VkSubmitInfo2 vk_submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = 0,
		.flags = 0,
		.waitSemaphoreInfoCount = 0,
		.pWaitSemaphoreInfos = NULL,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &command_buffer_info,
		.signalSemaphoreInfoCount = 0,
		.pSignalSemaphoreInfos = NULL,
	};

	APP_VK_ASSERT(vkQueueSubmit2(gpu.queue, 1, &vk_submit_info, NULL));

	gpu.staged_uploads_count = 0;
	gpu.staging_buffer_size = 0;
}

