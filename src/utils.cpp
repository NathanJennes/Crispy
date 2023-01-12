//
// Created by nathan on 1/9/23.
//

#include "utils.h"
#include "log.h"
#include <fstream>

namespace Vulkan {

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

std::vector<char> read_file(const std::string& file_name)
{
	std::ifstream file(file_name, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		CORE_WARN("Couldn't read file [%s]", file_name.c_str());
		return {};
	}

	size_t file_size = file.tellg();
	std::vector<char> buffer(file_size);
	file.seekg(0);
	file.read(buffer.data(), file_size);
	file.close();
	return buffer;
}

}
