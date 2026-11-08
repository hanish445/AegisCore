#include "cpu.h"
#include <iostream>
#include <iomanip>
#include <cstdlib> // For exit()
#include <fstream> //Für das schreiben in Dateien

const int BASE_CYCLES[256] = {
    4, 12, 8,  8,  4,  4,  12, 4,  20, 8,  8,  8,  4,  4,  12, 4,
    4, 12, 8,  8,  4,  4,  12, 4,  12, 8,  8,  8,  4,  4,  12, 4,
    8, 12, 8,  8,  4,  4,  12, 4,  8,  8,  8,  8,  4,  4,  12, 4,
    8, 12, 8,  8,  12, 12, 12, 4,  8,  8,  8,  8,  4,  4,  12, 4,
    4, 4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4, 4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4, 4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8, 8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
    4, 4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4, 4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4, 4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4, 4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8, 12, 12, 16, 12, 16, 8,  16, 8,  16, 12, 4,  12, 24, 8,  16,
    8, 12, 12, 0,  12, 16, 8,  16, 8,  16, 12, 0,  12, 0,  8,  16,
    12,12, 8,  0,  0,  16, 8,  16, 16, 4,  16, 0,  0,  0,  8,  16,
    12,12, 8,  4,  0,  16, 8,  16, 12, 8,  16, 4,  0,  0,  8,  16
};

