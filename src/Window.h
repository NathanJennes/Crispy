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
	//----
	// Initialization
	//----
	static bool	initialize(const std::string& name, i32 x, i32 y, u32 width, u32 height);
	static bool	initialize_surface();
	static void	shutdown();

	//----
	// Updating
	//----
	static void	update();

	//----
	// Getters
	//----
	static bool			initialized() 	{ return _initialized; };
	static bool			should_close()	{ return _should_close; };
	static VkSurfaceKHR	surface()		{ return _surface; };
	static u32			width()			{ return _width; }
	static u32			height()		{ return _height; }

private:	// Methods
	//----
	// Initialization
	//----
	static bool	initialize_window(i32 x, i32 y);

	//----
	// Getters
	//----
	static const std::string&	name()					{ return _name; }
	static Display				*display()				{ return _display; }
	static xcb_connection_t		*connexion()			{ return _connection; }
	static xcb_window_t			window()				{ return _window; }
	static xcb_screen_t			*screen()				{ return _screen; }
	static xcb_atom_t			wm_protocols_mutex()	{ return _wm_protocols_mutex; }
	static xcb_atom_t			wm_delete_win_mutex()	{ return _wm_delete_win_mutex; }

private:	// Members
	static bool			_should_close;
	static bool			_initialized;
	static std::string	_name;
	static u32			_width, _height;

	//----
	// X11
	//----
	static Display			*_display;
	static xcb_connection_t	*_connection;
	static xcb_window_t		_window;
	static xcb_screen_t		*_screen;
	static xcb_atom_t		_wm_protocols_mutex;
	static xcb_atom_t		_wm_delete_win_mutex;

	//----
	// Vulkan
	//----
	static VkSurfaceKHR	_surface;
};

}



#endif //WINDOW_H
