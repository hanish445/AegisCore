#pragma once
#include "registers.h"
#include "../mmu/mmu.h"
#include <cstdint>

namespace Aegis {

class CPU {
private:
    Registers& reg;
    MMU& mmu;
    bool ime = false;

    // Fetch helpers
    uint8_t fetch8();
    uint16_t fetch16();

    // --- ALU Helpers ---
    void add8(uint8_t value);
    void adc8(uint8_t value);
    void sub8(uint8_t value);
    void sbc8(uint8_t value);
    void and8(uint8_t value);
    void or8(uint8_t value);
    void xor8(uint8_t value);
    void cp8(uint8_t value);
    uint8_t inc8(uint8_t value);
    uint8_t dec8(uint8_t value);

    void execute_opcode(uint8_t opcode);

public:
    CPU(Registers& registers, MMU& memory_unit);
    void step();
};

} // namespace Aegis