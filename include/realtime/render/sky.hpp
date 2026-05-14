#pragma once

#include <GL/glut.h>

#include "realtime/environment/daycycle.hpp"

inline void draw_sky(){

    float hr,hg,hb,zr,zg,zb;

    realtime::g_day.sky_horizon(hr,hg,hb);
    realtime::g_day.sky_zenith(zr,zg,zb);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(-1,1,-1,1,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);

    glColor3f(hr,hg,hb);
    glVertex2f(-1,-1);

    glColor3f(hr,hg,hb);
    glVertex2f(1,-1);

    glColor3f(zr,zg,zb);
    glVertex2f(1,1);

    glColor3f(zr,zg,zb);
    glVertex2f(-1,1);

    glEnd();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
}