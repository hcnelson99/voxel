#include "../world.h"
#include "test_helpers.h"
#include <iostream>

World world;

void test_non_conductive() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::Display);
    world.set_block(3, 0, 0, Block::InactiveRedstone);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDisplay));
        assert(world.get_block(3, 0, 0).is(Block::InactiveRedstone));
    };

    world.tick();
    check();
    world.tick();
    check();
}

void test_basic() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 1, 0, Block::Display);
    world.set_block(2, 1, 0, Block::Display);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 0).is(Block::ActiveDisplay));
        assert(world.get_block(2, 1, 0).is(Block::ActiveDisplay));
    };

    world.tick();
    check();
    world.tick();
    check();

    world.delete_block(2, 0, 0);
    const auto check2 = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 0).is(Block::ActiveDisplay));
        assert(world.get_block(2, 1, 0).is(Block::Display));
    };

    world.tick();
    check2();
    world.tick();
    check2();
}

int main() {
    test_basic();
    world.reset();
    test_non_conductive();
}
