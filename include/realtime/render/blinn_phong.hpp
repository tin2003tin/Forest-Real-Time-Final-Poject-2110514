#ifndef REALTIME_BLINN_PHONG_HPP
#define REALTIME_BLINN_PHONG_HPP

#include "realtime/core/vec.hpp"
#include "realtime/render/material.hpp"
#include <algorithm>
#include <cmath>

namespace realtime {

inline Vec<3> blinn_phong_shading(
    const Vec<3>& normal,
    const Vec<3>& light_dir,
    const Vec<3>& view_dir,
    const realtime::Material& mat
)
{
    Vec<3> N = normalize<3>(normal);
    Vec<3> L = normalize<3>(light_dir);
    Vec<3> V = normalize<3>(view_dir);

    float NdotL = std::max(dot<3>(N,L),0.0f);

    Vec<3> ambient = mat.ka;
    Vec<3> diffuse = mat.kd * NdotL;

    Vec<3> H = normalize<3>(L + V);
    float NdotH = std::max(dot<3>(N,H),0.0f);

    float spec = std::pow(NdotH, mat.shininess);

    Vec<3> specular = mat.ks * spec;

    return ambient + diffuse + specular;
}

}

#endif
