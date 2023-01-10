//
// Created by nathan on 1/10/23.
//

#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>
#include "defines.h"

namespace Vulkan {

class VulkanInstance
{
public:
	static bool initialize();
	static void shutdown();

	static VkInstance instance() { return _instance; }
	static VkPhysicalDevice  physical_device() { return _physical_device; }

private:
	static bool init_instance();

	static bool pick_physical_device();
	static std::vector<VkPhysicalDevice> get_physical_device_list();
	static bool is_physical_device_suitable(VkPhysicalDevice device);
	static VkPhysicalDevice pick_best_device(const std::vector<VkPhysicalDevice>& devices);
	static u32 rate_physical_device(VkPhysicalDevice device);

	static bool init_debug_messenger();
	static void destroy_debug_messenger();

	static bool supports_validation_layer(const std::vector<const char*>& required_layers);
	static std::vector<const char*> get_required_validation_layers();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);


private:
	static VkInstance				_instance;
	static VkDebugUtilsMessengerEXT	_debug_messenger;
	static VkPhysicalDevice			_physical_device;
};

}


#endif //VULKANINSTANCE_H
