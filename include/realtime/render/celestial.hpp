#pragma once

#include <cmath>
#include <GL/glut.h>


#include "realtime/environment/daycycle.hpp"

inline void draw_sun(){

    float sun_el = realtime::g_day.sun_elevation();

    if(sun_el < -0.08f)
        return;

    float sun_az = realtime::g_day.sun_azimuth();

    float dist = 1800.0f;

    float sx = std::cos(sun_el)*std::cos(sun_az)*dist;
    float sy = std::cos(sun_el)*std::sin(sun_az)*dist;
    float sz = std::sin(sun_el)*dist;

    float sr, sg, sb;
    realtime::g_day.sun_color(sr, sg, sb);

    float fade =
        realtime::clampf(
            (sun_el + 0.08f) / 0.18f,
            0.0f,
            1.0f
        );

    // Billboard vectors
    float nx = sx/dist;
    float ny = sy/dist;
    float nz = sz/dist;

    float rx = -ny;
    float ry = nx;
    float rz = 0.0f;

    float rl = std::sqrt(rx*rx + ry*ry);

    if(rl < 0.001f){
        rx = 1;
        ry = 0;
        rl = 1;
    }

    rx /= rl;
    ry /= rl;

    float ux = ry*nz-rz*ny;
    float uy = rz*nx-rx*nz;
    float uz = rx*ny-ry*nx;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const int S = 64;
    float pi2 = 2.0f*(float)M_PI;

    // Halo
    float halo_r = 420.0f;

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(sr, sg*0.75f, sb*0.3f, 0.12f*fade);
    glVertex3f(sx,sy,sz);

    glColor4f(sr, sg*0.5f, 0.0f, 0.0f);

    for(int i=0;i<=S;i++){

        float a  = pi2*i/S;
        float ca = std::cos(a);
        float sa = std::sin(a);

        glVertex3f(
            sx + (rx*ca+ux*sa)*halo_r,
            sy + (ry*ca+uy*sa)*halo_r,
            sz + (rz*ca+uz*sa)*halo_r
        );
    }

    glEnd();

    // Disk
    float disk_r = 72.0f;

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(1.0f, 1.0f, 0.9f, 1.0f*fade);
    glVertex3f(sx,sy,sz);

    glColor4f(sr, sg, sb, 0.0f);

    for(int i=0;i<=S;i++){

        float a  = pi2*i/S;
        float ca = std::cos(a);
        float sa = std::sin(a);

        glVertex3f(
            sx + (rx*ca+ux*sa)*disk_r,
            sy + (ry*ca+uy*sa)*disk_r,
            sz + (rz*ca+uz*sa)*disk_r
        );
    }

    glEnd();

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
}

inline void draw_moon(){

 // Moon is opposite the sun
    float moon_el =
        -realtime::g_day.sun_elevation();

    // Only visible at night
    if(moon_el < -0.15f)
        return;

    float moon_az =
        realtime::g_day.sun_azimuth() +
        (float)M_PI;

    float dist = 1700.0f;

    float mx =
        std::cos(moon_el) *
        std::cos(moon_az) * dist;

    float my =
        std::cos(moon_el) *
        std::sin(moon_az) * dist;

    float mz =
        std::sin(moon_el) * dist;

    // Fade in/out near horizon
    float fade =
        realtime::clampf(
            (moon_el + 0.15f) / 0.25f,
            0.0f,
            1.0f
        );

    // Billboard vectors
    float nx = mx/dist;
    float ny = my/dist;
    float nz = mz/dist;

    float rx = -ny;
    float ry = nx;
    float rz = 0.0f;

    float rl =
        std::sqrt(rx*rx + ry*ry);

    if(rl < 0.001f){
        rx = 1;
        ry = 0;
        rl = 1;
    }

    rx /= rl;
    ry /= rl;

    float ux = ry*nz-rz*ny;
    float uy = rz*nx-rx*nz;
    float uz = rx*ny-ry*nx;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);

    glEnable(GL_BLEND);
    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    const int S = 64;
    float pi2 = 2.0f * (float)M_PI;

    // ── Moon glow ──────────────────────────
    float glow_r = 220.0f;

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(
        0.7f,
        0.8f,
        1.0f,
        0.10f * fade
    );

    glVertex3f(mx,my,mz);

    glColor4f(
        0.7f,
        0.8f,
        1.0f,
        0.0f
    );

    for(int i=0;i<=S;i++){

        float a  = pi2*i/S;
        float ca = std::cos(a);
        float sa = std::sin(a);

        glVertex3f(
            mx + (rx*ca+ux*sa)*glow_r,
            my + (ry*ca+uy*sa)*glow_r,
            mz + (rz*ca+uz*sa)*glow_r
        );
    }

    glEnd();

    // ── Moon disk ──────────────────────────
    float disk_r = 48.0f;

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(
        0.92f,
        0.95f,
        1.0f,
        1.0f * fade
    );

    glVertex3f(mx,my,mz);

    glColor4f(
        0.75f,
        0.80f,
        0.95f,
        0.0f
    );

    for(int i=0;i<=S;i++){

        float a  = pi2*i/S;
        float ca = std::cos(a);
        float sa = std::sin(a);

        glVertex3f(
            mx + (rx*ca+ux*sa)*disk_r,
            my + (ry*ca+uy*sa)*disk_r,
            mz + (rz*ca+uz*sa)*disk_r
        );
    }

    glEnd();

    // ── Crescent shadow ────────────────────
    float crescent_offset = 14.0f;

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(
        0.02f,
        0.03f,
        0.05f,
        0.95f * fade
    );

    glVertex3f(
        mx + rx*crescent_offset,
        my + ry*crescent_offset,
        mz
    );

    for(int i=0;i<=S;i++){

        float a  = pi2*i/S;
        float ca = std::cos(a);
        float sa = std::sin(a);

        glVertex3f(
            mx + rx*crescent_offset +
            (rx*ca+ux*sa)*disk_r*0.95f,

            my + ry*crescent_offset +
            (ry*ca+uy*sa)*disk_r*0.95f,

            mz + (rz*ca+uz*sa)*disk_r*0.95f
        );
    }

    glEnd();

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
}