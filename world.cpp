#include "world.h"

#include <algorithm>
#include <fcntl.h>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <optional>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

const Orientation Orientation::NegX(0);
const Orientation Orientation::PosX(1);
const Orientation Orientation::NegY(2);
const Orientation Orientation::PosY(3);
const Orientation Orientation::NegZ(4);
const Orientation Orientation::PosZ(5);
const Axis Orientation::_axis_map[6] = {Axis::X, Axis::X, Axis::Y, Axis::Y, Axis::Z, Axis::Z};
const Orientation Orientation::_opposite_map[6] = {Orientation::PosX, Orientation::NegX, Orientation::PosY,
                                                   Orientation::NegY, Orientation::PosZ, Orientation::NegZ};
const Vec3 Orientation::_direction_map[6] = {Vec3(-1, 0, 0), Vec3(1, 0, 0),  Vec3(0, -1, 0),
                                             Vec3(0, 1, 0),  Vec3(0, 0, -1), Vec3(0, 0, 1)};
const Orientation Orientation::orientations[] = {Orientation::NegX, Orientation::PosX, Orientation::NegY,
                                                 Orientation::PosY, Orientation::NegZ, Orientation::PosZ};

std::ostream &operator<<(std::ostream &strm, const Orientation &o) { return strm << o.to_string(); }

std::ostream &operator<<(std::ostream &strm, const Block &b) { return strm << b.to_string(); }

std::string Orientation::to_string() const {
    switch (_orientation) {
    case 0:
        return "NegX";
    case 1:
        return "PosX";
    case 2:
        return "NegY";
    case 3:
        return "PosY";
    case 4:
        return "NegZ";
    case 5:
        return "PosZ";
    default:
        assert(false);
        return "";
    }
}

Orientation Orientation::from_direction(glm::vec3 dir) {
    // Should be in the same order as orientations
    const glm::vec3 directions[6] = {glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0),  glm::vec3(0, -1, 0),
                                     glm::vec3(0, 1, 0),  glm::vec3(0, 0, -1), glm::vec3(0, 0, 1)};

    dir = glm::normalize(dir);
    float best_dot = 1;
    int best_index = -1;
    for (int i = 0; i < 6; i++) {
        const glm::vec3 orientation_dir = directions[i];
        float d = glm::dot(orientation_dir, dir);
        if (d < best_dot) {
            best_dot = d;
            best_index = i;
        }
    }
    return Orientation::orientations[best_index];
}

std::string Block::to_string() const {
    std::string type_name;
#define Q(x) #x
#define CASE(name)                                                                                                     \
    case BlockType::name: {                                                                                            \
        type_name = Q(name);                                                                                           \
        break;                                                                                                         \
    }
    switch (_block & TypeMask) {
        CASE(Air);
        CASE(Stone);
        CASE(Dirt)
        CASE(Glowstone);
        CASE(ActiveRedstone);
        CASE(InactiveRedstone);
        CASE(ActiveDelayGate);
        CASE(DelayGate);
        CASE(ActiveNotGate);
        CASE(NotGate);
        CASE(ActiveDiodeGate);
        CASE(DiodeGate);
        CASE(ActiveDisplay);
        CASE(Display);
        CASE(ActiveSwitch);
        CASE(Switch);
        CASE(ActiveBluestone);
        CASE(InactiveBluestone);
        CASE(ActiveGreenstone);
        CASE(InactiveGreenstone);
    default:
        assert(false);
        return "";
    }

    return type_name + "(" + get_orientation().to_string() + ")";
}

