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
#define NUM_LIGHTS 1
#define DEPTH_RES_WIDTH 1024
#define DEPTH_RES_HEIGHT 1024

LightShaderData light_data[NUM_LIGHTS] = {
    { glf3d::SUN, glm::vec3(0.0f, 10.0f, 12.0f), glm::vec3(0.0f, -1.0f, -1.0f),
      glm::vec3(1.0f), 0, 0, 1.0f, 60.0f, 0.05f }
};

// Model rotation axis
const glm::vec3 kGlobalUp = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 kModelRotAxis = kGlobalUp;
const glm::vec3 kSpotLightOffset = glm::vec3(0.05f, -0.5f, -0.1f);

class ShadowMap: public glf::BaseApp {
 public:
    ShadowMap() {
        util::set_color4f1(bg_col, 0.005f, 0.005f, 0.005f);
        first_call = true;

        scaleBiasMat = glm::mat4(
            glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    }

    void setup() {
        setTitle("Shadow + Normal Map");
        info.width = 1366; info.height = 768;
        info.flags.fullscreen = 1;
        info.flags.cursor = 0;
        info.samples = 4;

        enableFPSDisplay(true);
        enableStatsDisplay(true);
    }

    void startup() {
        setupShaders();

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
        glEnable(GL_MULTISAMPLE);

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

        // Shadow buffers
        glGenTextures(1, &depthTex);
        glBindTexture(GL_TEXTURE_2D, depthTex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, DEPTH_RES_WIDTH, DEPTH_RES_HEIGHT,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenFramebuffers(1, &depthBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
        glDrawBuffer(GL_NONE);

        fprintf(stdout, "%s\x1b[0m\n",
            glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ?
                "\x1b[32mShadow initialization complete" : "\x1b[31mShadow initialization failed");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Configure transformations
        pMat = glm::perspective(cam.fov(),
            (GLfloat) getWidth() / getHeight(), 0.1f, 100.0f);
        lpMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);

        cam.position(glm::vec3(0, 4.0f, 10.0f));
        cam.front(glm::vec3(0.0) - cam.position());
        cam.speed(4.0f);

        // Setup objects
        model[BOX].load("media/data/models/Cyborg/Cyborg.obj", true);
        model[BOX].material.shininess(100.0f);
        // model[BOX].translate(glm::vec3(0.0f, 0.501f, 0.0f));
        model[BOX].scalef(0.8f);

        model[PLANE].load("media/data/models/pavement/pavement.obj", true);
        model[PLANE].material.shininess(200.0f);
        model[PLANE].scalef(0.8f);

        // Animation data
        light_rot.set(0.0f, 360.0f, 30.0f);
        light_rot.setLoopType(anim::LOOP_START);
        light_rot.setInterpolator(
            anim::InterpolatorFactory<GLfloat>::create(anim::LINEAR));
        animReg.add(&light_rot);

        glClearColor(bg_col[0], bg_col[1], bg_col[2], bg_col[3]);
    }

    void setupShaders() {
        // Initiate shaders
        shadow_shader.load("media/shadow_map/shaders/shadow.vert",
            glf::Shader::VERTEX);
        shadow_shader.load("media/shadow_map/shaders/shadow.frag",
            glf::Shader::FRAGMENT);
        shadow_shader.compile();

        light_shader.load("media/shadow_map/shaders/light.vert",
            glf::Shader::VERTEX);
        light_shader.load("media/shadow_map/shaders/light.frag",
            glf::Shader::FRAGMENT);
        light_shader.compile();

        text_shader.load("media/shadow_map/shaders/text.vert",
            glf::Shader::VERTEX);
        text_shader.load("media/shadow_map/shaders/text.frag",
            glf::Shader::FRAGMENT);
        text_shader.compile();
        setBaseTextShader(&text_shader);

        scene_shader.load("media/shadow_map/shaders/scene.vert",
            glf::Shader::VERTEX);
        scene_shader.load("media/shadow_map/shaders/scene.frag",
            glf::Shader::FRAGMENT);
        scene_shader.compile(); scene_shader.use();

#define uni_loc(_shader, _name) \
    glGetUniformLocation(_shader.getProgram(), _name)

        // Get uniformScene locations
        uniformScene.vert.modelMat      = uni_loc(scene_shader, "model");
        uniformScene.vert.transMat      = uni_loc(scene_shader, "transform");
        uniformScene.vert.normalMat     = uni_loc(scene_shader, "normal_mat");
        uniformScene.vert.shadowMat     = uni_loc(scene_shader, "shadow_mat");
        uniformScene.frag.cam_pos       = uni_loc(scene_shader, "cam_pos");
        uniformScene.frag.lcount        = uni_loc(scene_shader, "light_count");
        uniformScene.frag.shadowTex     = uni_loc(scene_shader, "shadow_tex");

        uniformScene.world.amb          = uni_loc(scene_shader, "world.ambience");
        uniformScene.material.shine     = uni_loc(scene_shader, "material.shininess");
        uniformScene.material.specint   = uni_loc(scene_shader, "material.specular_int");
        uniformScene.material.amb       = uni_loc(scene_shader, "material.ambience");

#define get_uni util::uni_loc_i
#define scene_prog scene_shader.getProgram()

        for (i = 0; i < NUM_LIGHTS; ++i) {
            uniformScene.light[i].type       = get_uni(scene_prog, "light", "type", i);
            uniformScene.light[i].position   = get_uni(scene_prog, "light", "position", i);
            uniformScene.light[i].direction  = get_uni(scene_prog, "light", "direction", i);
            uniformScene.light[i].diffuse    = get_uni(scene_prog, "light", "diffuse", i);
            uniformScene.light[i].intensity  = get_uni(scene_prog, "light", "intensity", i);
            uniformScene.light[i].linear     = get_uni(scene_prog, "light", "linear", i);
            uniformScene.light[i].quadratic  = get_uni(scene_prog, "light", "quadratic", i);
            uniformScene.light[i].cutoff_out = get_uni(scene_prog, "light", "cutoff_out", i);
            uniformScene.light[i].epsilon    = get_uni(scene_prog, "light", "epsilon", i);
        }

#undef scene_prog
#undef get_uni

        uniformLight.vert.transMat      = uni_loc(light_shader, "transform");
        uniformLight.frag.col           = uni_loc(light_shader, "light_color");
        uniformShadow.vert.lmvpMat      = uni_loc(shadow_shader, "lmvpMat");
#undef uni_loc

        // Texture uniform locations
        shader_tex_info.loadTextureLocations(scene_shader, glf::TEX_DIFFUSE, 1);
        shader_tex_info.loadTextureLocations(scene_shader, glf::TEX_SPECULAR, 1);
        shader_tex_info.loadTextureLocations(scene_shader, glf::TEX_NORMAL, 1);

        // Setup in-game lights
        for (i = 0; i < NUM_LIGHTS; ++i) {
            light[i].type(light_data[i].type);
            light[i].translate(light_data[i].pos);
            light[i].direction(light_data[i].dir);  // Direction is already normalized
            light[i].intensity(light_data[i].inten);
            light[i].material.diffuse(light_data[i].diff);
            light[i].distance(light_data[i].distance);
            light[i].scale(glm::vec3(light_data[i].scale));
            light[i].cutoffInner(light_data[i].cutoff_in);
            light[i].cutoffOuter(light_data[i].cutoff_out);

            // Static shader uniforms (I assume them static that is)
            glUniform1i(uniformScene.light[i].type, light[i].type());
            glUniform3fv(uniformScene.light[i].diffuse, 1,
                glm::value_ptr(light[i].material.diffuse()));
            glUniform1f(uniformScene.light[i].intensity, light[i].intensity());
            glUniform1f(uniformScene.light[i].linear, light[i].linear());
            glUniform1f(uniformScene.light[i].quadratic, light[i].quadratic());

            if (light[i].type() == glf3d::SPOT) {
                glUniform1f(
                    uniformScene.light[i].cutoff_out, cosf(light[i].cutoffOuter()));
                glUniform1f(uniformScene.light[i].epsilon,
                    cosf(light[i].cutoffInner()) - cosf(light[i].cutoffOuter()));
            }
        }
        // Other static data
        glUniform1i(uniformScene.frag.lcount, NUM_LIGHTS);
        glUniform3f(uniformScene.world.amb, 0.8f, 0.8f, 0.8f);
    }

    void shutdown() {
        glDeleteTextures(1, &depthTex);
        glDeleteBuffers(1, &vBO);
        glDeleteBuffers(1, &depthBuffer);
        glDeleteVertexArrays(VAO_SZ, vAO);
    }

#define _vp(f) glm::value_ptr(f)

    void render(double elapsedTime, double duration) {
        // Update per-loop object states
        update_state(duration);
        lvMat = glm::lookAt(-5.0f * light[0].direction(), glm::vec3(0.0), kGlobalUp);
        lvpMat = lpMat * lvMat; shadowMat = scaleBiasMat * lvpMat;

        /* --------------------- Render shadow map --------------------- */
        shadow_shader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer);
        glViewport(0, 0, DEPTH_RES_WIDTH, DEPTH_RES_HEIGHT);

        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 2.0f);

        for (i = 0; i < MODEL_SZ; ++i) {
            glUniformMatrix4fv(uniformShadow.vert.lmvpMat, 1, GL_FALSE, _vp(lvpMat * model[i].modelMat()));
            model[i].render(&shader_tex_info);
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* ------------------------ Render scene ------------------------ */
        scene_shader.use();
        glViewport(0, 0, getWidth(), getHeight());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind to shadow texture uniform
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glUniform1i(uniformScene.frag.shadowTex, 3);
        glUniform3fv(uniformScene.frag.cam_pos, 1, _vp(cam.position()));

        // Send light data to shader
        for (i = 0; i < NUM_LIGHTS; ++i) {
            glUniform3fv(uniformScene.light[i].position, 1, _vp(light[i].position()));
            glUniform3fv(uniformScene.light[i].direction, 1, _vp(light[i].direction()));
        }

        // Render model
        vpMat = pMat * cam.viewMat();
        for (i = 0; i < MODEL_SZ; ++i) {
        // Model specific uniforms and transforms
            mvpMat = vpMat * model[i].modelMat();
            glm::mat3 normal_mat =
                glm::transpose(glm::inverse(glm::mat3(model[i].modelMat())));
            glUniformMatrix4fv(uniformScene.vert.modelMat, 1, GL_FALSE, _vp(model[i].modelMat()));
            glUniformMatrix4fv(uniformScene.vert.transMat, 1, GL_FALSE, _vp(mvpMat));
            glUniformMatrix3fv(uniformScene.vert.normalMat, 1, GL_FALSE, _vp(normal_mat));
            glUniformMatrix4fv(uniformScene.vert.shadowMat, 1, GL_FALSE, _vp(shadowMat * model[i].modelMat()));
            glUniform1f(uniformScene.material.amb, model[i].material.ambience());
            glUniform1f(uniformScene.material.shine, model[i].material.shininess());
            glUniform1f(uniformScene.material.specint, model[i].material.specularIntensity());
            model[i].render(&shader_tex_info);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        /* -------------------------- Render Lights --------------------- */
        light_shader.use();
        glBindVertexArray(vAO[LIGHT]);
        for (i = 0; i < NUM_LIGHTS; ++i) {
            if (light[i].type() != glf3d::POINT) continue;
            mvpMat = vpMat * light[i].modelMat();

            glUniformMatrix4fv(uniformLight.vert.transMat, 1, GL_FALSE, _vp(mvpMat));
            glUniform3fv(uniformLight.frag.col, 1, _vp(light[i].material.diffuse()));
            glDrawArrays(GL_TRIANGLES, 0, kVertsCount);
        }
        glBindVertexArray(0);
    }
#undef _vp

    // Ugly state machine
    void update_state(double duration) {
        // Handle animation
        animReg.animate(duration);
        float rad = glm::radians(light_rot.value());
        light[0].direction(glm::vec3(sin(rad), -1.0f, cos(rad)));

        // Handle keyboard input
        if (testKeyState(GLFW_KEY_W, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_FORWARD, duration);
        if (testKeyState(GLFW_KEY_S, GLFW_PRESS))
            cam.processMove(glf3d::CAM_MOVE_BACKWARD, duration);
        if (testKeyState(GLFW_KEY_A, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_LEFT, duration);
        if (testKeyState(GLFW_KEY_D, GLFW_PRESS))
            cam.processMove(glf3d::CAM_STRAFE_RIGHT, duration);

#if 0
        if (light[SPOT_IDX].type() == glf3d::SPOT) {
            light[SPOT_IDX].translate(cam.position() + kSpotLightOffset);
            light[SPOT_IDX].direction(cam.front());
        }
#endif
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
    enum MODEL { BOX, PLANE, MODEL_SZ };

    static const GLuint kVertsCount = 36;

    // OpenGL data
    GLfloat bg_col[4];
    GLuint vAO[VAO_SZ], vBO;
    GLuint depthBuffer, depthTex;
    glf::Shader scene_shader, light_shader, shadow_shader, text_shader;
    glf::ShaderTextureInfo shader_tex_info;
    GLuint i;  // Loop counter

    struct {
        struct { GLuint modelMat, transMat, normalMat, shadowMat; } vert;
        struct { GLuint cam_pos, lcount, shadowTex; } frag;
        struct { GLuint amb; } world;
        struct { GLuint shine, specint, amb; } material;

        struct {  // Shader locations for scene light params
            GLuint type, position, direction, diffuse,
                cutoff_out, epsilon, intensity, linear, quadratic;
        } light[NUM_LIGHTS];
    } uniformScene;

    struct {
        struct { GLuint col; } frag;
        struct { GLuint transMat; } vert;
    } uniformLight;

    struct {
        struct { GLuint lmvpMat; } vert;
    } uniformShadow;

    // Transform data
    glm::mat4 lpMat, lvMat, lvpMat, shadowMat, scaleBiasMat;
    glm::mat4 pMat, vpMat, mvpMat;

    // 3d objects
    glf3d::FreeCamera cam;
    glf3d::Light light[NUM_LIGHTS];
    glf3d::Model model[MODEL_SZ];

    // 3d object state data
    GLfloat mlast_x, mlast_y;
    bool first_call;

    // Animation data
    anim::AnimatorRegistry<GLfloat> animReg;
    anim::PropertyAnimator<GLfloat> light_rot;
};

DECLARE_MAIN(ShadowMap);
