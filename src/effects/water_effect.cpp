#include "water_effect.h"

WaterEffect::WaterEffect(int windowWidth, int windowHeight, float reflectionResolutionFactor, float refractionResolutionFactor,
                         const std::string& waterDistortionDuDvMapPath)
    : windowWidth(windowWidth)
    , windowHeight(windowHeight)
{

	REFLECTION_RESOLUTION_X = windowWidth * reflectionResolutionFactor;
	REFLECTION_RESOLUTION_Y = windowHeight * reflectionResolutionFactor;
	REFRACTION_RESOLUTION_X = windowWidth * refractionResolutionFactor;
	REFRACTION_RESOLUTION_Y = windowHeight * refractionResolutionFactor;

	////////////////////////////////////
	/// SETUP FRAMEBUFFERS
	/// setup framebuffers with color and depth attachments.
	/// the depth attachment is used directly by opengl for depth testing,
	/// only reason to use renderbuffer instead of texture is that it is more efficient
	/// but we can do no sampling, we just render to it (write-only)
	////////////////////////////////////

	fboReflection = createFrameBuffer();
	reflectionColorTexture = createColorTextureAttachment(REFLECTION_RESOLUTION_X, REFLECTION_RESOLUTION_Y);
	reflectionDepthBuffer = createDepthRenderbufferAttachment(REFLECTION_RESOLUTION_X, REFLECTION_RESOLUTION_Y);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR in WaterEffect: Reflection Framebuffer not complete" << std::endl;
	bindDefaultFrameBuffer();

	fboRefraction = createFrameBuffer();
	refractionColorTexture = createColorTextureAttachment(REFRACTION_RESOLUTION_X, REFRACTION_RESOLUTION_Y);
	refractionDepthTexture = createDepthTextureAttachment(REFRACTION_RESOLUTION_X, REFRACTION_RESOLUTION_Y);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR in WaterEffect: Refraction Framebuffer not complete" << std::endl;
	bindDefaultFrameBuffer();

	////////////////////////////////////
	/// SETUP WATER SHADERS
	/// uniforms are not assigned here since they are updated each frame
	////////////////////////////////////

	waterShader = new Shader("shaders/water.vert", "shaders/water.frag");

	waterDistortionDuDvMap = new Texture(waterDistortionDuDvMapPath);
	std::cout << "loaded texture: " << waterDistortionDuDvMapPath << std::endl;

}

WaterEffect::~WaterEffect()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fboReflection);
	glDeleteTextures(1, &reflectionColorTexture);
	glDeleteRenderbuffers(1, &reflectionDepthBuffer);

	glDeleteFramebuffers(1, &fboRefraction);
	glDeleteTextures(1, &refractionColorTexture);
	glDeleteRenderbuffers(1, &refractionDepthTexture);

	delete waterShader; waterShader = nullptr;

}

void WaterEffect::bindFrameBuffer(GLuint frameBuffer, int width, int height)
{
	glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0); // unbind any texture that might be bound
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);
}

void WaterEffect::bindDefaultFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

Shader *WaterEffect::setupWaterShader()
{
	waterShader->useShader();

	// bind reflection texture to texture location 0 of water shader
	glBindFramebuffer(GL_FRAMEBUFFER, fboReflection);
	glUniform1i(waterShader->getUniformLocation("reflectionTexture"), 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, reflectionColorTexture);

	// bind refraction texture to texture location 1 of water shader
	glBindFramebuffer(GL_FRAMEBUFFER, fboRefraction);
	glUniform1i(waterShader->getUniformLocation("refractionTexture"), 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, refractionColorTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// bind water distortion du/dv map to texture location 2 of water shader
	glUniform1i(waterShader->getUniformLocation("waterDistortionDuDvMap"), 2);
	waterDistortionDuDvMap->bind(2);

	return waterShader;
}

GLuint WaterEffect::createFrameBuffer()
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo); // generate 1 framebuffer object and get its id
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); // first bind to context initializes fbo state
	// fragment output location 0 will draw to color attachment 0 of current framebuffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	return fbo; // return the fbo id
}

GLuint WaterEffect::createColorTextureAttachment(int width, int height)
{
	GLuint texture;
	glGenTextures(1, &texture); // generate texture object and get its id (object state not yet initialized)
	glBindTexture(GL_TEXTURE_2D, texture); // first bind to context initializes object state
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // define color texture format
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // bilinear filtering when oversampled/close (no mipmapping)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // bilinear filtering when undersampled/far (no mipmapping)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0); // associate texture with color attachment of framebuffer

	return texture;
}

GLuint WaterEffect::createDepthTextureAttachment(int width, int height)
{
	// the depth attachment is used by opengl for depth testing.
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); // depth texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0); // associate with depth attachment of framebuffer

	return texture;
}

GLuint WaterEffect::createDepthRenderbufferAttachment(int width, int height)
{
	// the depth attachment is used by opengl for depth testing.
	// only reason to use renderbuffer instead of texture is that it is more efficient
	// but we can do no sampling, we just render to it.
	GLuint depthRenderbuffer;
	glGenRenderbuffers(1, &depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer); // associate with depth attachment of framebuffer

	return depthRenderbuffer;
}

