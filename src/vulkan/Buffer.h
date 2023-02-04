//
// Created by nathan on 1/15/23.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <unordered_map>
#include "defines.h"
#include "log.h"

namespace Vulkan {

class Buffer
{
public:		// Factory
	static Buffer create_vertex_buffer(VkDeviceSize size, bool host_visible);
	static Buffer create_index_buffer(VkDeviceSize size, bool host_visible);
	static Buffer create_uniform_buffer(VkDeviceSize size, bool host_visible);

public:
	Buffer();
	Buffer(const Buffer& other);
	Buffer(Buffer&& other) noexcept;
	~Buffer();

	Buffer& operator=(const Buffer& other);
	Buffer& operator=(Buffer&& other) noexcept;

	void	release_resources();

	void	copy_to(const Buffer& buffer, u32 dst_offset = 0) const;
	void	copy_to(const Buffer& buffer, u32 dst_offset, u32 size_to_copy, u32 src_offset = 0) const;

	void	set_data(const void *src_data, size_t byte_count, u32 offset = 0);
	template<typename T>
	void	set_data(const std::vector<T> &vector, u32 offset)
	{
#ifdef DEBUG
		u32 mem_requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if ((memory_properties & mem_requirements) != mem_requirements) {
			CORE_ERROR("Buffer::set_data(): the buffer memory is not coherent and visible by the host!");
			return ;
		}

		if (size < offset + vector.size() * sizeof(T)) {
			CORE_ERROR("Buffer::set_data(): The buffer is not large enough to copy this data!");
			CORE_ERROR("Buffer::set_data(): size(): %lu, offset: %lu, byte_count: %lu", size, offset, vector.size() * sizeof(T));
			return ;
		}
#endif
		set_data(vector.data(), vector.size() * sizeof(T), offset);
	}

	//----
	// Getters
	//----
	[[nodiscard]] const VkBuffer&				get_buffer()			const	{ return buffer; }
	[[nodiscard]] const VkDeviceSize&			get_size()				const	{ return size; }
	[[nodiscard]] const VkBufferUsageFlags&		get_usage()				const	{ return usage; }
	[[nodiscard]] const VkMemoryPropertyFlags&	get_memory_properties()	const	{ return memory_properties; }

private:	// Methods

	//----
	// Initialization
	//----
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties);
	void	initialize();
	void	create_buffer();
	void	allocate_buffer();

	//----
	// Shutdown
	//----
	void	shutdown();

	//----
	// Memory transfer back-end
	//----
	void	copy_to_impl(VkBuffer dst_buffer, u32 dst_offset, u32 bytes_to_copy, u32 src_offset) const;

	//----
	// Helpers
	//----
	[[nodiscard]] std::optional<u32>	find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const;

private:	// Members
	//----
	// Buffer infos & handles
	//----
	VkBuffer				buffer;
	VkDeviceSize			size;
	VkBufferUsageFlags		usage;
	VkMemoryPropertyFlags	memory_properties;
	VkDeviceMemory			memory;
	void					*mapped_memory;

	//----
	// Buffer references management
	//----
	static std::unordered_map<VkBuffer, u64>	buffer_references;
};

} // Vulkan

#endif //BUFFER_H
