//
// Created by nathan on 1/13/23.
//

#ifndef COMMANDBUFFERS_H
#define COMMANDBUFFERS_H

#include <vulkan/vulkan.h>
#include "defines.h"

namespace Vulkan {

class CommandBuffers
{
public:
	//----
	// Initialization
	//----
	static bool	initialize();
	static void	shutdown();

	//----
	// Command buffer creation
	//----
	static void	recore_command_buffer(VkCommandBuffer command_buffer, u32 image_index);

private:	// Methods
	//----
	// Getters
	//----
	static VkCommandBuffer	command_buffer()	{ return _command_buffer; }
	static VkCommandPool	command_pool()		{ return _command_pool; }

private:	// Members
	static VkCommandBuffer	_command_buffer;
	static VkCommandPool	_command_pool;
};
} // Vulkan

#endif //COMMANDBUFFERS_H
