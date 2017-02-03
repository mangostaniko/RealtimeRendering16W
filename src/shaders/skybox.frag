#version 450 core

layout(location = 0) out vec4 outColor;

// skybox cubemap is sampled via a 3D direction vector
// that corresponds to the interpolated vertex positions of a cube centered at origin
in vec3 texCoord;

uniform samplerCube skybox;

void main()
{
    outColor = texture(skybox, texCoord);
}
