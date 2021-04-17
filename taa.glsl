#version 430

layout (location = 0) uniform mat4 icamera;
layout (location = 1) uniform mat4 camera_prev;
layout (location = 2) uniform uint render_mode;

layout (binding = 0) uniform sampler2D g_position;
layout (binding = 1) uniform sampler2D g_normal;
layout (binding = 2) uniform sampler2D g_color_spec;
layout (binding = 3) uniform sampler2D lighting_texture;
layout (binding = 4) uniform sampler2D taa_prev;
layout (binding = 5) uniform sampler2D g_position_prev;

in vec2 uv;

out vec4 frag_color;

float alpha = 0.1;

float dist_sq(vec3 v) {
    return dot(v, v);
}

float width = 1920;
float height = 1080;
    float dx = 1.f / width;
float dy = 1.f / height;

vec3 box(sampler2D tex, vec2 uv) {
    vec3 res = vec3(0, 0, 0);

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            vec2 diff = vec2(dx * i, dy * j);
            res += texture(tex, uv + diff).rgb;
        }
    }
    res /= 9;
    return res;
}

bool in_bounds(vec2 prev_uv) {
    return (0 <= prev_uv.x && prev_uv.x <= 1 && 0 <= prev_uv.y && prev_uv.y <= 1);
}

bool pos_changed(vec2 prev_uv) {
    return dist_sq(box(g_position, uv).xyz - box(g_position_prev, prev_uv).xyz) >= 0.001;
}

// https://software.intel.com/en-us/node/503873
vec3 RGB_to_YCoCg(vec3 c)
{
    // Y = R/4 + G/2 + B/4
    // Co = R/2 - B/2
    // Cg = -R/4 + G/2 - B/4
    return vec3(
            c.x/4.0 + c.y/2.0 + c.z/4.0,
            c.x/2.0 - c.z/2.0,
        -c.x/4.0 + c.y/2.0 - c.z/4.0
    );
}

// https://software.intel.com/en-us/node/503873
vec3 YCoCg_to_RGB(vec3 c)
{
    // R = Y + Co - Cg
    // G = Y + Cg
    // B = Y - Co - Cg
    return clamp(vec3(
        c.x + c.y - c.z,
        c.x + c.z,
        c.x - c.y - c.z
    ), vec3(0.0), vec3(1.0));
}

void main() { 
    vec3 worldspace_pos = texture(g_position, uv).xyz;
    vec4 prev_pos = camera_prev * vec4(worldspace_pos, 1);
    vec2 prev_uv = 0.5 * prev_pos.xy / prev_pos.w + 0.5;

    vec3 lighting = box(lighting_texture, uv).rgb;
    vec3 taa_prev = texture(taa_prev, prev_uv).rgb;

    if (!in_bounds(prev_uv) || pos_changed(prev_uv)) {
        frag_color = vec4(lighting, 1);
    } else {
        float alpha = 0.1;

        vec3 lighting_ycocg = RGB_to_YCoCg(lighting);
        vec3 taa_prev_ycocg = RGB_to_YCoCg(taa_prev);


        vec3 s1 = RGB_to_YCoCg(texture(lighting_texture, uv + vec2(-dx, 0)).rgb);
        vec3 s2 = RGB_to_YCoCg(texture(lighting_texture, uv + vec2(dx, 0)).rgb);
        vec3 s3 = RGB_to_YCoCg(texture(lighting_texture, uv + vec2(0, dy)).rgb);
        vec3 s4 = RGB_to_YCoCg(texture(lighting_texture, uv + vec2(0, -dy)).rgb);

        vec3 half_size = vec3(0.2);
        vec3 bbox_min = min(lighting_ycocg, min(min(s1, s2), min(s3, s4))) - half_size;
        vec3 bbox_max = max(lighting_ycocg, max(max(s1, s2), max(s3, s4))) + half_size;

        vec3 clamped_taa_prev_ycocg = min(max(bbox_min, taa_prev_ycocg), bbox_max);
        vec3 clamped_taa_prev = YCoCg_to_RGB(clamped_taa_prev_ycocg);

        frag_color = vec4(clamped_taa_prev * (1.f - alpha) + lighting * alpha, 1);

    }


    // if (!(0 <= prev_uv.x && prev_uv.x <= 1 && 0 <= prev_uv.y && prev_uv.y <= 1)) {
    //     frag_color = vec4(1, 0, 0, 1);
    // } else {
    //     frag_color = vec4(0, 0, 0, 1);
    // }


}
