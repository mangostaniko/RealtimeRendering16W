#version 450 core

// NOTE: THIS IS A POSTPROCESSING SHADER, THUS DRAW A SCREEN FILLING QUAD

layout(location = 0) out vec4 outColor;

in vec2 texCoord; // texCoords of a screen filling quad

uniform sampler2D occludedSkyTexture; // texture of the sky with all objects (occluders) black
uniform vec2 sunPosScreenSpace; // relative screen space [(0,0),(1,1)], not [(0,0),(windowWidth,windowHeight)]
uniform int numSamples; // fixed number of samples between fragment and sun
uniform float sampleDensityBias; // if 1 we reach sun in numSamples. if < 1 sampling will be denser but end before reaching the sun
uniform float sampleWeight; // to adjust sample intensity contribution
uniform float decayFactor; // how much sample contribution decays with distance to fragment
uniform float exposure; // to adjust total intensity contribution

void main() {

    // volumetric scattering makes us see light beams that wouldnt reach us directly
    // due to some light being scattered towards us from each position on the beams.
    // along a light beam, photons may enter towards us via scattering or leave in other directions.
    // usually since our direction is just one of many possible scatter directions,
    // scattering will mostly lead to decay of perceived light intensity with distance from sun.

    // to very roughly simulate volumetric scattering and light beams as a screen space post process
    // we let each fragment have sky color depending on how much non occluded fragments where closer to the sun,
    // since each nonoccluded fragment could scatter to our occluded on which could then scatter towards us,
    // but fragments after occluded fragments away from sun shouldnt be much brighter to roughly simulate volumetric shadow.
    // visually this may look like a radial blur, i.e. a smearing of colors radially outward around the sun:
    // sky colors will smear into occluded regions, occluded regions will smear (shadow) into non occluded regions.

    // thus for each fragment we sample the sky between it and the sun (fixed numSamples independent of distance)
    // if the sky is clear take its sampled color, if it is occluded the sample is black.
    // each fragment receiveds the SUM of all these samples, sample contribution decays based on distance to fragment.
    // color of a point on a light beam thus depends on how much of the lightbeam is occluded up to that point:
    // if all is clear, we get a bright sky color at all points, that will fade due to decay. this will be like a bloom effect around the sun.
    // if we have (sun)---xxxx--------> where x marks occluded parts,
    // then those occluded fragments x will still have some sky color from the nonoccluded fragments before.
    // and the nonoccluded fragments after the occluded ones will have a bit less sky color due to "shadowing".

    // we use texCoord of a screen filling quad to sample occluded sky texture
    // thus fragment positions [(0,0),(windowWidth,windowHeight)] correspond to texCoord [(0,0),(1,1)].
    vec2 deltaTexCoord = vec2(texCoord - sunPosScreenSpace) / float(numSamples); // numSamples between fragment and sun
    deltaTexCoord *= sampleDensityBias; // if 1 we reach sun in numSamples. if < 1 sampling will be denser but end before reaching the sun
    vec2 sampleTexCoord = texCoord;

    vec3 fragColor = vec3(0.0, 0.0, 0.0);
    float illuminationDecay = 1.0;

    for (int i = 0; i < numSamples; ++i) { // fixed number of uniformly distanced samples

        sampleTexCoord -= deltaTexCoord; // move towards the sun
        vec3 sampleColor = texture2D(occludedSkyTexture, sampleTexCoord).rgb; // sky color except for where occluded
        fragColor += sampleColor * sampleWeight * illuminationDecay; // add color contribution
        illuminationDecay *= decayFactor; // color contribution decays with distance to fragment
    }

    fragColor *= exposure;

    // attenuation based on light incident angle (approximated via taxicab distance of light screen space pos from center)
    fragColor *= 1.0 - (abs(0.5 - sunPosScreenSpace.x) + abs(0.5 - sunPosScreenSpace.y));

    outColor = vec4(fragColor, 1.0);
}

