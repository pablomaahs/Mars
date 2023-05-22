#pragma once

#include "platform/vulkan/vkRenderBase.h"

namespace ms
{
	class vkVulkanModelRenderer : public vkRenderBase
	{
	public:
		vkVulkanModelRenderer(const VulkanRenderDevice& renderDevice, std::string modelFile, std::string textureFile, uint32_t uniformDataSize);
		virtual ~vkVulkanModelRenderer();

		// Remove Copy Constructor and Copy Assignment Operator
		vkVulkanModelRenderer(const vkVulkanModelRenderer&) = delete;
		vkVulkanModelRenderer& operator=(const vkVulkanModelRenderer&) = delete;
		// Remove Move Constructor and Move Assignment Operator
		vkVulkanModelRenderer(vkVulkanModelRenderer&&) = delete;
		vkVulkanModelRenderer& operator=(vkVulkanModelRenderer&&) = delete;

	public:
		virtual void FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override;
		// Called every frame to update the Uniform Buffer
		void UpdateUniformBuffer(const VulkanRenderDevice& renderDevice, uint32_t currentImage, const void* data, size_t dataSize);

	private:
		bool CreateDescriptorSet(const VulkanRenderDevice& renderDevice, uint32_t uniformDataSize);

		size_t mVertexBufferSize = 0;
		size_t mIndexBufferSize = 0;
		// Index and Vertex Data stored on the same Buffer
		VkBuffer mStorageBuffer;
		VkDeviceMemory mStorageBufferMemory;

		// Texture for the Model, only one for each model
		VkSampler mTextureSampler;
		VulkanImage mTexture;

		// Model State, mostly for Debug
		std::string mModelFile = "";
		std::string mTextureFile = "";
		uint32_t mUniformDataSize = 0;
	};
}