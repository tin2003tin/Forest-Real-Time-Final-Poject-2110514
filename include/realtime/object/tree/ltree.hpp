#ifndef LTREE_HPP
#define LTREE_HPP

#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "realtime/render/shading3d.hpp"
#include "realtime/render/material.hpp"

#include "realtime/camera/camera.hpp" 
#include "realtime/ui/hud.hpp"
#include "realtime/system/fps_counter.hpp"

#include "realtime/core/vec.hpp"
#include "realtime/core/random.hpp"

#include "realtime/environment/wind.hpp"
#include "realtime/environment/daycycle.hpp"
#include "realtime/environment/ground.hpp"

#include "realtime/object/tree/branch.hpp"
#include "realtime/object/tree/leaf.hpp"

namespace realtime {

// ═══════════════════════════════════════════════════════════════
//  Single Tree
// ═══════════════════════════════════════════════════════════════
struct Tree {
    Branch root;
    GLUquadric* quad;
    float tx, ty, scale;
    float leaf_r, leaf_g, leaf_b;
    float growth_time=0;

    struct TipPos{ float x,y,z; };
    std::vector<TipPos> tip_positions;

    Tree(float x, float y, float sc, float seed)
        : root(100.0f+randf(0,40.f),0,seed), tx(x), ty(y), scale(sc){
        quad=gluNewQuadric();
        gluQuadricNormals(quad,GLU_SMOOTH);
        srand((unsigned)(seed*1000+1));

        float season=randf(0,1);
        if(season<0.45f){
            leaf_r=randf(0.05f,0.18f); leaf_g=randf(0.45f,0.65f); leaf_b=randf(0.02f,0.09f);
        } else if(season<0.75f){
            leaf_r=randf(0.38f,0.60f); leaf_g=randf(0.50f,0.65f); leaf_b=randf(0.01f,0.07f);
        } else {
            leaf_r=randf(0.55f,0.88f); leaf_g=randf(0.22f,0.42f); leaf_b=randf(0.01f,0.07f);
        }
    }
    ~Tree(){
        root.free_display_list();
        if(quad) gluDeleteQuadric(quad);
    }
    Tree(const Tree&)=delete;
    Tree& operator=(const Tree&)=delete;
    Tree(Tree&& o) noexcept
        : root(std::move(o.root)), quad(o.quad),
          tx(o.tx),ty(o.ty),scale(o.scale),
          leaf_r(o.leaf_r),leaf_g(o.leaf_g),leaf_b(o.leaf_b),
          growth_time(o.growth_time),tip_positions(std::move(o.tip_positions))
    { o.quad=nullptr; }

    void update(float dt){
        growth_time+=dt;
        grow(root);
    }

    bool visible_from(float cx, float cy, float fwd_x, float fwd_y) const {
        float dx=tx-cx, dy=ty-cy;
        float dist2=dx*dx+dy*dy;
        if(dist2>1200*1200) return false; // hard cull distance
        // Behind camera frustum? (dot with forward)
        float dot=dx*fwd_x+dy*fwd_y;
        float dist=std::sqrt(dist2)+0.01f;
        // Allow wide angle: only cull if clearly behind AND far
        if(dot/dist < -0.5f && dist>200) return false;
        return true;
    }

    void draw(float global_time){
        glPushMatrix();
        glTranslatef(tx,ty,0);
        glScalef(scale,scale,scale);
        tip_positions.clear();
        draw_branch(root, global_time);
        glPopMatrix();
    }
    
