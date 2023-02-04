//
// Created by nathan on 1/10/23.
//

#include "Application.h"
#include "Window.h"
#include "input.h"
#include "vulkan/VulkanInstance.h"
#include "renderer/BasicRenderer.h"
#include "renderer/Camera.h"
#include <random>

namespace Vulkan {

Application::Application(const std::string &name, i32 x, i32 y, i32 width, i32 height)
	:_initialized_properly(false)
{
	if (!Window::initialize(name, x, y, width, height))
		return;
	if (!BasicRenderer::initialize())
		return;
	Camera::initialize({0.0f, 3.0f, -5.0f});

	std::vector<Vertex> vertices = {Vertex({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}),
											Vertex({5.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}),
											Vertex({5.0f, -5.0f, 0.0f}, {0.5f, 1.0f, 1.0f}),
											Vertex({0.0f, -5.0f, 0.0f}, {1.0f, 0.0f, 1.0f}),
											Vertex({0.0f, 0.0f, 5.0f}, {1.0f, 0.5f, 0.0f}),
											Vertex({5.0f, 0.0f, 5.0f}, {1.0f, 1.0f, 0.5f}),
											Vertex({5.0f, -5.0f, 5.0f}, {1.0f, 0.0f, 0.0f}),
											Vertex({0.0f, -5.0f, 5.0f}, {0.0f, 0.0f, 1.0f})};

	std::vector<u32> indices = {0, 1, 2, 0, 2, 3, 4, 5, 1, 4, 1, 0, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7, 3, 2, 6, 3, 6, 7, 5, 4, 7, 5, 7, 6};

	for (auto & model : models)
		model = BasicRenderer::load_model(vertices, indices);

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<> rand(-30.0f,30.0f);

	for (auto& position : positions) {
		position.x = rand(rng);
		position.y = rand(rng);
		position.z = rand(rng);
	}

	_initialized_properly = true;
}

Application::~Application()
{
	vkDeviceWaitIdle(VulkanInstance::logical_device());
	BasicRenderer::shutdown();
	Window::shutdown();
}

bool Application::should_close() const
{
	return !_initialized_properly || Window::should_close();
}

void Application::update()
{
	static double frames = 0.0;

	Window::update();
	Camera::update();

	BasicRenderer::begin_frame();
	for (u32 i = 0; i < models.size(); i++)
		BasicRenderer::draw(models[i], positions[i], glm::vec3(0.0f, frames, 0.0f));
	BasicRenderer::end_frame();
	frames += 0.005;
}

}
