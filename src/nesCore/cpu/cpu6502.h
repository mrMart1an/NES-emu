#ifndef CPU6502_H_
#define CPU6502_H_

#include <cstddef>
#include <cstdint>

#define RESET_VECTOR_ADDR 0xFFFC
#define NMI_VECTOR_ADDR 0xFFFA
#define IRQ_BRK_VECTOR_ADDR 0xFFFE

namespace nesCore {
namespace debug {
class Cpu6502Debug;
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
    void stackPush(uint8_t data);
    uint8_t stackPop();

    void stackPush16(uint16_t data);
    uint16_t stackPop16();

    // Addressing mode functions
    // these functions use the current position of the 
    // program counter as their input address
    // and return the address of the parameter for the instruction
    // !!! These functions update the program counter !!!
    uint16_t getImmediateAddr();
    uint16_t getZeroPageAddr();
    // Add the value in X or Y to the zero page value
    uint16_t getZeroPageXAddr();
    uint16_t getZeroPageYAddr();

    uint16_t getAbsoluteAddr();
    // Add the value in X or Y to the absolute value
    uint16_t getAbsoluteXAddr(bool pageCrossAddCycle = false);
    uint16_t getAbsoluteYAddr(bool pageCrossAddCycle = false);

    uint16_t getIndirectAddr();
    uint16_t getIndexedIndirectAddr();
    uint16_t getIndirectIndexedAddr(bool pageCrossAddCycle = false);

    // Get the jump offset for branch instructions
    int8_t getRelative();

    // CPU instructions op code
    //
    // Load/Store operations
    void LDA(uint16_t addr); void LDX(uint16_t addr); void LDY(uint16_t addr);
    void STA(uint16_t addr); void STX(uint16_t addr); void STY(uint16_t addr);
    // Register transfer operations
    void TAX(); void TAY(); void TXA(); void TYA();
    // Stack operations
    void TSX(); void TXS(); void PHA(); void PHP(); void PLA(); void PLP();
    // Logical operations
    void AND(uint16_t addr); void EOR(uint16_t addr); 
    void ORA(uint16_t addr); void BIT(uint16_t addr);
    // Arithmetic operations
    void ADC(uint16_t addr); void SBC(uint16_t addr);
    void CMP(uint16_t addr); void CPX(uint16_t addr); void CPY(uint16_t addr);
    // Increment / decrement operations
    void INC(uint16_t addr); void INX(); void INY();
    void DEC(uint16_t addr); void DEX(); void DEY();
    // Shift operations
    void ASL(uint16_t addr); void LSR(uint16_t addr);
    void ROL(uint16_t addr); void ROR(uint16_t addr);
    void ASL(); void LSR(); void ROL(); void ROR();
    // Jump / call operations
    void JMP(uint16_t addr); void JSR(uint16_t addr); void RTS();
    // Branches operations
    void BCC(int8_t offset); void BCS(int8_t offset);
    void BEQ(int8_t offset); void BMI(int8_t offset);
    void BNE(int8_t offset); void BPL(int8_t offset);
    void BVC(int8_t offset); void BVS(int8_t offset);
    // Status flag change operations
    void CLC(); void CLD(); void CLI(); void CLV();
    void SEC(); void SED(); void SEI();
    // System operations
    void BRK(); void RTI();

    // Illegal CPU instructions
    void LAX(uint16_t addr); void SAX(uint16_t addr); 
    void DCP(uint16_t addr); void ISC(uint16_t addr);
    void SLO(uint16_t addr); void RLA(uint16_t addr);
    void SRE(uint16_t addr); void RRA(uint16_t addr);

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
