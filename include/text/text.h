#ifndef __TEXT_H__
#define __TEXT_H__

/**
 * Provides text rendering services
 * @author: Methusael Murmu
 */

#include <base.h>
#include <base_shader.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <external/glm/vec2.hpp>
#include <external/glm/vec3.hpp>
#include <external/glm/gtc/type_ptr.hpp>

#include <string>

#define MAX_CHAR_SZ 128

namespace text {

// Individual character data
struct Char {
    GLuint      texId;
    glm::ivec2  size;
    glm::ivec2  bearing;
    int64_t     advance;
};

struct FontData {
    GLuint vAO, vBO;
    GLuint maxHeight;
    Char charMap[MAX_CHAR_SZ];

    FontData(): valid(false) {}
    ~FontData() { clean(); }

    GLint loadFonts(const GLchar* font_file, GLint height);
    void clean();

    const bool isValid() const { return valid; }

 private:
    bool valid;
};

// Use this to render fonts on quads using a given FontData
class FontRenderer {
 private:
    glm::vec3 color;
    GLfloat qVert[4][4];

    bool initCalled;

    static const GLchar* const kTextUniform;
    static const GLchar* const kTextColorUniform;

 public:
    FontRenderer(): initCalled(false) {}

    void init(FontData* _font, glf::Shader* _shader) {
        setFont(_font); setColor(glm::vec3(1.0f), _shader);
        _shader->use();
        glUniform1i(glGetUniformLocation(_shader->getProgram(), kTextUniform), 0);

        // Texture coords (inverted on Y axis)
        qVert[0][2] = 0.0; qVert[0][3] = 0.0;
        qVert[1][2] = 0.0; qVert[1][3] = 1.0;
        qVert[2][2] = 1.0; qVert[2][3] = 0.0;
        qVert[3][2] = 1.0; qVert[3][3] = 1.0;

        initCalled = true;
    }

    void setFont(FontData* _font) { font = _font; }
    void setColor(const glm::vec3& _color, const glf::Shader* _shader) {
        color = _color;
        _shader->use();
        glUniform3fv(
            glGetUniformLocation(
                _shader->getProgram(), kTextColorUniform), 1, glm::value_ptr(color));
    }

    void renderText(const glf::Shader* _shader, const std::string& str,
                    GLfloat x, GLfloat y, GLfloat scale);
    FontData* font;
};

}  // namespace text

#endif
