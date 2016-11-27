#version 450 core

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
uniform vec4 clippingPlane;
uniform mat4 lightVPMat;
uniform vec2 useYMirroredCamera; // first value is a bool (> 0 enabled, <= 0 disabled), second value is y mirror position

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

void main()
{

    mat4 camMat = inverse(viewMat);
    // use mirrored camera to draw surface as if reflected from water surface
    // just mirror all camera matrix vectors in y direction, for position mirror and increase y by water surface y.
    if (useYMirroredCamera.x > 0) { // x component is used as boolean
        camMat[0] = vec4(reflect(camMat[0].xyz, vec3(0,1,0)), 0);
        camMat[1] = vec4(reflect(camMat[1].xyz, vec3(0,1,0)), 0);
        camMat[2] = vec4(reflect(camMat[2].xyz, vec3(0,1,0)), 0);
        camMat[3] = vec4(reflect(camMat[3].xyz, vec3(0,1,0)), 1);
        camMat[3].y = camMat[3].y + useYMirroredCamera.y;
    }
    mat4 viewMat = inverse(camMat);


    gl_Position = projMat * viewMat * modelMat * vec4(position, 1);

    // use clipping plane 0 for vertex clipping
    // we define clipping plane as normalized direction vector and distance
    // we check if parallel component (cosine) of vertex is beyond the distance, i.e. dot(vertpos, direction) > d.
    // using negative plane distance in a vec4 (x,y,z,-d) the dot product will be the offset of vertpos distance to plane distance
    // thus simply check dot(vertpos, direction) > 0 to see if we are beyond or before the plane in oriented direction.
    // gl_ClipDistance[0] takes exactly the result of such a dot product clipping all that lies before the plane.
    // HOWEVER it defines the plane pointing towards the origin thus the result must be inverted.
    // the values are then interpolated for the fragment shader.
    gl_ClipDistance[0] = -dot(modelMat * vec4(position, 1), clippingPlane);

    P = (modelMat * vec4(position, 1)).xyz;
    N = normalMat * normal;
    texCoord = uv;

    PLightSpace = lightVPMat * vec4(P, 1.0);
    PViewSpace = viewMat * vec4(P, 1.0);

}
