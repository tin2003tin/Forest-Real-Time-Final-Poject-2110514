#ifndef NOISE_HPP
#define NOISE_HPP

#include <cmath>
#include "realtime/core/random.hpp"

namespace realtime {

inline float _hn(float n){
    float s = std::sin(n) * 43758.5453f;
    return s - std::floor(s);
}

inline float vnoise(float x){
    float i = std::floor(x);
    float f = x - i;

    f = f * f * (3 - 2 * f);

    return _hn(i) * (1 - f) + _hn(i + 1) * f;
}

inline float wnoise(float x, float seed = 0){
    return vnoise(x * 0.3f + seed) * 0.65f +
           vnoise(x * 0.8f + seed + 31.7f) * 0.35f;
}


}
#endif