static void _add_square(int vertex, uint8_t offset, int x, int y, int z, Orientation face,
                        uint8_t *vertex_texture_uv_data) {
    glm::vec3 p0, p1, p2, p3;
    uint8_t offset1 = offset, offset2 = offset, offset3 = offset;
    p0 = glm::vec3(x, y, z);
    uint8_t o = 1;
    if (face.axis() == Axis::Z) {
        p1 = glm::vec3(x + 1, y, z);
        offset1 |= 0b100;
        p2 = glm::vec3(x, y + 1, z);
        offset2 |= 0b010;
        p3 = glm::vec3(x + 1, y + 1, z);
        offset3 |= 0b110;
    } else if (face.axis() == Axis::X) {
        p1 = glm::vec3(x, y + 1, z);
        offset1 |= 0b010;
        p2 = glm::vec3(x, y, z + 1);
        offset2 |= 0b001;
        p3 = glm::vec3(x, y + 1, z + 1);
        offset3 |= 0b011;
        o = 0;
    } else if (face.axis() == Axis::Y) {
        p1 = glm::vec3(x + 1, y, z);
        offset1 |= 0b100;
        p2 = glm::vec3(x, y, z + 1);
        offset2 |= 0b001;
        p3 = glm::vec3(x + 1, y, z + 1);
        offset3 |= 0b101;
    }

    if (face == Orientation::PosY) {
        unsigned int id = 0;

        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset << 2) | 3;
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset1 << 2) | (2 - o);
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset3 << 2) | 0;

        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset3 << 2) | 0;
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset2 << 2) | (1 + o);
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset << 2) | 3;
    } else if (offset == 0 || face.axis() == Axis::Y) {
        unsigned int id = 0;

        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset3 << 2) | 0;
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset1 << 2) | (2 - o);
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset << 2) | 3;

        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset << 2) | 3;
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset2 << 2) | (1 + o);
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset3 << 2) | 0;
    } else {
        unsigned int id = 0;

        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset << 2) | 3;
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset1 << 2) | (2 - o);
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset3 << 2) | 0;

        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset3 << 2) | 0;
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset2 << 2) | (1 + o);
        vertex_texture_uv_data[vertex + id++] = (face << 5) | (offset << 2) | 3;
    }
}

void WorldGeometry::initialize() {
    glGenTextures(1, &buffers.block_ids);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, buffers.block_ids);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    int levels = 0;
    for (int i = WORLD_SIZE; i > 0; i /= 2) {
        levels++;
    }
    glTexStorage3D(GL_TEXTURE_3D, levels, GL_R8UI, WORLD_SIZE, WORLD_SIZE, WORLD_SIZE);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, WORLD_SIZE, WORLD_SIZE, WORLD_SIZE, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                    block_map.get_buffer());

    {
        _add_square(0, 0b000, 0, 0, 0, Orientation::PosX, vertex_texture_uv_data);
        _add_square(6, 0b000, 0, 0, 0, Orientation::PosY, vertex_texture_uv_data);
        _add_square(12, 0b000, 0, 0, 0, Orientation::PosZ, vertex_texture_uv_data);
        _add_square(18, 0b100, 1, 0, 0, Orientation::NegX, vertex_texture_uv_data);
        _add_square(24, 0b010, 0, 1, 0, Orientation::NegY, vertex_texture_uv_data);
        _add_square(30, 0b001, 0, 0, 1, Orientation::NegZ, vertex_texture_uv_data);

        glGenBuffers(1, &buffers.vertex_texture_uv);
        glBindBuffer(GL_ARRAY_BUFFER, buffers.vertex_texture_uv);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * VERTICES_PER_BLOCK, vertex_texture_uv_data, GL_DYNAMIC_DRAW);
    }

    glGenBuffers(1, &buffers.block_positions);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.block_positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t) * BLOCKS, block_positions, GL_DYNAMIC_DRAW);
}

