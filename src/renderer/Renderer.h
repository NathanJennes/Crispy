//
// Created by nathan on 1/10/23.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <optional>
#include "defines.h"
#include "math/vulkan_maths.h"

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

	static void	draw(const std::vector<Vertex>& verticies);

private:	// Methods
	static bool	create_sync_objects();
	static bool	create_vertex_buffer();

	static void	draw_call(const std::vector<Vertex>& verticies);

	static void	fill_vertex_buffer(const std::vector<Vertex>& verticies, u32 offset);

	static std::optional<u32>	find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties);

	//----
	// Getters
	//----
	static std::vector<VkSemaphore>&	image_available_semaphores()	{ return _image_available_semaphores; }
	static std::vector<VkSemaphore>&	render_finished_semaphores()	{ return _render_finished_semaphores; }
	static std::vector<VkFence>&		in_flight_fences()				{ return _in_flight_fences; }
	static u32							frames_in_flight_count()		{ return _frames_in_flight_count; }
	static u32							current_frame()					{ return _current_frame; }
	static VkBuffer						vertex_buffer()					{ return _vertex_buffer; }
	static VkDeviceMemory				vertex_buffer_memory()			{ return _vertex_buffer_memory; }
	static u32							vertex_buffer_capacity()		{ return _vertex_buffer_capacity; }

private:	// Members
	static std::vector<VkSemaphore>	_image_available_semaphores;
	static std::vector<VkSemaphore>	_render_finished_semaphores;
	static std::vector<VkFence>		_in_flight_fences;

	static const u32				_frames_in_flight_count;
	static u32						_current_frame;

	static u32						_vertex_buffer_capacity;
	static VkBuffer					_vertex_buffer;
	static VkDeviceMemory			_vertex_buffer_memory;
};

}

#endif //RENDERER_H
