#include "world.h"

#include <algorithm>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const Orientation Orientation::NegX(0);
const Orientation Orientation::PosX(1);
const Orientation Orientation::NegY(2);
const Orientation Orientation::PosY(3);
const Orientation Orientation::NegZ(4);
const Orientation Orientation::PosZ(5);

uint8_t Orientation::plane_orientation(const Orientation &o) const {
    // TODO: change to bitmask to reduce branching
    if (_orientation == PosX._orientation) {
        return 2;
    } else if (_orientation == NegX._orientation) {
        return 0;
    } else if (_orientation == PosY._orientation) {
        return 3;
    } else if (_orientation == NegY._orientation) {
        return 1;
    } else if (_orientation == PosZ._orientation && o.axis() == Axis::X) {
        return 2;
    } else if (_orientation == PosZ._orientation && o == PosY) {
        return 1;
    } else if (_orientation == PosZ._orientation && o == NegY) {
        return 3;
    } else if (_orientation == NegZ._orientation && o.axis() == Axis::X) {
        return 0;
    } else if (_orientation == NegZ._orientation && o == PosY) {
        return 3;
    } else if (_orientation == NegZ._orientation && o == NegY) {
        return 1;
    } else {
        assert(false);
    }
}

uint8_t Block::texture_id(const Orientation orientation) const {
    Orientation bor = get_orientation();
    // each block has 6 tiles in terrain.png that represent rotations
    if (bor == orientation) {
        return (_block >> 3) * 6 + 1;
    } else if (bor.axis() == orientation.axis()) {
        return (_block >> 3) * 6 + 0;
    } else {
        return (_block >> 3) * 6 + 2 + bor.plane_orientation(orientation);
    }
}

void WorldGeometry::initialize() {
    std::fill((int *)block_coordinates_to_id, (int *)block_coordinates_to_id + BLOCKS, -1);

    glGenBuffers(1, &buffers.block_ids);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.block_ids);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * VERTICES, block_face_data, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &buffers.vertices);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * VERTICES, vertex_data, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &buffers.vertex_texture_uv);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.vertex_texture_uv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * VERTICES, vertex_texture_uv_data, GL_DYNAMIC_DRAW);

    glGenTextures(1, &buffers.world_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, buffers.world_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, WORLD_SIZE, WORLD_SIZE, WORLD_SIZE, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                 world_buffer_data);
}

void WorldGeometry::sync_buffers() {
    const int start = 0;
    const int end = num_vertices;

    const auto copy = [&](GLuint buffer, size_t size, auto *ptr) {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferSubData(GL_ARRAY_BUFFER, (start), ((end) - (start)) * (size), &ptr[start]);
    };
    copy(buffers.block_ids, sizeof(uint8_t), block_face_data);
    copy(buffers.vertices, sizeof(glm::vec3), vertex_data);
    copy(buffers.vertex_texture_uv, sizeof(uint8_t), vertex_texture_uv_data);

    {
        glBindTexture(GL_TEXTURE_3D, buffers.world_texture);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, WORLD_SIZE, WORLD_SIZE, WORLD_SIZE, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                        world_buffer_data);
    }
}

Block WorldGeometry::get_block(int x, int y, int z) { return block_map[x][y][z]; }

void WorldGeometry::set_block(int x, int y, int z, Block block) {
    if (block.is(Block::BlockType::Air)) {
        delete_block(x, y, z);
        return;
    }

    int block_id = block_coordinates_to_id[x][y][z];
    int vertex;
    bool new_block = false;
    if (block_id == -1) {
        new_block = true;
        vertex = num_vertices;
        block_coordinates_to_id[x][y][z] = num_vertices / VERTICES_PER_BLOCK;
        block_coordinate_order[vertex / VERTICES_PER_BLOCK] = Vec3(x, y, z);
    } else {
        vertex = block_id * VERTICES_PER_BLOCK;
    }
    const int original_vertex = vertex;

    _add_square(block, vertex, x, y, z, Orientation::PosX);
    _add_square(block, vertex, x, y, z, Orientation::PosY);
    _add_square(block, vertex, x, y, z, Orientation::PosZ);
    _add_square(block, vertex, x + 1, y, z, Orientation::NegX);
    _add_square(block, vertex, x, y + 1, z, Orientation::NegY);
    _add_square(block, vertex, x, y, z + 1, Orientation::NegZ);

    if (new_block) {
        num_vertices = vertex;
    }

    block_map[x][y][z] = block;
    world_buffer_data[zyx_major(x, y, z)] = block;
}

void WorldGeometry::delete_block(int x, int y, int z) {
    int block_id = block_coordinates_to_id[x][y][z];
    if (block_id != -1) {
        int vertex = block_id * VERTICES_PER_BLOCK;
        num_vertices -= VERTICES_PER_BLOCK;

        block_map[x][y][z] = Block::Air;
        world_buffer_data[zyx_major(x, y, z)] = Block::Air;

        block_coordinates_to_id[x][y][z] = -1;

        if (vertex != num_vertices) {
            const auto swap = [&](auto *ptr, size_t size) {
                memcpy(&ptr[vertex], &ptr[num_vertices], size * VERTICES_PER_BLOCK);
            };
            swap(block_face_data, sizeof(uint8_t));
            swap(vertex_data, sizeof(glm::vec3));
            swap(vertex_texture_uv_data, sizeof(uint8_t));

            const Vec3 &c = block_coordinate_order[num_vertices / VERTICES_PER_BLOCK];
            block_coordinates_to_id[c.x][c.y][c.z] = block_id;
            block_coordinate_order[block_id] = Vec3(c.x, c.y, c.z);
        }
    }
}

