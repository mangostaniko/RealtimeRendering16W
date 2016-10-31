#version 330 core

in vec2 texCoord;
layout(location = 0) out vec4 outColor;

uniform sampler2D textBitmap; // texture is grayscale thus stored in a single channel (here red channel)
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textBitmap, texCoord).r); // use grayscale of bitmap for text opacity
    outColor = vec4(textColor, 1.0) * sampled; // add color
}
