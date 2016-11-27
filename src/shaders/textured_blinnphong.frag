#version 450 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outViewSpacePos;

in vec3 P;
in vec3 N;
in vec2 texCoord;
in vec4 PLightSpace;
in vec4 PViewSpace;

// uniforms shared with other shaders via a Uniform Buffer Object
// see vertex shader for details on how to work with UBOs !!
layout(std140, binding = 1) uniform LightAndCamera
{
    //                     // offset   // byte size
    vec4 lightWorldPos;    // 0        // 16 (4*4, since 4 byte per float, 4 float per vec
    vec4 lightAmbient;     // 16       // 16
    vec4 lightDiffuse;     // 32       // 16
    vec4 lightSpecular;    // 48       // 16
    vec4 cameraWorldPos;   // 64       // 16
    //                     // 80 (block total bytes)
};

struct Material {
    sampler2D diffuse; // texture unit 0
    vec3 specular;
    float shininess;
};
uniform Material material;

uniform sampler2D shadowMap; // texture unit 1
uniform sampler2D ssaoTexture; // texture unit 2
uniform bool useShadows;
uniform bool useVSM;
uniform bool useSSAO;
uniform bool drawTransparent;

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

    // Bias to prevent Shadow Acne
    float bias = max(0.005 * (1.0 - dot(normalize(N), normalize(lightWorldPos.xyz - P))), 0.0025);

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

    // if texture has rgb only, alpha is set to 1.
    // if there is no texture, all values are 0.
    vec4 diffuseColor = texture(material.diffuse, texCoord).rgba;

    // for transparent textures (e.g. the palm leaves)
    // since zbuffer would discard fragments of greater depth than the one already found
    // we cant use alpha blending without sorting triangles back to front
    // since we need colors of all fragments for blending with fragments behind transparent one.
    // but for simple binary alpha where we dont need blending
    // we can just discard fragments that have low alpha (i.e. should be transparent)
    // thus the depth buffer wont be set for them and so it doesnt matter if a transparent
    // fragment comes in front of another.
    if (diffuseColor.a < 0.1f){
        discard;
    }


    // BLINN-PHONG ILLUMINATION

    // Normalize normal, light and view vectors
    vec3 normal = normalize(N);
    vec3 lightDir = normalize(lightWorldPos.xyz - P);
    vec3 viewDir = normalize(cameraWorldPos.xyz - P);

    // Diffuse
    vec4 diffuse = max(dot(normal, lightDir), 0.0f) * diffuseColor * lightDiffuse;

    // Ambient
    vec4 ambient = diffuseColor * lightAmbient;

    // Specular
    vec3 halfVec = normalize(lightDir + viewDir); // half vector of light and view vectors
    vec4 specularColor = vec4(1.0f); //texture(specularTexture, texCoord).rgb;
    vec4 specular = pow(max(dot(halfVec, normal), 0.0f), material.shininess) * lightSpecular * vec4(material.specular, 1);


    // APPLY EFFECTS

    float AO = 1;
    if (useSSAO) { AO = texture(ssaoTexture, gl_FragCoord.xy / textureSize(ssaoTexture, 0)).r; }

    vec4 color;
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

    if (drawTransparent)
        color.a = color.a * 0.5;
    outColor = vec4((AO*AO * color).rgb, color.a);

    outViewSpacePos = PViewSpace;
}


