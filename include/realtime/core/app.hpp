#pragma once

#include <cmath>
#include <cstdio>

#ifdef _WIN32
    #include <windows.h>

    extern "C" {

    __declspec(dllexport)
    unsigned long NvOptimusEnablement = 0x00000001;

    __declspec(dllexport)
    int AmdPowerXpressRequestHighPerformance = 1;

    }
#endif

#include <GL/glut.h>

#ifdef _WIN32
    #include <GL/wglext.h>
#endif

#include "realtime/camera/camera.hpp"
#include "realtime/object/tree/ltree.hpp"
#include "realtime/render/celestial.hpp"
#include "realtime/render/sky.hpp"
#include "realtime/render/lighting.hpp"

inline float last_time  = 0.0f;
inline float delta_time = 0.016f;

inline realtime::LTree forest;

#ifdef _WIN32

typedef BOOL (__stdcall *PFNWGLSWAPINTERVALEXTPROC)(int);

inline PFNWGLSWAPINTERVALEXTPROC
wglSwapIntervalEXT = nullptr;

#endif

// ─────────────────────────────────────────────
// INPUT
// ─────────────────────────────────────────────
inline void mouse_button(
    int button,
    int state,
    int,
    int
){
    if(button == GLUT_LEFT_BUTTON &&
       state  == GLUT_DOWN){

        float wx, wy;

        if(ray_cast_ground(wx, wy)){
            forest.spawn_tree(wx, wy);
        }
    }
}

inline void print_startup_info(){

    printf("\n");
    printf("========================================\n");
    printf("     PROCEDURAL FOREST ENGINE\n");
    printf("========================================\n\n");

    printf("[System]\n");

    printf(
        " OpenGL Vendor   : %s\n",
        glGetString(GL_VENDOR)
    );

    printf(
        " OpenGL Renderer : %s\n",
        glGetString(GL_RENDERER)
    );

    printf(
        " OpenGL Version  : %s\n",
        glGetString(GL_VERSION)
    );

    printf("\n");

    printf("[Camera]\n");

    printf(
        " Position        : %.1f %.1f %.1f\n",
        cam.x,
        cam.y,
        cam.z
    );

    printf(
        " Yaw/Pitch       : %.1f %.1f\n",
        cam.yaw,
        cam.pitch
    );

    printf(
        " Speed           : %.1f\n",
        cam.speed
    );

    printf("\n");

    printf("[Environment]\n");

    printf(
        " Trees Loaded    : %zu\n",
        forest.trees.size()
    );

    printf(
        " Wind Enabled    : %s\n",
        realtime::g_wind.enabled
            ? "YES"
            : "NO"
    );

    printf(
        " Day Time        : %.2f\n",
        realtime::g_day.time
    );

    printf("\n");

    printf("[Controls]\n");

    printf(" WASD        -> Move\n");
    printf(" Mouse       -> Look Around\n");
    printf(" SHIFT       -> Run\n");
    printf(" F           -> Fly Mode\n");
    printf(" O           -> Wind ON/OFF\n");
    printf(" T           -> Time Menu\n");
    printf(" +/-         -> Camera Speed\n");
    printf(" Left Click  -> Spawn Tree\n");
    printf(" ESC         -> Exit\n");

    printf("\n");
    printf("========================================\n\n");
}

// ─────────────────────────────────────────────
// DISPLAY
// ─────────────────────────────────────────────
inline void display(){

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    glLoadIdentity();

    float rad_yaw =
        cam.yaw * (float)M_PI / 180.f;

    float rad_pitch =
        cam.pitch * (float)M_PI / 180.f;

    float cx =
        std::cos(rad_pitch) *
        std::cos(rad_yaw);

    float cy =
        std::cos(rad_pitch) *
        std::sin(rad_yaw);

    float cz =
        std::sin(rad_pitch);

    gluLookAt(

        cam.x,
        cam.y,
        cam.z,

        cam.x + cx,
        cam.y + cy,
        cam.z + cz,

        0,
        0,
        1
    );

    update_lighting();

    draw_sky();
    draw_sun();
    draw_moon();

    forest.draw();

    glutSwapBuffers();
}

// ─────────────────────────────────────────────
// UPDATE
// ─────────────────────────────────────────────
inline void update(){

    float current =
        glutGet(GLUT_ELAPSED_TIME)
        / 1000.0f;

    delta_time =
        current - last_time;

    last_time = current;

    if(delta_time > 0.05f)
        delta_time = 0.05f;

    update_camera(delta_time);

    realtime::g_day.update(delta_time);

    realtime::g_wind.update(delta_time);

    forest.update(delta_time);

    glutPostRedisplay();
}

// ─────────────────────────────────────────────
// OPENGL INIT
// ─────────────────────────────────────────────
inline void init_gl(){

    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LEQUAL);

    glClearColor(
        0.05f,
        0.07f,
        0.15f,
        1
    );

    glShadeModel(GL_SMOOTH);

    glHint(
        GL_PERSPECTIVE_CORRECTION_HINT,
        GL_NICEST
    );

    glEnable(GLUT_MULTISAMPLE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(
        GL_FRONT_AND_BACK,
        GL_AMBIENT_AND_DIFFUSE
    );

    glEnable(GL_NORMALIZE);

    glEnable(GL_FOG);

    glFogi(
        GL_FOG_MODE,
        GL_EXP2
    );

    glHint(
        GL_FOG_HINT,
        GL_NICEST
    );

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    int sw =
        glutGet(GLUT_SCREEN_WIDTH);

    int sh =
        glutGet(GLUT_SCREEN_HEIGHT);

    gluPerspective(
        70.0,
        sw/(double)sh,
        0.5,
        4000.0
    );

    glMatrixMode(GL_MODELVIEW);

    glutSetCursor(
        GLUT_CURSOR_NONE
    );

    glutWarpPointer(
        sw/2,
        sh/2
    );

    glutPassiveMotionFunc(
        mouse_motion
    );
}

// ─────────────────────────────────────────────
// APP INIT
// ─────────────────────────────────────────────
inline void init_app(
    int argc,
    char** argv
){
    glutInit(&argc, argv);

    glutInitDisplayMode(

        GLUT_DOUBLE |
        GLUT_RGB |
        GLUT_DEPTH |
        GLUT_ALPHA |
        GLUT_MULTISAMPLE
    );

    glutCreateWindow(
        "Procedural Forest"
    );

print_startup_info();

#ifdef _WIN32

    auto addr =
        wglGetProcAddress(
            "wglSwapIntervalEXT"
        );

    wglSwapIntervalEXT =
        reinterpret_cast
        <
            PFNWGLSWAPINTERVALEXTPROC
        >(
            reinterpret_cast<void*>(addr)
        );

    if(wglSwapIntervalEXT)
        wglSwapIntervalEXT(1);

#endif
    glutFullScreen();

    init_gl();

    glutDisplayFunc(display);

    glutIdleFunc(update);

    glutKeyboardFunc(
        key_down_func
    );

    glutKeyboardUpFunc(
        key_up_func
    );

    glutSpecialFunc(
        special_down
    );

    glutSpecialUpFunc(
        special_up
    );

    glutMouseFunc(
        mouse_button
    );
}