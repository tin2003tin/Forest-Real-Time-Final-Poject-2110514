#ifndef FALLING_LEAF_HPP
#define FALLING_LEAF_HPP

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "realtime/core/random.hpp"
#include "realtime/environment/wind.hpp"

namespace realtime {

struct FallingLeaf {

    float x,y,z;
    float vx,vy,vz;

    float rot;
    float spin;

    float life = 0.0f;
    float max_life;

    float size;

    float r,g,b;

    bool dead = false;
    bool settled = false;

    float settle_timer = 0.0f;

    FallingLeaf(
        float px,
        float py,
        float pz,
        float cr,
        float cg,
        float cb
    ){

        x = px;
        y = py;
        z = pz;

        vx = randf(-2.0f,2.0f);
        vy = randf(-2.0f,2.0f);
        vz = randf(-1.5f,-4.0f);

        rot  = randf(0,360);
        spin = randf(-120,120);

        max_life = randf(12.0f,25.0f);

        size = randf(1.8f,3.5f);

        r = clampf(cr + randf(-0.08f,0.08f),0,1);
        g = clampf(cg + randf(-0.08f,0.08f),0,1);
        b = clampf(cb + randf(-0.08f,0.08f),0,1);
    }

    // ─────────────────────────────────────────
    // UPDATE
    // ─────────────────────────────────────────
    void update(float dt){

        if(dead)
            return;

        life += dt;

        if(life >= max_life){

            dead = true;
            return;
        }

        // Skip expensive movement
        // after leaf settles
        if(settled){

            settle_timer += dt;

            // fade out while resting
            if(settle_timer > 6.0f)
                dead = true;

            return;
        }

        // Wind
        float wx = g_wind.total_x();
        float wy = g_wind.total_y();

        vx += wx * dt * 1.2f;
        vy += wy * dt * 1.2f;

        // Gravity
        vz -= 7.0f * dt;

        // Movement
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;

        // Cheap drag
        vx *= 0.985f;
        vy *= 0.985f;

        // Simple rotation
        rot += spin * dt;

        // Ground collision
        if(z <= 0.0f){

            z = randf(0.0f,0.12f);

            settled = true;

            vx = 0;
            vy = 0;
            vz = 0;
        }
    }

    // ─────────────────────────────────────────
    // DRAW
    // ─────────────────────────────────────────
    void draw() const {

        if(dead)
            return;

        float age =
            life / max_life;

        // shrink slowly
        float shrink =
            1.0f - age * 0.45f;

        float s =
            size * shrink;

        // fade near death
        float alpha = 1.0f;

        if(age > 0.75f){

            alpha =
                1.0f -
                (age - 0.75f) * 4.0f;
        }

        // NO push/pop matrix
        // MUCH faster

        glColor4f(r,g,b,alpha);

        float c = std::cos(rot * 0.0174533f);
        float si = std::sin(rot * 0.0174533f);

        float x1 = -s*c;
        float y1 = -s*si;

        float x2 =  s*c;
        float y2 =  s*si;

        float x3 = -si*s*0.5f;
        float y3 =  c*s*0.5f;

        glBegin(GL_TRIANGLES);

        glVertex3f(
            x + x1,
            y + y1,
            z
        );

        glVertex3f(
            x + x2,
            y + y2,
            z
        );

        glVertex3f(
            x + x3,
            y + y3,
            z + s
        );

        glEnd();
    }
};

} // namespace realtime

#endif