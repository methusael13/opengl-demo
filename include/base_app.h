#ifndef __BASE_APP__
#define __BASE_APP__

/**
 * Basic application framework for OpenGL development
 * @author: Methusael Murmu
 */

#include <stdio.h>
#include <string.h>

#include <base.h>
#include <text/text.h>
#include <base_util.h>
#include <base_shader.h>

#include <external/glm/mat4x4.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtc/matrix_transform.hpp>

#define BASE_APP_TITLE_SIZE 128

namespace glf {

class BaseApp {
 public:
    BaseApp() {
        baseInit();
    }
    virtual ~BaseApp() {}

    virtual void setup() {}
    virtual void startup() {}
    virtual void shutdown() {}

    /**
     * @param elapsedTime:  Time elapsed since initialization of GLFW
     * @param duration:     Time elapsed since last call to this function
     */
    virtual void render(double elapsedTime, double duration) = 0;

    virtual void onKeyPress(int key, int mods) {}
    virtual void onKeyRelease(int key, int mods) {
        if (key == GLFW_KEY_ESCAPE)
            onWindowClose();
    }

    virtual void onMouseMove(float x, float y) {}
    virtual void onMousePress(int button, int mods) {}
    virtual void onMouseRelease(int button, int mods) {}
    virtual void onMouseWheel(int offset) {}

    virtual void onWindowClose() {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    virtual void onWindowResize(int width, int height) {
        info.width = width;
        info.height = height;
    }

    void setTitle(const char* _title) {
        strncpy(info.title, _title, BASE_APP_TITLE_SIZE - 1);
        info.title[BASE_APP_TITLE_SIZE - 1] = '\0';
    }

    void enableFPSDisplay(bool enable) { info.showFPS = enable; validateTextRender(); }
    void enableStatsDisplay(bool enable) { info.showAppInfo = enable; validateTextRender(); }
    void setBaseTextShader(glf::Shader* _textShader) { baseTextShader = _textShader; }
    void setStatsColor(const glm::vec3& _color) { baseFontRenderer.setColor(_color, baseTextShader); }

    int run(glf::BaseApp* _app) {
        if (!valid)
            return -1;

        app = _app;
        setup();

        // Prepare window
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info.majorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info.minorVersion);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_SAMPLES, info.samples);
        glfwWindowHint(GLFW_STEREO, info.flags.stereo ? GL_TRUE : GL_FALSE);

