#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <string>

static std::string GLSL_PATH = "glsl/";

constexpr uint8_t DELAY_TICKS = 1;

// world is WORLD_SIZE x WORLD_SIZE x WORLD_SIZE
#ifndef WORLD_SIZE
#define WORLD_SIZE (64)
#endif

#define VERTICES_PER_BLOCK (6 * 3 * 2) // 6 faces, 2 triangles per face
#define BLOCKS (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE)
#define VERTICES (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE * VERTICES_PER_BLOCK)

#endif
