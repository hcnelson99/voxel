#pragma once

#include <GL/glew.h>
#include <cstring>
#include <functional>
#include <glm/glm.hpp>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

#include "config.h"
#include "log.h"
#include "ray.h"
#include "tracy/Tracy.hpp"
#include "util.h"

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
    static const Orientation orientations[6];

    operator uint8_t() const { return _orientation; }
    Axis axis() const { return _axis; }
    static Orientation from(uint8_t o) { return Orientation(o); }

    static Orientation from_direction(glm::vec3 dir);

    uint8_t plane_orientation(const Orientation &o) const;

    Orientation opposite() const { return _opposite_map[_orientation]; }
    Vec3 direction() const { return _direction_map[_orientation]; }

    std::string to_string() const;

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
        ActiveDiodeGate = 10 << OrientationWidth,
        DiodeGate = 11 << OrientationWidth,
        ActiveDisplay = 12 << OrientationWidth,
        Display = 13 << OrientationWidth,
        ActiveSwitch = 14 << OrientationWidth,
        Switch = 15 << OrientationWidth,
        ActiveBluestone = 16 << OrientationWidth,
        InactiveBluestone = 17 << OrientationWidth,
        ActiveGreenstone = 18 << OrientationWidth,
        InactiveGreenstone = 19 << OrientationWidth,
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

    std::string to_string() const;

    bool is_conductor() const { return is_redstone() || is_bluestone() || is_greenstone(); }

    bool output_in_direction(const Orientation &o) const {
        return is_conductor() || is_switch() || (is_directed() && get_orientation() == o);
    }

#define IS(name, active, inactive)                                                                                     \
    bool name() const { return (_block & TypeMask) == BlockType::active || (_block & TypeMask) == BlockType::inactive; }

    IS(is_redstone, ActiveRedstone, InactiveRedstone);
    IS(is_bluestone, ActiveBluestone, InactiveBluestone);
    IS(is_greenstone, ActiveGreenstone, InactiveGreenstone);
    IS(is_not_gate, ActiveNotGate, NotGate);
    IS(is_delay_gate, ActiveDelayGate, DelayGate);
    IS(is_diode_gate, ActiveDiodeGate, DiodeGate);
    IS(is_display, ActiveDisplay, Display);
    IS(is_switch, ActiveSwitch, Switch);

    bool is_directed() const { return is_not_gate() || is_delay_gate() || is_diode_gate(); }

    bool is_active() const {
        uint8_t type = _block & TypeMask;
        return type == BlockType::ActiveRedstone || type == BlockType::ActiveBluestone ||
               type == BlockType::ActiveGreenstone || type == BlockType::ActiveNotGate ||
               type == BlockType::ActiveDelayGate || type == BlockType::ActiveDiodeGate ||
               type == BlockType::ActiveSwitch || type == BlockType::ActiveDisplay;
    }
};

// Stores data for vertices that is passed into shader
class WorldGeometry {
    friend class RedstoneCircuit;

  public:
    WorldGeometry() {
        world_buffer_data = new uint8_t[BLOCKS];
        block_face_data = new uint8_t[VERTICES];
        vertex_data = new glm::vec3[VERTICES];
        vertex_texture_uv_data = new uint8_t[VERTICES];
    }

    ~WorldGeometry() {
        delete[] world_buffer_data;
        delete[] block_face_data;
        delete[] vertex_data;
        delete[] vertex_texture_uv_data;
    }

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
        uint8_t *world_buffer_data;

        // the block type for each vertex
        uint8_t *block_face_data;

        // the position of each vertex
        glm::vec3 *vertex_data;