        window = glfwCreateWindow(info.width, info.height, info.title,
            info.flags.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
        if (!window) {
            fprintf(stderr, "Failed to create window\n");
            return -2;
        }
        registerCallbacks();

        glfwMakeContextCurrent(window);
        glfwSwapInterval((int) info.flags.vsync);

        if (gl3wInit()) {
            fprintf(stderr, "Failed to initialize OpenGL\n");
            return -3;
        }

        glfwSetInputMode(window, GLFW_CURSOR,
            info.flags.cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        info.flags.stereo = (glfwGetWindowAttrib(window, GLFW_STEREO) ? 1 : 0);

        // Run application
        startup();

        // Setup base app fonts
        wfac = getWidth() / 1366.0f;

        if (info.renderTexts) {
            baseFontData.loadFonts("media/data/fonts/ubuntu/Ubuntu-R.ttf", 20.0f * wfac);
            baseFontRenderer.init(&baseFontData, baseTextShader);

            // Font transforms
            baseTextShader->use();
            glm::mat4 tpMat = glm::ortho(0.0f, (GLfloat)getWidth(), 0.0f, (GLfloat)getHeight());
            // Assuming text shader has its projection uniform named pMat
            glUniformMatrix4fv(
                glGetUniformLocation(baseTextShader->getProgram(), "pMat"),
                    1, GL_FALSE, glm::value_ptr(tpMat));

            // OpenGL setup
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        double cur, prev;
        cur = prev = glfwGetTime();

        do {
            cur = glfwGetTime();
            render(cur, cur - prev);

            // Post render
            if (info.showFPS) {
                baseFPSMetric.frameUpdated(cur);
                baseFontRenderer.renderText(baseTextShader, base_fps_str, 10.0f, 10.0f, 0.8f);
            }
            if (info.showAppInfo) { renderAppInfo(); }

            glfwSwapBuffers(window);
            glfwPollEvents();
            prev = cur;
        } while (!glfwWindowShouldClose(window));

        shutdown();
        glfwTerminate();

        return 0;
    }

    // Warning: Buggy!
    // Native support for fullscreen toggle available in GLFW 3.2
    void setFullScreen(bool enable) {
        if (info.flags.fullscreen ^ enable)
            info.flags.fullscreen = enable ? 1 : 0;
        else
            return;

        // Share window context with temp_window
        GLFWwindow* temp_window = NULL;
        if (info.flags.fullscreen) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            temp_window = glfwCreateWindow(mode->width, mode->height,
                info.title, monitor, window);
        } else {
            temp_window = glfwCreateWindow(info.width, info.height,
                info.title, NULL, window);
        }

        glfwDestroyWindow(window);
        window = temp_window;
        glfwMakeContextCurrent(window);
    }

    void setVsync(bool enable) {
        if (info.flags.vsync ^ enable)
            info.flags.vsync = enable ? 1 : 0;
        else
            return;

        glfwSwapInterval((int) info.flags.vsync);
    }

    int getWidth() const { return info.width; }
    int getHeight() const { return info.height; }

    void getCursorPosition(float& x, float& y) {
        double _x, _y;
        glfwGetCursorPos(window, &_x, &_y);
        x = static_cast<float>(_x);
        y = static_cast<float>(_y);
    }

    int testKeyState(int key, int test_state) const {
        return glfwGetKey(window, key) == test_state;
    }

    struct APPINFO {
        char title[BASE_APP_TITLE_SIZE];
        int width;
        int height;
        int minorVersion;
        int majorVersion;
        int samples;  // MSAA Samples
        bool showFPS, showAppInfo;
        bool renderTexts;

        union {
            struct {
                unsigned int fullscreen : 1;
                unsigned int vsync      : 1;
                unsigned int cursor     : 1;
                unsigned int stereo     : 1;
            };
            unsigned int all;
        } flags;
    };

 protected:
    APPINFO info;
    // Required by static event handlers to delegate
    // event to appropriate instance event handlers
    static glf::BaseApp* app;

    util::FPSMetric baseFPSMetric;
    text::FontData baseFontData;
    text::FontRenderer baseFontRenderer;
    glf::Shader* baseTextShader;

 private:
    bool valid;
    GLfloat wfac;  // Width factor [width / 1366.0f]
    GLFWwindow* window;

    static char base_fps_str[15];
    static void fps_callback(GLfloat fps) {
        snprintf(base_fps_str, sizeof(base_fps_str), "FPS: %.2f", fps);
    }

    void renderAppInfo() {
        baseFontRenderer.renderText(baseTextShader, info.title, 10.0f, 740.0f * wfac, 0.7f);
        baseFontRenderer.renderText(baseTextShader, "OpenGLDemo Alpha Developer Version", 10.0f, 725.0f * wfac, 0.6f);
        baseFontRenderer.renderText(baseTextShader, "Methusael Murmu", 10.0f, 710.0f * wfac, 0.6f);
    }

    // Set to default values
    void baseInit() {
        glfwSetErrorCallback(errorCallback);

        if (!glfwInit()) {
            fprintf(stderr, "Failed to initialize GLFW\n");
            valid = false;
            return;
        }

        strcpy(info.title, "BaseApp");
        info.width = 800;
        info.height = 600;
        info.minorVersion = 3;
        info.majorVersion = 3;
        info.samples = info.renderTexts = 0;
        info.showFPS = info.showAppInfo = 0;
        info.flags.all = 0;
        info.flags.cursor = 1;

        baseFPSMetric.setCallback(fps_callback);
        baseFPSMetric.setInterval(0.4f);
        valid = true;
    }

    void validateTextRender() {
        info.renderTexts = info.showFPS || info.showAppInfo;
    }

    // API event processing callbacks
    static void keyCallback(GLFWwindow* win, int key, int scan,
        int action, int mod) {
        switch (action) {
            case GLFW_PRESS:
            case GLFW_REPEAT:
                app->onKeyPress(key, mod);
                break;
            case GLFW_RELEASE:
                app->onKeyRelease(key, mod);
                break;
        }
    }

    static void cursorPosCallback(GLFWwindow* win, double x, double y) {
        app->onMouseMove(static_cast<float>(x), static_cast<float>(y));
    }

    static void mouseButtonCallback(GLFWwindow* win, int btn,
        int action, int mods) {
        switch (action) {
            case GLFW_PRESS:
                app->onMousePress(btn, mods);
                break;
            case GLFW_RELEASE:
                app->onMouseRelease(btn, mods);
                break;
        }
    }

    static void scrollCallback(GLFWwindow* win,
        double xoffset, double yoffset) {
        app->onMouseWheel(static_cast<int>(yoffset));
    }

    static void windowResizeCallback(GLFWwindow* win, int width, int height) {
        app->onWindowResize(width, height);
    }

    static void windowCloseCallback(GLFWwindow* win) {
        app->onWindowClose();
    }

    static void errorCallback(int code, const char* desc) {
        fprintf(stderr, "%s\n", desc);
    }

    void registerCallbacks() {
        if (!window) return;

        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetWindowSizeCallback(window, windowResizeCallback);
        glfwSetWindowCloseCallback(window, windowCloseCallback);
    }
};
}  // namespace glf

#define DECLARE_MAIN(a)                     \
int main(int argc, const char* argv[]) {    \
    glf::BaseApp* apl = new a;              \
    int ret = apl->run(apl);                \
    delete apl;                             \
    return ret;                             \
}

#endif
