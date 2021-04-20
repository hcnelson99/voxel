#pragma once

#include <atomic>
#include <iostream>
#include <string>
#include <vector>

#include "log.h"
#include "world.h"

extern World *world;

struct Repl {
    /*
     * We use a test-and-set lock here. This lock lock is only used by 2
     * threads, the main thread and the repl thread. It has low contention
     * since the repl thread only locks it upon user input, which we assume to
     * be extremely rate.
     */
    static void lock() {
        while (_lock.exchange(true))
            ;
    }

    static void unlock() { _lock.store(false); }

    // blocking
    static void *read(void *arg) {
        (void)arg;

        while (true) {
            std::string command;
            std::getline(std::cin, command);

            Repl::lock();
            Repl::loop(command);
            Repl::unlock();
        }

        return NULL;
    }

  private:
    inline static std::atomic<bool> _lock = {false};

    static void loop(const std::string &command) {
        char c;
        char buffer[50];
        // yikes possible buffer overflow
        if (sscanf(command.c_str(), "%c %49s", &c, buffer) == 2) {
            switch (c) {
            case 'o':
                world->load(buffer);
                break;
            case 's':
                world->save(buffer);
                break;
            case 'c':
                world->copy(command);
                break;
            }
        } else if (sscanf(command.c_str(), "%c", &c) == 1) {
            switch (c) {
            case 'h':
                printf("Help:\n");
                printf("  h - print this help message\n");
                printf("  r - reset the world\n");
                printf("  l - toggle logging to stderr\n");
                printf("  o [filename] - open a world file\n");
                printf("  s [filename] - save world to a file\n");
                break;
            case 'r':
                world->reset();
                break;
            case 'l':
                Log::toggle_logging();
                break;
            }
        }
    }
};
