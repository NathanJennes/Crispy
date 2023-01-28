#pragma once

#include "defines.h"

namespace Vulkan
{

enum class Keys {
    /** @brief An invalid key. */
    INVALID = 0x00,

    /** @brief The backspace key. */
    BACKSPACE = 0x08,
    /** @brief The enter key. */
    ENTER = 0x0D,
    /** @brief The tab key. */
    TAB = 0x09,
    /** @brief The shift key. */
    SHIFT = 0x10,
    /** @brief The Control/Ctrl key. */
    CONTROL = 0x11,

    /** @brief The pause key. */
    PAUSE = 0x13,
    /** @brief The Caps Lock key. */
    CAPITAL = 0x14,

    /** @brief The Escape key. */
    ESCAPE = 0x1B,

    CONVERT = 0x1C,
    NONCONVERT = 0x1D,
    ACCEPT = 0x1E,
    MODECHANGE = 0x1F,

    /** @brief The spacebar key. */
    SPACE = 0x20,
    PRIOR = 0x21,
    NEXT = 0x22,
    /** @brief The end key. */
    END = 0x23,
    /** @brief The home key. */
    HOME = 0x24,
    /** @brief The left arrow key. */
    LEFT = 0x25,
    /** @brief The up arrow key. */
    UP = 0x26,
    /** @brief The right arrow key. */
    RIGHT = 0x27,
    /** @brief The down arrow key. */
    DOWN = 0x28,
    SELECT = 0x29,
    PRINT = 0x2A,
    EXECUTE = 0x2B,
    SNAPSHOT = 0x2C,
    /** @brief The insert key. */
    INSERT = 0x2D,
    /** @brief The delete key. */
    DELETE = 0x2E,
    HELP = 0x2F,

    /** @brief The 0 key */
    ZERO = 0x30,
    /** @brief The 1 key */
    ONE = 0x31,
    /** @brief The 2 key */
    TWO = 0x32,
    /** @brief The 3 key */
    THREE = 0x33,
    /** @brief The 4 key */
    FOUR = 0x34,
    /** @brief The 5 key */
    FIVE = 0x35,
    /** @brief The 6 key */
    SIX = 0x36,
    /** @brief The 7 key */
    SEVEN = 0x37,
    /** @brief The 8 key */
    EIGHT = 0x38,
    /** @brief The 9 key */
    NINE = 0x39,

    /** @brief The A key. */
    A = 0x41,
    /** @brief The B key. */
    B = 0x42,
    /** @brief The C key. */
    C = 0x43,
    /** @brief The D key. */
    D = 0x44,
    /** @brief The E key. */
    E = 0x45,
    /** @brief The F key. */
    F = 0x46,
    /** @brief The G key. */
    G = 0x47,
    /** @brief The H key. */
    H = 0x48,
    /** @brief The I key. */
    I = 0x49,
    /** @brief The J key. */
    J = 0x4A,
    /** @brief The K key. */
    K = 0x4B,
    /** @brief The L key. */
    L = 0x4C,
    /** @brief The M key. */
    M = 0x4D,
    /** @brief The N key. */
    N = 0x4E,
    /** @brief The O key. */
    O = 0x4F,
    /** @brief The P key. */
    P = 0x50,
    /** @brief The Q key. */
    Q = 0x51,
    /** @brief The R key. */
    R = 0x52,
    /** @brief The S key. */
    S = 0x53,
    /** @brief The T key. */
    T = 0x54,
    /** @brief The U key. */
    U = 0x55,
    /** @brief The V key. */
    V = 0x56,
    /** @brief The W key. */
    W = 0x57,
    /** @brief The X key. */
    X = 0x58,
    /** @brief The Y key. */
    Y = 0x59,
    /** @brief The Z key. */
    Z = 0x5A,

    /** @brief The left Windows/Super key. */
    LWIN = 0x5B,
    /** @brief The right Windows/Super key. */
    RWIN = 0x5C,
    APPS = 0x5D,

    /** @brief The sleep key. */
    SLEEP = 0x5F,

    /** @brief The numberpad 0 key. */
    NUMPAD0 = 0x60,
    /** @brief The numberpad 1 key. */
    NUMPAD1 = 0x61,
    /** @brief The numberpad 2 key. */
    NUMPAD2 = 0x62,
    /** @brief The numberpad 3 key. */
    NUMPAD3 = 0x63,
    /** @brief The numberpad 4 key. */
    NUMPAD4 = 0x64,
    /** @brief The numberpad 5 key. */
    NUMPAD5 = 0x65,
    /** @brief The numberpad 6 key. */
    NUMPAD6 = 0x66,
    /** @brief The numberpad 7 key. */
    NUMPAD7 = 0x67,
    /** @brief The numberpad 8 key. */
    NUMPAD8 = 0x68,
    /** @brief The numberpad 9 key. */
    NUMPAD9 = 0x69,
    /** @brief The numberpad multiply key. */
    MULTIPLY = 0x6A,
    /** @brief The numberpad add key. */
    ADD = 0x6B,
    /** @brief The numberpad separator key. */
    SEPARATOR = 0x6C,
    /** @brief The numberpad subtract key. */
    SUBTRACT = 0x6D,
    /** @brief The numberpad decimal key. */
    DECIMAL = 0x6E,
    /** @brief The numberpad divide key. */
    DIVIDE = 0x6F,

