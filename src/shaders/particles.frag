#version 450 core

layout(location = 0) out vec4 outColor;

in vec2 texCoord;
in float timeToLive; // used to fade alpha or change color

uniform sampler2D particleTexture;
uniform vec3 color;

void main()
{
    outColor = texture(particleTexture, texCoord).rgba * vec4(color*timeToLive, timeToLive);
}
