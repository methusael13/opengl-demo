#include <base_app.h>
#include <base_util.h>
#include <base_shader.h>
#include <3d/camera.h>
#include <3d/object.h>
#include <3d/model.h>
#include <animator/anim.h>

#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtc/matrix_transform.hpp>
#include <external/soil/SOIL.h>

/**
 * @author: Methusael Murmu
 */

struct LightShaderData {
    glf3d::LightType type;
    glm::vec3 pos, dir, diff;
    GLfloat cutoff_in, cutoff_out;  // For spotlights
    GLfloat inten, distance, scale;
};

// Light setup
#define SPOT_IDX 0      // Light to be used as spotlight
#define NUM_LIGHTS 3

LightShaderData light_data[NUM_LIGHTS] = {
    { glf3d::SUN, glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  1.0f),
      glm::vec3(1.0f), 0, 0, 0.8f, 0.0f, 0.2f },
    { glf3d::SUN, glm::vec3(0.0f), glm::vec3(1.0f,  1.0f, -1.0f),
      glm::vec3(1.0f), 0, 0, 0.8f, 0.0f, 0.2f },
    { glf3d::SUN, glm::vec3(0.0f), glm::vec3(-1.0f),
      glm::vec3(1.0f), 0, 0, 1.5f, 0.0f, 0.2f }
};

// Model rotation axis
const glm::vec3 kModelRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);

class ModelExplode: public glf::BaseApp {
 public:
    ModelExplode() {
        util::set_color4f1(bg_col, 0.2f, 0.2f, 0.2f);
        view_state = VIEW_OBJECT;
        view_locked = false; first_call = true;
    }

    void setup() {
        setTitle("Assimp Model");
        info.width = 1366; info.height = 768;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
    }

