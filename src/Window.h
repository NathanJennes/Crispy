//
// Created by nathan on 1/9/23.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <sys/time.h>

#include <vulkan/vulkan.h>
#include <string>

#include "defines.h"

namespace Vulkan {

class Window
{
public:
	Window(const std::string& name, i32 x, i32 y, i32 width, i32 height);
	~Window();

	bool initialized();

	bool update();


private:
	bool initialize_window(i32 x, i32 y);

private:
	bool _initialized;
	std::string _name;
	i32 _width, _height;

	//X11
	Display *_display;
	xcb_connection_t *_connection;
	xcb_window_t _window;
	xcb_screen_t *_screen;
	xcb_atom_t _wm_protocols;
	xcb_atom_t _wm_delete_win;

	//Vulkan
	VkSurfaceKHR _surface;
};

}



#endif //WINDOW_H
