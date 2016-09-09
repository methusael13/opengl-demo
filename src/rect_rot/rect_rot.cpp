#include <base_app.h>
#include <base_util.h>
#include <base_shader.h>
#include <animator/anim.h>

#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/matrix_transform.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/soil/SOIL.h>

#include <math.h>

/**
 * @author: Methusael Murmu
 */

class RectRotate: public glf::BaseApp {
 public:
    RectRotate() {
        util::set_color4f255(bg_col, 38, 38, 38);
        tex_mix = 0.2f;
    }

    void setup() {
        setTitle("Rect Rotate");
    }

    void startup() {
        // Initiate shader
        shader.load("media/rect_rot/shaders/rect_rot.vert",
            glf::Shader::VERTEX);
        shader.load("media/rect_rot/shaders/rect_rot.frag",
            glf::Shader::FRAGMENT);
        shader.compile(); shader.use();

        // Rectangle vertex data
        const float p = 0.3f;
        const GLfloat verts[] = {
            // Verts    Texture
            -p,  p, 0,   0, 1,
             p,  p, 0,   1, 1,
             p, -p, 0,   1, 0,
            -p, -p, 0,   0, 0
        };
        const GLuint indices[idx_sz] = { 0, 1, 2, 0, 3, 2 };

        // Generate vertex and buffer objects
        glGenVertexArrays(1, &vAO);
        glGenBuffers(BO_SZ, buf_objs);

        // Configure vAO
        glBindVertexArray(vAO);
        // Data buffers
        glBindBuffer(GL_ARRAY_BUFFER, buf_objs[vBO]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_objs[eBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof(indices), indices, GL_STATIC_DRAW);

        // Assign vertex attributes
        glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POS_ID);
        glVertexAttribPointer(TEX_COORD_ID, 2, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(TEX_COORD_ID);
        glBindVertexArray(0);  // Unbind vAO;

        // Load textures
        const char* image_path[tex_sz] = {
            "media/rect_rot/tex/container.jpg",
            "media/rect_rot/tex/awesomeface.png"
        };
        glGenTextures(tex_sz, tex);
        for (int i = 0; i < tex_sz; ++i) {
            glBindTexture(GL_TEXTURE_2D, tex[i]);
            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            int width, height;
            unsigned char* image = SOIL_load_image(
                image_path[i], &width, &height, 0, SOIL_LOAD_RGB);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            glGenerateMipmap(GL_TEXTURE_2D);
            SOIL_free_image_data(image);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Bind textures to texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex[0]);
        glUniform1i(glGetUniformLocation(shader.getProgram(), "texture0"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex[1]);
        glUniform1i(glGetUniformLocation(shader.getProgram(), "texture1"), 1);

        // Configure animation data
        x_offset_anim.set(-0.5, 0.5, 5);
        x_offset_anim.setLoopType(anim::LOOP_REVERSE);
        animator.add(&x_offset_anim);

        y_offset_anim.set(-0.5, 0.5, 10);
        y_offset_anim.setLoopType(anim::LOOP_START);
        y_offset_anim.setInterpolator(
            anim::InterpolatorFactory<float>::create(anim::SINE));
        animator.add(&y_offset_anim);

        // Configure transformations
        transform_loc = glGetUniformLocation(shader.getProgram(), "transform");
        rot_axis.x = 0.0f; rot_axis.y = 0.0f; rot_axis.z = 1.0f;
        offset.z = 0.0f;
    }

    void shutdown() {
        glDeleteTextures(tex_sz, tex);
        glDeleteBuffers(BO_SZ, buf_objs);
        glDeleteVertexArrays(1, &vAO);
    }

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);

        // Animate translation offsets
        animator.animate(duration);
        offset.x = x_offset_anim.value();
        offset.y = y_offset_anim.value();

        // Calculate transformations
        glm::mat4 trans;
        trans = glm::translate(trans, offset);
        trans = glm::rotate(trans, (GLfloat) elapsedTime, rot_axis);
        glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(trans));

        glBindVertexArray(vAO);
        glVertexAttrib1f(TEX_MIX_ID, tex_mix);
        glDrawElements(GL_TRIANGLES, idx_sz, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void onKeyPress(int key, int mods) {
        glf::BaseApp::onKeyPress(key, mods);

        switch (key) {
            case GLFW_KEY_UP:
            case GLFW_KEY_DOWN: {
                tex_mix += key == GLFW_KEY_UP ? 0.01 : -0.01;
                tex_mix = glm::clamp<float>(tex_mix, 0, 1);
            } break;

            default: break;
        }
    }

 private:
    enum BO_ID { vBO, eBO, BO_SZ };
    enum Attrib_ID { POS_ID, TEX_COORD_ID, TEX_MIX_ID };

    static const GLuint idx_sz = 6;  // For rect composed of two triags
    static const GLuint tex_sz = 2;

    // OpenGL data
    glf::Shader shader;
    GLfloat bg_col[4], tex_mix;
    GLuint vAO, buf_objs[BO_SZ], tex[tex_sz];

    // Transform data
    glm::vec3 rot_axis, offset;
    GLuint transform_loc;

    // Animation data
    anim::AnimatorRegistry<float> animator;
    anim::PropertyAnimator<float> x_offset_anim;
    anim::PropertyAnimator<float> y_offset_anim;
};

DECLARE_MAIN(RectRotate);
