#ifndef APPVULKANCORE_H
#define APPVULKANCORE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "structs.h"

class AppVulkanCore
{
public:
    AppVulkanCore(int height, int width);
    void run();
private:
    int height;
    int width;
    GLFWwindow* window;
    VkInstance vkInstance;
    std::vector<const char*> validationLayers;
    std::vector<const char*> deviceExtensions;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;

    bool checkValidationLayerSupport();
    bool isDevicesSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionsSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    void initWindow();
    void initVulkan();
    void setupDebugSender();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createInstance();
    void mainLoop();
    void cleanup();
};

#endif // APPVULKANCORE_H