    void draw_shadow(float global_time, float lx, float ly, float lz){
        if(lz <= 0.15f) return;  // skip when sun is too low — shadow too long

        // Normalize directional light vector
        float len = std::sqrt(lx*lx + ly*ly + lz*lz) + 1e-6f;
        lx /= len; ly /= len; lz /= len;

        // Clamp lz AFTER normalizing: prevents near-singular matrix at low sun angles.
        // Without this clamp, lz near 0 makes shadow stretch to infinity at dawn/dusk.
        // MIN_LZ = sin(~8.6°) ≈ 0.15 — shadows stay at most ~6.7× tree height.
        const float MIN_LZ = 0.5f;
        if(lz < MIN_LZ) lz = MIN_LZ;

        // Planar shadow matrix: projects onto z=0 plane from directional light L.
        // For plane z=0 (normal n=(0,0,1), d=0) and directional light L=(lx,ly,lz):
        //   dot = n·L = lz
        //   M_col_major[col][row]:
        //     col0: (lz, 0, 0, 0)
        //     col1: (0, lz, 0, 0)
        //     col2: (-lx, -ly, 0, 0)   <-- z column zeroed out → flattens to z=0
        //     col3: (0, 0, 0, lz)
        // OpenGL stores column-major: M[col*4+row]
        float M[16];
        M[0]  = lz;    M[4]  = 0.0f;  M[8]  = -lx;  M[12] = 0.0f;
        M[1]  = 0.0f;  M[5]  = lz;    M[9]  = -ly;  M[13] = 0.0f;
        M[2]  = 0.0f;  M[6]  = 0.0f;  M[10] = 0.0f; M[14] = 0.0f;
        M[3]  = 0.0f;  M[7]  = 0.0f;  M[11] = 0.0f; M[15] = lz;

        // IMPORTANT: Apply shadow projection FIRST (world space),
        // then apply tree's local translate+scale so shadow lands on z=0 ground.
        glPushMatrix();
        glMultMatrixf(M);           // project world geometry onto ground plane
        glTranslatef(tx, ty, 0);    // position tree in world
        glScalef(scale, scale, scale);

        draw_branch_shadow(root, global_time);

        glPopMatrix();
    }

    private:
    void draw_branch_shadow(Branch& b, float t){
        int segs = std::max(4, 12 - b.depth*2);
        float r0 = std::max(0.3f, 3.2f - b.depth*0.53f);
        float r1 = r0 * 0.72f;

        gluCylinder(quad, r0, r1, b.length, segs, 1);

        glPushMatrix();
        glTranslatef(0,0,b.length);

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);
        for(int i = 0; i <= segs; i++){
            float a = (2.0f * 3.1415926f * i) / segs;
            glVertex3f(std::cos(a) * r1, std::sin(a) * r1, 0);
        }
        glEnd();

        glPopMatrix();

        glTranslatef(0,0,b.length);

        // 🌿 Leaf cluster (REPLACED circle disk -> polygon leaf)
        if(b.children.empty() && b.depth > 2){
            float s = 18.0f;

            glBegin(GL_TRIANGLES);

            glVertex3f(0, 0, 0);
            glVertex3f(-s * 0.6f, -s * 0.2f, 0);
            glVertex3f(0, s, 0);

            glVertex3f(0, 0, 0);
            glVertex3f(0, s, 0);
            glVertex3f(s * 0.6f, -s * 0.2f, 0);

            glEnd();
        }

        for(auto& c:b.children){
            glPushMatrix();

            float wi = 1.0f/(1.0f+float(c.depth)*0.55f);
            float ga = g_wind.gust*6.0f*wi;

            float swx = (std::sin(t*1.2f+c.noise_seed)*9.0f
                        +std::sin(t*2.9f+c.noise_seed*1.4f)*2.8f
                        +g_wind.total_x()*1.6f+ga)*wi;

            float swy = (std::cos(t*0.95f+c.noise_seed)*7.0f
                        +std::cos(t*2.2f+c.noise_seed*0.9f)*2.2f
                        +g_wind.total_y()*1.6f+ga)*wi;

            glRotatef(c.angle_x+swx,1,0,0);
            glRotatef(c.angle_y+swy,0,1,0);
            glRotatef(c.angle_z,    0,0,1);

            draw_branch_shadow(c,t);

            glPopMatrix();
        }
    }

private:
    void grow(Branch& b){

    if(b.length < b.max_length){
        b.length += 20.f * 0.016f;
        b.invalidate();
    }

    if(b.length > b.max_length)
        b.length = b.max_length;

    if(
        b.children.empty() &&
        b.length > b.max_length*0.62f &&
        b.depth < 5
    ){

        int n = 2 + rand()%2;

        for(int i = 0; i < n; i++){

            float scale = 1.0f;

            // BIG lower branches
            if(b.depth == 0)
                scale = randf(0.82f, 0.95f);

            else if(b.depth == 1)
                scale = randf(0.70f, 0.85f);

            else if(b.depth == 2)
                scale = randf(0.58f, 0.72f);

            else
                scale = randf(0.42f, 0.60f);

            b.children.emplace_back(
                b.max_length * scale,
                b.depth + 1,
                b.noise_seed + i * 17.3f
            );
        }

        b.leaves_built = false;

        b.invalidate();
    }

    for(auto& c : b.children)
        grow(c);
}
    
