#version 450 core

layout(location = 0) out vec4 outColor;

in vec3 P;
in vec3 N;
in vec2 texCoord;
in vec4 clipSpacePos;

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

uniform sampler2D reflectionTexture; // texture unit 0
uniform sampler2D refractionTexture; // texture unit 1
uniform sampler2D waterDistortionDuDvMap; // texture unit 2
uniform float waveAmplitude;
uniform float waveTimeBasedShift;

void main()
{

    vec2 distortion1 = texture(waterDistortionDuDvMap, vec2( texCoord.x*10 + waveTimeBasedShift, texCoord.y*10)).rg * waveAmplitude;
    vec2 distortion2 = texture(waterDistortionDuDvMap, vec2(-texCoord.x*10 + waveTimeBasedShift, texCoord.y*10 + waveTimeBasedShift)).rg * waveAmplitude;
    vec2 totalDistortion = distortion1 + distortion2;

    // use screen space position to sample reflection and refraction textures
    // since these are simply textures of the whole screen from different camera position and angle
    vec2 normDeviceCoords = clipSpacePos.xy / clipSpacePos.w; // perspective divide (homogenization after perspective projection)
    vec2 screenSpaceCoords = normDeviceCoords/2 + 0.5; // ndc [-1,1] to screen space [0,1]
    vec2 reflectTexCoords = clamp(screenSpaceCoords + totalDistortion, 0.0001, 0.9999);
    vec2 refractTexCoords = clamp(screenSpaceCoords + totalDistortion, 0.0001, 0.9999);
    vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
    vec4 refractColor = texture(refractionTexture, refractTexCoords);

    vec4 diffuseColor = mix(reflectColor, refractColor, 0.5);
    //diffuseColor = mix(diffuseColor, vec4(0.3, 0.3, 1.0f, 1), 0.2); // add blue tint


    // normalize normal, light and view vectors
    vec3 normal = normalize(N);
    vec3 lightDir = normalize(lightWorldPos.xyz - P);
    vec3 viewDir = normalize(cameraWorldPos.xyz - P);

    // diffuse
    vec4 diffuse = diffuseColor;

    // specular
    vec3 halfVec = normalize(lightDir + viewDir); // half vector of light and view vectors
    vec4 specularColor = vec4(1.0f); //texture(specularTexture, texCoord).rgb;
    vec4 specular = vec4(0); //pow(max(dot(halfVec, normal), 0.0f), 128) * lightSpecular;

    vec4 color = diffuse + specular;

    outColor = vec4(color.rgb, color.a);

}


