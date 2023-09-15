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
int NesEmulator::loadCartridge(const std::string& filename) {
    std::cout << "Attempting to load cartridge" << std::endl;

    // Deallocate old cartridge if attach
    if (mp_cartridge != nullptr) {
        delete mp_cartridge;
        mp_cartridge = nullptr;
    }

    // open the file and read the header
    std::ifstream file(filename, std::ios_base::binary);

    // Check if the given palette file exist
    if (!file.good()) {
        return 1;
    }

    uint8_t header[16];
    file.read((char*)header, 16);

    // Check the header for a iNES file
    if (header[0] != 0x4E || header[1] != 0x45 || header[2] != 0x53 || header[3] != 0x1A)
        return 2;

    // Check for NES2 rom format and parse cartridge options
    bool NES2Format = (header[7] & 0x0C) == 0x08;
    CartridgeOption cartOpt = NES2Format ? iNESparse(header) : NES2parse(header);
    
    // Load the program ROM
    uint8_t* prgRom = new uint8_t[16 * 1024 * cartOpt.prgBanksCount];
    file.read((char*)prgRom, 16 * 1024 * cartOpt.prgBanksCount);

    // Load the character ROM
    uint8_t* chrRom = new uint8_t[8 * 1024 * cartOpt.chrBanksCount];
    file.read((char*)chrRom, 8 * 1024 * cartOpt.chrBanksCount);

    // Print file information
    if (NES2Format)
        std::cout << "ROM format: NES 2.0" << std::endl;
    else
        std::cout << "ROM format: iNES" << std::endl;

    std::cout << "Loading cartridge with mapper: ";
    std::cout << cartOpt.mapperId << std::endl;

    switch (cartOpt.mirroringMode) {
        case HORIZONTAL_MIRRORING:
            std::cout << "Mirroring: horizontal" << std::endl;
            break;

        case VERTICAL_MIRRORING:
            std::cout << "Mirroring: vertical" << std::endl;
            break;

        case FOUR_SCREEN:
            std::cout << "Mirroring: four screen(Not supported)" << std::endl;
            return 2;
    }
            
    // Generate the cartridge if supported
    switch (cartOpt.mapperId) {
        // Mapper 0: NROM
        case 0x00:
            mp_cartridge = new NromCartridge(prgRom, chrRom, cartOpt);
            break;

        // Mapper 3: CNROM
        case 0x03:
            mp_cartridge = new CnromCartridge(prgRom, chrRom, cartOpt);
            break;

        default:
            std::cout << "Mapper unsupported";
            std::cout << std::endl;
            return 2;
    }

    // Close the file and return
    file.close();

    // Attach cartridge to the cpu and ppu bus and reset the CPU
    m_cpuBus.attachCartriadge(mp_cartridge);
    m_ppuBus.attachCartriadge(mp_cartridge);
    m_cpuBus.cpu.reset();
    m_ppuBus.ppu.reset();

    return 0;
}

// iNES file header parser
CartridgeOption NesEmulator::iNESparse(uint8_t* header) {
    CartridgeOption outputOpt;
    outputOpt.NES2format = false;

    // Get the mapper id from flag 6 and 7
    outputOpt.mapperId = (header[6] >> 4) | (header[7] & 0xF0);

    // Generate cartridge options 
    // Get the PRG and CHR ROM size in 16Kb blocks and get mirroring mode
    outputOpt.prgBanksCount = header[4];
    outputOpt.chrBanksCount = header[5];
    outputOpt.prgRamBanksCount = header[8];

    // Get mirroring mode
    if (header[6] & 0x04) {
        outputOpt.mirroringMode = FOUR_SCREEN;
    } else {
        if (header[6] & 0x01)
            outputOpt.mirroringMode = VERTICAL_MIRRORING;
        else
            outputOpt.mirroringMode = HORIZONTAL_MIRRORING;
    }

    return outputOpt;
}

// NES2 file header parser
CartridgeOption NesEmulator::NES2parse(uint8_t* header) {
    CartridgeOption outputOpt;
    outputOpt.NES2format = true;

    // Get the mapper id from flag 6 and 7
    outputOpt.mapperId = ((header[8] & 0x0F) << 8) | (header[7] & 0xF0) | (header[6] >> 4);

    // Generate cartridge options 
    // Get the PRG and CHR ROM size in 16Kb blocks and get mirroring mode
    outputOpt.prgBanksCount = header[4];
    outputOpt.chrBanksCount = header[5];
    outputOpt.prgRamBanksCount = 1;

    // Get mirroring mode
    if (header[6] & 0x04) {
        outputOpt.mirroringMode = FOUR_SCREEN;
    } else {
        if (header[6] & 0x01)
            outputOpt.mirroringMode = VERTICAL_MIRRORING;
        else
            outputOpt.mirroringMode = HORIZONTAL_MIRRORING;
    }

    return outputOpt;
}

bool NesEmulator::frameReady() {
    return m_ppuBus.ppu.frameReady();
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
    return m_cpuBus.cpu.getDebugInfo();
}
// return PPU debug info
debug::PPUDebug NesEmulator::ppuDebugInfo() {
    return m_ppuBus.ppu.getDebugInfo();
}
}
