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

#define IN_BOUND(x) (0 <= (x) && (x) < WORLD_SIZE)

struct Vec3 {
    int x;
    int y;
    int z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(int x, int y, int z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3 v) const { return Vec3(x + v.x, y + v.y, z + v.z); }

    bool in_world() const { return IN_BOUND(x) && IN_BOUND(y) && IN_BOUND(z); }
    void invalidate() { x = WORLD_SIZE + 1; }
};

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

    Orientation opposite() const { return _opposite_map[_orientation]; }
    Vec3 direction() const { return _direction_map[_orientation]; }

  private:
    static const Axis _axis_map[6];
    static const Orientation _opposite_map[6];
    static const Vec3 _direction_map[6];

    uint8_t _orientation;
    Axis _axis;
    Orientation(int o) {
        _orientation = o % 6;
        _axis = _axis_map[_orientation];
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
        ActiveDelayGate = 6 << OrientationWidth,
        DelayGate = 7 << OrientationWidth,
        ActiveNotGate = 8 << OrientationWidth,
        NotGate = 9 << OrientationWidth,
    };

    Block() = default;

    Block(BlockType type, Orientation orientation = Orientation::PosX) { _block = type | orientation; }

    bool is(BlockType type) const { return (_block & TypeMask) == (type & TypeMask); }

    Orientation get_orientation() const { return Orientation::from(_block & OrientationMask); }
    operator uint8_t() const { return _block; }

    uint8_t texture_id(const Orientation orientation) const;

    void rotate() { _block = (_block & TypeMask) | Orientation::from((get_orientation() + 1)); }

    void set_type(BlockType type) { _block = type | (_block & OrientationMask); }
    void set_orientation(Orientation bor) { _block = (_block & TypeMask) | bor; }

    bool operator==(const Block &other) { return (_block & TypeMask) == (other._block & TypeMask); }
    bool operator!=(const Block &other) { return !(*this == other); }

    bool is_redstone() const {
        return (_block & TypeMask) == BlockType::ActiveRedstone || (_block & TypeMask) == BlockType::InactiveRedstone;
    }

    bool is_not_gate() const {
        return (_block & TypeMask) == BlockType::NotGate || (_block & TypeMask) == BlockType::ActiveNotGate;
    }

    bool is_delay_gate() const {
        return (_block & TypeMask) == BlockType::DelayGate || (_block & TypeMask) == BlockType::ActiveDelayGate;
    }

    bool is_active() const {
        uint8_t type = _block & TypeMask;
        return type == BlockType::ActiveRedstone || type == BlockType::ActiveNotGate ||
               type == BlockType::ActiveDelayGate;
    }
};

// Stores data for vertices that is passed into shader
class WorldGeometry {
    friend class RedstoneCircuit;

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
        uint8_t world_buffer_data[BLOCKS] = {};

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

        Block block_map[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE] = {};

        // maps logical block coordinate to index into list of vertices
        int block_coordinates_to_id[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE] = {};
        Vec3 block_coordinate_order[BLOCKS];
    };

    Block get_block(int x, int y, int z);
    Block get_block(const Vec3 &v) { return get_block(v.x, v.y, v.z); };
    Block get_block_safe(int x, int y, int z) { return get_block_safe(Vec3(x, y, z)); }
    Block get_block_safe(const Vec3 &v) {
        if (v.in_world()) {
            return get_block(v);
        }
        return Block::Air;
    }
    void set_block(int x, int y, int z, Block block);
    void set_block(Vec3 v, Block block) { set_block(v.x, v.y, v.z, block); }
    void delete_block(int x, int y, int z);
    void rotate_block(int x, int y, int z);
    void randomize();
    void wireframe();

    void _add_square(Block block, int &vertex, int x, int y, int z, Orientation face);
};

class RedstoneCircuit {
  public:
    RedstoneCircuit(WorldGeometry *g) : world_geometry(g) { init(); }

    void rebuild();
    void tick();

  private:
    WorldGeometry *world_geometry;

    void init();

    uint8_t signal_map[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE];
    uint8_t delay_counts[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE];

    std::vector<Vec3> delay_gates;
    std::vector<Vec3> not_gates;

    struct IOStatus {
        Block block;
        bool output_match;
        Block input;
        bool input_match;
        bool active;
    };
    IOStatus io_status(const Vec3 &v);
};

class World : public WorldGeometry {
  public:
    World() : redstone(this) {}

    void initialize() {
        WorldGeometry::initialize();
        redstone_dirty = true;
    }

    enum class PlayerMouseModify { PlaceBlock, BreakBlock, RotateBlock };

    void player_click(Ray ray, Block block, PlayerMouseModify player_action);

    void tick() {
        if (redstone_dirty) {
            redstone_dirty = false;
            redstone.rebuild();
        }
        redstone.tick();
    }

    void reset();
    bool load(const char *filepath);
    bool save(const char *filepath);
    void log_frame() { Log::log_frame_world(num_vertices / VERTICES_PER_BLOCK); }

    Block get_block(int x, int y, int z) { return WorldGeometry::get_block(x, y, z); }
    void set_block(int x, int y, int z, Block block) {
        WorldGeometry::set_block(x, y, z, block);
        redstone_dirty = true;
    }
    void delete_block(int x, int y, int z) {
        WorldGeometry::delete_block(x, y, z);
        redstone_dirty = true;
    }
    void rotate_block(int x, int y, int z) {
        WorldGeometry::rotate_block(x, y, z);
        redstone_dirty = true;
    }
    void randomize() {
        WorldGeometry::randomize();
        redstone_dirty = true;
    }
    void wireframe() {
        WorldGeometry::wireframe();
        redstone_dirty = true;
    }

  private:
    RedstoneCircuit redstone;
    bool redstone_dirty = false;

    void _derive_geometry_from_block_map() {
        num_vertices = 0;
        std::fill((int *)block_coordinates_to_id, (int *)block_coordinates_to_id + BLOCKS, -1);
        std::fill((Vec3 *)block_coordinate_order, (Vec3 *)block_coordinate_order + BLOCKS, Vec3());
        memset((void *)&world_buffer_data[0], 0, BLOCKS * sizeof(uint8_t));

        for (int x = 0; x < WORLD_SIZE; ++x) {
            for (int y = 0; y < WORLD_SIZE; ++y) {
                for (int z = 0; z < WORLD_SIZE; ++z) {
                    set_block(x, y, z, block_map[x][y][z]);
                }
            }
        }
    }
};
