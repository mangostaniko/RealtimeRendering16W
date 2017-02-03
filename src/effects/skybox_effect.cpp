#include "skybox_effect.h"

// vertex positions defining the skybox cube
// GL_TRIANGLES draw mode, thus 2 triangles (6 vertices) per cube face
static const GLfloat skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

SkyboxEffect::SkyboxEffect(const std::vector<const GLchar *> &cubemapImgPaths)
{

	///////////////////////////////////////
	/// SETUP SKYBOX CUBE VBO AND VAO
	///////////////////////////////////////

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	///////////////////////////////////////
	/// SETUP CUBEMAP AND SKYBOX SHADER
	/// uniforms are not assigned here since they might be updated each frame
	///////////////////////////////////////

	cubeMap = loadCubemap(cubemapImgPaths);
	skyboxShader = new Shader("shaders/skybox.vert", "shaders/skybox.frag");

}

SkyboxEffect::~SkyboxEffect()
{
	glDeleteTextures(1, &cubeMap);
	delete skyboxShader;
}

void SkyboxEffect::drawSkybox(const glm::mat4 &viewMat, const glm::mat4 &projMat)
{
	glDepthMask(GL_FALSE); // turn depth writing off

	skyboxShader->useShader();

	glm::mat4 viewMatNoTranslation = glm::mat4(glm::mat3(viewMat));	// Remove any translation component of the view matrix
	glUniformMatrix4fv(skyboxShader->getUniformLocation("viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatNoTranslation));
	glUniformMatrix4fv(skyboxShader->getUniformLocation("projMat"), 1, GL_FALSE, glm::value_ptr(projMat));

	// bind skybox cube map to texture location 0 of skybox shader
	glUniform1i(skyboxShader->getUniformLocation("skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);

}

GLuint SkyboxEffect::loadCubemap(const std::vector<const GLchar *> &cubemapImgPaths)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// load and assign 6 cubemap face images
	// GL_TEXTURE_CUBE_MAP_POSITIVE_X is an integer enum that can be incremented
	fipImage img;
	for (GLuint i = 0; i < cubemapImgPaths.size(); ++i) {

		// load image from file using FreeImagePlus (the FreeImage C++ wrapper)
		if (!img.load(cubemapImgPaths[i], 0)) {
			std::cerr << "ERROR: FreeImage could not load image file '" << cubemapImgPaths[i] << "'." << std::endl;
		} else {
			std::cout << "loaded cubemap face " << i << ": " << cubemapImgPaths[i] << std::endl;
		}

		// create texture for current cubemap face
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
		             GL_RGB, img.getWidth(), img.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.accessPixels());
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

