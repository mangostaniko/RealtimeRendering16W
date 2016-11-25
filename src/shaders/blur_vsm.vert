#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 tex;

void main()
{
    gl_Position = vec4(position, 1.0f);
    tex = texCoords;
}