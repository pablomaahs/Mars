#include <mspch.h>

#include "platform/vulkan/vkVulkanFinish.h"

namespace ms
{

	vkVulkanFinish::vkVulkanFinish(VulkanRenderDevice& renderDevice, VulkanImage depthTexture)
		: vkRenderBase(renderDevice, depthTexture)
		, mShouldClearDepth(depthTexture.image != VK_NULL_HANDLE)
	{
		if (!CreateColorAndDepthRenderPass(renderDevice, mShouldClearDepth, &mRenderPass,
				RenderPassCreateInfo{ .clearColor_ = false, .clearDepth_ = false , .flags_ = eRenderPassBit_Last }))
		{
			exit(EXIT_FAILURE);
		}

		CreateColorAndDepthFramebuffers(renderDevice, mRenderPass, depthTexture.imageView, mSwapchainFramebuffers);
	}

	void vkVulkanFinish::FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage)
	{
		EASY_FUNCTION();

		const VkRect2D screenRect =
		{
			.offset = { 0, 0 },
			.extent = {.width = mFramebufferWidth, .height = mFramebufferHeight }
		};

		const VkRenderPassBeginInfo renderPassInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = mRenderPass,
			.framebuffer = mSwapchainFramebuffers[currentImage],
			.renderArea = screenRect
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(commandBuffer);
	}

}