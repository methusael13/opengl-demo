#ifndef __ANIM_H__
#define __ANIM_H__

/**
 * Animation framework
 * @author: Methusael Murmu
 */

#include <base_util.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <external/glm/common.hpp>
#include <external/glm/gtc/constants.hpp>

#include <algorithm>
#include <vector>

namespace anim {

template <typename T>
class PropertyContext {
 public:
    PropertyContext(T _init, T _final, float _duration) {
        set(_init, _final, _duration);
    }

    PropertyContext(const PropertyContext& ref) {
        set(ref.init, ref.final, ref.duration);
    }

    void set(T _init, T _final, float _duration) {
        init = _init; final = _final;
        duration = _duration;
    }

 public:
    T init, final;
    float duration;
};

enum InterpolatorType { LINEAR, SINE, COSINE };
enum AnimatorStatus { STOPPED, PAUSED, RUNNING };
enum AnimatorLoopType { LOOP_ONCE, LOOP_START, LOOP_REVERSE };

template <typename T>
class Interpolator {
 public:
    Interpolator() { context = NULL; }

    void setPropertyContext(PropertyContext<T>* _context) {
        context = _context;
    }

    virtual void interpolate(T& val, float frac) = 0;

 private:
    Interpolator(const Interpolator& ref) {}

 protected:
    PropertyContext<T>* context;
};

#define _ctxt Interpolator<T>::context

template <typename T>
class LinearInterpolator: public Interpolator<T> {
 public:
    void interpolate(T& val, float frac) {
        val = LINEAR_IP(frac, _ctxt->init, _ctxt->final);
    }
};

template <typename T>
class SineInterpolator: public Interpolator<T> {
 public:
    void interpolate(T& val, float frac) {
        float val_frac = (sin(frac * util::Constants::PI_TWO) + 1) / 2.0f;
        val = LINEAR_IP(val_frac, _ctxt->init, _ctxt->final);
    }
};

template <typename T>
class CosineInterpolator: public Interpolator<T> {
 public:
    void interpolate(T& val, float frac) {
        float val_frac = (cos(frac * util::Constants::PI_TWO) + 1) / 2.0f;
        val = LINEAR_IP(val_frac, _ctxt->init, _ctxt->final);
    }
};

#undef _ctxt

template <typename T>
class InterpolatorFactory {
 public:
    static Interpolator<T>* create(InterpolatorType _type) {
        switch (_type) {
            case LINEAR:
                return new LinearInterpolator<T>;
            case SINE:
                return new SineInterpolator<T>;
            case COSINE:
                return new CosineInterpolator<T>;
            default: return NULL;
        }
    }
};

template <typename T>
class PropertyAnimator {
 public:
    enum INC_FACTOR { FORWARD = 1, BACKWARD = -1 };

    PropertyAnimator() {
        construtor(0, 1, 1);
    }

    PropertyAnimator(T _init, T _final, float _duration) {
        construtor(_init, _final, _duration);
    }

    PropertyAnimator(const PropertyAnimator& ref) {
        size_t sz = sizeof(*ref.interpolator);
        Interpolator<T>* new_ip = static_cast<Interpolator<T>*>(malloc(sz));
        memcpy(new_ip, ref.interpolator, sz);

        init(ref.val, ref.elapsedTime, ref.loopType, ref.incFactor,
             ref.stat, new PropertyContext<T>(*ref.context), new_ip);
    }

    virtual ~PropertyAnimator() {
        delete context;
        delete interpolator;
    }

    const PropertyAnimator<T>& operator=(PropertyAnimator<T> pc) {
        val = pc.val; elapsedTime = pc.elapsedTime;
        loopType = pc.loopType; incFactor = pc.incFactor;
        stat = pc.stat;

        std::swap(this->context, pc.context);
        std::swap(this->interpolator, pc.interpolator);
        return *this;
    }

    inline const T value() const { return val; }
    inline AnimatorStatus status() const { return stat; }

    void play() { stat = RUNNING; }
    void pause() { stat = PAUSED; }
    void stop() { stat = STOPPED; reset(); }

    void set(T _init, T _final, float _duration) {
        context->set(_init, _final, _duration);
    }

    void setLoopType(AnimatorLoopType type) {
        loopType = type;
    }

    void setInterpolator(Interpolator<T>* _interpolator) {
        if (interpolator == _interpolator ||
            _interpolator == NULL)
            return;

        if (interpolator != NULL) { delete interpolator; }

        interpolator = _interpolator;
        interpolator->setPropertyContext(context);
    }

    void step(float tp) {
        if (stat != RUNNING)
            return;

        interpolator->interpolate(val, elapsedTime / context->duration);
        switch (loopType) {
            case LOOP_ONCE:
                if (elapsedTime >= context->duration)
                    stop();
                break;

            case LOOP_REVERSE:
                if (elapsedTime >= context->duration && incFactor == FORWARD)
                    incFactor = BACKWARD;
                else if (elapsedTime <= 0 && incFactor == BACKWARD)
                    incFactor = FORWARD;
                break;
        }

        elapsedTime += incFactor * tp;
        elapsedTime = loopType != LOOP_START ?
            glm::clamp<float>(elapsedTime, 0, context->duration) :
                fmod(elapsedTime, context->duration);
    }

 private:
    float elapsedTime;

    T val;
    AnimatorStatus stat;
    AnimatorLoopType loopType;
    INC_FACTOR incFactor;

    PropertyContext<T>* context;
    Interpolator<T>* interpolator;

    // Workaround for constructor delegation
    void construtor(T _init, T _final, float _duration) {
        init(_init, 0, LOOP_START, FORWARD, RUNNING,
             new PropertyContext<T>(_init, _final, _duration),
             InterpolatorFactory<T>::create(LINEAR));
    }

    void init(T _value, float _time, AnimatorLoopType _loop,
        INC_FACTOR _fac, AnimatorStatus _status, PropertyContext<T>* pc,
        Interpolator<T>* ip) {
        // Say NO to bad initial pointers
        context = NULL; interpolator = NULL;

        val = _value; elapsedTime = _time;
        loopType = _loop; incFactor = _fac;
        stat = _status;

        context = pc; setInterpolator(ip);
    }

    void reset() {
        elapsedTime = 0; incFactor = FORWARD;
    }
};

template <typename T>
class AnimatorRegistry {
 private:
    std::vector<PropertyAnimator<T>*> animators;
    typename std::vector<PropertyAnimator<T>*>::iterator itr;

 public:
    AnimatorRegistry() {}

    #define LOOP_ITR(v) \
        for (v = animators.begin(); v != animators.end(); ++v)

    void add(PropertyAnimator<T>* anim) {
        animators.push_back(anim);
    }

    PropertyAnimator<T> remove(PropertyAnimator<T>* anim) {
        LOOP_ITR(itr) {
            if (*itr == anim) { break; }
        }

        if (itr != animators.end())
            animators.erase(itr);
    }

    void clear() { animators.clear(); }

    void animate(float duration) {
        LOOP_ITR(itr) { (*itr)->step(duration); }
    }

    void stop() { LOOP_ITR(itr) { (*itr)->stop(); } }
    void play() { LOOP_ITR(itr) { (*itr)->play(); } }
    void pause() { LOOP_ITR(itr) { (*itr)->pause(); } }

    #undef LOOP_ITR

 private:
    AnimatorRegistry(const AnimatorRegistry& ref) {}
    const AnimatorRegistry<T>& operator=(const AnimatorRegistry<T>& rhs) {}
};

}  // namespace anim

#endif
