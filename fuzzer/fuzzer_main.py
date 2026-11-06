import subprocess
import sys
import os

def run_emulator(binary_path, target_rom):
    print(f"[*] Starting fuzzing session on {target_rom}")
    try:
        result = subprocess.run(
            [binary_path, target_rom],
            capture_output = True,
            text=True
        )

        if result.returncode != 0:
            print(f"[!] Crash detected! Exit code: {result.returncode}")
        else:
            print("[*] Execution finished cleanly")

    except FileNotFoundError:
        print("[Error] AegisCore.exe not found. Did you build it?")

if __name__ == "__main__":
    exe_path = os.path.join("..", "build", "AegisCore.exe")
    run_emulator(exe_path, "corpus/dummy.gb")