#version 430

layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in uint block_id_in;

layout (location = 2) uniform mat4 camera;

out vec3 world_pos;
flat out uint block_id;

void main() {
    gl_Position = camera * vec4(vertex_pos, 1);
    world_pos = vertex_pos;
    block_id = block_id_in;
}
