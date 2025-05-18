//
// Created by Hayden Rivas on 1/16/25.
//
#include "Slate/Common/Debug.h"

#include "Slate/Common/FastSTD.h"
#include "Slate/PipelineBuilder.h"
#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vkenums.h"

#include <volk.h>

namespace Slate {
	// some helper functions //
	bool isIntegarFormat(VkFormat format) {
		return (format == VK_FORMAT_R8_UINT || format == VK_FORMAT_R16_UINT || format == VK_FORMAT_R32_UINT ||
			format == VK_FORMAT_R8G8_UINT || format == VK_FORMAT_R16G16_UINT || format == VK_FORMAT_R32G32_UINT ||
			format == VK_FORMAT_R8G8B8_UINT || format == VK_FORMAT_R16G16B16_UINT || format == VK_FORMAT_R32G32B32_UINT ||
			format == VK_FORMAT_R8G8B8A8_UINT || format == VK_FORMAT_R16G16B16A16_UINT || format == VK_FORMAT_R32G32B32A32_UINT);
	}

	void PipelineBuilder::Clear() {
		// clear all of the structs we need back to 0 with their correct stype
		_inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		_rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		_multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		_depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		_renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

		_colorBlendAttachment = {};
		// vector clears
		_colorAttachmentFormats.clear();
		_shaderStages.clear();
	}
	// build() needs to be the last function called on the builder!
	VkPipeline PipelineBuilder::build(VkDevice device, VkPipelineLayout layout) {
		// the create info for the pipeline we are building
		VkGraphicsPipelineCreateInfo graphics_pipeline_ci = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .pNext = nullptr };

		// --- shaders --- //
		graphics_pipeline_ci.stageCount = static_cast<uint32_t>(_shaderStages.size());
		graphics_pipeline_ci.pStages = _shaderStages.data();

