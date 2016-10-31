#version 330 core

layout (location = 0) in vec4 vertex; // xy used for position, zw used for uvs
out vec2 texCoord;

uniform mat4 projMat;

void main()
{
    gl_Position = projMat * vec4(vertex.xy, 0.0, 1.0);
    texCoord = vertex.zw;
}
