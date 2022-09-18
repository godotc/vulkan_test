
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
#include<fstream>

#include "Vertex.h"


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


const std::vector<Vertex> vertices = {
	{{ 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	{{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}}
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

	GLFWwindow* p_window;  //glfw ����
	VkInstance m_instance;	// Vulkan ʵ��

	VkDebugUtilsMessengerEXT m_debugMessengerCallback;  // ��֤�� dubg messen,ger
	VkDebugReportCallbackEXT m_debugReportCallback;  // depercate

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; // �����Կ��豸
	VkDevice m_logicalDevice;	// �߼��Կ��豸

	VkQueue m_presentQueue;	// չʾ�˿�
	VkQueue m_graphicsQueue;	// ͼ�ζ˿�

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain; // ����һϵ��ͼƬ

	std::vector<VkFramebuffer> m_swapChainFrameBuffers; // VkFramebuffer:����renderpass ��Ŀ��ͼƬ��������VkImage����
	std::vector<VkImage> m_swapChainImages;	// VKImage: һ�����Զ�д��Texture
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;

	VkPipeline m_graphicsPipeLine;	// drawing ʱ GPU ��Ҫ��״̬��Ϣ���Ͻṹ�壬������shader,��դ��,depth��
	VkRenderPass m_renderPass;	// ������Ⱦ������drawing�����������, attachhment,subpass
	VkPipelineLayout m_pipelineLayout;	// ���߲��� linera �� Optimal(tileƽ��)��

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

	VkCommandPool m_commandPool;	// ����أ����滺�������ڴ� ����ֱ�ӵ��ú������л��ƻ��ڴ�����ȣ� ����д���������������
	std::vector<VkCommandBuffer> m_commandBuffers; // �������

	VkSemaphore m_imageAvailableSemaphore;	//Э���������¼����ź����������� դ��(?Fench) һ������դ��״̬�� ��Ҫ���ڶ����ڻ���������ͬ������
	VkSemaphore m_renderFinishedSemaphore;	//դ����Ҫ���� ���ó��� ��������Ⱦ����ͬ��

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
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createVertexBuffer();
		createCommandBuffers();
		createSemphores();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(p_window))
		{
			glfwPollEvents();

			drawFrame();

			vkDeviceWaitIdle(m_logicalDevice);
		}
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

		vkFreeMemory(m_logicalDevice, m_vertexBufferMemory, nullptr);
		vkDestroyBuffer(m_logicalDevice, m_vertexBuffer, nullptr);

		vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);

		vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

		vkDestroyDevice(m_logicalDevice, nullptr);

		//  Swap Chain ���
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

		// ���� ApplicaitonInfo �ṹ
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

		// ���� Creatinfo, ���� ApplicationInfo
		VkInstanceCreateInfo createInfo = {};
		{
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appinfo;

			// ��ȡ extesnsions ����������ÿ��extesnsion������
			extensions = getRequiredExtensions();

			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			// ����ȫ��layer
			createInfo.enabledLayerCount = 0;
		}

		// ���� debugMessenger
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

		// ����  instance
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

		// ����
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

		// ��Ҫ������
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// ���� device createInfo  
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

		// ���� device
		if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}


		// ��ȡ present ����
		vkGetDeviceQueue(m_logicalDevice, indices.presentFamily, 0, &m_presentQueue);

		// ��ȡ device ����
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
			swapChainCreateInfo.imageArrayLayers = 1;  // ����3D Ӧ�ã� ͼ�����Ϊ1
			swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


			QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

			uint32_t queueFamilyIndices[] = {
				(uint32_t)indices.graphicsFamily,
				(uint32_t)indices.presentFamily };

			// 1. VK_SHARING_MODE_EXCLUSIVE: ͬһʱ��ͼ��ֻ�ܱ�һ�����д�ռ�ã�����������д���Ҫ������Ȩ��Ҫ��ȷָ�������ַ�ʽ�ṩ����õ����ܡ�
			// 2. VK_SHARING_MODE_CONCURRENT : ͼ����Ա�������дط��ʣ�����Ҫ��ȷ����Ȩ������ϵ��
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
			swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // alpha ͨ���Ƿ�����������

			swapChainCreateInfo.presentMode = presentMode;
			swapChainCreateInfo.clipped = VK_TRUE;


			swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // ����old swapchain ��Դ�����þ��

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

				// ͼ�������ʽ viewType: 1D textures/2D textures/3D textures/cube maps
				imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				imageViewCreateInfo.format = m_swapChainImageFormat;

				// ��ɫͨ��ӳ���߼�
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
			colorAttachment.format = m_swapChainImageFormat;	// �뽻�����е�ͼ���ʽƥ��
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;	// �������ز���

			/*  ������ɫ���������
			loadOp��storeOp��������Ⱦǰ����Ⱦ�������ڶ�Ӧ�����Ĳ�����Ϊ������ loadOp ����������ѡ�
				VK_ATTACHMENT_LOAD_OP_LOAD : �����Ѿ������ڵ�ǰ����������
				VK_ATTACHMENT_LOAD_OP_CLEAR : ��ʼ�׶���һ����������������
				VK_ATTACHMENT_LOAD_OP_DONT_CARE : ���ڵ�����δ���壬��������
			�ڻ����µ�һ֡����֮ǰ������Ҫ������ʹ���������������֡������framebufferΪ��ɫ��ͬʱ���� storeOp ��������ѡ�
				VK_ATTACHMENT_STORE_OP_STORE : ��Ⱦ�����ݻ�洢���ڴ棬����֮����ж�ȡ����
				VK_ATTACHMENT_STORE_OP_DONT_CARE : ֡����������������Ⱦ������Ϻ�����Ϊundefined
				����Ҫ��������Ⱦһ������������Ļ�ϣ���������ѡ��洢������
			*/
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			/*
			loadOp��storeOpӦ������ɫ��������ݣ�ͬʱstencilLoadOp / stencilStoreOpӦ����ģ�����ݡ����ǵ�Ӧ�ó��򲻻����κ�ģ�滺�����Ĳ�������������loading��storing�޹ؽ�Ҫ��
			*/
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			/* ����ģ������
			�����֡��������Vulkan��ͨ����VkImage ��������ĳ�����ظ�ʽ�����������������ڴ��еĲ��ֿ��Ի���ԤҪ��imageͼ����еĲ��������ڴ沼�ֵı仯��
			һЩ���õĲ��� :
			VK_IMAGE_LAYOUT_COLOR_ATTACHMET_OPTIMAL: ͼ����Ϊ��ɫ����
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : ͼ���ڽ������б�����
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : ͼ����ΪĿ�꣬�����ڴ�COPY����*/
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Bug fix: init-undef sub-attachOptimal final-presentSrcKhr 
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		}

		// ����Ⱦͨ����Ҫ ��ǰ�涨��һ������ attachment ������
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;  // attchment ������
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

		// Subpass ��ͨ��
		VkSubpassDescription subpass = {};
		{
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef; // ��fragShader ����ʹ�� layout(locaiton=0)out vec4 outColor ������
			/*���Ա���ͨ�����õĸ�����������:
				pInputAttachments: ��������ɫ���ж�ȡ
				pResolveAttachments : ����������ɫ�����Ķ��ز���
				pDepthStencilAttachment : ����������Ⱥ�ģ������
				pPreserveAttachments : ����������ͨ��ʹ�ã��������ݱ�����*/
		}

		//  ����ڹ��ߵ���ʼ�׶� ����Ⱦ �� ����Ⱦͨ�� �ڽ���ת������ʱ û�л�ȡͼ�������, ָ��������ϵ�������������������¼�
		VkSubpassDependency dependency = {};
		{
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // ��ʽ��ͨ��,ȡ������ �Ƿ��� srcSubpass/dstSubpass ��ָ��
			dependency.dstSubpass = 0;	// subpass ����

			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // �ȴ� swapchain ��ɶ�Ӧͼ��Ķ�ȡ����
			dependency.srcAccessMask = 0;

			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // �ȴ� swapchain output
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // ��ֹ��д����ֱ��ֱ����Ҫ������ɫʱ
		}

		// RenderPass creatInfo ��Ⱦͨ��
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

		// ���� m_renderPass
		if (vkCreateRenderPass(m_logicalDevice, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to crete render pass!");
		}


	}

	void createGraphicsPipeline()
	{
		/************************** Shader Stages ***********************************************/

		/* shader ģ�� ������ͼ�ι��߿ɱ�̽׶εĹ���    */

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
			vertShaderStageInfo.pName = "main";  // ���õ�����������ڣ�
		}
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		{
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";  // ���õ�����������ڣ�
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

		/* �ýṹ�嶨��̶����߹��ܣ� ���磺 ����װ�䡢viewport���ü�����դ����blending��������*/


		// Assembly state cretInfo ( Triangle format in this example)
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		{
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			//VkPipelineInputAssemblyStateCreateInfo�ṹ��������������:����������ʲô���͵ļ���ͼԪ���˽��л��Ƽ��Ƿ����ö��������¿�ʼͼԪ��ͼԪ�����˽ṹ����topologyö��ֵ����:
			//VK_PRIMITIVE_TOPOLOGY_POINT_LIST: ���㵽��
			//VK_PRIMITIVE_TOPOLOGY_LINE_LIST : ������ߣ����㲻����
			//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : ������ߣ�ÿ���߶εĽ���������Ϊ��һ���߶εĿ�ʼ����
			//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : ������棬���㲻����
			//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : ÿ������ѵ�ĵڶ��������������㶼��Ϊ��һ�������ε�ǰ��������
		}

		// Viewport ���� ͼ��ת����ӳ�䣩�� framebuffer�����ڣ� �Ķ�Ӧ���� �������xy�����hw�������minmax
		VkViewport viewport = {};
		{
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)m_swapChainExtent.width;
			viewport.height = (float)m_swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
		}

		// Scissor �ü� ������Щ�����ڵ����ر�����
		VkRect2D scissor = {};
		{
			scissor.offset = { 0,0 };
			scissor.extent = m_swapChainExtent; // ���ӵ㱣��һ��
		}

		// Viewport State cretInfo ����ʹ�òü����ӵ�
		VkPipelineViewportStateCreateInfo viewportState = {};
		{
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;
		}


		// Resterization State cretInfo ��դ�����������ԡ�tansmit ͼ�� fragmentShader ��ɫ��ִ����Ȳ���depth testing����ü��Ͳü����ԣ������Ƿ���� ����ͼԪ���� ���� �߿򣨿�����Ⱦ��
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		{
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;

			//����Զ���ü����ͼԪ���������������������������Ӱ��ͼ��������ã���GPU support; ������ʿ����ͼԪ�����framebuffer
			rasterizer.rasterizerDiscardEnable = VK_FALSE;

			//polygonMode�������β���ͼƬ�����ݡ�������Чģʽ:
			//VK_POLYGON_MODE_FILL: ������������
			//VK_POLYGON_MODE_LINE : ����α�Ե�߿����
			//VK_POLYGON_MODE_POINT : ����ζ�����Ϊ������
			//ʹ���κ�ģʽ�����Ҫ����GPU���ܡ�
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;	// culling/font faces/call back facess/all ��ü��ķ�ʽ
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // ˳ʱ��/��ʱ�� �������˳��

			// ���ֵ config �����Ϊfalse
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f;	// Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		}

		// Multisample State cretInfo ���ز��� ���ﲻʹ��
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

		// Blender ��fragment �������ɫ��ɵ� framebuffer �д��ڵ���ɫ���л�ϣ� ��ϣ��������Ϊ�µ���ɫ���� λ���� , ������õ�һ�ַ�ʽ
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
			colorBlendingCreateInfo.logicOpEnable = VK_FALSE;  // ���õڶ��ַ�ʽ��Ҫ Ϊ true, �����Ʋ����� logicOp �ֶ���ָ��
			colorBlendingCreateInfo.attachmentCount = 1;
			colorBlendingCreateInfo.pAttachments = &colorBlendAttachment;
			colorBlendingCreateInfo.blendConstants[0] = 0.0f; // Optional
			colorBlendingCreateInfo.blendConstants[1] = 0.0f; // Optional
			colorBlendingCreateInfo.blendConstants[2] = 0.0f; // Optional
			colorBlendingCreateInfo.blendConstants[3] = 0.0f; // Optional
		}

		// ����ʱ��̬�޸�
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

		/* ���߲��ֶ����� uniform(Ҳ����DynamicState) �� push values �Ĳ��֣� �� shader ÿһ�� drawing ��ʱ�����*/

		// pipeLayout Createinfo
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		{
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 0; // Optional
			pipelineLayoutCreateInfo.pSetLayouts = nullptr;	//Optional
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;	//Optional
			pipelineLayoutCreateInfo.pPushConstantRanges = 0;	//Optional
		}
		if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("filed to create pipeline layout!");
		}
		/************************************* Pipeline Layout ********************************************/


		/************************************* Render Pass ********************************************/
		/*   ������createRenderPass()������          */
		/************************************* Render Pass ********************************************/


		// Pipeline cretInfo
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		{
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

			// p_state ��Ϊ�� state �� create info
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

			pipelineCreateInfo.renderPass = m_renderPass; //  ���� renderPass
			pipelineCreateInfo.subpass = 0; // subpass ��ͨ�� ������

			/*
			ʵ���ϻ�����������:basePipelineHandle �� basePipelineIndex��Vulkan������ͨ���Ѿ����ڵĹ��ߴ����µ�ͼ�ι��ߡ�
			�����������¹��ߵ��뷨���ڣ���Ҫ�����Ĺ��������йܵ�������ͬʱ����ýϵ͵Ŀ�����ͬʱҲ���Ը������ɹ����л�������������ͬһ�������ߡ�
			����ͨ��basePipelineHandleָ�����й��ߵľ����Ҳ����������basePipelineIndex���Դ�������һ�����ߡ�
			Ŀǰֻ��һ�����ߣ���������ֻ��Ҫָ��һ���վ����һ����Ч��������
			ֻ����VkGraphicsPipelineCreateInfo��flags�ֶ���Ҳָ����VK_PIPELINE_CREATE_DERIVATIVE_BIT��־ʱ������Ҫʹ����Щֵ��
			*/
			pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	//  Optional
			pipelineCreateInfo.basePipelineIndex = -1;	//  Optional

		}

		// 1. ���� ���� Pipeline ���浽��Ա����, ���Դ��ݶ�� pipelineCreateInfo ��������� ����
		// 2. �ڶ�������cache ���ڴ洢�͸�����ͨ����ε���VkCreateGraphicsPipelines ������ص����ݣ� �����ڳ���ִ��ʱ���浽һ���ļ��У� �������Լ��ٺ����Ĺ��ߴ����߼�, ������ʱ������
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
				framebufferInfo.pAttachments = attachments;	//�󶨵���Ӧ�ĸ��������� VkImageView ����

				framebufferInfo.width = m_swapChainExtent.width;
				framebufferInfo.height = m_swapChainExtent.height;
				framebufferInfo.layers = 1;	// ָ��ͼ�����еĲ���
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
		��������������ύ��һ���豸���У����Ǽ��������� graphics �� presentaion ���У� ����ִ��
		��ÿ�� commandPool ֻ�ܷ����ڵ�һ�Ķ����ϣ����������Ҫ���������һ�£�
		�������� ���� ѡ�� ͼ�ζ��д�
		*/
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		{
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
			poolInfo.flags = 0;  // Optional
			/*
			��������־λ����command pools :
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: ��ʾ��������ǳ�Ƶ�������¼�¼������(���ܻ�ı��ڴ������Ϊ)
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : ������������������¼�¼��û�������־�����е��������������һ������
			���ǽ����ڳ���ʼ��ʱ���¼���������������ѭ����main loop�ж��ִ�У�������ǲ���ʹ����Щ��־��
			*/
		}

		if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create commandPool!");
		}

	}

	void createVertexBuffer()
	{
		/*
		------>��䶥�㻺����(���붥�����ݣ� ��memmap m_vertBufMemory �� cpu �ô� -> copy �������ݵ����ڴ� -> ȡ�� map��
		method 1: �������򲻻������������ݵ��������У� ��Ҫӳ��һ���ڴ浽��������Ȼ�󿽱�������ڴ���, ���ڴ�ӳ����һ��������ʧ
		method 2: ����ڴ�ӳ��󣬵��� vkFlushMappedMemoryRanges, ��ȡ�����ڴ�ʱ������ vkInvalidateMappedMemoryRanges
		����ѡ�� ���� 1
		*/

		VkDeviceSize buffersize = sizeof(vertices[0]) * vertices.size();  // Ҫ�����Ķ������ݴ�С

		/*************************��ʱ�������� ʹ����ʱ��buffer,buffermemory����ʱ��commandBuffer����������********/

		// ����ʹ����ʱ�Ļ�����
		VkBuffer stagingBuffer; // �� stagingbuffer ������SBmemory
		VkDeviceMemory stagingBufferMemory;
		// ���� buffer ���㻺�塢host visible�� host coherent
		/*
		����ʹ��stagingBuffer������stagingBufferMemory����������ӳ�䡢�����������ݡ��ڱ��½�����ʹ�������µĻ�����usage�������ͣ�
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT����������������Դ�ڴ洫�������
			VK_BUFFER_USAGE_TRANSFER_DST_BIT����������������Ŀ���ڴ洫�������
		*/
		createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);



		// map ��ʱ buufer, �Ѷ������ݿ�����ȥ
		void* data;
		vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, buffersize, 0, &data); // map ����ʱ�ڴ�
		memcpy(data, vertices.data(), (size_t)buffersize);
		vkUnmapMemory(m_logicalDevice, stagingBufferMemory); // ֹͣӳ��

		// ���� �豸�� �� cpu�ô��ϵ� map
		createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vertexBuffer, m_vertexBufferMemory);


		//  ���Ѿ��������map���� stagingBuffer, ������ͬ��map�� device �� m_vertexBuffer ����
		copyBuffer(stagingBuffer, m_vertexBuffer, buffersize);  // encapsulation ��װ copy ����


		// �����ʹ����� stagingBuffer �� stagingBufferMemory
		vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
	}

	void createCommandBuffers()
	{
		m_commandBuffers.resize(m_swapChainFrameBuffers.size());

		// 1. �����������
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		{
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandPool = m_commandPool;
			commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			/*
			level����ָ�������������������ӹ�ϵ��
				VK_COMMAND_BUFFER_LEVEL_PRIMARY : �����ύ������ִ�У������ܴ�����������������á�
				VK_COMMAND_BUFFER_LEVEL_SECONDARY : �޷�ֱ���ύ�����ǿ��Դ�������������á�
			���ǲ���������ʹ�ø������������ܣ����ǿ������񣬶��ڸ������������ĳ��ò������а�����
			*/
			commandBufferAllocateInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
		}

		if (vkAllocateCommandBuffers(m_logicalDevice, &commandBufferAllocateInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			// 2. ����������¼
			VkCommandBufferBeginInfo beginInfo = {};
			{
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
				beginInfo.pInheritanceInfo = nullptr; // Optional
				/*
				flags��־λ��������ָ�����ʹ�������������ѡ�Ĳ�����������:
					VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: �����������ִ��һ�κ��������¼�¼��
					VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : ����һ������������������������һ����Ⱦͨ���С�
					VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : �������Ҳ���������ύ��ͬʱ��Ҳ�ڵȴ�ִ�С�
				����ʹ�������һ����־����Ϊ���ǿ����Ѿ�����һ֡��ʱ�����˻�����������һ֡��δ��ɡ�pInheritanceInfo�����븨����������ء���ָ��������������̳е�״̬��
				�����������Ѿ�����¼һ�Σ���ô����vkBeginCommandBuffer����ʽ������������������ӵ��������ǲ����ܵġ�
				*/
			}
			vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);

			// 3. ������Ⱦͨ��
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
			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			/*
			����ÿ�������һ���������Ǽ�¼�����������������ڶ�������ָ�����Ǵ��ݵ���Ⱦͨ���ľ�����Ϣ�����Ĳ�����������ṩrender pass��ҪӦ�õĻ��������ʹ��������ֵ����һ�� :
				VK_SUBPASS_CONTENTS_INLINE: ��Ⱦ�������Ƕ��������������У�û�и���������ִ�С�
				VK_SUBPASS_CONTENTS_SECONDARY_COOMAND_BUFFERS : ��Ⱦͨ�������Ӹ����������ִ�С�
			���ǲ���ʹ�ø��������������������ѡ���һ����
			*/

			// 4. ������ͼ����
			vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeLine);

			VkBuffer vertexBuffers[] = { m_vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
			/*
			ʵ�ʵ�vkCmdDraw�����е���������˼��һ�£�������˼򵥣�����Ϊ������ǰָ��������Ⱦ��ص���Ϣ���������µĲ�����Ҫָ���������������:
				vertexCount: ��ʹ����û�ж��㻺����������������Ȼ��3��������Ҫ���ơ�
				instanceCount : ����instanced ��Ⱦ�����û��ʹ������1��
				firstVertex : ��Ϊ���㻺������ƫ����������gl_VertexIndex����Сֵ��
				firstInstance : ��Ϊinstanced ��Ⱦ��ƫ������������gl_InstanceIndex����Сֵ��
			*/

			// 5. ������Ⱦ
			vkCmdEndRenderPass(m_commandBuffers[i]);

			// ��ֹͣ��������Ĺ���
			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
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
		vkDeviceWaitIdle(m_logicalDevice); // ����������ʹ���е���Դ

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


		// 1. �����ź��� : creteSemphores() in initVulkan()
		// 2. �ӽ�������ȡͼ��
		uint32_t imageIndex;
		VkResult  result = vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		/*
		vkAcquireNextImageKHR����ǰ��������������ϣ����ȡ��ͼ����߼��豸�ͽ�������
		����������ָ����ȡ��Чͼ��Ĳ���timeout����λ���롣����ʹ��64λ�޷������ֵ��ֹtimeout��
		����������������ָ��ʹ�õ�ͬ�����󣬵�presentation���������ͼ��ĳ��ֺ��ʹ�øö������źš�
		����ǿ�ʼ���Ƶ�ʱ��㡣������ָ��һ���ź���semaphore����դ���������ߡ�����Ŀ���ԣ����ǻ�ʹ��imageAvailableSemaphore��
		���Ĳ���ָ���������г�Ϊavailable״̬��ͼ���Ӧ���������������������ý�����ͼ������swapChainImages��ͼ��VkImage������ʹ���������ѡ����ȷ�����������
		*/
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {// swap chain �� surface ���ټ��ݣ����ɽ�����Ⱦ
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // SUPOPTIMAL �������Կ�����surface�ύͼ�񣬵���surface����Ϥ����ƥ��׼ȷ������ƽ̨�������µ���ͼ��ĳߴ���Ӧ��С
			throw std::runtime_error("failed to acquire swap chain image");
		}

		// 3. �ύ�������
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

		// 4. ������ύ�� swapchain ������, ��ʾ����Ļ��
		VkPresentInfoKHR presentInfo = {};
		VkSwapchainKHR swapChains[] = { m_swapChain };
		{
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores; // ָ����Ҫ�ȴ����ź����� �� VkSubmitInfo һ��

			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains; // ָ���ύ�� target swapchain �� ÿ�� swapchain ������
			presentInfo.pImageIndices = &imageIndex;

			presentInfo.pResults = nullptr; // ָ��У����ֵ�����ǿ���ֱ��ʹ�� vkQueuePresentKHR() �ķ���ֵ�ж�  Optional
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
		// ��� glfw extensions
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

		// Fallback : Grede and rank, but ͨ��ѡ���һ����ʽ
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

	// ������Χ
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

		// Ϊ�������ҵ����ʵ��ڴ�����
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
			if (typeFilter & (1 << i) &&  // vertexbuffer
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) // ����
			{
				return i;
			}
		}

		throwRE("failed to find suitable memory type!");
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{

		// 1. ����������,
		VkBufferCreateInfo bufferInfo = {};
		{
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;	//ʹ��bit��ָ�����ʹ��Ŀ��
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // ��ռģʽ�������л�����������������������
		}

		if (vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throwRE("failed to create vertexbuffer");
		}

		// 2. �ڴ�����
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memoryRequirements);

		// 3. �ڴ����
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

		// 4. ������������	
		vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		// ����һ����ʱ���������,���ύ��������
		VkCommandBufferAllocateInfo allocInfo = {};
		{
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = m_commandPool;
			allocInfo.commandBufferCount = 1;
		}

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

		// ���ƻ���������������ύ����
		VkCommandBufferBeginInfo beginInfo = {};
		{
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // ��ϰ��
		}

		// д������
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		{
			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		}
		vkEndCommandBuffer(commandBuffer); // ֹͣ��¼

		// �ύ����
		VkSubmitInfo submitInfo = {};
		{
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;
		}
		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue); // �ȴ������������ ��ʹ�� fence�����Ŷ�������Ĵ������


		// ɾ����ʱ�������
		vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
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
