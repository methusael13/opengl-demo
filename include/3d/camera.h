#ifndef __CAMERA__
#define __CAMERA__

/**
 * Camera classes
 * @author: Methusael Murmu
 */

#include <base.h>
#include <3d/type_common.h>
#include <external/glm/gtc/matrix_transform.hpp>

namespace glf3d {

enum Command {
    CAM_MOVE_FORWARD, CAM_MOVE_BACKWARD,
    CAM_STRAFE_LEFT, CAM_STRAFE_RIGHT
};

class FreeCamera {
 public:
    FreeCamera() {
        _init(t_v3(0.0f, 0.0f,  1.0f),
              t_v3(0.0f, 0.0f, -1.0f), sFOV);
    }

    FreeCamera(t_rcv3 _pos, t_rcv3 _local_front, GLfloat _fov = sFOV) {
        _init(_pos, _local_front, _fov);
    }

    // Getters
    inline t_rcv3 position() const { return mPos; }
    inline t_rcv3 front() const { return mLocalFront; }

    t_rcv3 right() {
        if (stale_axes) updateAxisOrientation();
        return mLocalRight;
    }

    t_rcv3 up() {
        if (stale_axes) updateAxisOrientation();
        return mLocalUp;
    }

    t_rcm4 viewMat() {
        if (stale_view || stale_axes) updateViewMatrix();
        return mViewMat;
    }

    inline GLfloat fov() const          { return mFov; }
    inline GLfloat speed() const        { return mCamSpeed; }
    inline GLfloat sensitivity() const  { return mSensitivity; }
    inline GLfloat near() const         { return mNear; }
    inline GLfloat far() const          { return mFar; }

    // Setters
    inline void position(t_rcv3 _pos) { mPos = _pos; stale_view = true; }
    inline void front(t_rcv3 _front) {
        mLocalFront = glm::normalize(_front);
        stale_axes = true;
    }

    inline void fov(GLfloat _fov)           { mFov = _fov; }
    inline void speed(GLfloat _speed)       { mCamSpeed = _speed; }
    inline void sensitivity(GLfloat _sen)   { mSensitivity = _sen; }
    inline void near(GLfloat _near)         { mNear = _near; }
    inline void far(GLfloat _far)           { mFar = _far; }

    inline void processMove(Command com, double duration) {
        GLfloat _duration = duration;

        switch (com) {
            case CAM_MOVE_FORWARD:
                mPos += front() * mCamSpeed * _duration;
                break;
            case CAM_MOVE_BACKWARD:
                mPos -= front() * mCamSpeed * _duration;
                break;
            case CAM_STRAFE_RIGHT:
                mPos += right() * mCamSpeed * _duration;
                break;
            case CAM_STRAFE_LEFT:
                mPos -= right() * mCamSpeed * _duration;
                break;
            default: return;
        }

        stale_view = true;
    }

    void processLook(GLfloat mouse_offset_x, GLfloat mouse_offset_y,
                     bool limit_pitch = true) {
        yaw   += mouse_offset_x * mSensitivity;
        pitch += mouse_offset_y * mSensitivity;

        yaw = fmod(yaw, 360.0f);
        if (limit_pitch)
            pitch = glm::clamp<GLfloat>(pitch, -89.0f, 89.0f);

        GLfloat yaw_rad = glm::radians(yaw),
                pitch_rad = glm::radians(pitch);
        double cos_pitch = cos(pitch_rad);

        // Already normalized
        mLocalFront.x =  cos_pitch * sin(yaw_rad);
        mLocalFront.y =  sin(pitch_rad);
        mLocalFront.z = -cos_pitch * cos(yaw_rad);

        mLocalFront = glm::normalize(mLocalFront);
        stale_axes = true;
    }

 private:
    // Loc and orientation data
    t_v3 mPos, mLocalFront;
    t_v3 mLocalUp, mLocalRight;
    t_m4 mViewMat;

    // Camera parameters
    GLfloat mNear, mFar;
    GLfloat mFov, yaw, pitch;  // FOV in radians
    GLfloat mCamSpeed, mSensitivity;

    bool stale_view, stale_axes;

    // Default constants
    static const t_v3 sGLOBAL_UP;
    static const GLfloat sFOV, sCAM_SPEED;
    static const GLfloat sSENSITIVITY;

    void _init(t_rcv3 _pos, t_rcv3 _local_front, GLfloat _fov) {
        mFov = _fov; yaw = pitch = 0.0f;
        mPos = _pos; mLocalFront = _local_front;
        mCamSpeed = sCAM_SPEED; mSensitivity = sSENSITIVITY;

        stale_view = stale_axes = true;
    }

    void updateAxisOrientation() {
        mLocalRight = glm::normalize(glm::cross(mLocalFront, sGLOBAL_UP));
        mLocalUp = glm::normalize(glm::cross(mLocalRight, mLocalFront));
        stale_axes = false;
    }

    void updateViewMatrix() {
        mViewMat = glm::lookAt(mPos, mPos + mLocalFront, sGLOBAL_UP);
        stale_view = false;
    }
};

}  // namespace glf3d

#endif
