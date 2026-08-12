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

template <class RetElementType> class ArrayConcat
{
public:
    ArrayConcat();
    ~ArrayConcat();

    template <int s1, int s2, typename T1, typename T2>
    auto concatArrays(const T1 &p1, const T2 &p2) -> RetElementType *;

    template <int s2, typename T1, typename T2> auto concatValueWithArray(T1 p1, const T2 &p2) -> RetElementType *;

    template <int s1, typename T1, typename T2> auto concatArrayWithValue(const T1 &p1, T2 p2) -> RetElementType *;

private:
    template <typename T> auto _getValue(T ret) -> T;

    template <typename T> auto _getValue(sc_core::sc_signal<T> &ret) -> T;

    template <typename T> auto _getValue(sc_core::sc_inout<T> &ret) -> T;

    template <typename T> auto _getValue(sc_core::sc_in<T> &ret) -> T;

    template <typename T> auto _getValue(sc_core::sc_out<T> &ret) -> T;

    RetElementType *_result;

    ArrayConcat(const ArrayConcat &)                     = delete;
    auto operator=(const ArrayConcat &) -> ArrayConcat & = delete;
};

} // namespace hif_systemc_extensions

#include "hif_systemc_extension_ArrayConcat.i.hpp"
