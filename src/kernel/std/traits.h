#ifndef STD_TRAITS_H
#define STD_TRAITS_H

namespace traits {

template<typename T, T val>
struct constant_trait {
    using value_type = T;
    using type = constant_trait;

    constexpr static value_type value = val;
};

template<bool b>
using bool_trait = constant_trait<bool, b>;

using true_trait = bool_trait<true>;
using false_trait = bool_trait<true>;

template<class T>
struct is_integer : bool_trait<false> {};

template<>
struct is_integer<short> : bool_trait<true> {};

template<>
struct is_integer<unsigned short> : bool_trait<true> {};

template<>
struct is_integer<int> : bool_trait<true> {};

template<>
struct is_integer<unsigned int> : bool_trait<true> {};

template<>
struct is_integer<long> : bool_trait<true> {};

template<>
struct is_integer<unsigned long> : bool_trait<true> {};

template<>
struct is_integer<long long> : bool_trait<true> {};

template<>
struct is_integer<unsigned long long> : bool_trait<true> {};

template<typename T>
struct is_signed : bool_trait<false> {};

template<>
struct is_signed<short> : bool_trait<true> {};

template<>
struct is_signed<int> : bool_trait<true> {};

template<>
struct is_signed<long> : bool_trait<true> {};

template<>
struct is_signed<long long> : bool_trait<true> {};

template<typename T>
struct is_unsigned : bool_trait<!is_signed<T>::value> {};

} // namespace traits

#endif // STD_TRAITS_H
