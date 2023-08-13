#include <cstdint>
#include <sstream>
#include <string>
#include "cpu6502debug.h"
#include "../cpuBus.h"
#include "../utility/utilityFunctions.h"

namespace nesCore {
namespace debug {

// Return a string with the formatted status registers
std::string Cpu6502Debug::formatStatusRegister() {
    std::stringstream s;
    // Top line
    s << "NV-BDIZC" << std::endl;

    // Register status
    s << static_cast<int>(negativeFlag);
    s << static_cast<int>(overflowFlag);
    s << "-";
    s << static_cast<int>(breakCommand);
    s << static_cast<int>(decimalMode);
    s << static_cast<int>(interruptDisable);
    s << static_cast<int>(zeroFlag);
    s << static_cast<int>(carryFlag);

    // print the status byte
    s << " (" << utility::paddedHex(statusByte, 2) << ")";

    // End line and return string
    s << std::endl;
    return s.str();
}

// Return a string with the formatted general purpose registers
std::string Cpu6502Debug::formatGeneralRegisters() {
    std::stringstream s;
    // Accumulator
    s << "A: " << utility::paddedHex(accumulator, 2);
    // X and Y registers
    s << ", X: " << utility::paddedHex(regX, 2);
    s << ", Y: " << utility::paddedHex(regY, 2);

    // End line and return string
    s << std::endl;
    return s.str();
}

// Return a string with the formatted stack pointer and program counter
std::string Cpu6502Debug::fomratProgramRegisters() {
    std::stringstream s;

    // Program counter
    s << "PC: " << utility::paddedHex(pc, 4);
    // Stack pointer
    s << ", SP: " << utility::paddedHex(stackPointer, 2);

    // End line and return string
    s << std::endl;
    return s.str();
}

// Return a string with the CPU cycles counter
std::string Cpu6502Debug::formatCycles() {
    std::stringstream s;

    s << "Current CPU cycle: " << cpuCycle;

    // End line and return string
    s << std::endl;
    return s.str();
}

// Return a string with the complete debug information
std::string Cpu6502Debug::format() {
    std::stringstream s;

    s << std::endl << "6502 debug information" << std::endl;
    s << "Registers:" << std::endl;
    s << this->fomratProgramRegisters();
    s << this->formatGeneralRegisters() << std::endl;

    s << "CPU status:" << std::endl;
    s << this->formatStatusRegister() << std::endl;

    s << this->formatCycles() << std::endl;
        
    // Return string
    return s.str();
        
    // Return string
    return s.str();
}

// Return the information formatted in one line
std::string Cpu6502Debug::log() {
    std::stringstream s;

    // Registers
    s << "PC: " << utility::paddedHex(pc, 4); 
    s << ", SP: " << utility::paddedHex(stackPointer, 2);
    s << " | ";
    s << "A: " << utility::paddedHex(accumulator, 2);
    s << ", X: " << utility::paddedHex(regX, 2);
    s << ", Y: " << utility::paddedHex(regY, 2);
    s << " | ";
    
    // Status
    s << "NV-BDIZC: ";
    s << static_cast<int>(negativeFlag);
    s << static_cast<int>(overflowFlag);
    s << "-";
    s << static_cast<int>(breakCommand);
    s << static_cast<int>(decimalMode);
    s << static_cast<int>(interruptDisable);
    s << static_cast<int>(zeroFlag);
    s << static_cast<int>(carryFlag);
    
    // print the status byte
    s << " (" << utility::paddedHex(statusByte, 2) << ")";
    s << " | ";

    // Cycle counter
    s << "Cycle: " << cpuCycle;

    // Return string
    return s.str();
}

// Format immediate addressing
std::string formatImmediateAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;
    s << "#" << utility::paddedHex(bus->read(addr, true), 2);

    // Return string
    return s.str();
}

// Format absolute addressing
std::string formatAbsoluteAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint16_t absoluteAddr = bus->read16(addr, true);
    uint8_t value = bus->read(absoluteAddr, true);

