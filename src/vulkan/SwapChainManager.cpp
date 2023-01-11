//
// Created by nathan on 1/11/23.
//

#include "SwapChainManager.h"

#include "Window.h"

namespace Vulkan {

bool SwapChainManager::is_device_capable(VkPhysicalDevice device)
{
	SwapchainSupportDetails details = get_device_swapchain_capabilities(device);
	return !details.formats.empty() && !details.present_modes.empty();
}

SwapChainManager::SwapchainSupportDetails SwapChainManager::get_device_swapchain_capabilities(VkPhysicalDevice device)
{
	SwapchainSupportDetails details{};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Window::surface(), &details.capabilities);

	u32 format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, Window::surface(), &format_count, nullptr);
	details.formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, Window::surface(), &format_count, details.formats.data());

	u32 present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, Window::surface(), &present_mode_count, nullptr);
	details.present_modes.resize(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, Window::surface(), &present_mode_count, details.present_modes.data());

	return details;
}
}
