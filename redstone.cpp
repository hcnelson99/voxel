#include "redstone_config.h"
#include "world.h"
#include <stack>
#include <string.h>
#include <vector>

void RedstoneCircuit::init() { memset(&delay_counts[0][0][0], 0, sizeof(delay_counts)); }

void RedstoneCircuit::rebuild() {
    not_gates.clear();
    delay_gates.clear();

    memset(&signal_map[0][0][0], 0, sizeof(signal_map));

    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int y = 0; y < WORLD_SIZE; ++y) {
            for (int z = 0; z < WORLD_SIZE; ++z) {
                Block block = world_geometry->get_block(x, y, z);

                if (!block.is_delay_gate()) {
                    delay_counts[x][y][z] = 0xff;
                }

                if (block.is_not_gate()) {
                    const Vec3 &v = Vec3(x, y, z);
                    not_gates.emplace_back(v);
                } else if (block.is_delay_gate()) {
                    const Vec3 &v = Vec3(x, y, z);
                    delay_gates.emplace_back(v);
                } else if (block.is_redstone()) {
                    world_geometry->set_block(x, y, z, Block::InactiveRedstone);
                }
            }
        }
    }
}

RedstoneCircuit::IOStatus RedstoneCircuit::io_status(const Vec3 &v) {
    const Block block = world_geometry->get_block(v);
    const Orientation orientation = block.get_orientation();
    const Block output = world_geometry->get_block_safe(v + orientation.direction());
    const Block input = world_geometry->get_block_safe(v + orientation.opposite().direction());
    const bool active = block.is_active();

    return {.block = block,
            .output_match = output.is_active() == active,
            .input = input,
            .input_match = input.is_active() == active,
            .active = active};
}

void RedstoneCircuit::tick() {
    std::stack<Signal> top_level;

    for (const Vec3 &delay_gate : delay_gates) {
        uint8_t &delay = delay_counts[delay_gate.x][delay_gate.y][delay_gate.z];
        if (delay != 0xff) {
            delay--;
        }
    }

    for (const Vec3 &delay_gate : delay_gates) {
        const IOStatus status = io_status(delay_gate);
        if (!status.input_match && (status.input.is_redstone() || status.input.is(Block::Air))) {
            const Orientation &orientation = status.block.get_orientation();
            send_signal(Signal(delay_gate, orientation, !status.active));
        }
        if (!status.output_match) {
            const Orientation &orientation = status.block.get_orientation();
            send_signal(Signal(delay_gate + orientation.direction(), orientation, status.active));
        }
    }

    for (const Vec3 &not_gate : not_gates) {
        const IOStatus status = io_status(not_gate);
        if (!status.input_match && (status.input.is_redstone() || status.input.is(Block::Air))) {
            const Orientation &orientation = status.block.get_orientation();
            send_signal(Signal(not_gate, orientation, !status.active));
        }
        if (status.output_match) {
            const Orientation &orientation = status.block.get_orientation();
            send_signal(Signal(not_gate + orientation.direction(), orientation, !status.active));
        }
    }
}

void RedstoneCircuit::send_signal(Signal root_signal) {
    frontier.clear();
    new_frontier.clear();

    frontier.push_back(root_signal);

    while (frontier.size() > 0) {
        for (const Signal &signal : frontier) {
            if (signal.position.in_world()) {
                Block block = world_geometry->get_block(signal.position);

                const Vec3 &p = signal.position;

                if (block.is_redstone()) {
                    uint8_t &existing_signal = signal_map[p.x][p.y][p.z];

                    if (signal.active) {
                        existing_signal |= (1 << signal.direction);
                    } else {
                        existing_signal &= ~(1 << signal.direction);
                    }

                    if (existing_signal & 0b111111 & (~(1 << signal.direction))) {
                        // another active signal is powering this redstone
                        if (!signal.active) {
                            Orientation o = signal.direction.opposite();
                            new_frontier.emplace_back(p + o.direction(), o, true);
                        }
                        continue;
                    }

                    Block::BlockType block_type = signal.active ? Block::ActiveRedstone : Block::InactiveRedstone;

                    if (block.is(block_type)) {
                        continue;
                    }

                    world_geometry->set_block(p, Block(block_type, block.get_orientation()));

                    // these must be in the same order as orientation
                    new_frontier.emplace_back(Vec3(p.x - 1, p.y, p.z), Orientation::NegX, signal.active);
                    new_frontier.emplace_back(Vec3(p.x + 1, p.y, p.z), Orientation::PosX, signal.active);
                    new_frontier.emplace_back(Vec3(p.x, p.y - 1, p.z), Orientation::NegY, signal.active);
                    new_frontier.emplace_back(Vec3(p.x, p.y + 1, p.z), Orientation::PosY, signal.active);
                    new_frontier.emplace_back(Vec3(p.x, p.y, p.z - 1), Orientation::NegZ, signal.active);
                    new_frontier.emplace_back(Vec3(p.x, p.y, p.z + 1), Orientation::PosZ, signal.active);

                    // TODO: try minimize branching
                    new_frontier[new_frontier.size() - 6 + signal.direction.opposite()].position.invalidate();
                } else if (block.is_not_gate() && block.get_orientation() == signal.direction) {
                    const Orientation orientation = block.get_orientation();
                    new_frontier.emplace_back(signal.position + orientation.direction(), orientation, !signal.active);
                    Block::BlockType block_type = signal.active ? Block::ActiveNotGate : Block::NotGate;
                    world_geometry->set_block(signal.position, Block(block_type, block.get_orientation()));
                } else if (block.is_delay_gate() && block.get_orientation() == signal.direction) {
                    // don't add back to frontier since the signal is delayed
                    Block::BlockType block_type = signal.active ? Block::ActiveDelayGate : Block::DelayGate;

                    uint8_t &delay = delay_counts[p.x][p.y][p.z];
                    if (!block.is(block_type)) {
                        delay = std::min(delay, DELAY_TICKS);

                        if (delay == 0) {
                            world_geometry->set_block(signal.position, Block(block_type, block.get_orientation()));
                            delay = 0xff;
                            const Orientation orientation = block.get_orientation();
                            new_frontier.emplace_back(signal.position + orientation.direction(), orientation,
                                                      signal.active);
                        }
                    } else {
                        delay = 0xff;
                    }
                }
            }
        }

        std::swap(frontier, new_frontier);
        new_frontier.clear();
    }
}
