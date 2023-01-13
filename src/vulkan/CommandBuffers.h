//
// Created by nathan on 1/13/23.
//

#ifndef COMMANDBUFFERS_H
#define COMMANDBUFFERS_H

#include <vulkan/vulkan.h>
#include "defines.h"
#include <vector>

namespace Vulkan {

class CommandBuffers
{
public:
	//----
	// Initialization
	//----
	static bool	initialize(u32 frames_in_flight_count);
	static void	shutdown();

	//----
	// Command buffer creation
	//----
	static void	record_command_buffer(VkCommandBuffer command_buffer, u32 image_index);

	//----
	// Getters
	//----
	static std::vector<VkCommandBuffer>&	command_buffers()	{ return _command_buffers; }
	static VkCommandBuffer&					get(u32 index)		{ return _command_buffers[index]; }

private:	// Methods
	//----
	// Getters
	//----
	static VkCommandPool&	command_pool()		{ return _command_pool; }

private:	// Members
	static std::vector<VkCommandBuffer>	_command_buffers;
	static VkCommandPool				_command_pool;
};
} // Vulkan

#endif //COMMANDBUFFERS_H
