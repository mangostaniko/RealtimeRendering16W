#version 330 core

layout(location = 0) out vec4 outColor;

// these are vertex shader variables interpolated by the gpu
// normals are taken from normalsTexture and put into vertex tangent space
in vec3 P; // world space position
in vec2 texCoord; // interpolated texture coordinates
in mat3 TBN; // the tangent space of the given vertex (tangent, bitangent, normal)

uniform vec3 cameraPos;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

uniform vec3 lightPos;
uniform vec3 lightColor;

const float ambientFactor = 0.2f;

// TODO: these should be made uniforms and defined by object materials
const float shininess = 16.0f;

void main()
{
	vec3 N = normalize(TBN * (texture(normalTexture, texCoord).rgb * 2 - 1));
	vec3 L = normalize(lightPos - P); // light vector (point to light)
	vec3 V = normalize(cameraPos - P);

	vec3 diffuseColor = texture(diffuseTexture, texCoord).rgb;
	vec3 specularColor = vec3(1.0f); //texture(specularTexture, texCoord).rgb;

	vec3 ambient = ambientFactor * lightColor * diffuseColor;
	vec3 diffuse = max(dot(N, L), 0.0f) * lightColor * diffuseColor;

	vec3 H = normalize(L + V); // half vector of light and view vectors

	// note that N is the half vector of light vector and its specular reflection
	vec3 specular = pow(max(dot(H, N), 0.0f), shininess) * lightColor * diffuseColor;

	outColor = vec4(ambient + diffuse + specular, 1);
}

