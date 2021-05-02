bool in_bounds(vec3 pos) {
    return 0 <= pos.x && pos.x < world_size && 0 <= pos.y && pos.y < world_size && 0 <= pos.z && pos.z < world_size;
}

uint raycast(vec3 fpos, vec3 dir, out vec3 dest_pos, out vec3 normal) {
    if (!in_bounds(fpos)) {
        return 0;
    }

    ivec3 pos = ivec3(clamp(fpos, 0, world_size - 1));

    uint res = lookup(pos);
    if (res != 0) {
        return res;
    }

    ivec3 istep = ivec3(sign(dir));

    ivec3 next = clamp(istep, 0, 1);

    vec3 t_max = (pos + next - fpos) / dir;

    vec3 t_delta = abs(1.f / dir);

    ivec3 just_out = next * ivec3(world_size + 1) - 1;

    int level = 0;
    int count = 0;
    int scale = 1;

    for (int i = 0; i < 100; i++) {
        vec3 prev_pos = pos;
        float t_min = min(t_max.x, min(t_max.y, t_max.z));
        ivec3 mask = ivec3(equal(t_max, vec3(t_min)));

        pos += istep * mask;
        t_max += t_delta * mask * scale;

        if (any(equal(pos, just_out))) {
            return 0;
        }

        res = block_at_mipmap(pos, level);
        if (res != 0) {
            if (level == 0) {
                dest_pos = fpos + dir * t_min;
                normal = prev_pos - pos;
                return res;
            } else {
                level--;
                scale /= 2;
                pos = ivec3((fpos + t_min * dir) / scale);
                t_max = (pos * scale + next * scale - fpos) / dir;
                just_out = next * ivec3(world_size / scale + 1) - 1;
                count = 0;
            }
        }

        count++;
        if (count > 4 && level < 6) {
            if (block_at_mipmap(pos / 2, level + 1) == 0) {
                level++;
                scale *= 2;
                pos /= 2;
                t_max = ivec3((pos * scale + next * scale - fpos) / dir);
                just_out = next * ivec3(world_size / scale + 1) - 1;
            }
            count = 0;
        }
    }
    return 0;
}

