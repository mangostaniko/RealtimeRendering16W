#version 450 core

layout(location = 0) in vec3 position;

// uniforms shared with other shaders via a Uniform Buffer Object
// note: no need to prepend block name when accessing these uniforms
// std140 is the gpu memory layout for uniform blocks.
// the compiler is not allowed to pack this layout for optimization,
// to ensure it stays the same for all shader programs that use it.
// MAKE SURE TO MAINTAIN UNIFORM BLOCK LAYOUT ACROSS ALL SHADERS FILES!
// THIS LAYOUT ONLY ALLOWS VECTORS TO BE VEC2 OR VEC4 !!
// (VEC3 ARE PADDED, BUT DONT RELY ON GL IMPLEMENTATION FOR IT)
// ORDER OF UNIFORMS WITHIN BLOCK MUST BE CONSISTENT FOR ALL SHADERS !
// block will be bound to binding point 0, make sure to bind UBO to same binding point.
layout(std140, binding = 0) uniform Matrices
{
    //                    // offset   // byte size
    mat4 viewMat;         // 0        // 64 (4*4*4, since 4 byte per float, 4 float per vec, 4 vec per mat)
    mat4 projMat;         // 64       // 64
    //                    // 128 (block total bytes)
};

uniform mat4 modelMat;

void main()
{
    gl_Position = projMat * viewMat * modelMat * vec4(position, 1.0);
}
