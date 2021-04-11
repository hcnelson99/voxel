#pragma once

#include <vector>
#include <glm/glm.hpp>

// world is WORLD_SIZE x WORLD_SIZE x WORLD_SIZE
#define WORLD_SIZE 16

#define VERTICES_PER_BLOCK (6 * 3 * 2) // 6 faces, 2 triangles per face
#define BLOCKS (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE)
#define VERTICES (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE * VERTICES_PER_BLOCK)

#define ZYX_MAJOR(x, y, z) ((z) * WORLD_SIZE * WORLD_SIZE + (y) * WORLD_SIZE + (x))

enum class Axis { X, Y, Z };

enum class Block : uint8_t {
    Air = 0,
    Stone = 1,
    Dirt = 2,
    Wood = 4,
};

// Stores data for vertices that is passed into shader
struct WorldGeometry {
  private:
    // maps logical block coordinate to index into list of vertices
    int block_coordinates[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE] = {};

  public:
    uint8_t world_buffer_data[BLOCKS] = {};

    // the block type for each vertex
    uint8_t block_face_data[VERTICES];

    // the position of each vertex
    glm::vec3 vertex_data[VERTICES];

    // the texture position of each vertex
    uint8_t vertex_texture_uv_data[VERTICES];

    unsigned int num_vertices = 0;

    WorldGeometry() {
        for (int x = 0; x < WORLD_SIZE; ++x) {
            for (int y = 0; y < WORLD_SIZE; ++y) {
                for (int z = 0; z < WORLD_SIZE; ++z) {
                    block_coordinates[x][y][z] = -1;
                }
            }
        }

        for (int x = 0; x < WORLD_SIZE; ++x) {
            for (int y = 0; y < WORLD_SIZE; ++y) {
                for (int z = 0; z < WORLD_SIZE; ++z) {
                    int r = rand() % 10;
                    Block face_type;
                    if (r == 0) {
                        face_type = Block::Stone;
                    } else if (r == 1) {
                        face_type = Block::Dirt;
                    } else if (r == 2) {
                        face_type = Block::Wood;
                    } else {
                        face_type = Block::Air;
                    }

                    if (face_type != Block::Air) {
                        set_block(x, y, z, face_type);
                    }
                }
            }
        }
    }

  private:
    void set_block(int x, int y, int z, Block block) {
        if (block == Block::Air) {
            delete_block(x, y, z);
            return;
        }

        int vertex = block_coordinates[x][y][z];
        bool new_block = false;
        if (vertex == -1) {
            new_block = true;
            vertex = num_vertices;
        }

        add_square(block, vertex, x, y, z, Axis::X);
        add_square(block, vertex, x, y, z, Axis::Y);
        add_square(block, vertex, x, y, z, Axis::Z);
        add_square(block, vertex, x + 1, y, z, Axis::X);
        add_square(block, vertex, x, y + 1, z, Axis::Y);
        add_square(block, vertex, x, y, z + 1, Axis::Z);

        if (new_block) {
            num_vertices = vertex;
        }

        world_buffer_data[ZYX_MAJOR(x, y, z)] = (uint8_t)block;
    }

    void delete_block(int x, int y, int z) {
        int vertex = block_coordinates[x][y][z];
        if (vertex != -1) {
            num_vertices -= VERTICES_PER_BLOCK;

            world_buffer_data[ZYX_MAJOR(x, y, z)] =  (uint8_t)Block::Air;

            int remainder = num_vertices - vertex - VERTICES_PER_BLOCK;

            memmove(&block_face_data[vertex], &block_face_data[vertex + VERTICES_PER_BLOCK], remainder);
            memmove(&vertex_data[vertex], &vertex_data[vertex + VERTICES_PER_BLOCK], remainder);
            memmove(&vertex_texture_uv_data[vertex], &vertex_texture_uv_data[vertex + VERTICES_PER_BLOCK], remainder);
        }
    }

    void add_square(Block block_face_type, int &vertex, int x, int y, int z, Axis norm) {
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
};

