#ifndef STD_MATH_H
#define STD_MATH_H

#include <stddef.h>

#include "traits.h"

template<typename T>
requires(traits::is_integer<T>::value)
struct integer_description {
    constexpr static bool is_unsigned = traits::is_unsigned<T>::value;
    constexpr static size_t bit_width = sizeof(T) * 8;
    constexpr static size_t max_decimal_digits = (bit_width * 28) / 93 + 1;
};

#endif // STD_MATH_H
