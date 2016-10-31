#version 330 core

in vec2 texCoord;
layout(location = 0) out vec4 outColor;

uniform sampler2D ssaoTexture; // the ssao factor for each fragment
uniform bool filterHorizontally; // else filter vertically

float offsets[9] = float[](-4, -3, -2, -1, 0, 1, 2, 3, 4);
float kernel[9] = float[](0.05f, 0.1f, 0.1f, 0.15f, 0.2f, 0.15f, 0.1f, 0.1f, 0.05f);

void main()
{

	float AO = 0.0f;

	// blur the ssao factors using the filter kernel
	// horizontal and vertical filtering separated
	for (int i = 0; i < 9; ++i) {
		vec2 tc = texCoord;

		if (filterHorizontally) {
			tc.x = texCoord.x + offsets[i] / textureSize(ssaoTexture, 0).x;
		} else {
			tc.y = texCoord.y + offsets[i] / textureSize(ssaoTexture, 0).y;
		}

		AO += texture(ssaoTexture, tc).r * kernel[i];
	}

	outColor = vec4(AO, AO, AO, 1);

}
