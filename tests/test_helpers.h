#pragma once

#include "../redstone_config.h"

extern World world;

void tick_delay(auto f, int cutoff = 0) {
    for (int i = 0; i < DELAY_TICKS - cutoff; i++) {
        if (i != DELAY_TICKS - cutoff - 1) {
            f();
        }
        world.tick();
    }
}
