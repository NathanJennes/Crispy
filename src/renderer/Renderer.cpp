//
// Created by nathan on 1/10/23.
//

#include <limits>
#include "Renderer.h"

#include "vulkan/VulkanInstance.h"
#include "vulkan/SwapchainManager.h"
#include "vulkan/GraphicsPipeline.h"
#include "vulkan/CommandBuffers.h"
#include "log.h"

namespace Vulkan {

VkSemaphore	Renderer::_image_available_semaphore;
VkSemaphore	Renderer::_render_finished_semaphore;
VkFence		Renderer::_in_flight_fence;

bool Renderer::initialize()
{
	if (!VulkanInstance::initialize())
		return false;
	if (!SwapchainManager::initialize())
		return false;
	if (!GraphicsPipeline::initialize())
		return false;
	if (!SwapchainManager::create_framebuffers())
		return false;
	if (!CommandBuffers::initialize())
		return false;
	if (!create_sync_objects())
		return false;
	return true;
}

void Renderer::shutdown()
{
	vkDestroySemaphore(VulkanInstance::logical_device(), image_available_semaphore(), nullptr);
	vkDestroySemaphore(VulkanInstance::logical_device(), render_finished_semaphore(), nullptr);
	vkDestroyFence(VulkanInstance::logical_device(), in_flight_fence(), nullptr);

	CommandBuffers::shutdown();
	GraphicsPipeline::shutdown();
	SwapchainManager::shutdown();
	VulkanInstance::shutdown();
}

void Renderer::draw_frame()
{
	vkWaitForFences(VulkanInstance::logical_device(), 1, &_in_flight_fence, VK_TRUE, std::numeric_limits<u64>::max());
	vkResetFences(VulkanInstance::logical_device(), 1, &_in_flight_fence);

	u32 image_index;
	vkAcquireNextImageKHR(VulkanInstance::logical_device(), SwapchainManager::swapchain(),
		std::numeric_limits<u64>::max(), image_available_semaphore(), VK_NULL_HANDLE, &image_index);

	vkResetCommandBuffer(CommandBuffers::command_buffer(), 0);
	CommandBuffers::record_command_buffer(CommandBuffers::command_buffer(), image_index);

	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkCommandBuffer command_buffers[] = {CommandBuffers::command_buffer()};
	VkSubmitInfo submit_infos{};
	submit_infos.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_infos.waitSemaphoreCount = 1;
	submit_infos.pWaitSemaphores = &_image_available_semaphore;
	submit_infos.pWaitDstStageMask = wait_stages;
	submit_infos.commandBufferCount = 1;
	submit_infos.pCommandBuffers = command_buffers;
	submit_infos.signalSemaphoreCount = 1;
	submit_infos.pSignalSemaphores = &_render_finished_semaphore;

	if (vkQueueSubmit(VulkanInstance::graphics_queue(), 1, &submit_infos, in_flight_fence()) != VK_SUCCESS) {
		CORE_ERROR("Couldn't submit draw command buffer!");
	}

	VkSwapchainKHR swapchains[] = {SwapchainManager::swapchain()};

	VkPresentInfoKHR present_infos{};
	present_infos.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_infos.waitSemaphoreCount = 1;
	present_infos.pWaitSemaphores = &_render_finished_semaphore;
	present_infos.swapchainCount = 1;
	present_infos.pSwapchains = swapchains;
	present_infos.pImageIndices = &image_index;
	present_infos.pResults = nullptr;

	if (vkQueuePresentKHR(VulkanInstance::present_queue(), &present_infos) != VK_SUCCESS) {
		CORE_ERROR("Couldn't present the finished rendered framebuffer!");
	}
}

bool Renderer::create_sync_objects()
{
	VkSemaphoreCreateInfo semaphore_infos{};
	semaphore_infos.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_infos{};
	fence_infos.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_infos.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(VulkanInstance::logical_device(), &semaphore_infos, nullptr, &_image_available_semaphore) != VK_SUCCESS ||
		vkCreateSemaphore(VulkanInstance::logical_device(), &semaphore_infos, nullptr, &_render_finished_semaphore) != VK_SUCCESS ||
		vkCreateFence(VulkanInstance::logical_device(), &fence_infos, nullptr, &_in_flight_fence) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create renderer sync objects!");
		return false;
	}

	return true;
}
}