#include "lightbeams_effect.h"

LightbeamsEffect::LightbeamsEffect(int windowWidth, int windowHeight)
    : windowWidth(windowWidth)
    , windowHeight(windowHeight)
{

	////////////////////////////////////
	/// SETUP PREPASS FRAMEBUFFER
	/// setup framebuffers with color texture attachment.
	/// depth attachment is not needed since all geometry is drawn black in prepass.
	////////////////////////////////////

	fboOccludedSky = createFrameBuffer();
	occludedSkyColorTexture = createColorTextureAttachment(windowWidth, windowHeight);
	occludedSkyDepthBuffer = createDepthRenderbufferAttachment(windowWidth, windowHeight);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR in LightbeamsEffect: Occlusion Framebuffer not complete" << std::endl;
	bindDefaultFrameBuffer();

	////////////////////////////////////
	/// LIGHTBEAMS SHADER
	////////////////////////////////////

	lightbeamsShader = new Shader("shaders/lightbeams.vert", "shaders/lightbeams.frag");

}

LightbeamsEffect::~LightbeamsEffect()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fboOccludedSky);
	glDeleteTextures(1, &occludedSkyColorTexture);
	glDeleteRenderbuffers(1, &occludedSkyDepthBuffer);

	delete lightbeamsShader;
}

void LightbeamsEffect::bindFrameBuffer(GLuint frameBuffer, int width, int height)
{
	glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0); // unbind any texture that might be bound
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);
}

void LightbeamsEffect::bindDefaultFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

Shader *LightbeamsEffect::setupLightbeamsShader(const glm::vec3 &sunWorldPos, const glm::mat4 &viewProjMat)
{
	lightbeamsShader->useShader();

	// bind occluded sky texture to texture location 0 of water shader
	glBindFramebuffer(GL_FRAMEBUFFER, fboOccludedSky);
	glUniform1i(lightbeamsShader->getUniformLocation("occludedSkyTexture"), 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, occludedSkyColorTexture);
	bindDefaultFrameBuffer();

	// other uniforms
	glm::vec2 sunScreenSpacePos = worldToRelativeScreenSpace(sunWorldPos, viewProjMat);
	glUniform2f(lightbeamsShader->getUniformLocation("sunPosScreenSpace"), sunScreenSpacePos.x, sunScreenSpacePos.y);
	glUniform1f(lightbeamsShader->getUniformLocation("sampleDensityBias"), sampleDensityBias);
	glUniform1f(lightbeamsShader->getUniformLocation("sampleWeight"), sampleWeight);
	glUniform1f(lightbeamsShader->getUniformLocation("decayFactor"), decayFactor);
	glUniform1f(lightbeamsShader->getUniformLocation("exposure"), exposure);
	glUniform1i(lightbeamsShader->getUniformLocation("numSamples"), numSamples);

	return lightbeamsShader;
}

glm::vec2 LightbeamsEffect::worldToRelativeScreenSpace(const glm::vec3 &worldPos, const glm::mat4 &viewProjMat)
{
	// view and projective transform to clip space (still orthographic)
	glm::vec4 clipSpacePos = viewProjMat * glm::vec4(worldPos, 1.0f);

	// ignore positions behind the camera
	// if behind set to (0,0) screen space position since positions at screen edges wont be drawn
	if (clipSpacePos.z < 0.0f)
		return glm::vec2(0.0f, 0.0f);

	// perspective divide (homogenization after perspective projection) to scale points towards center with greater distant
	glm::vec3 normalizedDeviceCoords = glm::vec3(clipSpacePos.xyz) / clipSpacePos.w;

	// transform ndc to relative screen space by mapping range [-1,1] to [0,1]
	glm::vec2 screenSpacePos = (normalizedDeviceCoords.xy + glm::vec2(1.0f, 1.0f)) * 0.5f;

	return screenSpacePos;
}

GLuint LightbeamsEffect::createFrameBuffer()
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo); // generate 1 framebuffer object and get its id
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); // first bind to context initializes fbo state
	// fragment output location 0 will draw to color attachment 0 of current framebuffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	return fbo; // return the fbo id
}

GLuint LightbeamsEffect::createColorTextureAttachment(int width, int height)
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

GLuint LightbeamsEffect::createDepthTextureAttachment(int width, int height)
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

GLuint LightbeamsEffect::createDepthRenderbufferAttachment(int width, int height)
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

