#include "../world.h"
#include <iostream>

World world;

void test_straight_line() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 0, 0, Block::InactiveBluestone);
    world.set_block(3, 0, 0, Block::InactiveBluestone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveBluestone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveBluestone));
}

void test_or_gate() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 0, 0, Block::InactiveBluestone);
    world.set_block(3, 0, 0, Block::InactiveBluestone);
    world.set_block(3, 1, 0, Block::InactiveBluestone);
    world.set_block(3, 0, 1, Block::InactiveBluestone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 1, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 0, 1).is(Block::ActiveBluestone));

    // state should be steady
    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
    assert(world.get_block(1, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(2, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 0, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 1, 0).is(Block::ActiveBluestone));
    assert(world.get_block(3, 0, 1).is(Block::ActiveBluestone));
}

void test_off_ring() {
    world.set_block(0, 0, 0, Block::InactiveBluestone);
    world.set_block(1, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 1, 0, Block::InactiveBluestone);
    world.set_block(2, 2, 0, Block::InactiveBluestone);
    world.set_block(1, 2, 0, Block::InactiveBluestone);
    world.set_block(0, 2, 0, Block::InactiveBluestone);
    world.set_block(0, 1, 0, Block::InactiveBluestone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::InactiveBluestone));
    assert(world.get_block(1, 0, 0).is(Block::InactiveBluestone));
    assert(world.get_block(2, 0, 0).is(Block::InactiveBluestone));
    assert(world.get_block(2, 1, 0).is(Block::InactiveBluestone));
    assert(world.get_block(2, 2, 0).is(Block::InactiveBluestone));
    assert(world.get_block(1, 2, 0).is(Block::InactiveBluestone));
    assert(world.get_block(0, 2, 0).is(Block::InactiveBluestone));
    assert(world.get_block(0, 1, 0).is(Block::InactiveBluestone));
}

void test_off_ring2() {
    world.set_block(0, 0, 0, Block::InactiveBluestone);
    world.set_block(1, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 1, 0, Block::InactiveBluestone);
    world.set_block(2, 2, 0, Block::InactiveBluestone);
    world.set_block(1, 2, 0, Block::InactiveBluestone);
    world.set_block(0, 2, 0, Block::InactiveBluestone);
    world.set_block(0, 1, 0, Block::InactiveBluestone);
    world.set_block(3, 0, 0, Block(Block::NotGate, Orientation::PosX));
    world.set_block(4, 0, 0, Block::InactiveBluestone);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::InactiveBluestone));
        assert(world.get_block(1, 0, 0).is(Block::InactiveBluestone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveBluestone));
        assert(world.get_block(2, 1, 0).is(Block::InactiveBluestone));
        assert(world.get_block(2, 2, 0).is(Block::InactiveBluestone));
        assert(world.get_block(1, 2, 0).is(Block::InactiveBluestone));
        assert(world.get_block(0, 2, 0).is(Block::InactiveBluestone));
        assert(world.get_block(0, 1, 0).is(Block::InactiveBluestone));
        assert(world.get_block(4, 0, 0).is(Block::ActiveBluestone));
    };

    world.tick();
    check();
    world.tick();
    check();
}

void test_blue_red_incompatible() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);
    world.set_block(3, 0, 0, Block::InactiveBluestone);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
        assert(world.get_block(1, 0, 0).is(Block::ActiveBluestone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(3, 0, 0).is(Block::InactiveBluestone));
    };

    world.tick();
    check();
    world.tick();
    check();
}

void test_blue_red_combine_with_diode() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveBluestone);
    world.set_block(2, 0, 0, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(3, 0, 0, Block::InactiveBluestone);

    world.set_block(0, 0, 2, Block::NotGate);
    world.set_block(1, 0, 2, Block::InactiveRedstone);
    world.set_block(2, 0, 2, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(3, 0, 2, Block::InactiveBluestone);

    world.set_block(3, 0, 1, Block::InactiveBluestone);
    world.set_block(4, 0, 1, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(5, 0, 1, Block::InactiveRedstone);

    const auto check = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveBluestone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveBluestone));

        assert(world.get_block(1, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 2).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 2).is(Block::ActiveBluestone));

        assert(world.get_block(3, 0, 1).is(Block::ActiveBluestone));
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

    test_blue_red_incompatible();

    world.reset();

    test_blue_red_combine_with_diode();

    return 0;
}
