#pragma once

#include "platform/vulkan/utils/UtilsVulkan.h"

#include "platform/vulkan/vkRenderBase.h"

namespace ms
{
	/* The VulkanClear object initializes and starts an empty rendering pass whose only purpose is to clear the color and depth buffers. */
	class vkVulkanClear : public vkRenderBase
	{
	public:
		vkVulkanClear(VulkanRenderDevice& vkDev, VulkanImage depthTexture);

		// Remove Copy Constructor and Copy Assignment Operator
		vkVulkanClear(const vkVulkanClear&) = delete;
		vkVulkanClear& operator=(const vkVulkanClear&) = delete;
		// Remove Move Constructor and Move Assignment Operator
		vkVulkanClear(vkVulkanClear&&) = delete;
		vkVulkanClear& operator=(vkVulkanClear&&) = delete;

	public:
		virtual void FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override;

	private:
		bool mShouldClearDepth;
	};
}