    s << utility::paddedHex(absoluteAddr, 4);
    s << " = ";
    s << utility::paddedHex(value, 2);

    // Return string
    return s.str();
}
// Format absolute addressing
std::string formatAbsoluteXAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint16_t absoluteAddr = bus->read16(addr, true);
    s << utility::paddedHex(absoluteAddr, 4) << ", X";

    // Return string
    return s.str();
}
// Format absolute addressing
std::string formatAbsoluteYAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint16_t absoluteAddr = bus->read16(addr, true);
    s <<  utility::paddedHex(absoluteAddr, 4) << ", Y";

    // Return string
    return s.str();
}

// Format zero page addressing
std::string formatZeroPageAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint8_t zeroPageAddr = bus->read(addr, true);
    uint8_t value = bus->read(zeroPageAddr, true);

    s << utility::paddedHex(zeroPageAddr, 2);
    s << " = ";
    s << utility::paddedHex(value, 2);

    // Return string
    return s.str();
}
// Format zero page addressing
std::string formatZeroPageXAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint8_t zeroPageAddr = bus->read(addr, true);
    s << utility::paddedHex(zeroPageAddr, 2) << ", X";

    // Return string
    return s.str();
}
// Format zero page addressing
std::string formatZeroPageYAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint8_t zeroPageAddr = bus->read(addr, true);
    s << utility::paddedHex(zeroPageAddr, 2) << ", Y";

    // Return string
    return s.str();
}


// Format indexed indirect addressing
std::string formatIndexedIndirectAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint8_t indirectAddr = bus->read(addr, true);
    s << "(" << utility::paddedHex(indirectAddr, 2) << ", X)";

    // Return string
    return s.str();
}
// Format indirect indexed addressing
std::string formatIndirectIndexedAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint8_t indirectAddr = bus->read(addr, true);
    s << "(" << utility::paddedHex(indirectAddr, 2) << "), Y";

    // Return string
    return s.str();
}

// Format relative addressing
std::string formatRelative(Bus* bus, uint16_t addr) {
    std::stringstream s;

    int8_t offset = static_cast<int8_t>(bus->read(addr));
    s << utility::paddedHex(addr + offset + 1, 4);

    // Return string
    return s.str();
}
// Format indirect addressing
std::string formatIndirectAddr(Bus* bus, uint16_t addr) {
    std::stringstream s;

    uint16_t indirectAddr = bus->read16(addr, true);
    uint16_t value = bus->read16(indirectAddr, true);

    s << utility::paddedHex(indirectAddr, 4);
    s << " = ";
    s << utility::paddedHex(value, 4);

    // Return string
    return s.str();
}

