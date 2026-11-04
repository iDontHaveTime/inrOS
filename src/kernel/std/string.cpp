#include "string.h"

extern "C" {

size_t strlen(const char* str) {
    size_t i = 0;
    while(str[i] != '\0') {
        i++;
    }
    return i;
}

void* memset(void* dest, int ch, size_t count) {
    char* p = (char*)dest;

    while(count--) {
        *p++ = (unsigned char)ch;
    }

    return dest;
}
}
