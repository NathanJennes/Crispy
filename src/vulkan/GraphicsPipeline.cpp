//
// Created by nathan on 1/12/23.
//

#include "GraphicsPipeline.h"
#include "utils.h"
#include "log.h"
#include "VulkanInstance.h"
#include "SwapchainManager.h"
#include "renderer/Renderer.h"

namespace Vulkan {

VkPipelineLayout		GraphicsPipeline::_pipeline_layout;
VkRenderPass			GraphicsPipeline::_render_pass;
VkPipeline				GraphicsPipeline::_pipeline;
VkDescriptorSetLayout	GraphicsPipeline::_descriptor_set_layout;

bool GraphicsPipeline::initialize()
{
	if (!initialize_render_pass())
		return false;

	if (!initialize_descriptor_sets())
		return false;

	auto vert_code = read_file("obj/shaders/shader.vert.spv");
	auto frag_code = read_file("obj/shaders/shader.frag.spv");

	if (vert_code.empty() || frag_code.empty()) {
		CORE_ERROR("Couldn't create the graphics pipeline: couldn't load SpirV shaders!");
		return false;
	}

	VkShaderModule vert_shader_module = create_shader_module(vert_code);
	VkShaderModule frag_shader_module = create_shader_module(frag_code);

	if (vert_shader_module == VK_NULL_HANDLE || frag_shader_module == VK_NULL_HANDLE) {
		CORE_ERROR("Couldn't create the graphics pipeline!");
		return false;
	}

	VkPipelineShaderStageCreateInfo vert_shader_create_infos{};
	vert_shader_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_create_infos.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_create_infos.module = vert_shader_module;
	vert_shader_create_infos.pName = "main";
	vert_shader_create_infos.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo frag_shader_create_infos{};
	frag_shader_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_create_infos.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_create_infos.module = frag_shader_module;
	frag_shader_create_infos.pName = "main";
	frag_shader_create_infos.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shader_stages[2] = {vert_shader_create_infos, frag_shader_create_infos};

	std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_infos{};
	dynamic_state_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_infos.dynamicStateCount = static_cast<u32>(dynamic_states.size());
	dynamic_state_create_infos.pDynamicStates = dynamic_states.data();

	auto binding_description = Vertex::get_binding_description();
	auto attribute_description = Vertex::get_attribute_description();

	VkPipelineVertexInputStateCreateInfo vertex_input_create_infos{};
	vertex_input_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_infos.vertexBindingDescriptionCount = 1;
	vertex_input_create_infos.vertexAttributeDescriptionCount = attribute_description.size();
	vertex_input_create_infos.pVertexBindingDescriptions = &binding_description;
	vertex_input_create_infos.pVertexAttributeDescriptions = attribute_description.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_infos{};
	input_assembly_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_create_infos.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_create_infos.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) SwapchainManager::swapchain_extent().width;
	viewport.height = (float) SwapchainManager::swapchain_extent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = SwapchainManager::swapchain_extent();

	VkPipelineViewportStateCreateInfo viewport_state_create_infos{};
	viewport_state_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_infos.viewportCount = 1;
	viewport_state_create_infos.pViewports = &viewport;
	viewport_state_create_infos.scissorCount = 1;
	viewport_state_create_infos.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f; // Optional
	color_blending.blendConstants[1] = 0.0f; // Optional
	color_blending.blendConstants[2] = 0.0f; // Optional
	color_blending.blendConstants[3] = 0.0f; // Optional

	VkPipelineDepthStencilStateCreateInfo depth_stencil_infos{};
	depth_stencil_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_infos.depthTestEnable = VK_TRUE;
	depth_stencil_infos.depthWriteEnable = VK_TRUE;
	depth_stencil_infos.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_infos.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_infos.minDepthBounds = 0.0f; // Optional
	depth_stencil_infos.maxDepthBounds = 1.0f; // Optional
	depth_stencil_infos.stencilTestEnable = VK_FALSE;
	depth_stencil_infos.front = {}; // Optional
	depth_stencil_infos.back = {}; // Optional

	VkPushConstantRange push_constants{};
	push_constants.offset = 0;
	push_constants.size = sizeof(glm::mat4);
	push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipeline_layout_create_infos{};
	pipeline_layout_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_infos.setLayoutCount = 1;
	pipeline_layout_create_infos.pSetLayouts = &_descriptor_set_layout;
	pipeline_layout_create_infos.pushConstantRangeCount = 1;
	pipeline_layout_create_infos.pPushConstantRanges = &push_constants;

