#include <base_app.h>
#include <base_util.h>
#include <3d/camera.h>
#include <3d/object.h>
#include <base_shader.h>
#include <animator/anim.h>

#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtc/matrix_transform.hpp>
#include <external/soil/SOIL.h>

/**
 * @author: Methusael Murmu
 */

const glm::vec3 cube_pos[] = {
    glm::vec3(-2.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  0.0f,  0.0f),
    glm::vec3( 0.0f,  2.0f,  0.0f),
    glm::vec3( 0.0f, -2.0f,  0.0f),
    glm::vec3(-2.0f,  2.0f,  0.0f),
    glm::vec3( 2.0f,  2.0f,  0.0f),
    glm::vec3( 2.0f, -2.0f,  0.0f),
    glm::vec3(-2.0f, -2.0f,  0.0f),
    glm::vec3( 0.0f,  4.0f,  0.0f),
    glm::vec3( 0.0f, -4.0f,  0.0f)
};

struct LightShaderData {
    glf3d::LightType type;
    glm::vec3 pos, dir, diff;
    GLfloat cutoff_in, cutoff_out;  // For spotlights
    GLfloat inten, distance, scale;
};

// Light setup
#define SPOT_IDX 0      // Light to be used as spotlight
#define NUM_LIGHTS 2

LightShaderData light_data[NUM_LIGHTS] = {
    { glf3d::SPOT, glm::vec3(0.0f), glm::vec3(0.0f),
      glm::vec3(1.0f), glm::radians(5.0f), glm::radians(15.0f),
      5.0f, 100.0f, 0.2f },
    { glf3d::SUN, glm::vec3(0.0f), glm::vec3(-1.0f),
      glm::vec3(0.1f, 0.1f, 0.3f), 0, 0, 0.5f, 0.0f, 0.2f }
};

class LightCast: public glf::BaseApp {
 public:
    LightCast() {
        util::set_color4f1(bg_col, 0.0f, 0.0f, 0.0f);
        first_call = true;
    }

    void setup() {
        setTitle("Basic Light Cast");
        info.width = 1366; info.height = 768;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
    }

