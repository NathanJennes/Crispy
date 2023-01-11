//
// Created by nathan on 1/10/23.
//

#include <vector>
#include <cstring>
#include <iostream>
#include "VulkanInstance.h"
#include "log.h"
#include "defines.h"

namespace Vulkan {

VkInstance					VulkanInstance::_instance;
VkDebugUtilsMessengerEXT	VulkanInstance::_debug_messenger;
VkPhysicalDevice			VulkanInstance::_physical_device;
VkDevice					VulkanInstance::_logical_device;
VkQueue						VulkanInstance::_graphics_queue;

bool VulkanInstance::initialize()
{
	if (!init_instance())
		return false;
	CORE_TRACE("Vulkan instance initialized!");

#ifdef DEBUG
	if (!init_debug_messenger())
		return false;
	CORE_TRACE("Vulkan validation layers initialized!");
#endif

	if (!pick_physical_device())
		return false;
	CORE_TRACE("Physical device picked!");

	if (!init_logical_device())
		return false;
	CORE_TRACE("Logical device initialized!");

	return true;
}

bool VulkanInstance::init_instance()
{
#ifdef DEBUG
	// Check early if validation layers are supported
	if (!supports_validation_layer(get_required_validation_layers()))
	{
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
	std::vector<const char *> extensions;
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
	debug_create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_create_info.pfnUserCallback = debug_callback;

	instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debug_create_info;
#endif

	if (vkCreateInstance(&instance_info, nullptr, &_instance) != VK_SUCCESS)
	{
		CORE_ERROR("Couldn't create the vulkan instance!");
		return false;
	}

	return true;
}

bool VulkanInstance::init_debug_messenger()
{
	VkDebugUtilsMessengerCreateInfoEXT create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_callback;
	create_info.pUserData = nullptr;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance(),
		"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance(), &create_info, nullptr, &_debug_messenger) == VK_SUCCESS;
	} else
	{
		CORE_ERROR("Couldn't load function: vkCreateDebugUtilsMessengerEXT");
		return false;
	}
}

bool VulkanInstance::supports_validation_layer(const std::vector<const char *> &required_layers)
{
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	for (const char *layer_name: required_layers)
	{
		bool layerFound = false;

		for (const auto &layer_properties: available_layers)
		{
			if (strcmp(layer_name, layer_properties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

void VulkanInstance::shutdown()
{
	vkDestroyDevice(logical_device(), nullptr);
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

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messsage_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
	void *p_user_data)
{
	(void) message_type;
	(void) p_user_data;

	if (messsage_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

	return VK_FALSE;
}

void VulkanInstance::destroy_debug_messenger()
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance(),
		"vkDestroyDebugUtilsMessengerEXT");
	if (func)
		func(instance(), _debug_messenger, nullptr);
}

bool VulkanInstance::pick_physical_device()
{
	std::vector<VkPhysicalDevice> devices = get_physical_device_list();
	if (devices.size() == 0)
	{
		CORE_ERROR("No physical device that supports vulkan was found!");
		return false;
	}

#ifdef DEBUG
	CORE_DEBUG("Available GPU devices: ");
	for (auto &device: devices)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);
		std::string text("Name: ");
		text += properties.deviceName;
		CORE_DEBUG(text.c_str());
		text = "API version: ";
		text += std::to_string(VK_API_VERSION_MAJOR(properties.apiVersion));
		text += ".";
		text += std::to_string(VK_API_VERSION_MINOR(properties.apiVersion));
		text += ".";
		text += std::to_string(VK_API_VERSION_PATCH(properties.apiVersion));
		CORE_DEBUG(text.c_str());
	}
#endif

	_physical_device = pick_best_device(devices);

#ifdef DEBUG
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(_physical_device, &properties);
	std::string text("Device picked: ");
	text += properties.deviceName;
	CORE_TRACE(text.c_str());
#endif

	return true;
}

std::vector<VkPhysicalDevice> VulkanInstance::get_physical_device_list()
{
	u32 device_count = 0;
	vkEnumeratePhysicalDevices(instance(), &device_count, nullptr);

	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(instance(), &device_count, devices.data());
	return devices;
}

bool VulkanInstance::is_physical_device_suitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);
	//VkPhysicalDeviceFeatures features{};
	//vkGetPhysicalDeviceFeatures(device, &features);
	QueueFamilyIndicies queue_families = get_queues_for_device(device);

	return properties.apiVersion >= VK_API_VERSION_1_3 && queue_families.is_complete();
}

VkPhysicalDevice VulkanInstance::pick_best_device(const std::vector<VkPhysicalDevice> &devices)
{
	if (devices.empty())
		return VK_NULL_HANDLE;

	VkPhysicalDevice best = devices[0];
	u32 best_score = rate_physical_device(best);
	for (auto &device: devices)
	{
		u32 score = rate_physical_device(device);
		if (score > best_score)
		{
			best_score = score;
			best = device;
		}
	}

	return best;
}

u32 VulkanInstance::rate_physical_device(VkPhysicalDevice device)
{
	if (!is_physical_device_suitable(device))
		return 0;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);

	VkPhysicalDeviceMemoryProperties memory{};
	vkGetPhysicalDeviceMemoryProperties(device, &memory);

	u32 score = 0;

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 100;

	u32 gigas = 0;
	for (u32 i = 0; i < memory.memoryHeapCount; i++)
		gigas += memory.memoryHeaps[i].size;

	score += gigas / 1000000000;
	return score;
}

VulkanInstance::QueueFamilyIndicies VulkanInstance::get_queues_for_device(VkPhysicalDevice device)
{
	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, properties.data());

	QueueFamilyIndicies indices{};
	u32 i = 0;
	for (const auto &queue_family: properties)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphics_index = i;

		if (indices.is_complete())
			break;
		i++;
	}

	return indices;
}

bool VulkanInstance::init_logical_device()
{
	QueueFamilyIndicies queues = get_queues_for_device(physical_device());

	VkDeviceQueueCreateInfo queue_infos{};
	queue_infos.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_infos.queueFamilyIndex = queues.graphics_index.value();
	queue_infos.queueCount = 1;
	float queue_priority = 1.0f;
	queue_infos.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures device_features{};

	VkDeviceCreateInfo device_infos{};
	device_infos.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_infos.pQueueCreateInfos = &queue_infos;
	device_infos.queueCreateInfoCount = 1;
	device_infos.pEnabledFeatures = &device_features;
	device_infos.enabledExtensionCount = 0;
	device_infos.enabledLayerCount = 0;

#ifdef DEBUG
	std::vector<const char*> required_layers = get_required_validation_layers();
	device_infos.ppEnabledLayerNames = required_layers.data();
	device_infos.enabledLayerCount = required_layers.size();
#endif

	if (vkCreateDevice(physical_device(), &device_infos, nullptr, &_logical_device) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a logical device!");
		return false;
	}

	vkGetDeviceQueue(logical_device(), queues.graphics_index.value(), 0, &_graphics_queue);

	return true;
}
}
