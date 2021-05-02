uint block_at(ivec3 pos) {
    return texelFetch(block_map, pos.zyx, 0).r;
}

uint block_at_mipmap(ivec3 pos, int level) {
    uint b = texelFetch(block_map, pos.zyx, level).r;
    if (level == 0) {
        return b >> 3;
    }
    return b;
}
