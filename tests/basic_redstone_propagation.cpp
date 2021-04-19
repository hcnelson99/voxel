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

void test_off_ring() {
    world.set_block(0, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 1, 0, Block::InactiveRedstone);
    world.set_block(2, 2, 0, Block::InactiveRedstone);
    world.set_block(1, 2, 0, Block::InactiveRedstone);
    world.set_block(0, 2, 0, Block::InactiveRedstone);
    world.set_block(0, 1, 0, Block::InactiveRedstone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::InactiveRedstone));
    assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
    assert(world.get_block(2, 0, 0).is(Block::InactiveRedstone));
    assert(world.get_block(2, 1, 0).is(Block::InactiveRedstone));
    assert(world.get_block(2, 2, 0).is(Block::InactiveRedstone));
    assert(world.get_block(1, 2, 0).is(Block::InactiveRedstone));
    assert(world.get_block(0, 2, 0).is(Block::InactiveRedstone));
    assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
}

void test_off_ring2() {
    world.set_block(0, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 1, 0, Block::InactiveRedstone);
    world.set_block(2, 2, 0, Block::InactiveRedstone);
    world.set_block(1, 2, 0, Block::InactiveRedstone);
    world.set_block(0, 2, 0, Block::InactiveRedstone);
    world.set_block(0, 1, 0, Block::InactiveRedstone);
    world.set_block(3, 0, 0, Block(Block::NotGate, Orientation::PosX));
    world.set_block(4, 0, 0, Block::InactiveRedstone);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(4, 0, 0).is(Block::ActiveRedstone));
    };

    world.tick();
    check();
    world.tick();
    check();
}

int main() {
    test_straight_line();

    world.reset();

    test_or_gate();

    world.reset();

    test_off_ring();

    world.reset();

    test_off_ring2();

    return 0;
}
