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
		MsAppVulkan(unsigned int w, unsigned int h, std::string name);

		void Run() override;

	private:
		void Initialize() override;

		MsWindowVulkan		mWindow;
		VkInstance mVkInstance;
	};
}