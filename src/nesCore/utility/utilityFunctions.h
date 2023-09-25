#ifndef NES_UTILITY_H_
#define NES_UTILITY_H_

#include "nesPch.h"

namespace nesCore {
namespace utility {

// Convert a number to a zero padded hex string
std::string paddedHex(int data, int nPadding);

} // utility
} // nesCore

#endif 
