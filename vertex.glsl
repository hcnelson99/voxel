#version 140

uniform mat4 camera;

in vec3 vertex_pos;
in vec3 in_color;

out vec3 color;

void main() {
    gl_Position = camera * vec4(vertex_pos, 1);
    color = in_color;
}
