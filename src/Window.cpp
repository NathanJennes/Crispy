//
// Created by nathan on 1/9/23.
//

#include "Window.h"
#include "log.h"
#include "input.h"
#include "vulkan/VulkanInstance.h"
#include "vulkan/vulkan_errors.h"

namespace Vulkan {

bool				Window::initialized = false;
bool				Window::has_resized = false;
bool				Window::visible = true;
std::string			Window::name;
u32					Window::width = 0, Window::height = 0;

VkSurfaceKHR		Window::surface = VK_NULL_HANDLE;
GLFWwindow			*Window::window = nullptr;

bool Window::initialize(const std::string &win_name, i32 x, i32 y, u32 win_width, u32 win_height)
{
	name = win_name;
	width = win_width;
	height = win_height;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		CORE_ERROR("Couldn't initialize glfw");
		return false;
	}

	initialize_window(x, y);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	return true;
}

void Window::shutdown()
{
	glfwTerminate();
}

void Window::destroy_surface()
{
	vkDestroySurfaceKHR(VulkanInstance::instance(), surface, nullptr);
}

bool Window::initialize_window(i32 x, i32 y)
{
	(void) x;
	(void) y;
	// Setup window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow((i32)width, (i32)height, name.c_str(), nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		CORE_ERROR("GLFW window creation failed!");
		return false;
	}
	CORE_TRACE("GLFW window created");

	return true;
}

void Window::update()
{
	has_resized = false;
	glfwPollEvents();
}

bool Window::initialize_surface()
{
	VkResult result = glfwCreateWindowSurface(VulkanInstance::instance(), window, nullptr, &surface);
	if (result != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a window surface: %s", vulkan_error_to_string(result));
		return false;
	}
	return true;
}

void Window::error_callback(int error, const char *description)
{
	(void) error;
	(void) description;
	CORE_WARN("Glfw error: %s\nERROR CODE: %d", description, error);
}

void Window::key_callback(GLFWwindow *from_window, int key, int scancode, int action, int mods)
{
	(void) scancode;
	(void) mods;
	(void) from_window;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(from_window, GLFW_TRUE);

	if (action == GLFW_PRESS)
		update_key(translate_keycode(key), true);
	if (action == GLFW_RELEASE)
		update_key(translate_keycode(key), false);
}

void Window::mouse_button_callback(GLFWwindow *from_window, int button, int action, int mods)
{
	(void) mods;
	(void) from_window;

	if (action == GLFW_PRESS)
		update_button(translate_button(button), true);
	if (action == GLFW_RELEASE)
		update_button(translate_button(button), false);
}

void Window::mouse_scroll_callback(GLFWwindow *from_window, double xoffset, double yoffset)
{
	(void) xoffset;
	(void) yoffset;
	(void) from_window;
}

void Window::mouse_position_callback(GLFWwindow *from_window, double xpos, double ypos)
{
    (void) window;
    (void) xpos;
    (void) ypos;
	(void) from_window;
}

void Window::framebuffer_size_callback(GLFWwindow *from_window, int new_width, int new_height)
{
	(void) from_window;

	CORE_DEBUG("Window resized to %dx%d", new_width, new_height);
	width = new_width;
	height = new_height;
	has_resized = true;
}

std::vector<const char *> Window::get_required_instance_extensions()
{
	u32 count;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	if (!extensions) {
		CORE_ERROR("Couldn't get glfw required extensions");
		return {};
	}

	std::vector<const char *> vec;
	for (u32 i = 0; i < count; i++)
		vec.push_back(extensions[i]);

	return vec;
}

bool Window::should_close()
{
	return glfwWindowShouldClose(window);
}

}
