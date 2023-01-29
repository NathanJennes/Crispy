//
// Created by nathan on 1/10/23.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <optional>
#include <memory>
#include "defines.h"
#include "vulkan/Buffer.h"
#include "glm/glm.hpp"
#include "Vertex.h"

namespace Vulkan {

struct CameraUBO
{
	glm::mat4 view_proj;
};

class Renderer
{
public:
	static bool initialize();
	static void shutdown();

	static void	draw(const std::vector<Vertex>& vertices, const std::vector<u16>& indices, const glm::vec3& pos);

private:	// Methods
	static bool	create_sync_objects();
	static bool	create_buffers();
	static bool	create_descriptor_pool();
	static bool	create_descriptor_sets();

	static void	draw_call(const std::vector<Vertex>& vertices, const std::vector<u16>& indices, const glm::vec3& pos);

	static void	fill_vertex_buffer(const std::vector<Vertex>& vertices, u32 offset);
	static void	fill_index_buffer(const std::vector<u16>& indices, u32 offset);
	static void	fill_uniform_buffer(const glm::vec3& pos);

	//----
	// Getters
	//----
	static std::vector<VkSemaphore>&	image_available_semaphores()	{ return _image_available_semaphores; }
	static std::vector<VkSemaphore>&	render_finished_semaphores()	{ return _render_finished_semaphores; }
	static std::vector<VkFence>&		in_flight_fences()				{ return _in_flight_fences; }
	static u32							frames_in_flight_count()		{ return _frames_in_flight_count; }
	static u32							current_frame()					{ return _current_frame; }
	static u32							vertex_buffer_capacity()		{ return _vertex_buffer_capacity; }
	static Buffer*						vertex_buffer()					{ return _vertex_buffer; }
	static Buffer*						vertex_staging_buffer()			{ return _vertex_staging_buffer; }
	static u32							index_buffer_capacity()			{ return _index_buffer_capacity; }
	static Buffer*						index_buffer()					{ return _index_buffer; }
	static Buffer*						index_staging_buffer()			{ return _index_staging_buffer; }
	static std::vector<Buffer*>			uniform_buffers()				{ return _uniform_buffers; }

private:	// Members
	static std::vector<VkSemaphore>		_image_available_semaphores;
	static std::vector<VkSemaphore>		_render_finished_semaphores;
	static std::vector<VkFence>			_in_flight_fences;

	static const u32					_frames_in_flight_count;
	static u32							_current_frame;

	static u32							_vertex_buffer_capacity;
	static Buffer*						_vertex_buffer;
	static Buffer*						_vertex_staging_buffer;

	static u32							_index_buffer_capacity;
	static Buffer*						_index_buffer;
	static Buffer*						_index_staging_buffer;

	static std::vector<Buffer*>			_uniform_buffers;
	static VkDescriptorPool				_descriptor_pool;
	static std::vector<VkDescriptorSet>	_descriptor_sets;
};

}

#endif //RENDERER_H