void WorldGeometry::sync_mipmapped_blockmap() {
    ZoneScoped;

    assert(WORLD_SIZE % 2 == 0);
    int N = WORLD_SIZE / 2;

    std::vector<uint8_t> prev_level;
    prev_level.reserve(N * N * N);

    for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
            for (int z = 0; z < N; ++z) {
                bool occupied = false;
                for (int zz = 0; zz <= 1; ++zz) {
                    for (int yy = 0; yy <= 1; ++yy) {
                        for (int xx = 0; xx <= 1; ++xx) {
                            bool is_block = !block_map.at(x * 2 + xx, y * 2 + yy, z * 2 + zz).is(Block::BlockType::Air);
                            occupied = occupied || is_block;
                        }
                    }
                }
                prev_level.push_back(occupied ? 8 : 0);
            }
        }
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, buffers.block_ids);
    glTexSubImage3D(GL_TEXTURE_3D, 1, 0, 0, 0, N, N, N, GL_RED_INTEGER, GL_UNSIGNED_BYTE, prev_level.data());

    int M = N / 2;
    for (int level = 2; M > 0; level++) {
        std::vector<uint8_t> current_level;
        current_level.reserve(M * M * M);

        for (int x = 0; x < M; ++x) {
            for (int y = 0; y < M; ++y) {
                for (int z = 0; z < M; ++z) {
                    bool occupied = false;
                    for (int l = 0; l < 2; ++l) {
                        for (int k = 0; k < 2; ++k) {
                            for (int j = 0; j < 2; ++j) {
                                int zz = z * 2 + l;
                                int yy = y * 2 + k;
                                int xx = x * 2 + j;

                                int pM = M * 2;
                                bool is_block = prev_level[xx * pM * pM + yy * pM + zz] != 0;
                                occupied = occupied || is_block;
                            }
                        }
                    }
                    current_level.push_back(occupied ? 8 : 0);
                }
            }
        }
        glTexSubImage3D(GL_TEXTURE_3D, level, 0, 0, 0, M, M, M, GL_RED_INTEGER, GL_UNSIGNED_BYTE, current_level.data());
        M /= 2;
        prev_level = current_level;
    }

    // for (int x = 0; x < WORLD_SIZE; x++) {
    //     for (int y = 0; y < WORLD_SIZE; y++) {
    //         for (int z = 0; z < WORLD_SIZE; z++) {
    //             uint8_t mask = level0[(z / 2) * N * N + (y / 2) * N + (x / 2)];
    //             int xx = (x & 1) ^ 1;
    //             int yy = (y & 1) ^ 1;
    //             int zz = (z & 1) ^ 1;
    //             uint8_t off = (xx << 2) | (yy << 1) | zz;
    //             bool res = !block_map.at(x, y, z).is(Block::BlockType::Air);
    //             bool res2 = ((mask >> off) & 1) == 1;
    //             printf("(%d %d %d) (%d %d %d) -> %d | %u | bm %d us %d\n", x, y, z, xx, yy, zz, off, mask, res,
    //             res2); assert(res == res2);
    //         }
    //     }
    // }
}

void WorldGeometry::sync_buffers() {
    ZoneScoped;

    if (!blocks_dirty) {
        return;
    }
    blocks_dirty = false;

    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers.block_positions);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(uint32_t) * num_blocks, (void *)block_positions);
    }

    {
        TracyGpuZone("copy world texture");
        glBindTexture(GL_TEXTURE_3D, buffers.block_ids);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, WORLD_SIZE, WORLD_SIZE, WORLD_SIZE, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                        block_map.get_buffer());
    }

    if (geometry_dirty) {
        sync_mipmapped_blockmap();
    }
    geometry_dirty = false;
}

Block WorldGeometry::get_block(int x, int y, int z) { return block_map(x, y, z); }

void WorldGeometry::set_block(int x, int y, int z, Block block) {
    if (block.is(Block::BlockType::Air)) {
        delete_block(x, y, z);
        return;
    }

    int block_id = block_coordinates_to_id(x, y, z);
    if (block_id == -1) {
        const Vec3 v(x, y, z);

        block_coordinates_to_id(x, y, z) = num_blocks;
        block_coordinate_order[num_blocks] = v;

        block_positions[num_blocks] = v.encode();

        num_blocks++;
    }

    _update_block_map(x, y, z, block);
}

void WorldGeometry::delete_block(int x, int y, int z) {
    int block_id = block_coordinates_to_id(x, y, z);
    if (block_id != -1) {
        num_blocks--;

        _update_block_map(x, y, z, Block::Air);

        block_coordinates_to_id(x, y, z) = -1;

        if (block_id != num_blocks) {
            const size_t last_block = num_blocks;

            const Vec3 &c = block_coordinate_order[last_block];
            block_coordinates_to_id(c) = block_id;
            block_coordinate_order[block_id] = c;

            block_positions[block_id] = c.encode();
        }
    }
}

