#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "log.h"

struct Repl {
    // blocking
    static void *read(void *arg) {
        (void)arg;

        while (true) {
            std::string command;
            std::getline(std::cin, command);

            char c;
            if (sscanf(command.c_str(), "%c", &c) > 0) {
                switch (c) {
                case 'h':
                    printf("Help:\n");
                    printf("  h - print this help message\n");
                    printf("  l - toggle logging to stderr\n");
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
