
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
//#include <vulkan/vulkan.hpp>

#define GLM_FORECE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ON
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>



#include<set>
#include<string>
#include <iostream>
#include <vector>
#include <stdio.h>
#include<cstdlib>
#include<stdexcept>
#include <functional>
#include<optional>
#include<fstream>
#include<chrono>


#include "Vertex.h"
#include"UniformBufferObject.h"


void throwRE(std::string info)
{
	throw std::runtime_error(info);
}


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


const std::vector<Vertex> gb_vertices = {
	{{ -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{ -0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> gb_indices = {
	0,1,2,
	2,3,0
};


/***************************************************************************************************/



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

	GLFWwindow* p_window;  //glfw 窗体
	VkInstance m_instance;	// Vulkan 实例

	VkDebugUtilsMessengerEXT m_debugMessengerCallback;  // 验证层 dubg messen,ger
	VkDebugReportCallbackEXT m_debugReportCallback;  // depercate

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; // 物理显卡设备
	VkDevice m_logicalDevice;	// 逻辑显卡设备

	VkQueue m_presentQueue;	// 展示端口
	VkQueue m_graphicsQueue;	// 图形端口

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain; // 包含一系列图片

	std::vector<VkFramebuffer> m_swapChainFrameBuffers; // VkFramebuffer:容纳renderpass 的目标图片（本身）由VkImage而来
	std::vector<VkImage> m_swapChainImages;	// VKImage: 一个可以读写的Texture
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;

	VkPipeline m_graphicsPipeLine;	// drawing 时 GPU 需要的状态信息集合结构体，包含如shader,光栅化,depth等
	VkRenderPass m_renderPass;	// 包含渲染等所有drawing命令在内完成, attachhment,subpass

	VkDescriptorSetLayout m_descriptorSetLayout; //描述符布局组合在一个对象中
	VkPipelineLayout m_pipelineLayout;	// 管线布局 linera 、 Optimal(tile平铺)·


	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	VkBuffer m_uniformBUffer;
	VkDeviceMemory m_uniformBUfferMemory;

	VkCommandPool m_commandPool;	// 命令池：储存缓存区的内存 不能直接调用函数进行绘制或内存操作等， 而是写入命令缓存区来调用
	std::vector<VkCommandBuffer> m_commandBuffers; // 命令缓冲区

	VkSemaphore m_imageAvailableSemaphore;	//协调交换链事件的信号量，不能像 栅栏(?Fench) 一样进入栅栏状态， 主要用于队列内或跨命令队列同步操作
	VkSemaphore m_renderFinishedSemaphore;	//栅栏主要用于 引用程序 自身与渲染操作同步

private:

	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		p_window = glfwCreateWindow(WIDTH, HEIGTH, "Vulkan window", nullptr, nullptr);

		glfwSetWindowUserPointer(p_window, this);
		glfwSetWindowSizeCallback(p_window, HelloTriangle::onWindowRedsized);

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
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();

		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffer();

		createCommandBuffers();
		createSemphores();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(p_window))
		{
			glfwPollEvents();

			updateUniformBuffer();
			drawFrame();
		}

		vkDeviceWaitIdle(m_logicalDevice);
	}

	void cleanupSwapChain()
	{
		for (size_t i = 0; i < m_swapChainFrameBuffers.size(); ++i)
		{
			vkDestroyFramebuffer(m_logicalDevice, m_swapChainFrameBuffers[i], nullptr);
		}
		vkDestroyPipeline(m_logicalDevice, m_graphicsPipeLine, nullptr);
		vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
		for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
		{
			vkDestroyImageView(m_logicalDevice, m_swapChainImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
	}

	void cleanup()
	{
		cleanupSwapChain();

		vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);

		vkDestroyBuffer(m_logicalDevice, m_uniformBUffer, nullptr);
		vkFreeMemory(m_logicalDevice, m_uniformBUfferMemory, nullptr);

		vkDestroyBuffer(m_logicalDevice, m_indexBuffer, nullptr);
		vkFreeMemory(m_logicalDevice, m_indexBufferMemory, nullptr);

		vkDestroyBuffer(m_logicalDevice, m_vertexBuffer, nullptr);
		vkFreeMemory(m_logicalDevice, m_vertexBufferMemory, nullptr);

		vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);

		vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

		vkDestroyDevice(m_logicalDevice, nullptr);

		//  Swap Chain 相关
		if (enableValidationLayers)
		{
			DestoryDebugUtilsMessengerEXT(m_instance, m_debugMessengerCallback, nullptr);
		}
		// DestoryDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);

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

			// Create swapchain
			if (vkCreateSwapchainKHR(m_logicalDevice, &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
				throw std::runtime_error("faild to create swap chain!");
			}
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

			if (vkCreateImageView(m_logicalDevice, &imageViewCreateInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image view!");
			}
		}
	}

	void createRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		{
			colorAttachment.format = m_swapChainImageFormat;	// 与交换链中的图像格式匹配
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;	// 不做多重采样

			/*  用于颜色和深度数据
			loadOp和storeOp决定了渲染前和渲染后数据在对应附件的操作行为。对于 loadOp 我们有如下选项：
				VK_ATTACHMENT_LOAD_OP_LOAD : 保存已经存在于当前附件的内容
				VK_ATTACHMENT_LOAD_OP_CLEAR : 起始阶段以一个常量清理附件内容
				VK_ATTACHMENT_LOAD_OP_DONT_CARE : 存在的内容未定义，忽略它们
			在绘制新的一帧内容之前，我们要做的是使用清理操作来清理帧缓冲区framebuffer为黑色。同时对于 storeOp 仅有两个选项：
				VK_ATTACHMENT_STORE_OP_STORE : 渲染的内容会存储在内存，并在之后进行读取操作
				VK_ATTACHMENT_STORE_OP_DONT_CARE : 帧缓冲区的内容在渲染操作完毕后设置为undefined
				我们要做的是渲染一个三角形在屏幕上，所以我们选择存储操作。
			*/
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			/*
			loadOp和storeOp应用在颜色和深度数据，同时stencilLoadOp / stencilStoreOp应用在模版数据。我们的应用程序不会做任何模版缓冲区的操作，所以它的loading和storing无关紧要。
			*/
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			/* 用于模板数据
			纹理和帧缓冲区在Vulkan中通常用VkImage 对象配以某种像素格式来代表。但是像素在内存中的布局可以基于预要对image图像进行的操作发生内存布局的变化。
			一些常用的布局 :
			VK_IMAGE_LAYOUT_COLOR_ATTACHMET_OPTIMAL: 图像作为颜色附件
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : 图像在交换链中被呈现
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : 图像作为目标，用于内存COPY操作*/
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Bug fix: init-undef sub-attachOptimal final-presentSrcKhr 
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		}

		// 子渲染通道需要 对前面定义一个或多个 attachment 的引用
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;  // attchment 的索引
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

		// Subpass 子通道
		VkSubpassDescription subpass = {};
		{
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef; // 在fragShader 中在使用 layout(locaiton=0)out vec4 outColor 来引用
			/*可以被子通道引用的附件类型如下:
				pInputAttachments: 附件从着色器中读取
				pResolveAttachments : 附件用于颜色附件的多重采样
				pDepthStencilAttachment : 附件用于深度和模版数据
				pPreserveAttachments : 附件不被子通道使用，但是数据被保存*/
		}

		//  解决在管线的起始阶段 主渲染 和 子渲染通道 在进行转换处理时 没有获取图像的问题, 指定依赖关系、从属、操作发生的事件
		VkSubpassDependency dependency = {};
		{
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 隐式子通道,取决于在 是否在 srcSubpass/dstSubpass 中指定
			dependency.dstSubpass = 0;	// subpass 索引

			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // 等待 swapchain 完成对应图像的读取操作
			dependency.srcAccessMask = 0;

			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // 等待 swapchain output
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // 阻止读写操作直到直到需要输入颜色时
		}

		// RenderPass creatInfo 渲染通道
		VkRenderPassCreateInfo renderPassCreateInfo = {};
		{
			renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

			renderPassCreateInfo.attachmentCount = 1;
			renderPassCreateInfo.pAttachments = &colorAttachment;

			renderPassCreateInfo.subpassCount = 1;
			renderPassCreateInfo.pSubpasses = &subpass;

			renderPassCreateInfo.dependencyCount = 1;
			renderPassCreateInfo.pDependencies = &dependency;
		}

		// 创建 m_renderPass
		if (vkCreateRenderPass(m_logicalDevice, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to crete render pass!");
		}


	}

	void createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		{
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 指定 binding 为 uniform 对象
			uboLayoutBinding.descriptorCount = 1;

			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // 定义描述符着色器在那个阶段使用，这里只在顶点
			uboLayoutBinding.pImmutableSamplers = nullptr; // 仅仅与图像采集有关 Optional
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		{
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &uboLayoutBinding;
		}

		if (vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createGraphicsPipeline()
	{
		/************************** Shader Stages ***********************************************/

		/* shader 模块 定义了图形管线可编程阶段的功能    */

		// input binary SPIR-V codes
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		// Compile Module
		auto vertShaderModule = createShaderModule(vertShaderCode);
		auto fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		{
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";  // 调用的主函数（入口）
		}
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		{
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";  // 调用的主函数（入口）
		}

		// Shader Stage cretInfo
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };


		// vertx input State cretInfo
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		{
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		}
		/************************** Shader Stages ***********************************************/


		/****************************** Fix-function State *****************************************/

		/* 用结构体定义固定管线功能， 比如： 输入装配、viewport、裁剪、光栅化、blending、采样等*/


		// Assembly state cretInfo ( Triangle format in this example)
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		{
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			//VkPipelineInputAssemblyStateCreateInfo结构体描述两件事情:顶点数据以什么类型的几何图元拓扑进行绘制及是否启用顶点索重新开始图元。图元的拓扑结构类型topology枚举值如下:
			//VK_PRIMITIVE_TOPOLOGY_POINT_LIST: 顶点到点
			//VK_PRIMITIVE_TOPOLOGY_LINE_LIST : 两点成线，顶点不共用
			//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : 两点成线，每个线段的结束顶点作为下一个线段的开始顶点
			//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : 三点成面，顶点不共用
			//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : 每个但教训的第二个、第三个顶点都作为下一个三角形的前两个顶点
		}

		// Viewport 描述 图像转换（映射）到 framebuffer（窗口） 的对应部分 包含入点xy，宽高hw，和深度minmax
		VkViewport viewport = {};
		{
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)m_swapChainExtent.width;
			viewport.height = (float)m_swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
		}

		// Scissor 裁剪 定义哪些区域内的像素被储存
		VkRect2D scissor = {};
		{
			scissor.offset = { 0,0 };
			scissor.extent = m_swapChainExtent; // 与视点保持一致
		}

		// Viewport State cretInfo 汇总使用裁剪和视点
		VkPipelineViewportStateCreateInfo viewportState = {};
		{
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;
		}


		// Resterization State cretInfo 光栅化，顶点塑性、tansmit 图像到 fragmentShader 着色、执行深度测试depth testing、面裁剪和裁剪测试，决定是否输出 整个图元拓扑 或是 边框（框线渲染）
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		{
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;

			//超过远近裁剪面的图元会进行收敛，而不丢弃，在如阴影贴图情况下有用，需GPU support; 否则会进士几何图元输出到framebuffer
			rasterizer.rasterizerDiscardEnable = VK_FALSE;

			//polygonMode决定几何产生图片的内容。下列有效模式:
			//VK_POLYGON_MODE_FILL: 多边形区域填充
			//VK_POLYGON_MODE_LINE : 多边形边缘线框绘制
			//VK_POLYGON_MODE_POINT : 多边形顶点作为描点绘制
			//使用任何模式填充需要开启GPU功能。
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;	// culling/font faces/call back facess/all 面裁剪的方式
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // 顺时针/逆时针 顶点绘制顺序

			// 深度值 config 这次设为false
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f;	// Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		}

		// Multisample State cretInfo 多重采样 这里不使用
		VkPipelineMultisampleStateCreateInfo multisampliing = {};
		{
			multisampliing.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampliing.sampleShadingEnable = VK_FALSE;
			multisampliing.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampliing.minSampleShading = 1.0f; // Optional
			multisampliing.pSampleMask = nullptr; // Optional
			multisampliing.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampliing.alphaToOneEnable = VK_FALSE; // Optional
		}

		// Blender 将fragment 输出的颜色与旧的 framebuffer 中存在的颜色进行混合： 混合（比例混合为新的颜色）或 位操作 , 这里采用第一种方式
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		{
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			if (colorBlendAttachment.blendEnable) {
				// finalcolor = newcolor & colorwritemask
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
				colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
				colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
				colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
				colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
			}
			else {
				// alpha blending: finalcolor.rgb = newAlpha*newColor + (1-newAlpha)*oldColor; finalcolor.a = newAlpha.a
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // blend mthod
				colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
				colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			}
		}

		// color blend cretInfo
		VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
		{
			colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendingCreateInfo.logicOpEnable = VK_FALSE;  // 采用第二种方式需要 为 true, 二进制操作在 logicOp 字段中指定
			colorBlendingCreateInfo.attachmentCount = 1;
			colorBlendingCreateInfo.pAttachments = &colorBlendAttachment;
			colorBlendingCreateInfo.blendConstants[0] = 0.0f; // Optional
			colorBlendingCreateInfo.blendConstants[1] = 0.0f; // Optional
			colorBlendingCreateInfo.blendConstants[2] = 0.0f; // Optional
			colorBlendingCreateInfo.blendConstants[3] = 0.0f; // Optional
		}

		// 运行时动态修改
		VkDynamicState dynamicState[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		{
			dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateCreateInfo.dynamicStateCount = 2; // same to above
			dynamicStateCreateInfo.pDynamicStates = dynamicState;
		}
		/********************************* Fixed-function State ********************************************/


		/************************************* Pipeline Layout ********************************************/

		/* 管线布局定义了 uniform(也许有DynamicState) 和 push values 的布局， 被 shader 每一次 drawing 的时候调用*/

		// pipeLayout Createinfo
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		{
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 1;
			pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout; // 使用成员变量持有的layout(createDescriptorSetLayout())
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;	//Optional
			pipelineLayoutCreateInfo.pPushConstantRanges = 0;	//Optional
		}
		if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("filed to create pipeline layout!");
		}
		/************************************* Pipeline Layout ********************************************/


		/************************************* Render Pass ********************************************/
		/*   定义于createRenderPass()函数中          */
		/************************************* Render Pass ********************************************/


		// Pipeline cretInfo
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		{
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

			// p_state 均为该 state 的 create info
			pipelineCreateInfo.stageCount = 2;  // vertex AND fragment State
			pipelineCreateInfo.pStages = shaderStages;

			pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
			pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
			pipelineCreateInfo.pViewportState = &viewportState;
			pipelineCreateInfo.pRasterizationState = &rasterizer;
			pipelineCreateInfo.pMultisampleState = &multisampliing;
			pipelineCreateInfo.pDepthStencilState = nullptr;  // Depth  Optional
			pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
			pipelineCreateInfo.pDynamicState = nullptr; // Dynamicstage Optional

			pipelineCreateInfo.layout = m_pipelineLayout;

			pipelineCreateInfo.renderPass = m_renderPass; //  引用 renderPass
			pipelineCreateInfo.subpass = 0; // subpass 子通道 的索引

			/*
			实际上还有两个参数:basePipelineHandle 和 basePipelineIndex。Vulkan允许您通过已经存在的管线创建新的图形管线。
			这种衍生出新管线的想法在于，当要创建的管线与现有管道功能相同时，获得较低的开销，同时也可以更快的完成管线切换，当它们来自同一个父管线。
			可以通过basePipelineHandle指定现有管线的句柄，也可以引用由basePipelineIndex所以创建的另一个管线。
			目前只有一个管线，所以我们只需要指定一个空句柄和一个无效的索引。
			只有在VkGraphicsPipelineCreateInfo的flags字段中也指定了VK_PIPELINE_CREATE_DERIVATIVE_BIT标志时，才需要使用这些值。
			*/
			pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	//  Optional
			pipelineCreateInfo.basePipelineIndex = -1;	//  Optional

		}

		// 1. 最终 创建 Pipeline 保存到成员变量, 可以传递多个 pipelineCreateInfo 来创建多个 管线
		// 2. 第二个参数cache 用于存储和复用与通过多次调用VkCreateGraphicsPipelines 函数相关的数据， 甚至在程序执行时缓存到一个文件中， 这样可以加速后续的管线创建逻辑, 这里暂时不设置
		if (vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_graphicsPipeLine) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}


		// Destroy shaders modules to release resources after complete createPipeline
		vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);
	}

	void createFramebuffers()
	{
		m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());

		for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
		{
			VkImageView attachments[] = {
				m_swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			{
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = m_renderPass;

				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;	//绑定到相应的附件描述的 VkImageView 对象

				framebufferInfo.width = m_swapChainExtent.width;
				framebufferInfo.height = m_swapChainExtent.height;
				framebufferInfo.layers = 1;	// 指定图像组中的层数
			}

			if (vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_swapChainFrameBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create freamebuffer!");
			}

		}
	}

	void createCommandPool()
	{
		/*
		命令缓存区将命令提交到一个设备队列（我们检索出来的 graphics 和 presentaion 队列） 上来执行
		但每个 commandPool 只能分配在单一的队列上（分配的命令要与队列类型一致）
		这里我们 绘制 选择 图形队列簇
		*/
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		{
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
			poolInfo.flags = 0;  // Optional
			/*
			有两个标志位用于command pools :
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: 提示命令缓冲区非常频繁的重新记录新命令(可能会改变内存分配行为)
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 允许命令缓冲区单独重新记录，没有这个标志，所有的命令缓冲区都必须一起重置
			我们仅仅在程序开始的时候记录命令缓冲区，并在主循环体main loop中多次执行，因此我们不会使用这些标志。
			*/
		}

		if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create commandPool!");
		}

	}

	void createVertexBuffer()
	{
		/*
		------>填充顶点缓冲区(传入顶点数据） （memmap m_vertBufMemory 到 cpu 访存 -> copy 顶点数据到该内存 -> 取消 map）
		method 1: 驱动程序不会立即拷贝数据到缓冲区中， 需要映射一块内存到缓冲区，然后拷贝到这块内存里, 但内存映射有一点性能损失
		method 2: 完成内存映射后，调用 vkFlushMappedMemoryRanges, 读取隐射内存时，调用 vkInvalidateMappedMemoryRanges
		这里选用 方法 1
		*/

		VkDeviceSize buffersize = sizeof(gb_vertices[0]) * gb_vertices.size();  // 要拷贝的顶点数据大小

		/*************************临时缓冲区： 使用临时的buffer,buffermemory和临时的commandBuffer来提升性能********/

		// 这里使用临时的缓冲区
		VkBuffer stagingBuffer; // 用 stagingbuffer 来划分SBmemory
		VkDeviceMemory stagingBufferMemory;
		// 创建 buffer 顶点缓冲、host visible、 host coherent
		/*
		我们使用stagingBuffer来划分stagingBufferMemory缓冲区用来映射、拷贝顶点数据。在本章节我们使用两个新的缓冲区usage标致类型：
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT：缓冲区可以用于源内存传输操作。
			VK_BUFFER_USAGE_TRANSFER_DST_BIT：缓冲区可以用于目标内存传输操作。
		*/
		createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);



		// map 临时 buufer, 把顶点数据拷贝过去
		void* data;
		vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, buffersize, 0, &data); // map 到临时内存
		memcpy(data, gb_vertices.data(), (size_t)buffersize);
		vkUnmapMemory(m_logicalDevice, stagingBufferMemory); // 停止映射

		// 创建 设备上 在 cpu访存上的 map
		createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vertexBuffer, m_vertexBufferMemory);


		//  从已经拷贝完的map区域 stagingBuffer, 拷贝到同样map的 device 的 m_vertexBuffer 区域
		copyBuffer(stagingBuffer, m_vertexBuffer, buffersize);  // encapsulation 封装 copy 操作


		// 清理掉使用完的 stagingBuffer 和 stagingBufferMemory
		vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
	}

	void createIndexBuffer()
	{
		// 类似于vertex buffer
		VkDeviceSize bufferSize = sizeof(gb_indices[0]) * gb_indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, gb_indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

		copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);


		// clean stage data
		vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);

	}

	void createUniformBuffer()
	{
		VkDeviceSize buffersize = sizeof(UniformBufferObject);

		createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBUffer, m_uniformBUfferMemory);
	}

	void createCommandBuffers()
	{
		m_commandBuffers.resize(m_swapChainFrameBuffers.size());

		// 1. 分配命令缓存区内存
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		{
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandPool = m_commandPool;
			commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			/*
			level参数指定分配的命令缓冲区的主从关系。
				VK_COMMAND_BUFFER_LEVEL_PRIMARY : 可以提交到队列执行，但不能从其他的命令缓冲区调用。
				VK_COMMAND_BUFFER_LEVEL_SECONDARY : 无法直接提交，但是可以从主命令缓冲区调用。
			我们不会在这里使用辅助缓冲区功能，但是可以想像，对于复用主缓冲区的常用操作很有帮助。
			*/
			commandBufferAllocateInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
		}
		if (vkAllocateCommandBuffers(m_logicalDevice, &commandBufferAllocateInfo, m_commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			// 2. 启动命令缓存记录
			VkCommandBufferBeginInfo beginInfo = {};
			{
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
				beginInfo.pInheritanceInfo = nullptr; // Optional
				/*
				flags标志位参数用于指定如何使用命令缓冲区。可选的参数类型如下:
					VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: 命令缓冲区将在执行一次后立即重新记录。
					VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 这是一个辅助缓冲区，它限制在在一个渲染通道中。
					VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 命令缓冲区也可以重新提交，同时它也在等待执行。
				我们使用了最后一个标志，因为我们可能已经在下一帧的时候安排了绘制命令，而最后一帧尚未完成。pInheritanceInfo参数与辅助缓冲区相关。它指定从主命令缓冲区继承的状态。
				如果命令缓冲区已经被记录一次，那么调用vkBeginCommandBuffer会隐式地重置它。否则将命令附加到缓冲区是不可能的。
				*/
			}

			/*<-------------------------------------------------------------------------------->*/
			vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo); // S 启动命令记录

			// 3. 启动渲染通道
			VkRenderPassBeginInfo renderPassInfo = {};
			VkClearValue clearColor;
			{
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_renderPass;
				renderPassInfo.framebuffer = m_swapChainFrameBuffers[i];

				renderPassInfo.renderArea.offset = { 0,0 };
				renderPassInfo.renderArea.extent = m_swapChainExtent;

				clearColor = { 0.0f,0.0f,0.0f,1.0f };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;
			}

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // S 开启渲染通道
			/// <vkCmdBeginRenderPass>
			///	对于每个命令，第一个参数总是记录该命令的命令缓冲区。第二个参数指定我们传递的渲染通道的具体信息。
			///	最后的参数控制如何提供render pass将要应用的绘制命令。它使用以下数值任意一个 :
			///		VK_SUBPASS_CONTENTS_INLINE: 渲染过程命令被嵌入在主命令缓冲区中，没有辅助缓冲区执行。
			///		VK_SUBPASS_CONTENTS_SECONDARY_COOMAND_BUFFERS : 渲染通道命令将会从辅助命令缓冲区执行。
			///	我们不会使用辅助命令缓冲区，所以我们选择第一个。
			/// </vkCmdBeginRenderPass>

			// 4. 基本绘图命令

			/// 绑定管线
			vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeLine);
			VkBuffer vertexBuffers[] = { m_vertexBuffer };
			VkDeviceSize offsets[] = { 0 };

			/// 绑定顶点缓冲区
			vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

			/// 绑定索引缓冲区
			vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

			/// 使用 vkCmdDrawIndexed 替代，复用顶点
			//vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
			/*
			实际的vkCmdDraw函数有点与字面意思不一致，它是如此简单，仅因为我们提前指定所有渲染相关的信息。它有如下的参数需要指定，除了命令缓冲区:
				vertexCount: 即使我们没有顶点缓冲区，但是我们仍然有3个定点需要绘制。
				instanceCount : 用于instanced 渲染，如果没有使用请填1。
				firstVertex : 作为顶点缓冲区的偏移量，定义gl_VertexIndex的最小值。
				firstInstance : 作为instanced 渲染的偏移量，定义了gl_InstanceIndex的最小值。
			*/
			vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(gb_indices.size()), 1, 0, 0, 0);


			// 5. 结束渲染
			vkCmdEndRenderPass(m_commandBuffers[i]);  // E 结束渲染

			auto result = (vkEndCommandBuffer(m_commandBuffers[i])); // E 结束命令记录
			/*<-------------------------------------------------------------------------------->*/
			if (result != VK_SUCCESS)
			{
				throw	std::runtime_error("failed to record command buffer!");
			}


		}
	}

	void createSemphores()
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}

