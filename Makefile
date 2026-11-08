CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Isrc
SRC = src/main.cpp src/mmu/mmu.cpp src/cpu/cpu.cpp
OUT = build/AegisCore.exe

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	del /q build\*.exe