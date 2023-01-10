//
// Created by nathan on 1/10/23.
//

#include <vector>
#include <cstring>
#include <iostream>
#include "VulkanInstance.h"
#include "log.h"

namespace Vulkan {

VkInstance VulkanInstance::_instance;
VkDebugUtilsMessengerEXT VulkanInstance::_debug_messenger;

bool VulkanInstance::initialize()
{
	if (!init_instance())
		return false;

#ifdef DEBUG
	if (!init_debug_messenger())
		return false;
#endif

	return true;
}

bool VulkanInstance::init_instance()
{
#ifdef DEBUG
	// Check early if validation layers are supported
	if (!supports_validation_layer(get_required_validation_layers())) {
		CORE_ERROR("Started in debug mode but validation layers are unavailable!");
		return false;
	}
#endif

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = "Vulkan Application";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Vulkan Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instance_info{};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pNext = nullptr;
	instance_info.pApplicationInfo = &app_info;

	// Extensions
	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.push_back("VK_KHR_xcb_surface");
#ifdef DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
	instance_info.enabledExtensionCount = extensions.size();
	instance_info.ppEnabledExtensionNames = extensions.data();

	// Layers
	std::vector<const char *> layers;
#ifdef DEBUG
	layers = get_required_validation_layers();
#endif
	instance_info.enabledLayerCount = layers.size();
	instance_info.ppEnabledLayerNames = layers.data();

	// Debugging instance creation
#ifdef DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
	debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_create_info.pfnUserCallback = debug_callback;

	instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
#endif

	if (vkCreateInstance(&instance_info, nullptr, &_instance) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create the vulkan instance!");
		return false;
	}

	return true;
}

bool VulkanInstance::init_debug_messenger()
{
	VkDebugUtilsMessengerCreateInfoEXT create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_callback;
	create_info.pUserData = nullptr;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance(), "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance(), &create_info, nullptr, &_debug_messenger);
	} else {
		CORE_ERROR("Couldn't load function: vkCreateDebugUtilsMessengerEXT");
		return false;
	}
}

bool VulkanInstance::supports_validation_layer(const std::vector<const char*>& required_layers)
{
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	for (const char* layer_name : required_layers) {
		bool layerFound = false;

		for (const auto& layer_properties : available_layers) {
			if (strcmp(layer_name, layer_properties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void VulkanInstance::shutdown()
{
#ifdef DEBUG
	destroy_debug_messenger();
#endif
	vkDestroyInstance(_instance, nullptr);
}

std::vector<const char *> VulkanInstance::get_required_validation_layers()
{
	std::vector<const char *> required_layers;
	required_layers.push_back("VK_LAYER_KHRONOS_validation");

	return required_layers;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messsage_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data)
{
	(void)message_type;
	(void)p_user_data;

	if (messsage_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

	return VK_FALSE;
}

void VulkanInstance::destroy_debug_messenger()
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance(), "vkDestroyDebugUtilsMessengerEXT");
	if (func)
		func(instance(), _debug_messenger, nullptr);
}

}
