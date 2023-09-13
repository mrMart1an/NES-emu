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

#include <algorithm>
#include <csignal>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>

namespace nesCore {
NesEmulator::NesEmulator() : m_cpuBus(), m_ppuBus(), mp_cartridge(nullptr) {
    // Setup CPU and CPU bus
    m_cpuBus.attachPpu(&m_ppuBus.ppu);
    m_cpuBus.cpu.reset();

    // Setup PPU
    m_ppuBus.ppu.attachFrameBuffer(&m_frameBuffer);

    // Set the PPU interrupt to NOINT 
    m_ppuInt = NOINT;
}
NesEmulator::~NesEmulator() {
    if (mp_cartridge != nullptr) 
        delete mp_cartridge;
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
    size_t cpuCycle = m_cpuBus.cpu.step(m_ppuInt);
    m_ppuInt = m_ppuBus.ppu.clock(cpuCycle);
}

// Load a cartridge from a file
void NesEmulator::loadCartridge(const std::string& filename) {
    std::cout << "Attempting to load cartridge" << std::endl;

    // Deallocate old cartridge if attach
    if (mp_cartridge != nullptr) {
        delete mp_cartridge;
        mp_cartridge = nullptr;
    }

    // open the file and read the header
    std::ifstream file(filename, std::ios_base::binary);

    char header[16];
    file.read(header, 16);
    
    // Check the header for a iNES file
    if (header[0] != 0x4E || header[1] != 0x45 || header[2] != 0x53 || header[3] != 0x1A)
        return;

    // Get the mapper id from flag 6 and 7
    uint8_t mapperId = (header[6] >> 4) | (header[7] & 0xF0);

    // for (int i = 0; i < 16; i++) {
    //     std::cout << std::hex << (int)header[i] << std::endl;
    // }
     
    // Get the PRG and CHR ROM size in 16Kb blocks
    uint8_t prgRomSize = header[4];
    uint8_t chrRomSize = header[5];

    // Load the program ROM
    uint8_t* prgRom = new uint8_t[16 * 1024 * prgRomSize];
    file.read((char*)prgRom, 16 * 1024 * prgRomSize);

    // Load the character ROM
    uint8_t* chrRom = new uint8_t[8 * 1024 * chrRomSize];
    file.read((char*)chrRom, 8 * 1024 * chrRomSize);

    // Get mirroring mode
    MirroringMode mirroringMode = static_cast<MirroringMode>(header[6] & 0b00000001);

    // Generate cartridge options
    CartridgeOption cartOpt;
    cartOpt.prgBanksCount = prgRomSize;
    cartOpt.chrBanksCount = chrRomSize;

    cartOpt.mirroringMode = mirroringMode;

    std::cout << "Mirroring: " << (int)mirroringMode << std::endl;

    // Generate the cartridge if supported
    switch (mapperId) {
        // Mapper 0: NROM
        case 0x00:
            mp_cartridge = new NromCartridge(prgRom, chrRom, cartOpt);
            std::cout << "Loading cartridge with mapper 0" << std::endl;
            break;

        // Mapper 3: CNROM
        case 0x03:
            mp_cartridge = new CnromCartridge(prgRom, chrRom, cartOpt);
            std::cout << "Loading cartridge with mapper 3" << std::endl;
            break;

        default:
            std::cout << "Mapper unsupported: ";
            std::cout << static_cast<int>(mapperId);
            std::cout << std::endl;
    }

    // Close the file and return
    file.close();

    // Attach cartridge to the cpu and ppu bus and reset the CPU
    m_cpuBus.attachCartriadge(mp_cartridge);
    m_ppuBus.attachCartriadge(mp_cartridge);
    m_cpuBus.cpu.reset();
    m_ppuBus.ppu.reset();
}

bool NesEmulator::frameReady() {
    return m_ppuBus.ppu.frameReady();
}
// Load the color palette from file
void NesEmulator::loadPalette(const std::string& filename) {
    m_frameBuffer.loadPalette(filename);
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
    return m_cpuBus.cpu.getDebugInfo();
}
// return PPU debug info
debug::PPUDebug NesEmulator::ppuDebugInfo() {
    return m_ppuBus.ppu.getDebugInfo();
}
}
