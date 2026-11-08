#pragma once
#include "registers.h"
#include "../mmu/mmu.h"
#include <cstdint>

namespace Aegis {
    class CPU {
        private:
            Registers& reg;;
            MMU& mmu;

            void execute_opcode(uint8_t opcode);
        public:
            CPU(Registers& registers, MMU& memory_unit);
            void step();
    };
}