private:

	void recreateSwapChain() {
		vkDeviceWaitIdle(m_logicalDevice); // 不触碰正在使用中的资源

		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandBuffers();
	}


private:

	void drawFrame()
	{


		// 1. 创建信号量 : creteSemphores() in initVulkan()
		// 2. 从交换链获取图像
		uint32_t imageIndex;
		VkResult  result = vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		/*
		vkAcquireNextImageKHR函数前两个参数是我们希望获取到图像的逻辑设备和交换链。
		第三个参数指定获取有效图像的操作timeout，单位纳秒。我们使用64位无符号最大值禁止timeout。
		接下来的两个参数指定使用的同步对象，当presentation引擎完成了图像的呈现后会使用该对象发起信号。
		这就是开始绘制的时间点。它可以指定一个信号量semaphore或者栅栏或者两者。出于目的性，我们会使用imageAvailableSemaphore。
		最后的参数指定交换链中成为available状态的图像对应的索引。其中索引会引用交换链图像数组swapChainImages的图像VkImage。我们使用这个索引选择正确的命令缓冲区。
		*/
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {// swap chain 与 surface 不再兼容，不可进行渲染
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // SUPOPTIMAL 交换链仍可以向surface提交图像，但是surface的熟悉不再匹配准确，比如平台可能重新调整图像的尺寸适应大小
			throw std::runtime_error("failed to acquire swap chain image");
		}

		// 3. 提交命令缓冲区
		VkSubmitInfo submitInfo = {};
		VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
		{
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1; // Bug fixed: Lost 2 define
			submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

		}

		if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// 4. 将结果提交到 swapchain 交换链, 显示在屏幕上
		VkPresentInfoKHR presentInfo = {};
		VkSwapchainKHR swapChains[] = { m_swapChain };
		{
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores; // 指定需要等待的信号量， 如 VkSubmitInfo 一样

			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains; // 指定提交的 target swapchain 和 每个 swapchain 的索引
			presentInfo.pImageIndices = &imageIndex;

			presentInfo.pResults = nullptr; // 指定校验结果值，我们可以直接使用 vkQueuePresentKHR() 的返回值判断  Optional
		}

		result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present image/imageIndex to swapchain!");
		}

		vkQueueWaitIdle(m_presentQueue);
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
			int width, height;
			glfwGetWindowSize(p_window, &width, &height);
			VkExtent2D actualExtent = { width,height };

			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		{
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = code.size();

			shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*> (code.data());
		}

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module");
		}

		return shaderModule;
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

		// 为缓冲区找到合适的内存类型
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
			if (typeFilter & (1 << i) &&  // vertexbuffer
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) // 顶点
			{
				return i;
			}
		}

		throwRE("failed to find suitable memory type!");
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{

		// 1. 创建缓冲区,
		VkBufferCreateInfo bufferInfo = {};
		{
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;	//使用bit来指定多个使用目的
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 独占模式，不进行缓冲区共享（给其他队列蔟）
		}

		if (vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throwRE("failed to create vertexbuffer");
		}

		// 2. 内存需求
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memoryRequirements);

		// 3. 内存分配
		VkMemoryAllocateInfo allocInfo = {};
		{
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memoryRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);
		}
		if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throwRE("failed to allocate vertex buffer memory!");
		}

		// 4. 关联到缓冲区	
		vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		// 分配一个临时的命令缓冲区,来提交操作命令
		VkCommandBufferAllocateInfo allocInfo = {};
		{
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = m_commandPool;
			allocInfo.commandBufferCount = 1;
		}

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

		// 类似绘制命令的启动与提交属性
		VkCommandBufferBeginInfo beginInfo = {};
		{
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 好习惯
		}

		// 写入命令
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		{
			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		}
		vkEndCommandBuffer(commandBuffer); // 停止记录

		// 提交命令
		VkSubmitInfo submitInfo = {};
		{
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;
		}
		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue); // 等待传输命令完成 或使用 fence来安排多个连续的传输操作


		// 删除临时命令缓冲区
		vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
	}

	void updateUniformBUffer()
	{
		using clock = std::chrono::high_resolution_clock;

		static auto startTime = clock::now(); // static 开始时间
		auto currentTime = clock::now();

		// 计算时间来控制旋转的比例
		float time = std::chrono::duration_cast<std::chrono::milliseconds>
			(currentTime - startTime).count() / 1000.0f;

		UniformBufferObject ubo = {};
		{
			//  归一化mat、 旋转角度、 旋转轴
			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			// 视角方向（角度）、中心位置、头顶指向方向
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			// Fov 45度角， 宽高比， 近/远裁剪面
			ubo.proj = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 10.0f);
			
			// GLM 为 openGL 设计的， 他的裁剪坐标(proj.y)与vulkan相反
			ubo.proj[1][1] *= -1;
		}

		// 将 ubo 对象拷贝到创建好的 uniformBufferMemory 中
		void* data;
		vkMapMemory(m_logicalDevice, m_uniformBUfferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(m_logicalDevice, m_uniformBUfferMemory);
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

	static void onWindowRedsized(GLFWwindow* window, int width, int height)
	{
		if (width == 0 || height == 0) return;

		HelloTriangle* app = reinterpret_cast<HelloTriangle*>(glfwGetWindowUserPointer(window));
		app->recreateSwapChain();
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