void WorldGeometry::set_active(int x, int y, int z, bool active) {
    Block block = get_block(x, y, z);

    if (block.is_active() == active) {
        return;
    }

    if (block.is_redstone()) {
        block.set_type(active ? Block::ActiveRedstone : Block::InactiveRedstone);
    } else if (block.is_bluestone()) {
        block.set_type(active ? Block::ActiveBluestone : Block::InactiveBluestone);
    } else if (block.is_greenstone()) {
        block.set_type(active ? Block::ActiveGreenstone : Block::InactiveGreenstone);
    } else if (block.is_not_gate()) {
        block.set_type(active ? Block::ActiveNotGate : Block::NotGate);
    } else if (block.is_diode_gate()) {
        block.set_type(active ? Block::ActiveDiodeGate : Block::DiodeGate);
    } else if (block.is_delay_gate()) {
        block.set_type(active ? Block::ActiveDelayGate : Block::DelayGate);
    } else if (block.is_display()) {
        block.set_type(active ? Block::ActiveDisplay : Block::Display);
    } else if (block.is_switch()) {
        block.set_type(active ? Block::ActiveSwitch : Block::Switch);
    } else {
        assert(false);
    }

    _update_block_map(x, y, z, block);
}

void WorldGeometry::rotate_block(int x, int y, int z) {
    Block block = get_block(x, y, z);
    block.rotate();
    set_block(x, y, z, block);
}

void WorldGeometry::randomize() {
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                int r = rand() % 10;
                Block block;
                if (r == 0) {
                    block = Block::Stone;
                } else if (r == 1) {
                    block = Block::Dirt;
                } else if (r == 2) {
                    block = Block::NotGate;
                } else {
                    block = Block::Air;
                }

                block.set_orientation(Orientation::from(rand() % 6));

                set_block(x, y, z, block);
            }
        }
    }
}

void WorldGeometry::flatworld() {
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                if (y < 3) {
                    set_block(x, y, z, Block::Stone);
                } else {
                    set_block(x, y, z, Block::Air);
                }
            }
        }
    }
}

void WorldGeometry::benchmark_world() {
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                bool x_edge = x == 0 || x == WORLD_SIZE - 1;
                bool y_edge = y == 0 || y == WORLD_SIZE - 1;
                bool z_edge = z == 0 || z == WORLD_SIZE - 1;
                if (x_edge || y_edge || z_edge) {
                    Block block = x % 5 == 1 || y % 5 == 1 || z % 5 == 1 ? Block::Glowstone : Block::Stone;
                    set_block(x, y, z, block);
                } else {
                    set_block(x, y, z, Block::Air);
                }
            }
        }
    }
}

void World::reset() {
    block_map.clear(Block::Air);
    _derive_geometry_from_block_map();
}

bool World::load(const char *filepath) {
    int fd = open(filepath, O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "Error opening world file to load: %s\n", filepath);
        return false;
    }

    const size_t size = BLOCKS * sizeof(Block);

    if (read(fd, block_map.get_buffer(), size) == -1) {
        close(fd);
        perror("Error loading world file");
        return false;
    }

    fprintf(stderr, "Loaded world from: %s\n", filepath);

    close(fd);

    _derive_geometry_from_block_map();
    redstone_dirty = true;
    geometry_dirty = true;

    return true;
}

bool World::save(const char *filepath) {
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

    if (fd == -1) {
        fprintf(stderr, "Error opening world file to save: %s\n", filepath);
        perror("Error opening file for saving");
        return false;
    }

    const size_t size = BLOCKS * sizeof(Block);

    if (write(fd, block_map.get_buffer(), size) == -1) {
        close(fd);
        perror("Error saving to file");
        return false;
    }

    fprintf(stderr, "Saved world to: %s\n", filepath);

    close(fd);

    return true;
}

bool in_bounds(glm::vec3 pos) {
    return 0 <= pos.x && pos.x < WORLD_SIZE && 0 <= pos.y && pos.y < WORLD_SIZE && 0 <= pos.z && pos.z < WORLD_SIZE;
}

