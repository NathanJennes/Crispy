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

#include "glm/gtc/matrix_transform.hpp"
#include "Vertex.h"

namespace Vulkan {

std::vector<VkSemaphore>		Renderer::_image_available_semaphores;
std::vector<VkSemaphore>		Renderer::_render_finished_semaphores;
std::vector<VkFence>			Renderer::_in_flight_fences;
const u32						Renderer::_frames_in_flight_count = 2;
u32								Renderer::_current_frame = 0;
u32								Renderer::_vertex_buffer_capacity = 3 * 10;
Buffer*							Renderer::_vertex_buffer = nullptr;
Buffer*							Renderer::_vertex_staging_buffer = nullptr;
u32								Renderer::_index_buffer_capacity = 100;
Buffer*							Renderer::_index_buffer = nullptr;
Buffer*							Renderer::_index_staging_buffer = nullptr;
VkDescriptorPool				Renderer::_descriptor_pool = VK_NULL_HANDLE;
std::vector<VkDescriptorSet>	Renderer::_descriptor_sets;
std::vector<Buffer*>			Renderer::_uniform_buffers;


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
	if (!create_buffers())
		return false;
	if (!create_descriptor_pool())
		return false;
	if (!create_descriptor_sets())
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

	delete _vertex_buffer;
	delete _vertex_staging_buffer;
	delete _index_buffer;
	delete _index_staging_buffer;

	for (auto& buffer : _uniform_buffers)
		delete buffer;

	vkDestroyDescriptorPool(VulkanInstance::logical_device(), _descriptor_pool, nullptr);

	GraphicsPipeline::shutdown();
	SwapchainManager::shutdown();
	VulkanInstance::shutdown();
}

void Renderer::draw(const std::vector<Vertex> &verticies, const std::vector<u16> &indices, const glm::vec3& pos)
{
	draw_call(verticies, indices, pos);
}

void Renderer::draw_call(const std::vector<Vertex>& verticies, const std::vector<u16>& indices, const glm::vec3& pos)
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
	fill_index_buffer(indices, 0);
	fill_uniform_buffer(pos);

	if (indices.size() >index_buffer_capacity()) {
		CommandBuffers::record_command_buffer(CommandBuffers::get(current_frame()), image_index, vertex_buffer()->buffer(),
			index_buffer()->buffer(), index_buffer_capacity(), _descriptor_sets[current_frame()]);
	} else {
		CommandBuffers::record_command_buffer(CommandBuffers::get(current_frame()), image_index, vertex_buffer()->buffer(),
			index_buffer()->buffer(), indices.size(), _descriptor_sets[current_frame()]);
	}


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
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Window::did_resize()) {
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

bool Renderer::create_buffers()
{
	_vertex_buffer = new Buffer(sizeof(Vertex) * vertex_buffer_capacity(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_vertex_staging_buffer = new Buffer(sizeof(Vertex) * vertex_buffer_capacity(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	_index_buffer = new Buffer(sizeof(u16) * index_buffer_capacity(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_index_staging_buffer = new Buffer(sizeof(u16) * index_buffer_capacity(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	_uniform_buffers.resize(frames_in_flight_count());
	for (u32 i = 0; i < frames_in_flight_count(); i++) {
		_uniform_buffers[i] = new Buffer(sizeof(CameraUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	return true;
}

void Renderer::fill_vertex_buffer(const std::vector<Vertex> &verticies, u32 offset)
{
	vertex_staging_buffer()->set_data(verticies, 0);
	vertex_staging_buffer()->copy_to(*vertex_buffer(), offset, verticies.size() * sizeof(Vertex), 0);
}

void Renderer::fill_index_buffer(const std::vector<u16> &indices, u32 offset)
{
	index_staging_buffer()->set_data(indices, 0);
	index_staging_buffer()->copy_to(*index_buffer(), offset, indices.size() * sizeof(u16), 0);
}

void Renderer::fill_uniform_buffer(const glm::vec3 &pos)
{
	(void)pos;
	CameraUBO ubo{};
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f),
		(float) SwapchainManager::swapchain_extent().width / (float) SwapchainManager::swapchain_extent().height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	_uniform_buffers[current_frame()]->set_data(&ubo, sizeof(CameraUBO));
}

bool Renderer::create_descriptor_pool()
{
	VkDescriptorPoolSize pool_size{};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = static_cast<uint32_t>(frames_in_flight_count());

	VkDescriptorPoolCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_infos.poolSizeCount = 1;
	create_infos.pPoolSizes = &pool_size;
	create_infos.maxSets = static_cast<uint32_t>(frames_in_flight_count());

	if (vkCreateDescriptorPool(VulkanInstance::logical_device(), &create_infos, nullptr, &_descriptor_pool) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a descrpitor pool");
		return false;
	}
	return true;
}

bool Renderer::create_descriptor_sets()
{
	std::vector<VkDescriptorSetLayout> layouts(frames_in_flight_count(), GraphicsPipeline::descriptor_set_layout());
	VkDescriptorSetAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_infos.descriptorPool = _descriptor_pool;
	alloc_infos.descriptorSetCount = frames_in_flight_count();
	alloc_infos.pSetLayouts = layouts.data();

	_descriptor_sets.resize(frames_in_flight_count());
	if (vkAllocateDescriptorSets(VulkanInstance::logical_device(), &alloc_infos, _descriptor_sets.data()) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create descriptor sets");
		return false;
	}

	for (u32 i = 0; i < frames_in_flight_count(); i++) {
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = _uniform_buffers[i]->buffer();
		buffer_info.offset = 0;
		buffer_info.range = sizeof(CameraUBO);

		VkWriteDescriptorSet desc_write{};
		desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc_write.dstSet = _descriptor_sets[i];
		desc_write.dstBinding = 0;
		desc_write.dstArrayElement = 0;
		desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		desc_write.descriptorCount = 1;
		desc_write.pBufferInfo = &buffer_info;
		desc_write.pImageInfo = nullptr;
		desc_write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(VulkanInstance::logical_device(), 1, &desc_write, 0, nullptr);
	}

	return true;
}
}