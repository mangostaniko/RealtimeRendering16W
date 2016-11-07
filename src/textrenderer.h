#ifndef SUZANNEISLAND_TEXTRENDERER_HPP
#define SUZANNEISLAND_TEXTRENDERER_HPP 1

#include <iostream>
#include <string>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.h"


//! Holds information defining the glyph (visual representation) of a character.
struct Glyph {
    GLuint textureId; //!< opengl texture handle
    glm::ivec2 size; //!< width/height (in pixels)
    glm::ivec2 bearing; //!< horizontal offset from origin to left of glyph / vertical from baseline to top of glyph (in pixels)
    GLuint advance; //!< horizontal offset from current to next glyph origin (in 1/64th pixels)
};


class TextRenderer
{
public:
    TextRenderer(const std::string &fontPath, const GLuint &windowWidth, const GLuint &windowHeight);
    ~TextRenderer();

    //! Render the given text to the framebuffer
    void renderText(
        const std::string &text, //!< [in] text string to render
        GLfloat x, //!< [in] left side horizontal position of the text
        GLfloat y, //!< [in] bottom side vertical position of the text
        GLfloat scaleFactor, //!< [in] the factor by which the font is scaled
        const glm::vec3 &color //!< [in] the color to render the text with
    );
private:
    Shader *textShader;
    GLuint vao, vbo;

    // stores preloaded glyphs for each character of 7-bit ASCII
    std::map<GLchar, Glyph> glyphs;

    // Load FreeType glyphs for each character and create opengl textures
    // from glyph bitmaps. The resulting Glyph structs (texture and glyph
    // metrics) are stored in the glyphs map.
    void loadGlyphs(const std::string &fontPath);
};


#endif // SUZANNEISLAND_TEXTRENDERER_HPP
