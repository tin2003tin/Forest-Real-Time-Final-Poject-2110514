#pragma once

#include <GL/glut.h>
#include <cmath>

#include "realtime/environment/daycycle.hpp"
#include <algorithm>

void update_lighting(){

    float sr,sg,sb;
    realtime::g_day.sun_color(sr,sg,sb);

    float ar,ag,ab;
    realtime::g_day.ambient_color(ar,ag,ab);

    float sun_el = realtime::g_day.sun_elevation();
    float sun_az = realtime::g_day.sun_azimuth();

    float sun_pos[] = {

        std::cos(sun_el)*std::cos(sun_az)*400.0f,
        std::cos(sun_el)*std::sin(sun_az)*400.0f,
        std::max(10.0f, std::sin(sun_el)*400.0f),
        1.0f
    };

    float sun_diffuse[]  = {sr,sg,sb,1};
    float sun_ambient[]  = {ar,ag,ab,1};
    float sun_specular[] = {sr*0.6f,sg*0.5f,sb*0.4f,1};

    glLightfv(GL_LIGHT0, GL_POSITION, sun_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  sun_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  sun_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sun_specular);

    float hr,hg,hb;
    realtime::g_day.sky_horizon(hr,hg,hb);

    float fog_color[] = {hr,hg,hb,1};

    glFogfv(GL_FOG_COLOR, fog_color);
    glFogf(GL_FOG_DENSITY, realtime::g_day.fog_density());
}