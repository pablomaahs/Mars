#include "mspch.h"

#include "vkVulkanClear.h"

namespace ms
{

	vkVulkanClear::vkVulkanClear(VulkanRenderDevice& renderDevice, VulkanImage depthTexture)
		: vkRenderBase(renderDevice, depthTexture)
		, mShouldClearDepth(depthTexture.image != VK_NULL_HANDLE)
	{
		if (!CreateColorAndDepthRenderPass(renderDevice, mShouldClearDepth, &mRenderPass,
				RenderPassCreateInfo { .clearColor_ = true, .clearDepth_ = true, .flags_ = eRenderPassBit_First }, VK_FORMAT_R8G8B8A8_UNORM))
		{
			exit(EXIT_FAILURE);
		}

		CreateColorAndDepthFramebuffers(renderDevice, mRenderPass, depthTexture.imageView, mSwapchainFramebuffers);
	}

	void vkVulkanClear::FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage)
	{
		EASY_FUNCTION();

		const VkClearValue clearValues[2] =
		{
			VkClearValue { .color = { .2f, .5f, .7f, 1.0f } },
			VkClearValue { .depthStencil = { 1.0f, 0 } }
		};

		const VkRect2D screenRect =
		{
			.offset = { 0, 0 },
			.extent = { .width = mFramebufferWidth, .height = mFramebufferHeight }
		};

		const VkRenderPassBeginInfo renderPassInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = mRenderPass,
			.framebuffer = mSwapchainFramebuffers[currentImage],
			.renderArea = screenRect,
			.clearValueCount = static_cast<uint32_t>(mShouldClearDepth ? 2 : 1),
			.pClearValues = &clearValues[0]
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(commandBuffer);
	}

}