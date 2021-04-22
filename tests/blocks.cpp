#include "../world.h"
#include "test_helpers.h"
#include <iostream>

World world;

void empty_world() {
    world.reset();

    world.tick();
    world.tick();
}

void single_gate() {
    world.set_block(0, 0, 0, Block::DelayGate);

    world.tick();
    assert(world.get_block(0, 0, 0).is(Block::DelayGate));
    world.tick();
    assert(world.get_block(0, 0, 0).is(Block::DelayGate));
}

int main() {
    empty_world();
    world.reset();
    single_gate();
}
