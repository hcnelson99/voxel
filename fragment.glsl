#version 410

uniform sampler2D terrain_texture;

in vec3 world_pos;

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

    vec2 tileUV = fract(vec2(dot(n1, world_pos), -dot(abs(n2), world_pos)));

    vec2 tileSize = vec2(1, 1) / 16;
    vec2 tileOffset = vec2(1, 1) / 16;

    vec2 texCoord = tileOffset + tileSize * tileUV;

    vec3 color = texture(terrain_texture, texCoord).rgb;

    frag_color = vec4(color * albedo, 1);
}
