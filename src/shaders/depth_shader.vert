#version 450 core

layout(location = 0) in vec3 position;

uniform mat4 lightVPMat;
uniform mat4 modelMat;

void main()
{
    gl_Position = lightVPMat * modelMat * vec4(position, 1.0f);
}
