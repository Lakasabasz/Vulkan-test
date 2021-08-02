#include "appvulkancore.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

AppVulkanCore::AppVulkanCore(int height, int width)
{
    this->height = height;
    this->width = width;
#ifdef NDEBUG
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    physicalDevice = VK_NULL_HANDLE;
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
    return indices.isComplete();
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
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
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
        if(indices.isComplete()) break;
        i++;
    }
    return indices;
}

void AppVulkanCore::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
}

void AppVulkanCore::initVulkan()
{
    createInstance();
    setupDebugSender();
    pickPhysicalDevice();
}

void AppVulkanCore::setupDebugSender()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if(CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS){
        throw std::runtime_error("Failed to set up debug messenger!");
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

void AppVulkanCore::mainLoop()
{
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
    }

}

void AppVulkanCore::cleanup()
{
    if(validationLayers.size() != 0){
        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
    }

    vkDestroyInstance(vkInstance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

}