void World::copy(std::string cmd) {
    int sx, sy, sz, ex, ey, ez, dx, dy, dz;
    char unused;
    sscanf(cmd.c_str(), "%c %d %d %d %d %d %d %d %d %d\n", &unused, &sx, &sy, &sz, &ex, &ey, &ez, &dx, &dy, &dz);

    for (int x = sx; x <= ex; x++) {
        for (int y = sy; y <= ey; y++) {
            for (int z = sz; z <= ez; z++) {
                int i = x - sx;
                int j = y - sy;
                int k = z - sz;
                set_block(dx + i, dy + j, dz + k, get_block(x, y, z));
            }
        }
    }
}

void World::handle_player_action(PlayerMouseModify player_action, Block block, glm::ivec3 pos,
                                 std::optional<glm::ivec3> prev_pos) {
    if (player_action == PlayerMouseModify::PlaceBlock && prev_pos.has_value()) {
        set_block(prev_pos->x, prev_pos->y, prev_pos->z, block);
    } else if (player_action == PlayerMouseModify::BreakBlock) {
        set_block(pos.x, pos.y, pos.z, Block::Air);
    } else if (player_action == PlayerMouseModify::RotateBlock) {
        const Block &selected_block = get_block(pos.x, pos.y, pos.z);
        if (selected_block.is_switch()) {
            set_active(pos.x, pos.y, pos.z, !selected_block.is_active());
        } else {
            rotate_block(pos.x, pos.y, pos.z);
        }
    } else if (player_action == PlayerMouseModify::Identify) {
        Block block = get_block(pos.x, pos.y, pos.z);
        std::cout << "(" << pos.x << ", " << pos.y << ", " << pos.z << "): " << block << std::endl;
    }
}

int sgn(float x) {
    if (x < 0)
        return -1;
    if (x > 0)
        return 1;
    return 0;
}

float clamp(float x, float min, float max) {
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

void World::player_click_normal(Ray ray, Block block, PlayerMouseModify player_action) {
    if (!in_bounds(ray.pos)) {
        BBox bbox(glm::vec3(0, 0, 0), glm::vec3(WORLD_SIZE, WORLD_SIZE, WORLD_SIZE));

        float t = bbox.hit(ray);
        if (t < 0) {
            return;
        }

        ray.pos += ray.dir * t;
    }

    int x = clamp(ray.pos.x, 0, WORLD_SIZE - 1);
    int y = clamp(ray.pos.y, 0, WORLD_SIZE - 1);
    int z = clamp(ray.pos.z, 0, WORLD_SIZE - 1);

    int step_x = sgn(ray.dir.x);
    int step_y = sgn(ray.dir.y);
    int step_z = sgn(ray.dir.z);

    // assert(0 <= x && x < WORLD_SIZE);
    // assert(0 <= y && y < WORLD_SIZE);
    // assert(0 <= z && z < WORLD_SIZE);

    // assert(x <= ray.pos.x && ray.pos.x <= x + step_x);
    // assert(y <= ray.pos.y && ray.pos.y <= y + step_y);
    // assert(z <= ray.pos.z && ray.pos.z <= z + step_z);

    int next_x = step_x == 1 ? 1 : 0;
    int next_y = step_y == 1 ? 1 : 0;
    int next_z = step_z == 1 ? 1 : 0;

    float t_max_x = (x + next_x - ray.pos.x) / ray.dir.x;
    float t_max_y = (y + next_y - ray.pos.y) / ray.dir.y;
    float t_max_z = (z + next_z - ray.pos.z) / ray.dir.z;

    float t_delta_x = fabs(1.f / ray.dir.x);
    float t_delta_y = fabs(1.f / ray.dir.y);
    float t_delta_z = fabs(1.f / ray.dir.z);

    int just_out_x = step_x == 1 ? WORLD_SIZE : -1;
    int just_out_y = step_y == 1 ? WORLD_SIZE : -1;
    int just_out_z = step_z == 1 ? WORLD_SIZE : -1;

    const Block &selected_block = get_block(x, y, z);
    if (!selected_block.is(Block::Air)) {
        handle_player_action(player_action, block, glm::ivec3(x, y, z), std::nullopt);
        return;
    }

    while (true) {
        glm::ivec3 prev_pos = glm::ivec3(x, y, z);

        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                x += step_x;
                if (x == just_out_x) {
                    return;
                }
                t_max_x += t_delta_x;
            } else {
                z += step_z;
                if (z == just_out_z) {
                    return;
                }
                t_max_z += t_delta_z;
            }
        } else {
            if (t_max_y < t_max_z) {
                y += step_y;
                if (y == just_out_y) {
                    return;
                }
                t_max_y += t_delta_y;

            } else {
                z += step_z;
                if (z == just_out_z) {
                    return;
                }
                t_max_z += t_delta_z;
            }
        }

        const Block &selected_block = get_block(x, y, z);
        if (!selected_block.is(Block::Air)) {
            handle_player_action(player_action, block, glm::ivec3(x, y, z), prev_pos);
            return;
        }
    }
}

