#ifndef HUD_HPP
#define HUD_HPP

#include <GL/glut.h>
#include <cstdio>
#include <cmath>

#include "realtime/environment/daycycle.hpp"
#include "realtime/environment/wind.hpp"

namespace realtime {

inline void draw_hud(
    float fps,
    float hour,
    bool flying,
    int tree_count
){
    // Switch to 2D ortho
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0, 0, 0, 0.45f);

    glBegin(GL_QUADS);
        glVertex2f(8,   h-8);
        glVertex2f(340, h-8);
        glVertex2f(340, h-122);
        glVertex2f(8,   h-122);
    glEnd();

    glDisable(GL_BLEND);

    auto draw_text = [&](float x, float y, const char* s){
        glRasterPos2f(x, y);

        while(*s)
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *s++);
    };

    glColor3f(1, 1, 0.6f);

    char buf[128];

    // FPS
    snprintf(buf, sizeof(buf), "FPS: %.0f", fps);
    draw_text(16, h-22, buf);

    // Time
    int hh = (int)hour;
    int mm = (int)((hour - (int)hour) * 60);

    const char* period =
        (hh < 6 || hh >= 18)
        ? "Night"
        : (hh < 8 || hh >= 17)
            ? "Dawn/Dusk"
            : "Day";

    snprintf(
        buf,
        sizeof(buf),
        "Time: %02d:%02d (%s)",
        hh,
        mm,
        period
    );

    draw_text(16, h-38, buf);

    // Day speed
    snprintf(
        buf,
        sizeof(buf),
        "Day Speed: %s [[/]] change",
        g_day_menu.speed_label()
    );

    draw_text(16, h-54, buf);

    // Wind
    float ws =
        std::sqrt(
            g_wind.total_x() * g_wind.total_x() +
            g_wind.total_y() * g_wind.total_y()
        );

    snprintf(
        buf,
        sizeof(buf),
        "Wind: %.1f",
        ws
    );

    draw_text(16, h-70, buf);

    // Trees + leaves
    snprintf(
        buf,
        sizeof(buf),
        "Trees: %d",
        tree_count
    );

    draw_text(16, h-86, buf);

    // Camera mode
    snprintf(
        buf,
        sizeof(buf),
        "Mode: %s [F]=toggle [+/-]=speed",
        flying ? "FLY" : "WALK"
    );

    draw_text(16, h-102, buf);

    // Help
    glColor3f(0.6f, 1.0f, 0.7f);

    draw_text(
        16,
        h-118,
        "Left Click=Spawn [T]=Time Menu"
    );

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
}

} // namespace realtime

#endif