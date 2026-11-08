#include <iostream>
#include <string>
#include "mmu/mmu.h"
#include "cpu/registers.h"
#include "cpu/cpu.h" // New inclusion

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
    
    // Instantiate the CPU, passing references to the registers and MMU
    Aegis::CPU cpu(registers, mmu);

    std::cout << "[AegisCore] Entering main emulation loop..." << std::endl;
    
    // Run the CPU for a maximum of 5 cycles just to test the loop
    int cycle_limit = 5; 
    
    while (cycle_limit > 0) {
        cpu.step();
        cycle_limit--;
    }
    
    std::cout << "[AegisCore] Emulation halted cleanly." << std::endl;
    return 0;
}