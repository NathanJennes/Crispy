//
// Created by nathan on 1/15/23.
//

#include <limits>
#include <cstring>
#include "Buffer.h"
#include "VulkanInstance.h"
#include "log.h"

namespace Vulkan {
Buffer::Buffer()
	: _buffer(VK_NULL_HANDLE), _size(0), _usage(0), _memory_properties(0), _memory(VK_NULL_HANDLE),
	_command_pool(VK_NULL_HANDLE), _command_buffer(VK_NULL_HANDLE), _copy_fence(VK_NULL_HANDLE), _mapped_memory(nullptr)
{
}

Buffer::Buffer(const Buffer &other)
	: _buffer(VK_NULL_HANDLE), _size(other.size()), _usage(other.usage()), _memory_properties(other.memory_properties()), _memory(VK_NULL_HANDLE),
	_command_pool(VK_NULL_HANDLE), _command_buffer(VK_NULL_HANDLE), _copy_fence(VK_NULL_HANDLE), _mapped_memory(nullptr)
{
	initialize();
	other.copy_to(*this);
}

Buffer::Buffer(Buffer &&other) noexcept
	: _buffer(other.buffer()), _size(other.size()), _usage(other.usage()), _memory_properties(other.memory_properties()), _memory(other.memory()),
	_command_pool(other.command_pool()), _command_buffer(other.command_buffer()), _copy_fence(other.copy_fence()), _mapped_memory(other.mapped_memory())
{
	other._buffer = VK_NULL_HANDLE;
	other._size = 0;
	other._usage = 0;
	other._memory_properties = 0;
	other._memory = VK_NULL_HANDLE;
	other._command_pool = VK_NULL_HANDLE;
	other._command_buffer = VK_NULL_HANDLE;
	other._copy_fence = VK_NULL_HANDLE;
	other._mapped_memory = nullptr;
}

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties)
	: _buffer(VK_NULL_HANDLE), _size(size), _usage(usage), _memory_properties(mem_properties), _memory(VK_NULL_HANDLE),
	_command_pool(VK_NULL_HANDLE), _command_buffer(VK_NULL_HANDLE), _copy_fence(VK_NULL_HANDLE), _mapped_memory(nullptr)
{
	if (size > 0)
		initialize();
	else {
		CORE_WARN("Trying to create a Buffer with size 0!");
	}
}

Buffer::~Buffer()
{
	shutdown();
}

Buffer &Buffer::operator=(const Buffer &other)
{
	if (&other == this)
		return *this;

	shutdown();

	_size = other.size();
	_usage = other.usage();
	_memory_properties = other.memory_properties();

	initialize();
	other.copy_to(*this);

	return *this;
}

Buffer &Buffer::operator=(Buffer &&other) noexcept
{
	if (&other == this)
		return *this;

	shutdown();

	_buffer = other.buffer();
	_size = other.size();
	_usage = other.usage();
	_memory_properties = other.memory_properties();
	_memory = other.memory();
	_copy_fence = other.copy_fence();
	_command_buffer = other.command_buffer();
	_command_pool = other.command_pool();
	_mapped_memory = other.mapped_memory();

	other._buffer = VK_NULL_HANDLE;
	other._size = 0;
	other._usage = 0;
	other._memory_properties = 0;
	other._memory = VK_NULL_HANDLE;
	other._copy_fence = VK_NULL_HANDLE;
	other._command_buffer = VK_NULL_HANDLE;
	other._command_pool = VK_NULL_HANDLE;
	other._mapped_memory = nullptr;

	return *this;
}

void Buffer::initialize()
{
	if (size() > 0) {
		create_buffer();
		if (buffer() != VK_NULL_HANDLE)
			allocate_buffer();
	}

	// TODO: no need to create command buffers pools and fences for buffers that wont be copied to other buffers
	create_command_pool();
	if (command_pool() != VK_NULL_HANDLE)
		create_command_buffer();

	create_fence();
}

