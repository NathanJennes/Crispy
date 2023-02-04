//
// Created by nathan on 1/10/23.
//

#include <vector>
#include <cstring>
#include <iostream>
#include <set>

#include "VulkanInstance.h"
#include "log.h"
#include "defines.h"
#include "Window.h"
#include "SwapchainManager.h"
#include "core/crispy_core.h"
#include "vulkan_errors.h"

namespace Vulkan {

VkInstance					VulkanInstance::_instance;
VkDebugUtilsMessengerEXT	VulkanInstance::_debug_messenger;
VkPhysicalDevice			VulkanInstance::_physical_device;
VkDevice					VulkanInstance::_logical_device;
VkQueue						VulkanInstance::_graphics_queue;
VkQueue						VulkanInstance::_present_queue;

VkFence						VulkanInstance::immediate_fence = VK_NULL_HANDLE;
VkCommandPool				VulkanInstance::immediate_command_pool = VK_NULL_HANDLE;
VkCommandBuffer				VulkanInstance::immediate_command_buffer = VK_NULL_HANDLE;

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

	if (!Window::initialize_surface())
		return false;
	CORE_TRACE("Window surface created!");

	if (!pick_physical_device())
		return false;
	CORE_TRACE("Physical device picked!");

	if (!init_logical_device())
		return false;
	CORE_TRACE("Logical device initialized!");

	if (!create_immediate_objects())
		return false;
	CORE_TRACE("Vulkan Instance's immediate objects created");

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
// TODO: check at runtime hardware api capabilities
	IN_MACOS(app_info.apiVersion = VK_API_VERSION_1_3);
	IN_LINUX(app_info.apiVersion = VK_API_VERSION_1_3);

	VkInstanceCreateInfo instance_info{};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pNext = nullptr;
	instance_info.pApplicationInfo = &app_info;
	IN_MACOS(instance_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR);

