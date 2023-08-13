#ifndef DUMMY_IO_H_
#define DUMMY_IO_H_

#include "IOInterface.h"
#include <cstdint>

namespace nesCore {
class DummyIO: public IOInterface {
public:
    // Write on the output port
    void writeOutput(uint8_t data) override;

    // Read data on the input port
    uint8_t readInputOne() override;
    uint8_t readInputTwo() override;
};
}

#endif
