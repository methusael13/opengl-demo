#include <base_app.h>
#include <base_util.h>
#include <base_shader.h>
#include <3d/camera.h>
#include <3d/object.h>
#include <3d/model.h>

#include <external/soil/SOIL.h>
#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/matrix_transform.hpp>
#include <external/glm/gtc/type_ptr.hpp>

#include <vector>

/**
 * @author: Methusael Murmu
 */

GLuint loadCubeMap(const std::vector<const GLchar*>& faces);

class CubeMap: public glf::BaseApp {
 public:
    CubeMap() {
        util::set_color4f1(bg_col, 0.8f, 0.8f, 0.8f);
        shader_idx = REFLECT; first_call = true;
    }

    void setup() {
        setTitle("Cube Map Demo");
        info.width = 1366; info.height = 768;
        info.samples = 4;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
    }

    void startup() {
        // Initiate scene_shader
        scene_shader[REFLECT].load("media/cube_map/shaders/scene_reflect.vert",
            glf::Shader::VERTEX);
        scene_shader[REFLECT].load("media/cube_map/shaders/scene_reflect.frag",
            glf::Shader::FRAGMENT);
        scene_shader[REFLECT].compile();

        scene_shader[REFRACT].load("media/cube_map/shaders/scene_refract.vert",
            glf::Shader::VERTEX);
        scene_shader[REFRACT].load("media/cube_map/shaders/scene_refract.frag",
            glf::Shader::FRAGMENT);
        scene_shader[REFRACT].compile();

        cube_map_shader.load("media/cube_map/shaders/cube_map.vert",
            glf::Shader::VERTEX);
        cube_map_shader.load("media/cube_map/shaders/cube_map.frag",
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
        skybox_files.push_back("media/data/skybox/right.jpg");
        skybox_files.push_back("media/data/skybox/left.jpg");
        skybox_files.push_back("media/data/skybox/top.jpg");
        skybox_files.push_back("media/data/skybox/bottom.jpg");
        skybox_files.push_back("media/data/skybox/back.jpg");
        skybox_files.push_back("media/data/skybox/front.jpg");
        skyboxTex = loadCubeMap(skybox_files);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
        glUniform1i(glGetUniformLocation(cube_map_shader.getProgram(), "cube_map"), GL_TEXTURE4);

        // Shader locations
        for (GLint i = 0; i < SHADER_SZ; ++i) {
            glUniform1i(glGetUniformLocation(scene_shader[i].getProgram(), "cube_map"), GL_TEXTURE4);
            scene[i].transform = glGetUniformLocation(scene_shader[i].getProgram(), "transform");
            scene[i].normal = glGetUniformLocation(scene_shader[i].getProgram(), "normal_mat");
            scene[i].model = glGetUniformLocation(scene_shader[i].getProgram(), "model_mat");
            scene[i].cam_pos = glGetUniformLocation(scene_shader[i].getProgram(), "cam_pos");
        }
        cube_map.transform = glGetUniformLocation(cube_map_shader.getProgram(), "transform");

        // Configure transformations
        proj_mat = glm::perspective(cam.fov(),
            (GLfloat) getWidth() / getHeight(), 0.1f, 100.0f);

        cam.position(glm::vec3(0.0f, 0.0f, 5.0f));
        cam.front(glm::vec3(0.0f) - cam.position());
        cam.speed(6.0f);  // 6 units in 1 second

        crysis.load("media/data/models/nanosuit/nanosuit.obj");
        crysis.scalef(0.5f);
    }

    void shutdown() {
        glDeleteTextures(1, &skyboxTex);
        glDeleteBuffers(1, &skyboxVBO);
        glDeleteVertexArrays(1, &skyboxVAO);
    }

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Update object states
        update_state(duration);

        scene_shader[shader_idx].use();
        glUniform3fv(scene[shader_idx].cam_pos, 1, glm::value_ptr(cam.position()));

        glm::mat4 tmat; glm::mat3 norm;
        tmat = proj_mat * cam.viewMat() * crysis.modelMat();
        glUniformMatrix4fv(scene[shader_idx].transform, 1, GL_FALSE, glm::value_ptr(tmat));
        glUniformMatrix4fv(scene[shader_idx].model, 1, GL_FALSE, glm::value_ptr(crysis.modelMat()));

        norm = glm::transpose(glm::inverse(glm::mat3(crysis.modelMat())));
        glUniformMatrix3fv(scene[shader_idx].normal, 1, GL_FALSE, glm::value_ptr(norm));
        crysis.render(&tex_info);

        // Draw cube map
        cube_map_shader.use();
        glDepthFunc(GL_LEQUAL);

        glBindVertexArray(skyboxVAO);
        tmat = proj_mat * glm::mat4(glm::mat3(cam.viewMat())) * skybox.modelMat();
        glUniformMatrix4fv(cube_map.transform, 1, GL_FALSE, glm::value_ptr(tmat));
        glDrawArrays(GL_TRIANGLES, 0, kVertsSize);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
    }

    // Ugly state machine
    void update_state(double duration) {
        bool tex_key = false;

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
    enum ShaderType { REFLECT, REFRACT, SHADER_SZ };

    static const GLuint kVertsSize = 36;
    static const GLuint kCubeSize = 10;

    // OpenGL data
    GLfloat bg_col[4];
    GLuint skyboxVAO, skyboxVBO, skyboxTex;
    glf::Shader scene_shader[SHADER_SZ], cube_map_shader;

    // Transform data
    glm::mat4 proj_mat;

    // Shader data
    ShaderType shader_idx;
    glf::ShaderTextureInfo tex_info;
    struct {
        GLuint normal, model, transform;
        GLuint cam_pos;
    } scene[SHADER_SZ];
    struct { GLuint transform; } cube_map;

    // 3d objects
    glf3d::FreeCamera cam;
    glf3d::Object3D skybox;
    glf3d::Model crysis;

    // 3d object state data
    GLfloat mlast_x, mlast_y;
    bool first_call;
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

        fprintf(stdout, "Loaded tetxure: %s\n", faces[i]);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return tex_id;
}

DECLARE_MAIN(CubeMap);
