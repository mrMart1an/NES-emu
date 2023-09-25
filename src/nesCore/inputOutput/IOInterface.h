#ifndef IO_INTERFACE_H_
#define IO_INTERFACE_H_

#include "nesPch.h"

namespace nesCore {
class IOInterface {
public:
    // Write on the output port
    virtual void writeOutput(uint8_t data) = 0;

    // Read data on the input port
    virtual uint8_t readInputOne() = 0;
    virtual uint8_t readInputTwo() = 0;
};
}

#endif
