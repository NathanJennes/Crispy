//
// Created by nathan on 1/9/23.
//

#include <iostream>

#include "utils.h"
#include "Application.h"
#include "input.h"

int main()
{
	Vulkan::Application app("Vulkan app", 200, 200, 400, 400);

	bool limited_framerate = true;
	f64 target_second_per_frame = 1.0 / 60.0;

	f64 last_time = get_absolute_time();

	while (!app.should_close())
	{
		app.update();

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
