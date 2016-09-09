#ifndef __OBJECT__
#define __OBJECT__

/**
 * @author: Methusael Murmu
 */

#include <base.h>
#include <base_util.h>
#include <3d/material.h>
#include <3d/type_common.h>
#include <external/glm/gtc/matrix_transform.hpp>

namespace glf3d {

class Object {
 public:
    explicit Object(t_rcv3 _pos = kDefaultTranslate) {
        translate(_pos); scalef(1.0f);
        rotate(0.0f, glm::vec3(0, 1, 0));
        rotateAnchor(0.0, glm::vec3(0, 1, 0), _pos);
    }

    // Getters
    inline t_rcv3 position() {
        if (stale.all != 0) updateModelMatrix();
        return mPos;
    }

    inline t_rcv3 translate() const { return mTranslate; }
    inline t_rcm4 modelMat() {
        if (stale.all != 0) updateModelMatrix();
        return mModelMat;
    }

    // Setters
    inline void translate(t_rcv3 _pos) { mTranslate = _pos; stale.pos = true; }
    inline void scale(t_rcv3 _scale) { mScale = _scale; stale.scale = true; }
    inline void scalef(GLfloat _scale) { scale(glm::vec3(_scale)); }

    inline void rotate(GLfloat _angle, t_rcv3 _rot_axis) {
        locAngle = _angle; locRotAxis = _rot_axis;
        stale.rot = true;
    }

    inline void rotateAnchor(GLfloat _angle, t_rcv3 _rot_axis, t_rcv3 _anchor) {
        ancAngle = _angle; ancRotAxis = _rot_axis;
        ancPos = _anchor;
        stale.ancRot = true;
    }

    // Default constants
    static const t_v3 kDefaultTranslate;

 private:
    t_m4 mModelMat, mTransMat, mRotMat, mScaleMat, mAncRotMat;
    t_v3 mTranslate, mScale, locRotAxis, ancRotAxis, ancPos, mPos;
    GLfloat locAngle, ancAngle;  // Local and anchored rotation angle in radians

    union Stale {
        struct {
            bool pos    : 1;
            bool rot    : 1;
            bool scale  : 1;
            bool ancRot : 1;
        };
        unsigned int all;
    } stale;

    inline void updateModelMatrix() {
        mModelMat = getAncRotMat() * getTransMat() * getRotMat() * getScaleMat();
        // Update world position
        mPos = glm::vec3(mModelMat * glm::vec4(mTranslate, 1.0));
        stale.all = 0;
    }

    inline t_rcm4 getTransMat() {
        if (stale.pos) {
            mTransMat = t_m4(1.0f);
            mTransMat = glm::translate(mTransMat, mTranslate);
            stale.pos = false;
        }
        return mTransMat;
    }

    inline t_rcm4 getRotMat() {
        if (stale.rot) {
            mRotMat = t_m4(1.0f);
            mRotMat = glm::rotate(mRotMat, locAngle, locRotAxis);
            stale.rot = false;
        }
        return mRotMat;
    }

    inline t_rcm4 getScaleMat() {
        if (stale.scale) {
            mScaleMat = t_m4(1.0f);
            mScaleMat = glm::scale(mScaleMat, mScale);
            stale.scale = false;
        }
        return mScaleMat;
    }

    inline t_rcm4 getAncRotMat() {
        if (stale.ancRot) {
            mAncRotMat = t_m4(1.0);
            mAncRotMat = glm::translate(mAncRotMat, ancPos);
            mAncRotMat = glm::rotate(mAncRotMat, ancAngle, ancRotAxis);
            stale.ancRot = false;
        }
        return mAncRotMat;
    }
};

const t_v3 Object::kDefaultTranslate = t_v3(0.0f);

class Object3D: public Object {
 public:
    Material material;

    explicit Object3D(t_rcv3 _pos = Object::kDefaultTranslate):
        Object(_pos) {}
};

// Common render properties to be used by Objects
struct RenderContext {
    RenderContext():
        shouldDrawInstanced(false), instanceAmount(0) {}

    bool shouldDrawInstanced;
    GLuint instanceAmount;
};

#define ATTENUATION_MAP_SZ 12
const GLfloat kAttenuationMap[ATTENUATION_MAP_SZ][3] = {
    {7,   0.7,   1.8},      {13,   0.35,   0.44},
    {20,  0.22,  0.2},      {32,   0.14,   0.07},
    {50,  0.09,  0.032},    {65,   0.07,   0.017},
    {100, 0.045, 0.0075},   {160,  0.027,  0.0028},
    {200, 0.022, 0.0019},   {325,  0.014,  0.0007},
    {600, 0.007, 0.0002},   {3250, 0.0014, 0.000007}
};

enum LightType { POINT, SPOT, SUN };

class LightMaterial {
 public:
    explicit LightMaterial(t_rcv3 _diff = kDefaultDiffuse):
            mDiffuse(_diff) {}

