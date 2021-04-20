#include "config.h"
#include "world.h"
#include <iostream>
#include <stack>
#include <string.h>
#include <vector>

constexpr uint32_t ALWAYS_TRUE = 1;
constexpr uint32_t ALWAYS_FALSE = 2;

constexpr uint8_t EVALUATION_UNDEFINED = 2;
constexpr uint8_t EVALUATION_IN_PROGRESS = 3;

void RedstoneCircuit::rebuild() {
    delay_gates.clear();

    expressions.clear();
    // index 0: unused since indices are unsigned so 0 is default (therefore invalid)
    // index 1: always true
    // index 2: always false
    expressions.resize(3);

    block_to_expression.clear(0);
    rebuild_visited.clear(0);

    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                const Vec3 v(x, y, z);
                Block block = world_geometry->get_block_safe(v);
                build_expression(v, block);

                if (block.is_delay_gate()) {
                    delay_gates.emplace_back(x, y, z);
                } else {
                    delays(v).reset();
                }
            }
        }
    }
}

uint32_t RedstoneCircuit::set_expression(const Vec3 &v, uint32_t expr_i, Expression &expr) {
    if (expr_i == 0) {
        if (block_to_expression(v) != 0) {
            int k = block_to_expression(v);
            expressions[k] = expr;
            return k;
        } else {
            uint32_t k = expressions.size();
            expressions.resize(k + 1);
            expressions[k] = expr;
            block_to_expression(v) = k;
            return k;
        }
    } else {
        if (block_to_expression(v) != 0) {
            int k = block_to_expression(v);
            expressions[k].init(Expression::Type::Alias);
            expressions[k].alias = expr_i;
            return k;
        } else {
            block_to_expression(v) = expr_i;
            return expr_i;
        }
    }
}

template <uint32_t Default, bool Negate>
uint32_t RedstoneCircuit::build_directed_expression(const Vec3 &v, const Block &block) {
    const Vec3 input_v = v + block.get_orientation().opposite().direction();
    const Block input = world_geometry->get_block_safe(input_v);
    Expression expr;

    if (!input.output_in_direction(block.get_orientation())) {
        return set_expression(v, Default, expr);
    } else {
        const uint32_t i = build_expression(input_v, input);

        constexpr uint32_t true_input = Negate ? ALWAYS_FALSE : ALWAYS_TRUE;
        constexpr uint32_t false_input = Negate ? ALWAYS_TRUE : ALWAYS_FALSE;

        switch (i) {
        case ALWAYS_TRUE:
            return set_expression(v, true_input, expr);
        case ALWAYS_FALSE:
            return set_expression(v, false_input, expr);
        case 0:
            assert(false);
        default:
            if (Negate) {
                expr.init(Expression::Type::Negation);
                expr.negation = i;
                return set_expression(v, 0, expr);
            } else {
                return set_expression(v, i, expr);
            }
        }
    }
}

template <bool BallPredicate(const Block &b), bool TerminalPredicate(const Block &b)>
uint32_t RedstoneCircuit::build_ball_expression(const Vec3 &v, const Block &block) {
    std::vector<Vec3> terminals;
    std::vector<Vec3> ball(1, v);
    std::vector<Vec3> frontier(1, v);
    std::vector<Vec3> new_frontier;

    const auto add = [&](const Vec3 &vec, const Orientation &o) {
        const Vec3 nv = vec + o.direction();

        if (!nv.in_world()) {
            return;
        }

        const Block neighbor = world_geometry->get_block(nv);
        if (BallPredicate(neighbor) && !rebuild_visited(nv)) {
            rebuild_visited(nv) = true;
            ball.push_back(nv);
            new_frontier.push_back(nv);
        } else if (TerminalPredicate(neighbor) && neighbor.output_in_direction(o.opposite())) {
            terminals.push_back(nv);
        }
    };

    while (frontier.size() > 0) {
        for (const Vec3 &vec : frontier) {
            add(vec, Orientation::PosX);
            add(vec, Orientation::NegX);
            add(vec, Orientation::PosY);
            add(vec, Orientation::NegY);
            add(vec, Orientation::PosZ);
            add(vec, Orientation::NegZ);
        }

        std::swap(frontier, new_frontier);
        new_frontier.clear();
    }

    Expression expr;
    expr.init(Expression::Type::Disjunction);
    expr.disjuncts->resize(terminals.size());

    bool always_true = false;
    bool always_false = true;

    for (size_t k = 0; k < terminals.size(); k++) {
        const Vec3 &vec = terminals[k];
        uint32_t i = build_expression(vec, world_geometry->get_block(vec));
        if (i != ALWAYS_FALSE) {
            always_false = false;
        }
        if (i == ALWAYS_TRUE) {
            always_true = true;
            break;
        }
        (*expr.disjuncts)[k] = i;
    }

    uint32_t expr_i;
    if (always_false) {
        expr_i = ALWAYS_FALSE;
    } else if (always_true) {
        expr_i = ALWAYS_TRUE;
    } else if (terminals.size() == 1) {
        expr_i = expr.disjuncts->at(0);
    } else {
        expr_i = expressions.size();
        expressions.resize(expr_i + 1);
        expressions[expr_i] = expr;
    }

    for (const Vec3 &vec : ball) {
        Expression temp_expr;
        set_expression(vec, expr_i, temp_expr);
    }
    return expr_i;
}

