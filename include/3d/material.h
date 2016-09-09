#ifndef __MATERIAL__
#define __MATERIAL__

#include <base.h>
#include <3d/type_common.h>

namespace glf3d {

class Material {
 public:
    explicit Material(t_rcv3 _color = kDiffuseColor) {
        mDiffuse = _color; mSpecular = kSpecularColor;
        mShininess = 10.0f; mAmbience = 0.1f; mSpecIntensity = 1.0f;
    }

    // Getters
    inline t_rcv3 diffuse() const { return mDiffuse; }
    inline t_rcv3 specular() const { return mSpecular; }
    inline GLfloat shininess() const { return mShininess; }
    inline GLfloat ambience() const { return mAmbience; }
    inline GLfloat specularIntensity() const { return mSpecIntensity; }

    // Setters
    inline void diffuse(t_rcv3 _color) { mDiffuse = _color; }
    inline void specular(t_rcv3 _color) { mSpecular = _color; }

    inline void ambience(GLfloat _amb) {
        mAmbience = glm::clamp<GLfloat>(_amb, 0.0f, 1.0f);
    }
    inline void shininess(GLfloat _shine) {
        mShininess = glm::clamp<GLfloat>(_shine, 1.0f, 500.0f);
    }
    inline void specularIntensity(GLfloat _spec) {
        mSpecIntensity = glm::clamp<GLfloat>(_spec, 0.0f, 1.0f);
    }

 private:
    t_v3 mDiffuse, mSpecular;
    GLfloat mAmbience, mSpecIntensity, mShininess;

    // Default constants
    static const t_v3 kDiffuseColor;
    static const t_v3 kSpecularColor;
};

const t_v3 Material::kDiffuseColor = t_v3(0.2f);  // Dark Grey
const t_v3 Material::kSpecularColor = t_v3(1.0f);  // White

}  // namespace glf3d

#endif
