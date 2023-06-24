#pragma once

#include "volk.h"

namespace ms
{
	class vkBufferMgr
	{
	public:
		static bool CreateBuffer(
			const VkDevice* device, const VkPhysicalDevice* physicalDevice,
			VkDeviceSize size,
			VkBuffer&		buffer,			VkBufferUsageFlags		usage,
			VkDeviceMemory& bufferMemory,	VkMemoryPropertyFlags	properties
		);

		static void DestroyBuffer(const VkDevice* device, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		static bool CreateImage(
			const VkDevice* device, const VkPhysicalDevice* physicalDevice,
			uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImage&		image,			VkImageUsageFlags		usage,
			VkDeviceMemory& imageMemory,	VkMemoryPropertyFlags	properties
		);

	private:
		static uint32_t FindMemoryType(const VkPhysicalDevice* device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
}