#version 430

layout (location = 0) uniform uint render_mode;
layout (location = 1) uniform uint frame_number;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform usampler3D world_buffer;
layout (binding = 4) uniform sampler2D terrain_texture;
layout (binding = 5) uniform sampler2D blue_noise;

in vec2 uv;

layout (location = 0) out vec4 frag_color;

uint lookup(ivec3 v) {
        return texelFetch(world_buffer, v, 0).r >> 3;
}

vec3 divide_w(vec4 v) {
    return vec3(v.x / v.w, v.y / v.w, v.z / v.w);
}

uint WORLD_SIZE = textureSize(world_buffer, 0).x;

bool in_bounds(vec3 pos) {
    return 0 <= pos.x && pos.x < WORLD_SIZE && 0 <= pos.y && pos.y < WORLD_SIZE && 0 <= pos.z && pos.z < WORLD_SIZE;
}

uint raycast(vec3 pos, vec3 dir) {
    uint res;

    if (!in_bounds(pos)) {
        return 0;
    }
    

    int x = int(clamp(pos.x, 0, WORLD_SIZE - 1));
    int y = int(clamp(pos.y, 0, WORLD_SIZE - 1));
    int z = int(clamp(pos.z, 0, WORLD_SIZE - 1));

    int step_x = int(sign(dir.x));
    int step_y = int(sign(dir.y));
    int step_z = int(sign(dir.z));

    int next_x = step_x == 1 ? 1 : 0;
    int next_y = step_y == 1 ? 1 : 0;
    int next_z = step_z == 1 ? 1 : 0;

    float t_max_x = (x + next_x - pos.x) / dir.x;
    float t_max_y = (y + next_y - pos.y) / dir.y;
    float t_max_z = (z + next_z - pos.z) / dir.z;

    float t_delta_x = abs(1.f / dir.x);
    float t_delta_y = abs(1.f / dir.y);
    float t_delta_z = abs(1.f / dir.z);

    int just_out_x = step_x == 1 ? int(WORLD_SIZE) : -1;
    int just_out_y = step_y == 1 ? int(WORLD_SIZE) : -1;
    int just_out_z = step_z == 1 ? int(WORLD_SIZE) : -1;

    res = lookup(ivec3(x, y, z));
    if (res != 0) {
        return res;
    }

    for (int i = 0; i < 1000; ++i) {
        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                x += step_x;
                if (x == just_out_x) {
                    return 0;
                }
                t_max_x += t_delta_x;
            } else {
                z += step_z;
                if (z == just_out_z) {
                    return 0;
                }
                t_max_z += t_delta_z;
            }
        } else {
            if (t_max_y < t_max_z) {
                y += step_y;
                if (y == just_out_y) {
                    return 0;
                }
                t_max_y += t_delta_y;

            } else {
                z += step_z;
                if (z == just_out_z) {
                    return 0;
                }
                t_max_z += t_delta_z;
            }
        }
        res = lookup(ivec3(x, y, z));
        if (res != 0) {
            return res;
        }
    }
    return 1;
}


