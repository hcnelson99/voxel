#pragma once

#include "log.h"
#include <chrono>
#include <stdio.h>

typedef std::chrono::steady_clock::time_point ctime_t;

struct Log {
    static size_t frames;
    static ctime_t prev_time;
    static bool enabled;

    static void toggle_logging(bool on) {
        enabled = on;
        if (enabled) {
            printf("logging enabled\n");
        } else {
            printf("logging disabled\n");
        }
    }
    static void toggle_logging() { toggle_logging(!enabled); }

    static void log_frame_world(unsigned int num_blocks) {
        if (enabled) {
            size_t frame = frames++;
            ctime_t now = std::chrono::steady_clock::now();
            double since_last_frame = std::chrono::duration<double>(now - prev_time).count();
            prev_time = now;

            fprintf(stderr, "> frame(%lu) blocks(%u) ms(%.1f) fps(%.1f)\n", frame, num_blocks, since_last_frame * 1000,
                    1.f / since_last_frame);
        }
    }
};