		// ---- colorblending ---- //
		VkPipelineColorBlendStateCreateInfo colorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .pNext = nullptr };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		// copy the set blend mode into our attachments, we than do some conditional modifying
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(_colorAttachmentFormats.size(), _colorBlendAttachment);
		// if blending is enabled in one of the helper functions, this catches formats that cant use transparency and disables it
		// disable blending if format doesnt accept it
		for (int i = 0; i < blendAttachments.size(); i++) {
			if (isIntegarFormat(_colorAttachmentFormats[i])) {
				blendAttachments[i].blendEnable = VK_FALSE;
			}
		}
		colorBlending.attachmentCount = blendAttachments.size();
		colorBlending.pAttachments = blendAttachments.data();
		std::fill(std::begin(colorBlending.blendConstants), std::end(colorBlending.blendConstants), 0.f);
		graphics_pipeline_ci.pColorBlendState = &colorBlending;

		// ----- dynamic states ----- //
		VkDynamicState state[8] = { VK_DYNAMIC_STATE_VIEWPORT,
								   VK_DYNAMIC_STATE_SCISSOR,
								   VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
								   VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
								   VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
								   VK_DYNAMIC_STATE_DEPTH_BIAS,
								   VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
								   VK_DYNAMIC_STATE_BLEND_CONSTANTS };
		VkPipelineDynamicStateCreateInfo dynamicInfo = vkinfo::CreatePipelineDynamicStateInfo(state, 8);
		graphics_pipeline_ci.pDynamicState = &dynamicInfo;

		// ------ vertex input ig doesnt matter because we are using (push constants + device address) ----- //
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, .pNext = nullptr };
		graphics_pipeline_ci.pVertexInputState = &_vertexInputInfo;

		// ------ viewport state ------ //
		VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .pNext = nullptr };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		graphics_pipeline_ci.pViewportState = &viewportState;

		// everything else that doesnt need configuring on build()
		graphics_pipeline_ci.pNext = &_renderInfo;
		graphics_pipeline_ci.pInputAssemblyState = &_inputAssembly;
		graphics_pipeline_ci.pRasterizationState = &_rasterizer;
		graphics_pipeline_ci.pMultisampleState = &_multisampling;
		graphics_pipeline_ci.pDepthStencilState = &_depthStencil; // for setncil operations which are not dynamic

		graphics_pipeline_ci.layout = layout; // just the user given layout

		// -- actual creation -- //
		VkPipeline newPipeline = VK_NULL_HANDLE;
		VK_CHECK(vkCreateGraphicsPipelines(device, nullptr, 1, &graphics_pipeline_ci, nullptr, &newPipeline));
		this->Clear(); // clear the entire pipeline struct to reuse the PipelineBuilder
		return newPipeline;
	}
	PipelineBuilder& PipelineBuilder::set_moduleEXT(const VkShaderModule& vertModule, const VkShaderModule& fragModule) {
		_shaderStages.clear();
		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main"));
		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main"));
		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_module(const VkShaderModule& module) {
		_shaderStages.clear();
		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, module, "vs_main"));
		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, module, "fs_main"));

		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_polygon_mode(PolygonMode polygonMode) {
		_rasterizer.polygonMode = toVulkan(polygonMode);
		_rasterizer.lineWidth = 1.0f; // we cant change width, metal doesnt support wide lines

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_topology_mode(TopologyMode topoMode) {
		VkPrimitiveTopology topology = toVulkan(topoMode);
		_inputAssembly.topology = topology;
		_inputAssembly.primitiveRestartEnable = VK_TRUE;
		if (topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST 				||
			topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST 				||
			topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST 			||
			topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY  ||
			topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY) _inputAssembly.primitiveRestartEnable = VK_FALSE;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_cull_mode(CullMode cullMode) {
		_rasterizer.cullMode = toVulkan(cullMode);
		_rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_multisampling_mode(SampleCount count) {
		_multisampling.sampleShadingEnable = VK_FALSE;
		_multisampling.rasterizationSamples = toVulkan(count);
		_multisampling.minSampleShading = 1.0f;
		_multisampling.pSampleMask = nullptr;
		_multisampling.alphaToCoverageEnable = VK_FALSE;
		_multisampling.alphaToOneEnable = VK_FALSE;

		return *this;
	}
	// for multiple formats
	PipelineBuilder& PipelineBuilder::set_color_formats(std::span<VkFormat> formats) {
		for (const VkFormat& format : formats) {
			_colorAttachmentFormats.push_back(format);
		}
		_renderInfo.colorAttachmentCount = formats.size();
		_renderInfo.pColorAttachmentFormats = formats.data();

		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_depth_format(VkFormat format) {
		_renderInfo.depthAttachmentFormat = format;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_depthtestEXT(DepthMode mode) {
		// variables that remain the same regardless of mode
		_depthStencil.depthBoundsTestEnable = VK_FALSE;
		_depthStencil.stencilTestEnable = VK_FALSE;
		_depthStencil.front = {};
		_depthStencil.back = {};
		_depthStencil.minDepthBounds = 0.f;
		_depthStencil.maxDepthBounds = 1.f;

		switch (mode) {
			case DepthMode::NONE:
				_depthStencil.depthTestEnable = VK_FALSE;
				_depthStencil.depthWriteEnable = VK_FALSE;
				_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
				break;
			case DepthMode::LESS:
				_depthStencil.depthTestEnable = VK_TRUE;
				_depthStencil.depthWriteEnable = VK_TRUE;
				_depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // was .. or equal
				break;
			case DepthMode::EQUAL:
				_depthStencil.depthTestEnable = VK_TRUE;
				_depthStencil.depthWriteEnable = VK_FALSE;
				_depthStencil.depthCompareOp = VK_COMPARE_OP_EQUAL;
				break;
			case DepthMode::GREATER:
				_depthStencil.depthTestEnable = VK_TRUE;
				_depthStencil.depthWriteEnable = VK_TRUE;
				_depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
				break;
			case DepthMode::ALWAYS:
				_depthStencil.depthTestEnable = VK_TRUE;
				_depthStencil.depthWriteEnable = VK_TRUE;
				_depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
				break;
		}
		return *this;
	}

	void PipelineBuilder::_setBlendtoOff() {
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		_colorBlendAttachment.blendEnable = VK_FALSE;
	}
	void PipelineBuilder::_setBlendtoAdditive() {
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		_colorBlendAttachment.blendEnable = VK_TRUE;
		_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;

		_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE; // diff

		_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	void PipelineBuilder::_setBlendtoAlphaBlend() {
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		_colorBlendAttachment.blendEnable = VK_TRUE;
		_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;

		_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // diff

		_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // diff
		_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	PipelineBuilder& PipelineBuilder::set_blending_mode(BlendingMode mode) {
		switch (mode) {
			case BlendingMode::OFF:
				this->_setBlendtoOff();
				break;
			case BlendingMode::ADDITIVE:
				this->_setBlendtoAdditive();
				break;
			case BlendingMode::ALPHA_BLEND:
				this->_setBlendtoAlphaBlend();
				break;
		}
		return *this;
	}

}



