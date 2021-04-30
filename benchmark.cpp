#define STB_INCLUDE_IMPLEMENTATION
#include "stb/stb_include.h"

#include "config.h"
#include "opengl_util.h"
#include "util.h"
#include "world.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <chrono>
#include <iostream>
#include <string>

using namespace std;

World *world;

#define XSTR(x) STR(x)
#define STR(x) #x
#define WORLD_FILE XSTR(WORLD)

std::string world_file = WORLD_FILE;

size_t tick() {
    auto start = std::chrono::steady_clock::now();
    { world->tick(); }
    auto end = std::chrono::steady_clock::now();
    size_t ms_elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    return ms_elapsed;
}

void setup() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to init video\n");
        exit(1);
    }

    SDL_Window *window = NULL;
    window = SDL_CreateWindow("Voxel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_OPENGL);

    if (!window) {
        fprintf(stderr, "failed to init window\n");
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#ifndef NDEBUG
    printf("Initializing OpenGL debug context\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#else
    printf("Initializing OpenGL release context\n");
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "failed to create gl context\n");
        exit(1);
    }
    initialize_opengl();
}

int main() {
    setup();

    world = new World;
    world->initialize();
    world->load(world_file.c_str());

    for (int f = 0; f < 2; f++) {
        world->set_block(0, 0, 0, world->get_block(0, 0, 0));
        size_t first_tick = tick();

        constexpr size_t n = 2;
        size_t ticks[n];
        for (size_t i = 0; i < n; i++) {
            ticks[i] = tick();
        }

        cout << first_tick << endl;
        for (size_t i = 0; i < n; i++) {
            cout << ticks[i] << endl;
        }
    }

    return 0;
}
