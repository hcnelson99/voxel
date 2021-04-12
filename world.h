#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "log.h"
#include "ray.h"

// world is WORLD_SIZE x WORLD_SIZE x WORLD_SIZE
#define WORLD_SIZE (16)

#define VERTICES_PER_BLOCK (6 * 3 * 2) // 6 faces, 2 triangles per face
#define BLOCKS (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE)
#define VERTICES (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE * VERTICES_PER_BLOCK)

inline int zyx_major(int x, int y, int z) { return ((z)*WORLD_SIZE * WORLD_SIZE + (y)*WORLD_SIZE + (x)); }

enum class Axis : uint8_t { X = 0, Y = 1, Z = 2 };

struct Orientation {
    static const Orientation PosX;
    static const Orientation NegX;
    static const Orientation PosY;
    static const Orientation NegY;
    static const Orientation PosZ;
    static const Orientation NegZ;

    operator uint8_t() const { return _orientation; }
    Axis axis() const { return _axis; }
    static Orientation from(uint8_t o) { return Orientation(o); }

    uint8_t plane_orientation(const Orientation &o) const;

  private:
    uint8_t _orientation;
    Axis _axis;
    Orientation(int o) {
        _orientation = o;
        if (o == 0 || o == 1) {
            _axis = Axis::X;
        } else if (o == 2 || o == 3) {
            _axis = Axis::Y;
        } else if (o == 4 || o == 5) {
            _axis = Axis::Z;
        } else {
            _orientation = 0;
            _axis = Axis::X;
        }
    }
};

class Block {
  private:
    uint8_t _block = 0;

    static const uint8_t TypeMask = 0b11111000;
    static const uint8_t OrientationMask = 0b111;
    static const int OrientationWidth = 3;

  public:
    // number of types of blocks before it in terrain.png
    enum BlockType {
        Air = 0 << OrientationWidth,
        Stone = 1 << OrientationWidth,
        Dirt = 2 << OrientationWidth,
        Wood = 3 << OrientationWidth,
        ActiveRedstone = 4 << OrientationWidth,
        InactiveRedstone = 5 << OrientationWidth,
        DelayGate = 6 << OrientationWidth,
        NotGate = 7 << OrientationWidth,
    };

    Block() = default;

    Block(BlockType type, Orientation orientation = Orientation::PosX) { _block = type | orientation; }

    bool is(BlockType type) const { return (_block & TypeMask) == type; }
    Orientation get_orientation() const { return Orientation::from(_block & OrientationMask); }
    operator uint8_t() const { return _block; }

    uint8_t texture_id(const Orientation orientation) const;

    void rotate() { _block = (_block & TypeMask) | Orientation::from((get_orientation() + 1)); }

    void set_orientation(Orientation bor) { _block = (_block & TypeMask) | bor; }
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

    // this is used instead of constructor because GL functions need to be
    // initialized by GLEW first
    void initialize();

    unsigned int get_num_vertices() const { return num_vertices; }
    const OpenGLBuffers &get_buffers() const { return buffers; }

    void sync_buffers();

  protected:
    OpenGLBuffers buffers;

    // buffers that are directly used by OpenGL
    struct {
        Block world_buffer_data[BLOCKS] = {};

        // the block type for each vertex
        uint8_t block_face_data[VERTICES];

        // the position of each vertex
        glm::vec3 vertex_data[VERTICES];

        // the texture position of each vertex
        uint8_t vertex_texture_uv_data[VERTICES];
    };

    // extra fields used to maintain geometry but not used by OpenGL
    struct {
        unsigned int num_vertices = 0;

        // maps logical block coordinate to index into list of vertices
        int block_coordinates_to_id[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE] = {};
        Vec3 block_coordinate_order[BLOCKS];
    };

    void set_block(int x, int y, int z, Block block);
    void delete_block(int x, int y, int z);
    void randomize();
    void wireframe();

    void _add_square(Block block, int &vertex, int x, int y, int z, Orientation face);
};

class RedstoneCircuit {
  public:
    RedstoneCircuit(WorldGeometry *geometry) {}

    void tick() {}
    void rebuild() {}
};

class World : public WorldGeometry {
  public:
    World() : redstone(this) {}

    void initialize() { WorldGeometry::initialize(); }

    void raycast(Ray ray);

    void tick() { redstone.tick(); }

    void reset();
    bool load(const char *filepath);
    bool save(const char *filepath);
    void log_frame() { Log::log_frame_world(num_vertices / VERTICES_PER_BLOCK); }

    void set_block(int x, int y, int z, Block block) { WorldGeometry::set_block(x, y, z, block); }
    void delete_block(int x, int y, int z) { WorldGeometry::delete_block(x, y, z); }
    void randomize() { WorldGeometry::randomize(); }
    void wireframe() { WorldGeometry::wireframe(); }

  private:
    RedstoneCircuit redstone;

    void _derive_geometry_from_world_buffer() {
        num_vertices = 0;
        std::fill((int *)block_coordinates_to_id, (int *)block_coordinates_to_id + BLOCKS, -1);

        for (int x = 0; x < WORLD_SIZE; ++x) {
            for (int y = 0; y < WORLD_SIZE; ++y) {
                for (int z = 0; z < WORLD_SIZE; ++z) {
                    set_block(x, y, z, world_buffer_data[zyx_major(x, y, z)]);
                }
            }
        }
    }
};
