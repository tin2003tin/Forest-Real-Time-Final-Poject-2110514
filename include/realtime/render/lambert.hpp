#ifndef REALTIME_LAMBERT_HPP
#define REALTIME_LAMBERT_HPP

#include "realtime/core/vec.hpp"
#include "realtime/render/material.hpp"
#include <algorithm>

namespace realtime {

inline Vec<3> lambert_shading(
    const Vec<3>& normal,
    const Vec<3>& light_dir,
    const realtime::Material& mat
)
{
    Vec<3> N = normalize<3>(normal);
    Vec<3> L = normalize<3>(light_dir);

    float NdotL = std::max(dot<3>(N,L),0.0f);

    Vec<3> ambient = mat.ka;
    Vec<3> diffuse = mat.kd * NdotL;

    return ambient + diffuse;
}

}

#endif
