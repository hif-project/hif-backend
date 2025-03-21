/// @file hif_systemc_extensions_HifAggregate.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_systemc_extensions
{

// /////////////////////////////////////////////////////////////////////////
// HifAggregateVector
// /////////////////////////////////////////////////////////////////////////

template <class C>
HifAggregateVector<C>::HifAggregateVector()
    : _result()
{
    // ntd
}

template <class C> HifAggregateVector<C>::~HifAggregateVector()
{
    // ntd
}

template <class C> typename HifAggregateVector<C>::VectorTypeReturn HifAggregateVector<C>::getResult()
{
    return _result;
}

template <class C>
typename HifAggregateVector<C>::ChildType &HifAggregateVector<C>::addPair(const int index, const ValueType value)
{
    static_cast<ChildType *>(this)->setMember(index, value);
    return *static_cast<ChildType *>(this);
}

template <class C>
typename HifAggregateVector<C>::ChildType &
HifAggregateVector<C>::addPairSet(const int lbound, const int rbound, const ValueType value)
{
    const bool gt      = (lbound > rbound);
    const int minbound = gt ? rbound : lbound;
    const int maxbound = gt ? lbound : rbound;

    for (int i = minbound; i <= maxbound; ++i) {
        static_cast<ChildType *>(this)->setMember(i, value);
    }

    return *static_cast<ChildType *>(this);
}

template <class C> typename HifAggregateVector<C>::ChildType &HifAggregateVector<C>::setOthers(const ValueType others)
{
    for (int i = 0; i < SIZE; ++i) {
        static_cast<ChildType *>(this)->setMember(i, others);
    }

    return *static_cast<ChildType *>(this);
}

// /////////////////////////////////////////////////////////////////////////
// HifAggregateArray
// /////////////////////////////////////////////////////////////////////////

template <class T, int size>
HifAggregateArray<T, size>::HifAggregateArray()
    : HifAggregateVector<HifAggregateArray<T, size>>()
{
    // ntd
}

template <class T, int size> HifAggregateArray<T, size>::~HifAggregateArray()
{
    // ntd
}

template <class T, int size> void HifAggregateArray<T, size>::setMember(int index, ValueType value)
{
    this->_result[index] = value;
}

// /////////////////////////////////////////////////////////////////////////
// HifAggregateBitVector
// /////////////////////////////////////////////////////////////////////////

template <int size>
HifAggregateBitVector<size>::HifAggregateBitVector()
    : HifAggregateVector<HifAggregateBitVector<size>>()
{
    // ntd
}

template <int size> HifAggregateBitVector<size>::~HifAggregateBitVector()
{
    // ntd
}

template <int size> void HifAggregateBitVector<size>::setMember(int index, ValueType value)
{
    this->_result[index] = value;
}

// /////////////////////////////////////////////////////////////////////////
// HifAggregateLogicVector
// /////////////////////////////////////////////////////////////////////////

template <int size>
HifAggregateLogicVector<size>::HifAggregateLogicVector()
    : HifAggregateVector<HifAggregateLogicVector<size>>()
{
    // ntd
}

template <int size> HifAggregateLogicVector<size>::~HifAggregateLogicVector()
{
    // ntd
}

template <int size> void HifAggregateLogicVector<size>::setMember(int index, ValueType value)
{
    this->_result[index] = value;
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////
// HifAggregateHlBv
// /////////////////////////////////////////////////////////////////////////

template <int size>
HifAggregateHlBv<size>::HifAggregateHlBv()
    : HifAggregateVector<HifAggregateHlBv<size>>()
{
    // ntd
}

template <int size> HifAggregateHlBv<size>::~HifAggregateHlBv()
{
    // ntd
}

template <int size> void HifAggregateHlBv<size>::setMember(int index, ValueType value)
{
    this->_result.set_bit(index, value);
}

// /////////////////////////////////////////////////////////////////////////
// HifAggregateHlLv
// /////////////////////////////////////////////////////////////////////////

template <int size>
HifAggregateHlLv<size>::HifAggregateHlLv()
    : HifAggregateVector<HifAggregateHlLv<size>>()
{
    // ntd
}

template <int size> HifAggregateHlLv<size>::~HifAggregateHlLv()
{
    // ntd
}

template <int size> void HifAggregateHlLv<size>::setMember(int index, ValueType value)
{
    this->_result.set_bit(index, value);
}

#endif // HIF2SCSUPPORT_USE_HDTLIB

} // namespace hif_systemc_extensions
