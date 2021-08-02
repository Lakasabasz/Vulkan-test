cmake_minimum_required(VERSION 3.5)

project(vulkan-zabawa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vulkan-zabawa main.cpp
    appvulkancore.h appvulkancore.cpp
    structs.h)


find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} glm::glm)


find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

add_executable(tutorial3 tutorial/tutorial3.cpp)
target_include_directories(tutorial3 PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(tutorial3 Vulkan::Vulkan)
target_link_libraries(tutorial3 glm::glm)
target_link_libraries(tutorial3 glfw)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNDEBUG")
