#include <iostream>
#include <string>
#include "mmu/mmu.h"
#include "cpu/registers.h"
#include "cpu/cpu.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: AegisCore.exe <path_to_rom.gb>" << std::endl;
        return 1;
    }

    std::string rom_path = argv[1];
    
    Aegis::MMU mmu;
    Aegis::Registers registers; 
    
    if (!mmu.load_rom(rom_path)) {
        return 1;
    }
    
    Aegis::CPU cpu(registers, mmu);

    std::cout << "[AegisCore] Entering main emulation loop..." << std::endl;
    
    // Run infinitely (it will stop when std::exit(1) is called for an unknown opcode)
    while (true) {
        cpu.step();
    }
    return 0;
}