    void startup() {
        // Initiate shaders
        light_shader.load("media/model_exp/shaders/light.vert",
            glf::Shader::VERTEX);
        light_shader.load("media/model_exp/shaders/light.frag",
            glf::Shader::FRAGMENT);
        light_shader.compile();

        scene_shader.load("media/model_exp/shaders/scene.vert", glf::Shader::VERTEX);
        scene_shader.load("media/model_exp/shaders/scene.frag", glf::Shader::FRAGMENT);
        scene_shader.load("media/model_exp/shaders/scene.geom", glf::Shader::GEOMETRY);
        scene_shader.compile(); scene_shader.use();

        normal_shader.load("media/model_exp/shaders/scene_norm.vert", glf::Shader::VERTEX);
        normal_shader.load("media/model_exp/shaders/scene_norm.frag", glf::Shader::FRAGMENT);
        normal_shader.load("media/model_exp/shaders/scene_norm.geom", glf::Shader::GEOMETRY);
        normal_shader.compile();

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

        // Configure light vAO
        glBindVertexArray(vAO[LIGHT]);
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // Assign vertex attributes
        glVertexAttribPointer(glf::ATTR_POS_ID, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(glf::ATTR_POS_ID);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);  // Unbind vAO;

#define uni_loc(_shader, _name) \
    glGetUniformLocation(_shader.getProgram(), _name)

        // Get uniform locations
        loc_scene_model     = uni_loc(scene_shader, "model");
        loc_scene_trans     = uni_loc(scene_shader, "transform");
        loc_scene_normal    = uni_loc(scene_shader, "normal_mat");
        loc_scene_cam_pos   = uni_loc(scene_shader, "cam_pos");
        loc_scene_sint      = uni_loc(scene_shader, "time");

        loc_world_amb       = uni_loc(scene_shader, "world.ambience");
        loc_scene_shine     = uni_loc(scene_shader, "material.shininess");
        loc_scene_specint   = uni_loc(scene_shader, "material.specular_int");
        loc_scene_amb       = uni_loc(scene_shader, "material.ambience");

#define get_uni util::uni_loc_i
#define scene_prog scene_shader.getProgram()

        for (i = 0; i < NUM_LIGHTS; ++i) {
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

        loc_norm_trans      = uni_loc(normal_shader, "transform");
        loc_norm_npmat      = uni_loc(normal_shader, "npMat");
#undef uni_loc

        // Texture uniform locations
        shader_tex_info.loadTextureLocations(scene_shader, glf::TEX_DIFFUSE, 2);
        shader_tex_info.loadTextureLocations(scene_shader, glf::TEX_SPECULAR, 2);

        // Configure transformations
        proj_mat = glm::perspective(cam.fov(),
            (GLfloat) getWidth() / getHeight(), 0.1f, 100.0f);

        cam.position(glm::vec3(0.053629f, 1.568647f, 4 * 1.268647f));
        cam.front(glm::vec3(0.0f, 0.0f, -1.0f));

        // Setup objects
        crysis.load("media/data/models/nanosuit/nanosuit.obj");
        crysis.scalef(0.2f);

        // Setup in-game lights
        for (i = 0; i < NUM_LIGHTS; ++i) {
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

        // Animation data
        y_axis_rot.set(0.0f, 360.0f, 6.0f);
        y_axis_rot.setLoopType(anim::LOOP_START);
        y_axis_rot.setInterpolator(
            anim::InterpolatorFactory<GLfloat>::create(anim::LINEAR));
        animator.add(&y_axis_rot);
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

        // Update per-loop object states
        update_state(duration);
        proj_view_mat = proj_mat * cam.viewMat();

        // Render scene vAO
        scene_shader.use();
        glUniform3fv(loc_scene_cam_pos, 1, _vp(cam.position()));
        glUniform1f(loc_scene_sint,
            view_state == VIEW_NORMALS ? 0 : elapsedTime);
                //pow((sin(10 * elapsedTime) + 1.0) * 0.5, 10));

        // Send light data to shader
        for (i = 0; i < NUM_LIGHTS; ++i) {
            glUniform3fv(light_loc[i].position, 1, _vp(light[i].position()));
            glUniform3fv(light_loc[i].direction, 1, _vp(light[i].direction()));
        }

        // Per model uniforms
        // Model specific transforms
        trans_mat = proj_view_mat * crysis.modelMat();
        glm::mat4 normal_mat4 = glm::transpose(glm::inverse(crysis.modelMat()));
        glm::mat3 normal_mat3 = glm::mat3(normal_mat4);

        glUniformMatrix4fv(loc_scene_model, 1, GL_FALSE, _vp(crysis.modelMat()));
        glUniformMatrix4fv(loc_scene_trans, 1, GL_FALSE, _vp(trans_mat));
        glUniformMatrix3fv(loc_scene_normal, 1, GL_FALSE, _vp(normal_mat3));

        // Model specific material
        glUniform1f(loc_scene_amb, crysis.material.ambience());
        glUniform1f(loc_scene_shine, crysis.material.shininess());
        glUniform1f(loc_scene_specint, crysis.material.specularIntensity());
        // Render model
        crysis.render(&shader_tex_info);

        if (view_state == VIEW_NORMALS) {
            normal_shader.use();
            glUniformMatrix4fv(loc_norm_trans, 1, GL_FALSE, _vp(trans_mat));

            normal_mat3 = glm::mat3(proj_view_mat * normal_mat4);
            glUniformMatrix3fv(loc_norm_npmat, 1, GL_FALSE, _vp(normal_mat3));
            crysis.render(&shader_tex_info);
        }

        // Render light vAO
        light_shader.use();
        glBindVertexArray(vAO[LIGHT]);
        for (i = 0; i < NUM_LIGHTS; ++i) {
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
        // Update animation data
        animator.animate(duration);
        crysis.rotate(glm::radians(y_axis_rot.value()), kModelRotAxis);

        // Handle keyboard input
        if (testKeyState(GLFW_KEY_W, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_FORWARD, duration);
        if (testKeyState(GLFW_KEY_S, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_BACKWARD, duration);
        if (testKeyState(GLFW_KEY_A, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_LEFT, duration);
        if (testKeyState(GLFW_KEY_D, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_RIGHT, duration);

        if (testKeyState(GLFW_KEY_SPACE, GLFW_PRESS) && !view_locked) {
            view_state = view_state == VIEW_OBJECT ?
                VIEW_NORMALS : VIEW_OBJECT;
            view_locked = true;
        } else if (testKeyState(GLFW_KEY_SPACE, GLFW_RELEASE)) {
            view_locked = false;
        }

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
    enum VAOs { LIGHT, VAO_SZ };
    enum ViewState { VIEW_OBJECT, VIEW_NORMALS };

    static const GLuint kVertsCount = 36;

    // OpenGL data
    GLfloat bg_col[4];
    GLuint vAO[VAO_SZ], vBO;
    glf::Shader scene_shader, light_shader, normal_shader;
    glf::ShaderTextureInfo shader_tex_info;
    GLuint i;  // Loop counter

    // Shader uniform locations (Scene shader)
    GLuint loc_scene_model, loc_scene_trans, loc_scene_normal,
           loc_scene_shine, loc_scene_specint, loc_scene_amb,
           loc_scene_cam_pos, loc_scene_lcount, loc_world_amb,
           loc_scene_sint;
    // Light shader
    GLuint loc_light_col, loc_light_trans;
    // Normal Shader
    GLuint loc_norm_trans, loc_norm_npmat;

    struct _LightLocation {  // Shader locations for scene light params
        GLuint type, position, direction, diffuse,
               cutoff_out, epsilon, intensity, linear, quadratic;
    } light_loc[NUM_LIGHTS];

    // Transform data
    glm::mat4 proj_mat, proj_view_mat, trans_mat;

    // 3d objects
    glf3d::FreeCamera cam;
    glf3d::Light light[NUM_LIGHTS];
    glf3d::Model crysis;

    // 3d object state data
    ViewState view_state;
    GLfloat mlast_x, mlast_y;
    bool first_call, view_locked;

    // Animation data
    anim::AnimatorRegistry<GLfloat> animator;
    anim::PropertyAnimator<GLfloat> y_axis_rot;
};

DECLARE_MAIN(ModelExplode);
