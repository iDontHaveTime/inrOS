#ifndef GRAPHICS_SBMTEXT_H
#define GRAPHICS_SBMTEXT_H

#include <stddef.h>

#include "../std/math.h"
#include "../std/string.h"
#include "gdriver.h"

/// @brief Simple bitmap text renderer for the graphics driver struct.
/// @note Uses '\n' for newline.
///
/// Uses an 8x8 font as described in https://github.com/dhepper/font8x8
class SBMText {
    GraphicsDriver& graphics_;
    unsigned lines_, columns_, charLine_, charColumn_;
    uint8_t red_, green_, blue_;

public:
    SBMText(GraphicsDriver& graphics) :
        graphics_(graphics),
        charLine_(0),
        charColumn_(0),
        red_(0xFF),
        green_(0xFF),
        blue_(0xFF) {
        lines_ = graphics.getVerticalRes() / 8;
        columns_ = graphics.getHorizontalRes() / 8;
    }

    void newline() {
        carriageret();
        if(charLine_ < lines_) charLine_++;
    }

    void carriageret() {
        charColumn_ = 0;
    }

    SBMText& put(char c);

    SBMText& write(const char* str, size_t n) {
        const char* it = str;
        const char* end = str + n;

        for(; it != end; ++it) {
            put(*it);
        }

        return *this;
    }

    SBMText& operator<<(const char* str) {
        return write(str, strlen(str));
    }

    SBMText& operator<<(char c) {
        return put(c);
    }

    // Prefer explicit cast.
    SBMText& operator<<(signed char) = delete;
    SBMText& operator<<(unsigned char) = delete;

    template<typename T>
    SBMText& printHex(T) = delete;

    template<>
    SBMText& printHex(unsigned char);

    template<>
    SBMText& printHex(unsigned short);

    template<>
    SBMText& printHex(unsigned);

    template<>
    SBMText& printHex(unsigned long);

    template<>
    SBMText& printHex(unsigned long long);

    template<>
    SBMText& printHex(const void* ptr) {
        return printHex(uintptr_t(ptr));
    }

    SBMText& operator<<(const void* ptr) {
        return printHex(ptr);
    }

    template<typename T>
    requires(traits::is_integer<T>::value)
    SBMText& operator<<(T n) {
        char buffer[integer_description<T>::max_decimal_digits + 5];
        size_t i = 0;
        bool sign = false;

        if constexpr(!integer_description<T>::is_unsigned) {
            if(n < 0) {
                sign = true;
                n = -n;
            }
        }

        while(n) {
            buffer[i++] = n % 10 + '0';
            n /= 10;
        }

        if constexpr(!integer_description<T>::is_unsigned) {
            if(sign) {
                buffer[i++] = '-';
            }
        }

        for(size_t j = 0, k = i - 1; j < k; j++, k--) {
            char temp = buffer[j];
            buffer[j] = buffer[k];
            buffer[k] = temp;
        }

        return write(buffer, i);
    }

    SBMText& changeColor(uint8_t r, uint8_t g, uint8_t b) {
        red_ = r;
        green_ = g;
        blue_ = b;
        return *this;
    }
};

#endif // GRAPHICS_SBMTEXT_H
