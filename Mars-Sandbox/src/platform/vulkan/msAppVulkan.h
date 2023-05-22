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

		// Remove Copy Constructor and Copy Assignment Operator
		MsAppVulkan(const MsAppVulkan&) = delete;
		MsAppVulkan& operator=(const MsAppVulkan&) = delete;
		// Remove Move Constructor and Move Assignment Operator
		MsAppVulkan(MsAppVulkan&&) = delete;
		MsAppVulkan& operator=(MsAppVulkan&&) = delete;

		virtual ~MsAppVulkan();

		void Run() override;

	private:
		void Initialize() override;
		void Destroy() override;

		MsWindowVulkan		mWindow;
		VulkanInstance		mVulkanInstance;
		VulkanRenderDevice	mVulkanRenderDevice;
		VulkanState			mVulkanState;
	};
}