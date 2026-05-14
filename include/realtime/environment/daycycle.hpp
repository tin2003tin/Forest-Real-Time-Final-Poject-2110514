#ifndef DAYCYCLE_HPP
#define DAYCYCLE_HPP

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include "realtime/core/random.hpp"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
namespace realtime {

struct DayCycle {
    float time   = 8.5f;
    float speed  = 0.008f;

    void update(float dt) {
        time += speed * dt;
        if(time >= 24.0f) time -= 24.0f;
    }

    float t01() const {
        return time / 24.0f;
    }

    float sun_elevation() const {
        float h = time;
        if(h < 6 || h > 18) return -0.3f;
        return std::sin((h - 6) / 12.0f * (float)M_PI) * 1.3f;
    }

    float sun_azimuth() const {
        return (time / 24.0f) * 2.0f * (float)M_PI;
    }

    float moon_elevation() const {

    // opposite side of sun
    float h = time + 12.0f;

        if(h >= 24.0f)
            h -= 24.0f;

        if(h < 6 || h > 18)
            return -0.3f;

        return std::sin(
            (h - 6) / 12.0f * (float)M_PI
        ) * 1.1f;
    }

    float moon_azimuth() const {

        // opposite side of sun
        float h = time + 12.0f;

        if(h >= 24.0f)
            h -= 24.0f;

        return (h / 24.0f)
            * 2.0f
            * (float)M_PI;
    }

    void sky_horizon(float& r, float& g, float& b) const {
        float h = time;

        if(h >= 6 && h <= 8) {
            float t = (h - 6) / 2;
            r = 0.95f + t * (-0.23f);
            g = 0.6f  + t * 0.22f;
            b = 0.4f  + t * 0.55f;
        }
        else if(h > 8 && h <= 16) {
            r = 0.72f; g = 0.84f; b = 0.95f;
        }
        else if(h > 16 && h <= 18) {
            float t = (h - 16) / 2;
            r = 0.72f + t * 0.23f;
            g = 0.84f - t * 0.34f;
            b = 0.95f - t * 0.55f;
        }
        else {
            float t = h > 18 ? (h - 18) / 6 : (6 - h) / 6;
            r = 0.04f + t * 0.05f;
            g = 0.05f + t * 0.05f;
            b = 0.15f + t * 0.08f;
        }
    }

    void sky_zenith(float& r, float& g, float& b) const {
        float h = time;

        if(h >= 6 && h <= 18) {
            float d = std::sin((h - 6) / 12.0f * (float)M_PI);
            r = 0.18f + d * 0.10f;
            g = 0.35f + d * 0.15f;
            b = 0.65f + d * 0.17f;
        } else {
            r = 0.01f;
            g = 0.01f;
            b = 0.06f;
        }
    }

    void sun_color(float& r, float& g, float& b) const {
        float h = time;
        float elev = sun_elevation();

        if(elev < 0) {
            r = 0.05f;
            g = 0.05f;
            b = 0.10f;
            return;
        }

        float low = clampf(1.0f - elev * 0.5f, 0, 1);

        r = 0.90f + low * 0.08f;
        g = 0.82f - low * 0.30f;
        b = 0.68f - low * 0.40f;

        float night = (h < 6 || h > 18) ? 1.0f : 0.0f;

        r = r * (1 - night) + 0.05f * night;
        g = g * (1 - night) + 0.05f * night;
        b = b * (1 - night) + 0.12f * night;
    }

    void moon_color(float& r, float& g, float& b) const {

        float elev = moon_elevation();

        if(elev <= 0.0f){

            r = g = b = 0.0f;
            return;
        }

        float m =
            clampf(elev * 0.8f, 0.15f, 1.0f);

        r = 0.55f * m;
        g = 0.65f * m;
        b = 0.95f * m;
    }

