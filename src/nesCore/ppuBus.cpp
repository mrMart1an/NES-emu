#include "nesPch.h"

#include "cartridge/cartridge.h"
#include "ppuBus.h"
#include "ppu/ppu.h"

namespace nesCore {
PpuBus::PpuBus() : m_ppu(this), mp_cartridge(nullptr) {
    // Initializing RAM to zero
    std::fill(mp_vram, mp_vram + sizeof(mp_vram), 0x00);

    // Initialize vram mirroring to horizontal
    this->setMirroringMode(HORIZONTAL_MIRRORING);
};

// Attach a cartridge to the PPU bus
void PpuBus::attachCartriadge(Cartridge* cartridge) {
    this->mp_cartridge = cartridge;
    this->setMirroringMode(cartridge->getMirroringMode());
}

// Set mirroring mode
void PpuBus::setMirroringMode(MirroringMode mode) {
    if (mode == HORIZONTAL_MIRRORING) {
        mp_nametableOne = &mp_vram[0x0000];
        mp_nametableTwo = &mp_vram[0x0000];
        mp_nametableThree = &mp_vram[0x0400];
        mp_nametableFour = &mp_vram[0x0400];
    } else if (mode == VERTICAL_MIRRORING) {
        mp_nametableOne = &mp_vram[0x0000];
        mp_nametableTwo = &mp_vram[0x0400];
        mp_nametableThree = &mp_vram[0x0000];
        mp_nametableFour = &mp_vram[0x0400];
    } else {
        std::cerr << "Unsupported mirroring mode" << std::endl;
    }
}

// Read a byte from the PPU PpuBus
uint8_t PpuBus::read(uint16_t inAddr) {
    // Mirrors the 0x0000 to 0x3FFF address range
    uint16_t addr = inAddr % 0x4000;

    // Cartridge pattern table address range
    if (addr >= 0x0000 && addr <= 0x1FFF && mp_cartridge != nullptr) 
        return mp_cartridge->ppuRead(addr);

    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        uint16_t addrVram = (addr - 0x2000) % 0x1000;

        if (addrVram >= 0x0000 && addrVram <= 0x03FF)
            return mp_nametableOne[addrVram]; 
        else if (addrVram >= 0x0400 && addrVram <= 0x07FF)
            return mp_nametableTwo[addrVram - 0x0400]; 
        else if (addrVram >= 0x0800 && addrVram <= 0x0BFF)
            return mp_nametableThree[addrVram - 0x0800]; 
        else if (addrVram >= 0x0C00 && addrVram <= 0x0FFF)
            return mp_nametableFour[addrVram - 0x0C00]; 
    } 
    
    // Read palette data
    else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        uint16_t addrPalette = (addr - 0x3F00) & 0x001F;

        // Mirror the background bytes
        uint16_t bgAddr = addrPalette & 0x0F;
        if (bgAddr == 0 || bgAddr == 0x04 || bgAddr == 0x08)
            addrPalette = bgAddr;

        return mp_palette[addrPalette];
    } 

    return 0x00;
}

// Write byte to the bus
void PpuBus::write(uint16_t inAddr, uint8_t data) {
    // Mirrors the 0x0000 to 0x3FFF address range
    uint16_t addr = inAddr % 0x4000;

    if (addr >= 0x2000 && addr <= 0x3EFF) {
        uint16_t addrVram = (addr - 0x2000) % 0x1000;

        if (addrVram >= 0x0000 && addrVram <= 0x03FF)
            mp_nametableOne[addrVram] = data; 
        else if (addrVram >= 0x0400 && addrVram <= 0x07FF)
           mp_nametableTwo[addrVram - 0x0400] = data; 
        else if (addrVram >= 0x0800 && addrVram <= 0x0BFF)
            mp_nametableThree[addrVram - 0x0800] = data; 
        else if (addrVram >= 0x0C00 && addrVram <= 0x0FFF)
           mp_nametableFour[addrVram - 0x0C00] = data; 
    } 
    
    // Write palette data
    else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        uint16_t addrPalette = (addr - 0x3F00) & 0x001F;

        // Mirror the background bytes
        uint16_t bgAddr = addrPalette & 0x0F;
        if (bgAddr == 0 || bgAddr == 0x04 || bgAddr == 0x08)
            addrPalette = bgAddr;

        mp_palette[addrPalette] = data;
    } 
}
} 
