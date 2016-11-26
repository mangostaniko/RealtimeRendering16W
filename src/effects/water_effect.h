#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader.h"


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

	// smaller texture size will lead to blurrier reflection
	int REFLECTION_WIDTH = 320;
	int REFLECTION_HEIGHT = 180;

	int REFRACTION_WIDTH = 1280;
	int REFRACTION_HEIGHT = 720;

	GLuint fboReflection, reflectionColorTexture, reflectionDepthBuffer;
	GLuint fboRefraction, refractionColorTexture, refractionDepthTexture;

	Shader *waterShader = nullptr;

	int windowWidth, windowHeight;


public:
	WaterEffect(int windowWidth, int windowHeight);
	~WaterEffect();

	inline void bindReflectionFrameBuffer() { bindFrameBuffer(fboReflection, REFLECTION_WIDTH, REFLECTION_HEIGHT); }
	inline void bindRefractionFrameBuffer() { bindFrameBuffer(fboRefraction, REFRACTION_WIDTH, REFRACTION_HEIGHT); }

	void bindFrameBuffer(GLuint frameBuffer, int width, int height);
	void bindDefaultFrameBuffer();

	inline GLuint getReflectionTexture() { return reflectionColorTexture; }
	inline GLuint getRefractionTexture() { return refractionColorTexture; }
	inline GLuint getRefractionDepthTexture() { return refractionDepthTexture; }


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

