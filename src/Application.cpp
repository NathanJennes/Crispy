//
// Created by nathan on 1/10/23.
//

#include "Application.h"
#include "Window.h"
#include "renderer/Renderer.h"
#include "input.h"
#include "vulkan/VulkanInstance.h"

namespace Vulkan {

Application::Application(const std::string &name, i32 x, i32 y, i32 width, i32 height)
	:_initialized_properly(false)
{
	if (!Window::initialize(name, x, y, width, height))
		return;
	if (!Renderer::initialize())
		return;

	_initialized_properly = true;
}

Application::~Application()
{
	vkDeviceWaitIdle(VulkanInstance::logical_device());
	Renderer::shutdown();
	Window::shutdown();
}

bool Application::should_close()
{
	return !_initialized_properly || Window::should_close() || is_key_down(Keys::ESCAPE);
}

void Application::update()
{
	Window::update();
	Renderer::draw_frame();
}

}
