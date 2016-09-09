#include <text/text.h>

#include <algorithm>

/**
 * Provides text rendering services (Implements text.h)
 * @author: Methusael Murmu
 */

namespace text {

/* A basic font set loader (maximum number of characters defined by MAX_CHAR_SZ)
 * Creates an individual struct Char for each character loaded, with separate
 * texture for each.
 * TODO: Create a texture atlas instead (use distance fields perhaps?) */
GLint FontData::loadFonts(const GLchar* font_file, GLint height) {
    if (isValid()) return 1;

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stdout, "Text: Failed to init FreetType\n");
        return 2;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_file, 0, &face)) {
        fprintf(stdout, "Text: Failed to create font face (%s)\n", font_file);
        return 3;
    }

    FT_Set_Pixel_Sizes(face, 0, height);
    FT_GlyphSlot slot = face->glyph;

    maxHeight = 0;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte i = 0; i < MAX_CHAR_SZ; ++i) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stdout, "Text: Failed to load glyph: %c\n", i);
            memset(&charMap[i], 0, sizeof(Char));
            continue;
        }

        // Generate and load textures
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
            slot->bitmap.width, slot->bitmap.rows, 0,
            GL_RED, GL_UNSIGNED_BYTE, slot->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        Char st_char = {
            texId,
            glm::ivec2(slot->bitmap.width, slot->bitmap.rows),
            glm::ivec2(slot->bitmap_left, slot->bitmap_top),
            slot->advance.x
        };

        charMap[i] = st_char;
        maxHeight = std::max(maxHeight, slot->bitmap.rows);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Generate the quads
    glGenVertexArrays(1, &vAO);
    glGenBuffers(1, &vBO);
    glBindVertexArray(vAO);

    glBindBuffer(GL_ARRAY_BUFFER, vBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    valid = true;
}

void FontData::clean() {
    if (!isValid()) return;

    for (GLint i = 0; i < MAX_CHAR_SZ; ++i)
        glDeleteTextures(1, &charMap[i].texId);
    glDeleteBuffers(1, &vBO);
    glDeleteVertexArrays(1, &vAO);

    valid = false;
}

/* Renders a given string on a quad
 * The text rendering shader must be provided by the calling method
 * Not recommended for production purposes (ad-hoc inefficient implementation) */
void FontRenderer::renderText(const glf::Shader* _shader, const std::string& str,
                GLfloat x, GLfloat y, GLfloat scale) {
    if (!initCalled || !font->isValid()) return;

    _shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->vAO);

    Char st_char;
    std::string::const_iterator itr;
    GLfloat xpos, ypos, w, h;

    for (itr = str.begin(); itr != str.end(); ++itr) {
        st_char = font->charMap[*itr];

        // Infer position from metrics
        w = st_char.size.x * scale;
        h = st_char.size.y * scale;
        xpos = x + st_char.bearing.x * scale;
        ypos = y - (st_char.size.y - st_char.bearing.y) * scale;

        // Update quad vertices
        qVert[0][0] = xpos;     qVert[0][1] = ypos + h;
        qVert[1][0] = xpos;     qVert[1][1] = ypos;
        qVert[2][0] = xpos + w; qVert[2][1] = ypos + h;
        qVert[3][0] = xpos + w; qVert[3][1] = ypos;

        glBindTexture(GL_TEXTURE_2D, st_char.texId);
        glBindBuffer(GL_ARRAY_BUFFER, font->vBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(qVert), qVert);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        x += (st_char.advance >> 6) * scale;
    }

    glBindVertexArray(0);
}

const GLchar* const FontRenderer::kTextUniform = "text";
const GLchar* const FontRenderer::kTextColorUniform = "textColor";

}  // namespace text