    void build_leaves(Branch& b){

        if(b.leaves_built)
            return;

        b.leaf_quads.clear();

        // MUCH LOWER COUNT
        int count = 6 + rand()%6;

        b.leaf_quads.reserve(count);

        for(int i=0;i<count;i++){

            Branch::LeafQuad q;

            q.ox = randf(-12,12);
            q.oy = randf(-12,12);
            q.oz = randf(-1,10);

            float nx = randf(-1,1);
            float ny = randf(-1,1);
            float nz = randf(0.2f,1.f);

            float nl =
                std::sqrt(
                    nx*nx +
                    ny*ny +
                    nz*nz
                ) + 1e-6f;

            q.nx = nx/nl;
            q.ny = ny/nl;
            q.nz = nz/nl;

            float ax = -q.ny;
            float ay =  q.nx;
            float az = 0;

            float al =
                std::sqrt(
                    ax*ax + ay*ay
                );

            if(al < 0.01f){

                ax=1;
                ay=0;
                az=0;
                al=1;
            }

            q.ax = ax/al;
            q.ay = ay/al;
            q.az = az/al;

            q.bx =
                q.ny*q.az -
                q.nz*q.ay;

            q.by =
                q.nz*q.ax -
                q.nx*q.az;

            q.bz =
                q.nx*q.ay -
                q.ny*q.ax;

            q.size = randf(2.8f,5.2f);

            q.phase = randf(0,6.283f);

            q.cr = clampf(
                leaf_r + randf(-0.08f,0.08f),
                0,
                1
            );

            q.cg = clampf(
                leaf_g + randf(-0.08f,0.08f),
                0,
                1
            );

            q.cb = clampf(
                leaf_b + randf(-0.04f,0.04f),
                0,
                1
            );

            b.leaf_quads.push_back(q);
        }

        b.leaves_built = true;
    }
    
