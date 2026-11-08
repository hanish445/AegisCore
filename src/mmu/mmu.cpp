#include "mmu.h"
#include <fstream>
#include <iostream>

namespace Aegis {

MMU::MMU() {
    // Initialize memory map to 0
    memory.fill(0);
}

uint8_t MMU::read8(uint16_t address) const {
    if (address == 0xFF00) {
        uint8_t request = memory[0xFF00]; 
        uint8_t val = 0xCF; 
        if (!(request & 0x20)) val &= joypad_action; 
        if (!(request & 0x10)) val &= joypad_dir;    
        return val;
    }

    // ROM Bank 0 (0x0000 - 0x3FFF)
    if (address < 0x4000) {
        if (rom_data.empty()) return 0;
        return rom_data[address];
    }

    // Switched ROM Bank (0x4000 - 0x7FFF)
    if (address < 0x8000) {
        if (rom_data.empty()) return 0;
        int bank = current_rom_bank;
        if (cart_type == MBC3 && bank == 0) bank = 1;
        size_t mapped_address = (bank * 0x4000) + (address - 0x4000);
        if (mapped_address < rom_data.size()) {
            return rom_data[mapped_address];
        }
        return 0;
    }

    // External Cartridge RAM / RTC (0xA000 - 0xBFFF)
    if (address >= 0xA000 && address < 0xC000) {
        if (!ram_enabled) return 0xFF;
        if (cart_type == MBC3 && current_ram_bank >= 0x08 && current_ram_bank <= 0x0C) {
            return 0; // RTC Register stub
        }
        if (external_ram.empty()) return 0xFF;
        size_t ram_address = (current_ram_bank * 0x2000) + (address - 0xA000);
        if (ram_address < external_ram.size()) {
            return external_ram[ram_address];
        }
        return 0xFF;
    }

    return memory[address];
}

void MMU::write8(uint16_t address, uint8_t value) {
    if (cart_type == MBC1 || cart_type == MBC3) {
        if (address < 0x2000) {
            ram_enabled = ((value & 0x0F) == 0x0A);
            return;
        }
        if (address < 0x4000) {
            int bank = value & (cart_type == MBC3 ? 0x7F : 0x1F);
            if (bank == 0) bank = 1;
            current_rom_bank = bank;
            return;
        }
        if (address < 0x6000) {
            if (cart_type == MBC3) {
                current_ram_bank = value; // RAM bank 0-3 or RTC register 0x08-0x0C
            } else {
                if (banking_mode) {
                    current_ram_bank = value & 0x03;
                } else {
                    current_rom_bank = ((value & 0x03) << 5) | (current_rom_bank & 0x1F);
                }
            }
            return;
        }
        if (address < 0x8000) {
            banking_mode = (value & 0x01);
            return;
        }
    } else if (address < 0x8000) {
        return; // Ignore ROM-only writes
    }

    // External RAM Writes (0xA000 - 0xBFFF)
    if (address >= 0xA000 && address < 0xC000) {
        if (ram_enabled) {
            if (cart_type == MBC3 && current_ram_bank >= 0x08 && current_ram_bank <= 0x0C) {
                return; // RTC write stub
            }
            if (!external_ram.empty()) {
                size_t ram_address = (current_ram_bank * 0x2000) + (address - 0xA000);
                if (ram_address < external_ram.size()) {
                    external_ram[ram_address] = value;
                }
            }
        }
        return;
    }

    if (address == 0xFF04) {
        memory[0xFF04] = 0;
        return;
    }

    if (address == 0xFF46) {
        uint16_t source_base = static_cast<uint16_t>(value) << 8;
        for (int i = 0; i < 160; i++) {
            uint8_t b = read8(source_base + i);
            memory[0xFE00 + i] = b;
        }
        return;
    }

    if (address == 0xFF02 && value == 0x81) {
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

// Update load_rom to support full variable-size files:
bool MMU::load_rom(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "[MMU] Error: Could not open ROM file: " << filepath << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    rom_data.resize(size);
    if (file.read(reinterpret_cast<char*>(rom_data.data()), size)) {
        std::cout << "[MMU] Loaded ROM: " << filepath << " (" << size << " bytes) successfully." << std::endl;
    } else {
        std::cerr << "[MMU] Error: Failed to read ROM data." << std::endl;
        return false;
    }
    file.close();

    // Automatically Detect Cartridge Type from Header at 0x0147
    if (rom_data.size() > 0x0147) {
        uint8_t type_byte = rom_data[0x0147];
        if (type_byte >= 0x01 && type_byte <= 0x03) {
            cart_type = MBC1;
            std::cout << "[MMU] Cartridge Type: MBC1 detected." << std::endl;
        } else if (type_byte >= 0x0F && type_byte <= 0x13) {
            cart_type = MBC3;
            std::cout << "[MMU] Cartridge Type: MBC3 (Pokemon compatible) detected." << std::endl;
        } else {
            cart_type = ROM_ONLY;
            std::cout << "[MMU] Cartridge Type: ROM-Only detected." << std::endl;
        }
    }

    external_ram.resize(0x8000, 0); // Allocate 32KB max external RAM
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