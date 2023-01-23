//
// Created by nathan on 1/19/23.
//

#ifndef VERTEX_H
#define VERTEX_H

#include <vulkan/vulkan.h>
#include <array>
#include "glm/glm.hpp"

namespace Vulkan {

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 col;

	Vertex(const glm::vec3& position, const glm::vec3& color)
		:pos(position), col(color) {}

	static VkVertexInputBindingDescription get_binding_description();

	static std::array<VkVertexInputAttributeDescription, 2> get_attribute_description();
};
} // Vulkan

#endif //VERTEX_H
