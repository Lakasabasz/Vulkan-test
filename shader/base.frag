#version 450

layout(location=2) in vec4 vColor;

layout(location=2) out vec4 outColor;

void main() {
    outColor = vColor;
}