    void draw_branch(Branch& b, float t, float world_z = 0 ){
        // ─────────────────────────────────────────
        // DISPLAY LIST BUILD
        // ─────────────────────────────────────────
        if(b.dl_dirty || b.dl_id == 0){

            if(b.dl_id == 0)
                b.dl_id = glGenLists(1);

            Vec<3> normal{0,0,1};

            float sun_el = g_day.sun_elevation();
            float sun_az = g_day.sun_azimuth();

            float lx =
                std::cos(sun_el) *
                std::cos(sun_az);

            float ly =
                std::cos(sun_el) *
                std::sin(sun_az);

            float lz =
                std::max(
                    0.1f,
                    std::sin(sun_el)
                );

            Vec<3> light_dir =
                normalize<3>(
                    Vec<3>{lx,ly,lz}
                );

            Vec<3> view_dir =
                normalize<3>(
                    Vec<3>{0,0,1}
                );

            float d =
                float(b.depth) / 5.0f;

            Material wood;

            wood.ka = Vec<3>{
                0.09f+d*0.03f,
                0.06f+d*0.02f,
                0.02f
            };

            wood.kd = Vec<3>{
                0.38f-d*0.08f,
                0.23f-d*0.04f,
                0.09f
            };

            wood.ks = Vec<3>{
                0.03f,
                0.02f,
                0.01f
            };

            wood.shininess = 4.f;

            Vec<3> col =
                shade3d(
                    ShadingType::Lambert,
                    normal,
                    light_dir,
                    view_dir,
                    wood
                );

            int segs = 4;

            float r0 =
                std::max(
                    0.3f,
                    3.2f - b.depth*0.53f
                );

            float r1 = r0 * 0.72f;

            glNewList(
                b.dl_id,
                GL_COMPILE
            );

                glColor3f(
                    clampf(col[0],0,1),
                    clampf(col[1],0,1),
                    clampf(col[2],0,1)
                );

                gluCylinder(
                    quad,
                    r0,
                    r1,
                    b.length,
                    segs,
                    1
                );

                glTranslatef(
                    0,
                    0,
                    b.length
                );

            glEndList();

            b.dl_dirty = false;
        }

        // ─────────────────────────────────────────
        // DRAW BRANCH
        // ─────────────────────────────────────────
        glCallList(b.dl_id);

        float nwz =
            world_z + b.length;

        // ─────────────────────────────────────────
        // LEAVES
        // ─────────────────────────────────────────
        if(
            b.children.empty() &&
            b.depth > 2
        ){

            build_leaves(b);

            float inv_d =
                1.0f /
                (1.0f + float(b.depth));

            float bsx =
                std::sin(
                    t + b.noise_seed
                ) * 3.0f * inv_d;

            float bsy =
                std::cos(
                    t + b.noise_seed
                ) * 2.5f * inv_d;

            glBegin(GL_TRIANGLES);

            for(const auto& q : b.leaf_quads){

                float disp =
                    std::sin(
                        t + q.phase
                    ) * 1.2f;

                float lx2 =
                    q.ox +
                    bsx +
                    disp*q.nx;

                float ly2 =
                    q.oy +
                    bsy +
                    disp*q.ny;

                float lz2 =
                    q.oz +
                    disp*q.nz*0.25f;

                float brightness =
                    0.7f +
                    q.nz * 0.3f;

                glColor3f(
                    q.cr * brightness,
                    q.cg * brightness,
                    q.cb * brightness
                );

                float s = q.size;

                glVertex3f(
                    lx2+(-q.ax-q.bx*0.5f)*s,
                    ly2+(-q.ay-q.by*0.5f)*s,
                    lz2+(-q.az-q.bz*0.5f)*s
                );

                glVertex3f(
                    lx2+( q.ax-q.bx*0.5f)*s,
                    ly2+( q.ay-q.by*0.5f)*s,
                    lz2+( q.az-q.bz*0.5f)*s
                );

                glVertex3f(
                    lx2+ q.bx*1.3f*s,
                    ly2+ q.by*1.3f*s,
                    lz2+ q.bz*1.3f*s
                );
            }

            glEnd();

            tip_positions.push_back({
                tx,
                ty,
                nwz * scale
            });
        }

        // ─────────────────────────────────────────
        // CHILD BRANCHES
        // ─────────────────────────────────────────
        for(auto& c : b.children){

            glPushMatrix();

            float wi =
                1.0f /
                (1.0f + float(c.depth)*0.55f);

            // turbulence
            float turbulence_x =
            (
                std::sin(
                    t * 1.3f +
                    c.noise_seed
                ) * 6.0f

                +

                std::sin(
                    t * 2.7f +
                    c.noise_seed * 1.4f
                ) * 2.5f
            );

            float turbulence_y =
            (
                std::cos(
                    t * 1.1f +
                    c.noise_seed
                ) * 5.0f

                +

                std::cos(
                    t * 2.2f +
                    c.noise_seed * 0.8f
                ) * 2.0f
            );

            // target sway
            float target_x =
            (
                turbulence_x +
                g_wind.total_x() * 1.8f +
                g_wind.gust * 8.0f
            ) * wi;

            float target_y =
            (
                turbulence_y +
                g_wind.total_y() * 1.8f +
                g_wind.gust * 8.0f
            ) * wi;

            // ─────────────────────────────────────
            // SPRING DAMPER
            // ─────────────────────────────────────
            float spring  = 14.0f;
            float damping = 0.82f;

            c.wind_vel_x +=
                (target_x - c.wind_x)
                * spring
                * 0.016f;

            c.wind_vel_y +=
                (target_y - c.wind_y)
                * spring
                * 0.016f;

            c.wind_vel_x *= damping;
            c.wind_vel_y *= damping;

            c.wind_x +=
                c.wind_vel_x * 0.016f;

            c.wind_y +=
                c.wind_vel_y * 0.016f;

            float swx = c.wind_x;
            float swy = c.wind_y;

            glRotatef(
                c.angle_x + swx,
                1,0,0
            );

            glRotatef(
                c.angle_y + swy,
                0,1,0
            );

            glRotatef(
                c.angle_z,
                0,0,1
            );

            draw_branch(
                c,
                t,
                nwz
            );

            glPopMatrix();
        }
    }
};

// ═══════════════════════════════════════════════════════════════
//  Forest (LTree class — public name kept for main.cpp compat)
// ═══════════════════════════════════════════════════════════════
class LTree {
public:
    std::vector<Tree>        trees;
    std::vector<FallingLeaf> falling_leaves;
    Ground  ground;
    float   global_time=0;
    float   spawn_timer=0;

