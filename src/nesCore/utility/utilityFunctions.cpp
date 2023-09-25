#include "nesPch.h"

namespace nesCore {
namespace utility {

// Convert a number to a zero padded hex string
std::string paddedHex(int data, int nPadding) {
    std::stringstream s;

    s << "0x";
    s << std::setw(nPadding) << std::setfill('0') << std::uppercase << std::hex << data;

    return s.str();
}

}
}
