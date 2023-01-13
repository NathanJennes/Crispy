//
// Created by nathan on 1/10/23.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>

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
	static VkSemaphore&	image_available_semaphore()	{ return _image_available_semaphore; }
	static VkSemaphore&	render_finished_semaphore()	{ return _render_finished_semaphore; }
	static VkFence&		in_flight_fence()			{ return _in_flight_fence; }

private:	// Members
	static VkSemaphore	_image_available_semaphore;
	static VkSemaphore	_render_finished_semaphore;
	static VkFence		_in_flight_fence;
};

}

#endif //RENDERER_H
