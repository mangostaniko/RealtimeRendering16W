#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader.h"
#include "../texture.hpp"


/// LightbeamsEffect
/// Based on Kenny Mitchell "Volumetric Light Scattering as a Post-Process"
/// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch13.html
///
/// This is used to roughly simulate volumetric scattering as a screen space post process.
/// Volumetric scattering makes us see light beams that wouldnt reach us directly
/// due to some light being scattered towards us from each position on the beams.
/// Along a light beam, photons may enter towards us via scattering or leave in other directions.
/// Usually since our direction is just one of many possible scatter directions,
/// scattering will mostly lead to decay of perceived light intensity with distance from sun.
///
/// To simulate this as a post process, in screen space from the sun outward
/// we let sky colored fragments smear into occluded regions (simulate volumetric scattering),
/// and occluded regions will smear into non occluded regions (simulate volumetric shadow).
/// Visually this may look like a radial blur, i.e. a smearing of colors radially outward around the sun.
///
/// In a PREPASS we render the whole screen to the occludedSkyColorTexture
/// containing the colored sky with all geometry in front of it drawn black.
///
/// In a POSTPASS we render the lightbeams which we can then blend with our default framebuffer.
/// The Lightbeams Shader works as follows:
/// For each fragment we sample the sky between it and the sun (fixed numSamples independent of distance)
/// If a sky sample is clear take its sampled color, if it is occluded the sample is black.
/// Each fragment receives the SUM of all these samples, sample contribution decays based on distance to fragment.
/// Color of a point on a light beam thus depends on how much of the lightbeam is occluded up to that point:
/// If all is clear, we get a bright sky color at all points, that will fade due to decay. this will be like a bloom effect around the sun.
/// If we have (sun)---xxxx--------> where x marks occluded parts,
/// Then those occluded fragments x will still have some sky color from the nonoccluded fragments before.
/// And the nonoccluded fragments after the occluded ones will have a bit less sky color due to "shadowing"
/// (less bright compared to the ones before, in total it will still be brighter as intensity only gets added up).
///
class LightbeamsEffect
{

private:

	int windowWidth, windowHeight;

	int numSamples          = 100;    // fixed number of samples between fragment and sun (higher means better quality).
	float sampleDensityBias = 0.85f; // if 1 we reach sun in numSamples. if < 1 sampling will be denser but end before reaching the sun
	float sampleWeight      = 5.00f; // intensity contribution of each sample (higher means brighter)
	float decayFactor       = 0.98f; // how contribution decays with distance to fragment (lower means shorter light beams)
	float exposure          = 0.0035f; // total intensity contribution (higher means brighter)

	GLuint fboOccludedSky, occludedSkyColorTexture, occludedSkyDepthBuffer;

	Shader *lightbeamsShader = nullptr;

public:

	LightbeamsEffect(int windowWidth, int windowHeight);
	~LightbeamsEffect();

	inline void bindOcclusionFrameBuffer() { bindFrameBuffer(fboOccludedSky, windowWidth, windowHeight); }

	void bindFrameBuffer(GLuint frameBuffer, int width, int height);
	void bindDefaultFrameBuffer();

	Shader *setupLightbeamsShader(const glm::vec3 &sunWorldPos, const glm::mat4 &viewProjMat);


private:

	/// transform a point from world coordinates to relative screen space coordinates
	/// [(0,0),(1,1)] instead of [(0,0),(windowWidth,windowHeight)]
	glm::vec2 worldToRelativeScreenSpace(const glm::vec3 &worldPos, const glm::mat4 &viewProjMat);

	/// create new framebuffer object and return its handle
	GLuint createFrameBuffer();

	/// create texture associated with color attachment 0 of current framebuffer
	GLuint createColorTextureAttachment(int width, int height);

	/// create texture associated with depth attachment of current framebuffer
	/// the depth attachment is used directly by opengl for depth testing
	GLuint createDepthTextureAttachment(int width, int height);

	/// create renderbuffer associated with depth attachment of current framebuffer
	/// the depth attachment is used directly by opengl for depth testing
	/// difference to texture is that renderbuffer cant be sampled (only rendered to) but is more efficient
	GLuint createDepthRenderbufferAttachment(int width, int height);

};


