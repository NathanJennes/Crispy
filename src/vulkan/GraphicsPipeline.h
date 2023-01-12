//
// Created by nathan on 1/12/23.
//

#ifndef GRAPHICSPIPELINE_H
#define GRAPHICSPIPELINE_H

#include <vulkan/vulkan_core.h>
#include <vector>

namespace Vulkan {

class GraphicsPipeline
{
public:
	//----
	// Initialization
	//----
	static bool initialize();
	static void shutdown();

private:	// Methods
	static VkShaderModule	create_shader_module(const std::vector<char>& code);

	//----
	// Getters
	//----
	static VkPipelineLayout	pipeline_layout()	{ return _pipeline_layout; }

private:	// Members
	static VkPipelineLayout		_pipeline_layout;
};
} // Vulkan

#endif //GRAPHICSPIPELINE_H
