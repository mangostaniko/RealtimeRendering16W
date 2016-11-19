#ifndef SUZANNEISLAND_TEXTURE_HPP
#define SUZANNEISLAND_TEXTURE_HPP 1

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImagePlus.h>


//! Texture class.

//! This loads an opengl texture from an image file and stores a handle to it.
//! note: use 8 bit RGB, no alpha channel!
class Texture
{
    GLuint handle;
    const std::string filePath;

public:
	Texture(const std::string &filePath);
    ~Texture();

    enum FilterType {
        NEAREST_MIPMAP_OFF     = 0,
        NEAREST_MIPMAP_NEAREST = 1,
        NEAREST_MIPMAP_LINEAR  = 2,
        LINEAR_MIPMAP_OFF      = 3,
        LINEAR_MIPMAP_NEAREST  = 4,
        LINEAR_MIPMAP_LINEAR   = 5
    };

    //! binds this texture to the given opengl texture unit
    void bind(
        int unit //!< [in] the opengl texture unit to bind to
    );

    /**
     * @brief set texture minification and magnification filters
     * minification:  how to filter downsampled texture when there's not enough space
     * magnification: how to interpolate texture to fill remaining space
     * @param filterType
     */
    void setFilterMode(FilterType filterType);

    //! get the texture file path
    //! \return the texture file path
    std::string getFilePath() const;
};


inline Texture::Texture(const std::string &filePath_)
    : filePath(filePath_)
{
    glGenTextures(1, &handle);
    // select the active texture unit of the context
    glActiveTexture(GL_TEXTURE0);
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
	if (img.isTransparent()) {
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, img.getWidth(), img.getHeight(), 0,
		    GL_BGRA, GL_UNSIGNED_BYTE, img.accessPixels());
		std::cout << "found texture with alpha channel: " << filePath << std::endl;
	} else {
		glTexImage2D(
		    GL_TEXTURE_2D, 0, GL_RGB, img.getWidth(), img.getHeight(), 0,
		    GL_BGR, GL_UNSIGNED_BYTE, img.accessPixels());
	}

    // automatically generate mipmaps (mip = multum in parvo, i.e. 'much in little')
    // mipmaps are filtered and downsampled copies of the texture stored compactly in a single file,
    // used to avoid aliasing effects when the sampling rate is too low for the texture frequency
    // e.g. for far away surfaces. by taking a filtered average it doesnt matter where the sample hits.
    glGenerateMipmap(GL_TEXTURE_2D);

    setFilterMode(LINEAR_MIPMAP_LINEAR);
}

inline Texture::~Texture()
{
    glDeleteTextures(1, &handle);
}

inline void Texture::bind(int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, handle);
}

inline void Texture::setFilterMode(FilterType filterType)
{
    switch (filterType) {
        case NEAREST_MIPMAP_OFF:
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case NEAREST_MIPMAP_NEAREST:
            glTexParameterf(
                GL_TEXTURE_2D,
                GL_TEXTURE_MIN_FILTER,
                GL_NEAREST_MIPMAP_NEAREST);
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case NEAREST_MIPMAP_LINEAR:
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case LINEAR_MIPMAP_OFF:
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case LINEAR_MIPMAP_NEAREST:
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case LINEAR_MIPMAP_LINEAR:
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameterf(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
    }
}

inline std::string Texture::getFilePath() const
{
    return filePath;
}


#endif // SUZANNEISLAND_TEXTURE_HPP
