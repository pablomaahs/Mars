#include <mspch.h>

#include "vkBufferMgr.h"
#include "UtilsVulkan.h"

namespace ms
{
    bool vkBufferMgr::CreateBuffer(const VkDevice* device, const VkPhysicalDevice* physicalDevice, VkDeviceSize size, VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags properties)
    {
        const VkBufferCreateInfo bufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VK_CHECK(vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(*device, buffer, &memoryRequirements);

        const VkMemoryAllocateInfo allocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = ms::vkBufferMgr::FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties)
        };

        VK_CHECK(vkAllocateMemory(*device, &allocateInfo, nullptr, &bufferMemory));
        VK_CHECK_RET(vkBindBufferMemory(*device, buffer, bufferMemory, 0));
    }

    void vkBufferMgr::DestroyBuffer(const VkDevice* device, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        vkDestroyBuffer(*device, buffer, nullptr);
        vkFreeMemory(*device, bufferMemory, nullptr);
    }

    bool vkBufferMgr::CreateImage(const VkDevice* device, const VkPhysicalDevice* physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImage& image, VkImageUsageFlags usage, VkDeviceMemory& imageMemory, VkMemoryPropertyFlags properties)
    {
        VkImageFormatProperties props;
        VK_CHECK(vkGetPhysicalDeviceImageFormatProperties(*physicalDevice, format, VK_IMAGE_TYPE_2D, tiling, usage, 0, &props));

        const VkImageCreateInfo imageCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = VkExtent3D {
                .width = width, .height = height, .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VK_CHECK(vkCreateImage(*device, &imageCreateInfo, nullptr, &image));
        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(*device, image, &memoryRequirements);

        const VkMemoryAllocateInfo memoryAllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = ms::vkBufferMgr::FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties)
        };

        VK_CHECK(vkAllocateMemory(*device, &memoryAllocateInfo, nullptr, &imageMemory));
        VK_CHECK_RET(vkBindImageMemory(*device, image, imageMemory, 0));
    }

    uint32_t vkBufferMgr::FindMemoryType(const VkPhysicalDevice* device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(*device, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        return 0xFFFFFFFF;
    }
}