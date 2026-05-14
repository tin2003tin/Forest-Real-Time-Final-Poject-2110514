#ifndef FPS_COUNTER_HPP
#define FPS_COUNTER_HPP

namespace realtime {

struct FPSCounter {

    float fps = 60.0f;

    float timer = 0.0f;

    int frames = 0;

    void update(float dt){

        frames++;

        timer += dt;

        if(timer >= 0.5f){

            fps = frames / timer;

            frames = 0;

            timer = 0.0f;
        }
    }
};

inline FPSCounter g_fps;

} // namespace realtime

#endif