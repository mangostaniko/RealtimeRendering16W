#include "texture.h"

Texture::Texture(const std::string &filePath_, bool alpha)
	: filePath(filePath_)
{
	glGenTextures(1, &handle);
	glActiveTexture(GL_TEXTURE0); // select the active texture unit of the context
	glBindTexture(GL_TEXTURE_2D, handle);

	// load image from file using FreeImagePlus (the FreeImage C++ wrapper)
	fipImage img;
	if (!img.load(filePath.c_str(), 0)) {
		std::cerr << "ERROR: FreeImage could not load image file '" << filePath << "'." << std::endl;
	}

	// specify a texture of the active texture unit at given target
	// a unit can contain multiple texture targets, but recommended to use only one per unit
	// parameters: target, mipmap level, internal format, width, heigth, border width, internal format, data format, image data
	// note: for some reason it seems that 8 bit RGB images are really stored in BGR format.
	if (alpha) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getWidth(), img.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img.accessPixels());
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.getWidth(), img.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, img.accessPixels());
	}

	// automatically generate mipmaps (mip = multum in parvo, i.e. 'much in little')
	// mipmaps are filtered and downsampled copies of the texture stored compactly in a single file,
	// used to avoid aliasing effects when the sampling rate is too low for the texture frequency
	// e.g. for far away surfaces. by taking a filtered average it doesnt matter where the sample hits.
	glGenerateMipmap(GL_TEXTURE_2D);

	setFilterMode(LINEAR_MIPMAP_LINEAR);

}

Texture::~Texture()
{
	glDeleteTextures(1, &handle);
}

void Texture::bind(int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::setFilterMode(FilterType filterType)
{
	switch (filterType) {
		case NEAREST_MIPMAP_OFF:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case NEAREST_MIPMAP_NEAREST:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case NEAREST_MIPMAP_LINEAR:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case LINEAR_MIPMAP_OFF:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case LINEAR_MIPMAP_NEAREST:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case LINEAR_MIPMAP_LINEAR:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
	}
}

std::string Texture::getFilePath() const
{
	return filePath;
}

