#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

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
class WorldGeometry {
  public:
    struct OpenGLBuffers {
        GLuint vertices;
        GLuint block_ids;
        GLuint vertex_texture_uv;
    };

    uint8_t world_buffer_data[BLOCKS] = {};

    // the block type for each vertex
    uint8_t block_face_data[VERTICES];

    // the position of each vertex
    glm::vec3 vertex_data[VERTICES];

    // the texture position of each vertex
    uint8_t vertex_texture_uv_data[VERTICES];

    unsigned int num_vertices = 0;

  private:
    OpenGLBuffers buffers;

    // maps logical block coordinate to index into list of vertices
    int block_coordinates[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE] = {};

    void randomly_initialize();
    void set_block(int x, int y, int z, Block block);
    void delete_block(int x, int y, int z);
    void add_square(Block block_face_type, int &vertex, int x, int y, int z, Axis norm);

  public:
    // this is used instead of constructor because GL functions need to be
    // initialized by GLEW first
    void initialize();

    OpenGLBuffers &get_buffers() { return buffers; }
};

