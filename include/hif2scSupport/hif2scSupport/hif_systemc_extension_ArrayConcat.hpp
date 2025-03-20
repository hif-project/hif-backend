/// @file hif_systemc_extension_ArrayConcat.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "config.hpp"
#include <systemc>
#include <vector>

namespace hif_systemc_extensions
{

template <class RetElementType>
class ArrayConcat
{
public:
    ArrayConcat();
    ~ArrayConcat();

    template <int s1, int s2, typename T1, typename T2>
    RetElementType *concatArrays(const T1 &p1, const T2 &p2);

    template <int s2, typename T1, typename T2>
    RetElementType *concatValueWithArray(T1 p1, const T2 &p2);

    template <int s1, typename T1, typename T2>
    RetElementType *concatArrayWithValue(const T1 &p1, T2 p2);

private:
    template <typename T>
    T _getValue(T ret);

    template <typename T>
    T _getValue(sc_core::sc_signal<T> &ret);

    template <typename T>
    T _getValue(sc_core::sc_inout<T> &ret);

    template <typename T>
    T _getValue(sc_core::sc_in<T> &ret);

    template <typename T>
    T _getValue(sc_core::sc_out<T> &ret);

    RetElementType *_result;

    ArrayConcat(const ArrayConcat &)            = delete;
    ArrayConcat &operator=(const ArrayConcat &) = delete;
};

} // namespace hif_systemc_extensions

#include "hif_systemc_extension_ArrayConcat.i.hpp"
