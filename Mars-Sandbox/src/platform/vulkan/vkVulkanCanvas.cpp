#include <mspch.h>

#include "platform/vulkan/vkVulkanCanvas.h"

#include "platform/vulkan/utils/vkCommandBufferMgr.h"
#include "platform/vulkan/utils/vkBufferMgr.h"

namespace ms
{
	vkVulkanCanvas::vkVulkanCanvas(VulkanRenderDevice& renderDevice, VulkanImage depthTexture)
		: vkRenderBase(renderDevice, depthTexture)
	{
		const size_t imgCount = renderDevice.swapchainImages.size();

		mLinesStorageBuffer.resize(imgCount);
		mLinesStorageBufferMemory.resize(imgCount);

		for (size_t i = 0; i < imgCount; i++)
		{
			ASSERT(
				vkBufferMgr::CreateBuffer(
					static_cast<const VkDevice*>(&renderDevice.device), static_cast<const VkPhysicalDevice*>(&renderDevice.physicalDevice), kMaxLinesDataSize,
					mLinesStorageBuffer[i], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
					mLinesStorageBufferMemory[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				)
			);
		}

		ASSERT(CreateColorAndDepthRenderPass(renderDevice, true, &mRenderPass, RenderPassCreateInfo()));
		ASSERT(CreateUniformBuffers(renderDevice, sizeof(UniformBuffer)));
		ASSERT(CreateColorAndDepthFramebuffers(renderDevice, mRenderPass, mDepthTexture.imageView, mSwapchainFramebuffers));
		ASSERT(CreateDescriptorPool(mDevice, imgCount, 1, 2, 1, &mDescriptorPool));
		ASSERT(CreateDescriptorSet(renderDevice));
		ASSERT(CreatePipelineLayout(renderDevice, mDescriptorSetLayout, &mPipelineLayout));

		const std::vector<std::string>& shaderFiles =
		{
			"./rsc/shaders/vkPVPCanvas.vert",
			"./rsc/shaders/vkPVPCanvas.frag"
		};
		ASSERT(CreateGraphicsPipeline(renderDevice, mRenderPass, mPipelineLayout, &mGraphicsPipeline, shaderFiles,
			VK_PRIMITIVE_TOPOLOGY_LINE_LIST, true, true, false, -1, -1, 0U, false));
	}

	vkVulkanCanvas::~vkVulkanCanvas()
	{
		for (size_t i = 0; i < mSwapchainFramebuffers.size(); i++)
		{
			vkDestroyBuffer(mDevice, mLinesStorageBuffer[i], nullptr);
			vkFreeMemory(mDevice, mLinesStorageBufferMemory[i], nullptr);
		}
	}

	void vkVulkanCanvas::FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage)
	{
		if (mLines.empty())
			return;

		BeginRenderPass(commandBuffer, currentImage);
		vkCmdDraw(commandBuffer, mLines.size(), 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
	}

	void vkVulkanCanvas::UpdateLinesBuffer(const VulkanRenderDevice& renderDevice, uint32_t currentImage)
	{
		if (mLines.empty())
			return;

		VkDeviceSize bufferSize = mLines.size() * sizeof(VertexData);
		vkCommandBufferMgr::UploadBufferDataCopy bufferCopy{ .deviceOffset = 0, .data = mLines.data(), .dataSize = bufferSize };

		vkCommandBufferMgr::UploadBufferData(static_cast<const VkDevice*>(&renderDevice.device), &mLinesStorageBufferMemory[currentImage], bufferCopy);
	}

	void vkVulkanCanvas::UpdateUniformBuffer(const VulkanRenderDevice& renderDevice, uint32_t currentImage, const glm::mat4& mp, float time)
	{
		const UniformBuffer ubo = { .mvp = mp, .time = time };
		vkCommandBufferMgr::UploadBufferDataCopy bufferCopy{ .deviceOffset = 0, .data = &ubo, .dataSize = sizeof(UniformBuffer) };

		vkCommandBufferMgr::UploadBufferData(static_cast<const VkDevice*>(&renderDevice.device), &mUniformBuffersMemory[currentImage], bufferCopy);
	}

	bool vkVulkanCanvas::CreateDescriptorSet(const VulkanRenderDevice& renderDevice)
	{
		const size_t imgCount = renderDevice.swapchainImages.size();

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
			imgCount,
			mDescriptorSetLayout
		};

		VkDescriptorSetAllocateInfo descSetAllocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = mDescriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(imgCount),
			.pSetLayouts = layouts.data()
		};

		mDescriptorSets.resize(imgCount);
		VK_CHECK(vkAllocateDescriptorSets(renderDevice.device, &descSetAllocateInfo, mDescriptorSets.data()));

		for (size_t i = 0; i < imgCount; i++)
		{
			const VkDescriptorBufferInfo bufferInfo =
			{
				.buffer = mUniformBuffers[i],
				.offset = 0,
				.range = sizeof(UniformBuffer)
			};

			const VkDescriptorBufferInfo bufferInfo2 =
			{
				.buffer = mLinesStorageBuffer[i],
				.offset = 0,
				.range = kMaxLinesDataSize
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
			};

			vkUpdateDescriptorSets(renderDevice.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		return true;
	}

	void vkVulkanCanvas::Clear()
	{
		mLines.clear();
	}

	void vkVulkanCanvas::Line(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color)
	{
		mLines.push_back({ .position = p1, .color = color });
		mLines.push_back({ .position = p2, .color = color });
	}
}