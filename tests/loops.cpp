#include "../world.h"
#include <iostream>

World world;

void setup() {
    world.set_block(0, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 0, 0, Block(Block::NotGate, Orientation::PosX));
    world.set_block(2, 0, 0, Block(Block::NotGate, Orientation::PosX));
    world.set_block(3, 0, 0, Block::InactiveRedstone);
    world.set_block(3, 1, 0, Block::InactiveRedstone);
    world.set_block(3, 2, 0, Block::InactiveRedstone);
    world.set_block(3, 3, 0, Block::InactiveRedstone);
    world.set_block(2, 3, 0, Block::InactiveRedstone);
    world.set_block(1, 3, 0, Block::InactiveRedstone);
    world.set_block(0, 3, 0, Block::InactiveRedstone);
    world.set_block(0, 2, 0, Block::InactiveRedstone);
    world.set_block(0, 1, 0, Block::InactiveRedstone);
}

void check(const Block::BlockType &block) {
    assert(world.get_block(0, 0, 0).is(block));
    assert(world.get_block(3, 0, 0).is(block));
    assert(world.get_block(3, 1, 0).is(block));
    assert(world.get_block(3, 2, 0).is(block));
    assert(world.get_block(3, 3, 0).is(block));
    assert(world.get_block(2, 3, 0).is(block));
    assert(world.get_block(1, 3, 0).is(block));
    assert(world.get_block(0, 3, 0).is(block));
    assert(world.get_block(0, 2, 0).is(block));
    assert(world.get_block(0, 1, 0).is(block));
}

void basic_double_negation_loop() {
    setup();

    world.tick();
    check(Block::InactiveRedstone);
    world.tick();
    check(Block::InactiveRedstone);
}

void activated_double_negation_loop() {
    setup();
    world.set_block(3, 4, 0, Block(Block::NotGate, Orientation::NegY));

    world.tick();
    check(Block::ActiveRedstone);
    world.tick();
    check(Block::ActiveRedstone);
}

void not_activated_double_negation_loop() {
    setup();
    world.set_block(3, 4, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(3, 5, 0, Block::InactiveRedstone);
    world.set_block(3, 6, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(3, 7, 0, Block::InactiveRedstone);

    world.tick();
    check(Block::InactiveRedstone);
    world.tick();
    check(Block::InactiveRedstone);
}

void activated_double_negation_loop2() {
    setup();
    world.set_block(3, 4, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(3, 5, 0, Block::InactiveRedstone);
    world.set_block(3, 6, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(3, 7, 0, Block::InactiveRedstone);
    world.set_block(3, 8, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(3, 9, 0, Block::InactiveRedstone);

    world.tick();
    check(Block::ActiveRedstone);
    world.tick();
    check(Block::ActiveRedstone);
}

void double_loop() {
    world.set_block(0, 0, 1, Block::InactiveRedstone);
    world.set_block(0, 0, 2, Block::InactiveRedstone);
    world.set_block(1, 0, 2, Block::InactiveRedstone);
    world.set_block(2, 0, 2, Block::InactiveRedstone);
    world.set_block(3, 0, 2, Block::InactiveRedstone);
    world.set_block(3, 0, 1, Block::InactiveRedstone);
    world.set_block(4, 0, 2, Block::InactiveRedstone);
    world.set_block(5, 0, 2, Block::InactiveRedstone);
    world.set_block(6, 0, 2, Block::InactiveRedstone);
    world.set_block(6, 0, 1, Block::InactiveRedstone);
    world.set_block(3, 0, 0, Block::ActiveSwitch);
    world.set_block(1, 0, 1, Block(Block::NotGate, Orientation::NegX));
    world.set_block(2, 0, 1, Block(Block::NotGate, Orientation::NegX));
    world.set_block(4, 0, 1, Block(Block::NotGate, Orientation::PosX));
    world.set_block(5, 0, 1, Block(Block::NotGate, Orientation::PosX));

    const auto check = []() {
        assert(world.get_block(0, 0, 1).is(Block::ActiveRedstone));
        assert(world.get_block(0, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(1, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(3, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(3, 0, 1).is(Block::ActiveRedstone));
        assert(world.get_block(4, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(5, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(6, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(6, 0, 1).is(Block::ActiveRedstone));
        assert(world.get_block(3, 0, 0).is(Block::ActiveSwitch));
    };

    world.tick();
    check();
    world.tick();
    check();
}

int main() {
    basic_double_negation_loop();
    world.reset();
    activated_double_negation_loop();
    world.reset();
    not_activated_double_negation_loop();
    world.reset();
    activated_double_negation_loop2();
    world.reset();
    double_loop();
    return 0;
}
