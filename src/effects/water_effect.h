#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader.h"
#include "../texture.hpp"

/// WaterEffect
/// This is used to create a real-time reflecting and refracting
/// horizontal water surface with small rippling waves.
/// The fragment colors of the water surface are mixed from
/// two textures of the scene at different camera angles for reflection and refraction:
/// For the reflection texture we render the scene above the water surface from below, and invert the y sampling.
/// For the refraction texture we render the scene below the water surface from above (maybe at an inclined angle).
/// The texture sampling is distorted to create a rippling effect.
/// The fresnel effect is modelled to attenuate reflection based on viewing angle to surface normal.
class WaterEffect
{

private:

	// smaller texture sizes will lead to blurrier reflection / refraction
	int reflectionResolutionX;
	int reflectionResolutionY;
	int refractionResolutionX;
	int refractionResolutionY;

	GLuint fboReflection, reflectionColorTexture, reflectionDepthBuffer;
	GLuint fboRefraction, refractionColorTexture, refractionDepthTexture;

	Shader *waterShader = nullptr;
	Texture *waterDistortionDuDvMap = nullptr;
	float waveAmplitude, waveSpeed, waveTimeBasedShift;

	int windowWidth, windowHeight;


public:
	/// reflectionResolutionFactor and refractionResolutionFactor will be used to determine
	/// size of reflection/refraction texture as percentage of screen resolution. values should be in [0,1].
	WaterEffect(int windowWidth, int windowHeight, float reflectionResolutionFactor, float refractionResolutionFactor,
	            const std::string& waterDistortionDuDvMapPath, float waveAmplitude, float waveSpeed);
	~WaterEffect();

	inline void bindReflectionFrameBuffer() { bindFrameBuffer(fboReflection, reflectionResolutionX, reflectionResolutionY); }
	inline void bindRefractionFrameBuffer() { bindFrameBuffer(fboRefraction, refractionResolutionX, refractionResolutionY); }

	void bindFrameBuffer(GLuint frameBuffer, int width, int height);
	void bindDefaultFrameBuffer();

	Shader *setupWaterShader();

	void updateWaves(float deltaT);


private:

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

