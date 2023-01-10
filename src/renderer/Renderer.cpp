//
// Created by nathan on 1/10/23.
//

#include "Renderer.h"

#include "vulkan/VulkanInstance.h"

namespace Vulkan {

bool Renderer::initialize()
{
	if (!VulkanInstance::initialize())
		return false;
	return true;
}

void Renderer::shutdown()
{
	VulkanInstance::shutdown();
}

}