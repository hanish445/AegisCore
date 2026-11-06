CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Isrc/include
SRC = src/main.cpp
OUT = build/AegisCore.exe

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	del /q build\*.exe