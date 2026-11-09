#include <windows.h>
#include <iostream>
#include "mmu/mmu.h"
#include "cpu/cpu.h"
#include "ppu/ppu.h"
#include <chrono>
#include <thread>

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
    std::string cmd = lpCmdLine;
    bool headless = false;

    // Prüfen, ob "--headless " übergeben wurde
    size_t headless_pos = cmd.find("--headless ");
    if (headless_pos != std::string::npos) {
        headless = true;
        cmd.erase(headless_pos, 11); // "--headless " aus dem Pfad löschen
    }
    
    // Anführungszeichen trimmen (falls Windows sie bei Leerzeichen im Pfad setzt)
    if (!cmd.empty() && cmd.front() == '"' && cmd.back() == '"') {
        cmd = cmd.substr(1, cmd.length() - 2);
    }
    std::string rom_path = cmd;

    if(!mmu.load_rom(rom_path)) return 1;

    //Headless mode: infinite loop without windows-events, window or frame-pacing
    if(headless) {
        while(true){
            int cycles_taken = cpu.step();
            ppu.step(cycles_taken);
        }
    }

    // Suppress unused parameter warnings
    (void)hPrevInstance;

    // 1. Load the ROM (Hardcoded for now, or use lpCmdLine)
    if (!mmu.load_rom("fuzzer/corpus/tetris.gb")) {
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
    // 4. Main Emulation Loop
    MSG msg = {}; // {} avoids missing-initializer warnings
    const int CYCLES_PER_FRAME = 70224; // 456 cycles * 154 scanlines
    const double TARGET_FRAME_TIME_MS = 1000.0 / 59.7275; // Exact hardware framerate

    while (true) {
        auto frame_start = std::chrono::high_resolution_clock::now();

        // 1. Handle Windows events without blocking the emulator
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 2. Joypad Input (Einmal pro Frame abfragen reicht völlig)
        uint8_t action = 0x0F;
        uint8_t dir = 0x0F;

        if (GetAsyncKeyState(VK_RETURN) & 0x8000) action &= ~0x08; // Start
        if (GetAsyncKeyState(VK_SHIFT)  & 0x8000) action &= ~0x04; // Select
        if (GetAsyncKeyState('Z')       & 0x8000) action &= ~0x02; // B
        if (GetAsyncKeyState('X')       & 0x8000) action &= ~0x01; // A
        if (GetAsyncKeyState(VK_DOWN)   & 0x8000) dir &= ~0x08; // Down
        if (GetAsyncKeyState(VK_UP)     & 0x8000) dir &= ~0x04; // Up
        if (GetAsyncKeyState(VK_LEFT)   & 0x8000) dir &= ~0x02; // Left
        if (GetAsyncKeyState(VK_RIGHT)  & 0x8000) dir &= ~0x01; // Right

        mmu.update_joypad(action, dir);

        // 3. HARDWARE BATCHING: Führe einen kompletten Bildaufbau am Stück aus
        int cycles_this_frame = 0;
        while (cycles_this_frame < CYCLES_PER_FRAME) {
            int cycles_taken = cpu.step();
            ppu.step(cycles_taken);
            cycles_this_frame += cycles_taken;
        }

        // 4. Wenn die PPU fertig ist, das Bild auf das Windows-Fenster zeichnen
        if (ppu.frame_complete) {
            ppu.frame_complete = false;
            InvalidateRect(hwnd, NULL, FALSE); // Triggers WM_PAINT
            UpdateWindow(hwnd);
        }

        // 5. PRÄZISES TIMING: Warten, bis die ~16.74ms für diesen Frame um sind
        auto frame_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> time_taken = frame_end - frame_start;

        if (time_taken.count() < TARGET_FRAME_TIME_MS) {
            std::chrono::duration<double, std::milli> sleep_time(TARGET_FRAME_TIME_MS - time_taken.count());
            std::this_thread::sleep_for(sleep_time);
        }
    }

    return 0;
}