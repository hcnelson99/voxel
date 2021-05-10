#version 430

layout (location = 0) uniform uint render_mode;
layout (location = 1) uniform uint frame_number;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform usampler3D block_map;
layout (binding = 4) uniform sampler2D terrain_texture;
layout (binding = 5) uniform sampler2D blue_noise;

in vec2 uv;

out vec4 frag_color;

// this should insert a line with const uint world_size = WORLD_SIZE
#inject

#include "block_map.glsl"
#include "util.glsl"
