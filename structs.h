#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef GLFW_INCLUDE_VULKAN
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#endif

#include <cstdint>
#include <optional>
#include <vector>

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete(){
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

#endif // STRUCTS_H
