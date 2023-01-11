//
// Created by nathan on 1/10/23.
//

#include "Renderer.h"

#include "vulkan/VulkanInstance.h"
#include "vulkan/SwapchainManager.h"

namespace Vulkan {

bool Renderer::initialize()
{
	if (!VulkanInstance::initialize())
		return false;
	if (!SwapchainManager::initialize())
		return false;
	return true;
}

void Renderer::shutdown()
{
	SwapchainManager::shutdown();
	VulkanInstance::shutdown();
}

}