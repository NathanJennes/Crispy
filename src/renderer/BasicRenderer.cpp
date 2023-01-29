//
// Created by nathan on 1/19/23.
//

#include "BasicRenderer.h"
#include "vulkan/VulkanInstance.h"
#include "vulkan/vulkan_errors.h"
#include "vulkan/GraphicsPipeline.h"
#include "Renderer.h"
#include "vulkan/SwapchainManager.h"
#include "Window.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Vulkan {

//----
// BasicRenderer::Mesh
//----

BasicRenderer::Mesh::Mesh()
	:vertex_count(0), index_count(0)
{
}

BasicRenderer::Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<u32> &indices)
	: vertex_count(vertices.size()), index_count(indices.size())
{
	// Vertex buffer
	Buffer vertex_staging_buffer = Buffer::create_vertex_buffer(vertex_count * sizeof(Vertex), true);
	vertex_buffer = Buffer::create_vertex_buffer(vertex_count * sizeof(Vertex), false);

	vertex_staging_buffer.set_data(vertices, 0);
	vertex_staging_buffer.copy_to(vertex_buffer);

	// Index buffer
	Buffer index_staging_buffer = Buffer::create_index_buffer(index_count * sizeof(u32), true);
	index_buffer = Buffer::create_index_buffer(index_count * sizeof(u32), false);

	index_staging_buffer.set_data(indices, 0);
	index_staging_buffer.copy_to(index_buffer);


}

BasicRenderer::Mesh::Mesh(Vulkan::BasicRenderer::Mesh &&other) noexcept
	: vertex_buffer(std::move(other.vertex_buffer)), index_buffer(std::move(other.index_buffer)), vertex_count(other.vertex_count), index_count(other.index_count)
{
}

BasicRenderer::Mesh &Vulkan::BasicRenderer::Mesh::operator=(const Vulkan::BasicRenderer::Mesh &other)
{
	if (&other == this)
		return *this;

	vertex_count = other.vertex_count;
	index_count = other.index_count;
	vertex_buffer = other.vertex_buffer;
	index_buffer = other.index_buffer;

	return *this;
}

BasicRenderer::Mesh &Vulkan::BasicRenderer::Mesh::operator=(Vulkan::BasicRenderer::Mesh &&other) noexcept
{
	if (&other == this)
		return *this;

	vertex_count = other.vertex_count;
	index_count = other.index_count;
	vertex_buffer = std::move(other.vertex_buffer);
	index_buffer = std::move(other.index_buffer);

	return *this;
}

//----
// Renderer
//----
VkSemaphore			BasicRenderer::image_available_semaphore = VK_NULL_HANDLE;
VkSemaphore			BasicRenderer::render_finished_semaphore = VK_NULL_HANDLE;
VkFence				BasicRenderer::last_frame_presented_fence = VK_NULL_HANDLE;

bool				BasicRenderer::frame_started = false;
u32					BasicRenderer::current_image_index = 0;

Buffer				BasicRenderer::camera_uniform_buffer;
VkDescriptorPool	BasicRenderer::descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSet		BasicRenderer::camera_descriptor_set = VK_NULL_HANDLE;

VkCommandPool		BasicRenderer::command_pool = VK_NULL_HANDLE;
VkCommandBuffer		BasicRenderer::command_buffer = VK_NULL_HANDLE;

bool Vulkan::BasicRenderer::initialize()
{
	if (!VulkanInstance::initialize())
		return false;
	if (!SwapchainManager::initialize())
		return false;
	if (!GraphicsPipeline::initialize())
		return false;
	if (!SwapchainManager::create_framebuffers())
		return false;

	if (!create_sync_objects())
		return false;
	CORE_TRACE("BasicRenderer's sync objects created");

	create_uniform_buffer();
	CORE_TRACE("BasicRenderer's uniform buffer created");

	if (!create_descriptor_pool())
		return false;
	CORE_TRACE("BasicRenderer's descriptor pool created");

	if (!create_camera_descriptor_set())
		return false;
	CORE_TRACE("BasicRenderer's descripitor set created");

	if (!create_command_pool())
		return false;
	CORE_TRACE("BasicRenderer's command pool created");

	if (!create_command_buffer())
		return false;
	CORE_TRACE("BasicRenderer's command buffer created");

	CORE_TRACE("BasicRenderer fully initialized!");
	return true;
}

