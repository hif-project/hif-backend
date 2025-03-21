/// @file hif_vector_slice.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <systemc>
#include <vector>

namespace hif_systemc_extensions
{

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_signal<T>> &array, int left, int right) -> std::vector<T>;

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_in<T>> &array, int left, int right) -> std::vector<T>;

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_out<T>> &array, int left, int right) -> std::vector<T>;

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_inout<T>> &array, int left, int right) -> std::vector<T>;

template <typename T> auto hif_vector_slice(const std::vector<T> &array, int left, int right) -> std::vector<T>;

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_signal<T>>> &array, int left, int right)
    -> std::vector<std::vector<T>>;

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_in<T>>> &array, int left, int right)
    -> std::vector<std::vector<T>>;

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_out<T>>> &array, int left, int right)
    -> std::vector<std::vector<T>>;

template <typename T>
auto hif_vector_slice(const sc_core::sc_vector<sc_core::sc_vector<sc_core::sc_inout<T>>> &array, int left, int right)
    -> std::vector<std::vector<T>>;

template <typename T> auto hif_vector_slice(T *array, int left, int right) -> T *;

} // namespace hif_systemc_extensions

/////////////////////////////////////////
// Template-implementation header
/////////////////////////////////////////

#include "hif_vector_slice.i.hpp"
