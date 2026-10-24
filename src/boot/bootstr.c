#include "bootstr.h"

VOID* EFIAPI memcpy(VOID* restrict dest, const VOID* restrict src, size_t n) {
    char* d = dest;
    const char* s = src;

    while(n--) {
        *d++ = *s++;
    }

    return dest;
}

VOID* EFIAPI memset(VOID* dest, int c, size_t n) {
    char* d = dest;
    while(n--) {
        *d++ = (unsigned char)c;
    }

    return dest;
}
