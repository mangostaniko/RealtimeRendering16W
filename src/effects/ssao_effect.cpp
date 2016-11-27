#include "ssao_effect.h"

// vertex positions and uvs defining a quad. used to render the screen texture.
static const GLfloat quadVertices[] = {
    // positions   // uvs
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

SSAOEffect::SSAOEffect(int windowWidth, int windowHeight, int samples_)
    : samples(samples_)
{

    ////////////////////////////////////
    /// SETUP SCREEN FILLING QUAD
    /// setup vao for the screen quad onto which the screen texture will be mapped.
    ////////////////////////////////////

    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    GLint positionAttribIndex   = 0;
    GLint uvsAttribIndex        = 1;
    glEnableVertexAttribArray(positionAttribIndex);
    glEnableVertexAttribArray(uvsAttribIndex);
    glVertexAttribPointer(positionAttribIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribPointer(uvsAttribIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);


    ////////////////////////////////////
    /// SETUP FRAMEBUFFERS
    /// setup framebuffers with texture or renderbuffer objects as attachments
    /// - texture: optimized for later sampling (e.g. for postprocessing)
    /// - renderbuffer object: optimized for use as render target, less flexible
    ////////////////////////////////////

    setupFramebuffers(windowWidth, windowHeight);


    ////////////////////////////////////
    /// SETUP SSAO SHADER
    /// compile shader programs and pass random vectors to ssao shader used for depth sampling.
    /// other uniforms are passed to ssao shader each frame and are not included here.
    ////////////////////////////////////

	ssaoShader = new Shader("shaders/ssao.vert", "shaders/ssao.frag");
	blurShader = new Shader("shaders/blur.vert", "shaders/blur.frag");

    // create array of random vectors for depth sampling in ssao shader
    std::vector<glm::vec3> randomVectors;
    for (GLuint i = 0; i < samples; ++i) {

        glm::vec3 randomVector;
        randomVector.x = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        randomVector.y = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        randomVector.z = 2.0f * (float)rand()/RAND_MAX - 1.0f;

        // scale vectors depending on array index
        // quadratic falloff so that more points lie closer to the origin
        float scale = i / (float)(samples);
        randomVector *= (0.5f + 0.5f * scale * scale);

        randomVectors.push_back(randomVector);
        //std::cout << "x: " << randomVector.x << ", y: " << randomVector.y << ", z: " << randomVector.z << std::endl;
    }

    // use uniform buffer object to pass random vectors to ssao shader for better performance
    glUniformBlockBinding(ssaoShader->programHandle, glGetUniformBlockIndex(ssaoShader->programHandle, "RandomVectors"), 0);
    GLuint uboRandomVectors;
    glGenBuffers(1, &uboRandomVectors);
    glBindBuffer(GL_UNIFORM_BUFFER, uboRandomVectors);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec3) * randomVectors.size(), &randomVectors[0], GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboRandomVectors, 0, sizeof(glm::vec3) * randomVectors.size());

}

SSAOEffect::~SSAOEffect()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fboScreenData);
    glDeleteTextures(1, &screenColorTexture);
    glDeleteTextures(1, &viewPosTexture);
    glDeleteRenderbuffers(1, &screenDepthBuffer);

    glDeleteFramebuffers(1, &fboSSAO);
    glDeleteTextures(1, &ssaoTexture);

    glDeleteBuffers(1, &screenQuadVBO);
    glDeleteVertexArrays(1, &screenQuadVAO);

    delete ssaoShader; ssaoShader = nullptr;
    delete blurShader; blurShader = nullptr;

}

