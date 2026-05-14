// wind.hpp
#ifndef WIND_HPP
#define WIND_HPP

#include <cmath>
#include "realtime/core/random.hpp"
#include "realtime/core/noise.hpp"

namespace realtime {

// ──────────────────────────────────────────────────────────────
// Wind System
// ──────────────────────────────────────────────────────────────
struct WindSystem {
    bool enabled = true;
    float timer = 0;

    float gust = 0;
    float gust_t = 0;
    float next_gust = 0;

    float wx = 0;
    float wy = 0;

    float gx = 0;
    float gy = 0;

  void update(float dt){
    if(!enabled){

        wx = 0;
        wy = 0;
        gust = 0;

        return;
    }
    timer += dt;

    // ─────────────────────────────────────
    // BASE WIND
    // ─────────────────────────────────────
    wx =
        8.0f *
        wnoise(timer * 0.35f, 0.0f)
        - 4.0f;

    wy =
        6.0f *
        wnoise(timer * 0.32f, 50.0f)
        - 3.0f;

    // ─────────────────────────────────────
    // NORMAL GUSTS
    // ─────────────────────────────────────
    next_gust -= dt;

    if(next_gust <= 0){

        next_gust =
            randf(3.f, 9.f);

        gust_t = 0;

        gx = randf(-1,1);
        gy = randf(-1,1);

        float gl =
            std::sqrt(
                gx*gx +
                gy*gy
            ) + 1e-6f;

        gx /= gl;
        gy /= gl;
    }

    gust_t += dt;

    float gust_duration = 1.8f;

    gust =
    (
        gust_t < gust_duration
    )
    ?
    std::sin(
        gust_t /
        gust_duration *
        3.14159f
    )
    :
    0.0f;

    // ─────────────────────────────────────
    // STRONG WIND EVENT
    // ─────────────────────────────────────

    static float storm_timer = 0;
    static float storm_wait  = 12.0f;

    static float storm_x = 0;
    static float storm_y = 0;

    storm_wait -= dt;

    // start storm
    if(storm_wait <= 0){

        storm_wait =
            randf(18.f, 35.f);

        storm_timer = 5.0f;

        storm_x = randf(-1,1);
        storm_y = randf(-1,1);

        float sl =
            std::sqrt(
                storm_x*storm_x +
                storm_y*storm_y
            ) + 1e-6f;

        storm_x /= sl;
        storm_y /= sl;
    }

    // active storm
    if(storm_timer > 0){

        storm_timer -= dt;

        // smooth envelope
        float k =
            std::sin(
                (1.0f -
                storm_timer/5.0f)
                * 3.14159f
            );

        float storm_strength = 45.0f;

        wx +=
            storm_x *
            storm_strength *
            k;

        wy +=
            storm_y *
            storm_strength *
            k;
    }

    // ─────────────────────────────────────
    // APPLY NORMAL GUST
    // ─────────────────────────────────────
    float gust_strength = 20.0f;

    wx +=
        gx *
        gust *
        gust_strength;

    wy +=
        gy *
        gust *
        gust_strength;
}

    float total_x() const { return wx; }
    float total_y() const { return wy; }
};

// Global wind object
inline WindSystem g_wind;

} // namespace realtime

#endif