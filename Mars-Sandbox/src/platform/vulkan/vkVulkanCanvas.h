#pragma once

#include "platform/vulkan/vkRenderBase.h"

namespace ms
{
	class vkVulkanCanvas : public vkRenderBase
	{
	public:
		vkVulkanCanvas(VulkanRenderDevice& renderDevice, VulkanImage depthTexture);
		virtual ~vkVulkanCanvas();

		// Remove Copy Constructor and Copy Assignment Operator
		vkVulkanCanvas(const vkVulkanCanvas&) = delete;
		vkVulkanCanvas& operator=(const vkVulkanCanvas&) = delete;
		// Remove Move Constructor and Move Assignment Operator
		vkVulkanCanvas(vkVulkanCanvas&&) = delete;
		vkVulkanCanvas& operator=(vkVulkanCanvas&&) = delete;

	public:
		virtual void FillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override;

		void UpdateLinesBuffer(const VulkanRenderDevice& renderDevice, uint32_t currentImage);
		// Called every frame to update the Uniform Buffer
		void UpdateUniformBuffer(const VulkanRenderDevice& renderDevice, uint32_t currentImage, const glm::mat4& m, float time);

		void Clear();
		void Line(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color);

	private:
		bool CreateDescriptorSet(const VulkanRenderDevice& renderDevice);

		struct VertexData
		{
			glm::vec3 position;
			glm::vec4 color;
		};

		struct UniformBuffer
		{
			glm::mat4 mvp;
			float time;
		};

		std::vector<VertexData> mLines;
		std::vector<VkBuffer> mLinesStorageBuffer;
		std::vector<VkDeviceMemory> mLinesStorageBufferMemory;

		static constexpr unsigned kMaxLinesCount = 65536;
		static constexpr unsigned kMaxLinesDataSize = 2 * kMaxLinesCount * sizeof(VertexData);
	};
}