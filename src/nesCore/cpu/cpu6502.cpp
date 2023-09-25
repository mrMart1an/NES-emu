#include "nesPch.h"

#include "cpu6502debug.h"
#include "nesCore/utility/utilityFunctions.h"
#include "cpu6502.h"
#include "nesCore/cpuBus.h"

namespace nesCore {
// CPU constructor
Cpu6502::Cpu6502(Bus* bus) : m_bus(bus) {
    // Initialize general purpose registers
    m_regX = 0;
    m_regY = 0;
    m_accumulator = 0;

    // Initialize status registers
    m_carryFlag = false;
    m_zeroFlag = false;
    m_interruptDisable = true;
    m_decimalMode = false; 
    m_overflowFlag = false;
    m_negativeFlag = false;
};

// Reset all the CPU register
void Cpu6502::reset() {
    // Reset the cycle counter
    m_cpuCycle = 8;

    // Reset program counter and stack pointer
    m_pc = m_bus->read16(RESET_VECTOR_ADDR);
    m_stackPointer = 0xFD;

    // Reset status registers
    m_interruptDisable = true;
}

// Return a copy of the status of the CPU
debug::Cpu6502Debug Cpu6502::getDebugInfo() {
    debug::Cpu6502Debug output;

    // Copy the CPU status
    output.cpuCycle = m_cpuCycle;

    output.regX = m_regX;
    output.regY = m_regY;
    output.accumulator = m_accumulator;

    output.pc = m_pc;
    output.stackPointer = m_stackPointer;

    output.carryFlag = m_carryFlag;
    output.zeroFlag = m_zeroFlag;
    output.overflowFlag = m_overflowFlag;
    output.negativeFlag = m_negativeFlag;

    output.interruptDisable = m_interruptDisable;
    output.decimalMode = m_decimalMode; 
    output.breakCommand = false;

    output.statusByte = this->getStatusByte(false);

    return output;
}

// Convert the status register to a byte using the format of the original 6502
uint8_t Cpu6502::getStatusByte(bool bFlag) {
    uint8_t output = 0;

    // Set the status bits to the right value
    output |= static_cast<uint8_t>(m_carryFlag);
    output |= static_cast<uint8_t>(m_zeroFlag) << 1;
    output |= static_cast<uint8_t>(m_interruptDisable) << 2;
    output |= static_cast<uint8_t>(m_decimalMode) << 3;
    output |= static_cast<uint8_t>(bFlag) << 4;
    // Bit 5 is always 1
    output |= 0b00100000;
    output |= static_cast<uint8_t>(m_overflowFlag) << 6;
    output |= static_cast<uint8_t>(m_negativeFlag) << 7;

    return output;
}

// Set the CPU program counter at the given address
void Cpu6502::setProgramCounter(uint16_t addr) {
    m_pc = addr;
}

// Get the interrupt and return the interrupt to execute
// based on the disable interrupt flag
Interrupt6502 Cpu6502::pollInterrupt(Interrupt6502 interrupt) {
    return static_cast<Interrupt6502>(
        interrupt & (static_cast<int>(!m_interruptDisable) | NMI)
    );
}

// Execute the interrupt if interrupt status contain an interrupt
// This functions update the CPU cycles counter
void Cpu6502::executeInterrupt(Interrupt6502 interrupt) {
    // If a non-maskable interrupt occur with a maskable one 
    // execute both but give the non maskable priority
    if ((interrupt & IRQ) != 0) {
        m_cpuCycle += 7;

        // Push the CPU status and the PC to the stack
        this->stackPush16(m_pc);
        this->stackPush(this->getStatusByte(false));

        // Set the PC to the new address and update the I disable flag
        m_interruptDisable = true;
        m_pc = m_bus->read16(IRQ_BRK_VECTOR_ADDR);       
    } 

    if ((interrupt & NMI) != 0) {
        m_cpuCycle += 7;

        // Push the CPU status and the PC to the stack
        this->stackPush16(m_pc);
        this->stackPush(this->getStatusByte(false));

        // Set the PC to the new address and update the I disable flag
        m_interruptDisable = true;
        m_pc = m_bus->read16(NMI_VECTOR_ADDR);       
    }
}

// Execute an instruction and return the number of cycle required
size_t Cpu6502::step(Interrupt6502 interrupt) {
    uint64_t startCycles = m_cpuCycle;

    // Poll the interrupt for the next instruction
    // the polling need to be execute before the instruction execution
    // execution to avoid problems with instructions that set the interrupt disable flags
    interrupt = pollInterrupt(interrupt);

    // Read OP code from the buffer and increment the program counter
    uint8_t opCode = m_bus->read(m_pc);
    m_pc++;

    // Execute instruction
    switch (opCode) {
        // Loads instructions
        //
        // LDA instruction
        case 0xA9:
            this->LDA(this->getImmediateAddr()); m_cpuCycle += 2; break;
        case 0xA5:
            this->LDA(this->getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xB5:
            this->LDA(this->getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0xAD:
            this->LDA(this->getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0xBD:
            this->LDA(this->getAbsoluteXAddr(true)); m_cpuCycle += 4; break;
        case 0xB9:
            this->LDA(this->getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0xA1:
            this->LDA(this->getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0xB1:
            this->LDA(this->getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;

        // LDX instruction
        case 0xA2:
            this->LDX(this->getImmediateAddr()); m_cpuCycle += 2; break;
        case 0xA6:
            this->LDX(this->getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xB6:
            this->LDX(this->getZeroPageYAddr()); m_cpuCycle += 4; break;
        case 0xAE:
            this->LDX(this->getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0xBE:
            this->LDX(this->getAbsoluteYAddr(true)); m_cpuCycle += 4; break;

        // LDY instruction
        case 0xA0:
            this->LDY(this->getImmediateAddr()); m_cpuCycle += 2; break;
        case 0xA4:
            this->LDY(this->getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xB4:
            this->LDY(this->getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0xAC:
            this->LDY(this->getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0xBC:
            this->LDY(this->getAbsoluteXAddr(true)); m_cpuCycle += 4; break;

        // Store instructions
        //
        // STA instruction
        case 0x85:
            this->STA(this->getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x95:
            this->STA(this->getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0x8D:
            this->STA(this->getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0x9D:
            this->STA(this->getAbsoluteXAddr()); m_cpuCycle += 5; break;
        case 0x99:
            this->STA(this->getAbsoluteYAddr()); m_cpuCycle += 5; break;
        case 0x81:
            this->STA(this->getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0x91:
            this->STA(this->getIndirectIndexedAddr()); m_cpuCycle += 6; break;

        // STX instruction
        case 0x86:
            this->STX(this->getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x96:
            this->STX(this->getZeroPageYAddr()); m_cpuCycle += 4; break;
        case 0x8E:
            this->STX(this->getAbsoluteAddr()); m_cpuCycle += 4; break;

        // STY instruction
        case 0x84:
            this->STY(this->getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x94:
            this->STY(this->getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0x8C:
            this->STY(this->getAbsoluteAddr()); m_cpuCycle += 4; break;

        // Registers transfer instructions
        //
        // General purpose registers
        case 0xAA:
            this->TAX(); m_cpuCycle += 2; break;
        case 0xA8:
            this->TAY(); m_cpuCycle += 2; break;
        case 0x8A:
            this->TXA(); m_cpuCycle += 2; break;
        case 0x98:
            this->TYA(); m_cpuCycle += 2; break;
        // Stack pointer register 
        case 0xBA:
            this->TSX(); m_cpuCycle += 2; break;
        case 0x9A:
            this->TXS(); m_cpuCycle += 2; break;

        // Increment / decrements instructions
        //
        // INC instruction
        case 0xE6:
            this->INC(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0xF6:
            this->INC(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0xEE:
            this->INC(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0xFE:
            this->INC(getAbsoluteXAddr()); m_cpuCycle += 7; break;
        // DEC instruction
        case 0xC6:
            this->DEC(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0xD6:
            this->DEC(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0xCE:
            this->DEC(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0xDE:
            this->DEC(getAbsoluteXAddr()); m_cpuCycle += 7; break;
        // Registers increment / decrements instructions
        case 0xE8:
            this->INX(); m_cpuCycle += 2; break;
        case 0xC8:
            this->INY(); m_cpuCycle += 2; break;
        case 0xCA:
            this->DEX(); m_cpuCycle += 2; break;
        case 0x88:
            this->DEY(); m_cpuCycle += 2; break;

        // Update status flag instructions
        //
        // Clear instructions
        case 0x18:
            this->CLC(); m_cpuCycle += 2; break;
        case 0xD8:
            this->CLD(); m_cpuCycle += 2; break;
        case 0x58:
            this->CLI(); m_cpuCycle += 2; break;
        case 0xB8:
            this->CLV(); m_cpuCycle += 2; break;
        // Set instructions
        case 0x38:
            this->SEC(); m_cpuCycle += 2; break;
        case 0xF8:
            this->SED(); m_cpuCycle += 2; break;
        case 0x78:
            this->SEI(); m_cpuCycle += 2; break;

        // Branch instructions
        case 0x90:
            this->BCC(getRelative()); m_cpuCycle += 2; break;
        case 0xB0:
            this->BCS(getRelative()); m_cpuCycle += 2; break;
        case 0xF0:
            this->BEQ(getRelative()); m_cpuCycle += 2; break;
        case 0x30:
            this->BMI(getRelative()); m_cpuCycle += 2; break;
        case 0xD0:
            this->BNE(getRelative()); m_cpuCycle += 2; break;
        case 0x10:
            this->BPL(getRelative()); m_cpuCycle += 2; break;
        case 0x50:
            this->BVC(getRelative()); m_cpuCycle += 2; break;
        case 0x70:
            this->BVS(getRelative()); m_cpuCycle += 2; break;

        // Jumps instructions
        //
        // JMP instruction
        case 0x4C:
            this->JMP(getAbsoluteAddr()); m_cpuCycle += 3; break;
        case 0x6C:
            this->JMP(getIndirectAddr()); m_cpuCycle += 5; break;
        // Subroutine instructions
        case 0x20:
            this->JSR(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x60:
            this->RTS(); m_cpuCycle += 6; break;

        // Arithmetic instructions
        //
        // ADC instruction
        case 0x69:
            this->ADC(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0x65:
            this->ADC(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x75:
            this->ADC(getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0x6D:
            this->ADC(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0x7D:
            this->ADC(getAbsoluteXAddr(true)); m_cpuCycle += 4; break;
        case 0x79:
            this->ADC(getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0x61:
            this->ADC(getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0x71:
            this->ADC(getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;
        // SBC instruction
        case 0xE9:
            this->SBC(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0xE5:
            this->SBC(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xF5:
            this->SBC(getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0xED:
            this->SBC(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0xFD:
            this->SBC(getAbsoluteXAddr(true)); m_cpuCycle += 4; break;
        case 0xF9:
            this->SBC(getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0xE1:
            this->SBC(getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0xF1:
            this->SBC(getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;
        // CMP instruction
        case 0xC9:
            this->CMP(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0xC5:
            this->CMP(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xD5:
            this->CMP(getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0xCD:
            this->CMP(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0xDD:
            this->CMP(getAbsoluteXAddr(true)); m_cpuCycle += 4; break;
        case 0xD9:
            this->CMP(getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0xC1:
            this->CMP(getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0xD1:
            this->CMP(getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;
        // CPX instruction
        case 0xE0:
            this->CPX(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0xE4:  
            this->CPX(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xEC:  
            this->CPX(getAbsoluteAddr()); m_cpuCycle += 4; break;
        // CPY instruction
        case 0xC0:
            this->CPY(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0xC4:  
            this->CPY(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xCC:  
            this->CPY(getAbsoluteAddr()); m_cpuCycle += 4; break;

        // Logical instructions
        //
        // AND instruction
        case 0x29:
            this->AND(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0x25:
            this->AND(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x35:
            this->AND(getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0x2D:
            this->AND(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0x3D:
            this->AND(getAbsoluteXAddr(true)); m_cpuCycle += 4; break;
        case 0x39:
            this->AND(getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0x21:
            this->AND(getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0x31:
            this->AND(getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;
        // EOR instruction
        case 0x49:
            this->EOR(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0x45:
            this->EOR(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x55:
            this->EOR(getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0x4D:
            this->EOR(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0x5D:
            this->EOR(getAbsoluteXAddr(true)); m_cpuCycle += 4; break;
        case 0x59:
            this->EOR(getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0x41:
            this->EOR(getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0x51:
            this->EOR(getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;
        // ORA instruction
        case 0x09:
            this->ORA(getImmediateAddr()); m_cpuCycle += 2; break;
        case 0x05:
            this->ORA(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x15:
            this->ORA(getZeroPageXAddr()); m_cpuCycle += 4; break;
        case 0x0D:
            this->ORA(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0x1D:
            this->ORA(getAbsoluteXAddr(true)); m_cpuCycle += 4; break;
        case 0x19:
            this->ORA(getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0x01:
            this->ORA(getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0x11:
            this->ORA(getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;
        // BIT instruction
        case 0x24:
            this->BIT(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x2C:
            this->BIT(getAbsoluteAddr()); m_cpuCycle += 4; break;

        // Shift instructions
        //
        // ASL instruction
        case 0x0A:
            this->ASL(); m_cpuCycle += 2; break;
        case 0x06:
            this->ASL(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x16:
            this->ASL(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x0E:
            this->ASL(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x1E:
            this->ASL(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        // LSR instruction
        case 0x4A:
            this->LSR(); m_cpuCycle += 2; break;
        case 0x46:
            this->LSR(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x56:
            this->LSR(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x4E:
            this->LSR(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x5E:
            this->LSR(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        // ROL instruction
        case 0x2A:
            this->ROL(); m_cpuCycle += 2; break;
        case 0x26:
            this->ROL(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x36:
            this->ROL(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x2E:
            this->ROL(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x3E:
            this->ROL(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        // ROR instruction
        case 0x6A:
            this->ROR(); m_cpuCycle += 2; break;
        case 0x66:
            this->ROR(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x76:
            this->ROR(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x6E:
            this->ROR(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x7E:
            this->ROR(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;

        // Stack instructions
        case 0x48:
            this->PHA(); m_cpuCycle += 3; break;
        case 0x08:
            this->PHP(); m_cpuCycle += 3; break;
        case 0x68:
            this->PLA(); m_cpuCycle += 4; break;
        case 0x28:
            this->PLP(); m_cpuCycle += 4; break;

        // Interrupt instructions
        case 0x40:
            this->RTI(); m_cpuCycle += 6; break;
        case 0x00:
            this->BRK(); m_cpuCycle += 7; break;

        // NOP instruction
        case 0xEA:
            m_cpuCycle += 2; break;

        // Illegal instructions
        //
        // LAX instruction
        case 0xA7:
            this->LAX(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0xB7:
            this->LAX(getZeroPageYAddr()); m_cpuCycle += 4; break;
        case 0xAF:
            this->LAX(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0xBF:
            this->LAX(getAbsoluteYAddr(true)); m_cpuCycle += 4; break;
        case 0xA3:
            this->LAX(getIndexedIndirectAddr()); m_cpuCycle += 6; break;
        case 0xB3:
            this->LAX(getIndirectIndexedAddr(true)); m_cpuCycle += 5; break;

        // SAX instruction
        case 0x87:
            this->SAX(getZeroPageAddr()); m_cpuCycle += 3; break;
        case 0x97:
            this->SAX(getZeroPageYAddr()); m_cpuCycle += 4; break;
        case 0x8F:
            this->SAX(getAbsoluteAddr()); m_cpuCycle += 4; break;
        case 0x83:
            this->SAX(getIndexedIndirectAddr()); m_cpuCycle += 6; break;

        // SBC + NOP instruction
        case 0xEB:
            this->SBC(getImmediateAddr()); m_cpuCycle += 2; break;

        // DCP instruction
        case 0xC7:
            this->DCP(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0xD7:
            this->DCP(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0xCF:
            this->DCP(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0xDF:
            this->DCP(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        case 0xDB:
            this->DCP(getAbsoluteYAddr(false)); m_cpuCycle += 7; break;
        case 0xC3:
            this->DCP(getIndexedIndirectAddr()); m_cpuCycle += 8; break;
        case 0xD3:
            this->DCP(getIndirectIndexedAddr(false)); m_cpuCycle += 8; break;
        
        // ISC instruction
        case 0xE7:
            this->ISC(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0xF7:
            this->ISC(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0xEF:
            this->ISC(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0xFF:
            this->ISC(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        case 0xFB:
            this->ISC(getAbsoluteYAddr(false)); m_cpuCycle += 7; break;
        case 0xE3:
            this->ISC(getIndexedIndirectAddr()); m_cpuCycle += 8; break;
        case 0xF3:
            this->ISC(getIndirectIndexedAddr(false)); m_cpuCycle += 8; break;
        
        // SLO instruction
        case 0x07:
            this->SLO(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x17:
            this->SLO(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x0F:
            this->SLO(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x1F:
            this->SLO(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        case 0x1B:
            this->SLO(getAbsoluteYAddr(false)); m_cpuCycle += 7; break;
        case 0x03:
            this->SLO(getIndexedIndirectAddr()); m_cpuCycle += 8; break;
        case 0x13:
            this->SLO(getIndirectIndexedAddr(false)); m_cpuCycle += 8; break;

        // RLA instruction
        case 0x27:
            this->RLA(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x37:
            this->RLA(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x2F:
            this->RLA(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x3F:
            this->RLA(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        case 0x3B:
            this->RLA(getAbsoluteYAddr(false)); m_cpuCycle += 7; break;
        case 0x23:
            this->RLA(getIndexedIndirectAddr()); m_cpuCycle += 8; break;
        case 0x33:
            this->RLA(getIndirectIndexedAddr(false)); m_cpuCycle += 8; break;
         
        // SRE instruction
        case 0x47:
            this->SRE(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x57:
            this->SRE(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x4F:
            this->SRE(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x5F:
            this->SRE(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        case 0x5B:
            this->SRE(getAbsoluteYAddr(false)); m_cpuCycle += 7; break;
        case 0x43:
            this->SRE(getIndexedIndirectAddr()); m_cpuCycle += 8; break;
        case 0x53:
            this->SRE(getIndirectIndexedAddr(false)); m_cpuCycle += 8; break;   

        // RRA instruction
        case 0x67:
            this->RRA(getZeroPageAddr()); m_cpuCycle += 5; break;
        case 0x77:
            this->RRA(getZeroPageXAddr()); m_cpuCycle += 6; break;
        case 0x6F:
            this->RRA(getAbsoluteAddr()); m_cpuCycle += 6; break;
        case 0x7F:
            this->RRA(getAbsoluteXAddr(false)); m_cpuCycle += 7; break;
        case 0x7B:
            this->RRA(getAbsoluteYAddr(false)); m_cpuCycle += 7; break;
        case 0x63:
            this->RRA(getIndexedIndirectAddr()); m_cpuCycle += 8; break;
        case 0x73:
            this->RRA(getIndirectIndexedAddr(false)); m_cpuCycle += 8; break;  

        // NOPs instructions
        // NOPs with implied addressing
        case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: 
            m_cpuCycle += 2; break;
        // NOPs with Immediate addressing
        case 0x80: case 0x82: case 0x89: case 0xC2: case 0xE2: 
            m_cpuCycle += 2; getImmediateAddr(); break;
        // NOPs with zero page addressing
        case 0x04: case 0x44: case 0x64: 
            m_cpuCycle += 3; getZeroPageAddr(); break;
        // NOPs with zero page X addressing
        case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4:
            m_cpuCycle += 4; getZeroPageXAddr(); break;
        // NOP with absolute addressing
        case 0x0C:
            m_cpuCycle += 4; getAbsoluteAddr(); break;
        // NOPs with absolute X addressing
        case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: 
            m_cpuCycle += 4; getAbsoluteXAddr(true); break;

        // JAM instructions (Stop execution and halt the CPU)
        case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: 
        case 0x62: case 0x72: case 0x92: case 0xB2: case 0xD2: case 0xF2: 
            std::cout << "CPU: Halted at instruction: " << utility::paddedHex(opCode, 2);
            std::cout << std::endl;
            m_pc--; return 0;

        default:
            std::cout << "CPU: Unknow instruction: " << utility::paddedHex(opCode, 2);
            std::cout << std::endl;
            return 0;
    }

    // Execute interrupt if one was received during the last instruction
    // and increment the cycle counter
    executeInterrupt(interrupt);

    // Check if the DMA was active
    if (m_bus->dmaCycles())
        // Add an aliment cycle on odd CPU cycles
        m_cpuCycle += m_cpuCycle % 2 ? 514 : 513;

    // Return the number of cycle and update CPU cycle counter
    return m_cpuCycle - startCycles;
}

/*
 *
 *  Addressing functions
 *
*/

// Immediate addressing
inline uint16_t Cpu6502::getImmediateAddr() {
    // Get value and update the program counter
    uint16_t address = m_pc;
    m_pc++;

    return address;
}

// Zero page addressing
inline uint16_t Cpu6502::getZeroPageAddr() {
    // Get address and update program counter
    uint16_t address = static_cast<uint16_t>(m_bus->read(m_pc));
    m_pc++;

    return address;
}
inline uint16_t Cpu6502::getZeroPageXAddr() {
    // Get address and update program counter
    uint16_t address = static_cast<uint16_t>(m_bus->read(m_pc));
    address = (address + m_regX) & 0xFF;
    m_pc++;

    return address;
}
inline uint16_t Cpu6502::getZeroPageYAddr() {
    // Get address and update program counter
    uint16_t address = static_cast<uint16_t>(m_bus->read(m_pc));
    address = (address + m_regY) & 0xFF;
    m_pc++;

    return address;
}

// Absolute addressing
inline uint16_t Cpu6502::getAbsoluteAddr() {
    // Get value and update program counter
    uint16_t address = m_bus->read16(m_pc);
    m_pc += 2;

    return address;
}
inline uint16_t Cpu6502::getAbsoluteXAddr(bool pageCrossAddCycle) {
    // Get value and update program counter
    uint16_t address = m_bus->read16(m_pc);
    m_pc += 2;

    // Branch less page crossing check 
    bool pageCross = (((address & 0x00FF) + m_regX) & 0xFF00) != 0;
    m_cpuCycle += static_cast<uint64_t>(pageCross && pageCrossAddCycle);

    return address + m_regX;
}
inline uint16_t Cpu6502::getAbsoluteYAddr(bool pageCrossAddCycle) {
    // Get value and update program counter
    uint16_t address = m_bus->read16(m_pc);
    m_pc += 2;

    // Branch less page crossing check 
    bool pageCross = (((address & 0x00FF) + m_regY) & 0xFF00) != 0;
    m_cpuCycle += static_cast<uint64_t>(pageCross && pageCrossAddCycle);

    return address + m_regY;
}

// Indirect addressing
inline uint16_t Cpu6502::getIndirectAddr() {
    // Get address and update program counter
    uint16_t address = m_bus->read16(m_pc);
    m_pc += 2;

    return m_bus->read16PageWrap(address);
    //return m_bus->read16(address);
}
inline uint16_t Cpu6502::getIndexedIndirectAddr() {
    // Get address and update program counter
    uint16_t address = static_cast<uint16_t>(m_bus->read(m_pc));
    address = (address + m_regX) & 0x00FF;
    m_pc++;

    return m_bus->read16PageWrap(address);
}
inline uint16_t Cpu6502::getIndirectIndexedAddr(bool pageCrossAddCycle) {
    // Get address and update program counter
    uint16_t address = static_cast<uint16_t>(m_bus->read(m_pc));
    address = m_bus->read16PageWrap(address);
    m_pc++;

    // Branch less page crossing check 
    bool pageCross = (((address & 0x00FF) + m_regY) & 0xFF00) != 0;
    m_cpuCycle += static_cast<uint64_t>(pageCross && pageCrossAddCycle);

    return address + m_regY;
}

// Relative addressing for branch operations
inline int8_t Cpu6502::getRelative() {
    // Get value and update program counter
    int8_t result = static_cast<int8_t>(m_bus->read(m_pc));
    m_pc++;

    return result;
}
}

// Implement the CPU operations
namespace nesCore {
/*
 *
 *  Stack functions
 *
*/

// Push 1 byte to the stack
inline void Cpu6502::stackPush(uint8_t data) {
    // Write the data to the stack
    uint16_t sAddr = 0x100 + m_stackPointer;
    m_bus->write(sAddr, data);

    // Update the stack pointer
    m_stackPointer -= 1;
}
// Push 2 byte to the stack
inline void Cpu6502::stackPush16(uint16_t data) {
    // Write the data to the stack
    uint16_t sAddr = 0x0FF + m_stackPointer;
    m_bus->write16(sAddr, data);

    // Update the stack pointer
    m_stackPointer -= 2;;
}

// Pop 1 byte from the stack
inline uint8_t Cpu6502::stackPop() {
    // Update the stack pointer
    m_stackPointer += 1;

    return m_bus->read(0x100 + m_stackPointer);
}
// Pop 2 byte from the stack
inline uint16_t Cpu6502::stackPop16() {
    // Update the stack pointer
    m_stackPointer += 2;

    return m_bus->read16(0xFF + m_stackPointer);
}

// Push the accumulator to the stack
inline void Cpu6502::PHA() {
    this->stackPush(m_accumulator);
}
// Push the processor status to the stack
inline void Cpu6502::PHP() {
    this->stackPush(this->getStatusByte(true));
}

// Pull the accumulator from the stack
inline void Cpu6502::PLA() {
    m_accumulator = this->stackPop();
    
    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
// Pull the processor status from the stack
inline void Cpu6502::PLP() {
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
inline void Cpu6502::AND(uint16_t addr) {
    // Fetch operation and perform the operation
    uint8_t memValue = m_bus->read(addr);
    m_accumulator &= memValue;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
 
// Perform an EOR operation on the accumulator with a byte from memory
// and update the status variable of the CPU
inline void Cpu6502::EOR(uint16_t addr) {
    // Fetch operation and perform the operation
    uint8_t memValue = m_bus->read(addr);
    m_accumulator ^= memValue;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
 
// Perform an ORA operation on the accumulator with a byte from memory
// and update the status variable of the CPU
inline void Cpu6502::ORA(uint16_t addr) {
    // Fetch operation and perform the operation
    uint8_t memValue = m_bus->read(addr);
    m_accumulator |= memValue;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}

// Get a number from the bus and mask it with the accumulator 
// Check if bit 7 and 6 are set or if the number is 0
inline void Cpu6502::BIT(uint16_t addr) {
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
inline void Cpu6502::ADC(uint16_t addr) {
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
inline void Cpu6502::SBC(uint16_t addr) {
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
inline void Cpu6502::CMP(uint16_t addr) {
    uint8_t memValue = m_bus->read(addr);
    uint8_t result = m_accumulator - static_cast<uint16_t>(memValue);

    // Update the flags
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_carryFlag = m_accumulator >= memValue;
    m_zeroFlag = m_accumulator == memValue;
}
// Compare the accumulator value with a memory value
inline void Cpu6502::CPX(uint16_t addr) {
    uint8_t memValue = m_bus->read(addr);
    uint8_t result = m_regX - static_cast<uint16_t>(memValue);

    // Update the flags
    m_negativeFlag = (result & 0b10000000) != 0; 
    m_carryFlag = m_regX >= memValue;
    m_zeroFlag = m_regX == memValue;
}
// Compare the accumulator value with a memory value
inline void Cpu6502::CPY(uint16_t addr) {
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
inline void Cpu6502::ASL(uint16_t addr) {
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
inline void Cpu6502::ASL() {
    // Perform the shift
    uint8_t value = m_accumulator;
    m_accumulator = value << 1;

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Shift right a byte of memory on the bus 
inline void Cpu6502::LSR(uint16_t addr) {
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
inline void Cpu6502::LSR() {
    // Perform the shift
    uint8_t value = m_accumulator;
    m_accumulator = value >> 1;

    // Update CPU status 
    m_carryFlag = (value & 0b00000001) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Roll left a byte of memory on the bus 
inline void Cpu6502::ROL(uint16_t addr) {
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
inline void Cpu6502::ROL() {
    // Perform the shift
    uint8_t value = m_accumulator;
    m_accumulator = (value << 1) | static_cast<uint8_t>(m_carryFlag);

    // Update CPU status 
    m_carryFlag = (value & 0b10000000) != 0;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
    m_zeroFlag = m_accumulator == 0x00;
}

// Roll right a byte of memory on the bus 
inline void Cpu6502::ROR(uint16_t addr) {
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
inline void Cpu6502::ROR() {
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
inline void Cpu6502::LDA(uint16_t addr) {
    m_accumulator = m_bus->read(addr);

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
// Load a byte of memory into the X register and update the status flags
inline void Cpu6502::LDX(uint16_t addr) {
    m_regX = m_bus->read(addr);

    // Update status registers
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Load a byte of memory into the Y register and update the status flags
inline void Cpu6502::LDY(uint16_t addr) {
    m_regY = m_bus->read(addr);

    // Update status registers
    m_zeroFlag = m_regY == 0x00;
    m_negativeFlag = (m_regY & 0b10000000) != 0;
}

// Save the value byte stored in the accumulator to an address of the bus
inline void Cpu6502::STA(uint16_t addr) {
    m_bus->write(addr, m_accumulator);
}
// Save the value byte stored in the X register to an address of the bus
inline void Cpu6502::STX(uint16_t addr) {
    m_bus->write(addr, m_regX);
}
// Save the value byte stored in the Y register to an address of the bus
inline void Cpu6502::STY(uint16_t addr) {
    m_bus->write(addr, m_regY);
}

/*
*
*   Branch instructions
*
*/

// Branch if carry flag is clear
inline void Cpu6502::BCC(int8_t offset) {
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
inline void Cpu6502::BCS(int8_t offset) {
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
inline void Cpu6502::BPL(int8_t offset) {
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
inline void Cpu6502::BMI(int8_t offset) {
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
inline void Cpu6502::BNE(int8_t offset) {
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
inline void Cpu6502::BEQ(int8_t offset) {
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
inline void Cpu6502::BVC(int8_t offset) {
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
inline void Cpu6502::BVS(int8_t offset) {
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
inline void Cpu6502::TAX() {
    m_regX = m_accumulator;

    // Update status registers
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Copy the value in the accumulator to the register Y
inline void Cpu6502::TAY() {
    m_regY = m_accumulator;

    // Update status registers
    m_zeroFlag = m_regY == 0x00;
    m_negativeFlag = (m_regY & 0b10000000) != 0;
}
// Copy the value in the register X in the accumulator
inline void Cpu6502::TXA() {
    m_accumulator = m_regX;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}
// Copy the value in the register Y in the accumulator
inline void Cpu6502::TYA() {
    m_accumulator = m_regY;

    // Update status registers
    m_zeroFlag = m_accumulator == 0x00;
    m_negativeFlag = (m_accumulator & 0b10000000) != 0;
}

// Copy the value in the stack pointer to the register X
inline void Cpu6502::TSX() {
    m_regX = m_stackPointer;

    // Update status registers
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Copy the value in the register X to the stack pointer
inline void Cpu6502::TXS() {
    m_stackPointer = m_regX;
}

/*
*
*   Increment and Decrements operations
*
*/

// Increment the value to the given memory address
// and update the status register
inline void Cpu6502::INC(uint16_t addr) {
    uint8_t result = m_bus->read(addr) + 1;
    m_bus->write(addr, result);

    // Update status register
    m_zeroFlag = result == 0x00;
    m_negativeFlag = (result & 0b10000000) != 0;
}
// Decrements the value to the given memory address
// and update the status register
inline void Cpu6502::DEC(uint16_t addr) {
    uint8_t result = m_bus->read(addr) - 1;
    m_bus->write(addr, result);

    // Update status register
    m_zeroFlag = result == 0x00;
    m_negativeFlag = (result & 0b10000000) != 0;
}

// Increment the value in the X register
// and update the status register
inline void Cpu6502::INX() {
    m_regX += 1;

    // Update status register
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}
// Decrements the value in the X register
// and update the status register
inline void Cpu6502::DEX() {
    m_regX -= 1;

    // Update status register
    m_zeroFlag = m_regX == 0x00;
    m_negativeFlag = (m_regX & 0b10000000) != 0;
}

// Increment the value in the Y register
// and update the status register
inline void Cpu6502::INY() {
    m_regY += 1;

    // Update status register
    m_zeroFlag = m_regY == 0x00;
    m_negativeFlag = (m_regY & 0b10000000) != 0;
}
// Decrements the value in the Y register
// and update the status register
inline void Cpu6502::DEY() {
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
inline void Cpu6502::CLC() {
    m_carryFlag = false;
}
// Clear the decimal mode flag
inline void Cpu6502::CLD() {
    m_decimalMode = false;
}
// Clear the interrupt disable flag
inline void Cpu6502::CLI() {
    m_interruptDisable = false;
}
// Clear the overflow flag
inline void Cpu6502::CLV() {
    m_overflowFlag = false;
}

// Set the carry flag
inline void Cpu6502::SEC() {
    m_carryFlag = true;
}
// Set the decimal mode flag
inline void Cpu6502::SED() {
    m_decimalMode = true;
}
// Set the interrupt disable flag
inline void Cpu6502::SEI() {
    m_interruptDisable = true;
}

/*
 *
 *  interrupt functions
 *
*/

// Generate an software interrupt and set the PC to the IRQ vector
inline void Cpu6502::BRK() {
    // Push PC and status to stack
    this->stackPush16(m_pc + 1);
    this->stackPush(this->getStatusByte(true));

    // Set the interrupt disable flag 
    m_interruptDisable = true;

    m_pc = m_bus->read16(IRQ_BRK_VECTOR_ADDR);
}

// Return from an interrupt by pulling PC and CPU status from the stack
inline void Cpu6502::RTI() {
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
inline void Cpu6502::JMP(uint16_t addr) {
    m_pc = addr;
}

// Jumps to the given address and push to stack
// the current PC minus one
inline void Cpu6502::JSR(uint16_t addr) {
    this->stackPush16(m_pc - 1);
    m_pc = addr;
}

// Pull an address from the stack an set the PC to it
inline void Cpu6502::RTS() {
    uint16_t addr = this->stackPop16();
    m_pc = addr + 1;
}

/*
*
*   Illegal Op code operations
*
*/

// Load a byte of memory into the accumulator and register X and update the status flags
inline void Cpu6502::LAX(uint16_t addr) {
    uint8_t value = m_bus->read(addr);
    m_accumulator = value;
    m_regX = value;

    // Update status registers
    // WRONG
    m_zeroFlag = value == 0x00;
    m_negativeFlag = (value & 0b10000000) != 0;
}

// Perform an AND operation between the accumulator and X register and store the result in memory
inline void Cpu6502::SAX(uint16_t addr) {
    m_bus->write(addr, m_accumulator & m_regX);
}

// Perform a decrement and compare operation
inline void Cpu6502::DCP(uint16_t addr) {
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
inline void Cpu6502::ISC(uint16_t addr) {
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
inline void Cpu6502::SLO(uint16_t addr) {
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
inline void Cpu6502::RLA(uint16_t addr) {
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
inline void Cpu6502::SRE(uint16_t addr) {
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
inline void Cpu6502::RRA(uint16_t addr) {
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