	if (vkCreatePipelineLayout(VulkanInstance::logical_device(), &pipeline_layout_create_infos, nullptr, &_pipeline_layout) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create the graphics pipeline's layout!");
		vkDestroyShaderModule(VulkanInstance::logical_device(), vert_shader_module, nullptr);
		vkDestroyShaderModule(VulkanInstance::logical_device(), frag_shader_module, nullptr);
		return false;
	}

	VkGraphicsPipelineCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_infos.stageCount = 2;
	create_infos.pStages = shader_stages;
	create_infos.pVertexInputState = &vertex_input_create_infos;
	create_infos.pInputAssemblyState = &input_assembly_create_infos;
	create_infos.pViewportState = &viewport_state_create_infos;
	create_infos.pRasterizationState = &rasterizer;
	create_infos.pMultisampleState = &multisampling;
	create_infos.pDepthStencilState = &depth_stencil_infos;
	create_infos.pColorBlendState = &color_blending;
	create_infos.pDynamicState = &dynamic_state_create_infos;
	create_infos.layout = pipeline_layout();
	create_infos.renderPass = render_pass();
	create_infos.subpass = 0;
	create_infos.basePipelineHandle = VK_NULL_HANDLE;
	create_infos.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(VulkanInstance::logical_device(), VK_NULL_HANDLE, 1, &create_infos, nullptr, &_pipeline) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create the graphics pipeline!");
		vkDestroyShaderModule(VulkanInstance::logical_device(), vert_shader_module, nullptr);
		vkDestroyShaderModule(VulkanInstance::logical_device(), frag_shader_module, nullptr);
		return false;
	}

	vkDestroyShaderModule(VulkanInstance::logical_device(), vert_shader_module, nullptr);
	vkDestroyShaderModule(VulkanInstance::logical_device(), frag_shader_module, nullptr);

	return true;
}

VkShaderModule GraphicsPipeline::create_shader_module(const std::vector<char> &code)
{
	VkShaderModuleCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_infos.codeSize = code.size();
	create_infos.pCode = reinterpret_cast<const u32*>(code.data());

	VkShaderModule module{};
	if (vkCreateShaderModule(VulkanInstance::logical_device(), &create_infos, nullptr, &module) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create a shader module!");
		return VK_NULL_HANDLE;
	}
	return module;
}

void GraphicsPipeline::shutdown()
{
	vkDestroyRenderPass(VulkanInstance::logical_device(), render_pass(), nullptr);
	vkDestroyDescriptorSetLayout(VulkanInstance::logical_device(), descriptor_set_layout(), nullptr);
	vkDestroyPipelineLayout(VulkanInstance::logical_device(), pipeline_layout(), nullptr);
	vkDestroyPipeline(VulkanInstance::logical_device(), pipeline(), nullptr);

}

bool GraphicsPipeline::initialize_render_pass()
{
	VkAttachmentDescription color_attachment{};
	color_attachment.format = SwapchainManager::swapchain_image_format();
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depth_attachment{};
	depth_attachment.format = SwapchainManager::find_depth_format();
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref{};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = {color_attachment, depth_attachment};
	VkRenderPassCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_infos.attachmentCount = 2;
	create_infos.pAttachments = attachments;
	create_infos.subpassCount = 1;
	create_infos.pSubpasses = &subpass;
	create_infos.dependencyCount = 1;
	create_infos.pDependencies = &dependency;

	if (vkCreateRenderPass(VulkanInstance::logical_device(), &create_infos, nullptr, &_render_pass) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create the render pass!");
		return false;
	}

	return true;
}

bool GraphicsPipeline::initialize_descriptor_sets()
{
	VkDescriptorSetLayoutBinding mvp_layout_binding{};
	mvp_layout_binding.binding = 0;
	mvp_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_layout_binding.descriptorCount = 1;
	mvp_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	mvp_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_infos.bindingCount = 1;
	create_infos.pBindings = &mvp_layout_binding;

	if (vkCreateDescriptorSetLayout(VulkanInstance::logical_device(), &create_infos, nullptr, &_descriptor_set_layout) != VK_SUCCESS) {
		CORE_ERROR("Couldn't create the descriptor set!");
		return false;
	}
	return true;
}
} // Vulkan