#version 430

layout (binding = 0) uniform sampler2D terrain_texture;


in vec3 world_pos;
flat in uint block_id;
in vec2 texture_uv;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_color_spec;

void main() { 
    vec3 dFdxPos = dFdx(world_pos);
    vec3 dFdyPos = dFdy(world_pos);
    vec3 normal = normalize(cross(dFdxPos,dFdyPos ));    
    
    float albedo = dot(normal, normalize(vec3(2, 3, 1)));

    albedo = (albedo + 1) / 2;

    /* vec3 n1; */
    /* vec3 n2; */
    /* if (abs(normal.x) > 0) { */
    /*     n1 = -normal.yzx; */
    /*     n2 = normal.yxz; */
    /* } else if (abs(normal.y) > 0) { */
    /*     n1 = normal.yxz; */
    /*     n2 = normal.xzy; */
    /* } else if (abs(normal.z) > 0) { */
    /*     n1 = normal.zxy; */
    /*     n2 = normal.xzy; */
    /* } else { */
    /*     n1 = vec3(0); */
    /*     n2 = vec3(0); */
    /* } */
    /*  */
    /* vec2 tile_uv = fract(vec2(dot(n1, world_pos), -dot(abs(n2), world_pos))); */

    vec2 tile_size = vec2(1, 1) / 16;
    vec2 tile_offset = vec2(block_id % 16, block_id / 16) / 16;

    vec2 tex_coord = tile_offset + tile_size * texture_uv;

    vec3 color = texture(terrain_texture, tex_coord).rgb;

    g_position = world_pos;
    g_normal = normal;
    g_color_spec.rgb = color; // albedo multiplied in for now...
    g_color_spec.a = 1; // no specularity for now...
}
