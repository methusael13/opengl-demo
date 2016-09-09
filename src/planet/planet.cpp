#include <base_app.h>
#include <base_util.h>
#include <3d/camera.h>
#include <3d/object.h>
#include <3d/model.h>
#include <base_shader.h>
#include <animator/anim.h>

#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtc/matrix_transform.hpp>

#include <vector>

/**
 * @author: Methusael Murmu
 */

// Planet rotation axis
const glm::vec3 kModelRotAxis = glm::vec3(1.0f, 1.0f, 1.0f);

class Planet: public glf::BaseApp {
 public:
    Planet() {
        util::set_color4f1(bg_col, 0.0f, 0.0f, 0.0f);
        first_call = true; srand(glfwGetTime());
    }

    void setup() {
        setTitle("Planet Demo");
        info.width = 1366; info.height = 768;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
        info.samples = 4;

        enableFPSDisplay(true);
        enableStatsDisplay(true);
    }

    void startup() {
        shader[PLANET].load("media/planet/shaders/planet.vert", glf::Shader::VERTEX);
        shader[PLANET].load("media/planet/shaders/planet.frag", glf::Shader::FRAGMENT);
        shader[PLANET].compile();

        shader[METEOR].load("media/planet/shaders/meteor.vert", glf::Shader::VERTEX);
        shader[METEOR].load("media/planet/shaders/meteor.frag", glf::Shader::FRAGMENT);
        shader[METEOR].compile();

        shader[TEXT].load("media/planet/shaders/text.vert", glf::Shader::VERTEX);
        shader[TEXT].load("media/planet/shaders/text.frag", glf::Shader::FRAGMENT);
        shader[TEXT].compile();
        setBaseTextShader(&shader[TEXT]);

        // Init shader loactions
#define uni_loc(_shader, _str) \
        glGetUniformLocation(_shader.getProgram(), _str)

        for (GLint i = 0; i < SHADER_SZ; ++i) {
            shader_loc[i].light.direction  = uni_loc(shader[i], "light.direction");
            shader_loc[i].light.diffuse    = uni_loc(shader[i], "light.diffuse");
            shader_loc[i].light.intensity  = uni_loc(shader[i], "light.intensity");

            // Texture uniform locations
            texInfo[i].loadTextureLocations(shader[i], glf::TEX_DIFFUSE, 2);
            texInfo[i].loadTextureLocations(shader[i], glf::TEX_SPECULAR, 2);
        }
        shader_loc[METEOR].pvMat           = uni_loc(shader[METEOR], "pvMat");
        shader_loc[PLANET].pvmMat          = uni_loc(shader[PLANET], "pvmMat");
        shader_loc[PLANET].mMat            = uni_loc(shader[PLANET], "mMat");
        shader_loc[PLANET].nMat            = uni_loc(shader[PLANET], "nMat");

#undef uni_loc

        models[PLANET].load("media/data/models/planet/planet.obj");
        models[PLANET].translate(glm::vec3(0.0f, -5.0f, 0.0));
        models[PLANET].scalef(100.0f);

        models[METEOR].load("media/data/models/rock/rock.obj");
        models[METEOR].renderContext().shouldDrawInstanced = true;
        models[METEOR].renderContext().instanceAmount = kMeteorCount;

        // Meteor matrices
        GLfloat x, y, z, angle, disp;
        glm::mat4* meteorMat = new glm::mat4[kMeteorCount];

        for (GLint i = 0; i < kMeteorCount; ++i) {
            glm::mat4 iMat;

            angle   = (360.0f * i) / kMeteorCount;
            disp    = fmod(rand(), 2 * kOffset) - kOffset;
            x       = sin(angle) * kRadius + disp;
            disp    = fmod(rand(), 2 * kOffset) - kOffset;
            z       = cos(angle) * kRadius + disp;
            disp    = fmod(rand(), 2 * kOffset) - kOffset;
            y       = disp * 0.3f;

            iMat    = glm::translate(iMat, glm::vec3(x, y, z));
            angle   = fmod(rand(), 360.0f);
            iMat    = glm::rotate(iMat, angle, glm::vec3(-0.1f, 0.5f, -0.3f));
            iMat    = glm::scale(iMat,
                        glm::vec3(kMaxSize - fmod(rand(), kSizeVariance * kMaxSize)));
            meteorMat[i] = iMat;
        }

        const glf3d::t_vmesh& meshes = models[METEOR].getMeshes();
        GLuint meshSz = meshes.size();

        for (GLint i = 0; i < meshSz; ++i) {
            glBindVertexArray(meshes[i].VAO());

            GLuint buffer;
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, kMeteorCount * sizeof(glm::mat4),
                &meteorMat[0], GL_STATIC_DRAW);

            GLsizei vec4_sz = sizeof(glm::vec4);
            for (GLint j = 0; j < 4; ++j) {
                glEnableVertexAttribArray(kInstanceAttribID + j);
                glVertexAttribPointer(kInstanceAttribID + j, 4, GL_FLOAT, GL_FALSE,
                    4 * vec4_sz, BUFFER_OFFSET(j * vec4_sz));
                glVertexAttribDivisor(kInstanceAttribID + j, 1);
            }

            glBindVertexArray(0);
        }
        delete[] meteorMat;

