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

        randomly_initialize();
    }

  private:
    void randomly_initialize();
    void set_block(int x, int y, int z, Block block);
    void delete_block(int x, int y, int z);
    void add_square(Block block_face_type, int &vertex, int x, int y, int z, Axis norm);
};

