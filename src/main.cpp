#include <windows.h>
#include <iostream>
#include "mmu/mmu.h"
#include "cpu/cpu.h"
#include "ppu/ppu.h"

// Global instances for the emulator components
Aegis::MMU mmu;
Aegis::Registers reg;
Aegis::CPU cpu(reg, mmu);
Aegis::PPU ppu(mmu);

// Win32 Window Procedure to handle OS events (like closing the window)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Bitmap header for a 32-bit 160x144 image (using {} avoids missing-initializer warnings)
            BITMAPINFO bmi = {};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = 160;
            bmi.bmiHeader.biHeight = -144; // Negative indicates top-down rendering
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            // Draw the PPU's framebuffer to the screen, scaling it up by 3x (480x432)
            StretchDIBits(hdc, 0, 0, 160 * 3, 144 * 3, 
                          0, 0, 160, 144, 
                          ppu.framebuffer, &bmi, 
                          DIB_RGB_COLORS, SRCCOPY);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Windows GUI Entry Point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Suppress unused parameter warnings
    (void)hPrevInstance;
    (void)lpCmdLine;

    // 1. Load the ROM (Hardcoded for now, or use lpCmdLine)
    if (!mmu.load_rom("fuzzer/corpus/01-special.gb")) {
        MessageBox(NULL, "Failed to load ROM!", "AegisCore Error", MB_ICONERROR);
        return 1;
    }

    // 2. Register the Window Class
    const char CLASS_NAME[] = "AegisCore Window Class";
    WNDCLASS wc = {}; // {} avoids missing-initializer warnings
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // 3. Create the Window (480x432 client area + window borders)
    RECT rect = {0, 0, 160 * 3, 144 * 3};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "AegisCore Game Boy Emulator", 
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // Disable resizing
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    // 4. Main Emulation Loop
    MSG msg = {}; // {} avoids missing-initializer warnings
    while (true) {
        // Handle Windows events without blocking the emulator
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Execute 1 CPU instruction
        cpu.step();

        // TODO: Replace '4' with the actual cycle count of the executed instruction
        int cycles_taken = 4; 
        
        // Pass those cycles to the PPU
        ppu.step(cycles_taken);

        // If the PPU finished a full 144-line frame, draw it to the window
        if (ppu.frame_complete) {
            ppu.frame_complete = false;
            InvalidateRect(hwnd, NULL, FALSE); // Triggers WM_PAINT
            UpdateWindow(hwnd);
            
            // Sleep for ~16ms to target 60FPS (Very basic timing)
            Sleep(16); 
        }
    }

    return 0;
}