        // Lights!
        sun.direction(glm::vec3(-1.0f, -1.0f, 0.0f));
        sun.material.diffuse(glm::vec3(1.0f));
        sun.intensity(2.0f);

        // Camera!
        cam.position(glm::vec3(0.0f, 40.0f, 500.0f));
        cam.front(glm::vec3(0.0f) - cam.position());
        cam.near(0.1f); cam.far(2000.0f);
        cam.speed(35.0f);  // 6 units in 1 second

        // Projection!
        pMat = glm::perspective(cam.fov(),
            (GLfloat) getWidth() / getHeight(), cam.near(), cam.far());

        for (GLint i = 0; i < SHADER_SZ; ++i) {
            shader[i].use();
            glUniform3fv(shader_loc[i].light.direction, 1, glm::value_ptr(sun.direction()));
            glUniform3fv(shader_loc[i].light.diffuse, 1, glm::value_ptr(sun.material.diffuse()));
            glUniform1f(shader_loc[i].light.intensity,
                sun.intensity() + (i == PLANET ? 70.0f : 0.0f));
        }

        // Set optional OpenGL flags
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);

        // Animation data
        y_axis_rot.set(0.0f, 360.0f, 120.0f);
        y_axis_rot.setLoopType(anim::LOOP_START);
        y_axis_rot.setInterpolator(
            anim::InterpolatorFactory<GLfloat>::create(anim::LINEAR));
        animator.add(&y_axis_rot);
    }

    void render(double elapsedTime, double duration) {
        glClearBufferfv(GL_COLOR, 0, bg_col);
        glClear(GL_DEPTH_BUFFER_BIT);

        update_state(duration);
        pvMat = pMat * cam.viewMat();
        nMat = glm::transpose(glm::inverse(glm::mat3(models[PLANET].modelMat())));

        shader[PLANET].use();
        glUniformMatrix4fv(shader_loc[PLANET].pvmMat, 1, GL_FALSE,
            glm::value_ptr(pvMat * models[PLANET].modelMat()));
        glUniformMatrix4fv(shader_loc[PLANET].mMat, 1, GL_FALSE,
            glm::value_ptr(models[PLANET].modelMat()));
        glUniformMatrix3fv(shader_loc[PLANET].nMat, 1, GL_FALSE, glm::value_ptr(nMat));
        models[PLANET].render(&texInfo[PLANET]);

        shader[METEOR].use();
        glUniformMatrix4fv(shader_loc[METEOR].pvMat, 1, GL_FALSE,
            glm::value_ptr(pvMat));
        models[METEOR].render(&texInfo[METEOR]);
    }

    // Ugly state machine
    void update_state(double duration) {
        // Update animation data
        animator.animate(duration);
        models[PLANET].rotate(glm::radians(y_axis_rot.value()), kModelRotAxis);

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
    static constexpr GLuint kInstanceAttribID = 3;
    static constexpr GLuint kMeteorCount = 15000;

    // Meteor params
    static constexpr GLfloat kRadius        = 700.0f;
    static constexpr GLfloat kOffset        = 40.0f;
    static constexpr GLfloat kMaxSize       = 0.2f;
    static constexpr GLfloat kSizeVariance  = 0.95f;  // [0.0 - 1.0]

    enum SHADER_TYPE { PLANET, METEOR, TEXT, SHADER_SZ };

    // OpenGL data
    GLfloat bg_col[4];
    glf::Shader shader[SHADER_SZ];
    glf::ShaderTextureInfo texInfo[SHADER_SZ];

    // Shader data
    struct ShaderUniformLoc {
        GLuint pvmMat, pvMat, mMat, nMat;
        struct Light {
            GLuint direction, diffuse;
            GLuint intensity;
        } light;
    } shader_loc[SHADER_SZ];

    // Model data
    glf3d::Model models[SHADER_SZ];
    glf3d::FreeCamera cam;
    glf3d::Light sun;

    // Transformation data
    glm::mat3 nMat;
    glm::mat4 pMat, pvMat;

    // Object state data
    GLfloat mlast_x, mlast_y;
    bool first_call;

    // Animation data
    anim::AnimatorRegistry<GLfloat> animator;
    anim::PropertyAnimator<GLfloat> y_axis_rot;
};

DECLARE_MAIN(Planet);
