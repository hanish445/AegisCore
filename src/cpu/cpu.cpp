#include "cpu.h"
#include <iostream>
#include <iomanip>
#include <cstdlib> // For exit()

namespace Aegis {

CPU::CPU(Registers& registers, MMU& memory_unit) 
    : reg(registers), mmu(memory_unit) {}

void CPU::step() {
    // 1. Fetch the opcode at the current Program Counter
    uint8_t opcode = mmu.read8(reg.pc);
    
    // Debug: Print the current instruction being executed
    std::cout << "[CPU] Executing PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << reg.pc 
              << " | Opcode: 0x" << std::setw(2) << static_cast<int>(opcode) << std::dec << std::endl;

    // 2. Increment PC to point to the next byte (operands or next instruction)
    reg.pc++;

    // 3. Decode & Execute
    execute_opcode(opcode);
}

void CPU::execute_opcode(uint8_t opcode) {
    switch (opcode) {
        case 0x00: 
            // NOP (No Operation) - Takes 1 byte, does nothing.
            break;
            
        // We will fill out the rest of the 256 opcodes in Step 6
        
        default:
            std::cerr << "[CPU] Fatal: Unimplemented Opcode 0x" 
                      << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode) 
                      << " at PC 0x" << (reg.pc - 1) << std::endl;
            // Halt execution if we hit an unknown instruction to prevent runaway state
            std::exit(1); 
    }
}

}