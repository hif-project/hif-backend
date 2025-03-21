/// @file hif_systemc_extension_ArrayConcat.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif_systemc_extension_ArrayConcat.hpp"

namespace hif_systemc_extensions
{

template <class RetElementType> template <typename T> T ArrayConcat<RetElementType>::_getValue(T ret) { return ret; }

template <class RetElementType>
template <typename T>
T ArrayConcat<RetElementType>::_getValue(sc_core::sc_signal<T> &ret)
{
    return ret.read();
}

template <class RetElementType>
template <typename T>
T ArrayConcat<RetElementType>::_getValue(sc_core::sc_inout<T> &ret)
{
    return ret.read();
}

template <class RetElementType> template <typename T> T ArrayConcat<RetElementType>::_getValue(sc_core::sc_in<T> &ret)
{
    return ret.read();
}

template <class RetElementType> template <typename T> T ArrayConcat<RetElementType>::_getValue(sc_core::sc_out<T> &ret)
{
    return ret.read();
}

template <class RetElementType>
ArrayConcat<RetElementType>::ArrayConcat()
    : _result(nullptr)
{
    // ntd
}

template <class RetElementType> ArrayConcat<RetElementType>::~ArrayConcat() { delete[] _result; }

template <class RetElementType>
template <int s1, int s2, typename T1, typename T2>
RetElementType *ArrayConcat<RetElementType>::concatArrays(const T1 &p1, const T2 &p2)
{
    _result = new RetElementType[s1 + s2];
    for (int i = 0; i < s1; ++i) {
        _result[i] = _getValue(p1[i]);
    }

    for (int i = 0; i < s2; ++i) {
        _result[s1 + i] = _getValue(p2[i]);
    }

    return _result;
}

template <class RetElementType>
template <int s2, typename T1, typename T2>
RetElementType *ArrayConcat<RetElementType>::concatValueWithArray(T1 p1, const T2 &p2)
{
    _result = new RetElementType[1 + s2];

    _result[0] = _getValue(p1);

    for (int i = 0; i < s2; ++i) {
        _result[1 + i] = _getValue(p2[i]);
    }

    return _result;
}

template <class RetElementType>
template <int s1, typename T1, typename T2>
RetElementType *ArrayConcat<RetElementType>::concatArrayWithValue(const T1 &p1, T2 p2)
{
    _result = new RetElementType[s1 + 1];
    for (int i = 0; i < s1; ++i) {
        _result[i] = _getValue(p1[i]);
    }

    _result[s1] = _getValue(p2);

    return _result;
}

} // namespace hif_systemc_extensions
