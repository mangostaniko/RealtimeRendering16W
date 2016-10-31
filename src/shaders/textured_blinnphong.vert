#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

// these will be interpolated by the gpu
// the interpolated values can be accessed by same name in fragment shader
out vec3 P;
out vec3 N;
out vec2 texCoord;
out vec4 PLightSpace;
out vec4 PViewSpace;

// uniforms use the same value for all vertices
uniform mat4 modelMat;
uniform mat3 normalMat;
uniform mat4 viewProjMat;
uniform mat4 viewMat;
uniform mat4 lightVP;

void main()
{
	gl_Position = viewProjMat * modelMat * vec4(position, 1);

	P = (modelMat * vec4(position, 1)).xyz;
	N = normalMat * normal;
	texCoord = uv;

	PLightSpace = lightVP * vec4(P, 1.0);
	PViewSpace = viewMat * vec4(P, 1.0);

}
