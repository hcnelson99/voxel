#version 430

layout (location = 0) in vec3 vertex_pos;
layout (location = 2) in uint texture_uv_in;

layout (location = 3) uniform mat4 camera;

out vec3 world_pos;
flat out uint block_id;
out vec2 texture_uv;

// this should insert a line with const uint world_size = WORLD_SIZE
#inject

layout(std430, binding = 1) buffer buffer0 { uint block_map[]; };

void main() {
    gl_Position = camera * vec4(vertex_pos, 1);
    world_pos = vertex_pos;

    uint face = (texture_uv_in >> 5) & 7;
    uint offset_x = (texture_uv_in >> 4) & 1;
    uint offset_y = (texture_uv_in >> 3) & 1;
    uint offset_z = (texture_uv_in >> 2) & 1;

    ivec3 vpos = ivec3(vertex_pos) - ivec3(offset_x, offset_y, offset_z);
    uint block_index = vpos.x * world_size * world_size + vpos.y * world_size + vpos.z;

    uint block = (block_map[block_index / 4] >> (8 * (block_index % 4))) & 0xff;
    uint orientation = block & 7;

    if (orientation == face) {
        block_id = (block >> 3) * 6 + 1;
    } else if (orientation / 2 == face / 2) {
        block_id = (block >> 3) * 6 + 0;
    } else {
        uint offset = 0;
        if (orientation == 1) {
            offset = 2;
        } else if (orientation == 0) {
            offset = 0;
        } else if (orientation == 3) {
            offset = 3;
        } else if (orientation == 2) {
            offset = 1;
        } else if (orientation == 5 && face / 2 == 0) {
            offset = 2;
        } else if (orientation == 5 && face == 3) {
            offset = 3;
        } else if (orientation == 5 && face == 2) {
            offset = 3;
        } else if (orientation == 4 && face / 2 == 0) {
            offset = 0;
        } else if (orientation == 4 && face == 3) {
            offset = 1;
        } else if (orientation == 4 && face == 2) {
            offset = 1;
        }
        block_id = (block >> 3) * 6 + 2 + offset;
    }

    texture_uv = vec2((texture_uv_in >> 1) & 1, texture_uv_in & 1);
}
