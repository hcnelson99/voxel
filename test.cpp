#include <assert.h>
#include <iostream>
#include <string>

#define STB_INCLUDE_IMPLEMENTATION

#include "config.h"
#include "opengl_util.h"
#include "util.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "redstone_tick.h"

int main(int argc, char *argv[]) {
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

    RedstoneTickShader shader;
    shader.init();
}
