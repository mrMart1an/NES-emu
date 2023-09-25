#include "nesPch.h"

#include "ppuDebug.h"

namespace nesCore {
namespace debug {

std::string PPUDebug::log() {
    std::stringstream s;

    s << "Scanline: " << (int)scanLine << ", " << (int)scanCycle;

    // Return string
    return s.str();
}

}
}
