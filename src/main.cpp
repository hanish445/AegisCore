#include <iostream>
#include <string>
#include "mmu/mmu.h"
#include "cpu/registers.h"

int main(int argc, char* argv[]) {
    std::cout << "[AegisCore] Emulation Engine Initialized." << std::endl;
    
    if(argc < 2){
        std::cerr << "Usage: AegisCore.exe <path_to_rom.gb>" << std::endl;
        return 1;
    }

    std::string rom_path = argv[1];

    Aegis::MMU mmu;
    Aegis::Registers registers;

    if(!mmu.load_rom(rom_path)){
        std::cerr << "[AegisCore] Critical Error: Failed to load ROM. Exiting..." << std::endl;
        return 1;
    }

    std::cout << "[AegisCore] Entering main emulation loop..." << std::endl;

    bool is_running = true;
    while(is_running){
        //Step 4 will come here:
        //1. Fetch opcode using mmu.read8(register.pc)
        //2. Decode opcode
        //3. Execute instruction
        //4. Update clock cycles / timers

        //for now, break immediately to prevent an infinite loop
        is_running = false;
    }

    std::cout << "[AegisCore] Emulation halted gracefully" << std::endl;
    return 0;
}