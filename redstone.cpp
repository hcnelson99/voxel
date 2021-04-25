#include "config.h"
#include "world.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <omp.h>
#include <stack>
#include <string.h>
#include <vector>

constexpr uint32_t ALWAYS_TRUE = 1;
constexpr uint32_t ALWAYS_FALSE = 2;

constexpr uint8_t EVALUATION_UNDEFINED = 2;
constexpr uint8_t EVALUATION_IN_PROGRESS = 3;

constexpr int THREAD_LOCAL_EXPRS = 100;

unsigned int num_threads;

inline unsigned int thread_mask() { return 1 << omp_get_thread_num(); }

void RedstoneCircuit::rebuild() {
    delay_gates.clear();

    expressions.clear();
    // index 0: unused since indices are unsigned so 0 is default (therefore invalid)
    // index 1: always true
    // index 2: always false
    num_expressions.store(3);

    block_to_expression.clear();

    rebuild_visited.clear();

    unsigned int max_expressions = 3;

    std::mutex delay_gates_lock;

    {
#pragma omp parallel
        {
#pragma omp single
            num_threads = omp_get_num_threads();
        }
    }

    thread_data.clear();
    thread_data.resize(num_threads);

#pragma omp parallel for collapse(3) reduction(+ : max_expressions)
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                const Block block = world_geometry->get_block_safe(x, y, z);
                if (block.is_delay_gate()) {
                    delay_gates_lock.lock();
                    delay_gates.emplace_back(x, y, z);
                    delay_gates_lock.unlock();
                } else {
                    delays(x, y, z).reset();
                }

                if (!block.is(Block::Air)) {
                    max_expressions++;
                }
            }
        }
    }

    expressions.resize(max_expressions + THREAD_LOCAL_EXPRS * num_threads);

#pragma omp parallel for collapse(3)
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                const Vec3 v(x, y, z);
                Block block = world_geometry->get_block_safe(v);
                build_expression(v, block);
            }
        }
    }

    const unsigned int _num_expressions = num_expressions.load();

    ordered_expressions.resize(_num_expressions);
    expression_indices.resize(_num_expressions);
    index_to_expression.resize(_num_expressions);
    std::iota(expression_indices.begin(), expression_indices.end(), 0);
    std::iota(index_to_expression.begin(), index_to_expression.end(), 0);

    // sort the expressions by height
    {
        std::vector<unsigned int> height_counts(1, 0);
        std::vector<unsigned int> height_indices;

        for (size_t i = 3; i < _num_expressions; i++) {
            const uint32_t &height = expressions[i].height;
            if (height == UINT_MAX) {
                height_counts[0]++;
            } else if (height + 1 >= height_counts.size()) {
                height_counts.resize(height + 2);
                height_counts[height + 1] = 1;
            } else {
                height_counts[height + 1]++;
            }
        }

        unsigned int levels = height_counts.size();
        height_indices.resize(levels);
        height_indices[1] = 3;
        for (size_t i = 2; i < levels; i++) {
            height_indices[i] = height_indices[i - 1] + height_counts[i - 1];
        }
        height_indices[0] = height_indices[levels - 1] + height_counts[levels - 1];

        for (size_t i = 3; i < _num_expressions; i++) {
            uint32_t height = expressions[i].height;
            if (height == UINT_MAX) {
                height = 0;
            } else {
                height++;
            }
            const unsigned int index = height_indices[height]++;
            expression_indices[index] = i;
        }

        // std::sort(expression_indices.begin() + 3, expression_indices.end(),
        //          [this](uint32_t i1, uint32_t i2) { return expressions[i1].height < expressions[i2].height; });
    }

    {
        for (size_t i = 0; i < expression_indices.size(); i++) {
            ordered_expressions[i] = expressions[expression_indices[i]];
            index_to_expression[expression_indices[i]] = i;
        }

        expression_indices.clear();
        expression_indices.push_back(3);
        {
            size_t i;
            for (i = 4; i < index_to_expression.size(); i++) {
                if (ordered_expressions[i].height == UINT_MAX) {
                    break;
                }
                if (ordered_expressions[i].height != ordered_expressions[i - 1].height) {
                    expression_indices.push_back(i);
                }
            }
            expression_indices.push_back(std::min(i, index_to_expression.size()));
        }
    }
}

inline void RedstoneCircuit::cancel_allocated_expression() {
    ThreadData &thread = thread_data[omp_get_thread_num()];
    thread.remaining++;
    thread.expression_index--;
}

inline uint32_t RedstoneCircuit::allocate_expression() {
    ThreadData &thread = thread_data[omp_get_thread_num()];
    uint32_t i;
    if (thread.remaining == 0) {
        thread.remaining = THREAD_LOCAL_EXPRS;
        thread.expression_index = num_expressions.fetch_add(THREAD_LOCAL_EXPRS);
    }

    i = thread.expression_index++;
    thread.remaining--;

    expressions[i].height = 0;
    return i;
}

