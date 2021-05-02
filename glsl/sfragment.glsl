#version 430

layout (location = 0) uniform mat4 icamera;
layout (location = 1) uniform uint render_mode;
layout (location = 2) uniform uint player_block_selection;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform usampler3D block_map;
layout (binding = 4) uniform sampler2D terrain_texture;
layout (binding = 5) uniform sampler2D lighting_texture;
layout (binding = 6) uniform sampler2D taa_lighting;

in vec2 uv;

out vec4 frag_color;

// this should insert a line with const uint world_size = WORLD_SIZE
#inject

#include "block_map.glsl"
#include "util.glsl"
#include "raycast.glsl"

vec4 blid_to_color(uint blid) {
    if (blid == 0) {
        return vec4(0, 0, 0, 1);
    }  else if (blid == 1) {
        return vec4(0.4902, 0.4902, 0.4902, 1);
    }  else if (blid == 2) {
        return vec4(0.52157, 0.37647, 0.25882, 1);
    }  else if (blid == 3) {
        return vec4(0.49412, 0.41961, 0.2549, 1);
    } else {
        return vec4(1, 1, 0, 1);
    }
}

vec4 debug_world_buffer(vec2 uv) {
    vec3 pos = texture(g_position, uv).xyz;
    uint blid = lookup(ivec3(floor(pos)));
    return blid_to_color(blid);
}

vec4 raytrace(vec2 uv) {
    vec3 pos = divide_w(icamera * vec4(0, 0, -1, 1));
    vec3 front = divide_w(icamera * vec4(uv * 2 - 1, 1, 1));
    vec3 dir = normalize(front - pos);

    vec3 unused1, unused2;
    uint blid = raycast(pos, dir, unused1, unused2);
    return blid_to_color(blid);
}

void gbuffer_debug() {
    vec2 uv_local = uv;
    if (uv.x < 0.5 && uv.y < 0.5) {
        uv_local *= 2;
        frag_color = texture(g_position, uv_local) / world_size;
    } else if (uv.x < 0.5 && uv.y >= 0.5) {
        uv_local.x *= 2;
        uv_local.y = uv_local.y * 2 - 1;
        vec3 norm = texture(g_normal, uv_local).xyz;
        frag_color = vec4(norm, 1);
    } else if (uv.x >= 0.5 && uv.y < 0.5) {
        uv_local.y *= 2;
        uv_local.x = uv_local.x * 2 - 1;
        frag_color = vec4(texture(g_color_spec, uv_local).xyz, 1);
    } else if (uv.x >= 0.5 && uv.y >= 0.5) {
        uv_local = uv_local * 2 - 1;
        // frag_color = debug_world_buffer(uv_local);
        frag_color = raytrace(uv_local);
    }
}

float width = 1920;
float height = 1080;
float aspect_ratio = width / height;

float crosshair_size = 0.04;
vec2 crosshair_box = vec2(crosshair_size / aspect_ratio, crosshair_size);


void draw_crosshair(vec2 uv) {
    vec2 center = vec2(0.5, 0.5);
    vec2 uv_min = center - crosshair_box / 2;
    vec2 uv_max = center + crosshair_box / 2;

    if (uv.x < uv_min.x || uv.y < uv_min.y || uv.x > uv_max.x || uv.y > uv_max.y) {
        return;
    }

    vec2 crosshair_uv = (uv - uv_min) / (uv_max - uv_min);

    vec2 tile_size = vec2(1, 1) / 16;
    vec2 tile_offset = vec2(0, 14) / 16;

    vec2 tex_coord = tile_offset + tile_size * crosshair_uv;

    vec4 tex_color = texture(terrain_texture, tex_coord);
    frag_color = vec4(mix(frag_color.rgb, tex_color.rgb, tex_color.a * 0.7), tex_color.a * 0.7);
}

float block_selection_size = 0.05;
void draw_block_selection(vec2 uv) {
    vec2 uv_min = vec2(0);
    vec2 uv_max = vec2(block_selection_size / aspect_ratio, block_selection_size);

    if (uv.x < uv_min.x || uv.y < uv_min.y || uv.x > uv_max.x || uv.y > uv_max.y) {
        return;
    }

    vec2 texture_uv = (uv - uv_min) / (uv_max - uv_min);

    vec2 tile_size = vec2(1, 1) / 16;

    uint tile_id = (player_block_selection >> 3) * 6 + 3;
    vec2 tile_offset = vec2(tile_id % 16, tile_id / 16) / 16;

    vec2 tex_coord = tile_offset + tile_size * texture_uv;

    vec4 tex_color = texture(terrain_texture, tex_coord);
    frag_color = vec4(mix(frag_color.rgb, tex_color.rgb, tex_color.a * 0.4), tex_color.a * 0.4);

}

void draw_gui(vec2 uv) {
    draw_crosshair(uv);
    draw_block_selection(uv);
}

vec3 gamma_correct(vec3 color) {
    return pow(color, vec3(1.f / 2.2));
}

vec3 blur(vec2 uv) {
    float dx = 1.f / width;
    float dy = 1.f / height;

    vec3 res = vec3(0);
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            res += texture(taa_lighting, uv + vec2(dx * i, dy * j)).rgb;
        }
    }
    res /= 9;
    return res;
}

void main() {
    if (render_mode == 0) {
        vec3 color = texture(g_color_spec, uv).xyz;
        vec3 brightness = texture(taa_lighting, uv).xyz;

        vec3 rgb = color * gamma_correct(brightness);

        frag_color = vec4(rgb, 1);
        draw_gui(uv);
    }  else if (render_mode == 1) {
        vec3 color = texture(g_color_spec, uv).xyz;
        vec3 brightness = texture(lighting_texture, uv).xyz;

        vec3 rgb = color * gamma_correct(brightness);

        frag_color = vec4(rgb, 1);
        draw_gui(uv);
    } else if (render_mode == 2) {
        vec3 brightness = texture(taa_lighting, uv).xyz;
        vec3 rgb = gamma_correct(brightness);

        frag_color = vec4(rgb, 1);
        draw_gui(uv);
    } else if (render_mode == 3) {
        vec3 brightness = texture(lighting_texture, uv).xyz;
        vec3 rgb = gamma_correct(brightness);

        frag_color = vec4(rgb, 1);
        draw_gui(uv);
    }  else if (render_mode == 4) {
        gbuffer_debug();
    }
}
