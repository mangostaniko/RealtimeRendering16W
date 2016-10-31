#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outViewSpacePos;

struct Material {
	sampler2D diffuse; // texture unit 0
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 P;
in vec3 N;
in vec2 texCoord;
in vec4 PLightSpace;
in vec4 PViewSpace;

uniform vec3 cameraPos;
uniform Material material;
uniform Light light;

uniform sampler2D shadowMap; // texture unit 1
uniform sampler2D ssaoTexture; // texture unit 2
uniform bool useShadows;
uniform bool useVSM;
uniform bool useSSAO;
uniform bool useAlpha;

float calcShadow(vec4 lightSpacePos)
{
	vec3 projC = lightSpacePos.xyz / lightSpacePos.w;
	float currentZ = projC.z;
	projC = projC * 0.5 + 0.5;

	float closestZ = texture(shadowMap, projC.xy).r;

	// we are outside the far plane, don't waste computation time
	if (projC.z > 1.0) {
		return 0.0;
	}

	// Bias for Shadow Acne
	float bias = max(0.005 * (1.0 - dot(normalize(N), normalize(light.position - P))), 0.0025);

	//float shadow = currentZ - bias > closestZ ? 1.0 : 0.0;

	// PCF for softer shadows
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float pcfZ = texture(shadowMap, projC.xy + vec2(x,y) * texelSize).r;
			shadow += currentZ - bias > pcfZ ? 1.0 : 0.0;
		}
	}
	shadow /= 9;

	return shadow;
}


// Calculate amount of Shadow using Variance Shadow Mapping
float shadowVSM(vec4 lightSpacePos)
{
	vec3 projC = lightSpacePos.xyz / lightSpacePos.w;
	float dist = projC.z;
	
	projC = projC * 0.5 + 0.5;
	
	// retrieve depth and depth squared
	vec2 moments = texture2D(shadowMap,projC.xy).rg;
		
	// no shadow -> fully lit
	if (dist <= moments.x)
		return 1.0;
	
	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, 0.00008f);
	
	float d = dist - moments.x;
	float pMax = variance / (variance + d * d);
	float amount = 0.5f;
	pMax = clamp((pMax - amount)/(1.0f - amount), 0.0f, 1.0f);

	return pMax;
}


void main()
{
	// Normalize normal, light and view vectors
	vec3 normal = normalize(N);
	vec3 lightDir = normalize(light.position - P);
	vec3 viewDir = normalize(cameraPos - P);

	// Ambient
	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb;

	// Diffuse
	vec3 diffuseColor = texture(material.diffuse, texCoord).rgb;
	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * diffuseColor * light.diffuse;

	// Specular
	vec3 halfVec = normalize(lightDir + viewDir); // half vector of light and view vectors
	vec3 specularColor = vec3(1.0f); //texture(specularTexture, texCoord).rgb;
	vec3 specular = pow(max(dot(halfVec, normal), 0.0f), material.shininess) * light.specular * material.specular;

	float AO = 1;
	if (useSSAO) { AO = texture(ssaoTexture, gl_FragCoord.xy / textureSize(ssaoTexture, 0)).r; }

	vec3 color;
	if (useShadows) {
		if (useVSM) {
			float shadow = shadowVSM(PLightSpace);
			color = ambient + (shadow) * (diffuse + specular);
		} 
		else {
			float shadow = calcShadow(PLightSpace);
			color = ambient + (1.0 - shadow) * (diffuse + specular);
		}
		
	} else {
		color = ambient + diffuse + specular;
	}

	if (useAlpha) {
		outColor = vec4(AO*AO * color, 0.5);
	} else {
		outColor = vec4(AO*AO * color, 1);
	}
	
	outViewSpacePos = PViewSpace;
}


