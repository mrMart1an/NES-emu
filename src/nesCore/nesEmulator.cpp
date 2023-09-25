#include "nesPch.h"

#include "nesEmulator.h"
#include "cartridge/cartridge.h"
#include "cartridge/nromCartridge.h"
#include "cartridge/cnromCartridge.h"
#include "cpu/cpu6502.h"
#include "cpuBus.h"
#include "cpu/cpu6502debug.h"
#include "frameBuffer.h"
#include "inputOutput/IOInterface.h"
#include "ppu/ppuDebug.h"

namespace nesCore {
NesEmulator::NesEmulator() : m_cpuBus(), m_ppuBus(), mp_cartridge(nullptr) {
    // Setup CPU and CPU bus
    m_cpuBus.attachPpu(&m_ppuBus.m_ppu);
    m_cpuBus.m_cpu.reset();

    // Setup PPU
    m_ppuBus.m_ppu.attachFrameBuffer(&m_frameBuffer);

    // Set the PPU interrupt to NOINT 
    m_ppuInt = NOINT;
}
NesEmulator::~NesEmulator() {
    if (mp_cartridge != nullptr) 
        delete mp_cartridge;
}
    
// Setup the emulator, return 0 on success
int NesEmulator::setup(
        const std::string& startupRomPath,
        const std::string& palettePath
) {
    // Attempt to load the emulator color palette
    int paletteErr = loadPalette(palettePath);
    if (paletteErr != 0) {
        std::cerr << "Failed to load color palette, error code: ";
        std::cerr << paletteErr << std::endl;
        return 10;
    }
    
    // Attempt to load the game ROM
    int cartErr = loadCartridgeFromFile(startupRomPath);
    if (cartErr != 0){
        std::cerr << "Failed to load game ROM, error code: ";
        std::cerr << cartErr << std::endl;
        return 11;
    }

    return 0;
}

// Get a pointer to the emulator frame buffer
FrameBuffer* NesEmulator::getFrameBuffer() {
    return &m_frameBuffer;
}
// Attach an IO interface to the cpu bus
void NesEmulator::attachIO(IOInterface* interface) {
    m_cpuBus.attachIO(interface);
}

// Execute one CPU instruction
void NesEmulator::step() {
    size_t cpuCycle = m_cpuBus.m_cpu.step(m_ppuInt);
    m_ppuInt = m_ppuBus.m_ppu.clock(cpuCycle);
}

// Reset the emulator
void NesEmulator::reset() {
    m_cpuBus.m_cpu.reset();
    m_ppuBus.m_ppu.reset();

    if (mp_cartridge != nullptr)
        mp_cartridge->reset();

    m_ppuInt = NOINT;
}

// Load a cartridge from a file
int NesEmulator::loadCartridgeFromFile(const std::string& filename) {
    std::cout << "Attempting to load cartridge" << std::endl;

    // Deallocate old cartridge if attach
    if (mp_cartridge != nullptr) {
        delete mp_cartridge;
        mp_cartridge = nullptr;
    }

    // Load the cartridge
    mp_cartridge = Cartridge::loadCartridgeFromFile(filename);
    if (mp_cartridge == nullptr) {
        std::cerr << "Cartridge loading Failed" << std::endl;
        return 1;
    }

    // Attach cartridge to the cpu and ppu bus and reset the CPU
    m_cpuBus.attachCartriadge(mp_cartridge);
    m_ppuBus.attachCartriadge(mp_cartridge);
    m_cpuBus.m_cpu.reset();
    m_ppuBus.m_ppu.reset();

    return 0;
}

bool NesEmulator::frameReady() {
    return m_ppuBus.m_ppu.frameReady();
}
// Load the color palette from file
int NesEmulator::loadPalette(const std::string& filename) {
    return m_frameBuffer.loadPalette(filename);
}

/*
 *
 *  Debug functions
 *
 */

// Bus format range function
std::string NesEmulator::formatBusRange(uint16_t from, uint16_t to, uint width) {
    return m_cpuBus.formatRange(from, to, width);
}
// Decompile the instruction at the given address
std::string NesEmulator::decompileInstruction(uint16_t addr) {
    return debug::decompileInstruction(&m_cpuBus, addr);
}

// return CPU debug info
debug::Cpu6502Debug NesEmulator::cpuDebugInfo() {
    return m_cpuBus.m_cpu.getDebugInfo();
}
// return PPU debug info
debug::PPUDebug NesEmulator::ppuDebugInfo() {
    return m_ppuBus.m_ppu.getDebugInfo();
}
}
