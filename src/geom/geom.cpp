#include <base_app.h>
#include <base_shader.h>
#include <3d/type_common.h>

#include <external/glm/gtc/type_ptr.hpp>

/**
 * @author: Methusael Murmu
 */

class Geometry: public glf::BaseApp {
 public:
    enum Attrib_ID { POSITION_ID, COLOR_ID };

    Geometry() {
        bg_col = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    }

    void setup() {
        info.width = 1366;
        info.height = 768;
        info.flags.fullscreen = 1;
    }

    void startup() {
        // Initiate shader
        shader.load("media/geom/shaders/geom.vert", glf::Shader::VERTEX);
        shader.load("media/geom/shaders/geom.frag", glf::Shader::FRAGMENT);
        shader.load("media/geom/shaders/geom.geom", glf::Shader::GEOMETRY);
        shader.compile(); shader.use();

        // Triangle vertex data
        const GLfloat verts[] = {
            //  Vertex data    |  Color data
             0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 1.0f, 1.0f, 0.0f
        };

        // Generate vertex and buffer objects
        glGenVertexArrays(1, &vAO);
        glGenBuffers(1, &vBO);

        // Configure vAO
        glBindVertexArray(vAO);
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // Assign vertex attributes
        glVertexAttribPointer(POSITION_ID, 2, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POSITION_ID);
        glVertexAttribPointer(COLOR_ID, 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), BUFFER_OFFSET(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(COLOR_ID);
        glBindVertexArray(0);  // Unbind vAO;
    }

    void shutdown() {
        glDeleteBuffers(1, &vBO);
        glDeleteVertexArrays(1, &vAO);
    }

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, glm::value_ptr(bg_col));

        glBindVertexArray(vAO);
        glDrawArrays(GL_POINTS, 0, kVertsCount);
        glBindVertexArray(0);
    }

 private:
    static const GLint kVertsCount = 4;

    glf::Shader shader;
    glm::vec4 bg_col;
    GLuint vAO, vBO;
};

DECLARE_MAIN(Geometry);
