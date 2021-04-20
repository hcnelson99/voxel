#include "world.h"
#include <iostream>

World world;

int main() {
    world.set_block(0, 0, 0, Block(Block::DiodeGate, Orientation::PosX));
    world.set_block(1, 0, 0, Block(Block::NotGate, Orientation::PosX));
    world.save("test.world");
    return 0;
}
