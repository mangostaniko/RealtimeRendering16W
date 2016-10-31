#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <iostream>
#include <string>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.h"

/**
 * @brief holds information defining the glyph (visual representation) of a character.
 */
struct Glyph {
    GLuint textureId;    // glyph opengl texture handle
    glm::ivec2 size;     // glyph width / height (in pixels)
    glm::ivec2 bearing;  // horizontal offset from origin to left of glyph / vertical from baseline to top of glyph (in pixels)
    GLuint advance;      // horizontal offset from current to next glyph origin (in 1/64th pixels)
};

class TextRenderer
{
	Shader *textShader;
	GLuint vao, vbo;

	// stores preloaded glyphs for each character of 7-bit ASCII
	std::map<GLchar, Glyph> glyphs;

	/**
	 * @brief load FreeType glyphs for each character and create opengl textures from glyph bitmaps
	 * the resulting Glyph structs (texture and glyph metrics) are stored in the glyphs map
	 */
	void loadGlyphs(const std::string &fontPath);	

public:
	TextRenderer(const std::string &fontPath, const GLuint &windowWidth, const GLuint &windowHeight);
	~TextRenderer();

	/**
	 * @brief render the given text to the framebuffer
	 * @param text the text string to render
	 * @param x the horizontal position of the left side of the text
	 * @param y the vertical position of the bottom side of the text
	 * @param scaleFactor the factor by which the font is scaled
	 * @param color the color to render the text with
	 */
	void renderText(const std::string &text, GLfloat x, GLfloat y, GLfloat scaleFactor, const glm::vec3 &color);
};

#endif // TEXTRENDERER_H