    /** @brief The F1 key. */
    F1 = 0x70,
    /** @brief The F2 key. */
    F2 = 0x71,
    /** @brief The F3 key. */
    F3 = 0x72,
    /** @brief The F4 key. */
    F4 = 0x73,
    /** @brief The F5 key. */
    F5 = 0x74,
    /** @brief The F6 key. */
    F6 = 0x75,
    /** @brief The F7 key. */
    F7 = 0x76,
    /** @brief The F8 key. */
    F8 = 0x77,
    /** @brief The F9 key. */
    F9 = 0x78,
    /** @brief The F10 key. */
    F10 = 0x79,
    /** @brief The F11 key. */
    F11 = 0x7A,
    /** @brief The F12 key. */
    F12 = 0x7B,
    /** @brief The F13 key. */
    F13 = 0x7C,
    /** @brief The F14 key. */
    F14 = 0x7D,
    /** @brief The F15 key. */
    F15 = 0x7E,
    /** @brief The F16 key. */
    F16 = 0x7F,
    /** @brief The F17 key. */
    F17 = 0x80,
    /** @brief The F18 key. */
    F18 = 0x81,
    /** @brief The F19 key. */
    F19 = 0x82,
    /** @brief The F20 key. */
    F20 = 0x83,
    /** @brief The F21 key. */
    F21 = 0x84,
    /** @brief The F22 key. */
    F22 = 0x85,
    /** @brief The F23 key. */
    F23 = 0x86,
    /** @brief The F24 key. */
    F24 = 0x87,

    /** @brief The number lock key. */
    NUMLOCK = 0x90,

    /** @brief The scroll lock key. */
    SCROLL = 0x91,

    /** @brief The numberpad equal key. */
    NUMPAD_EQUAL = 0x92,

    /** @brief The left shift key. */
    LSHIFT = 0xA0,
    /** @brief The right shift key. */
    RSHIFT = 0xA1,
    /** @brief The left control key. */
    LCONTROL = 0xA2,
    /** @brief The right control key. */
    RCONTROL = 0xA3,
    /** @brief The left alt key. */
    LALT = 0xA4,
    /** @brief The right alt key. */
    RALT = 0xA5,

    /** @brief The semicolon key. */
    SEMICOLON = 0xBA,
    /** @brief The plus key. */
    COMMA = 0xBC,
    /** @brief The minus key. */
    MINUS = 0xBD,
    /** @brief The period key. */
    PERIOD = 0xBE,
    /** @brief The slash key. */
    SLASH = 0xBF,

    /** @brief The grave key. */
    GRAVE = 0xC0,

	/** @brief The apostrophe key */
	APOSTROPHE = 0xC1,
	/** @brief The equals key. */
	EQUAL = 0xC2,
	/** @brief The equals key. */
	LEFT_BRACKET = 0xC3,
	/** @brief The equals key. */
	BACKSLASH = 0xC4,
	/** @brief The equals key. */
	RIGHT_BRACKET = 0xC5,
	/** @brief The equals key. */
	PAGE_UP = 0xC6,
	/** @brief The equals key. */
	PAGE_DOWN = 0xC7,
	/** @brief The equals key. */
	CAPS_LOCK = 0xC8,
	/** @brief The equals key. */
	SCROLL_LOCK = 0xC9,
	/** @brief The equals key. */
	NUM_LOCK = 0xCA,
	/** @brief The equals key. */
	PRINT_SCREEN = 0xCB

};

enum class Buttons : i32 {
	/** @brief An invalid mouse button */
	INVALID = 0x00,

	/** @brief The left mouse button */
	LEFT = 0x01,
	/** @brief The right mouse button */
	RIGHT = 0x02,
	/** @brief The middle mouse button */
	MIDDLE = 0x03,

	/** @brief The left mouse button */
	BUTTON_4 = 0x04,
	/** @brief The right mouse button */
	BUTTON_5 = 0x05,
	/** @brief The middle mouse button */
	BUTTON_6 = 0x06,
	/** @brief The left mouse button */
	BUTTON_7 = 0x07,
	/** @brief The right mouse button */
	BUTTON_8 = 0x08

};

}
