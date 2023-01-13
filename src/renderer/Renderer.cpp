//
// Created by nathan on 1/10/23.
//

#include <limits>
#include <cstring>
#include "Renderer.h"

#include "vulkan/VulkanInstance.h"
#include "vulkan/SwapchainManager.h"
#include "vulkan/GraphicsPipeline.h"
#include "vulkan/CommandBuffers.h"
#include "log.h"
#include "Window.h"
#include "math/vulkan_maths.h"

namespace Vulkan {

std::vector<VkSemaphore>	Renderer::_image_available_semaphores;
std::vector<VkSemaphore>	Renderer::_render_finished_semaphores;
std::vector<VkFence>		Renderer::_in_flight_fences;
const u32					Renderer::_frames_in_flight_count = 2;
u32							Renderer::_current_frame = 0;
VkBuffer					Renderer::_vertex_buffer = VK_NULL_HANDLE;
VkDeviceMemory				Renderer::_vertex_buffer_memory = VK_NULL_HANDLE;
u32							Renderer::_vertex_buffer_capacity = 3 * 10;

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
	if (!create_vertex_buffer())
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

	vkDestroyBuffer(VulkanInstance::logical_device(), vertex_buffer(), nullptr);
	vkFreeMemory(VulkanInstance::logical_device(), vertex_buffer_memory(), nullptr);

	GraphicsPipeline::shutdown();
	SwapchainManager::shutdown();
	VulkanInstance::shutdown();
}


void Renderer::draw(const std::vector<Vertex> &verticies)
{
	draw_call(verticies);
}

void Renderer::draw_call(const std::vector<Vertex>& verticies)
{
	vkWaitForFences(VulkanInstance::logical_device(), 1, &in_flight_fences()[current_frame()], VK_TRUE, std::numeric_limits<u64>::max());

	u32 image_index;
	VkResult result = vkAcquireNextImageKHR(VulkanInstance::logical_device(), SwapchainManager::swapchain(),
		std::numeric_limits<u64>::max(), image_available_semaphores()[current_frame()], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		SwapchainManager::recreate();
		return ;
	} else if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't acquire next swapchain image for rendering!");
		return ;
	}

	vkResetFences(VulkanInstance::logical_device(), 1, &in_flight_fences()[current_frame()]);

	vkResetCommandBuffer(CommandBuffers::get(current_frame()), 0);

	fill_vertex_buffer(verticies, 0);

	if (verticies.size() > vertex_buffer_capacity())
		CommandBuffers::record_command_buffer(CommandBuffers::get(current_frame()), image_index, vertex_buffer(), vertex_buffer_capacity());
	else
		CommandBuffers::record_command_buffer(CommandBuffers::get(current_frame()), image_index, vertex_buffer(), verticies.size());

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

	result = vkQueuePresentKHR(VulkanInstance::present_queue(), &present_infos);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Window::has_resized()) {
		SwapchainManager::recreate();
	} else if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't present swap chain image!");
		return ;
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

bool Renderer::create_vertex_buffer()
{
	VkBufferCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_infos.size = sizeof(Vertex) * vertex_buffer_capacity();
	create_infos.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	create_infos.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VulkanInstance::logical_device(), &create_infos, nullptr, &_vertex_buffer) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create the vertex buffer");
		return false;
	}

	VkMemoryRequirements mem_requirements{};
	vkGetBufferMemoryRequirements(VulkanInstance::logical_device(), vertex_buffer(), &mem_requirements);

	std::optional<u32> memory_type_index = find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT).value();
	if (!memory_type_index.has_value()) {
		CORE_ERROR("Couldn't find a memory region with the right type!");
		return false;
	}

	VkMemoryAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_infos.allocationSize = mem_requirements.size;
	alloc_infos.memoryTypeIndex = memory_type_index.value();

	if (vkAllocateMemory(VulkanInstance::logical_device(), &alloc_infos, nullptr, &_vertex_buffer_memory) != VK_SUCCESS) {
		CORE_ERROR("Couldn't allocate the vertex buffer!");
		return false;
	}

	vkBindBufferMemory(VulkanInstance::logical_device(), vertex_buffer(), vertex_buffer_memory(), 0);

	return true;
}

std::optional<u32> Renderer::find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties mem_properties{};
	vkGetPhysicalDeviceMemoryProperties(VulkanInstance::physical_device(), &mem_properties);

	for (u32 i = 0; i < mem_properties.memoryTypeCount; i++) {
		if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
	return {};
}

void Renderer::fill_vertex_buffer(const std::vector<Vertex> &verticies, u32 offset)
{
	void *data;
	vkMapMemory(VulkanInstance::logical_device(), vertex_buffer_memory(), 0, sizeof(Vertex) * vertex_buffer_capacity(), 0, &data);

	if (verticies.size() - offset > vertex_buffer_capacity())
		memmove(data, &verticies[offset], vertex_buffer_capacity() * sizeof(Vertex));
	else
		memmove(data, &verticies[offset], (verticies.size() - offset) * sizeof(Vertex));

	vkUnmapMemory(VulkanInstance::logical_device(), vertex_buffer_memory());
}

}