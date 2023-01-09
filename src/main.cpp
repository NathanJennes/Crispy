//
// Created by nathan on 1/9/23.
//

#include <iostream>

#include "utils.h"
#include "Window.h"
#include "input.h"

int main()
{
	Vulkan::Window window("Vulkan app", 200, 200, 400, 400);
	if (!window.initialized())
		return (1);

	bool limited_framerate = true;
	f64 target_second_per_frame = 1.0 / 60.0;

	bool running = true;
	f64 last_time = get_absolute_time();

	while (running)
	{
		if (!window.update()) {
			running = false;
		}
		if (is_key_down(Vulkan::Keys::ESCAPE))
			break;

		// Updating delta_time, sleeping if necessary to follow target fps
		f64 current_time = get_absolute_time();
		f64 delta_time = current_time - last_time;

		if (limited_framerate && delta_time < target_second_per_frame) {
			// Sleep for the remaining time to get to the target fps
			u64 remaining_time = (target_second_per_frame - delta_time) * 1000000;
			my_sleep(remaining_time);

			// Update delta_time
			current_time = get_absolute_time();
			delta_time = current_time - last_time;
			last_time = current_time;
		} else {
			last_time = current_time;
		}
	}

	return(0);
}
