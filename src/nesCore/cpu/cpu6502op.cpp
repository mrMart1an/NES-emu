#include "cpu6502.h"
#include "../cpuBus.h"
#include <cstdint>
#include <iostream>

// Implement the CPU operations
namespace nesCore {
/*
 *
 *  Stack functions
 *
*/

// Push 1 byte to the stack
void Cpu6502::stackPush(uint8_t data) {
    // Write the data to the stack
    uint16_t sAddr = 0x100 + m_stackPointer;
    m_bus->write(sAddr, data);

    // Update the stack pointer
    m_stackPointer -= 1;
}
// Push 2 byte to the stack
void Cpu6502::stackPush16(uint16_t data) {
    // Write the data to the stack
    uint16_t sAddr = 0x0FF + m_stackPointer;
    m_bus->write16(sAddr, data);

    // Update the stack pointer
    m_stackPointer -= 2;;
}

// Pop 1 byte from the stack
uint8_t Cpu6502::stackPop() {
    // Update the stack pointer
    m_stackPointer += 1;

    return m_bus->read(0x100 + m_stackPointer);
}
// Pop 2 byte from the stack
uint16_t Cpu6502::stackPop16() {
    // Update the stack pointer
    m_stackPointer += 2;

    return m_bus->read16(0xFF + m_stackPointer);
}

// Push the accumulator to the stack
void Cpu6502::PHA() {
    this->stackPush(m_accumulator);
}
// Push the processor status to the stack
void Cpu6502::PHP() {
    this->stackPush(this->getStatusByte(true));
}

// Pull the accumulator from the stack
void Cpu6502::PLA() {
    m_accumulator = this->stackPop();
    
    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
// Pull the processor status from the stack
void Cpu6502::PLP() {
    uint8_t status = this->stackPop();

    // Update the status register
    m_carryFlag = static_cast<bool>(status & 0b00000001);
    m_zeroFlag = static_cast<bool>(status & 0b00000010);
    m_interruptDisable = static_cast<bool>(status & 0b00000100);
    m_decimalMode = static_cast<bool>(status & 0b00001000);
    m_overflowFlag = static_cast<bool>(status & 0b01000000);
    m_negativeFlag = static_cast<bool>(status & 0b10000000);
}

/*
*
*   Logical operations
*
*/

// Perform an AND operation on the accumulator with a byte from memory
// and update the status variable of the CPU
void Cpu6502::AND(uint16_t addr) {
    // Fetch operation and perform the operation
    uint8_t memValue = m_bus->read(addr);
    m_accumulator &= memValue;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
 
// Perform an EOR operation on the accumulator with a byte from memory
// and update the status variable of the CPU
void Cpu6502::EOR(uint16_t addr) {
    // Fetch operation and perform the operation
    uint8_t memValue = m_bus->read(addr);
    m_accumulator ^= memValue;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
 
// Perform an ORA operation on the accumulator with a byte from memory
// and update the status variable of the CPU
void Cpu6502::ORA(uint16_t addr) {
    // Fetch operation and perform the operation
    uint8_t memValue = m_bus->read(addr);
    m_accumulator |= memValue;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}

// Get a number from the bus and mask it with the accumulator 
// Check if bit 7 and 6 are set or if the number is 0
void Cpu6502::BIT(uint16_t addr) {
    // Fetch operation and perform the operation
    uint8_t memValue = m_bus->read(addr);

    // Update status registers
    m_zeroFlag = (memValue & m_accumulator) == 0x00;
    m_negativeFlag = (memValue & 0b10000000) != 0;
    m_overflowFlag = (memValue & 0b01000000) != 0;
}

/*
*
*   Shift operations
*
*/

// Add with carry operation
void Cpu6502::ADC(uint16_t addr) {
    // Add memory data, accumulator and carry
    uint8_t memValue = m_bus->read(addr);
    uint16_t result = static_cast<uint16_t>(memValue) + m_accumulator;
    result += static_cast<uint16_t>(m_carryFlag);

    // Save result to the accumulator and update the status flags 
    m_zeroFlag = (result & 0x00FF) == 0x00;
    m_carryFlag = (result & 0xFF00) != 0;
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_overflowFlag = ((memValue ^ result) & (m_accumulator ^ result) & 0b10000000) != 0;

    m_accumulator = result & 0x00FF;
}

// Add with carry operation
void Cpu6502::SBC(uint16_t addr) {
    // Subtract memory data, accumulator and carry
    uint8_t memValue = m_bus->read(addr);
    uint8_t accumulatorInput = m_accumulator;
    uint16_t result = m_accumulator - static_cast<uint16_t>(memValue);
    result -= static_cast<uint16_t>(!m_carryFlag);

    // Save result to the accumulator and update the status flags 
    m_accumulator = result & 0x00FF;
    m_zeroFlag = m_accumulator == 0x00;
    m_carryFlag = (result & 0xFF00) == 0;
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_overflowFlag = (((0xFF - memValue) ^ result) & (accumulatorInput ^ result) & 0b10000000) != 0;
}

// Compare the accumulator value with a memory value
void Cpu6502::CMP(uint16_t addr) {
    uint8_t memValue = m_bus->read(addr);
    uint8_t result = m_accumulator - static_cast<uint16_t>(memValue);

    // Update the flags
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_carryFlag = m_accumulator >= memValue;
    m_zeroFlag = m_accumulator == memValue;
}
// Compare the accumulator value with a memory value
void Cpu6502::CPX(uint16_t addr) {
    uint8_t memValue = m_bus->read(addr);
    uint8_t result = m_regX - static_cast<uint16_t>(memValue);

    // Update the flags
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_carryFlag = m_regX >= memValue;
    m_zeroFlag = m_regX == memValue;
}
// Compare the accumulator value with a memory value
void Cpu6502::CPY(uint16_t addr) {
    uint8_t memValue = m_bus->read(addr);
    uint8_t result = m_regY - static_cast<uint16_t>(memValue);

    // Update the flags
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_carryFlag = m_regY >= memValue;
    m_zeroFlag = m_regY == memValue;
}

/*
*
*   Shift operations
*
*/

// Shift left a byte of memory on the bus 
void Cpu6502::ASL(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t result = value << 1;
    m_bus->write(addr, result);

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (result & 0b10000000) != 0;
    m_zeroFlag = result == 0x00;
}
// Shift left a byte of memory in the accumulator 
void Cpu6502::ASL() {
    // Perform the shift
    uint8_t value = m_accumulator;
    m_accumulator = value << 1;

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Shift right a byte of memory on the bus 
void Cpu6502::LSR(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t result = value >> 1;
    m_bus->write(addr, result);

    // Update CPU status 
    m_carryFlag = (value & 0b00000001) != 0;
    m_negativeFlag = (result & 0b10000000) != 0;
    m_zeroFlag = result == 0x00;
}
// Shift right a byte of memory in the accumulator 
void Cpu6502::LSR() {
    // Perform the shift
    uint8_t value = m_accumulator;
    m_accumulator = value >> 1;

    // Update CPU status 
    m_carryFlag = (value & 0b00000001) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Roll left a byte of memory on the bus 
void Cpu6502::ROL(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t result = (value << 1) | static_cast<uint8_t>(m_carryFlag);
    m_bus->write(addr, result);

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (result & 0b10000000) != 0;
    m_zeroFlag = result == 0x00;
}
// Roll left a byte of memory in the accumulator 
void Cpu6502::ROL() {
    // Perform the shift
    uint8_t value = m_accumulator;
    m_accumulator = (value << 1) | static_cast<uint8_t>(m_carryFlag);

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Roll right a byte of memory on the bus 
void Cpu6502::ROR(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t result = (value >> 1) | (static_cast<uint8_t>(m_carryFlag) << 7);
    m_bus->write(addr, result);

    // Update CPU status 
    m_carryFlag = (value & 0b00000001) != 0;
    m_negativeFlag = (result & 0b10000000) != 0;
    m_zeroFlag = result == 0x00;
}
// Roll right a byte of memory in the accumulator 
void Cpu6502::ROR() {
    // Perform the shift
    uint8_t value = m_accumulator;
    m_accumulator = (value >> 1) | (static_cast<uint8_t>(m_carryFlag) << 7);

    // Update CPU status 
    m_carryFlag = (value & 0b00000001) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

/*
*
*   Load / store operations
*
*/

// Load a byte of memory into the accumulator and update the status flags
void Cpu6502::LDA(uint16_t addr) {
    m_accumulator = m_bus->read(addr);

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
// Load a byte of memory into the X register and update the status flags
void Cpu6502::LDX(uint16_t addr) {
    m_regX = m_bus->read(addr);

    // Update status registers
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Load a byte of memory into the Y register and update the status flags
void Cpu6502::LDY(uint16_t addr) {
    m_regY = m_bus->read(addr);

    // Update status registers
    m_zeroFlag = m_regY == 0x00;
    m_negativeFlag = (m_regY & 0b10000000) != 0;
}

// Save the value byte stored in the accumulator to an address of the bus
void Cpu6502::STA(uint16_t addr) {
    m_bus->write(addr, m_accumulator);
}
// Save the value byte stored in the X register to an address of the bus
void Cpu6502::STX(uint16_t addr) {
    m_bus->write(addr, m_regX);
}
// Save the value byte stored in the Y register to an address of the bus
void Cpu6502::STY(uint16_t addr) {
    m_bus->write(addr, m_regY);
}

/*
*
*   Branch instructions
*
*/

// Branch if carry flag is clear
void Cpu6502::BCC(int8_t offset) {
    if (!m_carryFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}
// Branch if carry flag is set
void Cpu6502::BCS(int8_t offset) {
    if (m_carryFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}

// Branch if negative flags is clear
void Cpu6502::BPL(int8_t offset) {
    if (!m_negativeFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}
// Branch if negative flags is set
void Cpu6502::BMI(int8_t offset) {
    if (m_negativeFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}

// Branch if zero flags is clear
void Cpu6502::BNE(int8_t offset) {
    if (!m_zeroFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}
// Branch if zero flags is set
void Cpu6502::BEQ(int8_t offset) {
    if (m_zeroFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}

// Branch if overflow flags is clear
void Cpu6502::BVC(int8_t offset) {
    if (!m_overflowFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}
// Branch if overflow flags is set
void Cpu6502::BVS(int8_t offset) {
    if (m_overflowFlag) {
        uint16_t address = m_pc;
        m_cpuCycle++;
        
        // Branch less page crossing check 
        bool pageCross = (((address & 0x00FF) + offset) & 0xFF00) != 0;
        m_cpuCycle += static_cast<uint64_t>(pageCross);

        m_pc = address + offset;
    }
}

/*
*
*   Register transfer operations
*
*/

// Copy the value in the accumulator to the register X
void Cpu6502::TAX() {
    m_regX = m_accumulator;

    // Update status registers
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Copy the value in the accumulator to the register Y
void Cpu6502::TAY() {
    m_regY = m_accumulator;

    // Update status registers
    m_zeroFlag = m_regY == 0x00;
    m_negativeFlag = (m_regY & 0b10000000) != 0;
}
// Copy the value in the register X in the accumulator
void Cpu6502::TXA() {
    m_accumulator = m_regX;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
// Copy the value in the register Y in the accumulator
void Cpu6502::TYA() {
    m_accumulator = m_regY;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}

// Copy the value in the stack pointer to the register X
void Cpu6502::TSX() {
    m_regX = m_stackPointer;

    // Update status registers
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Copy the value in the register X to the stack pointer
void Cpu6502::TXS() {
    m_stackPointer = m_regX;
}

/*
*
*   Increment and Decrements operations
*
*/

// Increment the value to the given memory address
// and update the status register
void Cpu6502::INC(uint16_t addr) {
    uint8_t result = m_bus->read(addr) + 1;
    m_bus->write(addr, result);

    // Update status register
    m_zeroFlag = result == 0x00;
    m_negativeFlag = (result & 0b10000000) != 0;
}
// Decrements the value to the given memory address
// and update the status register
void Cpu6502::DEC(uint16_t addr) {
    uint8_t result = m_bus->read(addr) - 1;
    m_bus->write(addr, result);

    // Update status register
    m_zeroFlag = result == 0x00;
    m_negativeFlag = (result & 0b10000000) != 0;
}

// Increment the value in the X register
// and update the status register
void Cpu6502::INX() {
    m_regX += 1;

    // Update status register
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Decrements the value in the X register
// and update the status register
void Cpu6502::DEX() {
    m_regX -= 1;

    // Update status register
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}

// Increment the value in the Y register
// and update the status register
void Cpu6502::INY() {
    m_regY += 1;

    // Update status register
    m_zeroFlag = m_regY == 0x00;
    m_negativeFlag = (m_regY & 0b10000000) != 0;
}
// Decrements the value in the Y register
// and update the status register
void Cpu6502::DEY() {
    m_regY -= 1;

    // Update status register
    m_zeroFlag = m_regY == 0x00;
    m_negativeFlag = (m_regY & 0b10000000) != 0;
}

/*
*
*   Status flags changes operations
*
*/

// Clear the carry flag
void Cpu6502::CLC() {
    m_carryFlag = false;
}
// Clear the decimal mode flag
void Cpu6502::CLD() {
    m_decimalMode = false;
}
// Clear the interrupt disable flag
void Cpu6502::CLI() {
    m_interruptDisable = false;
}
// Clear the overflow flag
void Cpu6502::CLV() {
    m_overflowFlag = false;
}

// Set the carry flag
void Cpu6502::SEC() {
    m_carryFlag = true;
}
// Set the decimal mode flag
void Cpu6502::SED() {
    m_decimalMode = true;
}
// Set the interrupt disable flag
void Cpu6502::SEI() {
    m_interruptDisable = true;
}

/*
 *
 *  interrupt functions
 *
*/

// Generate an software interrupt and set the PC to the IRQ vector
void Cpu6502::BRK() {
    // Push PC and status to stack
    this->stackPush16(m_pc + 1);
    this->stackPush(this->getStatusByte(true));

    // Set the interrupt disable flag 
    m_interruptDisable = true;

    m_pc = m_bus->read16(IRQ_BRK_VECTOR_ADDR);
}

// Return from an interrupt by pulling PC and CPU status from the stack
void Cpu6502::RTI() {
    // Pull status and the PC from stack
    this->PLP();
    m_pc = this->stackPop16();
}

/*
*
*   Jumps operations
*
*/

// Jumps to the given address by setting the PC to it
void Cpu6502::JMP(uint16_t addr) {
    m_pc = addr;
}

// Jumps to the given address and push to stack
// the current PC minus one
void Cpu6502::JSR(uint16_t addr) {
    this->stackPush16(m_pc - 1);
    m_pc = addr;
}

// Pull an address from the stack an set the PC to it
void Cpu6502::RTS() {
    uint16_t addr = this->stackPop16();
    m_pc = addr + 1;
}

/*
*
*   Illegal Op code operations
*
*/

// Load a byte of memory into the accumulator and register X and update the status flags
void Cpu6502::LAX(uint16_t addr) {
    uint8_t value = m_bus->read(addr);
    m_accumulator = value;
    m_regX = value;

    // Update status registers
    m_zeroFlag = value == 0x00;
    m_negativeFlag = (value & 0b10000000) != 0;
}

// Perform an AND operation between the accumulator and X register and store the result in memory
void Cpu6502::SAX(uint16_t addr) {
    m_bus->write(addr, m_accumulator & m_regX);
}

// Perform a decrement and compare operation
void Cpu6502::DCP(uint16_t addr) {
    // Get memory value and decrement it
    uint8_t memValue = m_bus->read(addr) - 1;
    m_bus->write(addr, memValue);

    // Compare instruction
    uint8_t cmpResult = m_accumulator - static_cast<uint16_t>(memValue);

    // Update the flags
    m_negativeFlag = (cmpResult & 0b10000000) != 0; 
    m_carryFlag = m_accumulator >= memValue;
    m_zeroFlag = m_accumulator == memValue;
}

// Perform an increment and then subtract the value in memory to the accumulator
void Cpu6502::ISC(uint16_t addr) {
    // Increment the memory value 
    // Subtract memory data, accumulator and carry
    uint8_t memValue = m_bus->read(addr) + 1;
    m_bus->write(addr, memValue);

    uint8_t accumulatorInput = m_accumulator;
    uint16_t result = m_accumulator - static_cast<uint16_t>(memValue);
    result -= static_cast<uint16_t>(!m_carryFlag);

    // Save result to the accumulator and update the status flags 
    m_accumulator = result & 0x00FF;
    m_zeroFlag = m_accumulator == 0x00;
    m_carryFlag = (result & 0xFF00) == 0;
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_overflowFlag = (((0xFF - memValue) ^ result) & (accumulatorInput ^ result) & 0b10000000) != 0;
}

// Shift left one bit in memory, then OR accumulator with memory
void Cpu6502::SLO(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t result = value << 1;
    m_bus->write(addr, result);

    // OR between result and accumulator
    m_accumulator = m_accumulator | result;

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Roll left one bit in memory, then AND accumulator with memory
void Cpu6502::RLA(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t result = (value << 1) | static_cast<uint8_t>(m_carryFlag);
    m_bus->write(addr, result);

    // AND between result and accumulator
    m_accumulator = m_accumulator & result;

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
}

// Shift right one bit in memory, then XOR accumulator with memory
void Cpu6502::SRE(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t result = value >> 1;
    m_bus->write(addr, result);

    // XOR between result and accumulator
    m_accumulator = m_accumulator ^ result;

    // Update CPU status 
    m_carryFlag = (value & 0b00000001) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Roll right one bit in memory, then XOR accumulator with memory
void Cpu6502::RRA(uint16_t addr) {
    // Perform the shift
    uint8_t value = m_bus->read(addr);
    uint8_t rollResult = (value >> 1) | (static_cast<uint8_t>(m_carryFlag) << 7);
    m_bus->write(addr, rollResult);

    // ADD between result and accumulator
    m_carryFlag = (value & 0b00000001) != 0;
    uint16_t result = static_cast<uint16_t>(rollResult) + m_accumulator;
    result += static_cast<uint16_t>(m_carryFlag);

    // Save result to the accumulator and update the status flags 
    m_zeroFlag = (result & 0x00FF) == 0x00;
    m_carryFlag = (result & 0xFF00) != 0;
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_overflowFlag = ((rollResult ^ result) & (m_accumulator ^ result) & 0b10000000) != 0;

    m_accumulator = result & 0x00FF;
}
}

