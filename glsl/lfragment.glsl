#version 430

layout (location = 0) uniform uint render_mode;
layout (location = 1) uniform uint frame_number;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform usampler3D block_map;
layout (binding = 4) uniform sampler2D terrain_texture;
layout (binding = 5) uniform sampler2D blue_noise;

in vec2 uv;

out vec4 frag_color;

// this should insert a line with const uint world_size = WORLD_SIZE
#inject

#include "block_map.glsl"
#include "util.glsl"
#include "raycast.glsl"

int blue_noise_size = textureSize(blue_noise, 0).x;

vec4 generalized_golden_ratio = vec4(1.6180368732830122, 1.3247179943111884, 1.2207440862420311, 1.167303978412138);

float STEP = 0.0005f;

uint rays_per_pixel = 2;
uint bounces_per_pixel = 2;

uint noise_per_pixel = rays_per_pixel * bounces_per_pixel * 2;
vec4 generate_noise(vec2 uv, uint frame_number, uint i) {
    return mod(texture(blue_noise, mod(uv, blue_noise_size)) + generalized_golden_ratio * ((frame_number * 4 + i) % 2243), 1.0);
}

float sunlight_brightness = 0.5;
float unlit_brightness = 0.03;
float block_brightness = 5;

vec3 shadow_ray(vec3 pos, vec3 normal, uint i) {
    vec3 light = vec3(25, 100, 50) + (generate_noise(uv, frame_number, i).xyz * 2 - 1) * vec3(4, 4, 4);
    vec3 dir = normalize(light - pos);

    vec3 sunlight_color = vec3(255, 241, 224) / 255.f;
    vec3 unused1, unused2;
    if (raycast(pos + normal * STEP, dir, unused1, unused2) == 0) {
        return sunlight_brightness * sunlight_color * max(dot(normal, dir), 0);
    }
    return vec3(unlit_brightness) * sunlight_color;
}

vec3 blid_to_emissive_color(uint blid) {
    if (blid == 4) {
        return normalize(vec3(0.9, 0.05, 0.04)) * block_brightness;
    }  else if (blid == 3) {
        return normalize(vec3(255, 147, 41)) * block_brightness;
    } else if (blid == 16) {
        return normalize(vec3(0.04, 0.05, 0.9)) * block_brightness;
    } else if (blid == 18) {
        return normalize(vec3(0.04, 0.9, 0.05)) * block_brightness;
    }
    return vec3(0, 0, 0);
}


vec3 light(vec3 pos, vec3 normal, uint i) {
    vec3 res = vec3(unlit_brightness);
    float contribution = 1;

    uint blid = lookup(ivec3(pos - normal * STEP));
    res += blid_to_emissive_color(blid) / 4;

    for (int bounce = 0; bounce < bounces_per_pixel; bounce++) {
        uint j = i * bounces_per_pixel + 2 * bounce;
        res += contribution * shadow_ray(pos, normal, j);

        vec4 noise = generate_noise(uv, frame_number, j + 1);
        vec3 dir = normalize(noise.xyz * 2 - 1);
        if (dot(normal, dir) < 0) {
            dir = -dir;
        }
        vec3 new_pos, new_normal;
        uint blid = raycast(pos + normal * STEP, dir, new_pos, new_normal);
        if (blid == 0) break;
        vec3 dist = new_pos - pos;
        vec3 brightness = blid_to_emissive_color(blid) / max(dot(dist, dist), 1);
        res += contribution * brightness;

        contribution *= max(dot(dir, normal), 0);
        pos = new_pos;
        normal = new_normal;
    }

    return res;
}

vec3 lighting(vec2 uv) {

    vec3 brightness = vec3(0);

    for (int i = 0; i < rays_per_pixel; i++) {
        vec3 pos = texture(g_position, uv).xyz;
        vec3 normal = texture(g_normal, uv).xyz;
        brightness += light(pos, normal, i);
    }

    brightness /= rays_per_pixel;

    return brightness;
}

void main() {
    vec3 brightness = lighting(uv);
    frag_color = vec4(brightness, 1);
}
