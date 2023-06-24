#include <mspch.h>

#include "vkCommandBufferMgr.h"
#include "UtilsVulkan.h"

namespace ms
{
	bool vkCommandBufferMgr::AllocateCommandBuffer(const VkDevice* device, const VkCommandPool commandPool, VkCommandBuffer* commandBuffer, const VkCommandBufferAllocateInfo* commandBufferAllocateInfo)
	{
        if (commandBufferAllocateInfo)
        {
            VK_CHECK_RET(vkAllocateCommandBuffers(*device, commandBufferAllocateInfo, commandBuffer));
        }

        const VkCommandBufferAllocateInfo allocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VK_CHECK_RET(vkAllocateCommandBuffers(*device, &allocateInfo, commandBuffer));
	}

    void vkCommandBufferMgr::BeginCommandBuffer(VkCommandBuffer* commandBuffer, VkCommandBufferBeginInfo* commandBufferBeginInfo)
    {
        if (commandBufferBeginInfo)
        {
            VK_CHECK(vkBeginCommandBuffer(*commandBuffer, commandBufferBeginInfo));
        }

        const VkCommandBufferInheritanceInfo commandBufferInheritanceInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO
        };

        const VkCommandBufferBeginInfo beginInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = &commandBufferInheritanceInfo
        };

        VK_CHECK(vkBeginCommandBuffer(*commandBuffer, &beginInfo));
    }

    void vkCommandBufferMgr::EndCommandBuffer(VkCommandBuffer* commandBuffer)
    {
        VK_CHECK(vkEndCommandBuffer(*commandBuffer));
    }

    bool vkCommandBufferMgr::SubmitCommandBuffer(const VkQueue* queue, VkCommandBuffer* commandBuffer, const VkSubmitInfo* submitInfo, const VkFence* fence)
    {
        if (submitInfo)
        {
            VK_CHECK(vkQueueSubmit(*queue, 1, submitInfo, VK_NULL_HANDLE));
            VK_CHECK_RET(vkQueueWaitIdle(*queue));
        }

        const VkSubmitInfo tmpSubmitInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = (uint32_t) (sizeof(commandBuffer) / sizeof(VkCommandBuffer)),
            .pCommandBuffers = commandBuffer,

        };

        VK_CHECK(vkQueueSubmit(*queue, 1, &tmpSubmitInfo, VK_NULL_HANDLE));
        VK_CHECK_RET(vkQueueWaitIdle(*queue));
    }
    
    void vkCommandBufferMgr::FreeCommandBuffer(const VkDevice* device, const VkCommandPool commandPool, VkCommandBuffer* commandBuffer)
    {
        vkFreeCommandBuffers(*device, commandPool, 1, commandBuffer);
    }
    
    void vkCommandBufferMgr::CmdCopyBuffer(const VkDevice* device, const VkCommandPool commandPool, const VkQueue* queue, VkBuffer* srcBuffer, VkBuffer* dstBuffer, const VkBufferCopy* bufferCopyParams)
    {
        VkCommandBuffer commandBuffer;

        vkCommandBufferMgr::AllocateCommandBuffer(device, commandPool, &commandBuffer);
        vkCommandBufferMgr::BeginCommandBuffer(&commandBuffer);

        vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, bufferCopyParams);

        vkCommandBufferMgr::EndCommandBuffer(&commandBuffer);
        vkCommandBufferMgr::SubmitCommandBuffer(queue, &commandBuffer);
        vkCommandBufferMgr::FreeCommandBuffer(device, commandPool, &commandBuffer);
    }

    void vkCommandBufferMgr::CmdCopyBufferToImage(const VkDevice* device, const VkCommandPool commandPool, const VkQueue* queue, VkBuffer* srcBuffer, VkImage* dstImage, const VkBufferImageCopy* bufferImageCopyParams)
    {
        VkCommandBuffer commandBuffer;

        vkCommandBufferMgr::AllocateCommandBuffer(device, commandPool, &commandBuffer);
        vkCommandBufferMgr::BeginCommandBuffer(&commandBuffer);

        vkCmdCopyBufferToImage(commandBuffer, *srcBuffer, *dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, bufferImageCopyParams);

        vkCommandBufferMgr::EndCommandBuffer(&commandBuffer);
        vkCommandBufferMgr::SubmitCommandBuffer(queue, &commandBuffer);
        vkCommandBufferMgr::FreeCommandBuffer(device, commandPool, &commandBuffer);
    }

    void vkCommandBufferMgr::CmdPipelineBarrier(const VkDevice* device, const VkCommandPool commandPool, const VkQueue* queue, VkPipelineStageFlags src, VkPipelineStageFlags dst, VkDependencyFlags flags, const VkMemoryBarrier* memoryBarrier, const VkBufferMemoryBarrier* bufferMemoryBarrier, const VkImageMemoryBarrier* imageMemoryBarrier)
    {
        VkCommandBuffer commandBuffer;

        ms::vkCommandBufferMgr::AllocateCommandBuffer(device, commandPool, &commandBuffer);
        ms::vkCommandBufferMgr::BeginCommandBuffer(&commandBuffer);

        uint32_t memoryBarrierCount = memoryBarrier ? 1 : 0;
        uint32_t bufferMemoryBarrierCount = bufferMemoryBarrier ? 1 : 0;
        uint32_t imageMemoryBarrierCount = imageMemoryBarrier ? 1 : 0;

        vkCmdPipelineBarrier(commandBuffer, src, dst, flags,
            memoryBarrierCount, memoryBarrier, bufferMemoryBarrierCount, bufferMemoryBarrier, imageMemoryBarrierCount, imageMemoryBarrier
        );

        ms::vkCommandBufferMgr::EndCommandBuffer(&commandBuffer);
        ms::vkCommandBufferMgr::SubmitCommandBuffer(queue, &commandBuffer);
        ms::vkCommandBufferMgr::FreeCommandBuffer(device, commandPool, &commandBuffer);
    }

    void vkCommandBufferMgr::UploadBufferData(const VkDevice* device, const VkDeviceMemory* bufferMemory, UploadBufferDataCopy& bufferDataCopy)
    {
        void* mapperData = nullptr;
        vkMapMemory(*device, *bufferMemory, bufferDataCopy.deviceOffset, bufferDataCopy.dataSize, 0, &mapperData);
        memcpy(mapperData, bufferDataCopy.data, bufferDataCopy.dataSize);
        vkUnmapMemory(*device, *bufferMemory);
    }

    void vkCommandBufferMgr::UploadBufferData(const VkDevice* device, const VkDeviceMemory* bufferMemory, const VkDeviceSize bufferSize, std::vector<UploadBufferDataCopy>& bufferDataCopyVec)
    {
        void* mapperData = nullptr;
        vkMapMemory(*device, *bufferMemory, 0, bufferSize, 0, &mapperData);

        for (size_t i = 0; i < bufferDataCopyVec.size(); i++)
        {
            UploadBufferDataCopy dataCopy = bufferDataCopyVec[i];
            memcpy((unsigned char*) mapperData + dataCopy.deviceOffset, dataCopy.data, dataCopy.dataSize);
        }

        vkUnmapMemory(*device, *bufferMemory);
    }
}