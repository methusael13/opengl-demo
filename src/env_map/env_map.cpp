#include <base_app.h>
#include <base_util.h>
#include <base_shader.h>
#include <3d/camera.h>
#include <3d/object.h>
#include <3d/model.h>
#include <animator/anim.h>

#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/matrix_transform.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/soil/SOIL.h>

#include <vector>

/**
 * @author: Methusael Murmu
 */

// Model rotation axis
const glm::vec3 kModelRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);

GLuint loadCubeMap(const std::vector<const GLchar*>& faces);

class EnvMap: public glf::BaseApp {
 public:
    EnvMap() {
        util::set_color4f1(bg_col, 0.1f, 0.1f, 0.1f);
        first_call = true;
    }

    void setup() {
        setTitle("Cube Map Demo");
        info.width = 1366; info.height = 768;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
        info.samples = 4;
    }

    void startup() {
        // Initiate scene_shader
        scene_shader.load("media/env_map/shaders/scene.vert",
            glf::Shader::VERTEX);
        scene_shader.load("media/env_map/shaders/scene.frag",
            glf::Shader::FRAGMENT);
        scene_shader.compile();

        cube_map_shader.load("media/env_map/shaders/cube_map.vert",
            glf::Shader::VERTEX);
        cube_map_shader.load("media/env_map/shaders/cube_map.frag",
            glf::Shader::FRAGMENT);
        cube_map_shader.compile();

        GLfloat skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);

        // Configure Skybox
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);

        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POS_ID);
        glBindVertexArray(0);

        std::vector<const GLchar*> skybox_files;
        skybox_files.push_back("media/data/skybox_diff/right.png");
        skybox_files.push_back("media/data/skybox_diff/left.png");
        skybox_files.push_back("media/data/skybox_diff/top.png");
        skybox_files.push_back("media/data/skybox_diff/bottom.png");
        skybox_files.push_back("media/data/skybox_diff/back.png");
        skybox_files.push_back("media/data/skybox_diff/front.png");
        skyboxTex[0] = loadCubeMap(skybox_files);

        skybox_files.clear();
        skybox_files.push_back("media/data/skybox/right.jpg");
        skybox_files.push_back("media/data/skybox/left.jpg");
        skybox_files.push_back("media/data/skybox/top.jpg");
        skybox_files.push_back("media/data/skybox/bottom.jpg");
        skybox_files.push_back("media/data/skybox/back.jpg");
        skybox_files.push_back("media/data/skybox/front.jpg");
        skyboxTex[1] = loadCubeMap(skybox_files);

        // Shader locations
        scene.transform = glGetUniformLocation(scene_shader.getProgram(), "transform");
        scene.normal = glGetUniformLocation(scene_shader.getProgram(), "normal_mat");
        scene.model = glGetUniformLocation(scene_shader.getProgram(), "model_mat");
        scene.cam_pos = glGetUniformLocation(scene_shader.getProgram(), "cam_pos");
        scene.cube_map = glGetUniformLocation(scene_shader.getProgram(), "cube_map");

        cube_map.transform = glGetUniformLocation(cube_map_shader.getProgram(), "transform");
        cube_map.cube_map = glGetUniformLocation(cube_map_shader.getProgram(), "cube_map");

        // Configure transformations
        proj_mat = glm::perspective(cam.fov(),
            (GLfloat) getWidth() / getHeight(), 0.1f, 100.0f);

        cam.position(glm::vec3(0.0f, 0.0f, 5.0f));
        cam.front(glm::vec3(0.0f) - cam.position());
        cam.speed(6.0f);  // 6 units in 1 second

        crysis.load("media/data/models/monkey/monkey.obj");
        crysis.scalef(0.8f);

        // Animation data
        y_axis_rot.set(0.0f, 360.0f, 6.0f);
        y_axis_rot.setLoopType(anim::LOOP_START);
        y_axis_rot.setInterpolator(
            anim::InterpolatorFactory<GLfloat>::create(anim::LINEAR));
        animator.add(&y_axis_rot);
    }

    void shutdown() {
        glDeleteTextures(2, skyboxTex);
        glDeleteBuffers(1, &skyboxVBO);
        glDeleteVertexArrays(1, &skyboxVAO);
    }

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Update object states
        update_state(duration);

        scene_shader.use();
        glUniform3fv(scene.cam_pos, 1, glm::value_ptr(cam.position()));

        glm::mat4 tmat; glm::mat3 norm;
        tmat = proj_mat * cam.viewMat() * crysis.modelMat();
        glUniformMatrix4fv(scene.transform, 1, GL_FALSE, glm::value_ptr(tmat));
        glUniformMatrix4fv(scene.model, 1, GL_FALSE, glm::value_ptr(crysis.modelMat()));

        norm = glm::transpose(glm::inverse(glm::mat3(crysis.modelMat())));
        glUniformMatrix3fv(scene.normal, 1, GL_FALSE, glm::value_ptr(norm));

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex[0]);
        glUniform1i(scene.cube_map, 2);
        crysis.render(&tex_info);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

#if 1
        // Draw cube map
        cube_map_shader.use();
        glDepthFunc(GL_LEQUAL);

        glBindVertexArray(skyboxVAO);
        tmat = proj_mat * glm::mat4(glm::mat3(cam.viewMat())) * skybox.modelMat();
        glUniformMatrix4fv(cube_map.transform, 1, GL_FALSE, glm::value_ptr(tmat));

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex[1]);
        glUniform1i(cube_map.cube_map, 2);
        glDrawArrays(GL_TRIANGLES, 0, kVertsSize);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
#endif
    }

    // Ugly state machine
    void update_state(double duration) {
        // Update animation data
        animator.animate(duration);
        crysis.rotate(glm::radians(y_axis_rot.value()), kModelRotAxis);

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
    enum Attrib_ID { POS_ID, NORM_ID, TEX_COORD_ID };

    static const GLuint kVertsSize = 36;
    static const GLuint kCubeSize = 10;

    // OpenGL data
    GLfloat bg_col[4];
    GLuint skyboxVAO, skyboxVBO, skyboxTex[2];
    glf::Shader scene_shader, cube_map_shader;

    // Transform data
    glm::mat4 proj_mat;

    // Shader data
    glf::ShaderTextureInfo tex_info;
    struct {
        GLuint normal, model, transform;
        GLuint cam_pos, cube_map;
    } scene;
    struct { GLuint transform, cube_map; } cube_map;

    // 3d objects
    glf3d::FreeCamera cam;
    glf3d::Object3D skybox;
    glf3d::Model crysis;

    // 3d object state data
    GLfloat mlast_x, mlast_y;
    bool first_call;

    // Animation data
    anim::AnimatorRegistry<GLfloat> animator;
    anim::PropertyAnimator<GLfloat> y_axis_rot;
};

GLuint loadCubeMap(const std::vector<const GLchar*>& faces) {
    GLuint i, tex_id;
    GLint width, height;
    unsigned char* image;

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

    for (i = 0; i < faces.size(); ++i) {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
            0, GL_RGB, GL_UNSIGNED_BYTE, image);
        SOIL_free_image_data(image);

        if (image)
            fprintf(stdout, "Loaded tetxure: %s\n", faces[i]);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return tex_id;
}

DECLARE_MAIN(EnvMap);
