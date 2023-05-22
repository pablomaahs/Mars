#pragma once

#include "platform/vulkan/vkRenderBase.h"

namespace ms
{
	/* VulkanFinish object helps us to create another empty rendering pass that transitions the swapchain image to the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format.
	   This class essentially hides all the hassle necessary to finalize frame rendering in Vulkan.
	*/
	class vkVulkanFinish : public vkRenderBase
	{
	public:
		vkVulkanFinish(VulkanRenderDevice& renderDevice, VulkanImage depthTexture);

		// Remove Copy Constructor and Copy Assignment Operator
		vkVulkanFinish(const vkVulkanFinish&) = delete;
		vkVulkanFinish& operator=(const vkVulkanFinish&) = delete;
		// Remove Move Constructor and Move Assignment Operator
		vkVulkanFinish(vkVulkanFinish&&) = delete;
		vkVulkanFinish& operator=(vkVulkanFinish&&) = delete;

	public:
		virtual void FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override;

	private:
		bool mShouldClearDepth;
	};
}