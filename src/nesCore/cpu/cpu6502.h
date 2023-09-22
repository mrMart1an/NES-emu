#ifndef CPU6502_H_
#define CPU6502_H_

#include "../../nesPch.h"

#define RESET_VECTOR_ADDR 0xFFFC
#define NMI_VECTOR_ADDR 0xFFFA
#define IRQ_BRK_VECTOR_ADDR 0xFFFE

namespace nesCore {
namespace debug {
struct Cpu6502Debug;
}
class Bus;

// Interrupt emum
enum Interrupt6502 {
    NOINT = 0b000,
    IRQ =   0b001,
    NMI =   0b010,
    ALLINT= 0b011,
};

// 6502 based CPU powering the nes
class Cpu6502 {
// Public methods
public:
    // Construct the CPU on a given bus
    Cpu6502(Bus*);

    // Reset all the CPU registers
    void reset();
    void setProgramCounter(uint16_t addr);

    // Execute the next instruction and return
    // the number of cycle required
    size_t step(Interrupt6502 interrupt = NOINT);

    // Return a debug struct with the current CPU status
    debug::Cpu6502Debug getDebugInfo();

// Private methods
private:
    // Get the interrupt and return the interrupt to execute
    // based on the disable interrupt flag
    Interrupt6502 pollInterrupt(Interrupt6502 interrupt);
    // Execute the interrupt if interrupt status contain an interrupt
    // This functions update the CPU cycles counter
    void executeInterrupt(Interrupt6502 interrupt);

    // Convert the status booleans to a byte
    uint8_t getStatusByte(bool bFlag = false);

    // Stack operations
    inline void stackPush(uint8_t data);
    inline uint8_t stackPop();

    inline void stackPush16(uint16_t data);
    inline uint16_t stackPop16();

    // Addressing mode functions
    // these functions use the current position of the 
    // program counter as their input address
    // and return the address of the parameter for the instruction
    // !!! These functions update the program counter !!!
    inline uint16_t getImmediateAddr();
    inline uint16_t getZeroPageAddr();
    // Add the value in X or Y to the zero page value
    inline uint16_t getZeroPageXAddr();
    inline uint16_t getZeroPageYAddr();

    inline uint16_t getAbsoluteAddr();
    // Add the value in X or Y to the absolute value
    inline uint16_t getAbsoluteXAddr(bool pageCrossAddCycle = false);
    inline uint16_t getAbsoluteYAddr(bool pageCrossAddCycle = false);

    inline uint16_t getIndirectAddr();
    inline uint16_t getIndexedIndirectAddr();
    inline uint16_t getIndirectIndexedAddr(bool pageCrossAddCycle = false);

    // Get the jump offset for branch instructions
    inline int8_t getRelative();

    // CPU instructions op code
    //
    // Load/Store operations
    inline void LDA(uint16_t addr); inline void LDX(uint16_t addr); inline void LDY(uint16_t addr);
    inline void STA(uint16_t addr); inline void STX(uint16_t addr); inline void STY(uint16_t addr);
    // Register transfer operations
    inline void TAX(); inline void TAY(); inline void TXA(); inline void TYA();
    // Stack operations
    inline void TSX(); inline void TXS(); inline void PHA(); inline void PHP(); 
    inline void PLA(); inline void PLP();
    // Logical operations
    inline void AND(uint16_t addr); inline void EOR(uint16_t addr); 
    inline void ORA(uint16_t addr); inline void BIT(uint16_t addr);
    // Arithmetic operations
    inline void ADC(uint16_t addr); inline void SBC(uint16_t addr);
    inline void CMP(uint16_t addr); inline void CPX(uint16_t addr); inline void CPY(uint16_t addr);
    // Increment / decrement operations
    inline void INC(uint16_t addr); inline void INX(); inline void INY();
    inline void DEC(uint16_t addr); inline void DEX(); inline void DEY();
    // Shift operations
    inline void ASL(uint16_t addr); inline void LSR(uint16_t addr);
    inline void ROL(uint16_t addr); inline void ROR(uint16_t addr);
    inline void ASL(); inline void LSR(); inline void ROL(); inline void ROR();
    // Jump / call operations
    inline void JMP(uint16_t addr); inline void JSR(uint16_t addr); inline void RTS();
    // Branches operations
    inline void BCC(int8_t offset); inline void BCS(int8_t offset);
    inline void BEQ(int8_t offset); inline void BMI(int8_t offset);
    inline void BNE(int8_t offset); inline void BPL(int8_t offset);
    inline void BVC(int8_t offset); inline void BVS(int8_t offset);
    // Status flag change operations
    inline void CLC(); inline void CLD(); inline void CLI(); inline void CLV();
    inline void SEC(); inline void SED(); inline void SEI();
    // System operations
    inline void BRK(); inline void RTI();

    // Illegal CPU instructions
    inline void LAX(uint16_t addr); inline void SAX(uint16_t addr); 
    inline void DCP(uint16_t addr); inline void ISC(uint16_t addr);
    inline void SLO(uint16_t addr); inline void RLA(uint16_t addr);
    inline void SRE(uint16_t addr); inline void RRA(uint16_t addr);

// Private member variable
private:
    // CPU cycle since the last reset
    uint64_t m_cpuCycle;

    // Program pointer
    uint16_t m_pc;
    // Stack pointer
    uint8_t m_stackPointer;
    // CPU registers
    uint8_t m_regX, m_regY, m_accumulator;
    
    // Pointer to the nes CPU bus
    Bus* m_bus;

    // Status registers
    bool m_carryFlag;
    bool m_zeroFlag;
    bool m_interruptDisable;
    bool m_decimalMode; 

    bool m_overflowFlag;
    bool m_negativeFlag;
};
}

#endif
