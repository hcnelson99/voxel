/* Average fps: 37.518550 */
/* 50th percentile: 37.447148 */
/* 99th percentile: 35.968012 */

uint lookup(ivec3 v) {
        return texelFetch(world_buffer, v, 0).r >> 3;
}

uint WORLD_SIZE = textureSize(world_buffer, 0).x;

bool in_bounds(vec3 pos) {
    return 0 <= pos.x && pos.x < WORLD_SIZE && 0 <= pos.y && pos.y < WORLD_SIZE && 0 <= pos.z && pos.z < WORLD_SIZE;
}

uint raycast(vec3 pos, vec3 dir, out vec3 dest_pos, out vec3 normal) {
    uint res;

    if (!in_bounds(pos)) {
        return 0;
    }

    int x = int(clamp(pos.x, 0, WORLD_SIZE - 1));
    int y = int(clamp(pos.y, 0, WORLD_SIZE - 1));
    int z = int(clamp(pos.z, 0, WORLD_SIZE - 1));

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

    int just_out_x = step_x == 1 ? int(WORLD_SIZE) : -1;
    int just_out_y = step_y == 1 ? int(WORLD_SIZE) : -1;
    int just_out_z = step_z == 1 ? int(WORLD_SIZE) : -1;

    res = lookup(ivec3(x, y, z));
    if (res != 0) {
        return res;
    }

    for (int i = 0; i < 1000; ++i) {
        vec3 prev_t_max = vec3(t_max_x, t_max_y, t_max_z);
        vec3 prev_pos = vec3(x, y, z);


        float t_min = min(t_max_x, min(t_max_y, t_max_z));
        int x_smallest = t_max_x == t_min ? 1 : 0;
        int y_smallest = t_max_y == t_min ? 1 : 0;
        int z_smallest = t_max_z == t_min ? 1 : 0;

        x += step_x * x_smallest;
        t_max_x += t_delta_x * x_smallest;

        y += step_y * y_smallest;
        t_max_y += t_delta_y * y_smallest;

        z += step_z * z_smallest;
        t_max_z += t_delta_z * z_smallest;

        if (x == just_out_x || y == just_out_y || z == just_out_z) {
            return 0;
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

