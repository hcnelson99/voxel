#pragma once

#include <GL/glew.h>
#include <atomic>
#include <cstring>
#include <functional>
#include <glm/glm.hpp>
#include <mutex>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

#include "config.h"
#include "log.h"
#include "ray.h"
#include "tracy/Tracy.hpp"
#include "util.h"

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
        Glowstone = 3 << OrientationWidth,
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
  public:
    WorldGeometry() {
        vertex_texture_uv_data = new uint8_t[VERTICES_PER_BLOCK];
        block_positions = new uint32_t[BLOCKS + 1];
    }

    ~WorldGeometry() {
        delete[] vertex_texture_uv_data;
        delete[] block_positions;
    }

    struct OpenGLBuffers {
        GLuint block_ids;
        GLuint vertex_texture_uv;
        GLuint block_positions;
    };

    // this is used instead of constructor because GL functions need to be
    // initialized by GLEW first
    void initialize();

    unsigned int get_num_blocks() const { return num_blocks; }
    const OpenGLBuffers &get_buffers() const { return buffers; }

    void sync_buffers();

    Tensor<Block, WORLD_SIZE> block_map;

  protected:
    OpenGLBuffers buffers;
    bool blocks_dirty = true;
    bool geometry_dirty = true;

    // buffers that are directly used by OpenGL
    struct {
        // the texture position of each vertex
        uint8_t *vertex_texture_uv_data;

        uint32_t *block_positions;
    };

    unsigned int num_blocks = 0;

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

    template <bool mark_dirty = true> void set_active(int x, int y, int z, bool active) {
        Block block = get_block(x, y, z);

        if (block.is_active() == active) {
            return;
        }

        if (block.is_redstone()) {
            block.set_type(active ? Block::ActiveRedstone : Block::InactiveRedstone);
        } else if (block.is_bluestone()) {
            block.set_type(active ? Block::ActiveBluestone : Block::InactiveBluestone);
        } else if (block.is_greenstone()) {
            block.set_type(active ? Block::ActiveGreenstone : Block::InactiveGreenstone);
        } else if (block.is_not_gate()) {
            block.set_type(active ? Block::ActiveNotGate : Block::NotGate);
        } else if (block.is_diode_gate()) {
            block.set_type(active ? Block::ActiveDiodeGate : Block::DiodeGate);
        } else if (block.is_delay_gate()) {
            block.set_type(active ? Block::ActiveDelayGate : Block::DelayGate);
        } else if (block.is_display()) {
            block.set_type(active ? Block::ActiveDisplay : Block::Display);
        } else if (block.is_switch()) {
            block.set_type(active ? Block::ActiveSwitch : Block::Switch);
        } else {
            assert(false);
        }

        _update_block_map<mark_dirty>(x, y, z, block);
    }

    void rotate_block(int x, int y, int z);
    void randomize(float p);
    void flatworld();
    void outline();

  private:
    template <bool mark_dirty = true> void _update_block_map(int x, int y, int z, const Block &block) {
        if (mark_dirty) {
            blocks_dirty = true;
            if (block_map(x, y, z).is(Block::Air) != block.is(Block::Air)) {
                geometry_dirty = true;
            }
        }
        block_map(x, y, z) = block;
    }
    void sync_mipmapped_blockmap();
};

class WorldGeometryWithRedstone : public WorldGeometry {
  public:
    void rebuild();
    void tick();

  private:
    struct Expression {
        enum Type : uint8_t { Invalid = 0, Variable = 1, Negation = 2, Disjunction = 3, Alias = 4 };
        uint32_t height = 0;
        union {
            Vec3 variable;
            uint32_t negation;
            struct {
                uint32_t size;
                uint32_t index;
            } disjuncts;
            uint32_t alias;
        };

        std::string to_string() const;

        Expression() : type(Type::Invalid) {}
        Expression(Expression &&expr) { operator=(expr); }
        Expression(const Expression &expr) = delete;
        void operator=(Expression &expr) {
            memcpy(this, &expr, sizeof(Expression));
            expr.type = Type::Invalid;
        }

        Type get_type() const { return type; }

        void init_linear(Type _type) {
            assert(type == Type::Invalid);
            assert(_type != Type::Invalid && _type != Type::Disjunction);
            type = _type;
        }

        void init_disjunction(uint32_t terms, uint32_t index) {
            type = Type::Disjunction;
            disjuncts.size = terms;
            disjuncts.index = index;
        }

      private:
        Type type;
    };

    struct Delay {
        uint8_t ticks = 0xff;
        bool activating = false;
        void reset() { ticks = 0xff; }
    };

