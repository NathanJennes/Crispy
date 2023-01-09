//
// Created by nathan on 1/9/23.
//

#include "utils.h"

f64 get_absolute_time()
{
	struct timespec now{};
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (now.tv_sec + now.tv_nsec * 0.000000001);
}

void my_sleep(u64 us)
{
#if _POSIX_C_SOURCE >= 199309L
	struct timespec ts{};
	ts.tv_sec = us / 1000000;
	ts.tv_nsec = (us % 1000000) * 1000;
	nanosleep(&ts, nullptr);
#else
	usleep(us);
#endif
}
