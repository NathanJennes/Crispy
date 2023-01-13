//
// Created by nathan on 1/10/23.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "defines.h"

namespace Vulkan {

class Renderer
{
public:
	static bool initialize();
	static void shutdown();

	static void	draw_frame();

private:	// Methods
	static bool	create_sync_objects();

	//----
	// Getters
	//----
	static std::vector<VkSemaphore>&	image_available_semaphores()	{ return _image_available_semaphores; }
	static std::vector<VkSemaphore>&	render_finished_semaphores()	{ return _render_finished_semaphores; }
	static std::vector<VkFence>&		in_flight_fences()				{ return _in_flight_fences; }
	static u32							frames_in_flight_count()		{ return _frames_in_flight_count; }
	static u32							current_frame()					{ return _current_frame; }

private:	// Members
	static std::vector<VkSemaphore>	_image_available_semaphores;
	static std::vector<VkSemaphore>	_render_finished_semaphores;
	static std::vector<VkFence>		_in_flight_fences;

	static const u32				_frames_in_flight_count;
	static u32						_current_frame;
};

}

#endif //RENDERER_H