inline uint32_t RedstoneCircuit::get_expression_midbuild(const Vec3 &v) {
    uint32_t i = allocate_expression();
    uint32_t ret = 0;
    if (block_to_expression(v).compare_exchange_strong(ret, i)) {
        expressions[i].height = UINT_MAX;
        ret = i;
    } else {
        cancel_allocated_expression();
    }
    return ret;
}

uint32_t RedstoneCircuit::set_expression(const Vec3 &v, uint32_t expr_i) {
    uint32_t ret = 0;
    if (block_to_expression(v).compare_exchange_strong(ret, expr_i)) {
        ret = expr_i;
    } else {
        expressions[ret].init_linear(Expression::Type::Alias);
        expressions[ret].height = UINT_MAX;
        expressions[ret].alias = expr_i;
    }
    return ret;
}

uint32_t RedstoneCircuit::set_expression(const Vec3 &v, Expression &expr) {
    uint32_t ret = 0;
    uint32_t i = allocate_expression();

    if (block_to_expression(v).compare_exchange_strong(ret, i)) {
        expressions[i] = expr;
        ret = i;
    } else {
        expressions[ret] = expr;
        cancel_allocated_expression();
    }

    return ret;
}

template <uint32_t Default, bool Negate>
uint32_t RedstoneCircuit::build_directed_expression(const Vec3 &v, const Block &block) {
    const Vec3 input_v = v + block.get_orientation().opposite().direction();
    const Block input = world_geometry->get_block_safe(input_v);

    if (!input.output_in_direction(block.get_orientation())) {
        return set_expression(v, Default);
    } else {
        const uint32_t i = build_expression(input_v, input);

        constexpr uint32_t true_input = Negate ? ALWAYS_FALSE : ALWAYS_TRUE;
        constexpr uint32_t false_input = Negate ? ALWAYS_TRUE : ALWAYS_FALSE;

        switch (i) {
        case ALWAYS_TRUE:
            return set_expression(v, true_input);
        case ALWAYS_FALSE:
            return set_expression(v, false_input);
        case 0:
            assert(false);
        default:
            if (Negate) {
                Expression expr;
                expr.init_linear(Expression::Type::Negation);
                if (expressions[i].height == UINT_MAX) {
                    expr.height = UINT_MAX;
                } else {
                    expr.height = expressions[i].height + 1;
                }
                expr.negation = i;
                return set_expression(v, expr);
            } else {
                return set_expression(v, i);
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

    unsigned int thread = thread_mask();

    const auto _early_return = [&]() {
        for (const Vec3 &vec : ball) {
            rebuild_visited(vec).fetch_and(~thread);
        }

        return get_expression_midbuild(v);
    };

    const auto _add = [&](const Vec3 &vec, const Orientation &o) {
        const Vec3 nv = vec + o.direction();

        if (!nv.in_world()) {
            return false;
        }

        const Block neighbor = world_geometry->get_block(nv);
        if (BallPredicate(neighbor)) {
            unsigned int mask = rebuild_visited(nv).fetch_or(thread) & ~(thread - 1);
            if (thread < mask) {
                rebuild_visited(nv).fetch_and(~thread);
                return true;
            } else if (thread > mask) {
                ball.push_back(nv);
                new_frontier.push_back(nv);
            }
        } else if (TerminalPredicate(neighbor) && neighbor.output_in_direction(o.opposite())) {
            terminals.push_back(nv);
        }

        return false;
    };

    while (frontier.size() > 0) {
        for (const Vec3 &vec : frontier) {
            if (_add(vec, Orientation::PosX) || _add(vec, Orientation::NegX) || _add(vec, Orientation::PosY) ||
                _add(vec, Orientation::NegY) || _add(vec, Orientation::PosZ) || _add(vec, Orientation::NegZ)) {
                return _early_return();
            }
        }

        std::swap(frontier, new_frontier);
        new_frontier.clear();
    }

    bool always_true = false;
    bool always_false = true;

    uint32_t max_height = 0;
    std::vector<uint32_t> terms;
    for (const Vec3 &vec : terminals) {
        uint32_t i = build_expression(vec, world_geometry->get_block(vec));
        switch (i) {
        case ALWAYS_FALSE:
            break;
        case ALWAYS_TRUE:
            always_false = false;
            always_true = true;
            break;
        case 0:
            assert(false);
        default:
            always_false = false;
            max_height = std::max(max_height, expressions[i].height);
            terms.push_back(i);
        }
    }

    uint32_t expr_i;
    if (always_false) {
        expr_i = ALWAYS_FALSE;
    } else if (always_true) {
        expr_i = ALWAYS_TRUE;
    } else if (terms.size() == 1) {
        expr_i = terms[0];
    } else {
        Expression expr;
        expr.init_disjunction(terms.size());

        std::sort(terms.begin(), terms.end(),
                  [this](uint32_t a, uint32_t b) { return expressions[a].height < expressions[b].height; });
        memcpy(expr.disjuncts.expressions, terms.data(), sizeof(uint32_t) * terms.size());

        if (max_height == UINT_MAX) {
            expr.height = UINT_MAX;
        } else {
            expr.height = max_height + 1;
        }

        expr_i = allocate_expression();
        expressions[expr_i] = expr;
    }

    for (const Vec3 &vec : ball) {
        set_expression(vec, expr_i);
    }

    return expr_i;
}

namespace BallPredicates {
static inline bool is_redstone(const Block &b) { return b.is_redstone(); }
static inline bool is_bluestone(const Block &b) { return b.is_bluestone(); }
static inline bool is_greenstone(const Block &b) { return b.is_greenstone(); }
static inline bool not_conductor(const Block &b) { return !b.is_conductor(); }
static inline bool is_display(const Block &b) { return false; }
static inline bool not_display(const Block &b) { return true; }
} // namespace BallPredicates

uint32_t RedstoneCircuit::build_expression(const Vec3 &v, const Block &block) {
    using namespace BallPredicates;

    {
        unsigned int zero = 0;
        if (!rebuild_visited(v).compare_exchange_strong(zero, thread_mask())) {
            return get_expression_midbuild(v);
        }
    }

    if (block.is_redstone()) {
        return build_ball_expression<is_redstone, not_conductor>(v, block);
    } else if (block.is_bluestone()) {
        return build_ball_expression<is_bluestone, not_conductor>(v, block);
    } else if (block.is_greenstone()) {
        return build_ball_expression<is_greenstone, not_conductor>(v, block);
    } else if (block.is_display()) {
        return build_ball_expression<is_display, not_display>(v, block);
    } else if (block.is_not_gate()) {
        return build_directed_expression<ALWAYS_TRUE, true>(v, block);
    } else if (block.is_diode_gate()) {
        return build_directed_expression<ALWAYS_FALSE, false>(v, block);
    } else if (block.is_delay_gate() || block.is_switch()) {
        Expression expr;
        expr.init_linear(Expression::Type::Variable);
        expr.height = 0;
        expr.variable = v;
        return set_expression(v, expr);
    } else {
        return 0;
    }
}

void RedstoneCircuit::tick() {

#pragma omp parallel for
    for (size_t i = 0; i < delay_gates.size(); i++) {
        const Vec3 &v = delay_gates[i];
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

    evaluate_parallel();

#pragma omp parallel for collapse(3)
    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                if (block_to_expression(x, y, z).load() != 0) {
                    const bool active = evaluate(index_to_expression[block_to_expression(x, y, z).load()]);
                    world_geometry->set_active(x, y, z, active);
                }
            }
        }
    }

#pragma omp parallel for
    for (size_t i = 0; i < delay_gates.size(); i++) {
        const Vec3 &v = delay_gates[i];
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

void RedstoneCircuit::evaluate_parallel() {
    evaluation_memo.clear();
    evaluation_memo.resize(num_expressions.load());
    std::fill(evaluation_memo.begin(), evaluation_memo.end(), EVALUATION_UNDEFINED);

    // for each level in the expression tree
    uint32_t total_end = 0;
    for (size_t level = 0; level < expression_indices.size() - 1; level++) {
        uint32_t start = expression_indices[level];
        uint32_t end = expression_indices[level + 1];
        total_end = end;

#pragma omp parallel for
        for (uint32_t expr_i = start; expr_i < end; expr_i++) {
            evaluate(expr_i);
        }
    }

    // fallback to serial evaluation for degenerate loop case
    for (uint32_t expr_i = total_end; expr_i < ordered_expressions.size(); expr_i++) {
        evaluate(expr_i);
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

    const Expression &expr = ordered_expressions[expr_i];
    switch (expr.get_type()) {
    case Expression::Type::Variable:
        evaluation_memo[expr_i] = world_geometry->get_block(expr.variable).is_active();
        return evaluation_memo[expr_i];
    case Expression::Type::Negation:
        evaluation_memo[expr_i] = !evaluate(index_to_expression[expr.negation]);
        return evaluation_memo[expr_i];
    case Expression::Type::Disjunction:
        for (uint32_t i = 0; i < expr.disjuncts.size; i++) {
            if (evaluate(index_to_expression[expr.disjuncts.expressions[i]])) {
                evaluation_memo[expr_i] = true;
                return true;
            }
        }
        evaluation_memo[expr_i] = false;
        return false;
    case Expression::Type::Alias:
        evaluation_memo[expr_i] = evaluate(index_to_expression[expr.alias]);
        return evaluation_memo[expr_i];
    }
    return false;
}

std::string RedstoneCircuit::Expression::to_string() const {
    std::stringstream ss;

    switch (type) {
    case Type::Invalid:
        return "Invalid";
    case Type::Variable:
        ss << "Variable(" << variable.to_string() << ")";
        break;
    case Type::Negation:
        ss << "Negation(" << negation << ")";
        break;
    case Type::Disjunction: {
        ss << "Disjunction(";
        for (size_t i = 0; i < disjuncts.size; i++) {
            if (i == 0) {
                ss << disjuncts.expressions[i];
            } else {
                ss << " v " << disjuncts.expressions[i];
            }
        }
        ss << ")";
        break;
    }
    case Type::Alias:
        ss << "Alias(" << alias << ")";
        break;
    default:
        assert(false);
    }

    ss << "<" << height << ">";

    return ss.str();
}
