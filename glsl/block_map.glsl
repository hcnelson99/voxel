uint block_at(ivec3 pos) {
    uint block_index = pos.x * world_size * world_size + pos.y * world_size + pos.z;

    return (block_map[block_index / 4] >> (8 * (block_index % 4))) & 0xFF;
}

