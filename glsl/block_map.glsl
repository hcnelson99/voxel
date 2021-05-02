uint block_at(ivec3 pos) {
    return texelFetch(block_map, pos.zyx, 0).r;
}

uint lookup(ivec3 v) {
    return block_at(v) >> 3;
}

uint block_at_mipmap(ivec3 pos, int level) {
    return texelFetch(block_map, pos.zyx, level).r >> 3;
}