void World::player_click_mipmapped(Ray ray, Block block, PlayerMouseModify player_action) {
    if (!in_bounds(ray.pos)) {
        BBox bbox(glm::vec3(0, 0, 0), glm::vec3(WORLD_SIZE, WORLD_SIZE, WORLD_SIZE));

        float t = bbox.hit(ray);
        if (t < 0) {
            return;
        }

        ray.pos += ray.dir * t;
    }

    glm::ivec3 pos = glm::clamp(ray.pos, 0.f, WORLD_SIZE - 1.f);

    glm::ivec3 istep = glm::sign(ray.dir);

    glm::ivec3 next = glm::clamp(istep, 0, 1);

    glm::vec3 t_max = (glm::vec3(pos) + glm::vec3(next) - ray.pos) / ray.dir;

    glm::vec3 t_delta = glm::abs(glm::vec3(1.f) / ray.dir);

    glm::ivec3 just_out = next * glm::ivec3(WORLD_SIZE + 1) - glm::ivec3(1);

    int level = 0;
    int count = 0;
    int scale = 1;

    const Block &selected_block = get_block(pos.x, pos.y, pos.z);
    if (!selected_block.is(Block::Air)) {
        handle_player_action(player_action, block, pos, std::nullopt);
        return;
    }

    while (true) {
        glm::ivec3 prev_pos = pos;

        float t_min = std::min(t_max.x, std::min(t_max.y, t_max.z));
        glm::ivec3 mask = glm::ivec3(glm::equal(t_max, glm::vec3(t_min)));

        pos += istep * mask;
        t_max += t_delta * glm::vec3(mask) * (float)scale;

        if (glm::any(glm::equal(pos, just_out))) {
            return;
        }

        uint8_t block = get_block_mipmapped(pos.x, pos.y, pos.z, level);
        if (block) {
            if (level == 0) {
                handle_player_action(player_action, get_block(pos.x, pos.y, pos.z), pos, prev_pos);
                return;
            } else {
                // Go down a level
                level--;
                scale /= 2;
                pos = (ray.pos + t_min * ray.dir) / glm::vec3(scale);
                t_max = (glm::vec3(pos * scale) + glm::vec3(next * scale) - ray.pos) / ray.dir;
                just_out = next * glm::ivec3((WORLD_SIZE / scale) + 1) - glm::ivec3(1);
                count = 0;
            }
        }

        count++;
        if (count > 1 && level < 6) {
            if (!get_block_mipmapped(pos.x / 2, pos.y / 2, pos.z / 2, level + 1)) {
                // Go up a level
                level++;
                scale *= 2;
                pos /= 2;
                t_max = (glm::vec3(pos * scale) + glm::vec3(next * scale) - ray.pos) / ray.dir;
                just_out = next * glm::ivec3((WORLD_SIZE / scale) + 1) - glm::ivec3(1);
            }
            count = 0;
        }
    }
}

void World::player_click(Ray ray, Block block, PlayerMouseModify player_action) {
    player_click_normal(ray, block, player_action);
    // player_click_mipmapped(ray, block, player_action);
    // printf("\n");
}