    void ambient_color(float& r, float& g, float& b) const {
        float h = time;

        if(h >= 6 && h <= 18) {
            float d = std::sin((h - 6) / 12.0f * (float)M_PI);
            r = 0.10f + d * 0.08f;
            g = 0.10f + d * 0.08f;
            b = 0.14f + d * 0.08f;
        } else {
            r = 0.02f;
            g = 0.02f;
            b = 0.04f;
        }
    }

    float fog_density() const {
        float h = time;

        if(h >= 7 && h <= 17) return 0.0016f;
        if(h >= 6 && h < 7)   return 0.004f - (h - 6) * 0.0024f;
        if(h > 17 && h <= 18) return 0.0016f + (h - 17) * 0.004f;

        return 0.006f;
    }
};

inline DayCycle g_day;

// ── Day Menu ───────────────────────────────────────────────────
struct DayMenu {
    bool open = false;

    // ── Time-of-day presets ──────────────────────────────────
    struct Preset { const char* label; float hour; };
    static constexpr Preset presets[] = {
        { "Dawn      (06:00)",  6.0f  },
        { "Morning   (08:00)",  8.0f  },
        { "Noon      (12:00)", 12.0f  },
        { "Afternoon (15:00)", 15.0f  },
        { "Sunset    (18:00)", 18.0f  },
        { "Dusk      (19:30)", 19.5f  },
        { "Night     (22:00)", 22.0f  },
        { "Midnight  (00:00)",  0.0f  },
    };
    static constexpr int NUM_PRESETS = 8;

    // ── Time-speed presets (in-game hours per real second) ───
    struct SpeedPreset { const char* label; float speed; };
    static constexpr SpeedPreset speeds[] = {
        { "Paused",    0.0f    },
        { "x0.25",     0.002f  },
        { "x1  (Normal)", 0.008f },
        { "x4",        0.033f  },
        { "x10",       0.083f  },
        { "x30",       0.250f  },
        { "x100",      0.833f  },
    };
    static constexpr int NUM_SPEEDS = 7;
    int speed_index = 2; // default = "x1 Normal"

    void speed_up()   { if (speed_index < NUM_SPEEDS-1) { speed_index++; g_day.speed = speeds[speed_index].speed; } }
    void speed_down() { if (speed_index > 0)            { speed_index--; g_day.speed = speeds[speed_index].speed; } }
    const char* speed_label() const { return speeds[speed_index].label; }

    void toggle() { open = !open; }

    // Returns hour >= 0 if user picked a time preset (key '1'-'8').
    // Handles speed keys [ and ] regardless of menu open state.
    // Returns -1 if no time preset was chosen.
    float handle_key(unsigned char k) {
        // Speed keys always active
        if (k == ']') { speed_up();   return -1.0f; }
        if (k == '[') { speed_down(); return -1.0f; }

        if (!open) return -1.0f;
        if (k >= '1' && k <= '8') {
            open = false;
            return presets[k - '1'].hour;
        }
        if (k == 27 || k == 't' || k == 'T') { open = false; }
        return -1.0f;
    }

    void draw() const {
        if (!open) return;
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FOG);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrtho(0, w, 0, h, -1, 1);
        glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Panel dimensions: presets + speed section + padding
        float pw = 280.0f;
        float ph = 32.0f * NUM_PRESETS   // preset rows
                 + 32.0f                  // speed row
                 + 80.0f;                 // title + dividers + footer
        float px = (w - pw) * 0.5f;
        float py = (h - ph) * 0.5f;

        // Background
        glColor4f(0.05f, 0.08f, 0.05f, 0.88f);
        glBegin(GL_QUADS);
        glVertex2f(px, py);      glVertex2f(px+pw, py);
        glVertex2f(px+pw, py+ph); glVertex2f(px, py+ph);
        glEnd();

        // Border
        glColor4f(0.45f, 0.80f, 0.45f, 0.90f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(px+1, py+1);       glVertex2f(px+pw-1, py+1);
        glVertex2f(px+pw-1, py+ph-1); glVertex2f(px+1, py+ph-1);
        glEnd();

        glDisable(GL_BLEND);

        auto draw_text = [&](float x, float y, const char* s) {
            glRasterPos2f(x, y);
            while (*s) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *s++);
        };

