//
// Created by nathan on 1/15/23.
//

#include <cstring>
#include "Buffer.h"
#include "VulkanInstance.h"
#include "log.h"

namespace Vulkan {

std::unordered_map<VkBuffer, u64>	Buffer::buffer_references;

Buffer::Buffer()
	: buffer(VK_NULL_HANDLE), size(0), usage(0), memory_properties(0), memory(VK_NULL_HANDLE), mapped_memory(nullptr)
{
}

Buffer::Buffer(const Buffer &other)
	: buffer(other.buffer), size(other.size), usage(other.usage),
	  memory_properties(other.memory_properties), memory(other.memory), mapped_memory(other.mapped_memory)
{
	if (buffer != VK_NULL_HANDLE)
		buffer_references[buffer]++;
}

Buffer::Buffer(Buffer &&other) noexcept
	: buffer(other.buffer), size(other.size), usage(other.usage),
	  memory_properties(other.memory_properties), memory(other.memory), mapped_memory(other.mapped_memory)
{
	other.buffer = VK_NULL_HANDLE;
	other.size = 0;
	other.usage = 0;
	other.memory_properties = 0;
	other.memory = VK_NULL_HANDLE;
	other.mapped_memory = nullptr;
}

Buffer::Buffer(VkDeviceSize new_size, VkBufferUsageFlags new_usage, VkMemoryPropertyFlags new_mem_properties)
	: buffer(VK_NULL_HANDLE), size(new_size), usage(new_usage), memory_properties(new_mem_properties), memory(VK_NULL_HANDLE), mapped_memory(nullptr)
{
	if (size > 0) {
		initialize();
		if (buffer != VK_NULL_HANDLE)
			buffer_references[buffer]++;
	} else {
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

	buffer = other.buffer;
	size = other.size;
	usage = other.usage;
	memory_properties = other.memory_properties;
	memory = other.memory;
	mapped_memory = other.mapped_memory;

	if (buffer != VK_NULL_HANDLE)
		buffer_references[buffer]++;

	return *this;
}

Buffer &Buffer::operator=(Buffer &&other) noexcept
{
	if (&other == this)
		return *this;

	shutdown();

	buffer = other.buffer;
	size = other.size;
	usage = other.usage;
	memory_properties = other.memory_properties;
	memory = other.memory;
	mapped_memory = other.mapped_memory;

	other.buffer = VK_NULL_HANDLE;
	other.size = 0;
	other.usage = 0;
	other.memory_properties = 0;
	other.memory = VK_NULL_HANDLE;
	other.mapped_memory = nullptr;

	return *this;
}

void Buffer::initialize()
{
	create_buffer();
	if (buffer != VK_NULL_HANDLE)
		allocate_buffer();
}

void Buffer::shutdown()
{
	if (buffer != VK_NULL_HANDLE) {
		buffer_references[buffer]--;
		if (buffer_references.at(buffer) == 0) {
			if (mapped_memory != nullptr)
				vkUnmapMemory(VulkanInstance::logical_device(), memory);
			if (memory != VK_NULL_HANDLE)
				vkFreeMemory(VulkanInstance::logical_device(), memory, nullptr);
			vkDestroyBuffer(VulkanInstance::logical_device(), buffer, nullptr);
			buffer_references.erase(buffer);
		}
	}
	buffer = VK_NULL_HANDLE;
	memory = VK_NULL_HANDLE;
	mapped_memory = nullptr;
}

void Buffer::create_buffer()
{
	VkBufferCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_infos.size = size;
	create_infos.usage = usage;
	create_infos.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VulkanInstance::logical_device(), &create_infos, nullptr, &buffer) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_DEBUG("Couldn't create a Buffer!");
	}
}

