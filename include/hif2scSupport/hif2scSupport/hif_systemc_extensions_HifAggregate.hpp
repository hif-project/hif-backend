/// @file hif_systemc_extensions_HifAggregate.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_systemc_extensions
{

template <typename C> struct HifAggregateVectorTraits;

// /////////////////////////////////////////////////////////////////////////
// HifAggregateVector
// /////////////////////////////////////////////////////////////////////////

/// @brief This class creates a vector value, mapping VHDL aggregate.
/// @tparam C The type-size of returned vector
template <class C> class HifAggregateVector
{
public:
    /// @name Traits
    /// @{
    typedef HifAggregateVector<C> ThisType;

    typedef C ChildType;

    typedef typename HifAggregateVectorTraits<C>::VectorType VectorType;

    // Note: introduced for Array specialization, since VectorType as array
    // cannot be the returned type of a method.
    typedef typename HifAggregateVectorTraits<C>::VectorTypeReturn VectorTypeReturn;

    typedef typename HifAggregateVectorTraits<C>::ValueType ValueType;

    enum VectorSize { SIZE = HifAggregateVectorTraits<C>::SIZE };
    /// @}

    /// @brief Constructor
    HifAggregateVector();

    /// @brief Destructor
    ~HifAggregateVector();

    /// @brief Return the resulting array value
    VectorTypeReturn getResult();

    /// @brief Create a new pair (index, value).
    /// @param index The pair index
    /// @param value The value to set
    ChildType &addPair(const int index, const ValueType value);

    /// @brief Create a set of pairs (index, value) based on a range.
    /// The value will be replied for indexes from lbound to rbound.
    /// @param lbound The left bound of the range.
    /// @param rbound The right bound of the range.
    /// @param value The value to set
    ///
    ChildType &addPairSet(const int lbound, const int rbound, const ValueType value);

    /// @brief Assign a value to undefined pairs
    /// @param others The value to set
    ChildType &setOthers(const ValueType others);

protected:
    /// @brief The internal storage of result
    VectorType _result;

private:
    // Disabled
    HifAggregateVector(const HifAggregateVector<C> &);
    HifAggregateVector<C> &operator=(const HifAggregateVector<C> &);
};

// /////////////////////////////////////////////////////////////////////////
// HifAggregateArray
// /////////////////////////////////////////////////////////////////////////

/// @brief This class creates an array value, mapping VHDL aggregate.
/// @tparam T The type of returned array
/// @tparam size The type-size of returned array
template <class T, int size> class HifAggregateArray : public HifAggregateVector<HifAggregateArray<T, size>>
{
public:
    typedef HifAggregateArray<T, size> ThisType;
    typedef HifAggregateVectorTraits<ThisType> Traits;
    typedef typename Traits::ValueType ValueType;

    /// @brief Constructor
    HifAggregateArray();

    /// @brief Destructor
    ~HifAggregateArray();

    void setMember(int index, ValueType value);

private:
    // Disabled
    HifAggregateArray(const HifAggregateArray<T, size> &);
    HifAggregateArray<T, size> &operator=(const HifAggregateArray<T, size> &);
};

template <class T, int size> struct HifAggregateVectorTraits<HifAggregateArray<T, size>> {
    typedef T VectorType[size_t(size)];
    typedef T *VectorTypeReturn;
    typedef T ValueType;
    enum VectorSize { SIZE = size };
};

// /////////////////////////////////////////////////////////////////////////
// HifAggregateBitVector
// /////////////////////////////////////////////////////////////////////////

/// @brief This class creates a vector value, mapping VHDL aggregate.
/// @tparam size The type-size of returned vector
template <int size> class HifAggregateBitVector : public HifAggregateVector<HifAggregateBitVector<size>>
{
public:
    typedef HifAggregateBitVector<size> ThisType;
    typedef HifAggregateVectorTraits<ThisType> Traits;
    typedef typename Traits::ValueType ValueType;

    /// @brief Constructor
    HifAggregateBitVector();

    /// @brief Destructor
    ~HifAggregateBitVector();

    void setMember(int index, ValueType value);

private:
    // Disabled
    HifAggregateBitVector(const HifAggregateBitVector<size> &);
    HifAggregateBitVector<size> &operator=(const HifAggregateBitVector<size> &);
};

template <int size> struct HifAggregateVectorTraits<HifAggregateBitVector<size>> {
    typedef sc_dt::sc_bv<size> VectorType;
    typedef VectorType VectorTypeReturn;
    typedef bool ValueType;
    enum VectorSize { SIZE = size };
};

// /////////////////////////////////////////////////////////////////////////
// HifAggregateLogicVector
// /////////////////////////////////////////////////////////////////////////

/// @brief This class creates a vector value, mapping VHDL aggregate.
/// @tparam size The type-size of returned vector
template <int size> class HifAggregateLogicVector : public HifAggregateVector<HifAggregateLogicVector<size>>
{
public:
    typedef HifAggregateLogicVector<size> ThisType;
    typedef HifAggregateVectorTraits<ThisType> Traits;
    typedef typename Traits::ValueType ValueType;

    /// @brief Constructor
    HifAggregateLogicVector();

    /// @brief Destructor
    ~HifAggregateLogicVector();

    void setMember(int index, ValueType value);

private:
    // Disabled
    HifAggregateLogicVector(const HifAggregateLogicVector<size> &);
    HifAggregateLogicVector<size> &operator=(const HifAggregateLogicVector<size> &);
};

template <int size> struct HifAggregateVectorTraits<HifAggregateLogicVector<size>> {
    typedef sc_dt::sc_lv<size> VectorType;
    typedef VectorType VectorTypeReturn;
    typedef sc_dt::sc_logic ValueType;
    enum VectorSize { SIZE = size };
};

#ifdef HIF2SCSUPPORT_USE_HDTLIB

// /////////////////////////////////////////////////////////////////////////
// HifAggregateHlBv
// /////////////////////////////////////////////////////////////////////////

/// @brief This class creates a hdtlib vector value, mapping VHDL aggregate.
/// @tparam size The type-size of returned vector
template <int size> class HifAggregateHlBv : public HifAggregateVector<HifAggregateHlBv<size>>
{
public:
    typedef HifAggregateHlBv<size> ThisType;
    typedef HifAggregateVectorTraits<ThisType> Traits;
    typedef typename Traits::ValueType ValueType;

    /// @brief Constructor
    HifAggregateHlBv();

    /// @brief Destructor
    ~HifAggregateHlBv();

    void setMember(int index, ValueType value);

private:
    // Disabled
    HifAggregateHlBv(const HifAggregateHlBv<size> &);
    HifAggregateHlBv<size> &operator=(const HifAggregateHlBv<size> &);
};

template <int size> struct HifAggregateVectorTraits<HifAggregateHlBv<size>> {
    typedef hdtlib::hl_bv_t<size> VectorType;
    typedef VectorType VectorTypeReturn;
    typedef bool ValueType;
    enum VectorSize { SIZE = size };
};

// /////////////////////////////////////////////////////////////////////////
// HifAggregateHlLv
// /////////////////////////////////////////////////////////////////////////

/// @brief This class creates a hdtlib vector value, mapping VHDL aggregate.
/// @tparam size The type-size of returned vector
template <int size> class HifAggregateHlLv : public HifAggregateVector<HifAggregateHlLv<size>>
{
public:
    typedef HifAggregateHlLv<size> ThisType;
    typedef HifAggregateVectorTraits<ThisType> Traits;
    typedef typename Traits::ValueType ValueType;

    /// @brief Constructor
    HifAggregateHlLv();

    /// @brief Destructor
    ~HifAggregateHlLv();

    void setMember(int index, ValueType value);

private:
    // Disabled
    HifAggregateHlLv(const HifAggregateHlLv<size> &);
    HifAggregateHlLv<size> &operator=(const HifAggregateHlLv<size> &);
};

template <int size> struct HifAggregateVectorTraits<HifAggregateHlLv<size>> {
    typedef hdtlib::hl_lv_t<size> VectorType;
    typedef VectorType VectorTypeReturn;
    typedef hdtlib::hl_logic_t ValueType;
    enum VectorSize { SIZE = size };
};

#endif // HIF2SCSUPPORT_USE_HDTLIB

} // namespace hif_systemc_extensions

#include "hif_systemc_extensions_HifAggregate.i.hpp"
