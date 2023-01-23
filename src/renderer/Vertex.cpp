//
// Created by nathan on 1/19/23.
//

#include "Vertex.h"
#include <vulkan/vulkan.h>

namespace Vulkan {

VkVertexInputBindingDescription Vertex::get_binding_description() {
	VkVertexInputBindingDescription binding_description{};
	binding_description.binding = 0;
	binding_description.stride = sizeof (Vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::get_attribute_description() {
	std::array<VkVertexInputAttributeDescription, 2> attribute_description{};
	attribute_description[0].binding = 0;
	attribute_description[0].offset = offsetof(Vertex, pos);
	attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_description[0].location = 0;
	attribute_description[1].binding = 0;
	attribute_description[1].offset = offsetof(Vertex, col);
	attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_description[1].location = 1;
	return attribute_description;
}

} // Vulkan