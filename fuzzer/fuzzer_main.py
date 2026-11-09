import subprocess
import sys
import os
import shutil
from datetime import datetime

#Mutator from mutation file
from mutations.mutator import ROMMutator

def run_fuzzer(binary_path, seed_rom):
    #setup fuzzer/ folder
    base_dir = os.path.dirname(os.path.abspath(__file__))
    logs_dir = os.path.join(base_dir, "..", "build", "logs")
    test_rom = os.path.join(base_dir, "mutated_test.gb")

    os.makedirs(logs_dir, exist_ok=True)

    if not os.path.exists(binary_path):
        print(f"[Error] AegisCore.exe not found at {binary_path}. Did you build it?")
        return

    print(f"[*] Starting fuzzing session with seed: {seed_rom}")
    mutator = ROMMutator(seed_rom)
    iteration = 0

    while True:
        iteration += 1

        #mutate ROM and save
        mutator.mutate(intensity=5)
        mutator.save_mutated_rom(test_rom)

        try:
            #run Emulator in headless mode
            result = subprocess.run(
                [binary_path, "--headless", test_rom],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=1.5
            )

            #validate Crashes
            if result.returncode != 0:
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                print(f"[!] Crash detected at iteration {iteration}! Exit code: {result.returncode}")

                # Defekte ROM in den logs/ Ordner sichern
                crash_rom_dest = os.path.join(logs_dir, f"crash_{timestamp}.gb")
                shutil.copy(test_rom, crash_rom_dest)
                
                # Das C++ Crash-Log umbenennen und verschieben
                cxx_log = os.path.join(base_dir, "..", "crash_log.txt")
                if os.path.exists(cxx_log):
                    json_dest = os.path.join(logs_dir, f"trace_{timestamp}.json")
                    shutil.move(cxx_log, json_dest)
                    print(f"[*] Saved crash trace to: {json_dest}")
        except subprocess.TimeoutExpired:
            # CPU hängt in einer Endlosschleife (typisch für kaputte ROMs) – einfach weitermachen
            pass
        except KeyboardInterrupt:
            print("\n[*] Fuzzing stopped by user.")
            break

        if iteration % 100 == 0:
            print(f"[*] {iteration} iterations completed...")

if __name__ == "__main__":
    # Pfade relativ zur fuzzer_main.py
    base_dir = os.path.dirname(os.path.abspath(__file__))
    exe_path = os.path.abspath(os.path.join(base_dir, "..", "build", "AegisCore.exe"))
    
    # Nimm tetris.gb als Seed (da es erwiesenermaßen stabil im Emulator läuft)
    seed_path = os.path.abspath(os.path.join(base_dir, "corpus", "tetris.gb")) 
    
    run_fuzzer(exe_path, seed_path)
            