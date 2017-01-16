#version 450 core

// NOTE: THIS IS A POSTPROCESSING SHADER, THUS DRAW A SCREEN FILLING QUAD

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 texCoord;

void main() {

    texCoord = uv;
    gl_Position = vec4(position, 1.0);
}
