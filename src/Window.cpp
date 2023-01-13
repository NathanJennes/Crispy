//
// Created by nathan on 1/9/23.
//

#include <cstring>

#include "Window.h"
#include "log.h"
#include "input.h"
#include "vulkan/VulkanInstance.h"

namespace Vulkan {

bool				Window::_should_close = false;
bool				Window::_initialized = false;
bool				Window::_has_resized = false;
std::string			Window::_name;
u32					Window::_width = 0, Window::_height = 0;

Display				*Window::_display = nullptr;
xcb_connection_t	*Window::_connection = nullptr;
xcb_window_t		Window::_window = 0;
xcb_screen_t		*Window::_screen = nullptr;
xcb_atom_t			Window::_wm_protocols_mutex = 0;
xcb_atom_t			Window::_wm_delete_win_mutex = 0;

VkSurfaceKHR		Window::_surface = VK_NULL_HANDLE;

bool Window::initialize(const std::string &name, i32 x, i32 y, u32 width, u32 height)
{
	_name = name;
	_width = width;
	_height = height;
	_should_close = false;

	_initialized = initialize_window(x, y);
	if (initialized())
	{
		CORE_TRACE("Window successfully created !");
		return true;
	}

	CORE_ERROR("Couldn't create the _window !");
	_should_close = true;
	return false;
}

void Window::shutdown()
{
	// Turn key repeat back on
	XAutoRepeatOn(display());

	// Destroys the _window
	xcb_destroy_window(connexion(), window());
}

bool Window::initialize_window(i32 x, i32 y)
{
	// Connect to X
	_display = XOpenDisplay(nullptr);

	// Turn off key repeat
	XAutoRepeatOff(display());

	// Get the _connection from the _display
	_connection = XGetXCBConnection(display());

	if (xcb_connection_has_error(connexion()))
	{
		CORE_FATAL("Failed to connect to X server via XCB");
		return (false);
	}

	// Get data from the X server
	const struct xcb_setup_t *setup = xcb_get_setup(connexion());

	// Loop through screens
	xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
	i32 screen_p = 0;
	for (i32 i = screen_p; i > 0; i--)
	{
		xcb_screen_next(&it);
	}

	// After looping through the screens, assign it
	_screen = it.data;

	// Allocate a XID for the _window to be created
	_window = xcb_generate_id(connexion());

	// Register event types
	// XCB_CW_BACK_PIXEL = filling the _window background with a single color
	// XCB_CW_EVENT_MASK is required
	u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

	// Listen for keyboard and mouse buttons
	u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
					   XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
					   XCB_EVENT_MASK_STRUCTURE_NOTIFY;

	// Values to be sent over XCB
	u32 value_list[] = {_screen->black_pixel, event_values};

	// Creating the _window
	xcb_create_window(connexion(), XCB_COPY_FROM_PARENT, window(),
		_screen->root, static_cast<i16>(x), static_cast<i16>(y), width(), height(), 0, // No border
		XCB_WINDOW_CLASS_INPUT_OUTPUT, _screen->root_visual, event_mask, value_list);

	// Change the title
	xcb_change_property(connexion(), XCB_PROP_MODE_REPLACE, window(), XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
		8, // Data should be viewed 8 bits at a time
		name().size(), name().data());

	// Tells the server to notify when the _window manager attempts to destroy the _window
	xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(connexion(), 0, strlen("WM_DELETE_WINDOW"),
		"WM_DELETE_WINDOW");

	xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(connexion(), 0, strlen("WM_PROTOCOLS"),
		"WM_PROTOCOLS");

	xcb_intern_atom_reply_t *wm_delete_reply = xcb_intern_atom_reply(connexion(), wm_delete_cookie, nullptr);

	xcb_intern_atom_reply_t *wm_protocols_reply = xcb_intern_atom_reply(connexion(), wm_protocols_cookie, nullptr);

	_wm_delete_win_mutex = wm_delete_reply->atom;
	_wm_protocols_mutex = wm_protocols_reply->atom;

	xcb_change_property(connexion(), XCB_PROP_MODE_REPLACE, window(), wm_protocols_reply->atom, 4, 32, 1,
		&wm_delete_reply->atom);

	// Map the _window to the _screen
	xcb_map_window(connexion(), window());

	// Flush the stream
	i32 stream_result = xcb_flush(connexion());
	if (stream_result <= 0)
	{
		CORE_FATAL("An error occured when flushing the XCB stream: %d", stream_result);
		return (false);
	}

	return (true);
}

void Window::update()
{
	if (has_resized())
		_has_resized = false;

	if (should_close() || !initialized())
		return ;

	xcb_generic_event_t *event = xcb_poll_for_event(connexion());
	xcb_client_message_event_t *client_message;
	bool quit_flagged = false;

	// Poll events until NULL is returned
	while (event != nullptr) {
		switch (event->response_type & ~0x80) {
			// Keyboard key presses
			case XCB_KEY_PRESS:
			case XCB_KEY_RELEASE: {
				// xcb_key_press_event_t and xcb_key_release_event_t are the same
				xcb_key_press_event_t *kb_event = (xcb_key_press_event_t *)event;
				bool pressed = event->response_type == XCB_KEY_PRESS;
				xcb_keycode_t code = kb_event->detail;
				KeySym key_sym = XkbKeycodeToKeysym(display(), (KeyCode)code, 0, code & ShiftMask ? 1 : 0);

				// Translate the keycode and update the input system
				Keys key = translate_keycode(key_sym);
				if (key != Keys::INVALID)
					update_key(key, pressed);
			} break;

				// Mouse button presses
			case XCB_BUTTON_PRESS:
			case XCB_BUTTON_RELEASE: {
				xcb_button_press_event_t *mouse_event = (xcb_button_press_event_t *)event;
				bool pressed = event->response_type == XCB_BUTTON_PRESS;

				// Translate the button and update the input system
				Buttons button = translate_button(mouse_event->detail);
				if (button != Buttons::INVALID)
					update_button(button, pressed);
			} break;

				// Mouse movement
			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t *move_event = (xcb_motion_notify_event_t *)event;
				update_mouse_position(move_event->event_x, move_event->event_y);
			} break;

			case XCB_CONFIGURE_NOTIFY: {
				// Window resizing
				const xcb_configure_notify_event_t *cfgEvent = (const xcb_configure_notify_event_t *)event;
				if (cfgEvent->width != width() || cfgEvent->height != height())
				{
					_width = cfgEvent->width;
					_height = cfgEvent->height;
					_has_resized = true;
				}
			} break;

			case XCB_CLIENT_MESSAGE: {
				client_message = (xcb_client_message_event_t *)event;

				// Window close
				if (client_message->data.data32[0] == wm_delete_win_mutex()) {
					quit_flagged = true;
				}
			} break;
			default:
				break;
		}
		free(event);
		event = xcb_poll_for_event(connexion());
	}

	if (quit_flagged)
		_should_close = true;
}

bool Window::initialize_surface()
{
	VkXcbSurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	create_info.connection = connexion();
	create_info.window = window();

	if (vkCreateXcbSurfaceKHR(VulkanInstance::instance(), &create_info, nullptr, &_surface) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a vulkan xcb surface");
		return false;
	}
	return true;
}
}