void Vulkan::BasicRenderer::shutdown()
{
	destroy_command_pool();
	destroy_descriptor_pool();
	destroy_sync_objects();
	camera_uniform_buffer.release_resources();

	GraphicsPipeline::shutdown();
	SwapchainManager::shutdown();
	VulkanInstance::shutdown();
}

void Vulkan::BasicRenderer::begin_frame()
{
	frame_started = false;
	wait_for_last_frame_finished();

	auto image_index = get_swapchain_image();
	if (!image_index.has_value())
		return ;
	current_image_index = image_index.value();

	VkResult result = vkResetCommandBuffer(command_buffer, 0);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't reset command buffer of BasicRenderer: %s", vulkan_error_to_string(result));
		return;
	}

	if (!begin_command_buffer())
		return ;
	begin_renderpass();
	setup_viewport();
	setup_camera_ubo();

	frame_started = true;
}

void Vulkan::BasicRenderer::draw(const Vulkan::BasicRenderer::Mesh &mesh,
	const glm::vec3 &pos)
{
	draw(mesh, pos, glm::vec3(0.0), glm::vec3(1.0));
}

void
Vulkan::BasicRenderer::draw(const Vulkan::BasicRenderer::Mesh &mesh,
	const glm::vec3 &pos, const glm::vec3 &rotation)
{
	draw(mesh, pos, rotation, glm::vec3(1.0));
}

void
Vulkan::BasicRenderer::draw(const Vulkan::BasicRenderer::Mesh &mesh,
	const glm::vec3 &pos, const glm::vec3 &rotation, const glm::vec3 &scale)
{
	if (!frame_started) {
		CORE_DEBUG("Trying to draw() with BasicRenderer but the frame wasn't started");
		return ;
	}
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &mesh.get_vertex_buffer().buffer(), offsets);
	vkCmdBindIndexBuffer(command_buffer, mesh.get_index_buffer().buffer(), 0, VK_INDEX_TYPE_UINT32);

	// Push constants for model matrix
	auto model = glm::translate(glm::mat4(1.0f), pos);
	model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, scale);
	vkCmdPushConstants(command_buffer, GraphicsPipeline::pipeline_layout(),
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

	vkCmdDrawIndexed(command_buffer, mesh.get_index_count(), 1, 0, 0, 0);
}

void Vulkan::BasicRenderer::end_frame()
{
	vkResetFences(VulkanInstance::logical_device(), 1, &last_frame_presented_fence);
	end_renderpass();
	if (!end_command_buffer())
		return ;
	if (!submit_command_buffer())
		return ;
	present_frame();
}

bool Vulkan::BasicRenderer::create_sync_objects()
{
	VkSemaphoreCreateInfo semaphore_infos{};
	semaphore_infos.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_infos{};
	fence_infos.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_infos.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(VulkanInstance::logical_device(), &semaphore_infos, nullptr, &image_available_semaphore) != VK_SUCCESS ||
		vkCreateSemaphore(VulkanInstance::logical_device(), &semaphore_infos, nullptr, &render_finished_semaphore) != VK_SUCCESS ||
		vkCreateFence(VulkanInstance::logical_device(), &fence_infos, nullptr, &last_frame_presented_fence) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create BasicRenderer's sync objects!");
		return false;
	}
	return true;
}

bool Vulkan::BasicRenderer::create_descriptor_pool()
{
	VkDescriptorPoolSize pool_size{};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = 1;

	VkDescriptorPoolCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_infos.poolSizeCount = 1;
	create_infos.pPoolSizes = &pool_size;
	create_infos.maxSets = 1;

	if (vkCreateDescriptorPool(VulkanInstance::logical_device(), &create_infos, nullptr, &descriptor_pool) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create BasicRenderer's descriptor pool");
		return false;
	}
	return true;
}