void Buffer::shutdown()
{
	if (copy_fence() != VK_NULL_HANDLE) {
		vkDestroyFence(VulkanInstance::logical_device(), copy_fence(), nullptr);
		_copy_fence = VK_NULL_HANDLE;
	}
	if (command_buffer() != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(VulkanInstance::logical_device(), command_pool(), 1, &_command_buffer);
		_command_buffer = VK_NULL_HANDLE;
	}

	if (command_pool() != VK_NULL_HANDLE) {
		vkDestroyCommandPool(VulkanInstance::logical_device(), command_pool(), nullptr);
		_command_pool = VK_NULL_HANDLE;
	}

	if (mapped_memory() != nullptr) {
		vkUnmapMemory(VulkanInstance::logical_device(), memory());
		_mapped_memory = nullptr;
	}
	if (memory() != VK_NULL_HANDLE) {
		vkFreeMemory(VulkanInstance::logical_device(), memory(), nullptr);
		_memory = VK_NULL_HANDLE;
	}
	if (buffer() != VK_NULL_HANDLE) {
		vkDestroyBuffer(VulkanInstance::logical_device(), buffer(), nullptr);
		_buffer = VK_NULL_HANDLE;
	}
}

void Buffer::create_buffer()
{
	VkBufferCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_infos.size = size();
	create_infos.usage = usage();
	create_infos.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VulkanInstance::logical_device(), &create_infos, nullptr, &_buffer) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_DEBUG("Couldn't create a Buffer!");
	}
}

void Buffer::allocate_buffer()
{
	VkMemoryRequirements mem_requirements{};
	vkGetBufferMemoryRequirements(VulkanInstance::logical_device(), buffer(), &mem_requirements);

	std::optional<u32> memory_type_index = find_memory_type(mem_requirements.memoryTypeBits, memory_properties()).value();
	if (!memory_type_index.has_value()) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't find a memory region with the right type!");
		return ;
	}

	VkMemoryAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_infos.allocationSize = mem_requirements.size;
	alloc_infos.memoryTypeIndex = memory_type_index.value();

	if (vkAllocateMemory(VulkanInstance::logical_device(), &alloc_infos, nullptr, &_memory) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't allocate memory for a Buffer!");
	}

	vkBindBufferMemory(VulkanInstance::logical_device(), buffer(), memory(), 0);

	if ((memory_properties() & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) != 0)
		vkMapMemory(VulkanInstance::logical_device(), memory(), 0, size(), 0, &_mapped_memory);
}

std::optional<u32> Buffer::find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties mem_properties{};
	vkGetPhysicalDeviceMemoryProperties(VulkanInstance::physical_device(), &mem_properties);

	// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT bit specifies that memory allocated with this type is the most efficient for device access. This property will be set if and only if the memory type belongs to a heap with the VK_MEMORY_HEAP_DEVICE_LOCAL_BIT set
	for (u32 i = 0; i < mem_properties.memoryTypeCount; i++) {
		if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
	return {};
}

void Buffer::create_command_pool()
{
	QueueFamilyIndices queue_indices = VulkanInstance::get_queues_for_device(VulkanInstance::physical_device());

	VkCommandPoolCreateInfo pool_create_infos{};
	pool_create_infos.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_infos.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_infos.queueFamilyIndex = queue_indices.graphics_index.value();

	if (vkCreateCommandPool(VulkanInstance::logical_device(), &pool_create_infos, nullptr, &_command_pool) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't create a command pool for a Buffer!");
	}
}

void Buffer::create_command_buffer()
{
	VkCommandBufferAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_infos.commandPool = command_pool();
	alloc_infos.commandBufferCount = 1;
	alloc_infos.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(VulkanInstance::logical_device(), &alloc_infos, &_command_buffer) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't create the command buffer for a Buffer!");
		return ;
	}
}

void Buffer::create_fence()
{
	VkFenceCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (vkCreateFence(VulkanInstance::logical_device(), &create_infos, nullptr, &_copy_fence) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't create a fence for a Buffer!");
	}
}

void Buffer::record_command_buffer(VkBuffer dst_buffer, u32 dst_offset, u32 size_to_copy, u32 src_offset) const
{
	VkCommandBufferBeginInfo begin_infos{};
	begin_infos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_infos.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer(), &begin_infos);

	VkBufferCopy copy_region{};
	copy_region.srcOffset = src_offset;
	copy_region.size = size_to_copy;
	copy_region.dstOffset = dst_offset;
	vkCmdCopyBuffer(command_buffer(), buffer(), dst_buffer, 1, &copy_region);

	if (vkEndCommandBuffer(command_buffer()) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't record command buffer for a Buffer!");
	}
}

