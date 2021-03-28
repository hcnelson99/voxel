#version 410

in vec3 world_pos;

out vec4 frag_color; 

void main() { 
    vec3 dFdxPos = dFdx(world_pos);
    vec3 dFdyPos = dFdy(world_pos);
    vec3 normal = normalize(cross(dFdxPos,dFdyPos ));    
    
    float albedo = dot(normal, normalize(vec3(2, 3, 1)));

    frag_color = vec4(vec3((albedo + 1)/ 2 ), 1);
}