    // Getters
    inline t_rcv3 diffuse() const { return mDiffuse; }

    // Setters
    inline void diffuse(t_rcv3 _diff) { mDiffuse = _diff; }

 private:
    t_v3 mDiffuse;

    static const t_v3 kDefaultDiffuse;
};

const t_v3 LightMaterial::kDefaultDiffuse = t_v3(1.0f);

class Light: public Object {
 public:
    explicit Light(t_rcv3 _pos = Object::kDefaultTranslate): Object(_pos),
        mDistance(kDefaultDistance), mIntensity(kDefaultIntensity),
        mType(kDefaultLightType), mDirection(kDefaultDirection),
        mCutoffOuter(kDefaultCutoffOuter), mCutoffInner(kDefaultCutoffInner) {
        stale_attenuation = true;
    }

    // Getters
    inline LightType type() const { return mType; }
    inline t_rcv3 direction() const { return mDirection; }
    inline GLfloat distance() const { return mDistance; }
    inline GLfloat intensity() const { return mIntensity; }
    inline GLfloat cutoffInner() const { return mCutoffInner; }
    inline GLfloat cutoffOuter() const { return mCutoffOuter; }

    inline GLfloat linear() {
        if (stale_attenuation) updateAttenuation();
        return mLinear;
    }
    inline GLfloat quadratic() {
        if (stale_attenuation) updateAttenuation();
        return mQuadratic;
    }

    // Setters
    inline void type(LightType _type) { mType = _type; }
    inline void direction(t_rcv3 _dir) { mDirection = glm::normalize(_dir); }
    inline void cutoffInner(GLfloat _angle) { mCutoffInner = _angle; }
    inline void cutoffOuter(GLfloat _angle) { mCutoffOuter = _angle; }
    inline void distance(GLfloat _dist) {
        if (_dist < 0) return;
        mDistance = _dist; stale_attenuation = true;
    }
    inline void intensity(GLfloat _inten) {
        mIntensity = glm::clamp<GLfloat>(_inten, 0.0f, kMaxIntensity);
    }

    LightMaterial material;

 private:
    GLfloat mLinear, mQuadratic;
    GLfloat mDistance, mIntensity;
    bool stale_attenuation;

    // Cutoff locAngle for Spotlights in radians
    GLfloat mCutoffInner, mCutoffOuter;
    LightType mType; t_v3 mDirection;

    // Default constants
    static constexpr GLfloat kDefaultDistance = 30.0f;
    static constexpr GLfloat kMaxIntensity = 100.0f;
    static constexpr GLfloat kDefaultIntensity = 1.0f;
    static constexpr GLfloat kDefaultCutoffInner = 0.52359f;  // PI / 6
    static constexpr GLfloat kDefaultCutoffOuter = 0.26179f;  // PI / 6
    static constexpr LightType kDefaultLightType = POINT;
    static const t_v3 kDefaultDirection;

    void updateAttenuation() {
        GLint idx;
        // Search for appropriate attenuation params
        for (idx = 0; idx < ATTENUATION_MAP_SZ; ++idx)
            if (mDistance <= kAttenuationMap[idx][0])
                break;

        GLfloat fraction;
        if (idx == 0) {  // Distance less than 7
            fraction = mDistance / kAttenuationMap[idx][0];
            mLinear = LINEAR_IP(fraction, 1.4f, kAttenuationMap[idx][1]);
            mQuadratic = LINEAR_IP(fraction, 3.0f, kAttenuationMap[idx][2]);
        } else if (idx == ATTENUATION_MAP_SZ) {
            mLinear = kAttenuationMap[idx-1][1];
            mQuadratic = kAttenuationMap[idx-1][2];
        } else {
            fraction = (mDistance - kAttenuationMap[idx-1][0]) /
                (kAttenuationMap[idx][0] - kAttenuationMap[idx-1][0]);
            mLinear = LINEAR_IP(fraction, kAttenuationMap[idx-1][1],
                kAttenuationMap[idx][1]);
            mQuadratic = LINEAR_IP(fraction, kAttenuationMap[idx-1][2],
                kAttenuationMap[idx][2]);
        }

        stale_attenuation = false;
    }
};

const t_v3 Light::kDefaultDirection = t_v3(0.0f, -1.0f, 0.0f);

}  // namespace glf3d

#endif