bool Vulkan::BasicRenderer::create_camera_descriptor_set()
{
	VkDescriptorSetLayout layouts[] = {GraphicsPipeline::descriptor_set_layout()};
	VkDescriptorSetAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_infos.descriptorPool = descriptor_pool;
	alloc_infos.descriptorSetCount = 1;
	alloc_infos.pSetLayouts = layouts;

	VkResult result = vkAllocateDescriptorSets(VulkanInstance::logical_device(), &alloc_infos, &camera_descriptor_set);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't create BasicRenderer's descriptor sets: %s", vulkan_error_to_string(result));
		return false;
	}

	VkDescriptorBufferInfo buffer_info{};
	buffer_info.buffer = camera_uniform_buffer.buffer();
	buffer_info.offset = 0;
	buffer_info.range = sizeof(CameraUBO);

	VkWriteDescriptorSet desc_write{};
	desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	desc_write.dstSet = camera_descriptor_set;
	desc_write.dstBinding = 0;
	desc_write.dstArrayElement = 0;
	desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	desc_write.descriptorCount = 1;
	desc_write.pBufferInfo = &buffer_info;
	desc_write.pImageInfo = nullptr;
	desc_write.pTexelBufferView = nullptr;
	vkUpdateDescriptorSets(VulkanInstance::logical_device(), 1, &desc_write, 0, nullptr);

	return true;
}

void Vulkan::BasicRenderer::create_uniform_buffer()
{
	camera_uniform_buffer = Buffer::create_uniform_buffer(sizeof(CameraUBO), true);
}

void BasicRenderer::destroy_sync_objects()
{
	if (image_available_semaphore != VK_NULL_HANDLE)
		vkDestroySemaphore(VulkanInstance::logical_device(), image_available_semaphore, nullptr);
	if (render_finished_semaphore != VK_NULL_HANDLE)
		vkDestroySemaphore(VulkanInstance::logical_device(), render_finished_semaphore, nullptr);
	if (last_frame_presented_fence != VK_NULL_HANDLE)
		vkDestroyFence(VulkanInstance::logical_device(), last_frame_presented_fence, nullptr);
}

void BasicRenderer::destroy_descriptor_pool()
{
	// Descriptor sets are implicitly destroyed when the pool is destroyed
	if (descriptor_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VulkanInstance::logical_device(), descriptor_pool, nullptr);
}

void BasicRenderer::wait_for_last_frame_finished()
{
	// TODO: Add a timeout checking instead of waiting indefinitely
	vkWaitForFences(VulkanInstance::logical_device(), 1, &last_frame_presented_fence, VK_TRUE, std::numeric_limits<u64>::max());
}

std::optional<u32> BasicRenderer::get_swapchain_image()
{
	u32 image_index;
	VkResult result = vkAcquireNextImageKHR(VulkanInstance::logical_device(), SwapchainManager::swapchain(),
		std::numeric_limits<u64>::max(), image_available_semaphore, VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		SwapchainManager::recreate();
		return {};
	} else if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't acquire BasicRenderer's next swapchain image for rendering: %s", vulkan_error_to_string(result));
		return {};
	}

	return image_index;
}

bool BasicRenderer::create_command_pool()
{
	QueueFamilyIndices queue_indices = VulkanInstance::get_queues_for_device(VulkanInstance::physical_device());

	VkCommandPoolCreateInfo pool_create_infos{};
	pool_create_infos.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_infos.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_infos.queueFamilyIndex = queue_indices.graphics_index.value();

	VkResult result = vkCreateCommandPool(VulkanInstance::logical_device(), &pool_create_infos, nullptr, &command_pool);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't create BasicRenderer's command pool: %s", vulkan_error_to_string(result));
		return false;
	}

	return true;
}

bool BasicRenderer::create_command_buffer()
{	VkCommandBufferAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_infos.commandPool = command_pool;
	alloc_infos.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_infos.commandBufferCount = 1;

	VkResult  result = vkAllocateCommandBuffers(VulkanInstance::logical_device(), &alloc_infos, &command_buffer);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't create BasicRenderer's command buffer: %s", vulkan_error_to_string(result));
		return false;
	}

	return true;
}