    LTree(){
        struct P{ float x,y,sc,seed; };
        std::vector<P> pl={
            {   0,   0, 1.10f, 1.0f},
            { 180,  80, 0.85f, 2.0f},
            {-160, 120, 0.95f, 3.0f},
            {  90,-190, 0.90f, 4.0f},
            {-200,-100, 0.80f, 5.0f},
            { 260, -50, 0.75f, 6.0f},
            {-280,  30, 0.82f, 7.0f},
            {  50, 280, 0.88f, 8.0f},
            {-110,-280, 0.78f, 9.0f},
            { 340, 200, 0.72f,10.0f},
            {-320,-200, 0.70f,11.0f},
            { 150,-350, 0.76f,12.0f},
        };
        trees.reserve(pl.size());
        for(auto& p:pl) trees.emplace_back(p.x,p.y,p.sc,p.seed);

        // Register colliders (trunk radius = 3.2 * scale, padded a bit)
        g_tree_colliders.clear();
        for(auto& tr:trees)
            g_tree_colliders.push_back({tr.tx, tr.ty, 3.2f*tr.scale+6.0f});
    }

    void update(float dt){
        dt=std::min(dt,0.05f);
        global_time+=dt;
        g_day.update(dt);
        g_wind.update(dt);

        g_fps.update(dt);

        for(auto& tr:trees) tr.update(dt);
    }

    void draw(){
        // ── Ground ───────────────────────────────────────────────
        ground.draw(global_time);

        // ── Planar tree shadows on ground ────────────────────────
        float sun_el = g_day.sun_elevation();
        float sun_az = g_day.sun_azimuth();

        float sin_el = std::sin(sun_el);
        float lx = std::cos(sun_el) * std::cos(sun_az);
        float ly = std::cos(sun_el) * std::sin(sun_az);
        float lz = sin_el;   // raw — draw_shadow clamps internally

        // Fade alpha: zero below 0.15 rad elevation, full above 0.35 rad.
        // This hides the stretched shadow at low sun angles gracefully.
        float shadow_alpha = clampf((sin_el - 0.15f) / 0.20f, 0.0f, 1.0f) * 0.65f;

        if(shadow_alpha > 0.01f){

            float fwd_xs = std::cos(cam.yaw * (float)M_PI / 180.f);
            float fwd_ys = std::sin(cam.yaw * (float)M_PI / 180.f);

            glDisable(GL_LIGHTING);

            // 🔥 darker + more solid blending
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // 🔥 prevents weak washed shadows
            glDepthMask(GL_FALSE);

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-3.0f, -3.0f);

            // 🌑 stronger base shadow color (much darker than before)
            glColor4f(0.02f, 0.04f, 0.02f, shadow_alpha);

            for(auto& tr : trees){
                if(tr.visible_from(cam.x, cam.y, fwd_xs, fwd_ys))
                    tr.draw_shadow(global_time, lx, ly, lz);
            }

            glDisable(GL_POLYGON_OFFSET_FILL);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);

            glEnable(GL_LIGHTING);
        }

        // ── Trees with frustum culling ────────────────────────────
        float fwd_x=std::cos(cam.yaw*(float)M_PI/180.f);
        float fwd_y=std::sin(cam.yaw*(float)M_PI/180.f);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f,1.0f);
        for(auto& tr:trees)
            if(tr.visible_from(cam.x,cam.y,fwd_x,fwd_y))
                tr.draw(global_time);
        glDisable(GL_POLYGON_OFFSET_FILL);

        // ── Falling + settled leaves ──────────────────────────────
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for(const auto& lf:falling_leaves) lf.draw();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // ── HUD ───────────────────────────────────────────────────
        draw_hud(
            g_fps.fps,
            g_day.time,
            cam.flying,
            (int)trees.size()
        );

        // ── Day/Time menu overlay ─────────────────────────────────
        g_day_menu.draw();
    }


    void spawn_tree(float wx, float wy){
        float seed = float(trees.size()+1) * 13.7f + wx*0.01f + wy*0.01f;
        float sc = 0.72f + float(rand()%30)*0.01f;
        trees.emplace_back(wx, wy, sc, seed);
        g_tree_colliders.push_back({wx, wy, 3.2f*sc+6.0f});
    }
};

} // namespace realtime
#endif