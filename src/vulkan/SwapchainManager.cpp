//
// Created by nathan on 1/11/23.
//

#include <limits>
#include <algorithm>
#include "SwapchainManager.h"

#include "Window.h"
#include "log.h"
#include "VulkanInstance.h"
#include "GraphicsPipeline.h"

namespace Vulkan {

VkSwapchainKHR				SwapchainManager::_swapchain;
std::vector<VkImage>		SwapchainManager::_swapchain_images;
std::vector<VkImageView>	SwapchainManager::_swapchain_image_views;
std::vector<VkFramebuffer>	SwapchainManager::_swapchain_framebuffers;
VkFormat					SwapchainManager::_swapchain_image_format;
VkExtent2D					SwapchainManager::_swapchain_extent;

bool SwapchainManager::initialize()
{
	return create_swapchain();
}

void SwapchainManager::shutdown()
{
	cleanup_swapchain();
}

bool SwapchainManager::recreate()
{
	vkDeviceWaitIdle(VulkanInstance::logical_device());
	cleanup_swapchain();

	if (!create_swapchain()) {
		CORE_ERROR("Couldn't to recreate the swapchain");
		return false;
	}
	if (!create_framebuffers()) {
		CORE_ERROR("Couldn't to recreate the swapchain");
		return false;
	}
	return true;
}

bool SwapchainManager::is_device_capable(VkPhysicalDevice device)
{
	SwapchainSupportDetails details = get_device_swapchain_capabilities(device);
	return !details.formats.empty() && !details.present_modes.empty();
}

SwapchainManager::SwapchainSupportDetails SwapchainManager::get_device_swapchain_capabilities(VkPhysicalDevice device)
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

VkSurfaceFormatKHR SwapchainManager::choose_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats)
{
#ifdef DEBUG
	if (available_formats.empty()) {
		CORE_ERROR("Available format list is empty... Something has gone very wrong...");
		return VkSurfaceFormatKHR{};
	}
#endif

	for (const auto& format : available_formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	CORE_DEBUG("Couldn't use the preferred image format for the swapchain");
	return available_formats[0];
}

VkPresentModeKHR SwapchainManager::choose_present_mode(const std::vector<VkPresentModeKHR> &available_modes)
{
	for (const auto& mode : available_modes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}

	CORE_DEBUG("Couldn't use the preffered present mode for the swapchain");
	// The only guaranteed mode to be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapchainManager::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
		return capabilities.currentExtent;
	} else {
		// On retina displays this calculation is false
		VkExtent2D actual_extent = {Window::width(), Window::height()};

		actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}
}

void SwapchainManager::create_image_views()
{
	swapchain_image_views().resize(swapchain_images().size());

	for (size_t i = 0; i < swapchain_images().size(); i++) {
		VkImageViewCreateInfo create_infos{};
		create_infos.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_infos.image = swapchain_images()[i];
		create_infos.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_infos.format = swapchain_image_format();
		create_infos.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_infos.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_infos.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_infos.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_infos.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_infos.subresourceRange.baseArrayLayer = 0;
		create_infos.subresourceRange.baseMipLevel = 0;
		create_infos.subresourceRange.layerCount = 1;
		create_infos.subresourceRange.levelCount = 1;

		if (vkCreateImageView(VulkanInstance::logical_device(), &create_infos, nullptr, &_swapchain_image_views[i]) != VK_SUCCESS) {
			CORE_ERROR("Couldn't create a swapchain image view!");
			return ;
		}
	}
}

bool SwapchainManager::create_framebuffers()
{
	swapchain_framebuffers().resize(swapchain_image_views().size());

	for (size_t i = 0; i < swapchain_image_views().size(); i++) {
		VkFramebufferCreateInfo create_infos{};
		create_infos.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_infos.renderPass = GraphicsPipeline::render_pass();
		create_infos.attachmentCount = 1;
		create_infos.pAttachments = &swapchain_image_views()[i];
		create_infos.width = swapchain_extent().width;
		create_infos.height = swapchain_extent().height;
		create_infos.layers = 1;

		if (vkCreateFramebuffer(VulkanInstance::logical_device(), &create_infos, nullptr, &swapchain_framebuffers()[i]) != VK_SUCCESS) {
			CORE_DEBUG("Couldn't create a framebuffer!");
			return false;
		}
	}

	return true;
}

void SwapchainManager::cleanup_swapchain()
{
	for (auto& framebuffer : swapchain_framebuffers())
		vkDestroyFramebuffer(VulkanInstance::logical_device(), framebuffer, nullptr);
	for (auto& image_view : swapchain_image_views())
		vkDestroyImageView(VulkanInstance::logical_device(), image_view, nullptr);
	vkDestroySwapchainKHR(VulkanInstance::logical_device(), swapchain(), nullptr);
}

bool SwapchainManager::create_swapchain()
{
	SwapchainSupportDetails swapchain_support = get_device_swapchain_capabilities(VulkanInstance::physical_device());

	VkSurfaceFormatKHR surface_format = choose_surface_format(swapchain_support.formats);
	VkPresentModeKHR present_mode = choose_present_mode(swapchain_support.present_modes);
	VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);

	u32 image_count = swapchain_support.capabilities.minImageCount + 1;
	if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
		image_count = swapchain_support.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_infos.surface = Window::surface();
	create_infos.minImageCount = image_count;
	create_infos.imageFormat = surface_format.format;
	create_infos.imageColorSpace = surface_format.colorSpace;
	create_infos.imageExtent = extent;
	create_infos.imageArrayLayers = 1;
	create_infos.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = VulkanInstance::get_queues_for_device(VulkanInstance::physical_device());
	u32 indices_array[2] = { indices.graphics_index.value(), indices.present_index.value() };
	if (indices.graphics_index.value() == indices.present_index.value()) {
		create_infos.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_infos.pQueueFamilyIndices = indices_array;
		create_infos.queueFamilyIndexCount = 2;
	} else {
		create_infos.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_infos.pQueueFamilyIndices = nullptr;
		create_infos.queueFamilyIndexCount = 0;
	}

	create_infos.preTransform = swapchain_support.capabilities.currentTransform;
	create_infos.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_infos.presentMode = present_mode;
	create_infos.clipped = VK_TRUE;
	create_infos.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(VulkanInstance::logical_device(), &create_infos, nullptr, &_swapchain) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a swapchain!");
		return false;
	}

	vkGetSwapchainImagesKHR(VulkanInstance::logical_device(), swapchain(), &image_count, nullptr);
	swapchain_images().resize(image_count);
	vkGetSwapchainImagesKHR(VulkanInstance::logical_device(), swapchain(), &image_count, swapchain_images().data());
	_swapchain_extent = extent;
	_swapchain_image_format = surface_format.format;

	create_image_views();

	return true;
}
}
