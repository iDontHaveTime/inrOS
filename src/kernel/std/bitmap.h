#ifndef STD_BITMAP_H
#define STD_BITMAP_H

#include <stddef.h>

#include "string.h"

/// @brief A bitmap without owning the memory.
class kbitmap {
    char* memory_;
    size_t bytes_;

public:
    kbitmap(const kbitmap&) = default;
    kbitmap(kbitmap&&) = default;
    kbitmap& operator=(const kbitmap&) = default;
    kbitmap& operator=(kbitmap&&) = default;

    kbitmap(char* memory, size_t bytes) : memory_(memory), bytes_(bytes) {}

    char* data() {
        return memory_;
    }

    const char* data() const {
        return memory_;
    }

    /// @brief Returns the size in bytes.
    size_t size() const {
        return bytes_;
    }

    size_t bit_size() const {
        return bytes_ * 8;
    }

    void setAllBits(bool b) {
        memset(memory_, b ? -1 : 0, bytes_);
    }

    void setBit(size_t bit, bool b) {
        size_t byte = bit / 8;
        unsigned bitI = bit % 8;
        if(b) {
            memory_[byte] |= 1U << bitI;
        }
        else {
            memory_[byte] &= ~(1U << bitI);
        }
    }

    bool getBit(size_t bit) {
        size_t byte = bit / 8;
        unsigned bitI = bit % 8;
        return memory_[byte] & (1U << bitI);
    }

    size_t getFirstZeroBitInRange(size_t bitBegin, size_t bitEnd) {
        if(bitBegin > bitEnd || bitEnd > bit_size()) return -1;

        for(size_t i = bitBegin; i < bitEnd; i++) {
            if(!getBit(i)) return i;
        }

        return -1;
    }

    void setNBits(size_t bigBegin, size_t n, bool b) {
        for(size_t i = 0; i < n; i++) {
            setBit(bigBegin + i, b);
        }
    }
};

#endif // STD_BITMAP_H
