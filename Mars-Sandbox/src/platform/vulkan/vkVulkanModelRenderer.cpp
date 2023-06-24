#include "mspch.h"

#include "platform/vulkan/vkVulkanModelRenderer.h"

#include "platform/vulkan/utils/UtilsVulkan.h"
#include "platform/vulkan/utils/vkCommandBufferMgr.h"

namespace ms
{
	vkVulkanModelRenderer::vkVulkanModelRenderer
		(const VulkanRenderDevice& renderDevice, std::string modelFile, std::string textureFile, uint32_t uniformDataSize)
		: vkRenderBase(renderDevice, VulkanImage()),
		mUniformDataSize(uniformDataSize), mModelFile(modelFile), mTextureFile(textureFile)
	{
		ASSERT(CreateTexturedVertexBuffer(renderDevice, mModelFile.c_str(), &mStorageBuffer, &mStorageBufferMemory, &mVertexBufferSize, &mIndexBufferSize));

		ASSERT(CreateTextureImage(renderDevice, mTextureFile.c_str(), mTexture.image, mTexture.imageMemory));
		ASSERT(CreateImageView(mDevice, mTexture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &mTexture.imageView));
		ASSERT(CreateTextureSampler(mDevice, &mTextureSampler));

		ASSERT(CreateDepthResources(renderDevice, mFramebufferWidth, mFramebufferHeight, mDepthTexture));
		CreateColorAndDepthRenderPass(renderDevice, true, &mRenderPass, RenderPassCreateInfo());
		ASSERT(CreateUniformBuffers(renderDevice, mUniformDataSize));
		CreateColorAndDepthFramebuffers(renderDevice, mRenderPass, mDepthTexture.imageView, mSwapchainFramebuffers);
		CreateDescriptorPool(mDevice, renderDevice.swapchainImages.size(), 1, 2, 1, &mDescriptorPool);
        CreateDescriptorSet(renderDevice, uniformDataSize);
		CreatePipelineLayout(renderDevice, mDescriptorSetLayout, &mPipelineLayout);

		const std::vector<std::string>& shaderFiles =
		{
			"./rsc/shaders/vkPVPDefault.vert",
			"./rsc/shaders/vkPVPDefault.frag",
			"./rsc/shaders/vkPVPDefault.geom"
		};
		CreateGraphicsPipeline(renderDevice, mRenderPass, mPipelineLayout, &mGraphicsPipeline, shaderFiles);
	}

	vkVulkanModelRenderer::~vkVulkanModelRenderer()
	{
		vkDestroyBuffer(mDevice, mStorageBuffer, nullptr);
		vkFreeMemory(mDevice, mStorageBufferMemory, nullptr);

		vkDestroySampler(mDevice, mTextureSampler, nullptr);

		vkDestroyImage(mDevice, mTexture.image, nullptr);
		vkDestroyImageView(mDevice, mTexture.imageView, nullptr);
		vkFreeMemory(mDevice, mTexture.imageMemory, nullptr);

        vkDestroyImage(mDevice, mDepthTexture.image, nullptr);
        vkDestroyImageView(mDevice, mDepthTexture.imageView, nullptr);
        vkFreeMemory(mDevice, mDepthTexture.imageMemory, nullptr);
	}

	void vkVulkanModelRenderer::FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage)
	{
		EASY_FUNCTION();

		BeginRenderPass(commandBuffer, currentImage);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(mIndexBufferSize / (sizeof(unsigned int))), 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
	}

	void vkVulkanModelRenderer::UpdateUniformBuffer(const VulkanRenderDevice& renderDevice, uint32_t currentImage, const void* data, size_t dataSize)
	{
        vkCommandBufferMgr::UploadBufferDataCopy bufferCopy{ .deviceOffset = 0, .data = data, .dataSize = dataSize };

        vkCommandBufferMgr::UploadBufferData(static_cast<const VkDevice*>(&renderDevice.device), &mUniformBuffersMemory[currentImage], bufferCopy);
	}

	bool vkVulkanModelRenderer::CreateDescriptorSet(const VulkanRenderDevice& renderDevice, uint32_t uniformDataSize)
	{
        const std::vector<VkDescriptorSetLayoutBinding> bindings =
        {
            VkDescriptorSetLayoutBinding
            {
                .binding = static_cast<uint32_t>(0),
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding
            {
                .binding = static_cast<uint32_t>(1),
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding
            {
                .binding = static_cast<uint32_t>(2),
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding
            {
                .binding = static_cast<uint32_t>(3),
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr
            }
        };

        const VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };

        VK_CHECK(vkCreateDescriptorSetLayout(renderDevice.device, &descSetLayoutCreateInfo, nullptr, &mDescriptorSetLayout));

        std::vector<VkDescriptorSetLayout> layouts =
        {
            renderDevice.swapchainImages.size(),
            mDescriptorSetLayout
        };

        VkDescriptorSetAllocateInfo descSetAllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = mDescriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(renderDevice.swapchainImages.size()),
            .pSetLayouts = layouts.data()
        };

        mDescriptorSets.resize(renderDevice.swapchainImages.size());
        VK_CHECK(vkAllocateDescriptorSets(renderDevice.device, &descSetAllocateInfo, mDescriptorSets.data()));

        for (size_t i = 0; i < renderDevice.swapchainImages.size(); i++)
        {
            const VkDescriptorBufferInfo bufferInfo =
            {
                .buffer = mUniformBuffers[i],
                .offset = 0,
                .range = uniformDataSize
            };

            const VkDescriptorBufferInfo bufferInfo2 =
            {
                .buffer = mStorageBuffer,
                .offset = 0,
                .range = mVertexBufferSize
            };

            const VkDescriptorBufferInfo bufferInfo3 =
            {
                .buffer = mStorageBuffer,
                .offset = mVertexBufferSize,
                .range = mIndexBufferSize
            };

            const VkDescriptorImageInfo imageInfo =
            {
                .sampler = mTextureSampler,
                .imageView = mTexture.imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            std::vector<VkWriteDescriptorSet> descriptorWrites =
            {
                VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = mDescriptorSets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &bufferInfo
                },
                VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = mDescriptorSets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .pBufferInfo = &bufferInfo2
                },
                VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = mDescriptorSets[i],
                    .dstBinding = 2,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .pBufferInfo = &bufferInfo3
                },
                VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = mDescriptorSets[i],
                    .dstBinding = 3,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = &imageInfo
                }
            };

            vkUpdateDescriptorSets(renderDevice.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        return true;
	}
}