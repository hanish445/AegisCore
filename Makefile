CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Isrc
SRC = src/main.cpp src/mmu/mmu.cpp src/cpu/cpu.cpp
OUT = build/AegisCore.exe

all:
	g++ -std=c++20 -Wall -Wextra -Isrc src/main.cpp src/mmu/mmu.cpp src/cpu/cpu.cpp src/ppu/ppu.cpp -o build/AegisCore.exe -lgdi32 -mwindows

clean:
	del /q build\*.exe