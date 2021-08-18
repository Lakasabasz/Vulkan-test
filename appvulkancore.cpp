#include "appvulkancore.h"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <set>
#include <algorithm>
#include <fstream>

AppVulkanCore::AppVulkanCore(int height, int width)
{
    this->height = height;
    this->width = width;
#ifdef NDEBUG
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    physicalDevice = VK_NULL_HANDLE;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    vertices = {Vertex({0.0f, -0.5, 0}, {1, 0, 0}),
                Vertex({0.5, 0.5, 0}, {0, 1, 0}),
                Vertex({-0.5, 0.5, 0}, {0, 0, 1})};
}

void AppVulkanCore::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

bool AppVulkanCore::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
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

bool AppVulkanCore::isDevicesSuitable(VkPhysicalDevice device)
{
    auto indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionsSupport(device);

    bool swapChainAdque = false;
    if(extensionsSupported){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdque = !(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty());
    }
    return indices.isComplete() && extensionsSupported && swapChainAdque;
}

bool AppVulkanCore::checkDeviceExtensionsSupport(VkPhysicalDevice device)
{
    uint32_t extensionsCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for(const auto& ext : availableExtensions){
        requiredExtensions.erase(ext.extensionName);
    }

    return requiredExtensions.empty();
}

std::vector<const char *> AppVulkanCore::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (validationLayers.size() != 0) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VkBool32 AppVulkanCore::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    if(messageSeverity == VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)  return VK_FALSE;
    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void AppVulkanCore::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void AppVulkanCore::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void AppVulkanCore::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<AppVulkanCore*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

VkResult AppVulkanCore::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
       return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
       return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

QueueFamilyIndices AppVulkanCore::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamiliCounter = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliCounter, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliCounter);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliCounter, queueFamilies.data());

    int i=0;
    for(const auto& queueFamily : queueFamilies){
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(presentSupport) indices.presentFamily = i;

        if(indices.isComplete()) break;
        i++;
    }
    return indices;
}

