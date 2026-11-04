#ifndef STD_STRING_H
#define STD_STRING_H

#include <stddef.h>

extern "C" {

size_t strlen(const char* str);
void* memset(void* dest, int ch, size_t count);
}

#endif // STD_STRING_H