namespace BallPredicates {
static inline bool is_redstone(const Block &b) { return b.is_redstone(); }
static inline bool is_bluestone(const Block &b) { return b.is_bluestone(); }
static inline bool not_conductor(const Block &b) { return !b.is_conductor(); }
static inline bool is_display(const Block &b) { return false; }
static inline bool not_display(const Block &b) { return true; }
} // namespace BallPredicates

uint32_t RedstoneCircuit::build_expression(const Vec3 &v, const Block &block) {
    using namespace BallPredicates;

    if (rebuild_visited(v)) {
        if (block_to_expression(v) == 0) {
            uint32_t i = expressions.size();
            expressions.resize(i + 1);
            block_to_expression(v) = i;
            return i;
        } else {
            return block_to_expression(v);
        }
    }

    rebuild_visited(v) = true;

    if (block.is_redstone()) {
        return build_ball_expression<is_redstone, not_conductor>(v, block);
    } else if (block.is_bluestone()) {
        return build_ball_expression<is_bluestone, not_conductor>(v, block);
    } else if (block.is_display()) {
        return build_ball_expression<is_display, not_display>(v, block);
    } else if (block.is_not_gate()) {
        return build_directed_expression<ALWAYS_TRUE, true>(v, block);
    } else if (block.is_diode_gate()) {
        return build_directed_expression<ALWAYS_FALSE, false>(v, block);
    } else if (block.is_delay_gate() || block.is_switch()) {
        Expression expr;
        expr.init(Expression::Type::Variable);
        expr.variable = v;
        return set_expression(v, 0, expr);
    } else {
        return 0;
    }
}

void RedstoneCircuit::tick() {
    evaluation_memo.clear();
    evaluation_memo.resize(expressions.size());
    std::fill(evaluation_memo.begin(), evaluation_memo.end(), EVALUATION_UNDEFINED);

    for (const Vec3 &v : delay_gates) {
        Delay &delay = delays(v);
        uint8_t &ticks = delay.ticks;
        if (ticks != 0xff) {
            ticks--;

            if (ticks == 0) {
                world_geometry->set_active(v.x, v.y, v.z, delay.activating);
                ticks = 0xff;
            }
        }
    }

    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                if (block_to_expression(x, y, z) != 0) {
                    bool active = evaluate(block_to_expression(x, y, z));
                    world_geometry->set_active(x, y, z, active);
                }
            }
        }
    }

    for (const Vec3 &v : delay_gates) {
        Delay &delay = delays(v);
        const Block &block = world_geometry->get_block(v);

        const Vec3 input_v = v + block.get_orientation().opposite().direction();
        const Block &input = world_geometry->get_block_safe(input_v);
        if (input.is_active() != block.is_active()) {
            if (delay.ticks == 0xff || input.is_active() != delay.activating) {
                delay.activating = input.is_active();
                delay.ticks = DELAY_TICKS;
            }
        } else {
            delay.ticks = 0xff;
        }
    }
}

bool RedstoneCircuit::evaluate(uint32_t expr_i) {
    if (expr_i == ALWAYS_TRUE) {
        return true;
    } else if (expr_i == ALWAYS_FALSE) {
        return false;
    } else if (evaluation_memo[expr_i] < EVALUATION_UNDEFINED) {
        return evaluation_memo[expr_i];
    } else if (evaluation_memo[expr_i] == EVALUATION_IN_PROGRESS) {
        // in a loop
        evaluation_memo[expr_i] = false;
        return false;
    }
    evaluation_memo[expr_i] = EVALUATION_IN_PROGRESS;

    const Expression &expr = expressions[expr_i];
    switch (expr.get_type()) {
    case Expression::Type::Variable:
        evaluation_memo[expr_i] = world_geometry->get_block(expr.variable).is_active();
        return evaluation_memo[expr_i];
    case Expression::Type::Negation:
        evaluation_memo[expr_i] = !evaluate(expr.negation);
        return evaluation_memo[expr_i];
    case Expression::Type::Disjunction:
        for (const uint32_t &i : *(expr.disjuncts)) {
            if (evaluate(i)) {
                evaluation_memo[expr_i] = true;
                return true;
            }
        }
        evaluation_memo[expr_i] = false;
        return false;
    case Expression::Type::Alias:
        evaluation_memo[expr_i] = evaluate(expr.alias);
        return evaluation_memo[expr_i];
    }
    assert(false);
}

std::string RedstoneCircuit::Expression::to_string() {
    switch (type) {
    case Type::Invalid:
        return "Invalid";
    case Type::Variable:
        return "Variable(" + variable.to_string() + ")";
    case Type::Negation:
        return "Negation(" + std::to_string(negation) + ")";
    case Type::Disjunction: {
        std::stringstream ss;
        ss << "Disjunction(";
        for (size_t i = 0; i < disjuncts->size(); i++) {
            if (i == 0) {
                ss << disjuncts->at(i);
            } else {
                ss << " v " << disjuncts->at(i);
            }
        }
        ss << ")";
        return ss.str();
    }
    case Type::Alias:
        return "Alias(" + std::to_string(alias) + ")";
    }
    assert(false);
}
