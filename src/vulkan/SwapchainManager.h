//
// Created by nathan on 1/11/23.
//

#ifndef SWAPCHAINMANAGER_H
#define SWAPCHAINMANAGER_H

#include <vulkan/vulkan.h>
#include <vector>

namespace Vulkan {

class SwapchainManager
{
public:
	//----
	// Initialization
	//----
	static bool	initialize();
	static void	shutdown();

	//----
	// Compatibility checks
	//----
	static bool	is_device_capable(VkPhysicalDevice device);

private:	// Types
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR		capabilities;
		std::vector<VkSurfaceFormatKHR>	formats;
		std::vector<VkPresentModeKHR>	present_modes;
	};

private:	// Methods
	static SwapchainSupportDetails	get_device_swapchain_capabilities(VkPhysicalDevice device);
	static VkSurfaceFormatKHR		choose_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
	static VkPresentModeKHR			choose_present_mode(const std::vector<VkPresentModeKHR>& available_modes);
	static VkExtent2D				choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

	//----
	// Getters
	//----
	static VkSwapchainKHR		swapchain()					{ return _swapchain; };
	static std::vector<VkImage>	swapchain_images()			{ return _swapchain_images; };
	static VkFormat				swapchain_image_format()	{ return _swapchain_image_format; };
	static VkExtent2D			swapchain_extent()			{ return _swapchain_extent; };

private:	// Members
	static VkSwapchainKHR		_swapchain;
	static std::vector<VkImage>	_swapchain_images;
	static VkFormat				_swapchain_image_format;
	static VkExtent2D			_swapchain_extent;
};
}

#endif //SWAPCHAINMANAGER_H
