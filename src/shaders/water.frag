#version 450 core

layout(location = 0) out vec4 outColor;

in vec3 P;
in vec3 N;

// uniforms shared with other shaders via a Uniform Buffer Object
// see vertex shader for details on how to work with UBOs !!
layout(std140, binding = 1) uniform LightAndCamera
{
    //                     // offset   // byte size
    vec4 lightPos;         // 0        // 16 (4*4, since 4 byte per float, 4 float per vec
    vec4 lightAmbient;     // 16       // 16
    vec4 lightDiffuse;     // 32       // 16
    vec4 lightSpecular;    // 48       // 16
    vec4 cameraPos;        // 64       // 16
    //                     // 80 (block total bytes)
};

void main()
{
    // normalize normal, light and view vectors
    vec3 normal = normalize(N);
    vec3 lightDir = normalize(lightPos.xyz - P);
    vec3 viewDir = normalize(cameraPos.xyz - P);

    vec4 diffuseColor = vec4(0.7, 0.7, 1, 1);

    // diffuse
    vec4 diffuse = max(dot(normal, lightDir), 0.0f) * diffuseColor * lightDiffuse;

    // ambient
    vec4 ambient = diffuseColor * lightAmbient;

    // specular
    vec3 halfVec = normalize(lightDir + viewDir); // half vector of light and view vectors
    vec4 specularColor = vec4(1.0f); //texture(specularTexture, texCoord).rgb;
    vec4 specular = pow(max(dot(halfVec, normal), 0.0f), 128) * lightSpecular;

    vec4 color = ambient + diffuse + specular;

    outColor = vec4(color.rgb, color.a);

}


