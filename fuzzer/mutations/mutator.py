import os
import random

class ROMMutator:
    def __init__(self, seed_rom_path):
        """Loads a valid Game Boy ROM into memory for mutation."""
        if not os.path.exists(seed_rom_path):
            raise FileNotFoundError(f"Seed ROM not found: {seed_rom_path}")
            
        with open(seed_rom_path, 'rb') as f:
            self.original_data = bytearray(f.read())
            
        self.mutated_data = bytearray(self.original_data)

    def reset(self):
        """Resets the mutated data back to the clean, original ROM state."""
        self.mutated_data = bytearray(self.original_data)

    def bit_flip(self):
        """Flips a single random bit in a random byte of the ROM."""
        if not self.mutated_data: return
        idx = random.randint(0, len(self.mutated_data) - 1)
        bit_idx = random.randint(0, 7)
        self.mutated_data[idx] ^= (1 << bit_idx)

    def byte_swap(self):
        """Swaps the positions of two random bytes."""
        if len(self.mutated_data) < 2: return
        idx1 = random.randint(0, len(self.mutated_data) - 1)
        idx2 = random.randint(0, len(self.mutated_data) - 1)
        self.mutated_data[idx1], self.mutated_data[idx2] = self.mutated_data[idx2], self.mutated_data[idx1]

    def magic_number_injection(self):
        """Injects known boundary or crash-inducing integers into random offsets."""
        if not self.mutated_data: return
        # Common edge-case values in 8-bit systems
        magic_numbers = [0x00, 0xFF, 0x7F, 0x80, 0x01, 0xFE]
        idx = random.randint(0, len(self.mutated_data) - 1)
        self.mutated_data[idx] = random.choice(magic_numbers)

    def mutate(self, intensity=5):
        """
        Applies a series of random mutations to the ROM.
        :param intensity: The number of mutations to apply in one pass.
        """
        self.reset()
        mutation_strategies = [self.bit_flip, self.byte_swap, self.magic_number_injection]
        
        for _ in range(intensity):
            strategy = random.choice(mutation_strategies)
            strategy()
            
        return self.mutated_data

    def save_mutated_rom(self, output_path):
        """Writes the mutated bytearray to a new .gb file."""
        with open(output_path, 'wb') as f:
            f.write(self.mutated_data)

# Quick test block (runs only if you execute this file directly)
if __name__ == "__main__":
    # Adjust path if running directly from the mutations folder
    seed_path = "../corpus/tetris.gb" 
    output_path = "../mutated_test.gb"
    
    if os.path.exists(seed_path):
        mutator = ROMMutator(seed_path)
        mutator.mutate(intensity=10) # Apply 10 random corruptions
        mutator.save_mutated_rom(output_path)
        print(f"Successfully generated {output_path} with mutations.")
    else:
        print(f"Please place a valid ROM at {seed_path} to test the mutator.")