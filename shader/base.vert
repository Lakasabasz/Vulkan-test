#version 450

layout(location=0) in vec3 position;
layout(location=2) in vec4 color;

layout(location=2) out vec4 vColor;

void main()
{
    gl_Position = vec4(position, 1.0);
    vColor = color;
}
