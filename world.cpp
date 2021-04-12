#include "world.h"

void WorldGeometry::initialize() {
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                block_coordinates_to_id[x][y][z] = -1;
            }
        }
    }

    randomize<false>();

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
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, WORLD_SIZE, WORLD_SIZE, WORLD_SIZE, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, world_buffer_data);
}

void WorldGeometry::sync_buffers(int start, int end) {
#define _SYNC_BUFFERS_COPY(buffer, size, ptr) \
        glBindBuffer(GL_ARRAY_BUFFER, buffer);\
        glBufferSubData(GL_ARRAY_BUFFER, (start), ((end) - (start)) * (size), &ptr[start])
    _SYNC_BUFFERS_COPY(buffers.block_ids, sizeof(uint8_t), block_face_data);
    _SYNC_BUFFERS_COPY(buffers.vertices, sizeof(glm::vec3), vertex_data);
    _SYNC_BUFFERS_COPY(buffers.vertex_texture_uv, sizeof(uint8_t), vertex_texture_uv_data);

    {
        glBindTexture(GL_TEXTURE_3D, buffers.world_texture);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, WORLD_SIZE, WORLD_SIZE, WORLD_SIZE, GL_RED_INTEGER, GL_UNSIGNED_BYTE, world_buffer_data);
    }
}

template <bool sync>
void WorldGeometry::set_block(int x, int y, int z, Block block) {
    if (block == Block::Air) {
        delete_block<sync>(x, y, z);
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

    _add_square(block, vertex, x, y, z, Axis::X);
    _add_square(block, vertex, x, y, z, Axis::Y);
    _add_square(block, vertex, x, y, z, Axis::Z);
    _add_square(block, vertex, x + 1, y, z, Axis::X);
    _add_square(block, vertex, x, y + 1, z, Axis::Y);
    _add_square(block, vertex, x, y, z + 1, Axis::Z);

    if (new_block) {
        num_vertices = vertex;
    }

    world_buffer_data[ZYX_MAJOR(x, y, z)] = (uint8_t)block;

    if (sync) {
        sync_buffers(original_vertex, vertex);
    }
}

template <bool sync>
void WorldGeometry::delete_block(int x, int y, int z) {
    int block_id = block_coordinates_to_id[x][y][z];
    if (block_id != -1) {
        int vertex = block_id * VERTICES_PER_BLOCK;
        num_vertices -= VERTICES_PER_BLOCK;

        world_buffer_data[ZYX_MAJOR(x, y, z)] = (uint8_t)Block::Air;

        int remainder = num_vertices - vertex;

#define _DELETE_BLOCK_MOVE(p, s) memmove(&p[vertex], &p[vertex + VERTICES_PER_BLOCK], remainder * s)
        _DELETE_BLOCK_MOVE(block_face_data, sizeof(uint8_t));
        _DELETE_BLOCK_MOVE(vertex_data, sizeof(glm::vec3));
        _DELETE_BLOCK_MOVE(vertex_texture_uv_data, sizeof(uint8_t));

        {
            for (int i = block_id + 1; i <= num_vertices / VERTICES_PER_BLOCK; i++) {
                const Vec3 &c = block_coordinate_order[i];
                block_coordinate_order[i - 1] = c;
                block_coordinates_to_id[c.x][c.y][c.z] -= 1;
            }
            block_coordinates_to_id[x][y][z] = -1;
        }

        if (sync) {
            sync_buffers(vertex, num_vertices);
        }
    }
}

void WorldGeometry::_add_square(Block block_face_type, int &vertex, int x, int y, int z, Axis norm) {
    glm::vec3 p0, p1, p2, p3;
    p0 = glm::vec3(x, y, z);
    uint8_t o = 1;
    if (norm == Axis::Z) {
        p1 = glm::vec3(x + 1, y, z);
        p2 = glm::vec3(x, y + 1, z);
        p3 = glm::vec3(x + 1, y + 1, z);
    } else if (norm == Axis::X) {
        p1 = glm::vec3(x, y + 1, z);
        p2 = glm::vec3(x, y, z + 1);
        p3 = glm::vec3(x, y + 1, z + 1);
        o = 0;
    } else if (norm == Axis::Y) {
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
        block_face_data[vertex + i] = (uint8_t)block_face_type;
    }

    vertex += 6;
}
