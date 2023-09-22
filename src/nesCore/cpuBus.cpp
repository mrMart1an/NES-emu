#include "../nesPch.h"

#include "cpuBus.h"
#include "cpu/cpu6502.h"
#include "utility/utilityFunctions.h"

namespace nesCore {
// Initialize the bus and all the attached hardware
Bus::Bus() : cpu(Cpu6502(this)), ppu(nullptr), cartridge(nullptr), ioInterface(nullptr) {
    // Initializing RAM to zero
    std::fill(ram, ram + sizeof(ram), 0x00);
    m_dmaCycles = false;
}

// Return true if the CPU should be halted for DMA execution 
bool Bus::dmaCycles() {
    bool output = m_dmaCycles;
    m_dmaCycles = false;

    return output;
}

// Attach a cartridge to the bus
void Bus::attachCartriadge(Cartridge* cartridge) {
    this->cartridge = cartridge;
}
// Attach ppu to the bus 
void Bus::attachPpu(PPU* ppu) {
    this->ppu = ppu;
}
// Attach an IO interface to the bus
void Bus::attachIO(IOInterface* interface) {
    ioInterface = interface;
}

// Read a byte from the bus at the given address
uint8_t Bus::read(uint16_t addr, bool debugRead) {
    // RAM address range
    if (addr >= 0x0000 && addr <= 0x07FF) 
        return ram[addr];

    // PPU address range
    if (addr >= 0x2000 && addr <= 0x3FFF && ppu != nullptr && !debugRead)
        return ppu->readRegister(addr);

    // IO address range
    if (addr == 0x4016 && ioInterface != nullptr && !debugRead)
        return ioInterface->readInputOne();
    if (addr == 0x4017 && ioInterface != nullptr && !debugRead)
        return ioInterface->readInputTwo();

    // PRG ROM address range
    if (addr >= 0x6000 && addr <= 0xFFFF && cartridge != nullptr)
        return cartridge->cpuRead(addr);
    
    // If an invalid address is provide return 0
    return 0x00;
}

// Read a 16 bit integer stored in little endian format
uint16_t Bus::read16(uint16_t addr, bool debugRead) {
    // Use bit shifting to get the correct number
    return  static_cast<uint16_t>(this->read(addr, debugRead) | (this->read(addr + 1, debugRead) << 8)); 

    // If an invalid address is provide return 0
    return 0x0000;
}

// Read a 16 bit integer stored in little endian format
// but wrap around page boundaries 
uint16_t Bus::read16PageWrap(uint16_t addr, bool debugRead) {
    uint16_t addrByteTwo = (addr & 0xFF00) + ((addr + 1) & 0x00FF);
    // Use bit shifting to get the correct number
    return  static_cast<uint16_t>(this->read(addr, debugRead) | (this->read(addrByteTwo, debugRead) << 8)); 

    // If an invalid address is provide return 0
    return 0x0000;
}

// Write a byte to the bus at the given address
void Bus::write(uint16_t addr, uint8_t data) {
    // RAM address range
    if (addr >= 0x0000 && addr <= 0x07FF) 
        ram[addr] = data;
    
    // PPU address range
    if (addr >= 0x2000 && addr <= 0x3FFF && ppu != nullptr) 
        ppu->writeRegister(addr, data);

    // OAM DMA Write
    if (addr == 0x4014 && ppu != nullptr)
        OAMDMAtransfer(data);

    // IO address range
    if (addr == 0x4016 && ioInterface != nullptr)
        ioInterface->writeOutput(data);

    // PRG ROM address range
    else if (addr >= 0x6000 && addr <= 0xFFFF && cartridge != nullptr)
        cartridge->cpuWrite(addr, data);
}

// Write a 16 bit integer to the bus in little endian format
void Bus::write16(uint16_t addr, uint16_t data) {
    // Use bit shifting to save the correct number
    this->write(addr, static_cast<uint8_t>(data & 0x00FF));
    this->write(addr + 1, static_cast<uint8_t>((data & 0xFF00) >> 8));
}

// Write a 16 bit integer to the bus in little endian format
// but wrap around page boundaries 
void Bus::write16PageWrap(uint16_t addr, uint16_t data) {
    uint16_t addrByteTwo = (addr & 0xFF00) + ((addr + 1) & 0x00FF);
    // Use bit shifting to save the correct number
    this->write(addr, static_cast<uint8_t>(data & 0x00FF));
    this->write(addrByteTwo, static_cast<uint8_t>((data & 0xFF00) >> 8));
}

// DMA routines

// OAM DMA transfer routine 
void Bus::OAMDMAtransfer(uint8_t pageNumber) {
    m_dmaCycles = true;

    // Check if the PPU as being attached
    if (ppu == nullptr)
        return;

    uint16_t pageAddr = static_cast<uint16_t>(pageNumber) << 8;

    // Transfer loop
    this->write(0x2003, 0x00);
    uint8_t data;

    for (int i = 0; i < 256; i++) {
        data = this->read(pageAddr | i);
        this->write(0x2004, data);
    }
}

// Return a sting with a formatted region of the bus
std::string Bus::formatRange(uint16_t from, uint16_t to, uint width) {
    std::stringstream s;

    for (int addr = from; addr <= to; addr++) {
        // Add the current address to the beginning of a new line
        if ((addr - from) % width == 0) {
            s << std::endl << utility::paddedHex(addr, 4) << ":";
        }

        s << "  " << utility::paddedHex(this->read(addr, true), 2);
    }

    // End the last line and return the string
    s << std::endl;
    return s.str();
}

}
