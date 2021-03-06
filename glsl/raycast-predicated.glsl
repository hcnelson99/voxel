/* Average fps: 46.328575 */
/* 50th percentile: 45.861009 */
/* 99th percentile: 44.825161 */

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

    while (true) {
        vec3 prev_pos = pos;
        float t_min = min(t_max.x, min(t_max.y, t_max.z));
        ivec3 mask = ivec3(equal(t_max, vec3(t_min)));

        pos += istep * mask;
        t_max += t_delta * mask;

        if (any(equal(pos, just_out))) {
            return 0;
        }

        res = lookup(pos);
        if (res != 0) {
            dest_pos = fpos + dir * t_min;
            normal = prev_pos - pos;
            return res;
        }
    }
    return 0;
}