void Buffer::allocate_buffer()
{
	VkMemoryRequirements mem_requirements{};
	vkGetBufferMemoryRequirements(VulkanInstance::logical_device(), buffer, &mem_requirements);

	std::optional<u32> memory_type_index = find_memory_type(mem_requirements.memoryTypeBits, memory_properties).value();
	if (!memory_type_index.has_value()) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't find a memory region with the right type!");
		return ;
	}

	VkMemoryAllocateInfo alloc_infos{};
	alloc_infos.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_infos.allocationSize = mem_requirements.size;
	alloc_infos.memoryTypeIndex = memory_type_index.value();

	if (vkAllocateMemory(VulkanInstance::logical_device(), &alloc_infos, nullptr, &memory) != VK_SUCCESS) {
		TODO_PROPAGATE_ERRORS
		CORE_ERROR("Couldn't allocate memory for a Buffer!");
	}

	vkBindBufferMemory(VulkanInstance::logical_device(), buffer, memory, 0);

	if ((memory_properties & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) != 0)
		vkMapMemory(VulkanInstance::logical_device(), memory, 0, size, 0, &mapped_memory);
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

void Buffer::copy_to_impl(VkBuffer dst_buffer, u32 dst_offset, u32 bytes_to_copy, u32 src_offset) const
{
	VulkanInstance::immediate_submit([&](VkCommandBuffer cmd_buffer) {
		VkBufferCopy copy_region{};
		copy_region.srcOffset = src_offset;
		copy_region.dstOffset = dst_offset;
		copy_region.size = bytes_to_copy;
		vkCmdCopyBuffer(cmd_buffer, buffer, dst_buffer, 1, &copy_region);
	});
}

void Buffer::copy_to(const Buffer& buffer, u32 dst_offset, u32 size_to_copy, u32 src_offset) const
{
#ifdef DEBUG
	if (buffer.size < dst_offset + size_to_copy) {
		CORE_ERROR("Buffer::copy_to(): destination buffer not large enough!");
		CORE_ERROR("Buffer::copy_to(): buffer.size() = %u, dst_offset = %u, size_to_copy = %lu", buffer.size, dst_offset, size_to_copy);
		return ;
	}
	if (size < src_offset + size_to_copy) {
		CORE_ERROR("Buffer::copy_to(): source buffer not large enough! This will result in reading past the source buffer!");
		CORE_ERROR("Buffer::copy_to(): size() = %u, src_offset = %u, size_to_copy = %lu", size, src_offset, size_to_copy);
		return ;
	}

	if ((buffer.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): destination buffer was not set-up to be used as destination in a transfer!");
		return ;
	}

	if ((usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): source buffer was not set-up to be used as source in a transfer!");
		return ;
	}

#endif
	copy_to_impl(buffer.buffer, dst_offset, size_to_copy, src_offset);
}

void Buffer::copy_to(const Buffer &buffer, u32 dst_offset) const
{
#ifdef DEBUG
	if (buffer.size < dst_offset + size) {
		CORE_ERROR("Buffer::copy_to(): destination buffer not large enough!");
		CORE_ERROR("Buffer::copy_to(): buffer.size() = %u, dst_offset = %u, size_to_copy = %u", buffer.size, dst_offset, size);
		return ;
	}

	if ((buffer.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): destination buffer was not set-up to be used as destination in a transfer!");
		return ;
	}

	if ((usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0) {
		CORE_ERROR("Buffer::copy_to(): source buffer was not set-up to be used as source in a transfer!");
		return ;
	}

#endif
	copy_to_impl(buffer.buffer, dst_offset, size, 0);
}

void Buffer::set_data(const void *src_data, size_t byte_count, u32 offset)
{
	(void)offset;
#ifdef DEBUG
	u32 mem_requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if ((memory_properties & mem_requirements) != mem_requirements) {
		CORE_ERROR("Buffer::set_data(): the buffer memory is not coherent and visible by the host!");
		return ;
	}

	if (size < offset + byte_count) {
		CORE_ERROR("Buffer::set_data(): The buffer is not large enough to copy this data!");
		CORE_ERROR("Buffer::set_data(): size(): %u, offset: %u, byte_count: %lu", size, offset, byte_count);
		return ;
	}
#endif

	memmove(mapped_memory, src_data, byte_count);
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
