#version 450 core

layout(location = 0) in vec3 position;

out vec4 pos;

uniform mat4 modelMat;
uniform mat4 lightVPMat;

void main()
{
    gl_Position = lightVPMat * modelMat * vec4(position, 1.0f);
    pos = gl_Position;
}
