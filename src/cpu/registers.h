#pragma once
#include <cstdint>

namespace Aegis {

struct Registers {
    // 8-bit registers
    uint8_t a, f; // f is the Flags register
    uint8_t b, c;
    uint8_t d, e;
    uint8_t h, l;

    // 16-bit special registers
    uint16_t sp; // Stack Pointer
    uint16_t pc; // Program Counter

    // Flag Bitmasks for the F register
    // Bit 7: Zero Flag (Z)
    // Bit 6: Subtract Flag (N)
    // Bit 5: Half Carry Flag (H)
    // Bit 4: Carry Flag (C)
    // Bits 0-3 are always 0.
    static constexpr uint8_t FLAG_Z = 1 << 7;
    static constexpr uint8_t FLAG_N = 1 << 6;
    static constexpr uint8_t FLAG_H = 1 << 5;
    static constexpr uint8_t FLAG_C = 1 << 4;

    // Initialize registers (Boot ROM values roughly look like this)
    Registers() : a(0x01), f(0xB0), b(0x00), c(0x13), 
                  d(0x00), e(0xD8), h(0x01), l(0x4D), 
                  sp(0xFFFE), pc(0x0100) {}

    // --- 16-bit Virtual Register Getters & Setters ---
    
    uint16_t get_af() const { return (a << 8) | f; }
    void set_af(uint16_t val) { 
        a = (val >> 8) & 0xFF; 
        f = val & 0xF0; // The lower 4 bits of F are hardwired to 0
    }

    uint16_t get_bc() const { return (b << 8) | c; }
    void set_bc(uint16_t val) { 
        b = (val >> 8) & 0xFF; 
        c = val & 0xFF; 
    }

    uint16_t get_de() const { return (d << 8) | e; }
    void set_de(uint16_t val) { 
        d = (val >> 8) & 0xFF; 
        e = val & 0xFF; 
    }

    uint16_t get_hl() const { return (h << 8) | l; }
    void set_hl(uint16_t val) { 
        h = (val >> 8) & 0xFF; 
        l = val & 0xFF; 
    }

    // --- Flag Manipulation Helpers ---

    void set_flag(uint8_t flag, bool value) {
        if (value) {
            f |= flag;
        } else {
            f &= ~flag;
        }
    }

    bool get_flag(uint8_t flag) const {
        return (f & flag) != 0;
    }
};

} // namespace Aegis