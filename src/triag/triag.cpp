#include <base_app.h>
#include <base_util.h>
#include <base_shader.h>
#include <math.h>

/**
 * @author: Methusael Murmu
 */

class Triag: public glf::BaseApp {
 public:
    Triag() {
        util::set_color4f1(bg_col, 0.27, 0.35, 0.39);
        util::set_color4f1(fg_col, 0.69, 0.75, 0.77);
    }

    void startup() {
        const int shaderCount = 2;
        const char* shaderFiles[] = {
            "media/triag/shaders/triag.vert",
            "media/triag/shaders/triag.frag"
        };
        const GLenum shaderTypes[] = {
            GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
        };

        glGenVertexArrays(1, &vAO);
        glBindVertexArray(vAO);

        GLfloat vertices[numVerts][2] = {
            { -0.90, -0.90 }, {  0.85, -0.90 },
            { -0.90,  0.85 },
            {  0.90, -0.85 }, {  0.90,  0.90 },
            { -0.85,  0.90 }
        };
        glGenBuffers(1, &bAO);
        glBindBuffer(GL_ARRAY_BUFFER, bAO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);

        const GLuint* shaders = glf::shader::loadSet(
            shaderFiles, shaderTypes, shaderCount);
        renderProgram = glf::program::linkFromShaders(
            shaders, shaderCount);
        glUseProgram(renderProgram);

        glVertexAttribPointer(VERTEX_ATTR, 2, GL_FLOAT,
                              GL_FALSE, 0, BUFFER_OFFSET(0));
        glEnableVertexAttribArray(VERTEX_ATTR);
        glVertexAttrib4fv(COLOR_ATTR, fg_col);

        // Clear temporary objects
        delete[] shaders;
    }

    void shutdown() {
        glDeleteProgram(renderProgram);
        glDeleteBuffers(1, &bAO);
        glDeleteVertexArrays(1, &vAO);
    }

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);

        glBindVertexArray(vAO);
        glDrawArrays(GL_TRIANGLES, 0, numVerts);
        glFlush();
    }

 private:
    GLfloat bg_col[4];  // 0.27, 0.35, 0.39
    GLfloat fg_col[4];  // 0.69, 0.75, 0.77
    GLuint vAO, bAO, renderProgram;

    static const GLuint numVerts = 6;
    enum ATTRIB_ID { VERTEX_ATTR, COLOR_ATTR };
};

DECLARE_MAIN(Triag);
