#version 140

uniform mat4 camera;

in vec3 vertex_pos;
in uint block_id_in;

out vec3 world_pos;
flat out uint block_id;

void main() {
    gl_Position = camera * vec4(vertex_pos, 1);
    world_pos = vertex_pos;
    block_id = block_id_in;
}