    Tensor<std::atomic_uint, WORLD_SIZE> rebuild_visited;
    struct ThreadData {
        uint32_t remaining = 0;
        uint32_t expression_index;
        uint32_t padding[14];
    };
    std::vector<ThreadData> thread_data;
    std::atomic_uint num_expressions;
    std::vector<Expression> expressions;
    Tensor<std::atomic_uint, WORLD_SIZE> block_to_expression;

    std::atomic_uint disjunction_bump_allocator;
    std::vector<uint32_t> disjunction_memory;

    std::vector<uint32_t> index_to_expression;

    std::vector<uint8_t> evaluation_memo;

    Tensor<Delay, WORLD_SIZE> delays;
    std::vector<Vec3> delay_gates;

    inline uint32_t allocate_expression();
    inline void cancel_allocated_expression();
    inline uint32_t get_expression_midbuild(const Vec3 &v);
    uint32_t set_expression(const Vec3 &v, uint32_t expr_i);
    uint32_t set_expression(const Vec3 &v, Expression &expr);
    uint32_t build_expression(const Vec3 &v, const Block &block);

    template <uint32_t Default, bool Negate> uint32_t build_directed_expression(const Vec3 &v, const Block &block);

    template <bool BallPredicate(const Block &b), bool TerminalPredicate(const Block &b)>
    uint32_t build_ball_expression(const Vec3 &v, const Block &block);

    void update_blocks();

    bool evaluate(uint32_t expr_i);
    void evaluate_parallel();

  public:
    // make these public to allow benchmark to read them
    std::vector<uint32_t> expression_indices;
    std::vector<Expression> ordered_expressions;
};

class World : public WorldGeometryWithRedstone {
  public:
    World() { block_coordinates_to_id.clear(-1); }

    void initialize() {
        WorldGeometry::initialize();
        redstone_dirty = true;
    }

    enum class PlayerMouseModify { PlaceBlock, BreakBlock, RotateBlock, Identify };

    void handle_player_action(PlayerMouseModify player_action, Block block, glm::ivec3 pos,
                              std::optional<glm::ivec3> prev_pos);
    void player_click_normal(Ray ray, Block block, PlayerMouseModify player_action);
    void player_click_mipmapped(Ray ray, Block block, PlayerMouseModify player_action);
    void player_click(Ray ray, Block block, PlayerMouseModify player_action);

    void tick() {
        ZoneScoped;
        if (redstone_dirty) {
            redstone_dirty = false;
            WorldGeometryWithRedstone::rebuild();
        }
        WorldGeometryWithRedstone::tick();
    }

    void reset();
    bool load(const char *filepath);
    bool save(const char *filepath);
    void copy(std::string cmd);
    void log_frame() { Log::log_frame_world(num_blocks); }

    Block get_block(int x, int y, int z) { return WorldGeometry::get_block(x, y, z); }

    // used for testing my CPU implementation of mipmapped raycasting (hence why it's so inefficient)
    bool get_block_mipmapped(int x, int y, int z, int level) {
        if (level == 0) {
            Block block = get_block(x, y, z);
            return !block.is(Block::BlockType::Air);
        }

        int scale = 1;
        for (int i = 0; i < level; i++) {
            scale *= 2;
        }

        assert(0 <= x && x < WORLD_SIZE / scale);
        assert(0 <= y && y < WORLD_SIZE / scale);
        assert(0 <= z && z < WORLD_SIZE / scale);

        for (int i = 0; i < scale; i++) {
            for (int j = 0; j < scale; j++) {
                for (int k = 0; k < scale; k++) {
                    Block block = get_block(x * scale + i, y * scale + j, z * scale + k);
                    if (!block.is(Block::BlockType::Air)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
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
    void randomize(float p) {
        WorldGeometry::randomize(p);
        redstone_dirty = true;
    }
    void flatworld() {
        WorldGeometry::flatworld();
        redstone_dirty = true;
    }
    void outline() {
        WorldGeometry::outline();
        redstone_dirty = true;
    }

  private:
    bool redstone_dirty = false;

    void _derive_geometry_from_block_map() {
        num_blocks = 0;
        block_coordinates_to_id.clear(-1);
        std::fill((Vec3 *)block_coordinate_order, (Vec3 *)block_coordinate_order + BLOCKS, Vec3());

        for (int x = 0; x < WORLD_SIZE; ++x) {
            for (int y = 0; y < WORLD_SIZE; ++y) {
                for (int z = 0; z < WORLD_SIZE; ++z) {
                    set_block(x, y, z, block_map(x, y, z));
                }
            }
        }
    }
};
