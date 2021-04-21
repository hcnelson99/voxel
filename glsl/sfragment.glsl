#version 430

layout (location = 0) uniform mat4 icamera;
layout (location = 1) uniform uint render_mode;
layout (location = 2) uniform uint player_block_selection;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform usampler3D world_buffer;
layout (binding = 4) uniform sampler2D terrain_texture;
layout (binding = 5) uniform sampler2D lighting_texture;
layout (binding = 6) uniform sampler2D taa_lighting;

in vec2 uv;

out vec4 frag_color;

uint lookup(ivec3 v) {
        return texelFetch(world_buffer, v, 0).r >> 3;
}

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

vec4 raytrace(vec2 uv) {
    vec3 pos = divide_w(icamera * vec4(0, 0, -1, 1));
    vec3 front = divide_w(icamera * vec4(uv * 2 - 1, 1, 1));
    vec3 dir = normalize(front - pos);

    uint blid = raycast(pos, dir);
    return blid_to_color(blid);
}

void gbuffer_debug() {
    vec2 uv_local = uv;
    if (uv.x < 0.5 && uv.y < 0.5) {
        uv_local *= 2;
        frag_color = texture(g_position, uv_local) / WORLD_SIZE;
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