void SSAOEffect::setupFramebuffers(int windowWidth, int windowHeight)
{

    glDeleteFramebuffers(1, &fboScreenData);
    glDeleteTextures(1, &screenColorTexture);
    glDeleteTextures(1, &viewPosTexture);
    glDeleteRenderbuffers(1, &screenDepthBuffer);

    glDeleteFramebuffers(1, &fboSSAO);
    glDeleteTextures(1, &ssaoTexture);


    // generate screen color texture
    // note: GL_NEAREST interpolation is ok since there is no subpixel sampling anyway
    glGenTextures(1, &screenColorTexture);
    glBindTexture(GL_TEXTURE_2D, screenColorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // generate vertex view space position texture
    glGenTextures(1, &viewPosTexture);
    glBindTexture(GL_TEXTURE_2D, viewPosTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_BGR, GL_FLOAT, NULL);

    // generate depth renderbuffer. without this, depth testing wont work.
    // we use a renderbuffer since we wont have to sample this, opengl uses it directly.
    glGenRenderbuffers(1, &screenDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, screenDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);

    // generate framebuffer to attach color texture + view space positions texture + depth renderbuffer
	glGenFramebuffers(1, &fboScreenData); // generate 1 framebuffer object id
	glBindFramebuffer(GL_FRAMEBUFFER, fboScreenData); // first bind initializes the fbo
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, viewPosTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenDepthBuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in SSAOEffect: ScreenData Framebuffer not complete" << std::endl;
    }


    // generate ssao texture
    glGenTextures(1, &ssaoTexture);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);

    // generate framebuffer to attach ssao texture
    glGenFramebuffers(1, &fboSSAO); // generate framebuffer object layout in vram and associate handle
    glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO); // bind fbo to active framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in SSAOEffect: SSAO Framebuffer not complete" << std::endl;
    }

    // fbo for blurred ssao
    glGenFramebuffers(1, &fboSSAOBlurPingpong);
    glGenTextures(1, &ssaoBlurredTexturePingpong);
    glBindFramebuffer(GL_FRAMEBUFFER, fboSSAOBlurPingpong);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurredTexturePingpong);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurredTexturePingpong, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);


    // bind back to default framebuffer (as created by glfw)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void SSAOEffect::bindScreenDataFramebuffer()
{
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fboScreenData);
    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 }; // shader output locations
    glDrawBuffers(2, buffers);
}

void SSAOEffect::calulateSSAOValues(const glm::mat4 &projMat)
{
    ssaoShader->useShader();

	glUniformMatrix4fv(ssaoShader->getUniformLocation("projMat"), 1, GL_FALSE, glm::value_ptr(projMat));

	GLint sampleCountLocation = ssaoShader->getUniformLocation("random_vector_array_size");
    glUniform1i(sampleCountLocation, samples);

	GLint viewPosTexLocation = ssaoShader->getUniformLocation("viewPosTexture");
    glBindFramebuffer(GL_FRAMEBUFFER, fboScreenData);
	glUniform1i(viewPosTexLocation, 0); // bind texture unit 0 to texture location 0 of ssao shader
	glActiveTexture(GL_TEXTURE0 + 0); // activate texture unit 0
    glBindTexture(GL_TEXTURE_2D, viewPosTexture); // bind texture to active texture unit

    glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOEffect::blurSSAOResultTexture()
{
    blurShader->useShader();

    // filter horizontally
    glBindFramebuffer(GL_FRAMEBUFFER, fboSSAOBlurPingpong);
	glUniform1i(blurShader->getUniformLocation("ssaoTexture"), 0); // bind texture unit 0 to texture location 0 of blur shader
	glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glUniform1i(blurShader->getUniformLocation("filterHorizontally"), true);

    drawQuad();

    // filter vertically
    glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);
	glUniform1i(blurShader->getUniformLocation("ssaoTexture"), 0); // bind texture unit 0 to texture location 0 of blur shader
	glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurredTexturePingpong);
	glUniform1i(blurShader->getUniformLocation("filterHorizontally"), false);

    drawQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOEffect::bindSSAOResultTexture(GLint ssaoTexShaderLocation, GLuint textureUnit)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);
    glUniform1i(ssaoTexShaderLocation, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOEffect::drawQuad()
{
    glDisable(GL_DEPTH_TEST); // no need for depth testing since we just draw a single quad
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST); // reenable depth testing
}
