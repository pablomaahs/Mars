#include "volk.h"

namespace ms
{
	const std::vector<const char*> VK_LAYERS =
	{
		 "VK_LAYER_KHRONOS_validation"
		//,"VK_LAYER_LUNARG_api_dump"
		//,"VK_LAYER_RENDERDOC_Capture"
	};

	const std::vector<const char*> VK_EXTENSIONS =
	{
		VK_KHR_SURFACE_EXTENSION_NAME
	   ,VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	   ,VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	   ,VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME // for indexed textures
#if defined (_WIN32)
	   ,"VK_KHR_win32_surface"
#endif // WIN32
	};

	class vkInstance
	{
	public:
		explicit vkInstance();
		virtual ~vkInstance();

		// Remove Copy Constructor and Copy Assignment Operator
		vkInstance(const vkInstance&) = delete;
		vkInstance& operator=(const vkInstance&) = delete;
		// Remove Move Constructor and Move Assignment Operator
		vkInstance(vkInstance&&) = delete;
		vkInstance& operator=(vkInstance&&) = delete;

	public:
		inline VkInstance GetInstance() { return mVulkanInstance; };
		inline VkSurfaceKHR* GetSurfaceKHR() { return &mSurfaceKHR; };

	private:
		bool AreLayersSupported();
		void SetupDebugCallbaks();

		VkDebugUtilsMessengerEXT mMessengerEXT = VK_NULL_HANDLE;
		static VKAPI_ATTR VkBool32 VKAPI_CALL _VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

		VkDebugReportCallbackEXT mReportCallbackEXT = VK_NULL_HANDLE;
		static VKAPI_ATTR VkBool32 VKAPI_CALL _VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* userData);

		VkInstance mVulkanInstance = VK_NULL_HANDLE;
		VkSurfaceKHR mSurfaceKHR = VK_NULL_HANDLE;
	};
};