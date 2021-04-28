#pragma once

#include <stdint.h>
#include <string>

static std::string GLSL_PATH = "glsl/";

constexpr uint8_t DELAY_TICKS = 1;

// world is WORLD_SIZE x WORLD_SIZE x WORLD_SIZE
#ifndef WORLD_SIZE
#define WORLD_SIZE (64)
#endif
