#include "sbmtext.h"

#include "font/ascii.cpp"
#include "gdriver.h"

static inline void putOneLineToGraphics(uint8_t line, GraphicsDriver& gd,
                                        unsigned x, unsigned y, uint8_t r,
                                        uint8_t g, uint8_t b) {
    for(unsigned i = 0; i < 8; i++) {
        if(line & 1) {
            gd.drawPixel(x + i, y, r, g, b);
        }
        line >>= 1;
    }
}

SBMText& SBMText::put(char c) {
    if(c == '\n') {
        newline();
    }
    else if(c == '\r') {
        carriageret();
    }
    else {
        if(charColumn_ == columns_) return *this;

        if((unsigned char)c > 0x7F) {
            c = '?';
        }

        const unsigned char* fontline = font8x8_basic[(unsigned char)c];

        unsigned x = charColumn_ * 8;
        unsigned y = charLine_ * 8;

        for(unsigned i = 0; i < 8; i++) {
            putOneLineToGraphics(fontline[i], graphics_, x, y++, red_, green_,
                                 blue_);
        }

        charColumn_++;
    }

    return *this;
}
