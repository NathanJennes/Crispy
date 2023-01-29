//
// Created by nathan on 1/9/23.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <vulkan/vulkan.h>
#include "glfw3.h"
#include <sys/time.h>
#include <string>
#include <vector>
#include "glm/vec2.hpp"

#include "defines.h"

namespace Vulkan {

class Window
{
public:
	//----
	// Initialization
	//----
	static bool							initialize(const std::string& win_name, i32 x, i32 y, u32 win_width, u32 win_height);
	static bool							initialize_surface();
	static void							destroy_surface();
	static void							shutdown();
	static std::vector<const char *>	get_required_instance_extensions();

	//----
	// Updating
	//----
	static void	update();
	static bool	should_close();

	//----
	// Getters
	//----
	static bool					is_initialized() 		{ return initialized; }
	static bool					did_resize()			{ return has_resized; }
	static bool					is_visible()			{ return visible; }
	static const VkSurfaceKHR&	get_surface()			{ return surface; }
	static u32					get_width()				{ return width; }
	static u32					get_height()			{ return height; }
	static float				get_aspect_ratio()		{ return aspec_ratio; }
	static float				is_mouse_locked()		{ return mouse_locked; }
	static const glm::vec2&		get_last_mouse_pos()	{ return last_mouse_pos; }
	static const glm::vec2&		get_mouse_pos()			{ return mouse_pos; }

private:	// Methods
	//----
	// Initialization
	//----
	static bool	initialize_window(i32 x, i32 y);

	//----
	// Glfw
	//----
	static void	error_callback(int error, const char *description);
	static void	key_callback(GLFWwindow *from_window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow *from_window, int button, int action, int mods);
	static void mouse_scroll_callback(GLFWwindow *from_window, double xoffset, double yoffset);
	static void mouse_position_callback(GLFWwindow *from_window, double xpos, double ypos);
	static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

private:	// Members
	static bool			initialized;
	static bool			has_resized;
	static bool			visible;

	static std::string	name;
	static u32			width, height;
	static float		aspec_ratio;

	static bool			mouse_locked;

	static glm::vec2	last_mouse_pos;
	static glm::vec2	mouse_pos;

	//----
	// Glfw
	//----
	static	GLFWwindow	*window;

	//----
	// Vulkan
	//----
	static VkSurfaceKHR	surface;
};

}

#endif //WINDOW_H
