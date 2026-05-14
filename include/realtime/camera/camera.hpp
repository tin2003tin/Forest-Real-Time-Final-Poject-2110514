#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <GL/glut.h>
#include <cmath>
#include <vector>
#include "realtime/environment/daycycle.hpp"
#include "realtime/environment/wind.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define GLUT_KEY_SHIFT_L 0x0070
#define GLUT_KEY_SHIFT_R 0x0071

// ─────────────────────────────────────────────────────────────
//  Camera
// ─────────────────────────────────────────────────────────────
struct Camera {
    float x = -340, y = -220, z = 80;
    float pitch = 5.0f, yaw = 50.0f;
    float speed = 110.0f;
    float sensitivity = 0.18f;
    float ground_height = 22.0f;  // eye height above ground
    bool  flying = false;         // F key toggles free-fly vs walk
};
inline Camera cam;

// ─────────────────────────────────────────────────────────────
//  Tree positions for collision (filled by LTree constructor)
// ─────────────────────────────────────────────────────────────
struct TreeCollider { float x, y, radius; };
inline std::vector<TreeCollider> g_tree_colliders;

// ─────────────────────────────────────────────────────────────
//  Input state
// ─────────────────────────────────────────────────────────────
inline bool key_w=false, key_s=false, key_a=false, key_d=false;
inline bool key_up=false, key_down=false, key_shift=false;

inline void special_down(int key, int,int){
    if(key==GLUT_KEY_SHIFT_L||key==GLUT_KEY_SHIFT_R) key_shift=true;
}
inline void special_up(int key, int,int){
    if(key==GLUT_KEY_SHIFT_L||key==GLUT_KEY_SHIFT_R) key_shift=false;
}

inline void mouse_motion(int x, int y){
    int cw = glutGet(GLUT_SCREEN_WIDTH)/2;
    int ch = glutGet(GLUT_SCREEN_HEIGHT)/2;
    int dx = x-cw, dy = ch-y;
    cam.yaw   -= dx*cam.sensitivity;
    cam.pitch += dy*cam.sensitivity;
    if(cam.pitch> 89.f) cam.pitch= 89.f;
    if(cam.pitch<-89.f) cam.pitch=-89.f;
    glutWarpPointer(cw, ch);
}

inline void update_camera(float dt){
    float ry = cam.yaw  *(float)M_PI/180.f;
    float rp = cam.pitch*(float)M_PI/180.f;

    // Horizontal-only forward for walking (ignore pitch in XY)
    float fw_x = std::cos(ry);
    float fw_y = std::sin(ry);
    float fw_z = std::sin(rp);  // used in fly mode

    float right_x = -std::sin(ry);
    float right_y =  std::cos(ry);

    float spd = cam.speed * (key_shift ? 2.5f : 1.0f) * dt;

    float nx = cam.x, ny = cam.y, nz = cam.z;

    if(cam.flying){
        if(key_w){ nx+=fw_x*spd; ny+=fw_y*spd; nz+=fw_z*spd; }
        if(key_s){ nx-=fw_x*spd; ny-=fw_y*spd; nz-=fw_z*spd; }
    } else {
        // Walk: move only in XY, clamp Z to ground
        if(key_w){ nx+=fw_x*spd; ny+=fw_y*spd; }
        if(key_s){ nx-=fw_x*spd; ny-=fw_y*spd; }
    }
    if(key_a){ nx+=right_x*spd; ny+=right_y*spd; }
    if(key_d){ nx-=right_x*spd; ny-=right_y*spd; }
    if(key_up)   nz += spd;
    if(key_down) nz -= spd;

    // ── Tree trunk collision ──────────────────────────────────
    for(const auto& tc : g_tree_colliders){
        float dx = nx - tc.x;
        float dy2 = ny - tc.y;
        float dist = std::sqrt(dx*dx + dy2*dy2);
        if(dist < tc.radius){
            // Push camera out radially
            float push = tc.radius - dist + 0.5f;
            if(dist > 0.01f){ nx += (dx/dist)*push; ny += (dy2/dist)*push; }
            else { nx += tc.radius; } // degenerate
        }
    }

    // ── Ground clamp (walk mode) ──────────────────────────────
    if(!cam.flying){
        nz = cam.ground_height;
    } else {
        if(nz < cam.ground_height) nz = cam.ground_height;
    }

    cam.x = nx; cam.y = ny; cam.z = nz;
}

inline void key_down_func(unsigned char key, int, int){
    // Day menu handles [ ] speed keys always, plus preset picks when open
    float picked = realtime::g_day_menu.handle_key(key);
    if(picked >= 0.0f){ realtime::g_day.time = picked; return; }
    if(key == '[' || key == ']') return; // consumed by handle_key

    switch(key){
        case 'w': case 'W': key_w=true;  break;
        case 's': case 'S': key_s=true;  break;
        case 'a': case 'A': key_a=true;  break;
        case 'd': case 'D': key_d=true;  break;
        case ' ':            key_up=true; break;
        case 'f': case 'F':
            cam.flying = !cam.flying;
            break;
        case 't': case 'T':
            realtime::g_day_menu.toggle();
            break;
        case '+': cam.speed = std::min(cam.speed*1.3f, 600.0f); break;
        case '-': cam.speed = std::max(cam.speed/1.3f,  30.0f); break;
        case 'g':
        case 'G':
            realtime::g_wind.enabled =
                !realtime::g_wind.enabled;
            break;
        case 27: exit(0);
    }
}
inline void key_up_func(unsigned char key, int, int){
    switch(key){
        case 'w': case 'W': key_w=false;  break;
        case 's': case 'S': key_s=false;  break;
        case 'a': case 'A': key_a=false;  break;
        case 'd': case 'D': key_d=false;  break;
        case ' ':            key_up=false; break;
    }
}

// ─────────────────────────────────────────────────────────────
//  Mouse click → ground ray cast
//  Returns world X,Y where the camera's look direction hits z=0
// ─────────────────────────────────────────────────────────────
inline bool ray_cast_ground(float& wx, float& wy){
    float rad_yaw   = cam.yaw   * (float)M_PI / 180.f;
    float rad_pitch = cam.pitch * (float)M_PI / 180.f;
    float dx = std::cos(rad_pitch)*std::cos(rad_yaw);
    float dy = std::cos(rad_pitch)*std::sin(rad_yaw);
    float dz = std::sin(rad_pitch);
    // Ray: P = cam + t*dir, hits z=0 when cam.z + t*dz = 0
    if(dz >= 0) return false; // looking up/horizontal, no ground hit
    float t = -cam.z / dz;
    if(t < 0 || t > 1200.0f) return false;
    wx = cam.x + t*dx;
    wy = cam.y + t*dy;
    return true;
}

#endif
