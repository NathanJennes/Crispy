//
// Created by nathan on 1/9/23.
//

#ifndef UTILS_H
#define UTILS_H

#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <string>

#include "defines.h"

namespace Vulkan {

f64 get_absolute_time();
void my_sleep(u64 us);

std::vector<char> read_file(const std::string& file_name);

}

#endif //UTILS_H
