#pragma once

#include "platform/common/msApp.h"
#include "platform/vulkan/msWindowVulkan.h"

#include "utils/UtilsVulkan.h"

namespace ms
{
	class MsAppVulkan : public MsApp
	{
	public:
		MsAppVulkan();

		void Run() override;

	private:
		void Initialize() override;

		MsWindowVulkan mWindow{ WIDTH, HEIGHT, "Vulkan App" };
		VkInstance mVkInstance;
	};
}