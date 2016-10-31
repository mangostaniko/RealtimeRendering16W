#version 330 core

layout(location = 0) in vec3 position;

out vec4 pos;

uniform mat4 lightVP;
uniform mat4 modelMat;

void main()
{
    gl_Position = lightVP * modelMat * vec4(position, 1.0f);
	pos = gl_Position;
}
