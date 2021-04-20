#include "../world.h"
#include <iostream>

World world;

void test1() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::NotGate);
    world.set_block(2, 0, 0, Block::NotGate);
    world.set_block(3, 0, 0, Block::NotGate);
    world.set_block(4, 0, 0, Block::NotGate);
    world.set_block(5, 0, 0, Block::InactiveRedstone);

    // tick once to initialize redstone
    world.tick();

    // should alternate
    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::NotGate));
    assert(world.get_block(2, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(3, 0, 0).is(Block::NotGate));
    assert(world.get_block(4, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(5, 0, 0).is(Block::ActiveRedstone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::NotGate));
    assert(world.get_block(2, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(3, 0, 0).is(Block::NotGate));
    assert(world.get_block(4, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(5, 0, 0).is(Block::ActiveRedstone));
}

void test2() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::NotGate);
    world.set_block(2, 0, 0, Block::NotGate);
    world.set_block(3, 0, 0, Block::NotGate);
    world.set_block(4, 0, 0, Block::InactiveRedstone);

    // tick once to initialize redstone
    world.tick();

    // should alternate
    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::NotGate));
    assert(world.get_block(2, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(3, 0, 0).is(Block::NotGate));
    assert(world.get_block(4, 0, 0).is(Block::InactiveRedstone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::NotGate));
    assert(world.get_block(2, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(3, 0, 0).is(Block::NotGate));
    assert(world.get_block(4, 0, 0).is(Block::InactiveRedstone));
}

int main() {

    test1();

    world.reset();

    test2();

    return 0;
}
