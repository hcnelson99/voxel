#pragma once

#include <vector>
#include <glm/glm.hpp>

// world is WORLD_SIZE x WORLD_SIZE x WORLD_SIZE
#define WORLD_SIZE 16

#define BLOCKS (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE)
#define VERTICES (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE * 6 * 3 * 2)

enum class Axis { X, Y, Z };

enum class Block : uint8_t {
    Air = 0,
    Stone = 1,
    Dirt = 2,
    Wood = 4,
};

// Stores data for vertices that is passed into shader
struct WorldGeometry {
  public:
    uint8_t world_buffer_data[BLOCKS] = {};

    // the block type for each vertex
    uint8_t block_face_data[VERTICES];

    // the position of each vertex
    glm::vec3 vertex_data[VERTICES];

    // the texture position of each vertex
    uint8_t vertex_texture_uv_data[VERTICES];

    unsigned int num_blocks = 0;
    unsigned int num_vertices = 0;

    WorldGeometry() {
        // Init world
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
                        add_square(face_type, x, y, z, Axis::X);
                        add_square(face_type, x, y, z, Axis::Y);
                        add_square(face_type, x, y, z, Axis::Z);
                        add_square(face_type, x + 1, y, z, Axis::X);
                        add_square(face_type, x, y + 1, z, Axis::Y);
                        add_square(face_type, x, y, z + 1, Axis::Z);

                        world_buffer_data[z * WORLD_SIZE * WORLD_SIZE + y * WORLD_SIZE + x] = (uint8_t)face_type;
                    }
                }
            }
        }
    }

  private:
    void add_square(Block block_face_type, int x, int y, int z, Axis norm) {
        const unsigned int vertex = num_vertices;
        num_vertices += 6;

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
    }
};

