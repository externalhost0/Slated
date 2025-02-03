//
// Created by Hayden Rivas on 1/15/25.
//
#pragma once
#include <span>
#include <vulkan/vulkan.h>

namespace Slate::vkinfo {
	VkCommandPoolCreateInfo CreateCommandPoolInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo CreateCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count);
	VkCommandBufferBeginInfo CreateCommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
	VkCommandBufferSubmitInfo CreateCommandBufferSubmitInfo(VkCommandBuffer cmd);

	VkFenceCreateInfo CreateFenceInfo(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo CreateSemaphoreInfo(VkSemaphoreCreateFlags flags = 0);
	VkSemaphoreSubmitInfo CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

	VkPipelineShaderStageCreateInfo CreatePipelineShaderStageInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry = "main");
	VkPipelineLayoutCreateInfo CreatePipelineLayoutInfo();

	VkRenderingAttachmentInfo CreateColorAttachmentInfo(VkImageView view, VkClearColorValue* clear, VkImageView resolveView = nullptr, VkResolveModeFlags resolveFlags = VK_RESOLVE_MODE_AVERAGE_BIT);
	VkRenderingAttachmentInfo CreateDepthAttachmentInfo(VkImageView view, VkClearDepthStencilValue* clear);
	VkRenderingAttachmentInfo CreateDepthStencilAttachmentInfo(VkImageView view, VkClearDepthStencilValue* clear);

	VkRenderingInfo CreateRenderingInfo(VkExtent2D extent2D, VkRenderingAttachmentInfo* attachment, VkRenderingAttachmentInfo* depthAttachment);
	VkRenderingInfo CreateRenderingInfo(VkExtent2D extent2D, std::span<VkRenderingAttachmentInfo> colorAttachments, VkRenderingAttachmentInfo* depthAttachment);

	VkSubmitInfo2 CreateSubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);

	VkPipelineDynamicStateCreateInfo CreatePipelineDynamicStateInfo(const VkDynamicState* states, uint32_t statescount, VkPipelineDynamicStateCreateFlags flags = 0);

	VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkImageCreateInfo CreateImageInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage_flags, uint32_t mipmap_levels = 1, VkSampleCountFlagBits sample_flag = VK_SAMPLE_COUNT_1_BIT);
	VkImageSubresourceRange CreateImageSubresourceRange(VkImageAspectFlags aspectMask);

}





