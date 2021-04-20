#include "../world.h"
#include "test_helpers.h"
#include <iostream>

World world;

void test_non_conductive() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::Switch);
    world.set_block(3, 0, 0, Block::InactiveRedstone);

    const auto check = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveNotGate));
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::Switch));
        assert(world.get_block(3, 0, 0).is(Block::InactiveRedstone));
    };

    world.tick();
    check();
    world.tick();
    check();
}

void test_basic() {
    world.set_block(1, 1, 0, Block::Switch);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(0, 1, 0, Block::InactiveRedstone);
    world.set_block(2, 1, 0, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(3, 1, 0, Block::InactiveRedstone);
    world.set_block(1, 2, 0, Block(Block::NotGate, Orientation::PosY));
    world.set_block(1, 3, 0, Block::InactiveRedstone);

    const auto check_off = []() {
        assert(world.get_block(1, 1, 0).is(Block::Switch));
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::DiodeGate));
        assert(world.get_block(3, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::ActiveNotGate));
        assert(world.get_block(1, 3, 0).is(Block::ActiveRedstone));
    };

    const auto check_on = []() {
        assert(world.get_block(1, 1, 0).is(Block::ActiveSwitch));
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 1, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 1, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::NotGate));
        assert(world.get_block(1, 3, 0).is(Block::InactiveRedstone));
    };

    world.tick();
    check_off();
    world.tick();
    check_off();

    world.set_block(1, 1, 0, Block::ActiveSwitch);

    world.tick();
    check_on();
    world.tick();
    check_on();
}

void test_delay() {
    world.set_block(0, 0, 0, Block::Switch);
    world.set_block(1, 0, 0, Block::DelayGate);
    world.set_block(2, 0, 0, Block::DelayGate);
    world.set_block(3, 0, 0, Block::InactiveRedstone);

    world.tick();

    assert(world.get_block(0, 0, 0).is(Block::Switch));
    assert(world.get_block(1, 0, 0).is(Block::DelayGate));
    assert(world.get_block(2, 0, 0).is(Block::DelayGate));
    assert(world.get_block(3, 0, 0).is(Block::InactiveRedstone));

    world.set_block(0, 0, 0, Block::ActiveSwitch);
    world.tick();

    const auto check1 = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveSwitch));
        assert(world.get_block(1, 0, 0).is(Block::DelayGate));
        assert(world.get_block(2, 0, 0).is(Block::DelayGate));
        assert(world.get_block(3, 0, 0).is(Block::InactiveRedstone));
    };
    tick_delay(check1);

    const auto check2 = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveSwitch));
        assert(world.get_block(1, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(2, 0, 0).is(Block::DelayGate));
        assert(world.get_block(3, 0, 0).is(Block::InactiveRedstone));
    };
    tick_delay(check2);

    const auto check3 = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveSwitch));
        assert(world.get_block(1, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
    };
    tick_delay(check3);
    check3();
    world.tick();
    check3();
}

int main() {
    test_basic();
    world.reset();
    test_non_conductive();
    world.reset();
    test_delay();
    return 0;
}
