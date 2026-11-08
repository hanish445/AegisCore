#include "ppu.h"

namespace Aegis {

PPU::PPU(MMU& memory_unit) : mmu(memory_unit) {
    // Clear framebuffer to white
    for (int i = 0; i < 160 * 144; i++) {
        framebuffer[i] = 0xFFFFFFFF;
    }
}

void PPU::step(int cpu_cycles) {
    uint8_t lcd_control = mmu.read8(0xFF40);
    
    // If LCD is disabled, reset states
    if (!(lcd_control & 0x80)) {
        scanline_counter = 456;
        mmu.write8(0xFF44, 0); // LY = 0
        set_lcd_mode(1);       // Default to VBlank mode when off
        return;
    }

    scanline_counter -= cpu_cycles;

    uint8_t current_line = mmu.read8(0xFF44);

    // Mode transitions during visible scanlines (0-143)
    if (current_line < 144) {
        if (scanline_counter <= 376 && scanline_counter > 204) {
            set_lcd_mode(2); // OAM Scan
        } else if (scanline_counter <= 204 && scanline_counter > 84) {
            set_lcd_mode(3); // Pixel Transfer
        } else if (scanline_counter <= 84 && scanline_counter > 0) {
            set_lcd_mode(0); // HBlank
        }
    }

    if (scanline_counter <= 0) {
        scanline_counter += 456;
        current_line++;
        mmu.write8(0xFF44, current_line);

        // Check LYC == LY coincidence interrupt
        uint8_t lyc = mmu.read8(0xFF45);
        uint8_t stat = mmu.read8(0xFF41);
        if (current_line == lyc) {
            stat |= 0x04; // Set coincidence flag
            if (stat & 0x40) { // Coincidence interrupt enabled?
                mmu.write8(0xFF0F, mmu.read8(0xFF0F) | 0x02); // LCD STAT Interrupt
            }
        } else {
            stat &= ~0x04;
        }
        mmu.write8(0xFF41, stat);

        if (current_line < 144) {
            render_scanline();
        }
        else if (current_line == 144) {
            frame_complete = true;
            set_lcd_mode(1); // VBlank Mode
            mmu.write8(0xFF0F, mmu.read8(0xFF0F) | 0x01); // VBlank Interrupt
            if (stat & 0x10) {
                mmu.write8(0xFF0F, mmu.read8(0xFF0F) | 0x02); // STAT VBlank Interrupt
            }
        }
        else if (current_line > 153) {
            mmu.write8(0xFF44, 0);
            set_lcd_mode(2); // Restart cycle with OAM scan
        }
    }
}

void PPU::set_lcd_mode(uint8_t mode) {
    uint8_t stat = mmu.read8(0xFF41);
    stat = (stat & ~0x03) | mode;
    mmu.write8(0xFF41, stat);

    // Trigger Mode-specific STAT interrupts
    uint8_t if_reg = mmu.read8(0xFF0F);
    if ((mode == 0 && (stat & 0x08)) ||  // HBlank interrupt
        (mode == 1 && (stat & 0x10)) ||  // VBlank interrupt
        (mode == 2 && (stat & 0x20))) {  // OAM interrupt
        mmu.write8(0xFF0F, if_reg | 0x02);
    }
}

void PPU::render_scanline() {
    uint8_t lcd_control = mmu.read8(0xFF40);
    if (lcd_control & 0x01) render_background();
    if (lcd_control & 0x02) render_sprites();
}

void PPU::render_background() {
    uint8_t lcdc = mmu.read8(0xFF40);
    uint8_t scroll_y = mmu.read8(0xFF42);
    uint8_t scroll_x = mmu.read8(0xFF43);
    uint8_t window_y = mmu.read8(0xFF4A);
    uint8_t window_x = mmu.read8(0xFF4B) - 7;
    uint8_t current_line = mmu.read8(0xFF44);
    uint8_t palette = mmu.read8(0xFF47);

    // Determine which tile map (0x9800 or 0x9C00) and tile data (0x8000 or 0x8800) to use
    bool using_window = (lcdc & 0x20) && window_y <= current_line;
    uint16_t tile_data = (lcdc & 0x10) ? 0x8000 : 0x8800;
    uint16_t tile_map = using_window ? ((lcdc & 0x40) ? 0x9C00 : 0x9800) 
                                     : ((lcdc & 0x08) ? 0x9C00 : 0x9800);

    uint8_t y_pos = using_window ? current_line - window_y : current_line + scroll_y;
    uint16_t tile_row = (((uint16_t)(y_pos / 8)) * 32);

    for (int pixel = 0; pixel < 160; pixel++) {
        uint8_t x_pos = pixel + scroll_x;
        if (using_window && pixel >= window_x) {
            x_pos = pixel - window_x;
        }

        uint16_t tile_col = x_pos / 8;
        int16_t tile_num;
        
        // Fetch tile index
        if (tile_data == 0x8000) {
            tile_num = mmu.read8(tile_map + tile_row + tile_col);
        } else {
            tile_num = (int8_t)mmu.read8(tile_map + tile_row + tile_col);
        }

        // Fetch pixel data for this specific row of the tile
        uint16_t tile_location = tile_data + (tile_data == 0x8000 ? tile_num * 16 : (tile_num + 128) * 16);
        uint8_t line = (y_pos % 8) * 2;
        uint8_t data1 = mmu.read8(tile_location + line);
        uint8_t data2 = mmu.read8(tile_location + line + 1);

        // Decode color (2 bits per pixel)
        int color_bit = 7 - (x_pos % 8);
        int color_num = ((data2 >> color_bit) & 1) << 1;
        color_num |= ((data1 >> color_bit) & 1);

        // Map to BGP palette and draw
        int mapped_color = (palette >> (color_num * 2)) & 3;
        framebuffer[current_line * 160 + pixel] = colors[mapped_color];
    }
}

void PPU::render_sprites() {
    uint8_t lcdc = mmu.read8(0xFF40);
    uint8_t current_line = mmu.read8(0xFF44);
    int sprite_height = (lcdc & 0x04) ? 16 : 8;

    // OAM can hold 40 sprites (4 bytes each)
    for (int sprite = 39; sprite >= 0; sprite--) {
        int index = sprite * 4;
        uint8_t y_pos = mmu.read8(0xFE00 + index) - 16;
        uint8_t x_pos = mmu.read8(0xFE00 + index + 1) - 8;
        uint8_t tile_loc = mmu.read8(0xFE00 + index + 2);
        uint8_t attributes = mmu.read8(0xFE00 + index + 3);

        bool y_flip = (attributes & 0x40);
        bool x_flip = (attributes & 0x20);

        // Does this sprite intersect the current scanline?
        if (current_line >= y_pos && current_line < (y_pos + sprite_height)) {
            int line = current_line - y_pos;
            if (y_flip) line = (sprite_height - 1) - line; // Read backwards

            uint16_t data_address = 0x8000 + (tile_loc * 16) + (line * 2);
            uint8_t data1 = mmu.read8(data_address);
            uint8_t data2 = mmu.read8(data_address + 1);

            for (int tile_pixel = 7; tile_pixel >= 0; tile_pixel--) {
                int color_bit = tile_pixel;
                if (x_flip) color_bit = 7 - color_bit; // Read backwards

                int color_num = ((data2 >> color_bit) & 1) << 1;
                color_num |= ((data1 >> color_bit) & 1);

                // Color 0 is transparent for sprites
                if (color_num == 0) continue; 

                uint8_t palette = (attributes & 0x10) ? mmu.read8(0xFF49) : mmu.read8(0xFF48);
                int mapped_color = (palette >> (color_num * 2)) & 3;

                int x_pix = 0 - tile_pixel;
                x_pix += 7;
                int pixel = x_pos + x_pix;

                // Priority Check: If sprite is under background, only draw over background color 0
                bool bg_priority = (attributes & 0x80);
                if (pixel >= 0 && pixel < 160) {
                    if (!bg_priority || framebuffer[current_line * 160 + pixel] == colors[(mmu.read8(0xFF47) & 3)]) {
                        framebuffer[current_line * 160 + pixel] = colors[mapped_color];
                    }
                }
            }
        }
    }
}

} // namespace Aegis