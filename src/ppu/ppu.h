#pragma once
#include <cstdint>
#include "../mmu/mmu.h"

namespace Aegis{
    class PPU {
        private:
            MMU& mmu;
            int scanline_counter = 456; //A scanline takes exactly 456 CPU clock cycles

            //Internal rendering functions
            void render_scanline();
            void render_background();
            void render_sprites();

            //Palette color mapping (lightest to darkest)
            const uint32_t colors[4] = { 0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000 };

        public:
            //160x144 Framebuffer (Format: 0x00RRGGBB)
            uint32_t framebuffer[160 * 144];
            bool frame_complete = false;

            PPU(MMU& memory_unit);
            void step(int cpu_cyles);

    };
}