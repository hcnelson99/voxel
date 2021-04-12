#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "log.h"
#include "world.h"

extern World world;

struct Repl {
    // blocking
    static void *read(void *arg) {
        (void)arg;

        while (true) {
            std::string command;
            std::getline(std::cin, command);

            char c;
            char buffer[50];
            // yikes possible buffer overflow
            if (sscanf(command.c_str(), "%c %49s", &c, buffer) == 2) {
                switch (c) {
                case 'o':
                    world.load(buffer);
                    break;
                case 's':
                    world.save(buffer);
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
                    world.reset();
                    break;
                case 'l':
                    Log::toggle_logging();
                    break;
                }
            }
        }

        return NULL;
    }
};
