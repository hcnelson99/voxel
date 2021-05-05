#version 430

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 g_world_pos[];
in vec2 g_texture_uv[];
flat in vec3 g_normal[];
flat in uint cull[];

out vec3 world_pos;
out vec2 texture_uv;
flat out vec3 normal;

void main() {
    if (cull[0] > 0) {
        EndPrimitive();
        return;
    }

    for (uint i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        world_pos = g_world_pos[i];
        texture_uv = g_texture_uv[i];
        normal = g_normal[i];
        EmitVertex();
    }

    EndPrimitive();
}