void Buffer::submit_command_buffer() const
{
	VkSubmitInfo submit_infos{};
	submit_infos.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_infos.commandBufferCount = 1;
	submit_infos.pCommandBuffers = &_command_buffer;
	vkQueueSubmit(VulkanInstance::graphics_queue(), 1, &submit_infos, copy_fence());
	vkWaitForFences(VulkanInstance::logical_device(), 1, &_copy_fence, VK_TRUE, std::numeric_limits<u64>::max());
	vkResetFences(VulkanInstance::logical_device(), 1, &_copy_fence);
}

void Buffer::copy_to(const Buffer& buffer, u32 dst_offset, u32 size_to_copy, u32 src_offset) const
{
#ifdef DEBUG
	if (buffer.size() < dst_offset + size_to_copy) {
		CORE_ERROR("Buffer::copy_to(): destination buffer not large enough!");
		CORE_ERROR("Buffer::copy_to(): buffer.size() = %u, dst_offset = %u, size_to_copy = %lu", buffer.size(), dst_offset, size_to_copy);
		return ;
	}
	if (size() < src_offset + size_to_copy) {
		CORE_ERROR("Buffer::copy_to(): source buffer not large enough! This will result in reading past the source buffer!");
		CORE_ERROR("Buffer::copy_to(): size() = %u, src_offset = %u, size_to_copy = %lu", size(), src_offset, size_to_copy);
		return ;
	}

	if ((buffer.usage() & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): destination buffer was not set-up to be used as destination in a transfer!");
		return ;
	}

	if ((usage() & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): source buffer was not set-up to be used as source in a transfer!");
		return ;
	}

#endif
	record_command_buffer(buffer.buffer(), dst_offset, size_to_copy, src_offset);
	submit_command_buffer();
}

void Buffer::copy_to(const Buffer &buffer, u32 dst_offset) const
{
#ifdef DEBUG
	if (buffer.size() < dst_offset + size()) {
		CORE_ERROR("Buffer::copy_to(): destination buffer not large enough!");
		CORE_ERROR("Buffer::copy_to(): buffer.size() = %u, dst_offset = %u, size_to_copy = %u", buffer.size(), dst_offset, size());
		return ;
	}

	if ((buffer.usage() & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): destination buffer was not set-up to be used as destination in a transfer!");
		return ;
	}

	if ((usage() & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): source buffer was not set-up to be used as source in a transfer!");
		return ;
	}

#endif
	record_command_buffer(buffer.buffer(), dst_offset, size(), 0);
	submit_command_buffer();
}

void Buffer::set_data(const void *src_data, size_t byte_count, u32 offset)
{
	(void)offset;
#ifdef DEBUG
	u32 mem_requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if ((memory_properties() & mem_requirements) != mem_requirements) {
		CORE_ERROR("Buffer::set_data(): the buffer memory is not coherent and visible by the host!");
		return ;
	}

	if (size() < offset + byte_count) {
		CORE_ERROR("Buffer::set_data(): The buffer is not large enough to copy this data!");
		CORE_ERROR("Buffer::set_data(): size(): %u, offset: %u, byte_count: %lu", size(), offset, byte_count);
		return ;
	}
#endif

	memmove(_mapped_memory, src_data, byte_count);
}

Buffer Buffer::create_vertex_buffer(VkDeviceSize size, bool host_visible)
{
	VkMemoryPropertyFlags memory_flags{};
	if (host_visible)
		memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	else
		memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return Buffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memory_flags);
}

Buffer Buffer::create_index_buffer(VkDeviceSize size, bool host_visible)
{
	VkMemoryPropertyFlags memory_flags{};
	if (host_visible)
		memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	else
		memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return Buffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memory_flags);
}

Buffer Buffer::create_uniform_buffer(VkDeviceSize size, bool host_visible)
{
	VkMemoryPropertyFlags memory_flags{};
	if (host_visible)
		memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	else
		memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return Buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memory_flags);
}

void Buffer::release_resources()
{
	shutdown();
}
} // Vulkan