// Decompile an instruction
std::string decompileInstruction(Bus *bus, uint16_t addr){
    std::stringstream s;

    // Read the op code and increment the address
    uint8_t opCode = bus->read(addr, true);
    addr++;
    
    // Decompile instruction
    switch (opCode) {
        // Loads instructions
        //
        // LDA instruction
        case 0xA9:
            s << "LDA " << formatImmediateAddr(bus, addr); break;
        case 0xA5:
            s << "LDA " << formatZeroPageAddr(bus, addr); break;
        case 0xB5:
            s << "LDA " << formatZeroPageXAddr(bus, addr); break;
        case 0xAD:
            s << "LDA " << formatAbsoluteAddr(bus, addr); break;
        case 0xBD:
            s << "LDA " << formatAbsoluteXAddr(bus, addr); break;
        case 0xB9:
            s << "LDA " << formatAbsoluteYAddr(bus, addr); break;
        case 0xA1:
            s << "LDA " << formatIndexedIndirectAddr(bus, addr); break;
        case 0xB1:
            s << "LDA " << formatIndirectIndexedAddr(bus, addr); break;

        // LDX instruction
        case 0xA2:
            s << "LDX " << formatImmediateAddr(bus, addr); break;
        case 0xA6:
            s << "LDX " << formatZeroPageAddr(bus, addr); break;
        case 0xB6:
            s << "LDX " << formatZeroPageYAddr(bus, addr); break;
        case 0xAE:
            s << "LDX " << formatAbsoluteAddr(bus, addr); break;
        case 0xBE:
            s << "LDX " << formatAbsoluteYAddr(bus, addr); break;

        // LDY instruction
        case 0xA0:
            s << "LDY " << formatImmediateAddr(bus, addr); break;
        case 0xA4:
            s << "LDY " << formatZeroPageAddr(bus, addr); break;
        case 0xB4:
            s << "LDY " << formatZeroPageXAddr(bus, addr); break;
        case 0xAC:
            s << "LDY " << formatAbsoluteAddr(bus, addr); break;
        case 0xBC:
            s << "LDY " << formatAbsoluteXAddr(bus, addr); break;

        // Store instructions
        //
        // STA instruction
        case 0x85:
            s << "STA " << formatZeroPageAddr(bus, addr); break;
        case 0x95:
            s << "STA " << formatZeroPageXAddr(bus, addr); break;
        case 0x8D:
            s << "STA " << formatAbsoluteAddr(bus, addr); break;
        case 0x9D:
            s << "STA " << formatAbsoluteXAddr(bus, addr); break;
        case 0x99:
            s << "STA " << formatAbsoluteYAddr(bus, addr); break;
        case 0x81:
            s << "STA " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x91:
            s << "STA " << formatIndirectIndexedAddr(bus, addr); break;

        // STX instruction
        case 0x86:
            s << "STX " << formatZeroPageAddr(bus, addr); break;
        case 0x96:
            s << "STX " << formatZeroPageYAddr(bus, addr); break;
        case 0x8E:
            s << "STX " << formatAbsoluteAddr(bus, addr); break;

        // STY instruction
        case 0x84:
            s << "STY " << formatZeroPageAddr(bus, addr); break;
        case 0x94:
            s << "STY " << formatZeroPageXAddr(bus, addr); break;
        case 0x8C:
            s << "STY " << formatAbsoluteAddr(bus, addr); break;

        // Registers transfer instructions
        //
        // General purpose registers
        case 0xAA:
            s << "TAX"; break;
        case 0xA8:
            s << "TAY"; break;
        case 0x8A:
            s << "TXA"; break;
        case 0x98:
            s << "TYA"; break;
        // Stack pointer register 
        case 0xBA:
            s << "TSX"; break;
        case 0x9A:
            s << "TXS"; break;

        // Increment / decrements instructions
        //
        // INC instruction
        case 0xE6:
            s << "INC " << formatZeroPageAddr(bus, addr); break;
        case 0xF6:
            s << "INC " << formatZeroPageXAddr(bus, addr); break;
        case 0xEE:
            s << "INC " << formatAbsoluteAddr(bus, addr); break;
        case 0xFE:
            s << "INC " << formatAbsoluteXAddr(bus, addr); break;
        // DEC instruction
        case 0xC6:
            s << "DEC " << formatZeroPageAddr(bus, addr); break;
        case 0xD6:
            s << "DEC " << formatZeroPageXAddr(bus, addr); break;
        case 0xCE:
            s << "DEC " << formatAbsoluteAddr(bus, addr); break;
        case 0xDE:
            s << "DEC " << formatAbsoluteXAddr(bus, addr); break;
        // Registers increment / decrements instructions
        case 0xE8:
            s << "INX"; break;
        case 0xC8:
            s << "INY"; break;
        case 0xCA:
            s << "DEX"; break;
        case 0x88:
            s << "DEY"; break;

        // Update status flag instructions
        //
        // Clear instructions
        case 0x18:
            s << "CLC"; break;
        case 0xD8:
            s << "CLD"; break;
        case 0x58:
            s << "CLI"; break;
        case 0xB8:
            s << "CLV"; break;
        // Set instructions
        case 0x38:
            s << "SEC"; break;
        case 0xF8:
            s << "SED"; break;
        case 0x78:
            s << "SEI"; break;

        // Branch instructions
        case 0x90:
            s << "BCC " << formatRelative(bus, addr); break;
        case 0xB0:
            s << "BCS " << formatRelative(bus, addr); break;
        case 0xF0:
            s << "BEQ " << formatRelative(bus, addr); break;
        case 0x30:
            s << "BMI " << formatRelative(bus, addr); break;
        case 0xD0:
            s << "BNE " << formatRelative(bus, addr); break;
        case 0x10:
            s << "BPL " << formatRelative(bus, addr); break;
        case 0x50:
            s << "BVC " << formatRelative(bus, addr); break;
        case 0x70:
            s << "BVS " << formatRelative(bus, addr); break;

        // Jumps instructions
        //
        // JMP instruction
        case 0x4C:
            s << "JMP " << formatAbsoluteAddr(bus, addr); break;
        case 0x6C:
            s << "JMP " << formatIndirectAddr(bus, addr); break;
        // Subroutine instructions
        case 0x20:
            s << "JSR " << formatAbsoluteAddr(bus, addr); break;
        case 0x60:
            s << "RTS"; break;

        // Arithmetic instructions
        //
        // ADC instruction
        case 0x69:
            s << "ADC " << formatImmediateAddr(bus, addr); break;
        case 0x65:
            s << "ADC " << formatZeroPageAddr(bus, addr); break;
        case 0x75:
            s << "ADC " << formatZeroPageXAddr(bus, addr); break;
        case 0x6D:
            s << "ADC " << formatAbsoluteAddr(bus, addr); break;
        case 0x7D:
            s << "ADC " << formatAbsoluteXAddr(bus, addr); break;
        case 0x79:
            s << "ADC " << formatAbsoluteYAddr(bus, addr); break;
        case 0x61:
            s << "ADC " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x71:
            s << "ADC " << formatIndirectIndexedAddr(bus, addr); break;
        // SBC instruction
        case 0xE9:
            s << "SBC " << formatImmediateAddr(bus, addr); break;
        case 0xE5:
            s << "SBC " << formatZeroPageAddr(bus, addr); break;
        case 0xF5:
            s << "SBC " << formatZeroPageXAddr(bus, addr); break;
        case 0xED:
            s << "SBC " << formatAbsoluteAddr(bus, addr); break;
        case 0xFD:
            s << "SBC " << formatAbsoluteXAddr(bus, addr); break;
        case 0xF9:
            s << "SBC " << formatAbsoluteYAddr(bus, addr); break;
        case 0xE1:
            s << "SBC " << formatIndexedIndirectAddr(bus, addr); break;
        case 0xF1:
            s << "SBC " << formatIndirectIndexedAddr(bus, addr); break;
        // CMP instruction
        case 0xC9:
            s << "CMP " << formatImmediateAddr(bus, addr); break;
        case 0xC5:
            s << "CMP " << formatZeroPageAddr(bus, addr); break;
        case 0xD5:
            s << "CMP " << formatZeroPageXAddr(bus, addr); break;
        case 0xCD:
            s << "CMP " << formatAbsoluteAddr(bus, addr); break;
        case 0xDD:
            s << "CMP " << formatAbsoluteXAddr(bus, addr); break;
        case 0xD9:
            s << "CMP " << formatAbsoluteYAddr(bus, addr); break;
        case 0xC1:
            s << "CMP " << formatIndexedIndirectAddr(bus, addr); break;
        case 0xD1:
            s << "CMP " << formatIndirectIndexedAddr(bus, addr); break;
        // CPX instruction
        case 0xE0:
            s << "CPX " << formatImmediateAddr(bus, addr); break;
        case 0xE4:  
            s << "CPX " << formatZeroPageAddr(bus, addr); break;
        case 0xEC:  
            s << "CPX " << formatAbsoluteAddr(bus, addr); break;
        // CPY instruction
        case 0xC0:
            s << "CPY " << formatImmediateAddr(bus, addr); break;
        case 0xC4:  
            s << "CPY " << formatZeroPageAddr(bus, addr); break;
        case 0xCC:  
            s << "CPY " << formatAbsoluteAddr(bus, addr); break;

        // Logical instructions
        //
        // AND instruction
        case 0x29:
            s << "AND " << formatImmediateAddr(bus, addr); break;
        case 0x25:
            s << "AND " << formatZeroPageAddr(bus, addr); break;
        case 0x35:
            s << "AND " << formatZeroPageXAddr(bus, addr); break;
        case 0x2D:
            s << "AND " << formatAbsoluteAddr(bus, addr); break;
        case 0x3D:
            s << "AND " << formatAbsoluteXAddr(bus, addr); break;
        case 0x39:
            s << "AND " << formatAbsoluteYAddr(bus, addr); break;
        case 0x21:
            s << "AND " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x31:
            s << "AND " << formatIndirectIndexedAddr(bus, addr); break;
        // EOR instruction
        case 0x49:
            s << "EOR " << formatImmediateAddr(bus, addr); break;
        case 0x45:
            s << "EOR " << formatZeroPageAddr(bus, addr); break;
        case 0x55:
            s << "EOR " << formatZeroPageXAddr(bus, addr); break;
        case 0x4D:
            s << "EOR " << formatAbsoluteAddr(bus, addr); break;
        case 0x5D:
            s << "EOR " << formatAbsoluteXAddr(bus, addr); break;
        case 0x59:
            s << "EOR " << formatAbsoluteYAddr(bus, addr); break;
        case 0x41:
            s << "EOR " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x51:
            s << "EOR " << formatIndirectIndexedAddr(bus, addr); break;
        // ORA instruction
        case 0x09:
            s << "ORA " << formatImmediateAddr(bus, addr); break;
        case 0x05:
            s << "ORA " << formatZeroPageAddr(bus, addr); break;
        case 0x15:
            s << "ORA " << formatZeroPageXAddr(bus, addr); break;
        case 0x0D:
            s << "ORA " << formatAbsoluteAddr(bus, addr); break;
        case 0x1D:
            s << "ORA " << formatAbsoluteXAddr(bus, addr); break;
        case 0x19:
            s << "ORA " << formatAbsoluteYAddr(bus, addr); break;
        case 0x01:
            s << "ORA " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x11:
            s << "ORA " << formatIndirectIndexedAddr(bus, addr); break;
        // BIT instruction
        case 0x24:
            s << "BIT " << formatZeroPageAddr(bus, addr); break;
        case 0x2C:
            s << "BIT " << formatAbsoluteAddr(bus, addr); break;

        // Shift instructions
        //
        // ASL instruction
        case 0x0A:
            s << "ASL"; break;
        case 0x06:
            s << "ASL " << formatZeroPageAddr(bus, addr); break;
        case 0x16:
            s << "ASL " << formatZeroPageXAddr(bus, addr); break;
        case 0x0E:
            s << "ASL " << formatAbsoluteAddr(bus, addr); break;
        case 0x1E:
            s << "ASL " << formatAbsoluteXAddr(bus, addr); break;
        // LSR instruction
        case 0x4A:
            s << "LSR"; break;
        case 0x46:
            s << "LSR " << formatZeroPageAddr(bus, addr); break;
        case 0x56:
            s << "LSR " << formatZeroPageXAddr(bus, addr); break;
        case 0x4E:
            s << "LSR " << formatAbsoluteAddr(bus, addr); break;
        case 0x5E:
            s << "LSR " << formatAbsoluteXAddr(bus, addr); break;
        // ROL instruction
        case 0x2A:
            s << "ROL"; break;
        case 0x26:
            s << "ROL " << formatZeroPageAddr(bus, addr); break;
        case 0x36:
            s << "ROL " << formatZeroPageXAddr(bus, addr); break;
        case 0x2E:
            s << "ROL " << formatAbsoluteAddr(bus, addr); break;
        case 0x3E:
            s << "ROL " << formatAbsoluteXAddr(bus, addr); break;
        // ROR instruction
        case 0x6A:
            s << "ROR"; break;
        case 0x66:
            s << "ROR " << formatZeroPageAddr(bus, addr); break;
        case 0x76:
            s << "ROR " << formatZeroPageXAddr(bus, addr); break;
        case 0x6E:
            s << "ROR " << formatAbsoluteAddr(bus, addr); break;
        case 0x7E:
            s << "ROR " << formatAbsoluteXAddr(bus, addr); break;

        // Stack instructions
        case 0x48:
            s << "PHA"; break;
        case 0x08:
            s << "PHP"; break;
        case 0x68:
            s << "PLA"; break;
        case 0x28:
            s << "PLP"; break;

        // Interrupt instructions
        case 0x40:
            s << "RTI"; break;
        case 0x00:
            s << "BRK"; break;

        // NOP instruction
        case 0xEA:
            s << "NOP"; break;

        // Illegal instructions
        //
        // LAX instruction
        case 0xA7:
            s << "*LAX " << formatZeroPageAddr(bus, addr); break;
        case 0xB7:
            s << "*LAX " << formatZeroPageYAddr(bus, addr); break;
        case 0xAF:
            s << "*LAX " << formatAbsoluteAddr(bus, addr); break;
        case 0xBF:
            s << "*LAX " << formatAbsoluteYAddr(bus, addr); break;
        case 0xA3:
            s << "*LAX " << formatIndexedIndirectAddr(bus, addr); break;
        case 0xB3:
            s << "*LAX " << formatIndirectIndexedAddr(bus, addr); break;

        // SAX instruction
        case 0x87:
            s << "*SAX " << formatZeroPageAddr(bus, addr); break;
        case 0x97:
            s << "*SAX " << formatZeroPageYAddr(bus, addr); break;
        case 0x8F:
            s << "*SAX " << formatAbsoluteAddr(bus, addr); break;
        case 0x83:
            s << "*SAX " << formatIndexedIndirectAddr(bus, addr); break;

        // SBC + NOP instruction
        case 0xEB:
            s << "*SBC " << formatImmediateAddr(bus, addr); break;

        // DCP instruction
        case 0xC7:
            s << "*DCP " << formatZeroPageAddr(bus, addr); break;
        case 0xD7:
            s << "*DCP " << formatZeroPageXAddr(bus, addr); break;
        case 0xCF:
            s << "*DCP " << formatAbsoluteAddr(bus, addr); break;
        case 0xDF:
            s << "*DCP " << formatAbsoluteXAddr(bus, addr); break;
        case 0xDB:
            s << "*DCP " << formatAbsoluteYAddr(bus, addr); break;
        case 0xC3:
            s << "*DCP " << formatIndexedIndirectAddr(bus, addr); break;
        case 0xD3:
            s << "*DCP " << formatIndirectIndexedAddr(bus, addr); break;
        
        // ISC instruction
        case 0xE7:
            s << "*ISC " << formatZeroPageAddr(bus, addr); break;
        case 0xF7:
            s << "*ISC " << formatZeroPageXAddr(bus, addr); break;
        case 0xEF:
            s << "*ISC " << formatAbsoluteAddr(bus, addr); break;
        case 0xFF:
            s << "*ISC " << formatAbsoluteXAddr(bus, addr); break;
        case 0xFB:
            s << "*ISC " << formatAbsoluteYAddr(bus, addr); break;
        case 0xE3:
            s << "*ISC " << formatIndexedIndirectAddr(bus, addr); break;
        case 0xF3:
            s << "*ISC " << formatIndirectIndexedAddr(bus, addr); break;
        
        // SLO instruction
        case 0x07:
            s << "*SLO " << formatZeroPageAddr(bus, addr); break;
        case 0x17:
            s << "*SLO " << formatZeroPageXAddr(bus, addr); break;
        case 0x0F:
            s << "*SLO " << formatAbsoluteAddr(bus, addr); break;
        case 0x1F:
            s << "*SLO " << formatAbsoluteXAddr(bus, addr); break;
        case 0x1B:
            s << "*SLO " << formatAbsoluteYAddr(bus, addr); break;
        case 0x03:
            s << "*SLO " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x13:
            s << "*SLO " << formatIndirectIndexedAddr(bus, addr); break;

        // RLA instruction
        case 0x27:
            s << "*RLA " << formatZeroPageAddr(bus, addr); break;
        case 0x37:
            s << "*RLA " << formatZeroPageXAddr(bus, addr); break;
        case 0x2F:
            s << "*RLA " << formatAbsoluteAddr(bus, addr); break;
        case 0x3F:
            s << "*RLA " << formatAbsoluteXAddr(bus, addr); break;
        case 0x3B:
            s << "*RLA " << formatAbsoluteYAddr(bus, addr); break;
        case 0x23:
            s << "*RLA " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x33:
            s << "*RLA " << formatIndirectIndexedAddr(bus, addr); break;
         
        // SRE instruction
        case 0x47:
            s << "*SRE " << formatZeroPageAddr(bus, addr); break;
        case 0x57:
            s << "*SRE " << formatZeroPageXAddr(bus, addr); break;
        case 0x4F:
            s << "*SRE " << formatAbsoluteAddr(bus, addr); break;
        case 0x5F:
            s << "*SRE " << formatAbsoluteXAddr(bus, addr); break;
        case 0x5B:
            s << "*SRE " << formatAbsoluteYAddr(bus, addr); break;
        case 0x43:
            s << "*SRE " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x53:
            s << "*SRE " << formatIndirectIndexedAddr(bus, addr); break;   

        // RRA instruction
        case 0x67:
            s << "*RRA " << formatZeroPageAddr(bus, addr); break;
        case 0x77:
            s << "*RRA " << formatZeroPageXAddr(bus, addr); break;
        case 0x6F:
            s << "*RRA " << formatAbsoluteAddr(bus, addr); break;
        case 0x7F:
            s << "*RRA " << formatAbsoluteXAddr(bus, addr); break;
        case 0x7B:
            s << "*RRA " << formatAbsoluteYAddr(bus, addr); break;
        case 0x63:
            s << "*RRA " << formatIndexedIndirectAddr(bus, addr); break;
        case 0x73:
            s << "*RRA " << formatIndirectIndexedAddr(bus, addr); break;  

        // NOPs instructions
        // NOPs with implied addressing
        case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: 
            s << "*NOP"; break;
        // NOPs with Immediate addressing
        case 0x80: case 0x82: case 0x89: case 0xC2: case 0xE2: 
            s << "*NOP"; break;
        // NOPs with zero page addressing
        case 0x04: case 0x44: case 0x64: 
            s << "*NOP"; break;
        // NOPs with zero page X addressing
        case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4:
            s << "*NOP"; break;
        // NOP with absolute addressing
        case 0x0C:
            s << "*NOP"; break;
        // NOPs with absolute X addressing
        case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: 
            s << "*NOP"; break;

        // JAM instructions (Stop execution and halt the CPU)
        case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: 
        case 0x62: case 0x72: case 0x92: case 0xB2: case 0xD2: case 0xF2: 
            s << "*JAM"; break;

        default:
            s << "*UNKNOW*"; break;
        
    }

    // Return the formatted string
    return s.str();
}

}
}
