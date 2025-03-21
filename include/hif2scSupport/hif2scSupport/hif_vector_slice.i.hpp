/// @file hif_vector_slice.i.hpp
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

template <typename T>
std::vector<T> hif_vector_slice(const sc_core::sc_vector<sc_core::sc_signal<T>> &array, const int left, const int right)
{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<T> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(array[i].read());
    }

    return ret;
}

template <typename T>
std::vector<T> hif_vector_slice(const sc_core::sc_vector<sc_core::sc_in<T>> &array, const int left, const int right)
{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<T> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(array[i].read());
    }

    return ret;
}

template <typename T>
std::vector<T> hif_vector_slice(const sc_core::sc_vector<sc_core::sc_out<T>> &array, const int left, const int right)
{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<T> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(array[i].read());
    }

    return ret;
}

template <typename T>
std::vector<T> hif_vector_slice(const sc_core::sc_vector<sc_core::sc_inout<T>> &array, const int left, const int right)
{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<T> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(array[i].read());
    }

    return ret;
}

template <typename T> std::vector<T> hif_vector_slice(const std::vector<T> &array, const int left, const int right)
{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<T> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(array[i]);
    }

    return ret;
}

template <typename T>
std::vector<std::vector<T>> hif_vector_slice(
    const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_signal<T>>> &array,
    const int left,
    const int right)

{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<std::vector<T>> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(hif_vector_slice(array[i], 0, array[i].size() - 1));
    }

    return ret;
}

template <typename T>
std::vector<std::vector<T>> hif_vector_slice(
    const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_in<T>>> &array,
    const int left,
    const int right)

{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<std::vector<T>> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(hif_vector_slice(array[i], 0, array[i].size() - 1));
    }

    return ret;
}

template <typename T>
std::vector<std::vector<T>> hif_vector_slice(
    const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_out<T>>> &array,
    const int left,
    const int right)

{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<std::vector<T>> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(hif_vector_slice(array[i], 0, array[i].size() - 1));
    }

    return ret;
}

template <typename T>
std::vector<std::vector<T>> hif_vector_slice(
    const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_inout<T>>> &array,
    const int left,
    const int right)
{
    assert(right >= 0);
    assert(right <= left);
    assert(left < array.size());
    std::vector<std::vector<T>> ret;

    for (int i = right; i <= left; ++i) {
        ret.push_back(hif_vector_slice(array[i], 0, array[i].size() - 1));
    }

    return ret;
}

template <typename T> T *hif_vector_slice(T *array, const int /*left*/, const int right)
{
    assert(right >= 0);
    return array + right;
}

} // namespace hif_systemc_extensions
