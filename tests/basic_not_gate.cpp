#include "../world.h"
#include <iostream>

World world;

int main() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::NotGate);
    world.set_block(3, 0, 0, Block::InactiveRedstone);
    world.set_block(4, 0, 0, Block::NotGate);
    world.set_block(5, 0, 0, Block::InactiveRedstone);
    world.set_block(6, 0, 0, Block::NotGate);
    world.set_block(7, 0, 0, Block::InactiveRedstone);

    // tick once to initialize redstone
    world.tick();

    // should alternate
    assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(3, 0, 0).is(Block::InactiveRedstone));
    assert(world.get_block(4, 0, 0).is(Block::NotGate));
    assert(world.get_block(5, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(6, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(7, 0, 0).is(Block::InactiveRedstone));

    // state should be steady
    world.tick();

    assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(3, 0, 0).is(Block::InactiveRedstone));
    assert(world.get_block(4, 0, 0).is(Block::NotGate));
    assert(world.get_block(5, 0, 0).is(Block::ActiveRedstone));
    assert(world.get_block(6, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(7, 0, 0).is(Block::InactiveRedstone));

    return 0;
}
