#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

// these will be interpolated by the gpu
// the interpolated values can be accessed by same name in fragment shader
out vec3 P;
out vec2 texCoord;
out mat3 TBN; // the tangent space of the given vertex (tangent, bitangent, normal)

// uniforms use the same value for all vertices
uniform mat4 modelMat;
uniform mat3 normalMat;
uniform mat4 viewProjMat;

mat3 approximateTangentSpace(vec3 normal);

void main()
{
	P = (modelMat * vec4(position, 1)).xyz;
	texCoord = uv;

	TBN = approximateTangentSpace(normalMat * normal);

	gl_Position = viewProjMat * modelMat * vec4(position, 1);
}

mat3 approximateTangentSpace(vec3 normal)
{
	// 1. get the dominant base vector
	vec3 absnormal = abs(normal);
	float maxComponent = max(absnormal.x, max(absnormal.y, absnormal.z));
	// step evaluates componentwise to 1 if >= maxComponent else 0.
	// since only one component can be equal to the maximum component, we just get the dominant base vector
	// we multiply by normal again just in case it was negative.
	vec3 dominantBase = normal * step(maxComponent, absnormal);

	// 2. we cross the normal with its dominant base vector to increase the accuracy of the tangent estimation
	vec3 tangent = normalize(cross(normal, dominantBase));

	// 3. now we just use the normal and tangent to get the bitangent
	vec3 bitangent = normalize(cross(normal, tangent));

	return mat3(tangent, bitangent, normal);
}
