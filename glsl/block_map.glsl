uint block_at(ivec3 pos) {
    float size = world_size;
    vec3 _pos = pos / size;
    return texture(block_map, _pos.zyx).r;
}

