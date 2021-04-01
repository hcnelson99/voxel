#version 430

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;

in vec2 uv;

out vec4 frag_color;

void main() { 
    if (uv.x < 0.5 && uv.y < 0.5) {
        frag_color = texture(g_position, uv);
    } else if (uv.x < 0.5 && uv.y >= 0.5) {
        frag_color = texture(g_normal, uv);
    } else if (uv.x >= 0.5 && uv.y < 0.5) {
        frag_color = vec4(texture(g_color_spec, uv).xyz, 1);
    } else if (uv.x >= 0.5 && uv.y >= 0.5) {
        frag_color = vec4(vec3(texture(g_color_spec, uv).a), 1);
    }
    /* frag_color = vec4(uv.y, 0, 0, 1); */
}
