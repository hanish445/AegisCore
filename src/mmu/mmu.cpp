#include "mmu.h"
#include <fstream>
#include <iostream>

namespace Aegis {

MMU::MMU() {
    // Initialize memory map to 0
    memory.fill(0);
}

uint8_t MMU::read8(uint16_t address) const {
    return memory[address];
}

void MMU::write8(uint16_t address, uint8_t value) {
    // Address space 0x0000 to 0x7FFF is the ROM area.
    // Real hardware ignores writes to ROM (unless it's an MBC command, which we skip for now).
    if (address < 0x8000) {
        // Do not overwrite the game data!
        return; 
    }
    
    // For now, allow flat writes to the rest of the memory map (VRAM, WRAM, HRAM, etc.)
    memory[address] = value;
}

uint16_t MMU::read16(uint16_t address) const {
    // The Game Boy CPU is Little-Endian. 
    // This means the Least Significant Byte (LSB) is stored at the lower address.
    uint8_t lo = read8(address);
    uint8_t hi = read8(address + 1);
    return (hi << 8) | lo;
}

void MMU::write16(uint16_t address, uint16_t value) {
    // Write LSB first, then MSB
    write8(address, value & 0xFF);
    write8(address + 1, (value >> 8) & 0xFF);
}

bool MMU::load_rom(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[MMU] Error: Could not open ROM file: " << filepath << std::endl;
        return false;
    }

    // Read up to 32KB (0x8000 bytes) into the ROM area for flat unmapped games like Tetris
    file.read(reinterpret_cast<char*>(memory.data()), 0x8000);
    
    if (file) {
        std::cout << "[MMU] Loaded ROM: " << filepath << " successfully." << std::endl;
    } else {
        std::cout << "[MMU] Loaded partial ROM (less than 32KB) or hit EOF: " << filepath << std::endl;
    }
    
    file.close();
    return true;
}

} // namespace Aegis