uint raycast_pos(vec3 pos, vec3 dir, out vec3 dest_pos, out vec3 normal) {
    uint res;

    if (!in_bounds(pos)) {
        return 0;
    }
    

    int x = int(clamp(pos.x, 0, WORLD_SIZE - 1));
    int y = int(clamp(pos.y, 0, WORLD_SIZE - 1));
    int z = int(clamp(pos.z, 0, WORLD_SIZE - 1));

    int step_x = int(sign(dir.x));
    int step_y = int(sign(dir.y));
    int step_z = int(sign(dir.z));

    int next_x = step_x == 1 ? 1 : 0;
    int next_y = step_y == 1 ? 1 : 0;
    int next_z = step_z == 1 ? 1 : 0;

    float t_max_x = (x + next_x - pos.x) / dir.x;
    float t_max_y = (y + next_y - pos.y) / dir.y;
    float t_max_z = (z + next_z - pos.z) / dir.z;

    float t_delta_x = abs(1.f / dir.x);
    float t_delta_y = abs(1.f / dir.y);
    float t_delta_z = abs(1.f / dir.z);

    int just_out_x = step_x == 1 ? int(WORLD_SIZE) : -1;
    int just_out_y = step_y == 1 ? int(WORLD_SIZE) : -1;
    int just_out_z = step_z == 1 ? int(WORLD_SIZE) : -1;

    res = lookup(ivec3(x, y, z));
    if (res != 0) {
        return res;
    }

    for (int i = 0; i < 1000; ++i) {
        vec3 prev_t_max = vec3(t_max_x, t_max_y, t_max_z);
        vec3 prev_pos = vec3(x, y, z);
        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                x += step_x;
                if (x == just_out_x) {
                    return 0;
                }
                t_max_x += t_delta_x;
            } else {
                z += step_z;
                if (z == just_out_z) {
                    return 0;
                }
                t_max_z += t_delta_z;
            }
        } else {
            if (t_max_y < t_max_z) {
                y += step_y;
                if (y == just_out_y) {
                    return 0;
                }
                t_max_y += t_delta_y;

            } else {
                z += step_z;
                if (z == just_out_z) {
                    return 0;
                }
                t_max_z += t_delta_z;
            }
        }
        res = lookup(ivec3(x, y, z));
        if (res != 0) {
            float t = min(prev_t_max.x, min(prev_t_max.y, prev_t_max.z));
            dest_pos = pos + dir * t;
            normal = prev_pos - vec3(x, y, z);
            return res;
        }
    }
    return 0;
}


int blue_noise_size = textureSize(blue_noise, 0).x;

vec4 generalized_golden_ratio = vec4(1.6180368732830122, 1.3247179943111884, 1.2207440862420311, 1.167303978412138);

float STEP = 0.0005f;

uint rays_per_pixel = 2;
uint bounces_per_pixel = 2;

uint noise_per_pixel = rays_per_pixel * bounces_per_pixel * 2;
vec4 generate_noise(vec2 uv, uint frame_number, uint i) {
    return mod(texture(blue_noise, mod(uv, blue_noise_size)) + generalized_golden_ratio * ((frame_number * 4 + i) % 2243), 1.0);
}

vec3 shadow_ray(vec3 pos, vec3 normal, uint i) {
    vec3 light = vec3(25, 100, 50) + (generate_noise(uv, frame_number, i).xyz * 2 - 1) * vec3(4, 4, 4);
    vec3 dir = normalize(light - pos);

    vec3 sunlight_color = vec3(255, 241, 224) / 255.f;
    if (raycast(pos + normal * STEP, dir) == 0) {
        return 0.5 * sunlight_color * max(dot(normal, dir), 0);
    }
    return vec3(0.001) * sunlight_color;
}

vec3 blid_to_emissive_color(uint blid) {
    if (blid == 5) {
        return normalize(vec3(0.9, 0.05, 0.04)) * 5;
    }  else if (blid == 3) {
        return normalize(vec3(255, 147, 41)) * 5;
    }
    return vec3(0, 0, 0);
}


vec3 light(vec3 pos, vec3 normal, uint i) {
    vec3 res = vec3(0, 0, 0);
    float contribution = 1;

    uint blid = lookup(ivec3(pos - normal * STEP));
    res += blid_to_emissive_color(blid) / 4;

    for (int bounce = 0; bounce < bounces_per_pixel; bounce++) {
        uint j = i * bounces_per_pixel + 2 * bounce;
        vec3 brightness = shadow_ray(pos, normal, j);

        vec4 noise = generate_noise(uv, frame_number, j + 1);
        vec3 dir = normalize(noise.xyz * 2 - 1);
        if (dot(normal, dir) < 0) {
            dir = -dir;
        }
        vec3 new_pos, new_normal;
        uint blid = raycast_pos(pos + normal * STEP, dir, new_pos, new_normal);
        vec3 dist = new_pos - pos;
        brightness += blid_to_emissive_color(blid) / max(dot(dist, dist), 1);

        res += contribution * brightness;
        if (blid == 0) break;
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
