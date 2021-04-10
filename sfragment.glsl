#version 430

layout (location = 0) uniform mat4 iview;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform usampler3D world_buffer;

in vec2 uv;

out vec4 frag_color;

void gbuffer_debug() {
    vec2 uv_local = uv;
    if (uv.x < 0.5 && uv.y < 0.5) {
        uv_local *= 2;
        frag_color = texture(g_position, uv_local);
    } else if (uv.x < 0.5 && uv.y >= 0.5) {
        uv_local.x *= 2;
        uv_local.y = uv_local.y * 2 - 1;
        vec3 norm = texture(g_normal, uv_local).xyz;
        norm.xz = -norm.xz;
        frag_color = vec4(norm, 1);
    } else if (uv.x >= 0.5 && uv.y < 0.5) {
        uv_local.y *= 2;
        uv_local.x = uv_local.x * 2 - 1;
        frag_color = vec4(texture(g_color_spec, uv_local).xyz, 1);
    } else if (uv.x >= 0.5 && uv.y >= 0.5) {
        uv_local = uv_local * 2 - 1;
        vec3 pos = texture(g_position, uv_local).xyz;
        uint blid = texelFetch(world_buffer, ivec3(floor(pos)), 0).r;
        if (blid == 0) {
            frag_color = vec4(0, 0, 0, 1);
        }  else if (blid == 1) {
            frag_color = vec4(0.4902, 0.4902, 0.4902, 1);
        }  else if (blid == 2) {
            frag_color = vec4(0.52157, 0.37647, 0.25882, 1);
        }  else if (blid == 4) {
            frag_color = vec4(0.49412, 0.41961, 0.2549, 1);
        }
    }
}

void main() { 
    frag_color = vec4(texture(g_color_spec, uv).xyz, 1);
    // gbuffer_debug();
}
