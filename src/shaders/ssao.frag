#version 330 core

in vec2 texCoord;
layout(location = 0) out vec4 outColor;

const float SAMPLE_RADIUS = 2.5f; // reference uses 1.5 [causes issues, currently not used]

uniform sampler2D viewPosTexture; // interpolated vertex positions in view space
uniform mat4 projMat;
uniform int random_vector_array_size; // reference uses 64 [increase for higher quality]

// we use a uniform buffer object for better performance
layout (std140) uniform RandomVectors
{
    vec3 randomVectors[128]; // array size must be static, so we just allocate as much as we might need
};

void main()
{

    vec3 viewPos = texture(viewPosTexture, texCoord).xyz;

    float AO = 0.0f;

	// sample random points to compare depths around the view space position.
	// the more sampled points lie in front of the actual depth at the sampled position,
	// the higher the probability of the surface point to be occluded.
    for (int i = 0; i < random_vector_array_size; ++i) {

		// take a random sample point.
        vec3 samplePos = viewPos + randomVectors[i];

		// project sample point onto near clipping plane
		// to find the depth value (i.e. actual surface geometry)
		// at the given view space position for which to compare depth
        vec4 offset = vec4(samplePos, 1.0f);
        offset = projMat * offset; // project onto near clipping plane
        offset.xy /= offset.w; // perform perspective divide
        offset.xy = offset.xy * 0.5f + vec2(0.5f); // transform to [0,1] range
        float sampleActualSurfaceDepth = texture(viewPosTexture, offset.xy).z-0.08f;

		// compare depth of random sampled point to actual depth at sampled xy position:
		// the function step(edge, value) returns 1 if value > edge, else 0
		// thus if the random sampled point's depth is greater (lies behind) the actual surface depth at that point,
		// the probability of occlusion increases.
		// NOTE: in the reference, if actual depth at sampled position is too far off
		// from depth at fragment position, it doesnt add to occlusion, to avoid artifacts.
		// however, here this check causes artifacts
		float distance = abs(viewPos.z - sampleActualSurfaceDepth);
		if (distance < SAMPLE_RADIUS) {
			AO += step(sampleActualSurfaceDepth, samplePos.z) * 1.8f;
		}
    }

	// normalize the ratio of sampled points lying behind the surface to a probability in [0,1]
	AO = min(1, AO / float(random_vector_array_size));

	outColor = vec4(AO, AO, AO, 1);
}
