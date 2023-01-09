//
// Created by nathan on 1/9/23.
//

#include <X11/keysym.h>
#include <xcb/xcb.h>

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

Keys translate_keycode(u32 x_keycode)
{
	switch (x_keycode) {
		case XK_BackSpace:
			return Keys::BACKSPACE;
		case XK_Return:
			return Keys::ENTER;
		case XK_Tab:
			return Keys::TAB;
		case XK_Pause:
			return Keys::PAUSE;
		case XK_Caps_Lock:
			return Keys::CAPITAL;
		case XK_Escape:
			return Keys::ESCAPE;
			// Not supported
			// case : return Keys::CONVERT;
			// case : return Keys::NONCONVERT;
			// case : return Keys::ACCEPT;
		case XK_Mode_switch:
			return Keys::MODECHANGE;
		case XK_space:
			return Keys::SPACE;
		case XK_Prior:
			return Keys::PRIOR;
		case XK_Next:
			return Keys::NEXT;
		case XK_End:
			return Keys::END;
		case XK_Home:
			return Keys::HOME;
		case XK_Left:
			return Keys::LEFT;
		case XK_Up:
			return Keys::UP;
		case XK_Right:
			return Keys::RIGHT;
		case XK_Down:
			return Keys::DOWN;
		case XK_Select:
			return Keys::SELECT;
		case XK_Print:
			return Keys::PRINT;
		case XK_Execute:
			return Keys::EXECUTE;
			// case XK_snapshot: return Keys::SNAPSHOT; // not supported
		case XK_Insert:
			return Keys::INSERT;
		case XK_Delete:
			return Keys::DELETE;
		case XK_Help:
			return Keys::HELP;
		case XK_Meta_L:
			return Keys::LWIN;  // TODO: not sure this is right
		case XK_Meta_R:
			return Keys::RWIN;
			// case XK_apps: return Keys::APPS; // not supported
			// case XK_sleep: return Keys::SLEEP; //not supported
		case XK_KP_0:
			return Keys::NUMPAD0;
		case XK_KP_1:
			return Keys::NUMPAD1;
		case XK_KP_2:
			return Keys::NUMPAD2;
		case XK_KP_3:
			return Keys::NUMPAD3;
		case XK_KP_4:
			return Keys::NUMPAD4;
		case XK_KP_5:
			return Keys::NUMPAD5;
		case XK_KP_6:
			return Keys::NUMPAD6;
		case XK_KP_7:
			return Keys::NUMPAD7;
		case XK_KP_8:
			return Keys::NUMPAD8;
		case XK_KP_9:
			return Keys::NUMPAD9;
		case XK_multiply:
			return Keys::MULTIPLY;
		case XK_KP_Add:
			return Keys::ADD;
		case XK_KP_Separator:
			return Keys::SEPARATOR;
		case XK_KP_Subtract:
			return Keys::SUBTRACT;
		case XK_KP_Decimal:
			return Keys::DECIMAL;
		case XK_KP_Divide:
			return Keys::DIVIDE;
		case XK_F1:
			return Keys::F1;
		case XK_F2:
			return Keys::F2;
		case XK_F3:
			return Keys::F3;
		case XK_F4:
			return Keys::F4;
		case XK_F5:
			return Keys::F5;
		case XK_F6:
			return Keys::F6;
		case XK_F7:
			return Keys::F7;
		case XK_F8:
			return Keys::F8;
		case XK_F9:
			return Keys::F9;
		case XK_F10:
			return Keys::F10;
		case XK_F11:
			return Keys::F11;
		case XK_F12:
			return Keys::F12;
		case XK_F13:
			return Keys::F13;
		case XK_F14:
			return Keys::F14;
		case XK_F15:
			return Keys::F15;
		case XK_F16:
			return Keys::F16;
		case XK_F17:
			return Keys::F17;
		case XK_F18:
			return Keys::F18;
		case XK_F19:
			return Keys::F19;
		case XK_F20:
			return Keys::F20;
		case XK_F21:
			return Keys::F21;
		case XK_F22:
			return Keys::F22;
		case XK_F23:
			return Keys::F23;
		case XK_F24:
			return Keys::F24;
		case XK_Num_Lock:
			return Keys::NUMLOCK;
		case XK_Scroll_Lock:
			return Keys::SCROLL;
		case XK_KP_Equal:
			return Keys::NUMPAD_EQUAL;
		case XK_Shift_L:
			return Keys::LSHIFT;
		case XK_Shift_R:
			return Keys::RSHIFT;
		case XK_Control_L:
			return Keys::LCONTROL;
		case XK_Control_R:
			return Keys::RCONTROL;
			// case XK_Menu
			// case XK_Menu
		case XK_semicolon:
			return Keys::SEMICOLON;
		case XK_plus:
			return Keys::PLUS;
		case XK_comma:
			return Keys::COMMA;
		case XK_minus:
			return Keys::MINUS;
		case XK_period:
			return Keys::PERIOD;
		case XK_slash:
			return Keys::SLASH;
		case XK_grave:
			return Keys::GRAVE;
		case XK_a:
		case XK_A:
			return Keys::A;
		case XK_b:
		case XK_B:
			return Keys::B;
		case XK_c:
		case XK_C:
			return Keys::C;
		case XK_d:
		case XK_D:
			return Keys::D;
		case XK_e:
		case XK_E:
			return Keys::E;
		case XK_f:
		case XK_F:
			return Keys::F;
		case XK_g:
		case XK_G:
			return Keys::G;
		case XK_h:
		case XK_H:
			return Keys::H;
		case XK_i:
		case XK_I:
			return Keys::I;
		case XK_j:
		case XK_J:
			return Keys::J;
		case XK_k:
		case XK_K:
			return Keys::K;
		case XK_l:
		case XK_L:
			return Keys::L;
		case XK_m:
		case XK_M:
			return Keys::M;
		case XK_n:
		case XK_N:
			return Keys::N;
		case XK_o:
		case XK_O:
			return Keys::O;
		case XK_p:
		case XK_P:
			return Keys::P;
		case XK_q:
		case XK_Q:
			return Keys::Q;
		case XK_r:
		case XK_R:
			return Keys::R;
		case XK_s:
		case XK_S:
			return Keys::S;
		case XK_t:
		case XK_T:
			return Keys::T;
		case XK_u:
		case XK_U:
			return Keys::U;
		case XK_v:
		case XK_V:
			return Keys::V;
		case XK_w:
		case XK_W:
			return Keys::W;
		case XK_x:
		case XK_X:
			return Keys::X;
		case XK_y:
		case XK_Y:
			return Keys::Y;
		case XK_z:
		case XK_Z:
			return Keys::Z;
		default:
			return Keys::INVALID;
	}
}

Buttons translate_button(u32 x_button)
{
	switch (x_button) {
		case XCB_BUTTON_INDEX_1:
			return Buttons::LEFT;
		case XCB_BUTTON_INDEX_2:
			return Buttons::MIDDLE;
		case XCB_BUTTON_INDEX_3:
			return Buttons::RIGHT;
		default:
			return Buttons::INVALID;
	}
}

}
