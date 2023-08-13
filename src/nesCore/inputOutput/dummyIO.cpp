#include "dummyIO.h"
#include <cstdint>

namespace nesCore {
// Do nothing in the read and write functions
void DummyIO::writeOutput(uint8_t data) {};

uint8_t DummyIO::readInputOne() { return 0x00; };
uint8_t DummyIO::readInputTwo() { return 0x00; };
}
