#ifndef __BASE_SHADER__
#define __BASE_SHADER__

/**
 * Shader loader (Header)
 * @author: Methusael Murmu
 */

#include <base.h>
#include <stdio.h>

#include <string>
#include <sstream>
#include <algorithm>

namespace glf {

namespace shader {
GLuint load(const char* file,
            GLenum shader_type = GL_VERTEX_SHADER);

GLuint* loadSet(const char** files,
                const GLenum* shader_types,
                int shader_count);
}

namespace program {
GLuint linkFromShaders(const GLuint* shaders,
                       int shader_count,
                       bool delete_shaders = true);
}

// Tested for OpenGL 3.3 Core
class Shader {
 public:
    enum SHADER_TYPE { VERTEX, FRAGMENT, GEOMETRY, SHADERS_MAX };

    Shader() { init(); }
    ~Shader() { dispose(false); }

    GLuint getProgram() const { return program; }
    bool isUsable() const { return _status == COMPILED; }

    GLuint load(const char* path, SHADER_TYPE type) {
        if (path == NULL || _status != UNCOMPILED)
            return 0;

        shaders[type] = shader::load(path, gl_shader_type[type]);
        using_shader[type] = true;
        invalidateStatus();

        return shaders[type];
    }

    GLuint compile() {
        if (_status != UNCOMPILED)
            return 0;

        register int i, j = 0;
        GLuint temp_sh[SHADERS_MAX];
        for (i = 0; i < SHADERS_MAX; ++i) {
            if (shaders[i])
                temp_sh[j++] = shaders[i];
        }

        program = program::linkFromShaders(temp_sh, j, true);
        invalidateStatus();

        return  program;
    }

    void use() const {
        if (_status == COMPILED)
            glUseProgram(program);
    }

    void dispose(const bool should_init = true) {
        switch (_status) {
            case COMPILED:
                glDeleteProgram(program);
                break;

            case UNCOMPILED: {
                GLint del_stat; register int i;
                for (i = 0; i < SHADERS_MAX; ++i) {
                    if (shaders[i]) {
                        glGetShaderiv(shaders[i], GL_DELETE_STATUS, &del_stat);
                        if (del_stat)
                            glDeleteShader(shaders[i]);
                    }
                }
            } break;
        }

        if (should_init) init();
    }

 private:
    enum STATUS { UNCOMPILED, COMPILED, ERROR };
    static const GLenum gl_shader_type[SHADERS_MAX];

    STATUS _status;
    GLuint program;
    GLuint shaders[SHADERS_MAX];
    bool using_shader[SHADERS_MAX];

    void invalidateStatus() {
        if (program) {
            _status = COMPILED;
        } else {
            // Check if shaders are valid
            bool err = false; register int i;
            for (i = 0; i < SHADERS_MAX; ++i) {
                err = using_shader[i] && !shaders[i];
                if (err) break;
            }

            _status = err ? ERROR : UNCOMPILED;
        }
    }

    void init() {
        _status = UNCOMPILED;
        std::fill_n(shaders, SHADERS_MAX, 0);
        std::fill_n(using_shader, SHADERS_MAX, false);
    }

    Shader(const Shader& ref) {}
    const Shader& operator=(const Shader& rhs) {}
};

// Shader attributes for vertex shader
enum Attrib_IDs  { ATTR_POS_ID, ATTR_NORM_ID, ATTR_TEX_ID, ATTR_TAN_ID, ATTR_BTAN_ID, ATTRIB_SZ };
enum TextureType { TEX_DIFFUSE, TEX_SPECULAR, TEX_NORMAL, TEX_TYPE_SZ };

static const GLuint MAX_SAMPLER_SZ = 3;

class ShaderTextureInfo {
 public:
    ShaderTextureInfo() {
        textureTypeStr[TEX_DIFFUSE]     = "texture_diffuse";
        textureTypeStr[TEX_SPECULAR]    = "texture_specular";
        textureTypeStr[TEX_NORMAL]      = "texture_normal";

        std::fill_n((GLuint*) texUniformLoc, MAX_SAMPLER_SZ * TEX_TYPE_SZ, 0);
    }

    inline const std::string& getTextureTypeStr(const TextureType _type) const {
        return textureTypeStr[_type];
    }

    inline void setTextureTypeStr(const TextureType _type, const std::string& _str) {
        textureTypeStr[_type] = _str;
    }

    inline GLuint textureUniformLocation(const TextureType _type, const GLuint _idx) const {
        return texUniformLoc[_type][_idx];
    }

    void loadTextureLocations(const Shader& _shader, const TextureType _type, GLuint _count) {
        _count = std::min(_count, MAX_SAMPLER_SZ);

        for (GLuint i = 0; i < _count; ++i) {
            std::ostringstream str;
            str << textureTypeStr[_type] << i;

            texUniformLoc[_type][i] =
                glGetUniformLocation(_shader.getProgram(), str.str().c_str());
        }
    }

 private:
    GLuint texUniformLoc[TEX_TYPE_SZ][MAX_SAMPLER_SZ];
    std::string textureTypeStr[TEX_TYPE_SZ];
};

}  // namespace glf

#endif
