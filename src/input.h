//
// Created by nathan on 1/9/23.
//

#ifndef INPUT_H
#define INPUT_H

#include "input_codes.h"
#include "defines.h"

namespace Vulkan {

void update_key(Keys key, bool pressed);
void update_button(Buttons button, bool pressed);
void update_mouse_position(u32 x, u32 y);

bool is_key_down(Keys key);
bool is_button_down(Buttons button);

Buttons translate_button(u32 x_button);
Keys translate_keycode(u32 x_keycode);

}

#endif //INPUT_H
