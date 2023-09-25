#include "nesPch.h"

#include <argparse/argparse.hpp>

#include "argumentParser.h"

AppOptions parseArguments(int argc, char *argv[]) {
    // Parse commands line arguments
    argparse::ArgumentParser argParser("nes_emu");

    argParser.add_argument("romPath")
        .help("specify the ROM file path");

    argParser.add_argument("-p", "--palettes")
        .default_value(std::string("resources/palettes/2C02G.pal"))
        .required()
        .help("specify the color palettes file");

    argParser.add_argument("-w", "--windowed")
        .implicit_value(true)
        .default_value(false)
        .help("start the emulator as a window");

    argParser.add_argument("--no-vsync")
        .implicit_value(true)
        .default_value(false)
        .help("start the emulator as a window");

    argParser.add_argument("--show-overscan")
        .implicit_value(true)
        .default_value(false)
        .help("show the top and bottom 8 pixels of the NES screen");

    // Attempt to parse the arguments
    int parseStatus;
    try {
        argParser.parse_args(argc, argv);
        parseStatus = 0;
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << argParser;
        parseStatus = 1;
    }    

    // Create and return options struct
    AppOptions outputOptions;
    outputOptions.parseStatus = parseStatus;

    outputOptions.romPath = argParser.get("romPath");
    outputOptions.palettePath = argParser.get("palettes");
    outputOptions.windowed = argParser.get<bool>("windowed");
    outputOptions.hideDangerZone = !argParser.get<bool>("show-overscan");
    outputOptions.useVsync = !argParser.get<bool>("no-vsync");

    return outputOptions;
}
