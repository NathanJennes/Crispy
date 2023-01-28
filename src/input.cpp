//
// Created by nathan on 1/9/23.
//

#include "glfw3.h"

#include "input.h"

#define MAX_KEYS 256
#define MAX_BUTTONS 5

namespace Vulkan
{

static bool key_status[MAX_KEYS];
static bool button_status[MAX_BUTTONS];
static u32 mouse_x, mouse_y;

void update_key(Keys key, bool pressed)
{
	if (static_cast<i32>(key) >= MAX_KEYS)
		return ;
	key_status[static_cast<i32>(key)] = pressed;
}

void update_button(Buttons button, bool pressed)
{
	if (static_cast<i32>(button) >= MAX_BUTTONS)
		return ;
	button_status[static_cast<i32>(button)] = pressed;
}

void update_mouse_position(u32 x, u32 y)
{
	mouse_x = x;
	mouse_y = y;
}

bool is_key_down(Keys key)
{
	return key_status[static_cast<i32>(key)];
}

bool is_button_down(Buttons button)
{
	return button_status[static_cast<i32>(button)];
}

Keys translate_keycode(u32 keycode)
{
	switch (keycode) {
		case GLFW_KEY_SPACE:
			return Keys::SPACE;
		case GLFW_KEY_APOSTROPHE:
			return Keys::APOSTROPHE;
		case GLFW_KEY_COMMA:
			return Keys::COMMA;
		case GLFW_KEY_MINUS:
			return Keys::MINUS;
		case GLFW_KEY_PERIOD:
			return Keys::PERIOD;
		case GLFW_KEY_SLASH:
			return Keys::SLASH;
		case GLFW_KEY_0:
			return Keys::ZERO;
		case GLFW_KEY_1:
			return Keys::ONE;
		case GLFW_KEY_2:
			return Keys::TWO;
		case GLFW_KEY_3:
			return Keys::THREE;
		case GLFW_KEY_4:
			return Keys::FOUR;
		case GLFW_KEY_5:
			return Keys::FIVE;
		case GLFW_KEY_6:
			return Keys::SIX;
		case GLFW_KEY_7:
			return Keys::SEVEN;
		case GLFW_KEY_8:
			return Keys::EIGHT;
		case GLFW_KEY_9:
			return Keys::NINE;
		case GLFW_KEY_SEMICOLON:
			return Keys::SEMICOLON;
		case GLFW_KEY_EQUAL:
			return Keys::EQUAL;
		case GLFW_KEY_A:
			return Keys::A;
		case GLFW_KEY_B:
			return Keys::B;
		case GLFW_KEY_C:
			return Keys::C;
		case GLFW_KEY_D:
			return Keys::D;
		case GLFW_KEY_E:
			return Keys::E;
		case GLFW_KEY_F:
			return Keys::F;
		case GLFW_KEY_G:
			return Keys::G;
		case GLFW_KEY_H:
			return Keys::H;
		case GLFW_KEY_I:
			return Keys::I;
		case GLFW_KEY_J:
			return Keys::J;
		case GLFW_KEY_K:
			return Keys::K;
		case GLFW_KEY_L:
			return Keys::L;
		case GLFW_KEY_M:
			return Keys::M;
		case GLFW_KEY_N:
			return Keys::N;
		case GLFW_KEY_O:
			return Keys::O;
		case GLFW_KEY_P:
			return Keys::P;
		case GLFW_KEY_Q:
			return Keys::Q;
		case GLFW_KEY_R:
			return Keys::R;
		case GLFW_KEY_S:
			return Keys::S;
		case GLFW_KEY_T:
			return Keys::T;
		case GLFW_KEY_U:
			return Keys::U;
		case GLFW_KEY_V:
			return Keys::V;
		case GLFW_KEY_W:
			return Keys::W;
		case GLFW_KEY_X:
			return Keys::X;
		case GLFW_KEY_Y:
			return Keys::Y;
		case GLFW_KEY_Z:
			return Keys::Z;
		case GLFW_KEY_LEFT_BRACKET:
			return Keys::LEFT_BRACKET;
		case GLFW_KEY_BACKSLASH:
			return Keys::BACKSLASH;
		case GLFW_KEY_RIGHT_BRACKET:
			return Keys::RIGHT_BRACKET;
		case GLFW_KEY_GRAVE_ACCENT:
			return Keys::GRAVE;
		case GLFW_KEY_ESCAPE:
			return Keys::ESCAPE;
		case GLFW_KEY_ENTER:
			return Keys::ENTER;
		case GLFW_KEY_TAB:
			return Keys::TAB;
		case GLFW_KEY_BACKSPACE:
			return Keys::BACKSPACE;
		case GLFW_KEY_INSERT:
			return Keys::INSERT;
		case GLFW_KEY_DELETE:
			return Keys::DELETE;
		case GLFW_KEY_RIGHT:
			return Keys::RIGHT;
		case GLFW_KEY_LEFT:
			return Keys::LEFT;
		case GLFW_KEY_DOWN:
			return Keys::DOWN;
		case GLFW_KEY_UP:
			return Keys::UP;
		case GLFW_KEY_PAGE_UP:
			return Keys::PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:
			return Keys::PAGE_DOWN;
		case GLFW_KEY_HOME:
			return Keys::HOME;
		case GLFW_KEY_END:
			return Keys::END;
		case GLFW_KEY_CAPS_LOCK:
			return Keys::CAPS_LOCK;
		case GLFW_KEY_SCROLL_LOCK:
			return Keys::SCROLL_LOCK;
		case GLFW_KEY_NUM_LOCK:
			return Keys::NUM_LOCK;
		case GLFW_KEY_PRINT_SCREEN:
			return Keys::PRINT_SCREEN;
		case GLFW_KEY_PAUSE:
			return Keys::PAUSE;
		case GLFW_KEY_F1:
			return Keys::F1;
		case GLFW_KEY_F2:
			return Keys::F2;
		case GLFW_KEY_F3:
			return Keys::F3;
		case GLFW_KEY_F4:
			return Keys::F4;
		case GLFW_KEY_F5:
			return Keys::F5;
		case GLFW_KEY_F6:
			return Keys::F6;
		case GLFW_KEY_F7:
			return Keys::F7;
		case GLFW_KEY_F8:
			return Keys::F8;
		case GLFW_KEY_F9:
			return Keys::F9;
		case GLFW_KEY_F10:
			return Keys::F10;
		case GLFW_KEY_F11:
			return Keys::F11;
		case GLFW_KEY_F12:
			return Keys::F12;
		case GLFW_KEY_F13:
			return Keys::F13;
		case GLFW_KEY_F14:
			return Keys::F14;
		case GLFW_KEY_F15:
			return Keys::F15;
		case GLFW_KEY_F16:
			return Keys::F16;
		case GLFW_KEY_F17:
			return Keys::F17;
		case GLFW_KEY_F18:
			return Keys::F18;
		case GLFW_KEY_F19:
			return Keys::F19;
		case GLFW_KEY_F20:
			return Keys::F20;
		case GLFW_KEY_F21:
			return Keys::F21;
		case GLFW_KEY_F22:
			return Keys::F22;
		case GLFW_KEY_F23:
			return Keys::F23;
		case GLFW_KEY_F24:
			return Keys::F24;
		case GLFW_KEY_KP_0:
			return Keys::ZERO;
		case GLFW_KEY_KP_1:
			return Keys::ONE;
		case GLFW_KEY_KP_2:
			return Keys::TWO;
		case GLFW_KEY_KP_3:
			return Keys::THREE;
		case GLFW_KEY_KP_4:
			return Keys::FOUR;
		case GLFW_KEY_KP_5:
			return Keys::FIVE;
		case GLFW_KEY_KP_6:
			return Keys::SIX;
		case GLFW_KEY_KP_7:
			return Keys::SEVEN;
		case GLFW_KEY_KP_8:
			return Keys::EIGHT;
		case GLFW_KEY_KP_9:
			return Keys::NINE;
		case GLFW_KEY_KP_DECIMAL:
			return Keys::DECIMAL;
		case GLFW_KEY_KP_DIVIDE:
			return Keys::DIVIDE;
		case GLFW_KEY_KP_MULTIPLY:
			return Keys::MULTIPLY;
		case GLFW_KEY_KP_SUBTRACT:
			return Keys::SUBTRACT;
		case GLFW_KEY_KP_ADD:
			return Keys::ADD;
		case GLFW_KEY_KP_ENTER:
			return Keys::ENTER;
		case GLFW_KEY_KP_EQUAL:
			return Keys::EQUAL;
		case GLFW_KEY_LEFT_SHIFT:
			return Keys::LSHIFT;
		case GLFW_KEY_RIGHT_ALT:
			return Keys::RALT;
		case GLFW_KEY_LEFT_ALT:
			return Keys::LALT;
		case GLFW_KEY_RIGHT_CONTROL:
			return Keys::RCONTROL;
		case GLFW_KEY_LEFT_CONTROL:
			return Keys::LCONTROL;
		case GLFW_KEY_RIGHT_SUPER:
			return Keys::RWIN;
		case GLFW_KEY_LEFT_SUPER:
			return Keys::LWIN;
		default:
			return Keys::INVALID;
	}
}

Buttons translate_button(u32 button)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			return Buttons::LEFT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return Buttons::MIDDLE;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return Buttons::RIGHT;
		case GLFW_MOUSE_BUTTON_4:
            return Buttons::BUTTON_4;
		case GLFW_MOUSE_BUTTON_5:
            return Buttons::BUTTON_5;
		case GLFW_MOUSE_BUTTON_6:
            return Buttons::BUTTON_6;
		case GLFW_MOUSE_BUTTON_7:
            return Buttons::BUTTON_7;
		case GLFW_MOUSE_BUTTON_8:
            return Buttons::BUTTON_8;
		default:
			return Buttons::INVALID;
	}
}


}
