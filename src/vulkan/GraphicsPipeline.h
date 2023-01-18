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

	//----
	// Getters
	//----
	static VkRenderPass&			render_pass()			{ return _render_pass; }
	static VkPipeline&				pipeline()				{ return _pipeline; }
	static VkDescriptorSetLayout	descriptor_set_layout()	{ return _descriptor_set_layout; };
	static VkPipelineLayout&		pipeline_layout()		{ return _pipeline_layout; }

private:	// Methods
	static VkShaderModule	create_shader_module(const std::vector<char>& code);
	static bool				initialize_render_pass();
	static bool				initialize_descriptor_sets();

	//----
	// Getters
	//----

private:	// Members
	static VkDescriptorSetLayout	_descriptor_set_layout;
	static VkPipelineLayout			_pipeline_layout;
	static VkRenderPass				_render_pass;
	static VkPipeline				_pipeline;
};
} // Vulkan

#endif //GRAPHICSPIPELINE_H