void WorldGeometry::rotate_block(int x, int y, int z) {
    Block block = get_block(x, y, z);
    block.rotate();
    set_block(x, y, z, block);
}

void WorldGeometry::_add_square(Block block, int &vertex, int x, int y, int z, Orientation face) {
    glm::vec3 p0, p1, p2, p3;
    p0 = glm::vec3(x, y, z);
    uint8_t o = 1;
    if (face.axis() == Axis::Z) {
        p1 = glm::vec3(x + 1, y, z);
        p2 = glm::vec3(x, y + 1, z);
        p3 = glm::vec3(x + 1, y + 1, z);
    } else if (face.axis() == Axis::X) {
        p1 = glm::vec3(x, y + 1, z);
        p2 = glm::vec3(x, y, z + 1);
        p3 = glm::vec3(x, y + 1, z + 1);
        o = 0;
    } else if (face.axis() == Axis::Y) {
        p1 = glm::vec3(x + 1, y, z);
        p2 = glm::vec3(x, y, z + 1);
        p3 = glm::vec3(x + 1, y, z + 1);
    }

    {
        unsigned int id = 0;

        vertex_data[vertex + id++] = p0;
        vertex_data[vertex + id++] = p1;
        vertex_data[vertex + id++] = p3;

        vertex_data[vertex + id++] = p0;
        vertex_data[vertex + id++] = p2;
        vertex_data[vertex + id++] = p3;
    }

    {
        unsigned int id = 0;

        vertex_texture_uv_data[vertex + id++] = 3;
        vertex_texture_uv_data[vertex + id++] = 2 - o;
        vertex_texture_uv_data[vertex + id++] = 0;

        vertex_texture_uv_data[vertex + id++] = 3;
        vertex_texture_uv_data[vertex + id++] = 1 + o;
        vertex_texture_uv_data[vertex + id++] = 0;
    }

    for (unsigned int i = 0; i < 6; i++) {
        block_face_data[vertex + i] = block.texture_id(face);
    }

    vertex += 6;
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

void WorldGeometry::wireframe() {
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                int a = x == 0 || x == WORLD_SIZE - 1;
                int b = y == 0 || y == WORLD_SIZE - 1;
                int c = z == 0 || z == WORLD_SIZE - 1;
                if (a + b + c >= 2) {
                    set_block(x, y, z, Block::Stone);
                } else {
                    set_block(x, y, z, Block::Air);
                }
            }
        }
    }
}

void World::reset() {
    std::fill((int *)block_map, (int *)block_map + BLOCKS, 0);
    _derive_geometry_from_block_map();
}

bool World::load(const char *filepath) {
    int fd = open(filepath, O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "Error opening world file to load: %s\n", filepath);
        return false;
    }

    const size_t size = BLOCKS * sizeof(Block);

    if (read(fd, block_map, size) == -1) {
        close(fd);
        perror("Error loading world file");
        return false;
    }

    fprintf(stderr, "Loaded world from: %s\n", filepath);

    close(fd);

    _derive_geometry_from_block_map();
    redstone_dirty = true;

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

    if (write(fd, block_map, size) == -1) {
        close(fd);
        perror("Error saving to file");
        return false;
    }

    fprintf(stderr, "Saved world to: %s\n", filepath);

    close(fd);

    return true;
}

int sgn(float x) {
    if (x < 0)
        return -1;
    if (x > 0)
        return 1;
    return 0;
}

bool in_bounds(glm::vec3 pos) {
    return 0 <= pos.x && pos.x < WORLD_SIZE && 0 <= pos.y && pos.y < WORLD_SIZE && 0 <= pos.z && pos.z < WORLD_SIZE;
}

float clamp(float x, float min, float max) {
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
};

void World::player_click(Ray ray, Block block, PlayerMouseModify player_action) {
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

    if (!get_block(x, y, z).is(Block::Air)) {
        if (player_action == PlayerMouseModify::BreakBlock) {
            set_block(x, y, z, Block::Air);
        } else if (player_action == PlayerMouseModify::RotateBlock) {
            rotate_block(x, y, z);
        }
    }

    while (true) {
        int prev_x = x, prev_y = y, prev_z = z;

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
        if (!get_block(x, y, z).is(Block::Air)) {
            if (player_action == PlayerMouseModify::PlaceBlock) {
                set_block(prev_x, prev_y, prev_z, block);
            } else if (player_action == PlayerMouseModify::BreakBlock) {
                set_block(x, y, z, Block::Air);
            } else if (player_action == PlayerMouseModify::RotateBlock) {
                rotate_block(x, y, z);
            }
            return;
        }
    }
}