bool BasicRenderer::begin_command_buffer()
{
	VkCommandBufferBeginInfo begin_infos{};
	begin_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_infos.flags = 0;
	begin_infos.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(command_buffer, &begin_infos);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't begin BasicRenderer's command buffer: %s", vulkan_error_to_string(result));
		return false;
	}
	return true;
}

void BasicRenderer::begin_renderpass()
{
	VkRenderPassBeginInfo render_pass_infos{};
	render_pass_infos.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_infos.renderPass = GraphicsPipeline::render_pass();
	render_pass_infos.framebuffer = SwapchainManager::swapchain_framebuffers()[current_image_index];
	render_pass_infos.renderArea.offset = {0, 0};
	render_pass_infos.renderArea.extent = SwapchainManager::swapchain_extent();

	VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	render_pass_infos.clearValueCount = 1;
	render_pass_infos.pClearValues = &clear_color;

	vkCmdBeginRenderPass(command_buffer, &render_pass_infos, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline::pipeline());
}

void BasicRenderer::setup_viewport()
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(SwapchainManager::swapchain_extent().width);
	viewport.height = static_cast<float>(SwapchainManager::swapchain_extent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissors{};
	scissors.offset = {0, 0};
	scissors.extent = SwapchainManager::swapchain_extent();
	vkCmdSetScissor(command_buffer, 0, 1, &scissors);
}

void BasicRenderer::end_renderpass()
{
	vkCmdEndRenderPass(command_buffer);
}

bool BasicRenderer::end_command_buffer()
{
	VkResult result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't end BasicRenderer's command buffer: %s", vulkan_error_to_string(result));
		return false;
	}
	return true;
}

bool BasicRenderer::submit_command_buffer()
{
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_infos{};
	submit_infos.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_infos.waitSemaphoreCount = 1;
	submit_infos.pWaitSemaphores = &image_available_semaphore;
	submit_infos.pWaitDstStageMask = wait_stages;
	submit_infos.commandBufferCount = 1;
	submit_infos.pCommandBuffers = &command_buffer;
	submit_infos.signalSemaphoreCount = 1;
	submit_infos.pSignalSemaphores = &render_finished_semaphore;

	VkResult result = vkQueueSubmit(VulkanInstance::graphics_queue(), 1, &submit_infos, last_frame_presented_fence);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't submit BasicRenderer's draw command buffer: %s", vulkan_error_to_string(result));
		return false;
	}
	return true;
}

bool BasicRenderer::present_frame()
{
	VkSwapchainKHR swapchains[] = {SwapchainManager::swapchain()};

	VkPresentInfoKHR present_infos{};
	present_infos.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_infos.waitSemaphoreCount = 1;
	present_infos.pWaitSemaphores = &render_finished_semaphore;
	present_infos.swapchainCount = 1;
	present_infos.pSwapchains = swapchains;
	present_infos.pImageIndices = &current_image_index;
	present_infos.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(VulkanInstance::present_queue(), &present_infos);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Window::did_resize()) {
		SwapchainManager::recreate();
	} else if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't present BasicRenderer's swap chain image: %s", vulkan_error_to_string(result));
		return false;
	}
	return true;
}

void BasicRenderer::setup_camera_ubo()
{
	CameraUBO ubo{};
	ubo.view = glm::lookAt(glm::vec3(0.0f, 3.0f, -5.0f), glm::vec3(2.5f, -2.5f, 2.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f),
		(float) SwapchainManager::swapchain_extent().width / (float) SwapchainManager::swapchain_extent().height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;
	camera_uniform_buffer.set_data(&ubo, sizeof(CameraUBO));
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		GraphicsPipeline::pipeline_layout(), 0, 1, &camera_descriptor_set, 0, nullptr);
}

void BasicRenderer::destroy_command_pool()
{
	if (command_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(VulkanInstance::logical_device(), command_pool, nullptr);
}
}
