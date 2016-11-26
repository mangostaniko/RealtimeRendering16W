#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>


/// Shader class.
/// This loads and compiles glsl shader files and creates a linked shader
/// program. The program can later be activated when needed.
class Shader
{
public:

    Shader(const std::string& vertexShader, const std::string& fragmentShader);
    ~Shader();

    GLuint programHandle;

	/// Set this as the active shader program.
	/// Only one shader program can be active at a time in the current opengl context.
    void useShader() const;

	/// Return location of shader uniform
	/// if value not yet cached it is retrieved via glGetUniformLocation
	/// glGetUniformLocation should not be called every frame since it involves expensive string search!
	GLint getUniformLocation(const std::string& name);

private:

	// gl shader uniform locations cached by name
	std::map<std::string, GLint> uniforms;

    GLuint vertexHandle;
    GLuint fragmentHandle;

	/// load and compile glsl shader
    void loadShader(
        const std::string& shader, //!< [in] the glsl shader source file
        GLenum shaderType, //!< [in] the type of glsl shader
        GLuint& handle //!< [in,out] the gl context shader id used for retrieval
    );

	/// link compiled shader objects into shader program object
    void linkShaders();

};

