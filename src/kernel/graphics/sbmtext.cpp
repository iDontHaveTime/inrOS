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

static const char* hex_digits = "0123456789ABCDEF";

template<>
SBMText& SBMText::printHex(unsigned char n) {
    char buffer[sizeof(n) * 2];
    memset(buffer, '0', sizeof(buffer));

    for(unsigned i = 0; i < sizeof(buffer); i++) {
        buffer[i] = hex_digits[((n >> (((sizeof(n) * 8) - 4) - i * 4)) & 0xF)];
    }

    return write(buffer, sizeof(buffer));
}

template<>
SBMText& SBMText::printHex(unsigned short n) {
    char buffer[sizeof(n) * 2];
    memset(buffer, '0', sizeof(buffer));

    for(unsigned i = 0; i < sizeof(buffer); i++) {
        buffer[i] = hex_digits[((n >> (((sizeof(n) * 8) - 4) - i * 4)) & 0xF)];
    }

    return write(buffer, sizeof(buffer));
}

template<>
SBMText& SBMText::printHex(unsigned n) {
    char buffer[sizeof(n) * 2];
    memset(buffer, '0', sizeof(buffer));

    for(unsigned i = 0; i < sizeof(buffer); i++) {
        buffer[i] = hex_digits[((n >> (((sizeof(n) * 8) - 4) - i * 4)) & 0xF)];
    }

    return write(buffer, sizeof(buffer));
}

template<>
SBMText& SBMText::printHex(unsigned long n) {
    char buffer[sizeof(n) * 2];
    memset(buffer, '0', sizeof(buffer));

    for(unsigned i = 0; i < sizeof(buffer); i++) {
        buffer[i] = hex_digits[((n >> (((sizeof(n) * 8) - 4) - i * 4)) & 0xF)];
    }

    return write(buffer, sizeof(buffer));
}

template<>
SBMText& SBMText::printHex(unsigned long long n) {
    char buffer[sizeof(n) * 2];
    memset(buffer, '0', sizeof(buffer));

    for(unsigned i = 0; i < sizeof(buffer); i++) {
        buffer[i] = hex_digits[((n >> (((sizeof(n) * 8) - 4) - i * 4)) & 0xF)];
    }

    return write(buffer, sizeof(buffer));
}
