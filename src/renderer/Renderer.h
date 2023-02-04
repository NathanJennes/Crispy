//
// Created by nathan on 1/10/23.
//

#ifndef RENDERER_H
#define RENDERER_H

#include "glm/glm.hpp"

namespace Vulkan {

struct CameraUBO
{
	glm::mat4 view_proj;
};

}

#endif //RENDERER_H
