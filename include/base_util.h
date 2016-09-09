#ifndef __UTIL_H__
#define __UTIL_H__

/**
 * @author: Methusael Murmu
 */

#include <base.h>
#include <external/glm/vec3.hpp>

#include <stdio.h>
#include <string>
#include <sstream>

namespace util {

void set_color4f1(GLfloat* color,
    GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);

void set_color4f255(GLfloat* color,
    GLfloat r, GLfloat g, GLfloat b, GLfloat a = 255.0f);

glm::vec3 color_random();
glm::vec3 color(GLuint hex);

// Default fpsCallback
void show_fps(GLfloat fps);

class FPSMetric {
    typedef void (*fpsCallback)(GLfloat);

 public:
    FPSMetric():
        func_callback(show_fps), frame_count(0),
        fps_disp_count(1), fps_interval(kFPSInterval) {}

    // FPS update interval. @see kFPSInterval
    void setInterval(GLfloat _fps_interval) { fps_interval = _fps_interval; }
    // Function to call in event of updated FPS metric
    void setCallback(fpsCallback callback) { func_callback = callback; }

    void frameUpdated(GLdouble elapsedTime) {
        frame_count++;

        if (elapsedTime > fps_disp_count * fps_interval) {
            if (func_callback != NULL) {
                func_callback(frame_count /
                    (elapsedTime - (fps_disp_count - 1) * fps_interval));
            }
            fps_disp_count++; frame_count = 0;
        }
    }

 private:
    fpsCallback func_callback;

    uint64_t frame_count, fps_disp_count;
    GLfloat fps_interval;

    // Interval in seconds between consecutive FPS updates
    static constexpr GLfloat kFPSInterval = 5.0f;
};

inline std::string itos(int num) {
    std::stringstream ostr;
    ostr << num; return ostr.str();
}

inline GLint uni_loc_i(GLuint prog, const char* type,
                      const char* member, GLint idx) {
    std::string str = type;
    str = str + "[" + itos(idx) + "]." + member;
    return glGetUniformLocation(prog, str.c_str());
}

inline GLint uni_loc_i(GLuint prog, const char* member, GLint idx) {
    std::string str = member;
    str = str + "[" + itos(idx) + "]";
    return glGetUniformLocation(prog, str.c_str());
}

#define LINEAR_IP(fr, in, fi) ((1 - (fr)) * (in) + (fr) * (fi))

struct Constants {
    static constexpr double PI          = 3.14159265359;
    static constexpr double PI_TWO      = 6.28318530718;
    static constexpr double PI_HALF     = 1.57079632679;
    static constexpr double PI_QUARTER  = 0.78539816339;
    static constexpr double PI_3QUARTER = 2.35619449019;
};

}  // namespace util

#endif
