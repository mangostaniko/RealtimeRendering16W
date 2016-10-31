#version 330 core

layout (location = 0) in vec4 particleQuadVertex; // xy used for position, zw used for uvs
layout (location = 1) in vec4 particleData; // particle center x, y, z + timeToLive [0, 1]

out vec2 texCoord;
out float timeToLive;

// uniforms use the same value for all vertices
uniform mat4 modelViewMat;
uniform mat4 projMat;

void main()
{
    gl_Position = projMat * (modelViewMat * vec4(particleData.xyz, 1) + vec4(particleQuadVertex.xy*2, 0, 1)); // particles face camera
	texCoord = particleQuadVertex.zw;
	timeToLive = particleData.w;
}
