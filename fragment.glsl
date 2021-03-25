#version 410

in vec3 world_pos;

out vec4 frag_color; 

void main() { 
    vec3 dFdxPos = dFdx(world_pos);
    vec3 dFdyPos = dFdy(world_pos);
    vec3 normal = normalize(cross(dFdxPos,dFdyPos ));    

    vec3 color = vec3(1, 0, 1);
    if (abs(normal.x) > 0) {
        color = vec3(0.3);
    } else if (normal.y > 0) {
        color = vec3(0.5);
    } else if (normal.y < 0) {
        color = vec3(0.2);
    } else if (abs(normal.z) > 0) {
        color = vec3(0.4);
    }
    frag_color = vec4(color, 1);
}
