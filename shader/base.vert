#version 450

layout(location=0) in vec3 position;
layout(location=2) in vec4 color;

layout(location=2) out vec4 vColor;

layout(binding=0) uniform UniformBufferObject{
    mat4 scene;
    mat4 camera;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * ubo.camera * ubo.scene * vec4(position, 1.0);
    vColor = color;
}
