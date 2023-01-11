//
// Created by nathan on 1/11/23.
//

#ifndef SWAPCHAINMANAGER_H
#define SWAPCHAINMANAGER_H

#include <vulkan/vulkan.h>
#include <vector>

namespace Vulkan {

class SwapChainManager
{
public:
	static bool is_device_capable(VkPhysicalDevice device);

private:    // Types
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

private:    // Methods
	static SwapchainSupportDetails get_device_swapchain_capabilities(VkPhysicalDevice device);
};
}

#endif //SWAPCHAINMANAGER_H
