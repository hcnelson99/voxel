#version 430

layout (location = 0) uniform mat4 icamera;
layout (location = 1) uniform uint render_mode;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform usampler3D world_buffer;
layout (binding = 4) uniform sampler2D terrain_texture;


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

float bbox_hit(vec3 bbox_min, vec3 bbox_max, vec3 pos, vec3 dir) {
    vec3 invdir = 1.f / dir;

    float tmin, tmax;
    if (invdir.x >= 0) {
        tmin = (bbox_min.x - pos.x) * invdir.x;
        tmax = (bbox_max.x - pos.x) * invdir.x;
    } else {
        tmin = (bbox_max.x - pos.x) * invdir.x;
        tmax = (bbox_min.x - pos.x) * invdir.x;
    }

    float tymin, tymax;

    if (invdir.y >= 0) {
        tymin = (bbox_min.y - pos.y) * invdir.y;
        tymax = (bbox_max.y - pos.y) * invdir.y;
    } else {
        tymin = (bbox_max.y - pos.y) * invdir.y;
        tymax = (bbox_min.y - pos.y) * invdir.y;
    }

    if ((tmin > tymax) || (tymin > tmax))
        return -1;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin, tzmax;
    if (invdir.z >= 0) {
        tzmin = (bbox_min.z - pos.z) * invdir.z;
        tzmax = (bbox_max.z - pos.z) * invdir.z;
    } else {
        tzmin = (bbox_max.z - pos.z) * invdir.z;
        tzmax = (bbox_min.z - pos.z) * invdir.z;
    }

    if ((tmin > tzmax) || (tzmin > tmax))
        return -1;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    bool hit = false;

    if (tmin >= 0) {
        return tmin;
    }
    if (tmax >= 0) {
        return tmax;
    }
    return -1;
}

uint raycast(vec3 pos, vec3 dir) {
    uint res;

    if (!in_bounds(pos)) {
        vec3 bbox_min = vec3(0, 0, 0);
        vec3 bbox_max = vec3(WORLD_SIZE, WORLD_SIZE, WORLD_SIZE);

        float t = bbox_hit(bbox_min, bbox_max, pos, dir);
        if (t >= 0) {
            pos += dir * t;
        } else {
            return 0;
        }
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



vec4 raytrace(vec2 uv) {
    uv = uv * 2 - 1;

    vec3 pos = divide_w(icamera * vec4(0, 0, 0, 1));
    vec3 front = divide_w(icamera * vec4(uv, 1, 1));
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
        norm.xz = -norm.xz;
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

void main() { 
    if (render_mode == 0) {
        frag_color = vec4(texture(g_color_spec, uv).xyz, 1);
    } else  {
        frag_color = raytrace(uv);
    }

    draw_crosshair(uv);

    /* gbuffer_debug(); */
}