    void startup() {
        // Initiate shaders
        light_shader.load("media/light_cast/shaders/light.vert",
            glf::Shader::VERTEX);
        light_shader.load("media/light_cast/shaders/light.frag",
            glf::Shader::FRAGMENT);
        light_shader.compile();

        scene_shader.load("media/light_cast/shaders/scene.vert",
            glf::Shader::VERTEX);
        scene_shader.load("media/light_cast/shaders/scene.frag",
            glf::Shader::FRAGMENT);
        scene_shader.compile(); scene_shader.use();

        // Cube vertex data
        GLfloat verts[] = {
            // Positions           // Normals           // Texture
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
        };
        glEnable(GL_DEPTH_TEST);

        // Generate vertex and buffer objects
        glGenVertexArrays(VAO_SZ, vAO);
        glGenBuffers(1, &vBO);

        // Fill data buffer object
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // Configure scene vAO
        glBindVertexArray(vAO[SCENE]);
        // Assign vertex attributes
        glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POS_ID);
        glVertexAttribPointer(NORM_ID, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(NORM_ID);
        glVertexAttribPointer(TEX_ID, 2, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), BUFFER_OFFSET(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(TEX_ID);
        glBindVertexArray(0);  // Unbind vAO;

        // Configure light vAO
        glBindVertexArray(vAO[LIGHT]);
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        // Assign vertex attributes
        glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POS_ID);
        glBindVertexArray(0);  // Unbind vAO;

        // Load and configure textures
        const char* image_path[TEX_SZ] = {
            "media/light_cast/tex/crate_x.png",
            "media/light_cast/tex/crate_x_specular.png"
        };
        glGenTextures(TEX_SZ, tex);

        for (int i = 0; i < TEX_SZ; ++i) {
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

#define uni_loc(_shader, _name) \
    glGetUniformLocation(_shader.getProgram(), _name)

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex[DIFF_MAP]);
        glUniform1i(uni_loc(scene_shader, "material_diff_map"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex[SPEC_MAP]);
        glUniform1i(uni_loc(scene_shader, "material_spec_map"), 1);

        // Get uniform locations
        loc_scene_model     = uni_loc(scene_shader, "model");
        loc_scene_trans     = uni_loc(scene_shader, "transform");
        loc_scene_normal    = uni_loc(scene_shader, "normal_mat");
        loc_scene_cam_pos   = uni_loc(scene_shader, "cam_pos");

        loc_world_amb       = uni_loc(scene_shader, "world.ambience");
        loc_scene_shine     = uni_loc(scene_shader, "material.shininess");
        loc_scene_specint   = uni_loc(scene_shader, "material.specular_int");
        loc_scene_amb       = uni_loc(scene_shader, "material.ambience");

#define get_uni util::uni_loc_i
#define scene_prog scene_shader.getProgram()

        for (int i = 0; i < NUM_LIGHTS; ++i) {
            light_loc[i].type       = get_uni(scene_prog, "light", "type", i);
            light_loc[i].position   = get_uni(scene_prog, "light", "position", i);
            light_loc[i].direction  = get_uni(scene_prog, "light", "direction", i);
            light_loc[i].diffuse    = get_uni(scene_prog, "light", "diffuse", i);
            light_loc[i].intensity  = get_uni(scene_prog, "light", "intensity", i);
            light_loc[i].linear     = get_uni(scene_prog, "light", "linear", i);
            light_loc[i].quadratic  = get_uni(scene_prog, "light", "quadratic", i);
            light_loc[i].cutoff_out = get_uni(scene_prog, "light", "cutoff_out", i);
            light_loc[i].epsilon    = get_uni(scene_prog, "light", "epsilon", i);
        }

#undef scene_prog
#undef get_uni

        loc_scene_lcount    = uni_loc(scene_shader, "light_count");
        loc_light_trans     = uni_loc(light_shader, "transform");
        loc_light_col       = uni_loc(light_shader, "light_color");
#undef uni_loc

        // Configure transformations
        proj_mat = glm::perspective(cam.fov(),
            (GLfloat) getWidth() / getHeight(), 0.1f, 100.0f);

        cam.position(glm::vec3(0.0f, 0.0f, 10.0f));
        cam.front(glm::vec3(0.0f) - cam.position());

        // Configure objects
        glm::vec3 rot_axis = glm::vec3(0.0f, 1.0f, 0.0f);
        for (int i = 0; i < kCubeCount; ++i) {
            cubes[i].translate(cube_pos[i]);
            cubes[i].rotate(glm::radians(15.0f * cube_pos[i].x), rot_axis);
            cubes[i].material.specularIntensity(1.0f);
            cubes[i].material.shininess(32.0f);
            cubes[i].material.ambience(0.6f);
        }

        // Setup in-game lights
        for (int i = 0; i < NUM_LIGHTS; ++i) {
            light[i].type(light_data[i].type);
            light[i].translate(light_data[i].pos);
            light[i].direction(light_data[i].dir);
            light[i].intensity(light_data[i].inten);
            light[i].material.diffuse(light_data[i].diff);
            light[i].distance(light_data[i].distance);
            light[i].scale(glm::vec3(light_data[i].scale));
            light[i].cutoffInner(light_data[i].cutoff_in);
            light[i].cutoffOuter(light_data[i].cutoff_out);

            // Static shader uniforms (I assume them static that is)
            glUniform1i(light_loc[i].type, light[i].type());
            glUniform3fv(light_loc[i].diffuse, 1,
                glm::value_ptr(light[i].material.diffuse()));
            glUniform1f(light_loc[i].intensity, light[i].intensity());
            glUniform1f(light_loc[i].linear, light[i].linear());
            glUniform1f(light_loc[i].quadratic, light[i].quadratic());

            if (light[i].type() == glf3d::SPOT) {
                glUniform1f(
                    light_loc[i].cutoff_out, cosf(light[i].cutoffOuter()));
                glUniform1f(light_loc[i].epsilon,
                    cosf(light[i].cutoffInner()) - cosf(light[i].cutoffOuter()));
            }
        }
        // Other static data
        glUniform1i(loc_scene_lcount, NUM_LIGHTS);
        glUniform3f(loc_world_amb, bg_col[0], bg_col[1], bg_col[2]);
    }

    void shutdown() {
        glDeleteTextures(TEX_SZ, tex);
        glDeleteBuffers(1, &vBO);
        glDeleteVertexArrays(VAO_SZ, vAO);
    }

#define _vp(f) glm::value_ptr(f)

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Update per-loop object states
        update_state(duration);
        proj_view_mat = proj_mat * cam.viewMat();

        // Render scene vAO
        scene_shader.use();
        glBindVertexArray(vAO[SCENE]);
        glUniform3fv(loc_scene_cam_pos, 1, _vp(cam.position()));

        // Send light data to shader
        for (int i = 0; i < NUM_LIGHTS; ++i) {
            glUniform3fv(light_loc[i].position, 1, _vp(light[i].translate()));
            glUniform3fv(light_loc[i].direction, 1, _vp(light[i].direction()));
        }

        for (int i = 0; i < kCubeCount; ++i) {
            trans_mat = proj_view_mat * cubes[i].modelMat();
            glUniformMatrix4fv(loc_scene_model, 1, GL_FALSE, _vp(cubes[i].modelMat()));
            glUniformMatrix4fv(loc_scene_trans, 1, GL_FALSE, _vp(trans_mat));

            // Materials
            glUniform1f(loc_scene_shine, cubes[i].material.shininess());
            glUniform1f(loc_scene_specint, cubes[i].material.specularIntensity());
            glUniform1f(loc_scene_amb, cubes[i].material.ambience());

            glm::mat3 normal = glm::transpose(
                glm::inverse(glm::mat3(cubes[i].modelMat())));
            glUniformMatrix3fv(loc_scene_normal, 1, GL_FALSE, _vp(normal));
            glDrawArrays(GL_TRIANGLES, 0, kVertsCount);
        }
        glBindVertexArray(0);

        // Render light vAO
        light_shader.use();
        glBindVertexArray(vAO[LIGHT]);
        for (int i = 0; i < NUM_LIGHTS; ++i) {
            if (light[i].type() != glf3d::POINT) continue;
            trans_mat = proj_view_mat * light[i].modelMat();

            glUniformMatrix4fv(loc_light_trans, 1, GL_FALSE, _vp(trans_mat));
            glUniform3fv(loc_light_col, 1, _vp(light[i].material.diffuse()));
            glDrawArrays(GL_TRIANGLES, 0, kVertsCount);
        }
        glBindVertexArray(0);
    }
#undef _vp

