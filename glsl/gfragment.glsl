#version 430

layout (binding = 0) uniform sampler2D terrain_texture;

in vec3 world_pos;
in vec2 texture_uv;
flat in vec3 normal;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_color_spec;

void main() {
    vec3 color = texture(terrain_texture, texture_uv).rgb;

    g_position = world_pos;
    g_normal = normal;
    g_color_spec.rgb = color;
    g_color_spec.a = 1; // no specularity for now...
}
