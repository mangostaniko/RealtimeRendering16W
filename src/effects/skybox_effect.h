#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FreeImagePlus.h>

#include "../shader.h"

/// SkyboxEffect
/// This is used to TODO
class SkyboxEffect
{

private:

	GLuint skyboxVAO, skyboxVBO; // skybox cube geometry
	GLuint cubeMap; // cubemap consisting of 6 texture faces that are sampled via a direction vector
	Shader *skyboxShader = nullptr;

public:

	SkyboxEffect(const std::vector<const GLchar *> &cubemapImgPaths);
	~SkyboxEffect();

	void drawSkybox(const glm::mat4 &viewMat, const glm::mat4 &projMat);

private:

	/// load cubemap texture from 6 individual image files
	/// image paths should be given in following order:
	/// +X (right)
	/// -X (left)
	/// +Y (top)
	/// -Y (bottom)
	/// +Z (front)
	/// -Z (back)
	GLuint loadCubemap(const std::vector<const GLchar *> &cubemapImgPaths);

};

