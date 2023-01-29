//
// Created by nathan on 1/10/23.
//

#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "defines.h"

namespace Vulkan {

struct QueueFamilyIndices
{
private:
	using index = std::optional<u32>;

public:
	index	graphics_index;
	index	present_index;

	bool is_complete() const { return graphics_index.has_value() && present_index.has_value(); }
};

class VulkanInstance
{
public:
	//----
	// ON/OFF
	//----
	static bool	initialize();
	static void	shutdown();

	//----
	// Getters
	//----
	static VkInstance&			instance()			{ return _instance; }
	static VkPhysicalDevice&	physical_device()	{ return _physical_device; }
	static VkDevice&			logical_device()	{ return _logical_device; }
	static VkQueue&				graphics_queue()	{ return _graphics_queue; }
	static VkQueue&				present_queue()		{ return _present_queue; }

	static QueueFamilyIndices	get_queues_for_device(VkPhysicalDevice device);

private:	// Methods
	//----
	// Initialization
	//----
	static bool	init_instance();

	//---
	// Physical Device management
	//---
	static bool								pick_physical_device();
	static std::vector<VkPhysicalDevice>	get_physical_device_list();
	static bool								is_physical_device_suitable(VkPhysicalDevice device);
	static VkPhysicalDevice					pick_best_device(const std::vector<VkPhysicalDevice>& devices);
	static u32								rate_physical_device(VkPhysicalDevice device);

	//----
	// Logical Device management
	//----
	static bool	init_logical_device();
	static std::vector<const char*>	get_required_device_extensions();

	//----
	// Debug
	//----
	static bool								init_debug_messenger();
	static void								destroy_debug_messenger();
	static bool								supports_validation_layer(const std::vector<const char*>& required_layers);
	static std::vector<const char*>			get_required_validation_layers();
	static VKAPI_ATTR VkBool32 VKAPI_CALL	debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
		void* p_user_data);

	//----
	// Getters
	//----
	static VkDebugUtilsMessengerEXT&	debug_messenger()	{ return _debug_messenger; }

private:	// Members
	static VkInstance				_instance;
	static VkDebugUtilsMessengerEXT	_debug_messenger;
	static VkPhysicalDevice			_physical_device;
	static VkDevice					_logical_device;
	static VkQueue					_graphics_queue;
	static VkQueue					_present_queue;

};

}


#endif //VULKANINSTANCE_H
