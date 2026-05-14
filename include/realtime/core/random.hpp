#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdlib>

namespace realtime {

inline float randf(float a, float b){
    return a + (b-a)*float(rand())/float(RAND_MAX);
}

inline float clampf(float v, float lo, float hi){
    return v<lo?lo:(v>hi?hi:v);
}

}

#endif