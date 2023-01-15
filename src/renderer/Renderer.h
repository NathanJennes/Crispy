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
#include "math/vulkan_maths.h"
#include "vulkan/Buffer.h"

namespace Vulkan {

struct Vertex
{
	vec2 pos;
	vec3 color;

	Vertex(float x, float y, float r, float g, float b)
	:pos(x, y), color(r, g, b) {}

	static VkVertexInputBindingDescription get_binding_description() {
		VkVertexInputBindingDescription binding_description{};
		binding_description.binding = 0;
		binding_description.stride = sizeof (Vertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_description;
	}

	static std::array<VkVertexInputAttributeDescription, 2> get_attribute_description() {
		std::array<VkVertexInputAttributeDescription, 2> attribute_description{};
		attribute_description[0].binding = 0;
		attribute_description[0].offset = offsetof(Vertex, pos);
		attribute_description[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_description[0].location = 0;
		attribute_description[1].binding = 0;
		attribute_description[1].offset = offsetof(Vertex, color);
		attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_description[1].location = 1;
		return attribute_description;
	}
};

class Renderer
{
public:
	static bool initialize();
	static void shutdown();

	static void	draw(const std::vector<Vertex>& verticies, const std::vector<u16>& indices);

private:	// Methods
	static bool	create_sync_objects();
	static bool	create_buffers();

	static void	draw_call(const std::vector<Vertex>& verticies, const std::vector<u16>& indices);

	static void	fill_vertex_buffer(const std::vector<Vertex>& verticies, u32 offset);
	static void	fill_index_buffer(const std::vector<u16>& indices, u32 offset);

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

private:	// Members
	static std::vector<VkSemaphore>	_image_available_semaphores;
	static std::vector<VkSemaphore>	_render_finished_semaphores;
	static std::vector<VkFence>		_in_flight_fences;

	static const u32				_frames_in_flight_count;
	static u32						_current_frame;

	static u32						_vertex_buffer_capacity;
	static Buffer*					_vertex_buffer;
	static Buffer*					_vertex_staging_buffer;

	static u32						_index_buffer_capacity;
	static Buffer*					_index_buffer;
	static Buffer*					_index_staging_buffer;
};

}

#endif //RENDERER_H
