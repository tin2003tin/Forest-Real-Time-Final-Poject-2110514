#ifndef GROUND_HPP
#define GROUND_HPP

#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>

#include "realtime/core/random.hpp"
#include "realtime/environment/wind.hpp"

namespace realtime {

// Simple 2D hash for ground colour variation
inline float hash2(float x, float y){
    float s = std::sin(x*127.1f + y*311.7f)*43758.5453f;
    return s - std::floor(s);
}

// ═══════════════════════════════════════════════════════════════
//  Ground
// ═══════════════════════════════════════════════════════════════
struct Ground {

    struct GrassBlade{
        float x,y,h,lean_x,lean_y,r,g,b;
    };

    struct Rock {
        float x,y,z,sx,sy,sz,rot;
    };

    struct Stump {
        float x,y,r,h;
    };

    std::vector<GrassBlade> grass;
    std::vector<Rock> rocks;
    std::vector<Stump> stumps;

    GLUquadric* quad;

    Ground(){
        quad = gluNewQuadric();

        srand(7777);

        // Grass
        for(int i=0;i<2400;i++){
            GrassBlade gb;

            gb.x = randf(-450,450);
            gb.y = randf(-450,450);

            gb.h = randf(3,10);

            gb.lean_x = randf(-0.45f,0.45f);
            gb.lean_y = randf(-0.45f,0.45f);

            float v = randf(0,1);

            gb.r = 0.07f + v*0.12f;
            gb.g = 0.26f + v*0.20f;
            gb.b = 0.03f + v*0.05f;

            grass.push_back(gb);
        }

        // Rocks
        for(int i=0;i<28;i++){
            Rock rk;

            rk.x = randf(-420,420);
            rk.y = randf(-420,420);
            rk.z = 0;

            rk.sx = randf(4,14);
            rk.sy = randf(3,11);
            rk.sz = randf(2,8);

            rk.rot = randf(0,360);

            rocks.push_back(rk);
        }

        // Stumps
        for(int i=0;i<6;i++){
            Stump st;

            st.x = randf(-180,180);
            st.y = randf(-180,180);

            st.r = randf(3,7);
            st.h = randf(4,14);

            stumps.push_back(st);
        }
    }

    ~Ground(){
        gluDeleteQuadric(quad);
    }

    void draw(float t){

        glDisable(GL_TEXTURE_2D);

        // ── Ground tiles ─────────────────────────────
        int tiles = 12;
        float tsz = 100.f;

        float half = tiles*tsz*0.5f;

        glBegin(GL_QUADS);

        for(int tx=0; tx<tiles; tx++){
            for(int ty=0; ty<tiles; ty++){

                float gx = -half + tx*tsz;
                float gy = -half + ty*tsz;

                float h2 = hash2(gx,gy);

                float gr = 0.14f + h2*0.08f;
                float gg = 0.28f + h2*0.12f;
                float gb = 0.06f + h2*0.04f;

                glColor3f(gr,gg,gb);

                glNormal3f(0,0,1);

                glVertex3f(gx,     gy,     0);
                glVertex3f(gx+tsz, gy,     0);
                glVertex3f(gx+tsz, gy+tsz, 0);
                glVertex3f(gx,     gy+tsz, 0);
            }
        }

        glEnd();

        // ── Dirt paths ──────────────────────────────
        struct {
            float x,y,r;
        } paths[] = {
            {0,0,25},
            {180,80,16},
            {-160,120,16},
            {90,-190,16},
            {-200,-100,16}
        };

        float dr=0.32f;
        float dg=0.22f;
        float db=0.10f;

        for(auto& p : paths){

            glColor3f(dr,dg,db);

            glBegin(GL_TRIANGLE_FAN);

            glNormal3f(0,0,1);

            glVertex3f(p.x,p.y,0.05f);

            for(int i=0;i<=32;i++){

                float a=i*6.2832f/32;

                glVertex3f(
                    p.x+std::cos(a)*p.r,
                    p.y+std::sin(a)*p.r,
                    0.05f
                );
            }

            glEnd();
        }

        // ── Rocks ───────────────────────────────────
        for(const auto& rk:rocks){

            glPushMatrix();

            glTranslatef(rk.x,rk.y,rk.z);

            glRotatef(rk.rot,0,0,1);

            glScalef(rk.sx,rk.sy,rk.sz);

            float rh = hash2(rk.x,rk.y);

            glColor3f(
                0.35f+rh*0.15f,
                0.32f+rh*0.12f,
                0.28f+rh*0.08f
            );

            glutSolidSphere(1.0,7,5);

            glPopMatrix();
        }

        // ── Stumps ──────────────────────────────────
        glColor3f(0.28f,0.18f,0.09f);

        for(const auto& st:stumps){

            glPushMatrix();

            glTranslatef(st.x,st.y,0);

            gluCylinder(
                quad,
                st.r,
                st.r*0.85f,
                st.h,
                8,
                1
            );

            glTranslatef(0,0,st.h);

            gluDisk(
                quad,
                0,
                st.r*0.85f,
                8,
                1
            );

            glPopMatrix();
        }

        // ── Grass ───────────────────────────────────
        float wx = g_wind.total_x()*0.025f;
        float wy = g_wind.total_y()*0.025f;

        glBegin(GL_TRIANGLES);

        for(const auto& gb:grass){

            float sway =
                std::sin(
                    t*2.1f +
                    gb.x*0.11f +
                    gb.y*0.08f
                ) * 0.18f;

            float gust_lean = g_wind.gust*0.4f;

            float lean_x =
                gb.lean_x +
                wx +
                sway +
                g_wind.gx*gust_lean;

            float lean_y =
                gb.lean_y +
                wy +
                sway*0.8f +
                g_wind.gy*gust_lean;

            float tipx = gb.x + lean_x*gb.h;
            float tipy = gb.y + lean_y*gb.h;
            float tipz = gb.h;

            float w = 0.6f;

            // Blade 1
            glColor3f(
                gb.r*0.45f,
                gb.g*0.45f,
                gb.b*0.45f
            );

            glNormal3f(1,0,0);

            glVertex3f(gb.x-w,gb.y,0.05f);
            glVertex3f(gb.x+w,gb.y,0.05f);

            glColor3f(
                gb.r*1.6f,
                gb.g*1.5f,
                gb.b*1.2f
            );

            glVertex3f(tipx,tipy,tipz);

            // Blade 2
            glColor3f(
                gb.r*0.45f,
                gb.g*0.45f,
                gb.b*0.45f
            );

            glNormal3f(0,1,0);

            glVertex3f(gb.x,gb.y-w,0.05f);
            glVertex3f(gb.x,gb.y+w,0.05f);

            glColor3f(
                gb.r*1.6f,
                gb.g*1.5f,
                gb.b*1.2f
            );

            glVertex3f(tipx,tipy,tipz);
        }

        glEnd();
    }
};

} // namespace realtime

#endif