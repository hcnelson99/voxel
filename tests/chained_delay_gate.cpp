#include "../world.h"
#include "test_helpers.h"
#include <iostream>

World world;

int main() {
    world.set_block(0, 0, 0, Block::NotGate);
    world.set_block(1, 0, 0, Block::DelayGate);
    world.set_block(2, 0, 0, Block::DelayGate);
    world.set_block(3, 0, 0, Block::DelayGate);
    world.set_block(4, 0, 0, Block::DelayGate);
    world.set_block(5, 0, 0, Block::InactiveRedstone);

    world.tick();

    tick_delay([]() {
        assert(world.get_block(1, 0, 0).is(Block::DelayGate));
        assert(world.get_block(2, 0, 0).is(Block::DelayGate));
        assert(world.get_block(3, 0, 0).is(Block::DelayGate));
        assert(world.get_block(4, 0, 0).is(Block::DelayGate));
        assert(world.get_block(5, 0, 0).is(Block::InactiveRedstone));
    });

    auto check = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(2, 0, 0).is(Block::DelayGate));
        assert(world.get_block(3, 0, 0).is(Block::DelayGate));
        assert(world.get_block(4, 0, 0).is(Block::DelayGate));
        assert(world.get_block(5, 0, 0).is(Block::InactiveRedstone));
    };
    check();

    tick_delay(check);

    auto check2 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(3, 0, 0).is(Block::DelayGate));
        assert(world.get_block(4, 0, 0).is(Block::DelayGate));
        assert(world.get_block(5, 0, 0).is(Block::InactiveRedstone));
    };
    check2();

    tick_delay(check2);

    auto check3 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(4, 0, 0).is(Block::DelayGate));
        assert(world.get_block(5, 0, 0).is(Block::InactiveRedstone));
    };
    check3();

    tick_delay(check3);

    auto check4 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(2, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(3, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(4, 0, 0).is(Block::ActiveDelayGate));
        assert(world.get_block(5, 0, 0).is(Block::ActiveRedstone));
    };
    check4();

    return 0;
}