    // Ugly state machine
    void update_state(double duration) {
        // Handle keyboard input
        if (testKeyState(GLFW_KEY_W, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_FORWARD, duration);
        if (testKeyState(GLFW_KEY_S, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_BACKWARD, duration);
        if (testKeyState(GLFW_KEY_A, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_LEFT, duration);
        if (testKeyState(GLFW_KEY_D, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_RIGHT, duration);

        if (light[SPOT_IDX].type() == glf3d::SPOT) {
            light[SPOT_IDX].translate(cam.position());
            light[SPOT_IDX].direction(cam.front());
        }
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
    enum Attrib_ID { POS_ID, NORM_ID, TEX_ID };
    enum VAOs { SCENE, LIGHT, VAO_SZ };
    enum Textures { DIFF_MAP, SPEC_MAP, TEX_SZ };

    static const GLuint kVertsCount = 36;
    static const GLuint kCubeCount  = 10;

    // OpenGL data
    GLfloat bg_col[4];
    GLuint vAO[VAO_SZ], vBO, tex[TEX_SZ];
    glf::Shader scene_shader, light_shader;

    // Shader uniform locations (Scene shader)
    GLuint loc_scene_model, loc_scene_trans, loc_scene_normal,
           loc_scene_shine, loc_scene_specint, loc_scene_amb,
           loc_scene_cam_pos, loc_scene_lcount, loc_world_amb;
    // Light shader
    GLuint loc_light_col, loc_light_trans;

    struct _Light {
        GLuint type, position, direction, diffuse,
               cutoff_out, epsilon, intensity, linear, quadratic;
    } light_loc[NUM_LIGHTS];

    // Transform data
    glm::mat4 proj_mat, proj_view_mat, trans_mat;

    // 3d objects
    glf3d::FreeCamera cam;
    glf3d::Object3D cubes[kCubeCount];
    glf3d::Light light[NUM_LIGHTS];

    // 3d object state data
    GLfloat mlast_x, mlast_y;
    bool first_call;
};

DECLARE_MAIN(LightCast);
