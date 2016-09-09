#include <base_app.h>
#include <base_util.h>
#include <base_shader.h>
#include <animator/anim.h>
#include <math.h>

/**
 * @author: Methusael Murmu
 */

class Basic: public glf::BaseApp {
 public:
    enum Attrib_ID { POSITION_ID, OFFSET_ID, COLOR_ID };

    Basic() {
        util::set_color4f255(bg_col, 51, 76, 76);
    }

    void setup() {
        info.width = 1366;
        info.height = 768;
        info.flags.fullscreen = 1;
    }

    void startup() {
        // Initiate shader
        shader.load("media/basic/shaders/basic.vert",
            glf::Shader::VERTEX);
        shader.load("media/basic/shaders/basic.frag",
            glf::Shader::FRAGMENT);
        shader.compile(); shader.use();

        // Triangle vertex data
        const GLfloat verts[] = {
            //  Vertex data    |  Color data
             0.0f,  0.2f, 0.0f, 1.0, 0.0f, 0.0f,
            -0.2f, -0.2f, 0.0f, 0.0, 1.0f, 0.0f,
             0.2f, -0.2f, 0.0f, 0.0, 0.0f, 1.0f
        };
        attr_offset[0] = attr_offset[1] = attr_offset[2] = 0;

        // Generate vertex and buffer objects
        glGenVertexArrays(1, &vAO);
        glGenBuffers(1, &vBO);

        // Configure vAO
        glBindVertexArray(vAO);
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // Assign vertex attributes
        glVertexAttribPointer(POSITION_ID, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POSITION_ID);
        glVertexAttribPointer(COLOR_ID, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(COLOR_ID);
        glBindVertexArray(0);  // Unbind vAO;

        // Configure animation data
        x_offset_anim.set(-0.5, 0.5, 1);
        x_offset_anim.setLoopType(anim::LOOP_REVERSE);
        animator.add(&x_offset_anim);

        y_offset_anim.set(-0.5, 0.5, 1);
        y_offset_anim.setLoopType(anim::LOOP_START);
        y_offset_anim.setInterpolator(
            anim::InterpolatorFactory<float>::create(anim::SINE));
        animator.add(&y_offset_anim);
    }

    void shutdown() {
        glDeleteBuffers(1, &vBO);
        glDeleteVertexArrays(1, &vAO);
    }

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);

        animator.animate(duration);
        attr_offset[0] = x_offset_anim.value();
        attr_offset[1] = y_offset_anim.value();

        glBindVertexArray(vAO);
        glVertexAttrib3fv(OFFSET_ID, attr_offset);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }

 private:
    glf::Shader shader;
    GLfloat bg_col[4], attr_offset[3];
    GLuint vAO, vBO;

    // Animation data
    anim::AnimatorRegistry<float> animator;
    anim::PropertyAnimator<float> x_offset_anim;
    anim::PropertyAnimator<float> y_offset_anim;
};

DECLARE_MAIN(Basic);
