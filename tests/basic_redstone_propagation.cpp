#include "../world.h"
#include <iostream>

World world;

void test_straight_line() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);
    world.set_block(3, 0, 0, Block::InactiveRedstone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::NotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::NotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
}

void test_or_gate() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);
    world.set_block(3, 0, 0, Block::InactiveRedstone);
    world.set_block(3, 1, 0, Block::InactiveRedstone);
    world.set_block(3, 0, 1, Block::InactiveRedstone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::NotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 1, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 0, 1).is(Block::ActiveRedstone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::NotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 1, 0).is(Block::ActiveRedstone));
    assert(world.get_block(3, 0, 1).is(Block::ActiveRedstone));
}

int main() {

    test_straight_line();

    world.reset();

    test_or_gate();

    world.reset();

    return 0;
}
