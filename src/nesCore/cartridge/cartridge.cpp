#include "nesPch.h"

#include "cartridge.h"
#include "nromCartridge.h"
#include "cnromCartridge.h"

namespace nesCore {
// Parse file header for iNES and NES 2
CartridgeOption iNESparse(uint8_t* header);
CartridgeOption NES2parse(uint8_t* header);

Cartridge* Cartridge::loadCartridgeFromFile(const std::string& filename) {
   // open the file and read the header
    std::ifstream file(filename, std::ios_base::binary);

    // Check if the given palette file exist
    if (!file.good()) {
        return nullptr;
    }

    uint8_t header[16];
    file.read((char*)header, 16);

    // Check the header for a iNES file
    if (header[0] != 0x4E || header[1] != 0x45 || header[2] != 0x53 || header[3] != 0x1A)
        return nullptr;

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
            return nullptr;
    }
            
    // Generate the cartridge if supported
    Cartridge* outputCartridge = nullptr;
    switch (cartOpt.mapperId) {
        // Mapper 0: NROM
        case 0x00:
            outputCartridge = new NromCartridge(prgRom, chrRom, cartOpt);
            break;

        // Mapper 3: CNROM
        case 0x03:
            outputCartridge = new CnromCartridge(prgRom, chrRom, cartOpt);
            break;

        default:
            std::cout << "Mapper unsupported";
            std::cout << std::endl;
    }

    // Close the file and return
    file.close();

    return outputCartridge;
}

CartridgeOption iNESparse(uint8_t* header) {
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

CartridgeOption NES2parse(uint8_t* header) {
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
}
