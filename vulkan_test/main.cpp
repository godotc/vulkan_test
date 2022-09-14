
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
//#include <vulkan/vulkan.hpp>

#define GLM_FORECE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ON
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>


#include<set>
#include<string>
#include <iostream>
#include <vector>
#include <stdio.h>
#include<cstdlib>
#include<stdexcept>
#include <functional>
#include<optional>


const int WIDTH = 800;
const int HEIGTH = 600;


const std::vector<const char*> gb_validationLayers = {
	"VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> gb_deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME, // "VK_KHR_swapchain"
};

#ifdef NDBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDBUG


VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (nullptr != func) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestoryDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT debugerMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (nullptr != func) {
		func(instance, debugerMessenger, pAllocator);
	}
}

VkResult CreateDebugReportCallbackEXT(
	VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestoryDebugReportCallbackEXT(
	VkInstance instance, VkDebugReportCallbackEXT reporterCallback, const VkAllocationCallbacks* pAllocator)
{
	auto func =
		(PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (nullptr != func) {
		func(instance, reporterCallback, pAllocator);
	}
}


struct QueueFamilyIndices
{
	int graphicsFamily{ -1 };
	int presentFamily{ -1 };

	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


class HelloTriangle
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:

	GLFWwindow* p_window;
	VkInstance m_instance;

	VkDebugUtilsMessengerEXT m_debugMessengerCallback;
	VkDebugReportCallbackEXT m_debugReportCallback;  // depercate

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_logicalDevice;

	VkQueue m_presentQueue;
	VkQueue m_graphicsQueue;

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	std::vector<VkImageView> m_swapChainImageViews;

private:

	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		p_window = glfwCreateWindow(WIDTH, HEIGTH, "Vulkan window", nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		//setupDebugCallback();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(p_window))
		{
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
		{
			vkDestroyImageView(m_logicalDevice, m_swapChainImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
		if (enableValidationLayers)
		{
			DestoryDebugUtilsMessengerEXT(m_instance, m_debugMessengerCallback, nullptr);
		}
		// DestoryDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);
		vkDestroyDevice(m_logicalDevice, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
		glfwDestroyWindow(p_window);
		glfwTerminate();
	}

private:

	void createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		// 配置 ApplicaitonInfo 结构
		VkApplicationInfo appinfo{};
		{
			appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appinfo.pNext = nullptr;
			appinfo.pApplicationName = "Hello Triangle";
			appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appinfo.pEngineName = "No Engine";
			appinfo.apiVersion = VK_API_VERSION_1_0;
		}

		std::vector<const char*> extensions;

		// 配置 Creatinfo, 包含 ApplicationInfo
		VkInstanceCreateInfo createInfo = {};
		{
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appinfo;

			// 获取 extesnsions 数量，需求，每个extesnsion的名字
			extensions = getRequiredExtensions();

			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			// 开启全局layer
			createInfo.enabledLayerCount = 0;
		}

		// 配置 debugMessenger
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		{
			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(gb_validationLayers.size());
				createInfo.ppEnabledLayerNames = gb_validationLayers.data();

				populateDebugMessengerCreateInfo(debugCreateInfo);

				// Assign to cretinfo
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			}
			else {
				createInfo.enabledLayerCount = 0;
				createInfo.pNext = nullptr;
			}
		}

		// 创建  instance
		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void setupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessengerCallback) != VK_SUCCESS)
		{
			throw::std::runtime_error("failed to set up debug messenger!");
		}
	}

	void createSurface()
	{
		if (glfwCreateWindowSurface(m_instance, p_window, nullptr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}

	}

	void createLogicalDevice()
	{

		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

		// 队列
		float queuePriority;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily,indices.presentFamily };
		{
			queuePriority = 1.0f;
			for (int queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}
		}

		// 需要的特性
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// 配置 device createInfo  
		VkDeviceCreateInfo deviceCreateInfo = {};
		{
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			// Queue
			deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t> (queueCreateInfos.size());
			deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();


			// Features
			deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

			// Layer
			deviceCreateInfo.enabledLayerCount = 0;
			if (enableValidationLayers)
			{
				deviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (gb_validationLayers.size());
				deviceCreateInfo.ppEnabledLayerNames = gb_validationLayers.data();
			}
			else {
				deviceCreateInfo.enabledLayerCount = 0;
			}

			// Extension
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(gb_deviceExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = gb_deviceExtensions.data();

		}

		// 创建 device
		if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}


		// 获取 present 队列
		vkGetDeviceQueue(m_logicalDevice, indices.presentFamily, 0, &m_presentQueue);

		// 获取 device 队列
		vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily, 0, &m_graphicsQueue);

	}

	void createSwapChain()
	{
		// Swap chain support details
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;


		// Image count
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Create info
		VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
		{
			swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapChainCreateInfo.surface = m_surface;

			swapChainCreateInfo.minImageCount = imageCount;
			swapChainCreateInfo.imageFormat = surfaceFormat.format;
			swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
			swapChainCreateInfo.imageExtent = extent;
			swapChainCreateInfo.imageArrayLayers = 1;  // 不是3D 应用， 图像层数为1
			swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

			uint32_t queueFamilyIndices[] = {
				(uint32_t)indices.graphicsFamily,
				(uint32_t)indices.presentFamily };

			// 1. VK_SHARING_MODE_EXCLUSIVE: 同一时间图像只能被一个队列簇占用，如果其他队列簇需要其所有权需要明确指定。这种方式提供了最好的性能。
			// 2. VK_SHARING_MODE_CONCURRENT : 图像可以被多个队列簇访问，不需要明确所有权从属关系。
			if (indices.graphicsFamily != indices.presentFamily)
			{
				swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapChainCreateInfo.queueFamilyIndexCount = 2;
				swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
				swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
			}


			swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform; // No transform op
			swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // alpha 通道是否混合其他窗体

			swapChainCreateInfo.presentMode = presentMode;
			swapChainCreateInfo.clipped = VK_TRUE;

			swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // 回收old swapchain 资源的引用句柄
		}

		// Create swapchain
		if (vkCreateSwapchainKHR(m_logicalDevice, &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
			throw std::runtime_error("faild to create swap chain!");
		}

		// Init swapChainImages
		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());
	}

	void createImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo imageViewCreateInfo = {};
			{
				imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				imageViewCreateInfo.image = m_swapChainImages[i];

				// 图像解析方式 viewType: 1D textures/2D textures/3D textures/cube maps
				imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				imageViewCreateInfo.format = m_swapChainImageFormat;

				// 颜色通道映射逻辑
				imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

				// Specify target of usage of image
				imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
				imageViewCreateInfo.subresourceRange.levelCount = 1;
				imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
				imageViewCreateInfo.subresourceRange.layerCount = 1;
			}

			if (vkCreateImageView(m_logicalDevice, &imageViewCreateInfo, nullptr, &m_swapChainImageViews[1]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image view!");
			}
		}
	}

private:

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreatInfo)
	{
		debugCreatInfo = {};;
		debugCreatInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreatInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreatInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreatInfo.pfnUserCallback = debugMessengerCallback;
	}

	void setupDebugCallback()
	{
		if (!enableValidationLayers) return;

		VkDebugReportCallbackCreateInfoEXT callbackCreatInfo = {};
		callbackCreatInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		callbackCreatInfo.flags = -VK_DEBUG_REPORT_ERROR_BIT_EXT | -VK_DEBUG_REPORT_WARNING_BIT_EXT;
		callbackCreatInfo.pfnCallback = debugReportCallback;

		if (CreateDebugReportCallbackEXT(m_instance, &callbackCreatInfo, nullptr, &m_debugReportCallback) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug callback!");
		}
	}

	void pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		VkResult result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		std::cout << "--Physical Device(" << deviceCount << "):\n";

		for (const auto& device : devices) {
			std::cout << "----Physical Device-" << device << std::endl;
			// TODO(fix) : no handle for present?
			if (isDeviceSuitable(device))
			{
				m_physicalDevice = device;
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

	}

	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		//VkPhysicalDeviceProperties deviceProperties;
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);

		//VkPhysicalDeviceFeatures devicesFeatures;
		//vkGetPhysicalDeviceFeatures(device, &devicesFeatures);

		//return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		//	&& devicesFeatures.geometryShader;

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);

			swapChainAdequate = (!swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty());
		}


		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());


		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}
			++i;
		}


		return indices;
	}

	std::vector<const char*> getRequiredExtensions()
	{
		std::vector<const char*> extensions;

		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
		std::cout << "glfwGetRequiredInstanceExtensions: " << glfwExtensions << std::endl;

		for (unsigned int i = 0; i < glfwExtensionsCount; ++i) {
			extensions.push_back(glfwExtensions[i]);
		}

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	std::vector<VkExtensionProperties> getRequiredExtensions_vec_VKExtensionProperties()
	{
		// 检查 glfw extensions
		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;

		// TODO(Fix maybe): duplicate get extensionCount
		vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, nullptr);
		std::cout << glfwExtensionCount << "extension supported" << std::endl;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::cout << "glfwGetRequiredInstanceExtensions: " << glfwExtensions << std::endl;

		std::vector<VkExtensionProperties> extensions(glfwExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, extensions.data());

		std::cout << "available extensions:" << std::endl;

		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << std::endl;

		}

		return extensions;
	}

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : gb_validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {

				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// Global predefine extensions
		std::set<std::string> requiredExtensions(gb_deviceExtensions.begin(), gb_deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		// Formats
		uint32_t formatCount;
		{
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
			}
		}


		// PresentModes
		uint32_t presentModeCount;
		{
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
			}
		}


		return details;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableForamts)
	{
		// Method 1
		if (availableForamts.size() == 1 && availableForamts[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		// Method 2
		for (const auto& availableForamt : availableForamts)
		{
			if (availableForamt.format == VK_FORMAT_B8G8R8A8_UNORM && availableForamt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableForamt;
			}
		}

		// Fallback : Grede and rank, but 通常选择第一个格式
		return availableForamts[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
	{
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				return availablePresentMode;
			}
		}

		return bestMode;
	}

	// 交换范围
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = { WIDTH,HEIGTH };

			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

private:

	static VKAPI_ATTR VkBool32 VKAPI_CALL
		debugReportCallback(
			VkDebugReportFlagsEXT flagss,
			VkDebugReportObjectTypeEXT flags_messageType,
			uint64_t obj, size_t location, int32_t code,
			const char* layerPrefix, const char* msg,
			void* pUserData)
	{
		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL
		debugMessengerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

};

int main()
{
	HelloTriangle app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	//glm::mat4 matrix;
	//glm::vec4 vec;
	//auto test = matrix * vec;



	return EXIT_SUCCESS;
}
