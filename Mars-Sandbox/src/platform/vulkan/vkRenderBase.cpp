#include "mspch.h"

#include "vkRenderBase.h"

#include "platform/vulkan/utils/vkBufferMgr.h"

namespace ms
{
	vkRenderBase::~vkRenderBase()
	{
		for (VkBuffer buffer : mUniformBuffers)
		{
			vkDestroyBuffer(mDevice, buffer, nullptr);
		}

		for (VkDeviceMemory bufferMemory : mUniformBuffersMemory)
		{
			vkFreeMemory(mDevice, bufferMemory, nullptr);
		}

		vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

		for (VkFramebuffer framebuffer : mSwapchainFramebuffers)
		{
			vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
		}

		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
		vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	}

	void vkRenderBase::BeginRenderPass(VkCommandBuffer commandBuffer, size_t currentImage)
	{
		const VkRect2D screenRect =
		{
			.offset = { 0, 0 },
			.extent = {.width = mFramebufferWidth, .height = mFramebufferHeight }
		};

		const VkRenderPassBeginInfo renderPassInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = mRenderPass,
			.framebuffer = mSwapchainFramebuffers[currentImage],
			.renderArea = screenRect,
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[currentImage], 0, nullptr);
	}

	bool vkRenderBase::CreateUniformBuffers(const VulkanRenderDevice& vulkanRenderDevice, size_t uniformDataSize)
	{
		mUniformBuffers.resize(vulkanRenderDevice.swapchainImages.size());
		mUniformBuffersMemory.resize(vulkanRenderDevice.swapchainImages.size());

		for (size_t i = 0; i < vulkanRenderDevice.swapchainImages.size(); i++)
		{
			if (!vkBufferMgr::CreateBuffer(
				static_cast<const VkDevice*>(&vulkanRenderDevice.device), static_cast<const VkPhysicalDevice*>(&vulkanRenderDevice.physicalDevice), uniformDataSize,
				mUniformBuffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				mUniformBuffersMemory[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			))
			{
				ASSERT(false);
				return false;
			}
		}

		return true;
	}
}