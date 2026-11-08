#include "mmu.h"
#include <fstream>
#include <iostream>

namespace Aegis {

MMU::MMU() {
    // Initialize memory map to 0
    memory.fill(0);
}

uint8_t MMU::read8(uint16_t address) const {
    // Intercept Joypad reads
    if (address == 0xFF00) {
        // Read the selection bits the CPU just wrote (bits 4 and 5)
        // Note: Assuming your raw memory array is called `memory` or similar. 
        // Adjust the variable name below to match your actual internal array.
        uint8_t request = memory[0xFF00]; 
        
        uint8_t val = 0xCF; // Bits 6 & 7 are always 1, bits 0-3 default to 1 (unpressed)
        
        // Bit 5 (0x20) = Select Action Buttons
        if (!(request & 0x20)) val &= joypad_action; 
        
        // Bit 4 (0x10) = Select Direction Buttons
        if (!(request & 0x10)) val &= joypad_dir;    
        
        return val;
    }
    return memory[address];
}

void MMU::write8(uint16_t address, uint8_t value) {
    // Address space 0x0000 to 0x7FFF is the ROM area.
    // Real hardware ignores writes to ROM (unless it's an MBC command, which we skip for now).
    if (address < 0x8000) {
        // Do not overwrite the game data!
        return; 
    }
    if(address == 0xFF02 && value == 0x81){
        //0x81 --> the CPU is requesting a serial transfer
        //intercept the character currently sitting in 0xFF01 and print it natively
        char c = static_cast<char>(memory[0xFF01]);
        std::cout << c << std::flush;

        memory[0xFF02] = 0;
        return;
    }
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

void MMU::update_joypad(uint8_t action, uint8_t dir) {
    // If a button transitioned from unpressed (1) to pressed (0), trigger interrupt
    bool button_pressed = false;
    if ((joypad_action & ~action) || (joypad_dir & ~dir)) {
        button_pressed = true;
    }

    joypad_action = action;
    joypad_dir = dir;

    // Request Joypad Interrupt in the IF register (0xFF0F)
    if (button_pressed) {
        write8(0xFF0F, read8(0xFF0F) | 0x10); 
    }
}

} // namespace Aegis