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

class BasicLight: public glf::BaseApp {
 public:
    BasicLight() {
        util::set_color4f1(bg_col, 0.1f, 0.1f, 0.1f);
        first_call = true;
    }

    void setup() {
        setTitle("Basic Light Demo");
        info.width = 1366; info.height = 768;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
    }

    void startup() {
        // Initiate shaders
        light_shader.load("media/light_basic/shaders/light.vert",
            glf::Shader::VERTEX);
        light_shader.load("media/light_basic/shaders/light.frag",
            glf::Shader::FRAGMENT);
        light_shader.compile();

        scene_shader.load("media/light_basic/shaders/scene.vert",
            glf::Shader::VERTEX);
        scene_shader.load("media/light_basic/shaders/scene.frag",
            glf::Shader::FRAGMENT);
        scene_shader.compile();

        // Cube vertex data
        GLfloat verts[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
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
            6 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POS_ID);
        glVertexAttribPointer(NORM_ID, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(NORM_ID);
        glBindVertexArray(0);  // Unbind vAO;

        // Configure light vAO
        glBindVertexArray(vAO[LIGHT]);
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        // Assign vertex attributes
        glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(POS_ID);
        glBindVertexArray(0);  // Unbind vAO;

        // Get uniform locations
#define uni_loc(_shader, _name) \
    glGetUniformLocation(_shader.getProgram(), _name)

            loc_scene_model     = uni_loc(scene_shader, "model");
            loc_scene_trans     = uni_loc(scene_shader, "transform");
            loc_scene_normal    = uni_loc(scene_shader, "normal_mat");
            loc_scene_cam_pos   = uni_loc(scene_shader, "cam_pos");

            loc_world_amb       = uni_loc(scene_shader, "world.ambience");
            loc_scene_diff      = uni_loc(scene_shader, "material.diffuse");
            loc_scene_spec      = uni_loc(scene_shader, "material.specular");
            loc_scene_shine     = uni_loc(scene_shader, "material.shininess");
            loc_scene_specint   = uni_loc(scene_shader, "material.specular_int");
            loc_scene_amb       = uni_loc(scene_shader, "material.ambience");

            loc_scene_lcolor    = uni_loc(scene_shader, "light.color");
            loc_scene_lpos      = uni_loc(scene_shader, "light.position");
            loc_scene_linten    = uni_loc(scene_shader, "light.intensity");
            loc_scene_latten    = uni_loc(scene_shader, "light.attenuation");

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
        for (int i = 0; i < kCubeSize; ++i) {
            cubes[i].translate(cube_pos[i]);
            cubes[i].rotate(glm::radians(15.0f * cube_pos[i].x), rot_axis);
            cubes[i].material.diffuse(util::color_random());
            cubes[i].material.specularIntensity(0.5f);
            cubes[i].material.shininess(32.0f);
            cubes[i].material.ambience(0.8f);
        }

        lamp.translate(light_pos = glm::vec3(0.0f));
        lamp.material.diffuse(glm::vec3(1.0f)); lamp.scale(glm::vec3(0.2f));
        lamp.distance(30.0f); lamp.intensity(1.0f);

        // Static shader uniforms (I assume them static that is)
        scene_shader.use();
        glUniform1f(loc_scene_linten, lamp.intensity());
        glUniform1f(loc_scene_latten, lamp.quadratic());
        glUniform3f(loc_world_amb, bg_col[0], bg_col[1], bg_col[2]);
        glUniform3fv(loc_scene_lcolor, 1, glm::value_ptr(lamp.material.diffuse()));

        // Configure animation data
        x_offset_anim.set(-5.0f, 5.0f, 2.0f);
        x_offset_anim.setLoopType(anim::LOOP_START);
        x_offset_anim.setInterpolator(
            anim::InterpolatorFactory<float>::create(anim::COSINE));
        animator.add(&x_offset_anim);

        z_offset_anim.set(-5.0f, 5.0f, 2.0f);
        z_offset_anim.setLoopType(anim::LOOP_START);
        z_offset_anim.setInterpolator(
            anim::InterpolatorFactory<float>::create(anim::SINE));
        animator.add(&z_offset_anim);
    }

    void shutdown() {
        glDeleteBuffers(1, &vBO);
        glDeleteVertexArrays(VAO_SZ, vAO);
    }

#define _vp(f) glm::value_ptr(f)

    void render(double elapsedTime, double duration) {
        // Clear to background
        glClearBufferfv(GL_COLOR, 0, bg_col);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Update object states
        animator.animate(duration);
        update_state(duration);
        glm::mat4 tmat;

        // Render scene vAO
        scene_shader.use();
        glBindVertexArray(vAO[SCENE]);
        glUniform3fv(loc_scene_cam_pos, 1, _vp(cam.position()));
        glUniform3fv(loc_scene_lpos, 1, _vp(lamp.translate()));

        for (int i = 0; i < kCubeSize; ++i) {
            tmat = proj_mat * cam.viewMat() * cubes[i].modelMat();
            glUniformMatrix4fv(loc_scene_model, 1, GL_FALSE, _vp(cubes[i].modelMat()));
            glUniformMatrix4fv(loc_scene_trans, 1, GL_FALSE, _vp(tmat));

            // Materials
            glUniform3fv(loc_scene_diff, 1, _vp(cubes[i].material.diffuse()));
            glUniform3fv(loc_scene_spec, 1, _vp(cubes[i].material.specular()));
            glUniform1f(loc_scene_shine, cubes[i].material.shininess());
            glUniform1f(loc_scene_specint, cubes[i].material.specularIntensity());
            glUniform1f(loc_scene_amb, cubes[i].material.ambience());

            glm::mat3 normal = glm::transpose(
                glm::inverse(glm::mat3(cubes[i].modelMat())));
            glUniformMatrix3fv(loc_scene_normal, 1, GL_FALSE, _vp(normal));
            glDrawArrays(GL_TRIANGLES, 0, kVertsSize);
        }
        glBindVertexArray(0);

        // Render lamp vAO
        light_shader.use();
        glBindVertexArray(vAO[LIGHT]);
        tmat = proj_mat * cam.viewMat() * lamp.modelMat();
        glUniformMatrix4fv(loc_light_trans, 1, GL_FALSE, _vp(tmat));
        glUniform3fv(loc_light_col, 1, _vp(lamp.material.diffuse()));
        glDrawArrays(GL_TRIANGLES, 0, kVertsSize);
        glBindVertexArray(0);
    }
#undef _vp

    // Ugly state machine
    void update_state(double duration) {
        // Animation updates
        light_pos.x = x_offset_anim.value();
        light_pos.z = z_offset_anim.value();
        lamp.translate(light_pos);

        // Handle keyboard input
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
    enum Attrib_ID { POS_ID, NORM_ID };
    enum VAOs { SCENE, LIGHT, VAO_SZ };

    static const GLuint kVertsSize = 36;
    static const GLuint kCubeSize = 10;

    // OpenGL data
    GLfloat bg_col[4];
    GLuint vAO[VAO_SZ], vBO;
    glf::Shader scene_shader, light_shader;

    // Shader uniforms (Scene shader)
    GLuint loc_scene_model, loc_scene_trans, loc_scene_normal,
           loc_scene_diff, loc_scene_spec, loc_scene_shine,
           loc_scene_specint, loc_scene_amb,
           loc_scene_lcolor, loc_scene_lpos, loc_scene_latten,
           loc_scene_linten, loc_scene_cam_pos,
           loc_world_amb;
    // Light shader
    GLuint loc_light_col, loc_light_trans;

    // Transform data
    glm::mat4 proj_mat;
    glm::vec3 light_pos;

    // 3d objects
    glf3d::FreeCamera cam;
    glf3d::Object3D cubes[kCubeSize];
    glf3d::Light lamp;

    // 3d object state data
    GLfloat mlast_x, mlast_y;
    bool first_call;

    // Animation data
    anim::AnimatorRegistry<float> animator;
    anim::PropertyAnimator<float> x_offset_anim;
    anim::PropertyAnimator<float> z_offset_anim;
};

DECLARE_MAIN(BasicLight);
