#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include "nesPch.h"

struct AppOptions {
    // Status of the parse operation
    // 0 is a success
    int parseStatus;

    // File paths
    std::string romPath;
    std::string palettePath;

    // Run emulator in windowed mode
    bool windowed;
    // Hide the top and bottom 8 pixels of the screen
    bool hideDangerZone;
    // Use vsync
    bool useVsync;
};

AppOptions parseArguments(int argc, char *argv[]);

#endif
