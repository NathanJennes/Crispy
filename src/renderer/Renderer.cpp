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

std::vector<VkSemaphore>	Renderer::_image_available_semaphores;
std::vector<VkSemaphore>	Renderer::_render_finished_semaphores;
std::vector<VkFence>		Renderer::_in_flight_fences;
const u32					Renderer::_frames_in_flight_count = 2;
u32							Renderer::_current_frame = 0;

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
	if (!CommandBuffers::initialize(frames_in_flight_count()))
		return false;
	if (!create_sync_objects())
		return false;
	return true;
}

void Renderer::shutdown()
{
	for (u32 i = 0; i < frames_in_flight_count(); i++) {
		vkDestroySemaphore(VulkanInstance::logical_device(), image_available_semaphores()[i], nullptr);
		vkDestroySemaphore(VulkanInstance::logical_device(), render_finished_semaphores()[i], nullptr);
		vkDestroyFence(VulkanInstance::logical_device(), in_flight_fences()[i], nullptr);
	}

	CommandBuffers::shutdown();
	GraphicsPipeline::shutdown();
	SwapchainManager::shutdown();
	VulkanInstance::shutdown();
}

void Renderer::draw_frame()
{
	vkWaitForFences(VulkanInstance::logical_device(), 1, &in_flight_fences()[current_frame()], VK_TRUE, std::numeric_limits<u64>::max());
	vkResetFences(VulkanInstance::logical_device(), 1, &in_flight_fences()[current_frame()]);

	u32 image_index;
	vkAcquireNextImageKHR(VulkanInstance::logical_device(), SwapchainManager::swapchain(),
		std::numeric_limits<u64>::max(), image_available_semaphores()[current_frame()], VK_NULL_HANDLE, &image_index);

	vkResetCommandBuffer(CommandBuffers::get(current_frame()), 0);
	CommandBuffers::record_command_buffer(CommandBuffers::get(current_frame()), image_index);

	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_infos{};
	submit_infos.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_infos.waitSemaphoreCount = 1;
	submit_infos.pWaitSemaphores = &image_available_semaphores()[current_frame()];
	submit_infos.pWaitDstStageMask = wait_stages;
	submit_infos.commandBufferCount = 1;
	submit_infos.pCommandBuffers = &CommandBuffers::get(current_frame());
	submit_infos.signalSemaphoreCount = 1;
	submit_infos.pSignalSemaphores = &render_finished_semaphores()[current_frame()];

	if (vkQueueSubmit(VulkanInstance::graphics_queue(), 1, &submit_infos, in_flight_fences()[current_frame()]) != VK_SUCCESS) {
		CORE_ERROR("Couldn't submit draw command buffer!");
	}

	VkSwapchainKHR swapchains[] = {SwapchainManager::swapchain()};

	VkPresentInfoKHR present_infos{};
	present_infos.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_infos.waitSemaphoreCount = 1;
	present_infos.pWaitSemaphores = &render_finished_semaphores()[current_frame()];
	present_infos.swapchainCount = 1;
	present_infos.pSwapchains = swapchains;
	present_infos.pImageIndices = &image_index;
	present_infos.pResults = nullptr;

	if (vkQueuePresentKHR(VulkanInstance::present_queue(), &present_infos) != VK_SUCCESS) {
		CORE_ERROR("Couldn't present the finished rendered framebuffer!");
	}

	_current_frame = (current_frame() + 1) % frames_in_flight_count();
}

bool Renderer::create_sync_objects()
{
	VkSemaphoreCreateInfo semaphore_infos{};
	semaphore_infos.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_infos{};
	fence_infos.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_infos.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	image_available_semaphores().resize(frames_in_flight_count());
	render_finished_semaphores().resize(frames_in_flight_count());
	in_flight_fences().resize(frames_in_flight_count());

	for (u32 i = 0; i < frames_in_flight_count(); i++) {
		if (vkCreateSemaphore(VulkanInstance::logical_device(), &semaphore_infos, nullptr, &image_available_semaphores()[i]) != VK_SUCCESS ||
			vkCreateSemaphore(VulkanInstance::logical_device(), &semaphore_infos, nullptr, &render_finished_semaphores()[i]) != VK_SUCCESS ||
			vkCreateFence(VulkanInstance::logical_device(), &fence_infos, nullptr, &in_flight_fences()[i]) != VK_SUCCESS) {
			CORE_ERROR("Couldn't create renderer sync objects!");
			return false;
		}
	}

	return true;
}
}