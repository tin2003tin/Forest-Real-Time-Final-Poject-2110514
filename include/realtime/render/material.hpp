#ifndef REALTIME_MATERIAL_HPP
#define REALTIME_MATERIAL_HPP

#include "realtime/core/vec.hpp"

namespace realtime {

struct Material
{
    Vec<3> ka = {0.15f,0.15f,0.15f}; // ambient
    Vec<3> kd = {0.8f,0.5f,0.5f};    // diffuse
    Vec<3> ks = {1.0f,1.0f,1.0f};    // specular
    float shininess = 32.0f;
};

namespace Materials
{

/* Basic */

inline const Material Default =
{
    {0.15f,0.15f,0.15f},
    {0.8f,0.5f,0.5f},
    {1.0f,1.0f,1.0f},
    32.0f
};

/* Plastic */

inline const Material PlasticRed =
{
    {0.1f,0.0f,0.0f},
    {0.8f,0.1f,0.1f},
    {0.5f,0.5f,0.5f},
    32.0f
};

inline const Material PlasticBlue =
{
    {0.0f,0.0f,0.1f},
    {0.1f,0.1f,0.8f},
    {0.5f,0.5f,0.5f},
    32.0f
};

/* Metals */

inline const Material Gold =
{
    {0.247f,0.199f,0.074f},
    {0.751f,0.606f,0.226f},
    {0.628f,0.556f,0.366f},
    64.0f
};

inline const Material Silver =
{
    {0.192f,0.192f,0.192f},
    {0.507f,0.507f,0.507f},
    {0.508f,0.508f,0.508f},
    64.0f
};

inline const Material Copper =
{
    {0.191f,0.073f,0.022f},
    {0.703f,0.270f,0.082f},
    {0.257f,0.137f,0.086f},
    32.0f
};

/* Gems */

inline const Material Emerald =
{
    {0.021f,0.174f,0.021f},
    {0.076f,0.614f,0.076f},
    {0.633f,0.727f,0.633f},
    76.8f
};

inline const Material Ruby =
{
    {0.174f,0.011f,0.011f},
    {0.614f,0.041f,0.041f},
    {0.728f,0.627f,0.627f},
    76.8f
};

}

}

#endif
