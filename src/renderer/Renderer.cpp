//
// Created by nathan on 1/10/23.
//

#include "Renderer.h"

#include "vulkan/VulkanInstance.h"
#include "vulkan/SwapchainManager.h"
#include "vulkan/GraphicsPipeline.h"
#include "vulkan/CommandBuffers.h"

namespace Vulkan {

bool Renderer::initialize()
{
	if (!VulkanInstance::initialize())
		return false;
	if (!SwapchainManager::initialize())
		return false;
	if (!GraphicsPipeline::initialize())
		return false;
	if (!SwapchainManager::create_famebuffers())
		return false;
	if (!CommandBuffers::initialize())
		return false;
	return true;
}

void Renderer::shutdown()
{
	CommandBuffers::shutdown();
	GraphicsPipeline::shutdown();
	SwapchainManager::shutdown();
	VulkanInstance::shutdown();
}

}