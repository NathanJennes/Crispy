//
// Created by nathan on 1/12/23.
//

#include "GraphicsPipeline.h"
#include "utils.h"
#include "log.h"
#include "VulkanInstance.h"
#include "SwapchainManager.h"

namespace Vulkan {

VkPipelineLayout	GraphicsPipeline::_pipeline_layout;
VkRenderPass		GraphicsPipeline::_render_pass;
VkPipeline			GraphicsPipeline::_pipeline;

bool GraphicsPipeline::initialize()
{
	if (!initialize_render_pass())
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

	VkPipelineVertexInputStateCreateInfo vertex_input_create_infos{};
	vertex_input_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_infos.vertexBindingDescriptionCount = 0;
	vertex_input_create_infos.vertexAttributeDescriptionCount = 0;
	vertex_input_create_infos.pVertexBindingDescriptions = nullptr;
	vertex_input_create_infos.pVertexAttributeDescriptions = nullptr;

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
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

	VkPipelineColorBlendAttachmentState colorBlend_attachment{};
	colorBlend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlend_attachment.blendEnable = VK_FALSE;
	colorBlend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &colorBlend_attachment;
	color_blending.blendConstants[0] = 0.0f; // Optional
	color_blending.blendConstants[1] = 0.0f; // Optional
	color_blending.blendConstants[2] = 0.0f; // Optional
	color_blending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipeline_layout_create_infos{};
	pipeline_layout_create_infos.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_infos.setLayoutCount = 0; // Optional
	pipeline_layout_create_infos.pSetLayouts = nullptr; // Optional
	pipeline_layout_create_infos.pushConstantRangeCount = 0; // Optional
	pipeline_layout_create_infos.pPushConstantRanges = nullptr; // Optional

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
	create_infos.pDepthStencilState = nullptr;
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
	vkDestroyPipeline(VulkanInstance::logical_device(), pipeline(), nullptr);
	vkDestroyPipelineLayout(VulkanInstance::logical_device(), pipeline_layout(), nullptr);
	vkDestroyRenderPass(VulkanInstance::logical_device(), render_pass(), nullptr);
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

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

	VkRenderPassCreateInfo create_infos{};
	create_infos.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_infos.attachmentCount = 1;
	create_infos.pAttachments = &color_attachment;
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
} // Vulkan