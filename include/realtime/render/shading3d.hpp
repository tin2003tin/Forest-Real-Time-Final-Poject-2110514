#ifndef REALTIME_SHADING3D_HPP
#define REALTIME_SHADING3D_HPP

#include "realtime/core/vec.hpp"
#include "realtime/render/blinn_phong.hpp"
#include  "realtime/render/lambert.hpp"

namespace realtime {

enum class ShadingType
{
    Lambert,
    BlinnPhong
};

inline Vec<3> shade3d(
    ShadingType type,
    const Vec<3>& normal,
    const Vec<3>& light,
    const Vec<3>& view,
    const Material& mat
)
{
    switch(type)
    {
        case ShadingType::Lambert:
            return lambert_shading(normal, light, mat);

        case ShadingType::BlinnPhong:
            return blinn_phong_shading(normal, light, view, mat);
    }

    return mat.ka;
}

}

#endif
