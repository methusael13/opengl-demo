#include <base_util.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @author: Methusael Murmu
 */

namespace util {

extern
void set_color4f1(GLfloat* color,
    GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    color[0] = r; color[1] = g; color[2] = b;
    color[3] = a;
}

extern
void set_color4f255(GLfloat* color,
    GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    const GLfloat rmax = 255;
    set_color4f1(color,
        r / rmax, g / rmax, b / rmax, a / rmax);
}

extern
glm::vec3 color(GLuint hex) {
    GLfloat max = 0xff;
    return glm::vec3(
        ((hex >> 16) & 0xff) / max, ((hex >> 8) & 0xff) / max,
        (hex & 0xff) / max);
}

extern
glm::vec3 color_random() {
    srand(rand());
    return color((GLuint) (static_cast<double>(rand()) / RAND_MAX * 0xffffff));
}

extern
void show_fps(GLfloat fps) { printf("FPS: %f\n", fps); }

}  // namespace util
