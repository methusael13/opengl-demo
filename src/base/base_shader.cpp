#include <debug.h>
#include <cstdio>

/**
 * Shader loader (Implementation)
 * @author: Methusael Murmu
 */

#include <base_shader.h>

namespace glf {

namespace shader {
extern GLuint load(const char* file, GLenum shader_type) {
    FILE* fp;
    char* data;
    size_t fsize;
    GLint status = 0;
    GLuint result = 0;

    fp = fopen(file, "rb");
    CHECK(fp, file_error);

    // Get file data
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = new char[fsize + 1];
    CHECK(data, bad_alloc);

    fread(data, 1, fsize, fp);
    data[fsize] = 0;
    fclose(fp);

    result = glCreateShader(shader_type);
    CHECK(result, shader_create_error);

    glShaderSource(result, 1, &data, NULL);
    delete[] data;
    glCompileShader(result);

    // Check for errors
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    CHECK(status, compile_error);

    return result;

    compile_error:
        char buffer[4096];
        fprintf(stderr, "Unable to compile %s\n", file);
        glGetShaderInfoLog(result, 4096, NULL, buffer);
        fprintf(stderr, "\x1b[31;1mLog output:\x1b[0m\n%s\n", buffer);
        glDeleteShader(result);
        return 0;
    file_error:
        char err_str[1000];
        snprintf(err_str, sizeof(err_str),
            "Shader file error [%s]", file);
        perror(err_str);
        return 0;
    bad_alloc:
        fprintf(stderr, "Shader allocation error: %s\n", file);
        return 0;
    shader_create_error:
        fprintf(stderr, "Shader create error: %s\n", file);
        return 0;
}

/* Returns an array of compiled shaders
 * The array should be deleted manually */
extern GLuint* loadSet(const char** files,
                const GLenum* shader_types,
                int shader_count) {
    register int i;
    GLuint* shader_set = new GLuint[shader_count];
    CHECK(shader_set, bad_alloc);

    for (i = 0; i < shader_count; ++i)
        shader_set[i] = load(files[i], shader_types[i]);

    return shader_set;
    bad_alloc:
        return NULL;
}
}  // namespace shader

namespace program {
extern GLuint linkFromShaders(const GLuint* shaders,
                       int shader_count,
                       bool delete_shaders) {
    register int i;
    GLuint program = glCreateProgram();

    for (i = 0; i < shader_count; ++i)
        glAttachShader(program, shaders[i]);
    glLinkProgram(program);

    // Check for errors
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
        glDeleteProgram(program);

    if (delete_shaders) {
        for (i = 0; i < shader_count; ++i)
            glDeleteShader(shaders[i]);
    }

    return status ? program : 0;
}
}  // namespace program

using glf::Shader;
const GLenum Shader::gl_shader_type[Shader::SHADERS_MAX] = {
    GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER
};

}  // namespace glf