namespace Aegis {

CPU::CPU(Registers& registers, MMU& memory_unit) 
    : reg(registers), mmu(memory_unit) {}

uint8_t CPU::fetch8() {
    uint8_t val = mmu.read8(reg.pc);
    reg.pc++;
    return val;
}

uint16_t CPU::fetch16(){
    uint16_t val = mmu.read16(reg.pc);
    reg.pc += 2;
    return val;
}

// --- ALU Helper Implementations ---
void CPU::add8(uint8_t value) {
    uint16_t result = reg.a + value;
    reg.set_flag(Registers::FLAG_Z, (result & 0xFF) == 0);
    reg.set_flag(Registers::FLAG_N, false);
    reg.set_flag(Registers::FLAG_H, ((reg.a & 0x0F) + (value & 0x0F)) > 0x0F);
    reg.set_flag(Registers::FLAG_C, result > 0xFF);
    reg.a = static_cast<uint8_t>(result);
}

void CPU::adc8(uint8_t value) {
    uint8_t carry = reg.get_flag(Registers::FLAG_C) ? 1 : 0;
    uint16_t result = reg.a + value + carry;
    reg.set_flag(Registers::FLAG_Z, (result & 0xFF) == 0);
    reg.set_flag(Registers::FLAG_N, false);
    reg.set_flag(Registers::FLAG_H, ((reg.a & 0x0F) + (value & 0x0F) + carry) > 0x0F);
    reg.set_flag(Registers::FLAG_C, result > 0xFF);
    reg.a = static_cast<uint8_t>(result);
}

void CPU::sub8(uint8_t value) {
    reg.set_flag(Registers::FLAG_Z, reg.a == value);
    reg.set_flag(Registers::FLAG_N, true);
    reg.set_flag(Registers::FLAG_H, (reg.a & 0x0F) < (value & 0x0F));
    reg.set_flag(Registers::FLAG_C, reg.a < value);
    reg.a -= value;
}

void CPU::sbc8(uint8_t value) {
    uint8_t carry = reg.get_flag(Registers::FLAG_C) ? 1 : 0;
    int result = reg.a - value - carry;
    reg.set_flag(Registers::FLAG_Z, (result & 0xFF) == 0);
    reg.set_flag(Registers::FLAG_N, true);
    reg.set_flag(Registers::FLAG_H, (reg.a & 0x0F) - (value & 0x0F) - carry < 0);
    reg.set_flag(Registers::FLAG_C, result < 0);
    reg.a = static_cast<uint8_t>(result);
}

void CPU::and8(uint8_t value) {
    reg.a &= value;
    reg.set_flag(Registers::FLAG_Z, reg.a == 0);
    reg.set_flag(Registers::FLAG_N, false);
    reg.set_flag(Registers::FLAG_H, true);
    reg.set_flag(Registers::FLAG_C, false);
}

void CPU::or8(uint8_t value) {
    reg.a |= value;
    reg.set_flag(Registers::FLAG_Z, reg.a == 0);
    reg.set_flag(Registers::FLAG_N, false);
    reg.set_flag(Registers::FLAG_H, false);
    reg.set_flag(Registers::FLAG_C, false);
}

void CPU::xor8(uint8_t value) {
    reg.a ^= value;
    reg.set_flag(Registers::FLAG_Z, reg.a == 0);
    reg.set_flag(Registers::FLAG_N, false);
    reg.set_flag(Registers::FLAG_H, false);
    reg.set_flag(Registers::FLAG_C, false);
}

void CPU::cp8(uint8_t value) {
    reg.set_flag(Registers::FLAG_Z, reg.a == value);
    reg.set_flag(Registers::FLAG_N, true);
    reg.set_flag(Registers::FLAG_H, (reg.a & 0x0F) < (value & 0x0F));
    reg.set_flag(Registers::FLAG_C, reg.a < value);
}

uint8_t CPU::inc8(uint8_t value) {
    uint8_t result = value + 1;
    reg.set_flag(Registers::FLAG_Z, result == 0);
    reg.set_flag(Registers::FLAG_N, false);
    reg.set_flag(Registers::FLAG_H, (value & 0x0F) == 0x0F);
    return result;
}

uint8_t CPU::dec8(uint8_t value) {
    uint8_t result = value - 1;
    reg.set_flag(Registers::FLAG_Z, result == 0);
    reg.set_flag(Registers::FLAG_N, true);
    reg.set_flag(Registers::FLAG_H, (value & 0x0F) == 0x00);
    return result;
}

void CPU::update_timers(int cycles) {
    // 1. DIV Register (increments at 16,384 Hz -> every 256 T-cycles)
    divider_counter += cycles;
    if (divider_counter >= 256) {
        divider_counter -= 256;
        mmu.tick_div();
    }

    // 2. TIMA Timer
    uint8_t tac = mmu.read8(0xFF07);
    if (tac & 0x04) { // Is timer enabled?
        timer_counter += cycles;
        
        // Determine threshold based on TAC bits 0-2
        int threshold = 1024; // Default for 4096 Hz
        switch (tac & 0x03) {
            case 0: threshold = 1024; break; // 4096 Hz
            case 1: threshold = 16;   break; // 262143 Hz
            case 2: threshold = 64;   break; // 65536 Hz
            case 3: threshold = 256;  break; // 16384 Hz
        }

        while (timer_counter >= threshold) {
            timer_counter -= threshold;
            uint8_t tima = mmu.read8(0xFF05);
            if (tima == 0xFF) {
                // TIMA overflow! Reset to TMA and request Timer Interrupt (Bit 2 of 0xFF0F)
                mmu.write8(0xFF05, mmu.read8(0xFF06)); // Reload with TMA (0xFF06)
                mmu.write8(0xFF0F, mmu.read8(0xFF0F) | 0x04); 
            } else {
                mmu.write8(0xFF05, tima + 1);
            }
        }
    }
}

int CPU::step() {
    check_interrupts();

    if(halted) {
        update_timers(4);
        return 4;
    }

    uint8_t opcode = mmu.read8(reg.pc);
    reg.pc++;

    int cycles = BASE_CYCLES[opcode];

    if(opcode == 0xCB) {
        uint8_t cb_opcode = mmu.read8(reg.pc);
        cycles = ((cb_opcode & 0x07) == 0x06) ? 16 : 8;
    }

    execute_opcode(opcode);
    update_timers(cycles);

    return cycles;
}

void CPU::check_interrupts() {
    // 1. Lese IE und IF unabhängig vom IME-Status
    uint8_t ie = mmu.read8(0xFFFF);
    uint8_t if_reg = mmu.read8(0xFF0F);
    uint8_t masked = ie & if_reg;

    // 2. CPU aufwecken, sobald ein Interrupt ansteht (auch wenn IME false ist)
    if (masked > 0) {
        halted = false;
    }

    // 3. Interrupt-Vektor nur anspringen, wenn IME aktiviert ist
    if (ime && masked > 0) {
        // Find which interrupt was triggered (prioritized from bit 0 to 4)
        int interrupt_bit = 0;
        for (int i = 0; i < 5; i++) {
            if (masked & (1 << i)) {
                interrupt_bit = i;
                break;
            }
        }

        // Disable further interrupts
        ime = false;

        // Clear the specific bit in IF
        mmu.write8(0xFF0F, if_reg & ~(1 << interrupt_bit));

        // Push current PC to the stack
        mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF);
        mmu.write8(--reg.sp, reg.pc & 0xFF);

        // Jump to the corresponding interrupt vector address
        switch (interrupt_bit) {
            case 0: reg.pc = 0x0040; break; // VBlank
            case 1: reg.pc = 0x0048; break; // LCD STAT
            case 2: reg.pc = 0x0050; break; // Timer
            case 3: reg.pc = 0x0058; break; // Serial
            case 4: reg.pc = 0x0060; break; // Joypad
        }
    }
}

