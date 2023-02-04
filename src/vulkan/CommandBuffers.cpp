//
// Created by nathan on 1/13/23.
//

#include "CommandBuffers.h"
#include "VulkanInstance.h"
#include "GraphicsPipeline.h"
#include "SwapchainManager.h"
#include "log.h"

namespace Vulkan {

std::vector<VkCommandBuffer>	CommandBuffers::_command_buffers;
VkCommandPool					CommandBuffers::_command_pool;

bool CommandBuffers::initialize(u32 frames_in_flight_count)
{
	QueueFamilyIndices queue_indices = VulkanInstance::get_queues_for_device(VulkanInstance::physical_device());

	VkCommandPoolCreateInfo pool_create_infos{};
	pool_create_infos.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_infos.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_infos.queueFamilyIndex = queue_indices.graphics_index.value();

	if (vkCreateCommandPool(VulkanInstance::logical_device(), &pool_create_infos, nullptr, &_command_pool) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a command pool!")
		return false;
	}

	VkCommandBufferAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_infos.commandPool = command_pool();
	alloc_infos.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_infos.commandBufferCount = frames_in_flight_count;

	command_buffers().resize(frames_in_flight_count);
	if (vkAllocateCommandBuffers(VulkanInstance::logical_device(), &alloc_infos, command_buffers().data()) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create the command buffer!")
		return false;
	}

	return true;
}

void CommandBuffers::shutdown()
{
	vkFreeCommandBuffers(VulkanInstance::logical_device(), command_pool(),
		command_buffers().size(), command_buffers().data());
	vkDestroyCommandPool(VulkanInstance::logical_device(), command_pool(), nullptr);
}

void CommandBuffers::record_command_buffer(VkCommandBuffer command_buffer, u32 image_index, VkBuffer vertex_buffer,
	VkBuffer index_buffer, u32 index_count, VkDescriptorSet descriptor)
{
	VkCommandBufferBeginInfo begin_infos{};
	begin_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_infos.flags = 0;
	begin_infos.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(command_buffer, &begin_infos) != VK_SUCCESS) {
		CORE_ERROR("Couldn't begin a command buffer!")
		return ;
	}

	VkRenderPassBeginInfo render_pass_infos{};
	render_pass_infos.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_infos.renderPass = GraphicsPipeline::render_pass();
	render_pass_infos.framebuffer = SwapchainManager::swapchain_framebuffers()[image_index];
	render_pass_infos.renderArea.offset = {0, 0};
	render_pass_infos.renderArea.extent = SwapchainManager::swapchain_extent();

	VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	render_pass_infos.clearValueCount = 1;
	render_pass_infos.pClearValues = &clear_color;

	vkCmdBeginRenderPass(command_buffer, &render_pass_infos, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline::pipeline());

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

	VkBuffer vertex_buffers[] = {vertex_buffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

	vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);

	VkDescriptorSet descriptors[] = {descriptor};
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		GraphicsPipeline::pipeline_layout(), 0, 1, descriptors, 0, nullptr);

	vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
		CORE_ERROR("Couldn't record command buffer!")
}
} // Vulkan