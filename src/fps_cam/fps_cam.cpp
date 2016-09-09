#include <base_app.h>
#include <base_util.h>
#include <base_shader.h>
#include <3d/camera.h>
#include <3d/object.h>

#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/matrix_transform.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/soil/SOIL.h>

/**
 * @author: Methusael Murmu
 */

const glm::vec3 cube_pos[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f,  2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f,  3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f,  2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f,  1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

class FPSCamDemo: public glf::BaseApp {
 public:
    FPSCamDemo() {
        util::set_color4f1(bg_col, 0.8f, 0.8f, 0.8f);
        tex_mix = 0.2f; first_call = true;
    }

    void setup() {
        setTitle("FPS Camera Demo");
        info.width = 1366; info.height = 768;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
    }

    void startup() {
        // Initiate shader
        shader.load("media/fps_cam/shaders/fps_cam.vert",
            glf::Shader::VERTEX);
        shader.load("media/fps_cam/shaders/fps_cam.frag",
            glf::Shader::FRAGMENT);
        shader.compile(); shader.use();

        // Cube vertex data
        GLfloat verts[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };
        glEnable(GL_DEPTH_TEST);

        // Generate vertex and buffer objects
        glGenVertexArrays(1, &vAO);
        glGenBuffers(1, &vBO);

        // Configure vAO
        glBindVertexArray(vAO);
        // Data buffers
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // Assign vertex attributes
        glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POS_ID);
        glVertexAttribPointer(TEX_COORD_ID, 2, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(TEX_COORD_ID);
        glBindVertexArray(0);  // Unbind vAO;

        // Load textures
        const char* image_path[kTexSize] = {
            "media/fps_cam/tex/crate_x.png",
            "media/fps_cam/tex/crate_bio.png"
        };
        glGenTextures(kTexSize, tex);

        for (int i = 0; i < kTexSize; ++i) {
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

        // Configure transformations
        trans_loc = glGetUniformLocation(shader.getProgram(), "transform");
        proj_mat = glm::perspective(cam.fov(),
            (GLfloat) getWidth() / getHeight(), 0.1f, 100.0f);

        cam.position(glm::vec3(0.0f, 0.0f, 5.0f));
        cam.front(glm::vec3(0.0f) - cam.position());
        cam.speed(4.0f);  // 4 units in 1 second

        // Configure cubes
        glm::vec3 rot_axis = glm::vec3(1.0f, 0.3f, 0.5f);
        for (int i = 0; i < kCubeSize; ++i) {
            cubes[i].translate(cube_pos[i]);
            cubes[i].rotate(glm::radians(20.0f * (i+1)), rot_axis);
        }
    }

    void shutdown() {
        glDeleteTextures(kTexSize, tex);
        glDeleteBuffers(1, &vBO);
        glDeleteVertexArrays(1, &vAO);
    }

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Update object states
        update_state(duration);

        glBindVertexArray(vAO);
        glVertexAttrib1f(TEX_MIX_ID, tex_mix);

        for (int i = 0; i < kCubeSize; ++i) {
            glm::mat4 tmat = proj_mat * cam.viewMat() * cubes[i].modelMat();
            glUniformMatrix4fv(trans_loc, 1, GL_FALSE, glm::value_ptr(tmat));
            glDrawArrays(GL_TRIANGLES, 0, kVertsSize);
        }

        glBindVertexArray(0);
    }

    // Ugly state machine
    void update_state(double duration) {
        bool tex_key = false;

        if (testKeyState(GLFW_KEY_UP, GLFW_PRESS)) {
            tex_mix += duration * 0.8f;
            tex_key = true;
        }
        if (testKeyState(GLFW_KEY_DOWN, GLFW_PRESS)) {
            tex_mix -= duration * 0.8f;
            tex_key = true;
        }

        if (tex_key)  // Texture mix modified
            tex_mix = glm::clamp<float>(tex_mix, 0, 1);

        if (testKeyState(GLFW_KEY_W, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_FORWARD, duration);
        if (testKeyState(GLFW_KEY_S, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_BACKWARD, duration);
        if (testKeyState(GLFW_KEY_A, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_LEFT, duration);
        if (testKeyState(GLFW_KEY_D, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_RIGHT, duration);
    }

    void onMouseMove(float x, float y) {
        if (first_call) {
            mlast_x = x; mlast_y = y;
            first_call = false;
        }

        cam.processLook(x - mlast_x, mlast_y - y);
        mlast_x = x; mlast_y = y;
    }

 private:
    enum Attrib_ID { POS_ID, TEX_COORD_ID, TEX_MIX_ID };

    static const GLuint kVertsSize = 36;
    static const GLuint kTexSize = 2;
    static const GLuint kCubeSize = 10;

    // OpenGL data
    glf::Shader shader;
    GLfloat bg_col[4], tex_mix;
    GLuint vAO, vBO, tex[kTexSize];

    // Transform data
    glm::mat4 proj_mat;
    GLuint trans_loc;

    // 3d objects
    glf3d::FreeCamera cam;
    glf3d::Object3D cubes[kCubeSize];

    // 3d object state data
    GLfloat mlast_x, mlast_y;
    bool first_call;
};

DECLARE_MAIN(FPSCamDemo);