void CPU::execute_opcode(uint8_t opcode) {
    switch (opcode) {
        case 0x00: 
            // NOP
            break; 
            
        case 0x3E: {
            reg.a = fetch8();
            break;
        }    

        case 0xC3: {
            uint16_t address = fetch16();
            reg.pc = address;
            break;
        }

        case 0x21: {
            uint16_t value = fetch16();
            reg.set_hl(value);
            break;
        }

        case 0x47: {
            reg.b = reg.a;
            break;
        }

        case 0x11: {
            uint16_t value = fetch16();
            reg.set_de(value);
            break;
        }

        case 0x0E: {
            reg.c = fetch8();
            break;
        }

        // --- 16-bit Memory Loads & Increments/Decrements ---
        case 0x2A: { 
            reg.a = mmu.read8(reg.get_hl());
            reg.set_hl(reg.get_hl() + 1);
            break;
        }
        case 0x22: {
            mmu.write8(reg.get_hl(), reg.a);
            reg.set_hl(reg.get_hl() + 1);
            break;
        }
        case 0x3A: {
            reg.a = mmu.read8(reg.get_hl());
            reg.set_hl(reg.get_hl() - 1);
            break;
        }
        case 0x32: {
            mmu.write8(reg.get_hl(), reg.a);
            reg.set_hl(reg.get_hl() - 1);
            break;
        }

        // --- 8-Bit Immediate Loads (LD r, d8) ---
        case 0x06: reg.b = fetch8(); break; 
        case 0x16: reg.d = fetch8(); break; 
        case 0x26: reg.h = fetch8(); break; 
        case 0x1E: reg.e = fetch8(); break; 
        case 0x2E: reg.l = fetch8(); break; 
        case 0x36: mmu.write8(reg.get_hl(), fetch8()); break; 

        // --- 16-Bit Immediate Loads (LD rr, d16) ---
        case 0x01: reg.set_bc(fetch16()); break; 

        // --- Memory Loads to Register A ---
        case 0x0A: reg.a = mmu.read8(reg.get_bc()); break; 
        case 0x1A: reg.a = mmu.read8(reg.get_de()); break; 
        case 0x7E: reg.a = mmu.read8(reg.get_hl()); break; 
        case 0xFA: reg.a = mmu.read8(fetch16()); break;    
        
        // --- Register A to Memory Loads ---
        case 0x02: mmu.write8(reg.get_bc(), reg.a); break; 
        case 0x12: mmu.write8(reg.get_de(), reg.a); break; 
        case 0x77: mmu.write8(reg.get_hl(), reg.a); break; 
        case 0xEA: mmu.write8(fetch16(), reg.a); break;    

        // --- High RAM (Zero Page) Loads ---
        case 0xE0: {
            uint16_t addr = 0xFF00 + fetch8();
            mmu.write8(addr, reg.a);
            break;
        }
        case 0xF0: {
            uint16_t addr = 0xFF00 + fetch8();
            reg.a = mmu.read8(addr);
            break;
        }
        case 0xE2: {
            mmu.write8(0xFF00 + reg.c, reg.a);
            break;
        }
        case 0xF2: {
            reg.a = mmu.read8(0xFF00 + reg.c);
            break;
        }

        // --- Basic 8-bit Register-to-Register Loads ---
        case 0x7F: reg.a = reg.a; break; case 0x78: reg.a = reg.b; break;
        case 0x79: reg.a = reg.c; break; case 0x7A: reg.a = reg.d; break;
        case 0x7B: reg.a = reg.e; break; case 0x7C: reg.a = reg.h; break;
        case 0x7D: reg.a = reg.l; break; 

        case 0x40: reg.b = reg.b; break; case 0x41: reg.b = reg.c; break;
        case 0x42: reg.b = reg.d; break; case 0x43: reg.b = reg.e; break;
        case 0x44: reg.b = reg.h; break; case 0x45: reg.b = reg.l; break;
        case 0x46: reg.b = mmu.read8(reg.get_hl()); break;

        case 0x48: reg.c = reg.b; break; case 0x49: reg.c = reg.c; break;
        case 0x4A: reg.c = reg.d; break; case 0x4B: reg.c = reg.e; break;
        case 0x4C: reg.c = reg.h; break; case 0x4D: reg.c = reg.l; break;
        case 0x4E: reg.c = mmu.read8(reg.get_hl()); break;

        case 0x50: reg.d = reg.b; break; case 0x51: reg.d = reg.c; break;
        case 0x52: reg.d = reg.d; break; case 0x53: reg.d = reg.e; break;
        case 0x54: reg.d = reg.h; break; case 0x55: reg.d = reg.l; break;
        case 0x56: reg.d = mmu.read8(reg.get_hl()); break;

        case 0x58: reg.e = reg.b; break; case 0x59: reg.e = reg.c; break;
        case 0x5A: reg.e = reg.d; break; case 0x5B: reg.e = reg.e; break;
        case 0x5C: reg.e = reg.h; break; case 0x5D: reg.e = reg.l; break;
        case 0x5E: reg.e = mmu.read8(reg.get_hl()); break;

        case 0x60: reg.h = reg.b; break; case 0x61: reg.h = reg.c; break;
        case 0x62: reg.h = reg.d; break; case 0x63: reg.h = reg.e; break;
        case 0x64: reg.h = reg.h; break; case 0x65: reg.h = reg.l; break;
        case 0x66: reg.h = mmu.read8(reg.get_hl()); break;

        case 0x68: reg.l = reg.b; break; case 0x69: reg.l = reg.c; break;
        case 0x6A: reg.l = reg.d; break; case 0x6B: reg.l = reg.e; break;
        case 0x6C: reg.l = reg.h; break; case 0x6D: reg.l = reg.l; break;
        case 0x6E: reg.l = mmu.read8(reg.get_hl()); break;

        case 0x70: mmu.write8(reg.get_hl(), reg.b); break;
        case 0x71: mmu.write8(reg.get_hl(), reg.c); break;
        case 0x72: mmu.write8(reg.get_hl(), reg.d); break;
        case 0x73: mmu.write8(reg.get_hl(), reg.e); break;
        case 0x74: mmu.write8(reg.get_hl(), reg.h); break;
        case 0x75: mmu.write8(reg.get_hl(), reg.l); break;

        // --- Relative Jumps ---
        // --- Relative Jumps ---
        case 0x18: {
            // JR r8 (Relative Jump)
            int8_t offset = static_cast<int8_t>(fetch8());
            reg.pc += offset;
            break;
        }
        case 0x20: {
            // JR NZ, r8 (Jump Relative if Not Zero)
            int8_t offset = static_cast<int8_t>(fetch8());
            if (!reg.get_flag(Registers::FLAG_Z)) reg.pc += offset;
            break;
        }
        case 0x28: {
            // JR Z, r8 (Jump Relative if Zero)
            int8_t offset = static_cast<int8_t>(fetch8());
            if (reg.get_flag(Registers::FLAG_Z)) reg.pc += offset;
            break;
        }
        case 0x30: {
            // JR NC, r8 (Jump Relative if Not Carry)
            int8_t offset = static_cast<int8_t>(fetch8());
            if (!reg.get_flag(Registers::FLAG_C)) reg.pc += offset;
            break;
        }
        case 0x38: {
            // JR C, r8 (Jump Relative if Carry) - might as well add its pair too!
            int8_t offset = static_cast<int8_t>(fetch8());
            if (reg.get_flag(Registers::FLAG_C)) reg.pc += offset;
            break;
        }
        
        // --- NEW ALU BLOCK STARTS HERE ---

        // --- 8-Bit Increments ---
        case 0x04: reg.b = inc8(reg.b); break;
        case 0x0C: reg.c = inc8(reg.c); break;
        case 0x14: reg.d = inc8(reg.d); break;
        case 0x1C: reg.e = inc8(reg.e); break; // Replaced manual 0x1C
        case 0x24: reg.h = inc8(reg.h); break;
        case 0x2C: reg.l = inc8(reg.l); break;
        case 0x34: mmu.write8(reg.get_hl(), inc8(mmu.read8(reg.get_hl()))); break;
        case 0x3C: reg.a = inc8(reg.a); break; // Replaced manual 0x3C

        // --- 8-Bit Decrements ---
        case 0x05: reg.b = dec8(reg.b); break;
        case 0x0D: reg.c = dec8(reg.c); break;
        case 0x15: reg.d = dec8(reg.d); break;
        case 0x1D: reg.e = dec8(reg.e); break;
        case 0x25: reg.h = dec8(reg.h); break;
        case 0x2D: reg.l = dec8(reg.l); break;
        case 0x35: mmu.write8(reg.get_hl(), dec8(mmu.read8(reg.get_hl()))); break;
        case 0x3D: reg.a = dec8(reg.a); break;

        // --- ADD A, r ---
        case 0x80: add8(reg.b); break; case 0x81: add8(reg.c); break;
        case 0x82: add8(reg.d); break; case 0x83: add8(reg.e); break;
        case 0x84: add8(reg.h); break; case 0x85: add8(reg.l); break;
        case 0x86: add8(mmu.read8(reg.get_hl())); break; case 0x87: add8(reg.a); break;
        case 0xC6: add8(fetch8()); break; 

        // --- ADC A, r (Add with Carry) ---
        case 0x88: adc8(reg.b); break; case 0x89: adc8(reg.c); break;
        case 0x8A: adc8(reg.d); break; case 0x8B: adc8(reg.e); break;
        case 0x8C: adc8(reg.h); break; case 0x8D: adc8(reg.l); break;
        case 0x8E: adc8(mmu.read8(reg.get_hl())); break; case 0x8F: adc8(reg.a); break;
        case 0xCE: adc8(fetch8()); break; 

        // --- SUB A, r ---
        case 0x90: sub8(reg.b); break; case 0x91: sub8(reg.c); break;
        case 0x92: sub8(reg.d); break; case 0x93: sub8(reg.e); break;
        case 0x94: sub8(reg.h); break; case 0x95: sub8(reg.l); break;
        case 0x96: sub8(mmu.read8(reg.get_hl())); break; case 0x97: sub8(reg.a); break;
        case 0xD6: sub8(fetch8()); break; 

        // --- SBC A, r (Subtract with Carry) ---
        case 0x98: sbc8(reg.b); break; case 0x99: sbc8(reg.c); break;
        case 0x9A: sbc8(reg.d); break; case 0x9B: sbc8(reg.e); break;
        case 0x9C: sbc8(reg.h); break; case 0x9D: sbc8(reg.l); break;
        case 0x9E: sbc8(mmu.read8(reg.get_hl())); break; case 0x9F: sbc8(reg.a); break;
        case 0xDE: sbc8(fetch8()); break; 

        // --- AND A, r ---
        case 0xA0: and8(reg.b); break; case 0xA1: and8(reg.c); break;
        case 0xA2: and8(reg.d); break; case 0xA3: and8(reg.e); break;
        case 0xA4: and8(reg.h); break; case 0xA5: and8(reg.l); break;
        case 0xA6: and8(mmu.read8(reg.get_hl())); break; case 0xA7: and8(reg.a); break;
        case 0xE6: and8(fetch8()); break; 

        // --- XOR A, r ---
        case 0xA8: xor8(reg.b); break; case 0xA9: xor8(reg.c); break;
        case 0xAA: xor8(reg.d); break; case 0xAB: xor8(reg.e); break;
        case 0xAC: xor8(reg.h); break; case 0xAD: xor8(reg.l); break;
        case 0xAE: xor8(mmu.read8(reg.get_hl())); break; 
        case 0xAF: xor8(reg.a); break; // Replaced manual 0xAF
        case 0xEE: xor8(fetch8()); break; 

        // --- OR A, r ---
        case 0xB0: or8(reg.b); break; case 0xB1: or8(reg.c); break;
        case 0xB2: or8(reg.d); break; case 0xB3: or8(reg.e); break;
        case 0xB4: or8(reg.h); break; case 0xB5: or8(reg.l); break;
        case 0xB6: or8(mmu.read8(reg.get_hl())); break; case 0xB7: or8(reg.a); break;
        case 0xF6: or8(fetch8()); break; 

        // --- CP A, r (Compare) ---
        case 0xB8: cp8(reg.b); break; case 0xB9: cp8(reg.c); break;
        case 0xBA: cp8(reg.d); break; case 0xBB: cp8(reg.e); break;
        case 0xBC: cp8(reg.h); break; case 0xBD: cp8(reg.l); break;
        case 0xBE: cp8(mmu.read8(reg.get_hl())); break; case 0xBF: cp8(reg.a); break;
        case 0xFE: cp8(fetch8()); break;

        //--- Interrupt Controls ---
        case 0xF3: ime = false; break; //DI (Disable Interrupts)
        case 0xFB: ime = true; break; //EI (Enable Interrupts)

        // --- The Stack (PUSH / POP) ---
        case 0xC5: { mmu.write8(--reg.sp, reg.b); mmu.write8(--reg.sp, reg.c); break; } // PUSH BC
        case 0xD5: { mmu.write8(--reg.sp, reg.d); mmu.write8(--reg.sp, reg.e); break; } // PUSH DE
        case 0xE5: { mmu.write8(--reg.sp, reg.h); mmu.write8(--reg.sp, reg.l); break; } // PUSH HL
        case 0xF5: { mmu.write8(--reg.sp, reg.a); mmu.write8(--reg.sp, reg.f); break; } // PUSH AF
        
        case 0xC1: { reg.c = mmu.read8(reg.sp++); reg.b = mmu.read8(reg.sp++); break; } // POP BC
        case 0xD1: { reg.e = mmu.read8(reg.sp++); reg.d = mmu.read8(reg.sp++); break; } // POP DE
        case 0xE1: { reg.l = mmu.read8(reg.sp++); reg.h = mmu.read8(reg.sp++); break; } // POP HL
        case 0xF1: { 
            reg.f = mmu.read8(reg.sp++) & 0xF0; // Lower 4 bits of F are always 0
            reg.a = mmu.read8(reg.sp++); 
            break; 
        } // POP AF

        // --- Subroutines (CALL / RET) ---
        case 0xCD: { // CALL a16
            uint16_t target = fetch16();
            mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); // Push High PC
            mmu.write8(--reg.sp, reg.pc & 0xFF);        // Push Low PC
            reg.pc = target;
            break;
        }
        case 0xC4: { // CALL NZ, a16
            uint16_t target = fetch16();
            if (!reg.get_flag(Registers::FLAG_Z)) {
                mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF);
                mmu.write8(--reg.sp, reg.pc & 0xFF);
                reg.pc = target;
            }
            break;
        }
        case 0xCC: { // CALL Z, a16
            uint16_t target = fetch16();
            if (reg.get_flag(Registers::FLAG_Z)) {
                mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF);
                mmu.write8(--reg.sp, reg.pc & 0xFF);
                reg.pc = target;
            }
            break;
        }
        case 0xD4: { // CALL NC, a16
            uint16_t target = fetch16();
            if (!reg.get_flag(Registers::FLAG_C)) {
                mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF);
                mmu.write8(--reg.sp, reg.pc & 0xFF);
                reg.pc = target;
            }
            break;
        }
        case 0xDC: { // CALL C, a16
            uint16_t target = fetch16();
            if (reg.get_flag(Registers::FLAG_C)) {
                mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF);
                mmu.write8(--reg.sp, reg.pc & 0xFF);
                reg.pc = target;
            }
            break;
        }

        case 0xC9: { // RET (Return)
            uint8_t lo = mmu.read8(reg.sp++);
            uint8_t hi = mmu.read8(reg.sp++);
            reg.pc = (hi << 8) | lo;
            break;
        }
        case 0xC0: { // RET NZ
            if (!reg.get_flag(Registers::FLAG_Z)) {
                uint8_t lo = mmu.read8(reg.sp++);
                uint8_t hi = mmu.read8(reg.sp++);
                reg.pc = (hi << 8) | lo;
            }
            break;
        }
        case 0xC8: { // RET Z
            if (reg.get_flag(Registers::FLAG_Z)) {
                uint8_t lo = mmu.read8(reg.sp++);
                uint8_t hi = mmu.read8(reg.sp++);
                reg.pc = (hi << 8) | lo;
            }
            break;
        }
        case 0xD0: { // RET NC
            if (!reg.get_flag(Registers::FLAG_C)) {
                uint8_t lo = mmu.read8(reg.sp++);
                uint8_t hi = mmu.read8(reg.sp++);
                reg.pc = (hi << 8) | lo;
            }
            break;
        }
        case 0xD8: { // RET C
            if (reg.get_flag(Registers::FLAG_C)) {
                uint8_t lo = mmu.read8(reg.sp++);
                uint8_t hi = mmu.read8(reg.sp++);
                reg.pc = (hi << 8) | lo;
            }
            break;
        }
        case 0xD9: { // RETI (Return and Enable Interrupts)
            uint8_t lo = mmu.read8(reg.sp++);
            uint8_t hi = mmu.read8(reg.sp++);
            reg.pc = (hi << 8) | lo;
            ime = true;
            break;
        }

        // --- Restarts (RST) ---
        case 0xC7: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x00; break;
        case 0xCF: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x08; break;
        case 0xD7: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x10; break;
        case 0xDF: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x18; break;
        case 0xE7: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x20; break;
        case 0xEF: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x28; break;
        case 0xF7: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x30; break;
        case 0xFF: mmu.write8(--reg.sp, (reg.pc >> 8) & 0xFF); mmu.write8(--reg.sp, reg.pc & 0xFF); reg.pc = 0x38; break;

        case 0x31: {
            // LD SP, d16
            reg.sp = fetch16(); 
            break;
        }

        // --- 16-Bit ALU (ADD HL, rr) ---
        case 0x09: { // ADD HL, BC
            uint32_t result = reg.get_hl() + reg.get_bc();
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, (reg.get_hl() & 0x0FFF) + (reg.get_bc() & 0x0FFF) > 0x0FFF);
            reg.set_flag(Registers::FLAG_C, result > 0xFFFF);
            reg.set_hl(static_cast<uint16_t>(result));
            break;
        }
        case 0x19: { // ADD HL, DE
            uint32_t result = reg.get_hl() + reg.get_de();
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, (reg.get_hl() & 0x0FFF) + (reg.get_de() & 0x0FFF) > 0x0FFF);
            reg.set_flag(Registers::FLAG_C, result > 0xFFFF);
            reg.set_hl(static_cast<uint16_t>(result));
            break;
        }
        case 0x29: { // ADD HL, HL
            uint32_t result = reg.get_hl() + reg.get_hl();
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, (reg.get_hl() & 0x0FFF) + (reg.get_hl() & 0x0FFF) > 0x0FFF);
            reg.set_flag(Registers::FLAG_C, result > 0xFFFF);
            reg.set_hl(static_cast<uint16_t>(result));
            break;
        }
        case 0x39: { // ADD HL, SP
            uint32_t result = reg.get_hl() + reg.sp;
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, (reg.get_hl() & 0x0FFF) + (reg.sp & 0x0FFF) > 0x0FFF);
            reg.set_flag(Registers::FLAG_C, result > 0xFFFF);
            reg.set_hl(static_cast<uint16_t>(result));
            break;
        }

        // --- 16-Bit Increments / Decrements (No flags affected) ---
        case 0x03: reg.set_bc(reg.get_bc() + 1); break; // INC BC
        case 0x13: reg.set_de(reg.get_de() + 1); break; // INC DE
        case 0x23: reg.set_hl(reg.get_hl() + 1); break; // INC HL
        case 0x33: reg.sp++; break;                     // INC SP
        
        case 0x0B: reg.set_bc(reg.get_bc() - 1); break; // DEC BC
        case 0x1B: reg.set_de(reg.get_de() - 1); break; // DEC DE
        case 0x2B: reg.set_hl(reg.get_hl() - 1); break; // DEC HL
        case 0x3B: reg.sp--; break;                     // DEC SP

        // --- SP Offsets ---
        case 0xE8: { // ADD SP, r8
            int8_t offset = static_cast<int8_t>(fetch8());
            reg.set_flag(Registers::FLAG_Z, false);
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, ((reg.sp & 0x0F) + (offset & 0x0F)) > 0x0F);
            reg.set_flag(Registers::FLAG_C, ((reg.sp & 0xFF) + (uint8_t)offset) > 0xFF);
            reg.sp += offset;
            break;
        }
        case 0xF8: { // LD HL, SP+r8
            int8_t offset = static_cast<int8_t>(fetch8());
            reg.set_flag(Registers::FLAG_Z, false);
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, ((reg.sp & 0x0F) + (offset & 0x0F)) > 0x0F);
            reg.set_flag(Registers::FLAG_C, ((reg.sp & 0xFF) + (uint8_t)offset) > 0xFF);
            reg.set_hl(reg.sp + offset);
            break;
        }
        case 0x08: { // LD (a16), SP
            uint16_t addr = fetch16();
            mmu.write16(addr, reg.sp);
            break;
        }

        // --- Absolute Conditional Jumps ---
        case 0xC2: { uint16_t a = fetch16(); if (!reg.get_flag(Registers::FLAG_Z)) reg.pc = a; break; } // JP NZ, a16
        case 0xCA: { uint16_t a = fetch16(); if (reg.get_flag(Registers::FLAG_Z)) reg.pc = a; break; }  // JP Z, a16
        case 0xD2: { uint16_t a = fetch16(); if (!reg.get_flag(Registers::FLAG_C)) reg.pc = a; break; } // JP NC, a16
        case 0xDA: { uint16_t a = fetch16(); if (reg.get_flag(Registers::FLAG_C)) reg.pc = a; break; }  // JP C, a16
        case 0xE9: { reg.pc = reg.get_hl(); break; } // JP (HL)

        // --- Accumulator & Misc Controls ---
        case 0x27: { // DAA (Decimal Adjust Accumulator)
            uint16_t a = reg.a;
            if (!reg.get_flag(Registers::FLAG_N)) {
                if (reg.get_flag(Registers::FLAG_H) || (a & 0x0F) > 9) a += 0x06;
                if (reg.get_flag(Registers::FLAG_C) || a > 0x9F) a += 0x60;
            } else {
                if (reg.get_flag(Registers::FLAG_H)) a = (a - 6) & 0xFF;
                if (reg.get_flag(Registers::FLAG_C)) a -= 0x60;
            }
            reg.set_flag(Registers::FLAG_H, false);
            if ((a & 0x100) == 0x100) reg.set_flag(Registers::FLAG_C, true);
            reg.a = static_cast<uint8_t>(a);
            reg.set_flag(Registers::FLAG_Z, reg.a == 0);
            break;
        }
        case 0x2F: { // CPL (Complement A)
            reg.a = ~reg.a;
            reg.set_flag(Registers::FLAG_N, true);
            reg.set_flag(Registers::FLAG_H, true);
            break;
        }
        case 0x37: { // SCF (Set Carry Flag)
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, false);
            reg.set_flag(Registers::FLAG_C, true);
            break;
        }
        case 0x3F: { // CCF (Complement Carry Flag)
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, false);
            reg.set_flag(Registers::FLAG_C, !reg.get_flag(Registers::FLAG_C));
            break;
        }
        
        // --- Accumulator Rotates (Base Table) ---
        case 0x07: { // RLCA
            uint8_t carry = reg.a >> 7;
            reg.a = (reg.a << 1) | carry;
            reg.set_flag(Registers::FLAG_Z, false); // Base rotates always clear Z
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, false);
            reg.set_flag(Registers::FLAG_C, carry != 0);
            break;
        }
        case 0x0F: { // RRCA
            uint8_t carry = reg.a & 0x01;
            reg.a = (reg.a >> 1) | (carry << 7);
            reg.set_flag(Registers::FLAG_Z, false);
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, false);
            reg.set_flag(Registers::FLAG_C, carry != 0);
            break;
        }
        case 0x17: { // RLA
            uint8_t carry = reg.get_flag(Registers::FLAG_C) ? 1 : 0;
            uint8_t new_carry = reg.a >> 7;
            reg.a = (reg.a << 1) | carry;
            reg.set_flag(Registers::FLAG_Z, false);
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, false);
            reg.set_flag(Registers::FLAG_C, new_carry != 0);
            break;
        }
        case 0x1F: { // RRA
            uint8_t carry = reg.get_flag(Registers::FLAG_C) ? 1 : 0;
            uint8_t new_carry = reg.a & 0x01;
            reg.a = (reg.a >> 1) | (carry << 7);
            reg.set_flag(Registers::FLAG_Z, false);
            reg.set_flag(Registers::FLAG_N, false);
            reg.set_flag(Registers::FLAG_H, false);
            reg.set_flag(Registers::FLAG_C, new_carry != 0);
            break;
        }

        case 0x76: // HALT 
            halted = true;
            break; 
        case 0x10: // STOP 
            fetch8(); // STOP is actually a 2-byte opcode (0x10 0x00)
            break;

        case 0xCB: {
            // Fetch the second byte
            uint8_t cb_op = fetch8();
            
            // Decode the instruction parts
            uint8_t reg_idx = cb_op & 0x07;         // Target register (0 to 7)
            uint8_t bit_idx = (cb_op >> 3) & 0x07;  // Target bit (0 to 7) or shift type
            uint8_t category = cb_op >> 6;          // Instruction family (0 to 3)
            
            // 1. Read the target value
            uint8_t val = 0;
            switch(reg_idx) {
                case 0: val = reg.b; break;
                case 1: val = reg.c; break;
                case 2: val = reg.d; break;
                case 3: val = reg.e; break;
                case 4: val = reg.h; break;
                case 5: val = reg.l; break;
                case 6: val = mmu.read8(reg.get_hl()); break;
                case 7: val = reg.a; break;
            }

            // 2. Perform the Operation
            if (category == 1) { 
                // BIT (Test Bit)
                reg.set_flag(Registers::FLAG_Z, (val & (1 << bit_idx)) == 0);
                reg.set_flag(Registers::FLAG_N, false);
                reg.set_flag(Registers::FLAG_H, true);
                break; // BIT is read-only, so break early to skip write-back
            } 
            else if (category == 2) { 
                // RES (Reset Bit)
                val &= ~(1 << bit_idx);
            } 
            else if (category == 3) { 
                // SET (Set Bit)
                val |= (1 << bit_idx);
            } 
            else { 
                // Category 0: Rotations and Shifts
                switch (bit_idx) {
                    case 0: { // RLC (Rotate Left Circular)
                        uint8_t carry = val >> 7;
                        val = (val << 1) | carry;
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, carry != 0);
                        break;
                    }
                    case 1: { // RRC (Rotate Right Circular)
                        uint8_t carry = val & 0x01;
                        val = (val >> 1) | (carry << 7);
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, carry != 0);
                        break;
                    }
                    case 2: { // RL (Rotate Left through Carry)
                        uint8_t carry = reg.get_flag(Registers::FLAG_C) ? 1 : 0;
                        uint8_t new_carry = val >> 7;
                        val = (val << 1) | carry;
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, new_carry != 0);
                        break;
                    }
                    case 3: { // RR (Rotate Right through Carry)
                        uint8_t carry = reg.get_flag(Registers::FLAG_C) ? 1 : 0;
                        uint8_t new_carry = val & 0x01;
                        val = (val >> 1) | (carry << 7);
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, new_carry != 0);
                        break;
                    }
                    case 4: { // SLA (Shift Left Arithmetic)
                        uint8_t carry = val >> 7;
                        val <<= 1;
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, carry != 0);
                        break;
                    }
                    case 5: { // SRA (Shift Right Arithmetic - MSB remains unchanged)
                        uint8_t carry = val & 0x01;
                        val = (val >> 1) | (val & 0x80); 
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, carry != 0);
                        break;
                    }
                    case 6: { // SWAP (Swap upper and lower nibbles)
                        val = ((val & 0x0F) << 4) | ((val & 0xF0) >> 4);
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, false);
                        break;
                    }
                    case 7: { // SRL (Shift Right Logical - MSB becomes 0)
                        uint8_t carry = val & 0x01;
                        val >>= 1;
                        reg.set_flag(Registers::FLAG_Z, val == 0);
                        reg.set_flag(Registers::FLAG_N, false);
                        reg.set_flag(Registers::FLAG_H, false);
                        reg.set_flag(Registers::FLAG_C, carry != 0);
                        break;
                    }
                }
            }

            // 3. Write the modified value back to the target register/memory
            switch(reg_idx) {
                case 0: reg.b = val; break;
                case 1: reg.c = val; break;
                case 2: reg.d = val; break;
                case 3: reg.e = val; break;
                case 4: reg.h = val; break;
                case 5: reg.l = val; break;
                case 6: mmu.write8(reg.get_hl(), val); break;
                case 7: reg.a = val; break;
            }
            break; // End of 0xCB instruction
        }

        // --- The Missing "Load A into Register" Column ---
        case 0x4F: reg.c = reg.a; break; // LD C, A
        case 0x57: reg.d = reg.a; break; // LD D, A
        case 0x5F: reg.e = reg.a; break; // LD E, A
        case 0x67: reg.h = reg.a; break; // LD H, A
        case 0x6F: reg.l = reg.a; break; // LD L, A

        case 0xF9: { // LD SP, HL
            reg.sp = reg.get_hl();
            // Dieser Befehl benötigt 8 Zyklen. Da BASE_CYCLES[0xF9] bei dir
            // momentan als 8 definiert ist, passt das automatisch.
            break;
        }

        default: {
            // Öffne oder erstelle eine Logdatei im selben Ordner
            std::ofstream log_file("crash_log.txt", std::ios::app);
            if (log_file.is_open()) {
                log_file << "[CPU] Warning: Unimplemented Opcode 0x" 
                         << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode) 
                         << " at PC 0x" << (reg.pc - 1) << std::endl;
                log_file.close();
            }
            // Beende das Programm wie bisher
            std::exit(1);
        }
    }
}

} // namespace AegisS