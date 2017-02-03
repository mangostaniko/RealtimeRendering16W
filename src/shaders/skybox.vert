#version 450 core

layout(location = 0) in vec3 position; // cube vertex positions

// these will be interpolated by the gpu
// the interpolated values can be accessed by same name in fragment shader
// skybox cubemap is sampled via a 3D direction vector
// that corresponds to the interpolated vertex positions of a cube centered at origin
out vec3 texCoord;

uniform mat4 viewMat;
uniform mat4 projMat;

void main()
{
    gl_Position = projMat * viewMat * vec4(position, 1.0);

    // the 3D direction vector to sample the cubemap corresponds to
    // the interpolated vertex positions of a cube centered at origin
    texCoord = position;
}
