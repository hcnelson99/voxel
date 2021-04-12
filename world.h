#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

// world is WORLD_SIZE x WORLD_SIZE x WORLD_SIZE
#define WORLD_SIZE (16)

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

struct Vec3 {
    int x;
    int y;
    int z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(int x, int y, int z) : x(x), y(y), z(z) {}
};

// Stores data for vertices that is passed into shader
class WorldGeometry {
  public:
    struct OpenGLBuffers {
        GLuint block_ids;
        GLuint vertices;
        GLuint vertex_texture_uv;
        GLuint world_texture;
    };

    uint8_t world_buffer_data[BLOCKS] = {};

    // the block type for each vertex
    uint8_t block_face_data[VERTICES];

    // the position of each vertex
    glm::vec3 vertex_data[VERTICES];

    // the texture position of each vertex
    uint8_t vertex_texture_uv_data[VERTICES];

    unsigned int num_vertices = 0;

    // this is used instead of constructor because GL functions need to be
    // initialized by GLEW first
    void initialize();

    OpenGLBuffers &get_buffers() { return buffers; }

    void set_block(int x, int y, int z, Block block);
    void delete_block(int x, int y, int z);

    void sync_buffers();

    void randomize() {
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

                    set_block(x, y, z, face_type);
                }
            }
        }
    }

  private:
    OpenGLBuffers buffers;

    // maps logical block coordinate to index into list of vertices
    int block_coordinates_to_id[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE] = {};
    Vec3 block_coordinate_order[BLOCKS];

    void _add_square(Block block_face_type, int &vertex, int x, int y, int z, Axis norm);
};

