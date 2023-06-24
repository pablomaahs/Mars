#pragma once

#include "volk.h"

namespace ms
{
	class vkCommandBufferMgr
	{
	public:
		// Allocate memory for a CommandBuffer from the CommandPool
		static bool AllocateCommandBuffer(const VkDevice* device, const VkCommandPool commandPool, VkCommandBuffer* commandBuffer, const VkCommandBufferAllocateInfo* commandBufferAllocateInfo = VK_NULL_HANDLE);
		// Start the CommandBuffer recording
		static void BeginCommandBuffer(VkCommandBuffer* commandBuffer, VkCommandBufferBeginInfo* commandBufferBeginInfo = VK_NULL_HANDLE);
		// End the CommandBuffer recording
		static void EndCommandBuffer(VkCommandBuffer* commandBuffer);
		// Submit the command buffer for execution
		static bool SubmitCommandBuffer(const VkQueue* queue, VkCommandBuffer* commandBuffer, const VkSubmitInfo* submitInfo = VK_NULL_HANDLE, const VkFence* fence = VK_NULL_HANDLE);
		// Free CommandBuffer
		static void FreeCommandBuffer(const VkDevice* device, const VkCommandPool commandPool, VkCommandBuffer* commandBuffer);

	public:
		static void CmdCopyBuffer(const VkDevice* device, const VkCommandPool commandPool, const VkQueue* queue, VkBuffer* srcBuffer, VkBuffer* dstBuffer, const VkBufferCopy* bufferCopyParams);

		static void CmdCopyBufferToImage(const VkDevice* device, const VkCommandPool commandPool, const VkQueue* queue, VkBuffer* srcBuffer, VkImage* dstImage, const VkBufferImageCopy* bufferImageCopyParams);

		static void CmdPipelineBarrier(
			const VkDevice* device, const VkCommandPool commandPool, const VkQueue* queue,
			VkPipelineStageFlags src, VkPipelineStageFlags dst, VkDependencyFlags flags = 0,
			const VkMemoryBarrier* memoryBarrier = nullptr,
			const VkBufferMemoryBarrier* bufferMemoryBarrier = nullptr,
			const VkImageMemoryBarrier* imageMemoryBarrier = nullptr
		);

		struct UploadBufferDataCopy
		{
			VkDeviceSize deviceOffset;
			const void*  data;
			const size_t dataSize;
		};

		static void UploadBufferData(const VkDevice* device, const VkDeviceMemory* bufferMemory, UploadBufferDataCopy& bufferDataCopy);

		static void UploadBufferData(const VkDevice* device, const VkDeviceMemory* bufferMemory, const VkDeviceSize bufferSize, std::vector<UploadBufferDataCopy>& bufferDataCopyVec);
	};
}