        // the texture position of each vertex
        uint8_t *vertex_texture_uv_data;
    };

    unsigned int num_vertices = 0;

    Tensor<Block, WORLD_SIZE> block_map;

    // maps logical block coordinate to index into list of vertices
    Tensor<int, WORLD_SIZE> block_coordinates_to_id;
    Vec3 block_coordinate_order[BLOCKS];

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
    void set_active(int x, int y, int z, bool active);
    void rotate_block(int x, int y, int z);
    void randomize();
    void flatworld();

    void _add_square(Block block, int &vertex, int x, int y, int z, Orientation face);
};

class RedstoneCircuit {
  public:
    RedstoneCircuit(WorldGeometry *g) : world_geometry(g) {}

    void rebuild();
    void tick();

  private:
    WorldGeometry *world_geometry;

    struct Expression {
        enum Type { Invalid, Variable, Negation, Disjunction, Alias };
        union {
            Vec3 variable;
            uint32_t negation;
            std::vector<uint32_t> *disjuncts;
            uint32_t alias;
        };

        std::string to_string();

        Expression() : type(Type::Invalid) {}
        Expression(Expression &&expr) { operator=(expr); }
        void operator=(Expression &expr) {
            memcpy(this, &expr, sizeof(Expression));
            expr.type = Type::Invalid;
        }

        Type get_type() const { return type; }

        void init(Type _type) {
            assert(type == Type::Invalid);
            type = _type;
            if (type == Type::Disjunction) {
                disjuncts = new std::vector<uint32_t>;
            }
        }

        ~Expression() {
            if (type == Type::Disjunction) {
                delete disjuncts;
            }
        }

      private:
        Type type;
    };

    struct Delay {
        uint8_t ticks = 0xff;
        bool activating = false;
        void reset() { ticks = 0xff; }
    };

    Tensor<bool, WORLD_SIZE> rebuild_visited;
    std::vector<Expression> expressions;
    std::vector<uint8_t> evaluation_memo;
    Tensor<Delay, WORLD_SIZE> delays;
    std::vector<Vec3> delay_gates;
    Tensor<uint32_t, WORLD_SIZE> block_to_expression;

    uint32_t set_expression(const Vec3 &v, uint32_t expr_i, Expression &expr);
    uint32_t build_expression(const Vec3 &v, const Block &block);

    template <uint32_t Default, bool Negate> uint32_t build_directed_expression(const Vec3 &v, const Block &block);

    template <bool BallPredicate(const Block &b), bool TerminalPredicate(const Block &b)>
    uint32_t build_ball_expression(const Vec3 &v, const Block &block);

    bool evaluate(uint32_t expr_i);
};

class World : public WorldGeometry {
  public:
    World() : redstone(this) { block_coordinates_to_id.clear(-1); }

    void initialize() {
        WorldGeometry::initialize();
        redstone_dirty = true;
    }

    enum class PlayerMouseModify { PlaceBlock, BreakBlock, RotateBlock, Identify };

    void player_click(Ray ray, Block block, PlayerMouseModify player_action);

    void tick() {
        ZoneScoped;
        if (redstone_dirty) {
            redstone_dirty = false;
            redstone.rebuild();
        }
        redstone.tick();
    }

    void reset();
    bool load(const char *filepath);
    bool save(const char *filepath);
    void copy(std::string cmd);
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
    void flatworld() {
        WorldGeometry::flatworld();
        redstone_dirty = true;
    }

  private:
    RedstoneCircuit redstone;
    bool redstone_dirty = false;

    void _derive_geometry_from_block_map() {
        num_vertices = 0;
        block_coordinates_to_id.clear(-1);
        std::fill((Vec3 *)block_coordinate_order, (Vec3 *)block_coordinate_order + BLOCKS, Vec3());
        memset((void *)&world_buffer_data[0], 0, BLOCKS * sizeof(uint8_t));

        for (int x = 0; x < WORLD_SIZE; ++x) {
            for (int y = 0; y < WORLD_SIZE; ++y) {
                for (int z = 0; z < WORLD_SIZE; ++z) {
                    set_block(x, y, z, block_map(x, y, z));
                }
            }
        }
    }
};
