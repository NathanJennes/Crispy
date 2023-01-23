//
// Created by nathan on 1/19/23.
//

#ifndef VULKAN_ERRORS_H
#define VULKAN_ERRORS_H

#include <vulkan/vulkan.h>

namespace Vulkan
{
const char* vulkan_error_to_string(VkResult result);
}

#endif //VULKAN_ERRORS_H
