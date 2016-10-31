#include "shader.h"

Shader::Shader(const std::string &vertexShader, const std::string &fragmentShader)
    : vertexHandle(0)
    , fragmentHandle(0)
    , programHandle(0)

{
	programHandle = glCreateProgram();

	if (programHandle == 0) {
		std::cerr << "ERROR in Shader::Shader: Could not create glsl shader program" << std::endl;
		exit(EXIT_FAILURE);
	}


	loadShader(vertexShader, GL_VERTEX_SHADER, vertexHandle);
	loadShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentHandle);

	linkShaders();

}

Shader::~Shader()
{
	glDeleteProgram(programHandle);
	glDeleteShader(fragmentHandle);
	glDeleteShader(vertexHandle);
}

void Shader::loadShader(const std::string &shader, GLenum shaderType, GLuint &handle)
{
	std::ifstream shaderFile(shader);

	if (!shaderFile.good()) {
		std::cerr << "ERROR in Shader::loadShader: Could not read shader file " << shader << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string shaderSource = std::string(std::istreambuf_iterator<char>(shaderFile),
	                                       std::istreambuf_iterator<char>());
	shaderFile.close();

	handle = glCreateShader(shaderType);

	if (handle == 0) {
		std::cerr << "ERROR in Shader::loadShader: Could not read create shader object" << shader << std::endl;
		exit(EXIT_FAILURE);
	}

	const char *shaderSourcePtr = shaderSource.c_str();
	glShaderSource(handle, 1, &shaderSourcePtr, nullptr);
	glCompileShader(handle);

	// print log on failure
	GLint succeeded;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &succeeded); // get int vector (iv) of parameters from shader object
	if (!succeeded) {
		GLint logSize;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize);

		GLchar *msg = new GLchar[logSize];
		glGetShaderInfoLog(handle, logSize, nullptr, msg);

		std::cerr << "GLSL ERROR: " << msg << std::endl;
		delete[] msg;

		exit(EXIT_FAILURE);
	}


}

void Shader::useShader() const
{
	glUseProgram(programHandle);
}

void Shader::linkShaders()
{
	glAttachShader(programHandle, vertexHandle);
	glAttachShader(programHandle, fragmentHandle);
	glLinkProgram(programHandle);

	// print log on failure
	GLint succeeded;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &succeeded);
	if (!succeeded) {
		GLint logSize;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logSize);

		GLchar *msg = new GLchar[logSize];
		glGetProgramInfoLog(programHandle, logSize, nullptr, msg);

		std::cerr << "GLSL ERROR: " << msg << std::endl;
		delete[] msg;

		exit(EXIT_FAILURE);
	}
}
