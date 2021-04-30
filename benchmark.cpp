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

int main() {
    world = new World;
    world->load(world_file.c_str());

    for (int f = 0; f < 2; f++) {
        world->set_block(0, 0, 0, world->get_block(0, 0, 0));
        size_t first_tick = tick();

        constexpr size_t n = 2;
        size_t ticks[n];
        for (size_t i = 0; i < n; i++) {
            ticks[i] = tick();
        }

        cout << first_tick << endl;
        for (size_t i = 0; i < n; i++) {
            cout << ticks[i] << endl;
        }
    }

    return 0;
}