SwapChainSupportDetails AppVulkanCore::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if(formatCount != 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModes, nullptr);
    if(presentModes != 0){
        details.presentModes.resize(presentModes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModes, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR AppVulkanCore::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for(const auto& avFormat : availableFormats){
        if(avFormat.format == VK_FORMAT_B8G8R8A8_SRGB && avFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return avFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR AppVulkanCore::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availableModes)
{
    for(const auto& avMode : availableModes){
        if(avMode == VK_PRESENT_MODE_MAILBOX_KHR) return avMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D AppVulkanCore::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if(capabilities.currentExtent.width != UINT32_MAX){
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

std::vector<char> AppVulkanCore::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule AppVulkanCore::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create shader module");
    }

    return shaderModule;
}

uint32_t AppVulkanCore::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void AppVulkanCore::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void AppVulkanCore::initVulkan()
{
    createInstance();
    setupDebugSender();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffer();
    createCommandPool();
    createVertexBuffers();
    createCommandBuffers();
    createSyncObjects();
}

void AppVulkanCore::setupDebugSender()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if(CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS){
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void AppVulkanCore::createSurface()
{
    if(glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("Failed to create window surface!");
    }

}

void AppVulkanCore::pickPhysicalDevice()
{
    uint32_t devicesCounter = 0;
    vkEnumeratePhysicalDevices(vkInstance, &devicesCounter, nullptr);
    if(devicesCounter == 0){
        throw std::runtime_error("No Vulkan support devices!");
    }

    std::vector<VkPhysicalDevice> devices(devicesCounter);
    vkEnumeratePhysicalDevices(vkInstance, &devicesCounter, devices.data());

    for(const auto& device : devices){
        if(isDevicesSuitable(device)){
            physicalDevice = device;
            break;
        }
    }

    if(physicalDevice == VK_NULL_HANDLE){
        throw std::runtime_error("No suitable GPU!");
    }
}

void AppVulkanCore::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for(uint32_t queueFamily : uniqueQueueFamilies){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if(validationLayers.size() != 0){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else{
        createInfo.enabledLayerCount = 0;
    }

    if(physicalDevice == nullptr) {
        throw std::runtime_error("Co do kurwy?");
    }
    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

}

void AppVulkanCore::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount){
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else{
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS){
        throw std::runtime_error("Failed to create swap chain");
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainImageExtent = extent;

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
}

void AppVulkanCore::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for(size_t i = 0; i < swapChainImages.size(); i++){
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void AppVulkanCore::createRenderPass()
{
    VkAttachmentDescription colorAttachement{};
    colorAttachement.format = swapChainImageFormat;
    colorAttachement.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachementRef{};
    colorAttachementRef.attachment = 0;
    colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachementRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachement;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS){
        throw std::runtime_error("Failed to create render pass!");
    }
}

void AppVulkanCore::createGraphicsPipeline()
{
    auto vertShaderCode = readFile("shader/base.vert.spv");
    auto fragShaderCode = readFile("shader/base.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto binding = Vertex::getBindingDescription();
    auto attribs = Vertex::getAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &binding;
    vertexInputInfo.vertexAttributeDescriptionCount = attribs.size();
    vertexInputInfo.pVertexAttributeDescriptions = attribs.data();//*/
    /*vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;//*/

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.height = swapChainImageExtent.height;
    viewport.width = swapChainImageExtent.width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainImageExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachement{};
    colorBlendAttachement.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachement.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachement;

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 0;
    dynamicState.pDynamicStates = nullptr;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;

    if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS){
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void AppVulkanCore::createFramebuffer()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for(size_t i = 0; i < swapChainImageViews.size(); i++){
        VkImageView attachements[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachements;
        framebufferInfo.width = swapChainImageExtent.width;
        framebufferInfo.height = swapChainImageExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void AppVulkanCore::createCommandPool()
{
    QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = familyIndices.graphicsFamily.value();
    poolInfo.flags = 0;

    if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create command pool!");
    }
}

void AppVulkanCore::createVertexBuffers()
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0])*vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create vertex buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), bufferInfo.size);
    vkUnmapMemory(device, vertexBufferMemory);

}

void AppVulkanCore::createCommandBuffers()
{
    commandBuffers.resize(swapChainFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBuffers.size();

    if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffer!");
    }

    for(size_t i = 0; i < commandBuffers.size(); i++){
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;

        if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS){
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainImageExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
}

void AppVulkanCore::createSyncObjects()
{
    VkSemaphoreCreateInfo semCreateInfo{};
    semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fencCreateInfo{};
    fencCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fencCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    for(size_t i = 0; i<MAX_FRAMES_IN_FLIGHT; i++){
        if(vkCreateSemaphore(device, &semCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fencCreateInfo, nullptr, &inFlightFences[i])){
            throw std::runtime_error("Failed to create sync objects!");
        }
    }
}

void AppVulkanCore::createInstance()
{
    if(validationLayers.size() != 0 && !checkValidationLayerSupport()){
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Zabawa z Vulkanem";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (validationLayers.size() != 0) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }
    if(vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS){
        throw std::runtime_error("Failde to create instance!");
    }
}

void AppVulkanCore::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while(width == 0 || height == 0){
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffer();
    createCommandBuffers();
}

void AppVulkanCore::mainLoop()
{
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);

}

void AppVulkanCore::cleanup()
{
    for(size_t i = 0; i<MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, commandPool, nullptr);

    for(auto framebuffer : swapChainFramebuffers){
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for(auto imageView : swapChainImageViews){
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    vkDestroyDevice(device, nullptr);

    if(validationLayers.size() != 0){
        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(vkInstance, surface, nullptr);
    vkDestroyInstance(vkInstance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

}

void AppVulkanCore::cleanupSwapChain()
{
    for(size_t i = 0; i < swapChainFramebuffers.size(); i++){
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for(auto imageView : swapChainImageViews){
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void AppVulkanCore::drawFrame()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if(res == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return;
    } else if(res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    if(imagesInFlight[imageIndex] != VK_NULL_HANDLE){
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitForSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitForSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    res = vkQueuePresentKHR(presentQueue, &presentInfo);

    if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || framebufferResized){
        framebufferResized = false;
        recreateSwapChain();
    } else if(res != VK_SUCCESS){
        throw std::runtime_error("Failed to present swap chain image!");
    }

    vkQueueWaitIdle(presentQueue);

    currentFrame++;
    currentFrame %= MAX_FRAMES_IN_FLIGHT;
}
