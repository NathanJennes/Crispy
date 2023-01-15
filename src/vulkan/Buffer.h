//
// Created by nathan on 1/15/23.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include "defines.h"
#include "log.h"

namespace Vulkan {

class Buffer
{
public:
	Buffer();
	Buffer(const Buffer& other);
	Buffer(Buffer&& other) noexcept;
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties);
	~Buffer();

	Buffer& operator=(const Buffer& other);
	Buffer& operator=(Buffer&& other) noexcept;

	void	copy_to(const Buffer& buffer, u32 dst_offset = 0);
	void	copy_to(const Buffer& buffer, u32 dst_offset, u32 size_to_copy, u32 src_offset = 0);

	void	set_data(const void *src_data, size_t byte_count, u32 offset = 0);
	template<typename T>
	void	set_data(const std::vector<T> &vector, u32 offset)
	{
#ifdef DEBUG
		u32 mem_requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if ((memory_properties() & mem_requirements) != mem_requirements) {
			CORE_ERROR("Buffer::set_data(): the buffer memory is not coherent and visible by the host!");
			return ;
		}

		if (size() < offset + vector.size() * sizeof(T)) {
			CORE_ERROR("Buffer::set_data(): The buffer is not large enough to copy this data!");
			CORE_ERROR("Buffer::set_data(): size(): %lu, offset: %lu, byte_count: %lu", size(), offset, vector.size() * sizeof(T));
			return ;
		}
#endif
		set_data(vector.data(), vector.size() * sizeof(T), offset);
	}

	//----
	// Getters
	//----
	const VkBuffer&					buffer()			const	{ return _buffer; }
	const VkDeviceSize&				size()				const	{ return _size; }
	const VkBufferUsageFlags&		usage()				const	{ return _usage; }
	const VkMemoryPropertyFlags&	memory_properties()	const	{ return _memory_properties; }

private:	// Methods

	void	initialize();
	void	shutdown();

	void	create_buffer();
	void	allocate_buffer();
	void	create_command_pool();
	void	create_command_buffer();
	void	create_fence();
	void	record_command_buffer(VkBuffer dst_buffer, u32 dst_offset, u32 size_to_copy, u32 src_offset);
	void	submit_command_buffer();

	std::optional<u32>	find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties);

	//----
	// Getters
	//----
	const VkDeviceMemory&			memory()			const	{ return _memory; }
	const VkCommandPool&			command_pool()		const	{ return _command_pool; }
	const VkCommandBuffer&			command_buffer()	const	{ return _command_buffer; }
	const VkFence&					copy_fence()		const	{ return _copy_fence; }

private:	// Members
	VkBuffer				_buffer;
	VkDeviceSize			_size;
	VkBufferUsageFlags		_usage;
	VkMemoryPropertyFlags	_memory_properties;
	VkDeviceMemory			_memory;

	VkCommandPool			_command_pool;
	VkCommandBuffer			_command_buffer;
	VkFence					_copy_fence;
};

} // Vulkan

#endif //BUFFER_H
