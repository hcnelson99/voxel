// Average fps: 22.865364
// 50th percentile: 22.870298
// 99th percentile: 17.122457

bool in_bounds(vec3 pos) {
    return 0 <= pos.x && pos.x < world_size && 0 <= pos.y && pos.y < world_size && 0 <= pos.z && pos.z < world_size;
}

uint raycast(vec3 pos, vec3 dir, out vec3 dest_pos, out vec3 normal) {
    if (!in_bounds(pos)) {
        return 0;
    }


    int x = int(clamp(pos.x, 0, world_size - 1));
    int y = int(clamp(pos.y, 0, world_size - 1));
    int z = int(clamp(pos.z, 0, world_size - 1));

    uint res = lookup(ivec3(x, y, z));
    if (res != 0) {
        return res;
    }


    int step_x = int(sign(dir.x));
    int step_y = int(sign(dir.y));
    int step_z = int(sign(dir.z));

    int next_x = step_x == 1 ? 1 : 0;
    int next_y = step_y == 1 ? 1 : 0;
    int next_z = step_z == 1 ? 1 : 0;

    float t_max_x = (x + next_x - pos.x) / dir.x;
    float t_max_y = (y + next_y - pos.y) / dir.y;
    float t_max_z = (z + next_z - pos.z) / dir.z;

    float t_delta_x = abs(1.f / dir.x);
    float t_delta_y = abs(1.f / dir.y);
    float t_delta_z = abs(1.f / dir.z);

    int just_out_x = step_x == 1 ? int(world_size) : -1;
    int just_out_y = step_y == 1 ? int(world_size) : -1;
    int just_out_z = step_z == 1 ? int(world_size) : -1;

    // even though raycast-predicated has this as a while (true), don't change
    // it here. crashes video drivers. we should investigate this.
    for (int i = 0; i < 1000; ++i) {
        vec3 prev_t_max = vec3(t_max_x, t_max_y, t_max_z);
        vec3 prev_pos = vec3(x, y, z);
        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                x += step_x;
                if (x == just_out_x) {
                    return 0;
                }
                t_max_x += t_delta_x;
            } else {
                z += step_z;
                if (z == just_out_z) {
                    return 0;
                }
                t_max_z += t_delta_z;
            }
        } else {
            if (t_max_y < t_max_z) {
                y += step_y;
                if (y == just_out_y) {
                    return 0;
                }
                t_max_y += t_delta_y;

            } else {
                z += step_z;
                if (z == just_out_z) {
                    return 0;
                }
                t_max_z += t_delta_z;
            }
        }
        res = lookup(ivec3(x, y, z));
        if (res != 0) {
            float t = min(prev_t_max.x, min(prev_t_max.y, prev_t_max.z));
            dest_pos = pos + dir * t;
            normal = prev_pos - vec3(x, y, z);
            return res;
        }
    }
    return 0;
}

