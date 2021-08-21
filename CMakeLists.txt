cmake_minimum_required(VERSION 3.5)

project(vulkan-zabawa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vulkan-zabawa main.cpp
    appvulkancore.h appvulkancore.cpp
    aktor.h aktor.cpp
    structs.h
    shader/base.vert shader/base.frag)

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} glm::glm)

find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

find_file(glslc NAME glslc.exe)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${glslc} ${CMAKE_CURRENT_SOURCE_DIR}/shader/base.frag -o ${CMAKE_CURRENT_BINARY_DIR}/shader/base.frag.spv
    )
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${glslc} ${CMAKE_CURRENT_SOURCE_DIR}/shader/base.vert -o ${CMAKE_CURRENT_BINARY_DIR}/shader/base.vert.spv
    )

##########################################################################

add_executable(tutorial3 tutorial/tutorial3.cpp)
target_include_directories(tutorial3 PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(tutorial3 Vulkan::Vulkan)
target_link_libraries(tutorial3 glm::glm)
target_link_libraries(tutorial3 glfw)

add_executable(tutorial4 tutorial/tutorial4.cpp)
target_include_directories(tutorial4 PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(tutorial4 Vulkan::Vulkan)
target_link_libraries(tutorial4 glm::glm)
target_link_libraries(tutorial4 glfw)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNDEBUG")
