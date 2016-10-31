#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <fstream>

/**
 * @brief Shader class.
 * This loads and compiles glsl shader files and creates a linked shader program.
 * The program can later be activated when needed.
 */
class Shader
{

	GLuint vertexHandle;
	GLuint fragmentHandle;

	/**
	 * @brief load and compile glsl shader
	 * @param shader the glsl shader source file
	 * @param shaderType the type of the shader
	 * @param handle the id by which the shader is retrieved within the gl context
	 */
	void loadShader(const std::string& shader, GLenum shaderType, GLuint& handle);

	/**
	 * @brief link compiled shader objects into shader program object
	 */
	void linkShaders();

public:
	Shader(const std::string& vertexShader, const std::string& fragmentShader);
	~Shader();

	GLuint programHandle;

	/**
	 * @brief set this as the active shader program in the current opengl context
	 * only one shader program be active at a time
	 */
	void useShader() const;

};

#endif // SHADER_H
