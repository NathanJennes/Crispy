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
	static bool				initialize_render_pass();

	//----
	// Getters
	//----
	static VkPipelineLayout	pipeline_layout()	{ return _pipeline_layout; }
	static VkRenderPass		render_pass()		{ return _render_pass; }

private:	// Members
	static VkPipelineLayout		_pipeline_layout;
	static VkRenderPass			_render_pass;
};
} // Vulkan

#endif //GRAPHICSPIPELINE_H
