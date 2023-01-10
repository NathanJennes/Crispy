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
	static bool initialize(const std::string& name, i32 x, i32 y, i32 width, i32 height);
	static void shutdown();
	static bool initialized();

	static void update();
	static bool should_close();

private:
	static bool initialize_window(i32 x, i32 y);

private:
	static bool _should_close;
	static bool _initialized;
	static std::string _name;
	static i32 _width, _height;

	//X11
	static Display *_display;
	static xcb_connection_t *_connection;
	static xcb_window_t _window;
	static xcb_screen_t *_screen;
	static xcb_atom_t _wm_protocols;
	static xcb_atom_t _wm_delete_win;

	//Vulkan
	static VkSurfaceKHR _surface;
};

}



#endif //WINDOW_H
