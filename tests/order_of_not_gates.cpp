#include "../world.h"
#include <iostream>

World world;

void on_overrides_off1() {
    world.set_block(0, 0, 0, Block(Block::NotGate, Orientation::PosY));
    world.set_block(0, 1, 0, Block::InactiveRedstone);
    world.set_block(0, 2, 0, Block::InactiveRedstone);
    world.set_block(0, 3, 0, Block::InactiveRedstone);
    world.set_block(0, 4, 0, Block::InactiveRedstone);
    world.set_block(0, 5, 0, Block::InactiveRedstone);

    world.set_block(1, 3, 0, Block(Block::NotGate, Orientation::NegX));
    world.set_block(2, 3, 0, Block(Block::NotGate, Orientation::NegX));

    world.tick();

    const auto check1 = []() {
        assert(world.get_block(0, 1, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 3, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 4, 0).is(Block::ActiveRedstone));
        assert(world.get_block(0, 5, 0).is(Block::ActiveRedstone));
    };

    check1();
    world.tick();
    check1();

    world.delete_block(2, 3, 0);
    world.tick();

    check1();
    world.tick();
    check1();

    world.delete_block(1, 3, 0);
    world.tick();

    check1();
    world.tick();
    check1();
}

void setup(int z) {
    world.set_block(0, 0, z, Block::NotGate);
    world.set_block(1, 0, z, Block::InactiveRedstone);
    world.set_block(2, 0, z, Block::NotGate);
    world.set_block(3, 0, z, Block::InactiveRedstone);
    world.set_block(1, 1, z, Block::InactiveRedstone);
    world.set_block(3, 1, z, Block(Block::NotGate, Orientation::NegY));
    world.set_block(1, 2, z, Block::InactiveRedstone);
    world.set_block(2, 2, z, Block(Block::NotGate, Orientation::PosX));
    world.set_block(3, 2, z, Block::InactiveRedstone);
}

void on_overrides_off2() {
    setup(0);

    world.tick();

    const auto check1 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(3, 2, 0).is(Block::InactiveRedstone));
    };
    check1();

    // state should be steady
    world.tick();

    check1();

    world.delete_block(0, 0, 0);
    world.tick();

    const auto check2 = []() {
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(3, 2, 0).is(Block::ActiveRedstone));
    };
    check2();

    world.tick();
    check2();
}

void on_propagate_overrides_off() {
    setup(0);
    setup(2);

    world.set_block(4, 0, 0, Block::NotGate);
    world.set_block(4, 0, 2, Block::NotGate);
    world.set_block(5, 0, 0, Block::InactiveRedstone);
    world.set_block(5, 0, 1, Block::InactiveRedstone);
    world.set_block(5, 0, 2, Block::InactiveRedstone);

    world.tick();

    const auto check1 = []() {
        assert(world.get_block(1, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::ActiveRedstone));
        assert(world.get_block(3, 2, 0).is(Block::InactiveRedstone));

        assert(world.get_block(1, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(3, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 2).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 2).is(Block::ActiveRedstone));
        assert(world.get_block(3, 2, 2).is(Block::InactiveRedstone));

        assert(world.get_block(5, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(5, 0, 1).is(Block::InactiveRedstone));
        assert(world.get_block(5, 0, 2).is(Block::InactiveRedstone));
    };

    check1();
    world.tick();
    check1();

    world.delete_block(0, 0, 0);
    world.tick();

    const auto check2 = []() {
        assert(world.get_block(1, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(3, 0, 0).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 0).is(Block::InactiveRedstone));
        assert(world.get_block(1, 2, 0).is(Block::InactiveRedstone));
        assert(world.get_block(3, 2, 0).is(Block::ActiveRedstone));

        assert(world.get_block(1, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(3, 0, 2).is(Block::ActiveRedstone));
        assert(world.get_block(1, 1, 2).is(Block::ActiveRedstone));
        assert(world.get_block(1, 2, 2).is(Block::ActiveRedstone));
        assert(world.get_block(3, 2, 2).is(Block::InactiveRedstone));

        assert(world.get_block(5, 0, 0).is(Block::InactiveRedstone));
        assert(world.get_block(5, 0, 1).is(Block::InactiveRedstone));
        assert(world.get_block(5, 0, 2).is(Block::InactiveRedstone));
    };
    check2();
    world.tick();
    check2();
}

int main() {
    on_overrides_off1();
    world.reset();
    on_overrides_off2();
    world.reset();
    on_propagate_overrides_off();
    return 0;
}
