#include "../world.h"
#include <iostream>

World world;

void test_straight_line() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 0, 0, Block::InactiveGreenstone);
    world.set_block(3, 0, 0, Block::InactiveGreenstone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveGreenstone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveGreenstone));
}

void test_or_gate() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 0, 0, Block::InactiveGreenstone);
    world.set_block(3, 0, 0, Block::InactiveGreenstone);
    world.set_block(3, 1, 0, Block::InactiveGreenstone);
    world.set_block(3, 0, 1, Block::InactiveGreenstone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 1, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 0, 1).is(Block::ActiveGreenstone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 1, 0).is(Block::ActiveGreenstone));
    assert(world.get_block(3, 0, 1).is(Block::ActiveGreenstone));
}

void test_off_ring() {
    world.set_block(0, 0, 0, Block::InactiveGreenstone);
    world.set_block(1, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 1, 0, Block::InactiveGreenstone);
    world.set_block(2, 2, 0, Block::InactiveGreenstone);
    world.set_block(1, 2, 0, Block::InactiveGreenstone);
    world.set_block(0, 2, 0, Block::InactiveGreenstone);
    world.set_block(0, 1, 0, Block::InactiveGreenstone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::InactiveGreenstone));
    assert(world.get_block(1, 0, 0).is(Block::InactiveGreenstone));
    assert(world.get_block(2, 0, 0).is(Block::InactiveGreenstone));
    assert(world.get_block(2, 1, 0).is(Block::InactiveGreenstone));
    assert(world.get_block(2, 2, 0).is(Block::InactiveGreenstone));
    assert(world.get_block(1, 2, 0).is(Block::InactiveGreenstone));
    assert(world.get_block(0, 2, 0).is(Block::InactiveGreenstone));
    assert(world.get_block(0, 1, 0).is(Block::InactiveGreenstone));
}

void test_off_ring2() {
    world.set_block(0, 0, 0, Block::InactiveGreenstone);
    world.set_block(1, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 1, 0, Block::InactiveGreenstone);
    world.set_block(2, 2, 0, Block::InactiveGreenstone);
    world.set_block(1, 2, 0, Block::InactiveGreenstone);
    world.set_block(0, 2, 0, Block::InactiveGreenstone);
    world.set_block(0, 1, 0, Block::InactiveGreenstone);
    world.set_block(3, 0, 0, Block(Block::NotGate, Orientation::PosX));
    world.set_block(4, 0, 0, Block::InactiveGreenstone);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(1, 0, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(2, 1, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(2, 2, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(1, 2, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(0, 2, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(0, 1, 0).is(Block::InactiveGreenstone));
        assert(world.get_block(4, 0, 0).is(Block::ActiveGreenstone));
    };

    world.tick();
    check();
    world.tick();
    check();
}

void test_red_blue_incompatible() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 1, 0, Block::InactiveBluestone);
    world.set_block(3, 0, 0, Block::InactiveGreenstone);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
        assert(world.get_block(1, 0, 0).is(Block::ActiveGreenstone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 1, 0).is(Block::InactiveBluestone));
        assert(world.get_block(3, 0, 0).is(Block::InactiveGreenstone));
    };

    world.tick();
    check();
    world.tick();
    check();
}

void test_blue_red_combine_with_diode() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveGreenstone);
    world.set_block(2, 0, 0, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(3, 0, 0, Block::InactiveGreenstone);

    world.set_block(0, 0, 2, Block::NotGate);
    world.set_block(1, 0, 2, Block::InactiveRedstone);
    world.set_block(2, 0, 2, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(3, 0, 2, Block::InactiveGreenstone);

    world.set_block(3, 0, 1, Block::InactiveGreenstone);
    world.set_block(4, 0, 1, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(5, 0, 1, Block::InactiveRedstone);

    const auto check = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveGreenstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveGreenstone));

        assert(world.get_block(1, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 2).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 2).is(Block::ActiveGreenstone));

        assert(world.get_block(3, 0, 1).is(Block::ActiveGreenstone));
        assert(world.get_block(4, 0, 1).is(Block::ActiveDiodeGate));
        assert(world.get_block(5, 0, 1).is(Block::ActiveRedstone));
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

    world.reset();

    test_red_blue_incompatible();

    world.reset();

    test_blue_red_combine_with_diode();

    return 0;
}
