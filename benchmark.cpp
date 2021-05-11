#include "world.h"
#include <chrono>
#include <iostream>
#include <string>

using namespace std;

World *world;

#define XSTR(x) STR(x)
#define STR(x) #x
#define WORLD_FILE XSTR(WORLD)

std::string world_file = WORLD_FILE;

size_t tick() {
    auto start = std::chrono::steady_clock::now();
    { world->tick(); }
    auto end = std::chrono::steady_clock::now();
    size_t ms_elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    return ms_elapsed;
}

size_t diff(Tensor<Block, WORLD_SIZE> &temp) {
    size_t count = 0;
    for (int x = 0; x < WORLD_SIZE; x++) {
        for (int y = 0; y < WORLD_SIZE; y++) {
            for (int z = 0; z < WORLD_SIZE; z++) {
                const auto block = world->get_block(x, y, z);
                if (temp(x, y, z) != block) {
                    count++;
                }
                temp(x, y, z) = block;
            }
        }
    }
    return count;
}

int main() {
    world = new World;
    world->load(world_file.c_str());

    std::cout << world_file << "\n" << std::endl;

    for (int f = 0; f < 2; f++) {
        world->set_block(0, 0, 0, world->get_block(0, 0, 0));
        size_t first_tick = tick();
        cout << "tick & rebuild  : " << first_tick << "\n" << endl;

        constexpr size_t n = 2;
        size_t ticks[n];
        for (size_t i = 0; i < n; i++) {
            ticks[i] = tick();
            cout << "tick            : " << ticks[i] << "\n" << endl;
        }
    }

    {
        int redstone = 0;
        int not_gate = 0;
        int delay_gate = 0;
        int diode = 0;
        int blocks = 0;
        for (int x = 0; x < WORLD_SIZE; x++) {
            for (int y = 0; y < WORLD_SIZE; y++) {
                for (int z = 0; z < WORLD_SIZE; z++) {
                    Block block = world->get_block(x, y, z);

                    if (!block.is(Block::Air)) {
                        blocks++;
                    }

                    if (block.is_redstone() || block.is_bluestone() || block.is_greenstone()) {
                        redstone++;
                    } else if (block.is_not_gate()) {
                        not_gate++;
                    } else if (block.is_delay_gate()) {
                        delay_gate++;
                    } else if (block.is_diode_gate()) {
                        diode++;
                    }
                }
            }
        }

        std::cout << "\nBlock Counts:" << std::endl;
        std::cout << "total     : " << blocks << std::endl;
        std::cout << "redstone  : " << redstone << std::endl;
        std::cout << "not_gate  : " << not_gate << std::endl;
        std::cout << "delay_gate: " << delay_gate << std::endl;
        std::cout << "diode     : " << diode << std::endl;
    }

    Tensor<Block, WORLD_SIZE> temp;
    diff(temp);

    std::cout << "\nGathering tick stats: " << std::endl;

    size_t changed = 0;
    tick();
    changed += diff(temp);
    tick();
    changed += diff(temp);
    tick();
    changed += diff(temp);
    tick();
    changed += diff(temp);

    std::cout << "\nAverage blocks changed per tick: " << changed / 4 << std::endl;

    const auto &indices = world->expression_indices;
    std::cout << "\nLevels: " << indices.size() - 1 << std::endl;
    for (size_t i = 0; i < indices.size() - 1; i++) {
        std::cout << i << ": " << indices[i + 1] - indices[i] << std::endl;
    }
    std::cout << "unknown height: " << ((world->ordered_expressions.size()) - (*indices.rbegin())) << std::endl;

    return 0;
}
