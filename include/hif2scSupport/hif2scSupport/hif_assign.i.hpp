/// @file hif_assign.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif_assign.hpp"

#include <vector>

namespace hif_systemc_extensions
{

template <class T>
struct TemporaryType {
    typedef T type;
};

template <int S>
struct TemporaryType<sc_dt::sc_bv<S>> {
    typedef sc_dt::sc_logic type;
};

template <int S>
struct TemporaryType<sc_dt::sc_lv<S>> {
    typedef sc_dt::sc_logic type;
};

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int S>
struct TemporaryType<hdtlib::hl_bv_t<S>> {
    typedef bool type;
};

template <int S>
struct TemporaryType<hdtlib::hl_lv_t<S>> {
    typedef hdtlib::hl_logic_t type;
};

#endif //HIF2SCSUPPORT_USE_HDTLIB

template <>
struct TemporaryType<uint8_t> {
    typedef bool type;
};

template <>
struct TemporaryType<uint16_t> {
    typedef bool type;
};

template <>
struct TemporaryType<uint32_t> {
    typedef bool type;
};

template <>
struct TemporaryType<uint64_t> {
    typedef bool type;
};
template <>
struct TemporaryType<int8_t> {
    typedef bool type;
};

template <>
struct TemporaryType<int16_t> {
    typedef bool type;
};

template <>
struct TemporaryType<int32_t> {
    typedef bool type;
};

template <>
struct TemporaryType<int64_t> {
    typedef bool type;
};

// ------------------------------------------------------------

template <typename T>
const T &_hif_assign_getValue(const T &ret)
{
    return ret;
}

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<8> _hif_assign_getValue(const uint8_t &ret);

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<16> _hif_assign_getValue(const uint16_t &ret);

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<32> _hif_assign_getValue(const uint32_t &ret);

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<64> _hif_assign_getValue(const uint64_t &ret);

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<8> _hif_assign_getValue(const int8_t &ret);

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<16> _hif_assign_getValue(const int16_t &ret);

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<32> _hif_assign_getValue(const int32_t &ret);

HIF2SCSUPPORT_EXPORT
const sc_dt::sc_bv<64> _hif_assign_getValue(const int64_t &ret);

template <typename T>
const T &_hif_assign_getValue(const sc_core::sc_signal<T> &ret)
{
    return _hif_assign_getValue(ret.read());
}

template <typename T>
const T &_hif_assign_getValue(const sc_core::sc_inout<T> &ret)
{
    return _hif_assign_getValue(ret.read());
}

template <typename T>
const T &_hif_assign_getValue(const sc_core::sc_in<T> &ret)
{
    return _hif_assign_getValue(ret.read());
}

template <typename T>
const T &_hif_assign_getValue(const sc_core::sc_out<T> &ret)
{
    return _hif_assign_getValue(ret.read());
}

// ------------------------------------------------------------

template <typename T>
int _hif_assign_getMemberLength(const T & /*array*/)
{
    return 1;
}

template <template <int> class T, int i>
int _hif_assign_getMemberLength(const T<i> & /*array*/)
{
    return i;
}

template <typename T>
int _hif_assign_getMemberLength(const sc_core::sc_signal<T> &array)
{
    return _hif_assign_getMemberLength(array.read());
}

template <typename T>
int _hif_assign_getMemberLength(const sc_core::sc_inout<T> &array)
{
    return _hif_assign_getMemberLength(array.read());
}

template <typename T>
int _hif_assign_getMemberLength(const sc_core::sc_in<T> &array)
{
    return _hif_assign_getMemberLength(array.read());
}

template <typename T>
int _hif_assign_getMemberLength(const sc_core::sc_out<T> &array)
{
    return _hif_assign_getMemberLength(array.read());
}

template <typename T>
int _hif_assign_getMemberLength(const sc_core::sc_vector<T> &array)
{
    return array.size();
}

template <typename T>
int _hif_assign_getMemberLength(const std::vector<T> &array)
{
    return array.size();
}

template <typename T, size_t size>
int _hif_assign_getMemberLength(const T (& /*array*/)[size])
{
    return int(size);
}

// ------------------------------------------------------------

template <typename T, typename S>
void _hif_set_bit(T &target, const uint64_t index, S source)
{
    target.set_bit(index, source);
}

template <typename S>
void _hif_set_bit(uint8_t &target, const uint64_t index, S source)
{
    if (source)
        target |= uint8_t(1) << index;
    else
        target &= ~(uint8_t(1) << index);
}

template <typename S>
void _hif_set_bit(uint16_t &target, const uint64_t index, S source)
{
    if (source)
        target |= uint16_t(1) << index;
    else
        target &= ~(uint16_t(1) << index);
}

template <typename S>
void _hif_set_bit(uint32_t &target, const uint64_t index, S source)
{
    if (source)
        target |= uint32_t(1) << index;
    else
        target &= ~(uint32_t(1) << index);
}

template <typename S>
void _hif_set_bit(uint64_t &target, const uint64_t index, S source)
{
    if (source)
        target |= uint64_t(1) << index;
    else
        target &= ~(uint64_t(1) << index);
}

template <typename S>
void _hif_set_bit(int8_t &target, const uint64_t index, S source)
{
    if (source)
        target |= int8_t(1) << index;
    else
        target &= ~(int8_t(1) << index);
}

template <typename S>
void _hif_set_bit(int16_t &target, const uint64_t index, S source)
{
    if (source)
        target |= int16_t(1) << index;
    else
        target &= ~(int16_t(1) << index);
}

template <typename S>
void _hif_set_bit(int32_t &target, const uint64_t index, S source)
{
    if (source)
        target |= int32_t(1) << index;
    else
        target &= ~(int32_t(1) << index);
}

template <typename S>
void _hif_set_bit(int64_t &target, const uint64_t index, S source)
{
    if (source)
        target |= int64_t(1) << index;
    else
        target &= ~(int64_t(1) << index);
}

// ------------------------------------------------------------

template <typename T, typename S>
void _hif_assign_setValue(
    T &target,
    const S &source,
    unsigned int /*size*/,
    unsigned int /*left1*/,
    unsigned int /*right1*/,
    unsigned int /*left2*/,
    unsigned int /*right2*/
)
{
    target = T(source);
}

template <typename S>
void _hif_assign_setValue(
    bool &target,
    const sc_dt::sc_bitref<S> &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    hif_assign(target, source.to_bool(), size, left1, right1, left2, right2);
}

template <typename T, typename S>
void _hif_assign_setValue(
    T &target,
    const sc_core::sc_signal<S> &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    hif_assign(target, source.read(), size, left1, right1, left2, right2);
}

template <typename T, typename S>
void _hif_assign_setValue(
    T &target,
    const sc_core::sc_in<S> &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    hif_assign(target, source.read(), size, left1, right1, left2, right2);
}

template <typename T, typename S>
void _hif_assign_setValue(
    T &target,
    const sc_core::sc_out<S> &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    hif_assign(target, source.read(), size, left1, right1, left2, right2);
}

template <typename T, typename S>
void _hif_assign_setValue(
    T &target,
    const sc_core::sc_inout<S> &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    hif_assign(target, source.read(), size, left1, right1, left2, right2);
}

template <typename T, typename S>
void _hif_assign_setValue(
    T &target,
    const sc_core::sc_vector<S> &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    if (left1 == static_cast<unsigned int>(-1))
        left1 = 0;
    const unsigned int min = left1 < right1 ? left1 : right1;

    // Workaround to avoid needing of const references.
    typename TemporaryType<T>::type tmp;
    for (unsigned int i = 0; i < size; ++i) {
        // Call recursive case with less dimension (base).
        hif_assign(tmp, source[i], _hif_assign_getMemberLength(_hif_assign_getValue(source)[i]), left2, right2);
        _hif_set_bit(target, i + min, to_bit(tmp));
    }
}

template <typename T, typename S>
void _hif_assign_setValue(
    T &target,
    const std::vector<S> &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    if (left1 == static_cast<unsigned int>(-1))
        left1 = 0;
    const unsigned int min = left1 < right1 ? left1 : right1;

    // Workaround to avoid needing of const references.
    typename TemporaryType<T>::type tmp;
    for (unsigned int i = 0; i < size; ++i) {
        // Call recursive case with less dimension (base).
        hif_assign(tmp, source[i], _hif_assign_getMemberLength(_hif_assign_getValue(source)[i]), left2, right2);
        _hif_set_bit(target, i + min, to_bit(tmp));
    }
}

template <typename T, typename S, size_t as>
void _hif_assign_setValue(
    T &target,
    S (&source)[as],
    size_t size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    if (left1 == static_cast<unsigned int>(-1))
        left1 = 0;
    const unsigned int min = left1 < right1 ? left1 : right1;

    // Workaround to avoid needing of const references.
    typename TemporaryType<T>::type tmp;
    for (size_t i = 0; i < size; ++i) {
        // Call recursive case with less dimension (base).
        hif_assign(tmp, source[i], _hif_assign_getMemberLength(_hif_assign_getValue(source)[i]), left2, right2);
        _hif_set_bit(target, i + min, to_bit(tmp));
    }
}

// ------------------------------------------------------------

template <typename T, typename S>
void hif_assign(
    T &target,
    const S &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    _hif_assign_setValue(target, source, size, left1, right1, left2, right2);
}

template <typename T, typename S>
void hif_assign(
    sc_core::sc_signal<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    T tmp(target.read());
    hif_assign(tmp, source, size, left1, right1, left2, right2);
    target = tmp;
}

template <typename T, typename S>
void hif_assign(
    sc_core::sc_out<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    T tmp(target.read());
    hif_assign(tmp, source, size, left1, right1, left2, right2);
    target = tmp;
}

template <typename T, typename S>
void hif_assign(
    sc_core::sc_inout<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    T tmp(target.read());
    hif_assign(tmp, source, size, left1, right1, left2, right2);
    target = tmp;
}

template <typename T, typename S>
void hif_assign(
    sc_core::sc_vector<T> &target,
    const S &source,
    unsigned int size,
    unsigned int left1,
    unsigned int right1,
    unsigned int left2,
    unsigned int right2)
{
    if (left1 == static_cast<unsigned int>(-1))
        left1 = 0;
    const unsigned int min1 = left1 < right1 ? left1 : right1;

    // Cycling on the rows (1st dimension).
    for (unsigned int i = 0; i < size; ++i) {
        // Call recursive case with less dimension (array, no array).
        hif_assign(
            target[i + min1], _hif_assign_getValue(source)[i],
            _hif_assign_getMemberLength(_hif_assign_getValue(source)[i]), left2, right2);
    }
}

template <typename T, typename S, size_t as>
void hif_assign(T (&target)[as], const S &source, size_t size, size_t left1, size_t right1, size_t left2, size_t right2)
{
    if (left1 == static_cast<size_t>(-1))
        left1 = 0;
    const size_t min1 = left1 < right1 ? left1 : right1;

    // Cycling on the rows (1st dimension).
    for (size_t i = 0; i < size; ++i) {
        // Call recursive case with less dimension (array, no array).
        hif_assign(
            target[i + min1], _hif_assign_getValue(source)[i],
            _hif_assign_getMemberLength(_hif_assign_getValue(source)[i]), left2, right2);
    }
}

template <typename T, typename S, size_t as1, size_t as2>
void hif_assign(
    T (&target)[as1][as2],
    const S &source,
    size_t size,
    size_t left1,
    size_t right1,
    size_t left2,
    size_t right2)
{
    if (left1 == static_cast<size_t>(-1))
        left1 = 0;
    const size_t min1 = left1 < right1 ? left1 : right1;

    // Cycling on the rows (1st dimension).
    for (size_t i = 0; i < size; ++i) {
        // Call recursive case with less dimension (array, no array).
        hif_assign(
            target[i + min1], _hif_assign_getValue(source)[i],
            _hif_assign_getMemberLength(_hif_assign_getValue(source)[i]), left2, right2);
    }
}

} // namespace hif_systemc_extensions
