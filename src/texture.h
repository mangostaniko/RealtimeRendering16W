#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImagePlus.h>

#include <iostream>
#include <string>

/**
 * @brief Texture class.
 * This loads an opengl texture from an image file and stores a handle to it.
 * note: use 8 bit RGB, no alpha channel !
 */
class Texture
{

	GLuint handle;
	const std::string filePath;

public:
	Texture(const std::string &filePath, bool alpha);
	~Texture();

	enum FilterType {
		NEAREST_MIPMAP_OFF     = 0,
		NEAREST_MIPMAP_NEAREST = 1,
		NEAREST_MIPMAP_LINEAR  = 2,
		LINEAR_MIPMAP_OFF      = 3,
		LINEAR_MIPMAP_NEAREST  = 4,
		LINEAR_MIPMAP_LINEAR   = 5
	};

	/**
	 * @brief binds this texture to the given opengl texture unit
	 * @param unit the opengl texture unit to bind to
	 */
	void bind(int unit);

	/**
	 * @brief set texture minification and magnification filters
	 * minification:  how to filter downsampled texture when there's not enough space
	 * magnification: how to interpolate texture to fill remaining space
	 * @param filterType
	 */
	void setFilterMode(FilterType filterType);

	/**
	 * @brief get the texture file path
	 * @return the texture file path
	 */
	std::string getFilePath() const;
};

#endif // TEXTURE_H
