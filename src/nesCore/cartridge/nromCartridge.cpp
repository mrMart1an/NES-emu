#include "nromCartridge.h"
#include "cartridge.h"
#include <algorithm>
#include <iostream>

namespace nesCore {
// Take a pointer to the program ROM and the number of 16Kb memory banks in the ROM
// If a nullptr is provided allocate an empty memory initialized to zero
NromCartridge::NromCartridge(uint8_t* p_prgRom, uint8_t* p_chrRom, CartridgeOption cartOpt) {
    // Store the number of memory banks
    if (cartOpt.prgBanksCount > 2)
        cartOpt.prgBanksCount = 2;
    if (cartOpt.prgBanksCount == 0)
        cartOpt.prgBanksCount = 1;

    m_banksCount = cartOpt.prgBanksCount;

    // If a null pointer is given to p_prgRom allocate a 16Kb memory bank
    if (p_prgRom != nullptr) {
        mp_prgRom = p_prgRom;
    } else {
        mp_prgRom = new uint8_t[16 * 1024 * cartOpt.prgBanksCount];
        std::fill(mp_prgRom, mp_prgRom + (16 * 1024 * cartOpt.prgBanksCount), 0x00);
    }

    // If a null pointer to p_chrRom is given allocate a 16Kb memory bank
    if (p_chrRom != nullptr) {
        mp_chrRom = p_chrRom;
    } else {
        mp_chrRom = new uint8_t[8 * 1024];
        std::fill(mp_chrRom, mp_chrRom + (8 * 1024), 0x00);
    }

    // Store mirroring mode
    m_mirroringMode = cartOpt.mirroringMode;
}

// Deallocate the memory banks
NromCartridge::~NromCartridge() {
    delete[] mp_prgRom;
    delete[] mp_chrRom;
}

// Write to a specific address of the cartridge
void NromCartridge::cpuWrite(uint16_t addr, uint8_t data) {}

// Read to a specific address of the cartridge
uint8_t NromCartridge::cpuRead(uint16_t addr) {
    if (addr >= 0x8000 && addr <= 0xFFFF)
        return mp_prgRom[(addr - 0x8000) % (m_banksCount * 16 * 1024)];
    
    return 0x00;
}

// Read to a specific address of the cartridge
uint8_t NromCartridge::ppuRead(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        return mp_chrRom[addr];
    
    return 0x00;
}

// Return the mirroring mode
MirroringMode NromCartridge::getMirroringMode() {
    return m_mirroringMode;
}
}