#pragma once

#include <assert.h>
#include <iostream>
#include <string>

#include "stb/stb_include.h"

#include "config.h"
#include "opengl_util.h"
#include "util.h"
#include <GL/glew.h>

class RedstoneTickShader {
  public:
    void init();

    struct TickData {
        uint8_t *block_map;
        uint8_t *expression_values;
        uint32_t *block_to_expression;
    };

    void tick(TickData tick_data);

  private:
    static GLuint create_compute_shader(std::string filename);

    GLuint gComputeProgram;
    GLuint _block_map;
    GLuint _expression_values;
    GLuint _block_to_expression;
};

inline GLuint RedstoneTickShader::create_compute_shader(std::string filename) {
    GLuint gComputeProgram = glCreateProgram();

    // Create and compile the compute shader
    GLuint mComputeShader = glCreateShader(GL_COMPUTE_SHADER);
    {
        std::string path = GLSL_PATH + filename;
        char inject[1] = "";
        char error[256];
        char *code = stb_include_file(&path[0], inject, &GLSL_PATH[0], error);
        if (!code) {
            fprintf(stderr, "%s\n", error);
            exit(1);
        }

        glShaderSource(mComputeShader, 1, &code, NULL);

        free(code);
    }

    glCompileShader(mComputeShader);

    // Check if there were any issues when compiling the shader

    int rvalue;

    glGetShaderiv(mComputeShader, GL_COMPILE_STATUS, &rvalue);

    int length;
    char log[256];
    if (!rvalue) {
        glGetShaderInfoLog(mComputeShader, sizeof(log), &length, log);
        printf("Error: Compiler log:\n%s\n", log);
        exit(1);
    }

    glAttachShader(gComputeProgram, mComputeShader);

    glLinkProgram(gComputeProgram);

    glGetProgramiv(gComputeProgram, GL_LINK_STATUS, &rvalue);

    if (!rvalue) {
        glGetProgramInfoLog(gComputeProgram, sizeof(log), &length, log);
        printf("Error: Linker log:\n%s\n", log);
        exit(1);
    }

    return gComputeProgram;
}

inline void RedstoneTickShader::init() {
    gComputeProgram = create_compute_shader("redstone_tick.glsl");

    glUseProgram(gComputeProgram);

    //{
    //    GLuint iLocRadius = glGetUniformLocation(gComputeProgram, "radius");
    //    glUniform1ui(iLocRadius, 69);
    //}

    const auto create_buffer = [&](GLuint &buffer, size_t size, const GLuint binding) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer);
    };

    create_buffer(_block_map, sizeof(uint8_t) * BLOCKS, 0);
    create_buffer(_expression_values, sizeof(uint8_t) * BLOCKS, 2);
    create_buffer(_block_to_expression, sizeof(uint32_t) * BLOCKS, 4);
}

inline void RedstoneTickShader::tick(TickData tick_data) {
    glUseProgram(gComputeProgram);

    const auto host_to_device = [&](void *src, GLuint &buffer, size_t size) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, src);
    };

    // CPU -> GPU transfer
    {
        host_to_device(tick_data.block_map, _block_map, sizeof(uint8_t) * BLOCKS);
        host_to_device(tick_data.expression_values, _expression_values, sizeof(uint8_t) * BLOCKS);
        host_to_device(tick_data.block_to_expression, _block_to_expression, sizeof(uint32_t) * BLOCKS);
    }

    // Execute the shader
    {
        const size_t block_size = WORLD_SIZE / 8;
        glUseProgram(gComputeProgram);
        glDispatchCompute(block_size, block_size, block_size / 4);
    }

    const auto device_to_host = [&](void *dest, GLuint &buffer, size_t size) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        void *deviceBuffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size / 4, GL_MAP_READ_BIT);
        memcpy(dest, deviceBuffer, size);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    };

    // GPU -> CPU transfer
    device_to_host(tick_data.block_map, _block_map, sizeof(uint8_t) * BLOCKS);
}
