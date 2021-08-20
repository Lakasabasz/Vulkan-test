#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef GLFW_INCLUDE_VULKAN
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#endif

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <vector>
#include <array>

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

struct Vertex{
    glm::vec3 pos;
    glm::vec4 color;

    Vertex(glm::vec3 pos, glm::vec3 color){
        this->pos = pos;
        this->color = glm::vec4(color, 1.0);
    }

    Vertex(glm::vec3 pos, glm::vec4 color){
        this->pos = pos;
        this->color = color;
    }

    Vertex(){
        this->pos = glm::vec3(0.0);
        this->color = glm::vec4(1.0);
    }

    static VkVertexInputBindingDescription getBindingDescription(){
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof (Vertex);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDesc;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription(){
        std::array<VkVertexInputAttributeDescription, 2> attribDescs{};
        attribDescs[0].binding = 0;
        attribDescs[0].location = 0;
        attribDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribDescs[0].offset = offsetof(Vertex, pos);
        attribDescs[1].binding = 0;
        attribDescs[1].location = 2;
        attribDescs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribDescs[1].offset = offsetof(Vertex, color);
        return attribDescs;
    }
};

struct UniformBufferObject{
    glm::mat4 scene;
    glm::mat4 camera;
    glm::mat4 proj;
};

#endif // STRUCTS_H
