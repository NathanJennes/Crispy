//
// Created by nathan on 1/10/23.
//

#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace Vulkan {

class VulkanInstance
{
public:
	static bool initialize();
	static void shutdown();

	static VkInstance instance() { return _instance; }

private:
	static bool init_instance();
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
	static VkInstance _instance;
	static VkDebugUtilsMessengerEXT _debug_messenger;
};

}


#endif //VULKANINSTANCE_H