        // ── Title ──────────────────────────────────────────────
        glColor3f(0.9f, 1.0f, 0.6f);
        draw_text(px + 14, py + ph - 26, "[ T ] Set Time of Day");

        float divY1 = py + ph - 34;
        glColor3f(0.35f, 0.55f, 0.35f);
        glBegin(GL_LINES);
        glVertex2f(px+8, divY1); glVertex2f(px+pw-8, divY1);
        glEnd();

        // ── Time preset rows ───────────────────────────────────
        for (int i = 0; i < NUM_PRESETS; i++) {
            float row_y = py + ph - 50 - i * 32;
            if (i % 2 == 0) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(1.0f, 1.0f, 1.0f, 0.04f);
                glBegin(GL_QUADS);
                glVertex2f(px+4, row_y-4);    glVertex2f(px+pw-4, row_y-4);
                glVertex2f(px+pw-4, row_y+20); glVertex2f(px+4, row_y+20);
                glEnd();
                glDisable(GL_BLEND);
            }
            char buf[64];
            snprintf(buf, sizeof(buf), "  [%d]  %s", i+1, presets[i].label);
            glColor3f(0.85f, 0.95f, 0.75f);
            draw_text(px + 8, row_y + 3, buf);
        }

        // ── Speed section ──────────────────────────────────────
        float divY2 = py + 58;
        glColor3f(0.35f, 0.55f, 0.35f);
        glBegin(GL_LINES);
        glVertex2f(px+8, divY2); glVertex2f(px+pw-8, divY2);
        glEnd();

        glColor3f(0.75f, 0.90f, 0.55f);
        draw_text(px + 14, py + 44, "Time Speed:  [ = slower   ] = faster");

        // Speed bar — highlight active speed slot
        float bar_x = px + 14;
        float bar_y = py + 18;
        float slot_w = (pw - 28.0f) / NUM_SPEEDS;
        for (int i = 0; i < NUM_SPEEDS; i++) {
            float sx = bar_x + i * slot_w;
            bool active = (i == speed_index);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if (active)
                glColor4f(0.30f, 0.85f, 0.30f, 0.85f);  // bright green = selected
            else
                glColor4f(0.20f, 0.35f, 0.20f, 0.65f);  // dim = unselected
            glBegin(GL_QUADS);
            glVertex2f(sx+1,          bar_y);
            glVertex2f(sx+slot_w-2,   bar_y);
            glVertex2f(sx+slot_w-2,   bar_y+14);
            glVertex2f(sx+1,          bar_y+14);
            glEnd();
            glDisable(GL_BLEND);

            // Tick border on active
            if (active) {
                glColor3f(0.6f, 1.0f, 0.6f);
                glLineWidth(1.5f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(sx+1,        bar_y);
                glVertex2f(sx+slot_w-2, bar_y);
                glVertex2f(sx+slot_w-2, bar_y+14);
                glVertex2f(sx+1,        bar_y+14);
                glEnd();
            }
        }

        // Current speed label centred in bar area
        char spd_buf[32];
        snprintf(spd_buf, sizeof(spd_buf), ">> %s <<", speeds[speed_index].label);
        glColor3f(1.0f, 1.0f, 0.5f);
        // measure roughly: 9px per char
        float txt_w = (float)strlen(spd_buf) * 9.0f;
        draw_text(px + (pw - txt_w) * 0.5f, bar_y + 2, spd_buf);

        // ── Footer ─────────────────────────────────────────────
        glColor3f(0.5f, 0.65f, 0.5f);
        draw_text(px + 14, py + 6, "  ESC / T  close");

        glPopMatrix();
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_FOG);
    }
};

inline DayMenu g_day_menu;

} // namespace realtime

#endif