#version 430

layout (location = 1) in uint texture_uv_in;
layout (location = 2) in uint block_position;

layout(std430, binding = 1) buffer buffer0 { uint block_map[]; };

layout (location = 2) uniform mat4 camera;

out vec3 world_pos;
flat out uint block_id;
out vec2 texture_uv;

// this should insert a line with const uint world_size = WORLD_SIZE
#inject

#include "util.glsl"
#include "block_map.glsl"
#include "face_orientation_to_block_id.glsl"

void main() {

    uint face = (texture_uv_in >> 5) & 7;
    uint offset_x = (texture_uv_in >> 4) & 1;
    uint offset_y = (texture_uv_in >> 3) & 1;
    uint offset_z = (texture_uv_in >> 2) & 1;

    ivec3 vpos = ivec3(
        (block_position >> 20) & 1023,
        (block_position >> 10) & 1023,
        (block_position >>  0) & 1023
    );
    ivec3 offset = ivec3(offset_x, offset_y, offset_z);

    vec3 vertex_coord = vpos + offset;

    uint block = block_at(vpos);
    block_id = face_orientation_to_block_id(block, face);

    gl_Position = camera * vec4(vertex_coord, 1);
    world_pos = vertex_coord;
    texture_uv = vec2((texture_uv_in >> 1) & 1, texture_uv_in & 1);
}