	// Extensions
	std::vector<const char *> extensions(Window::get_required_instance_extensions());
	IN_MACOS(extensions.push_back("VK_KHR_portability_enumeration"));
	IN_DEBUG(extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
	instance_info.enabledExtensionCount = extensions.size();
	instance_info.ppEnabledExtensionNames = extensions.data();

	// Layers
	std::vector<const char *> layers;
	IN_DEBUG(layers = get_required_validation_layers());
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
	if (func != nullptr) {
		return func(instance(), &create_info, nullptr, &_debug_messenger) == VK_SUCCESS;
	} else {
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

	for (const char *layer_name: required_layers) {
		bool layerFound = false;

		for (const auto &layer_properties: available_layers) {
			if (strcmp(layer_name, layer_properties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

void VulkanInstance::shutdown()
{
	destroy_immediate_objects();
	vkDestroyDevice(logical_device(), nullptr);
	Window::destroy_surface();
	IN_DEBUG(destroy_debug_messenger());
	vkDestroyInstance(instance(), nullptr);
}

std::vector<const char *> VulkanInstance::get_required_validation_layers()
{
	std::vector<const char *> required_layers;
	required_layers.push_back("VK_LAYER_KHRONOS_validation");

	return required_layers;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
	void *p_user_data)
{
	(void) message_type;
	(void) p_user_data;

	if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
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
	if (devices.empty()) {
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
	QueueFamilyIndices queue_families = get_queues_for_device(device);

	bool meets_extensions_requirements = true;
	u32 extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());
	std::vector<const char*> required_extensions = get_required_device_extensions();
	for (const auto& required_extension : required_extensions) {
		bool found = false;
		for (const auto &available_extension: available_extensions) {
			if (strcmp(available_extension.extensionName, required_extension) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			meets_extensions_requirements = false;
			break;
		}
	}

	return properties.apiVersion >= VK_API_VERSION_1_3 && queue_families.is_complete() && meets_extensions_requirements && SwapchainManager::is_device_capable(device);
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
		if (score > best_score) {
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

	u32 gigs = 0;
	for (u32 i = 0; i < memory.memoryHeapCount; i++)
		gigs += memory.memoryHeaps[i].size;

	score += gigs / 1000000000;
	return score;
}

QueueFamilyIndices VulkanInstance::get_queues_for_device(VkPhysicalDevice device)
{
	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, properties.data());

	QueueFamilyIndices indices{};
	u32 i = 0;
	for (const auto &queue_family: properties)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphics_index = i;

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Window::get_surface(), &present_support);
		if (present_support)
			indices.present_index = i;

		if (indices.is_complete())
			break;
		i++;
	}

	return indices;
}

bool VulkanInstance::init_logical_device()
{
	QueueFamilyIndices queues = get_queues_for_device(physical_device());

	std::vector<VkDeviceQueueCreateInfo> queues_infos;
	std::set<uint32_t> unique_queue_families = {queues.graphics_index.value(), queues.present_index.value()};

	float queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_info{};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.queueFamilyIndex = queue_family;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = &queue_priority;
		queues_infos.push_back(queue_info);
	}

	VkPhysicalDeviceFeatures device_features{};

	std::vector<const char*> device_extensions = get_required_device_extensions();

	VkDeviceCreateInfo device_infos{};
	device_infos.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_infos.pQueueCreateInfos = queues_infos.data();
	device_infos.queueCreateInfoCount = queues_infos.size();
	device_infos.pEnabledFeatures = &device_features;
	device_infos.enabledExtensionCount = device_extensions.size();
	device_infos.ppEnabledExtensionNames = device_extensions.data();
	device_infos.enabledLayerCount = 0;

	IN_DEBUG(std::vector<const char*> required_layers = get_required_validation_layers());
	IN_DEBUG(device_infos.ppEnabledLayerNames = required_layers.data());
	IN_DEBUG(device_infos.enabledLayerCount = required_layers.size());

	if (vkCreateDevice(physical_device(), &device_infos, nullptr, &_logical_device) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a logical device!");
		return false;
	}

	vkGetDeviceQueue(logical_device(), queues.graphics_index.value(), 0, &_graphics_queue);
	vkGetDeviceQueue(logical_device(), queues.present_index.value(), 0, &_present_queue);

	return true;
}

std::vector<const char *> VulkanInstance::get_required_device_extensions()
{
	std::vector<const char*> requirements;
	requirements.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	IN_MACOS(requirements.push_back("VK_KHR_portability_subset"));
	return requirements;
}

bool VulkanInstance::create_immediate_objects()
{
	QueueFamilyIndices queue_indices = get_queues_for_device(physical_device());

	VkCommandPoolCreateInfo pool_create_infos{};
	pool_create_infos.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_infos.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_infos.queueFamilyIndex = queue_indices.graphics_index.value();

	VkResult result = vkCreateCommandPool(logical_device(), &pool_create_infos, nullptr, &immediate_command_pool);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't create VulkanInstance's immediate command pool: %s", vulkan_error_to_string(result));
		return false;
	}

	VkCommandBufferAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_infos.commandPool = immediate_command_pool;
	alloc_infos.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_infos.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(logical_device(), &alloc_infos, &immediate_command_buffer);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't create VulkanInstance's immediate command buffer: %s", vulkan_error_to_string(result));
		return false;
	}

	VkFenceCreateInfo fence_infos{};
	fence_infos.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	result = vkCreateFence(logical_device(), &fence_infos, nullptr, &immediate_fence);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't create VulkanInstance's immediate fence!");
		return false;
	}

	return true;
}

void VulkanInstance::destroy_immediate_objects()
{
	if (immediate_command_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(logical_device(), immediate_command_pool, nullptr);
	if (immediate_fence != VK_NULL_HANDLE)
		vkDestroyFence(logical_device(), immediate_fence, nullptr);
}

void VulkanInstance::immediate_submit(std::function<void(VkCommandBuffer cmd_buffer)>&& function)
{
	VkCommandBufferBeginInfo begin_infos{};
	begin_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_infos.flags = 0;
	begin_infos.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(immediate_command_buffer, &begin_infos);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't begin BasicRenderer's command buffer: %s", vulkan_error_to_string(result));
		return ;
	}

	function(immediate_command_buffer);

	result = vkEndCommandBuffer(immediate_command_buffer);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't end command buffer: %s", vulkan_error_to_string(result));
		return ;
	}

	VkSubmitInfo submit_infos{};
	submit_infos.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_infos.pCommandBuffers = &immediate_command_buffer;


	result = vkQueueSubmit(graphics_queue(), 1, &submit_infos, immediate_fence);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't submit VulkanInstance's immediate command buffer: %s", vulkan_error_to_string(result));
		return ;
	}

	vkWaitForFences(logical_device(), 1, &immediate_fence, VK_TRUE, std::numeric_limits<u64>::max());
	vkResetFences(logical_device(), 1, &immediate_fence);
	vkResetCommandPool(logical_device(), immediate_command_pool, 0);
}
}
