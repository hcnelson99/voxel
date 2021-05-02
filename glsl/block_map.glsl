uint block_at(ivec3 pos) {
    return texelFetch(block_map, pos.zyx, 0).r;
}

