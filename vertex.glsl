#version 140

uniform mat4 camera;

in vec3 vertex_pos;

out vec3 world_pos;

void main() {
    gl_Position = camera * vec4(vertex_pos, 1);
    world_pos = vertex_pos;
}
