#include "nesPch.h"

#include "cartridge.h"
#include "cnromCartridge.h"

namespace nesCore {
// Take a pointer to the program ROM and the number of 16Kb memory banks in the ROM
// If a nullptr is provided allocate an empty memory initialized to zero
CnromCartridge::CnromCartridge(uint8_t* p_prgRom, uint8_t* p_chrRom, CartridgeOption cartOpt) {
    // Store the number of memory banks
    if (cartOpt.prgBanksCount > 2)
        cartOpt.prgBanksCount = 2;
    if (cartOpt.prgBanksCount == 0)
        cartOpt.prgBanksCount = 1;
    if (cartOpt.chrBanksCount == 0)
        cartOpt.chrBanksCount = 1;

    m_prgBanksCount = cartOpt.prgBanksCount;
    m_chrBanksCount = cartOpt.chrBanksCount;

    // Prg ram size is always one for compatibility 
    cartOpt.prgRamBanksCount = 1;
    mp_prgRam = new uint8_t[8 * 1024];
    std::fill(mp_prgRam, mp_prgRam + (8 * 1024), 0x00);

    // If a null pointer is given to p_prgRom allocate a 16Kb memory bank
    if (p_prgRom != nullptr) {
        mp_prgRom = p_prgRom;
    } else {
        mp_prgRom = new uint8_t[16 * 1024 * cartOpt.prgBanksCount];
        std::fill(mp_prgRom, mp_prgRom + (16 * 1024 * cartOpt.chrBanksCount), 0x00);
    }

    // If a null pointer to p_chrRom is given allocate a 16Kb memory bank
    if (p_chrRom != nullptr) {
        mp_chrRom = p_chrRom;
    } else {
        mp_chrRom = new uint8_t[8 * 1024 * cartOpt.chrBanksCount];
        std::fill(mp_chrRom, mp_chrRom + (8 * 1024 * cartOpt.chrBanksCount), 0x00);
    }

    // Set the banks window pointer to the beginning of chr memory 
    mp_chrWindow = mp_chrRom;

    // Store mirroring mode
    m_mirroringMode = cartOpt.mirroringMode;
}

// Deallocate the memory banks
CnromCartridge::~CnromCartridge() {
    delete[] mp_prgRom;
    delete[] mp_prgRam;
    delete[] mp_chrRom;
}

// Handle reset signal
void CnromCartridge::reset() {
    mp_chrWindow = mp_chrRom;
}

// Write to a specific address of the cartridge
void CnromCartridge::cpuWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0x6000 && addr <= 0x7FFF)
        mp_prgRam[addr - 0x6000] = data;
    
    // Move the ppu data window
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        // Perform the modulo of the requested window and 
        // the number of banks to avoid overflow
        data %= m_chrBanksCount;

        mp_chrWindow = mp_chrRom + (8 * 1024 * data);
    }
}

// Read to a specific address of the cartridge
uint8_t CnromCartridge::cpuRead(uint16_t addr) {
    if (addr >= 0x6000 && addr <= 0x7FFF)
        return mp_prgRam[addr - 0x6000];
    
    if (addr >= 0x8000 && addr <= 0xFFFF)
        return mp_prgRom[(addr - 0x8000) % (m_prgBanksCount * 16 * 1024)];
    
    return 0x00;
}

// Read to a specific address of the cartridge
uint8_t CnromCartridge::ppuRead(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        return mp_chrWindow[addr];
    
    return 0x00;
}

// Return the mirroring mode
MirroringMode CnromCartridge::getMirroringMode() {
    return m_mirroringMode;
}
}
