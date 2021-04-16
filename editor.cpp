#include "world.h"
#include <iostream>

World world;

int main() {
    world.set_block(0, 0, 0, Block::Wood);
    world.set_block(0, 1, 0, Block(Block::NotGate, Orientation::PosZ));
    world.save("test.world");
    return 0;
}
