#include "../world.h"
#include "test_helpers.h"
#include <functional>
#include <iostream>

World world;

#define ROUNDS (12)

void test_three_part_blinker() {
    world.set_block(0, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 0, 0, Block(Block::DelayGate, Orientation::PosX));
    world.set_block(2, 0, 0, Block::InactiveRedstone);

    world.set_block(0, 1, 0, Block::InactiveRedstone);
    world.set_block(2, 1, 0, Block::InactiveRedstone);

    world.set_block(0, 2, 0, Block(Block::DelayGate, Orientation::NegY));
    world.set_block(2, 2, 0, Block(Block::DelayGate, Orientation::PosY));

    world.set_block(0, 3, 0, Block::InactiveRedstone);
    world.set_block(2, 3, 0, Block::InactiveRedstone);

    world.set_block(0, 4, 0, Block::InactiveRedstone);
    world.set_block(1, 4, 0, Block::InactiveRedstone);
    world.set_block(2, 4, 0, Block::InactiveRedstone);

    world.set_block(2, 5, 0, Block(Block::NotGate, Orientation::NegY));

    world.tick();

    const auto check_first_blink = []() {
        assert(world.get_block(0, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 3, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 3, 0).is(Block::ActiveRedstone));

        assert(world.get_block(0, 4, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 4, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::ActiveRedstone));
    };

    const auto check_second_blink = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 1, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 3, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 3, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::InactiveRedstone));
    };

    const auto check_third_blink = []() {
        assert(world.get_block(0, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveRedstone));

        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::ActiveRedstone));

        assert(world.get_block(0, 3, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 3, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::InactiveRedstone));
    };

    tick_delay(check_first_blink, 1);

    world.delete_block(2, 5, 0);
    world.tick();

    std::function<void()> checks[3] = {check_second_blink, check_third_blink, check_first_blink};

    for (int round = 0; round < ROUNDS; round++) {
        checks[0]();
        tick_delay(checks[0]);

        std::swap(checks[0], checks[1]);
        std::swap(checks[1], checks[2]);
    }
}

void test_two_part_blinker() {
    world.set_block(0, 0, 0, Block::InactiveRedstone);
    world.set_block(1, 0, 0, Block::InactiveRedstone);
    world.set_block(2, 0, 0, Block::InactiveRedstone);

    world.set_block(0, 1, 0, Block::InactiveRedstone);
    world.set_block(2, 1, 0, Block::InactiveRedstone);

    world.set_block(0, 2, 0, Block(Block::DelayGate, Orientation::NegY));
    world.set_block(2, 2, 0, Block(Block::DelayGate, Orientation::PosY));

    world.set_block(0, 3, 0, Block::InactiveRedstone);
    world.set_block(2, 3, 0, Block::InactiveRedstone);

    world.set_block(0, 4, 0, Block::InactiveRedstone);
    world.set_block(1, 4, 0, Block::InactiveRedstone);
    world.set_block(2, 4, 0, Block::InactiveRedstone);

    world.set_block(2, 5, 0, Block(Block::NotGate, Orientation::NegY));

    world.tick();

    const auto check_first_blink = []() {
        assert(world.get_block(0, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 3, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 3, 0).is(Block::ActiveRedstone));

        assert(world.get_block(0, 4, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 4, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::ActiveRedstone));
    };

    const auto check_second_blink = []() {
        assert(world.get_block(0, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 0, 0).is(Block::ActiveRedstone));

        assert(world.get_block(0, 1, 0).is(Block::ActiveRedstone));
        assert(world.get_block(2, 1, 0).is(Block::ActiveRedstone));

        assert(world.get_block(0, 3, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 3, 0).is(Block::InactiveRedstone));

        assert(world.get_block(0, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 4, 0).is(Block::InactiveRedstone));
        assert(world.get_block(2, 4, 0).is(Block::InactiveRedstone));
    };

    tick_delay(check_first_blink, 1);

    world.delete_block(2, 5, 0);
    world.tick();

    std::function<void()> checks[2] = {check_second_blink, check_first_blink};

    for (int round = 0; round < ROUNDS; round++) {
        checks[0]();
        tick_delay(checks[0]);

        std::swap(checks[0], checks[1]);
    }
}

int main() {
    test_two_part_blinker();
    world.reset();
    test_three_part_blinker();
    return 0;
}
