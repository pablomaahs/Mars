#pragma once

#include "platform/vulkan/utils/UtilsVulkan.h"

namespace ms
{
	class vkRenderBase
	{
	public:
		explicit vkRenderBase(const VulkanRenderDevice& vulkanRenderDevice, VulkanImage depthTexture) :
			mDevice(vulkanRenderDevice.device), mDepthTexture(depthTexture),
			mFramebufferHeight(vulkanRenderDevice.framebufferHeight), mFramebufferWidth(vulkanRenderDevice.framebufferWidth),
            mDescriptorSetLayout(nullptr), mDescriptorPool(nullptr), mGraphicsPipeline(nullptr), mPipelineLayout(nullptr), mRenderPass(nullptr)
		{}
		virtual ~vkRenderBase();

        // Remove Copy Constructor and Copy Assignment Operator
        vkRenderBase(const vkRenderBase&) = delete;
        vkRenderBase& operator=(const vkRenderBase&) = delete;
        // Remove Move Constructor and Move Assignment Operator
        vkRenderBase(vkRenderBase&&) = delete;
        vkRenderBase& operator=(vkRenderBase&&) = delete;

    public:
		virtual void FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) = 0;
		inline VulkanImage GetDepthTexture() const { return mDepthTexture; }

	protected:
        void BeginRenderPass(VkCommandBuffer commandBuffer, size_t currentImage);
        bool CreateUniformBuffers(const VulkanRenderDevice& vulkanRenderDevice, size_t uniformDataSize);

        // VkDevice - Represents an initialized Vulkan device that is ready to create all other objects.
		VkDevice mDevice = nullptr;

        // VkDescriptorSetLayout - Before creating a Descriptor Set, its layout must be specified by creating a DescriptorSetLayout.
        VkDescriptorSetLayout mDescriptorSetLayout;
        // VkDescriptorPool - Object used to allocate Descriptor Sets.
        VkDescriptorPool mDescriptorPool;
        // VkDescriptorSet - The way shaders can access resources (Buffers, Images and Samplers) is through Descriptors.
        // Descriptors don't exist on their own, but are grouped in Descriptor Sets.
        std::vector<VkDescriptorSet> mDescriptorSets;

        // VkFramebuffer - Represents a link to actual Images that can be use as attachments.
        // Attachment is the Vulkan's name for Render Target. An Image to be use as output from rendering.
        std::vector<VkFramebuffer> mSwapchainFramebuffers;

        // VkPipelineLayout - Represents a configuration of the rendering pipeline in terms of what types of Descriptor Sets will be bound to the Command Buffer.
        VkPipelineLayout mPipelineLayout;
        // VkPipeline - Is a big object, as it composes most of the objects. It represents the configuration of the whole pipeline and has a lot of parameters.
        VkPipeline mGraphicsPipeline;
        // VkRenderPass - In Vulkan you need to plan the rendering of your frame in advance and organize it into Passes and Subpasses.
        VkRenderPass mRenderPass;

        // TODO Uniform buffer
        std::vector<VkBuffer> mUniformBuffers;
        std::vector<VkDeviceMemory> mUniformBuffersMemory;

        // TODO Depth buffer
        VulkanImage mDepthTexture;

		uint32_t mFramebufferWidth = 0;
		uint32_t mFramebufferHeight = 0;
	};
}