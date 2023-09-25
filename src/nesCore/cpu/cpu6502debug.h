#ifndef CPU6502_DEBUG_H_
#define CPU6502_DEBUG_H_

#include "nesPch.h"

#include "cpu6502.h"

namespace nesCore {
namespace debug {

// Decompile a 6502 instruction
// Take a pointer to a bus and an address
std::string decompileInstruction(Bus* bus, uint16_t addr);

// Struct to hold the debug information of the CPU
struct Cpu6502Debug {
    // CPU cycle since the last reset
    uint64_t cpuCycle;

    // Program pointer
    uint16_t pc;
    // Stack pointer
    uint8_t stackPointer;
    // CPU registers
    uint8_t regX, regY, accumulator;
    
    // Status registers
    uint8_t statusByte;

    bool carryFlag;
    bool zeroFlag;
    bool overflowFlag;
    bool negativeFlag;

    bool interruptDisable;
    bool decimalMode; 
    bool breakCommand;

    // Formatting methods
    std::string formatStatusRegister();
    std::string formatGeneralRegisters();
    std::string fomratProgramRegisters();
    std::string formatCycles();

    // Format expended information 
    std::string format();
    // Format information in one line
    std::string log();
};

}
}

#endif
