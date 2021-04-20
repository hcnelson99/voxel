#include "../world.h"
#include "test_helpers.h"
#include <iostream>

World world;

void test1() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::DiodeGate);
    world.set_block(3, 0, 0, Block::InactiveRedstone);

    const auto check1 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
    };

    world.tick();
    check1();
    world.tick();
    check1();
}

void test2() {
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::DiodeGate);
    world.set_block(3, 0, 0, Block::InactiveRedstone);
    world.set_block(4, 0, 0, Block(Block::NotGate, Orientation::NegX));

    const auto check1 = []() {
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::DiodeGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
    };

    world.tick();
    check1();
    world.tick();
    check1();
}

void test3() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::DiodeGate);
    world.set_block(3, 0, 0, Block::DiodeGate);
    world.set_block(4, 0, 0, Block::InactiveRedstone);
    world.set_block(5, 0, 0, Block::DelayGate);
    world.set_block(6, 0, 0, Block::DiodeGate);
    world.set_block(7, 0, 0, Block::DelayGate);
    world.set_block(8, 0, 0, Block::InactiveRedstone);
    world.set_block(0, 1, 0, Block::InactiveRedstone);

    world.set_block(2, 1, 0, Block::DiodeGate);
    world.set_block(3, 1, 0, Block::DiodeGate);

    world.tick();
    const auto check1 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(4, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(5, 0, 0).is(Block::DelayGate));
        assert(world.get_block(6, 0, 0).is(Block::DiodeGate));
        assert(world.get_block(7, 0, 0).is(Block::DelayGate));
        assert(world.get_block(8, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
    };
    check1();
    tick_delay(check1);

    const auto check2 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(4, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(5, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(6, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(7, 0, 0).is(Block::DelayGate));
        assert(world.get_block(8, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
    };
    check2();
    tick_delay(check2);

    {
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(4, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(5, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(6, 0, 0).is(Block::ActiveDiodeGate));
        assert(world.get_block(7, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(8, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
    }
}

int main() {
    test1();
    world.reset();
    test2();
    world.reset();
    test3();
}
