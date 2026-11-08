#pragma once
#include <cstdint>
#include <array>
#include <string>

namespace Aegis {

class MMU {
private:
    // The Game Boy has a 16-bit address bus, meaning it can access 64KB of memory.
    // 0x0000 - 0xFFFF
    std::array<uint8_t, 0x10000> memory{};

public:
    MMU();

    // Joypad states (0 = pressed, 1 = unpressed)
    uint8_t joypad_action = 0x0F;
    uint8_t joypad_dir = 0x0F;

    void update_joypad(uint8_t action, uint8_t dir);

    // Core read/write operations
    uint8_t read8(uint16_t address) const;
    void write8(uint16_t address, uint8_t value);

    // 16-bit helper operations (Game Boy is Little-Endian)
    uint16_t read16(uint16_t address) const;
    void write16(uint16_t address, uint16_t value);

    // Utility to load a ROM file into the memory array
    bool load_rom(const std::string& filepath);
};

} // namespace Aegis