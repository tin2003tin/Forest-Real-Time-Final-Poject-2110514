#ifndef REALTIME_VEC_HPP
#define REALTIME_VEC_HPP

#include <cmath>
#include <initializer_list>

namespace realtime {

/* ============================================================
   Generic Vector
   ============================================================ */

template<size_t N>
class Vec
{
public:

    alignas(32) float data[N];

    /* ================= Constructors ================= */

    constexpr Vec()
    {
        for(size_t i = 0; i < N; ++i)
            data[i] = 0.0f;
    }

    constexpr explicit Vec(float v)
    {
        for(size_t i = 0; i < N; ++i)
            data[i] = v;
    }

    Vec(std::initializer_list<float> list)
    {
        size_t i = 0;

        for(float v : list)
        {
            if(i < N)
                data[i++] = v;
        }

        for(; i < N; ++i)
            data[i] = 0.0f;
    }

    /* ================= Access ================= */

    constexpr float& operator[](size_t i) noexcept
    {
        return data[i];
    }

    constexpr const float& operator[](size_t i) const noexcept
    {
        return data[i];
    }
};

/* ============================================================
   Type Aliases
   ============================================================ */

using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;

/* ============================================================
   Vec + Vec
   ============================================================ */

template<size_t N>
inline Vec<N> operator+(const Vec<N>& a, const Vec<N>& b)
{
    Vec<N> r;

    for(size_t i = 0; i < N; ++i)
        r[i] = a[i] + b[i];

    return r;
}

/* ============================================================
   Vec - Vec
   ============================================================ */

template<size_t N>
inline Vec<N> operator-(const Vec<N>& a, const Vec<N>& b)
{
    Vec<N> r;

    for(size_t i = 0; i < N; ++i)
        r[i] = a[i] - b[i];

    return r;
}

/* ============================================================
   Unary Minus
   ============================================================ */

template<size_t N>
inline Vec<N> operator-(const Vec<N>& v)
{
    Vec<N> r;

    for(size_t i = 0; i < N; ++i)
        r[i] = -v[i];

    return r;
}

/* ============================================================
   Vec * Scalar
   ============================================================ */

template<size_t N>
inline Vec<N> operator*(const Vec<N>& v, float s)
{
    Vec<N> r;

    for(size_t i = 0; i < N; ++i)
        r[i] = v[i] * s;

    return r;
}

template<size_t N>
inline Vec<N> operator*(float s, const Vec<N>& v)
{
    return v * s;
}

/* ============================================================
   Component-wise Multiply
   ============================================================ */

template<size_t N>
inline Vec<N> operator*(const Vec<N>& a, const Vec<N>& b)
{
    Vec<N> r;

    for(size_t i = 0; i < N; ++i)
        r[i] = a[i] * b[i];

    return r;
}

/* ============================================================
   Dot Product
   ============================================================ */

template<size_t N>
inline float dot(const Vec<N>& a, const Vec<N>& b)
{
    float r = 0.0f;

    for(size_t i = 0; i < N; ++i)
        r += a[i] * b[i];

    return r;
}

/* ============================================================
   Length
   ============================================================ */

template<size_t N>
inline float length(const Vec<N>& v)
{
    return std::sqrt(dot(v, v));
}

/* ============================================================
   Normalize
   ============================================================ */

template<size_t N>
inline Vec<N> normalize(const Vec<N>& v)
{
    float len = length(v);

    if(len == 0.0f)
        return Vec<N>();

    Vec<N> r;

    float inv = 1.0f / len;

    for(size_t i = 0; i < N; ++i)
        r[i] = v[i] * inv;

    return r;
}

/* ============================================================
   Cross Product (Vec3 only)
   ============================================================ */

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
    Vec3 r;

    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];

    return r;
}

} // namespace realtime

#endif