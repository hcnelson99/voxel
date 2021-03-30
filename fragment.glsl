#version 410

uniform sampler2D terrain_texture;
uniform usampler3D world_texture;

in vec3 world_pos;
flat in uint block_id;

out vec4 frag_color; 

void main() { 
    vec3 dFdxPos = dFdx(world_pos);
    vec3 dFdyPos = dFdy(world_pos);
    vec3 normal = normalize(cross(dFdxPos,dFdyPos ));    
    
    float albedo = dot(normal, normalize(vec3(2, 3, 1)));

    albedo = (albedo + 1) / 2;

    vec3 n1;
    vec3 n2;
    if (abs(normal.x) > 0) {
        n1 = -normal.yzx;
        n2 = normal.yxz;
    } else if (abs(normal.y) > 0) {
        n1 = normal.yxz;
        n2 = normal.xzy;
    } else if (abs(normal.z) > 0) {
        n1 = normal.zxy;
        n2 = normal.xzy;
    } else {
        n1 = vec3(0);
        n2 = vec3(0);
    }

    vec2 tile_uv = fract(vec2(dot(n1, world_pos), -dot(abs(n2), world_pos)));

    vec2 tile_size = vec2(1, 1) / 16;
    vec2 tile_offset = vec2(block_id % 16, block_id / 16) / 16;

    vec2 tex_coord = tile_offset + tile_size * tile_uv;

    vec3 color = texture(terrain_texture, tex_coord).rgb;

    ivec3 block_pos = ivec3(floor(world_pos));
    block_pos = ivec3(1, 1, 1);

    ivec3 test = textureSize(world_texture, 0);

    uint block_type; // = texelFetch(world_texture, block_pos, 0).r;

    block_type = texture(world_texture, ivec3(15, 15, 15)).r;
    if (block_type == 1) {
        color = vec3(1, 0, 0);
    }

    frag_color = vec4(color * albedo, 1);
}
