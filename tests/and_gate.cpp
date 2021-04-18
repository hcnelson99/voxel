#include "../world.h"
#include "test_helpers.h"
#include <iostream>

World world;

void setup() {
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 1, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(0, 2, 0, Block::InactiveRedstone);
    world.set_block(1, 2, 0, Block::InactiveRedstone);
    world.set_block(2, 2, 0, Block::InactiveRedstone);
    world.set_block(0, 3, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(2, 3, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(0, 4, 0, Block::InactiveRedstone);
    world.set_block(2, 4, 0, Block::InactiveRedstone);
}

void test_false_false() {
    setup();

    world.tick();

    const auto check = []() {
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::InactiveRedstone));
    };

    check();
    world.tick();
    check();
}

void test_true_false() {
    setup();
    world.set_block(0, 5, 0, Block(Block::NotGate, Orientation::NegY));

    world.tick();

    const auto check = []() {
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 4, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::InactiveRedstone));
    };

    check();
    world.tick();
    check();
}

void test_false_true() {
    setup();
    world.set_block(2, 5, 0, Block(Block::NotGate, Orientation::NegY));

    world.tick();

    const auto check = []() {
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::ActiveRedstone));
    };

    check();
    world.tick();
    check();
}

void test_true_true() {
    setup();
    world.set_block(0, 5, 0, Block(Block::NotGate, Orientation::NegY));
    world.set_block(2, 5, 0, Block(Block::NotGate, Orientation::NegY));

    world.tick();

    const auto check = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 4, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::ActiveRedstone));
    };

    check();
    world.tick();
    check();
}

int main() {
    test_false_false();
    world.reset();
    test_true_false();
    world.reset();
    test_false_true();
    world.reset();
    test_true_true();

    return 0;
}
