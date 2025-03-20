/// @file hif_after.i.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <sysc/kernel/sc_dynamic_processes.h>

#include "hif_after.hpp"

namespace hif_systemc_extensions
{

// ///////////////////////////////////////////////////////////////////
// HifAfter class
// ///////////////////////////////////////////////////////////////////

template <class Target, class Value>
HifAfter<Target, Value>::HifAfter(Target &t, const Value v, const char *file, const int line)
    : IHifAfter(file, line)
    , _target(&t)
    , _value(v)
{
    // ntd
}

template <class Target, class Value>
HifAfter<Target, Value>::~HifAfter()
{
    // ntd
}

template <class Target, class Value>
HifAfter<Target, Value>::HifAfter(const HifAfter<Target, Value> &other)
    : IHifAfter(other)
    , _target(other._target)
    , _value(other._value)
{
    // ntd
}

template <class Target, class Value>
HifAfter<Target, Value> &HifAfter<Target, Value>::operator=(const HifAfter<Target, Value> &other)
{
    HifAfter<Target, Value> tmp(other);
    swap(tmp);
    return *this;
}

template <class Target, class Value>
void HifAfter<Target, Value>::swap(HifAfter<Target, Value> &other)
{
    IHifAfter::swap(other);
    std::swap(_target, other._target);
    std::swap(_value, other._value);
}

template <class Target, class Value>
void HifAfter<Target, Value>::operator()()
{
    if (IHifAfter::_isActive)
        *_target = _value;
}

// ///////////////////////////////////////////////////////////////////
// hif_after()
// ///////////////////////////////////////////////////////////////////

template <typename Target, typename Value>
void _hif_after(Target &target, const Value v, const sc_core::sc_time &delay, const char *file, const int line)
{
    HifAfter<Target, Value> func(target, v, file, line);
    sc_core::sc_event *event = func._event;
    sc_core::sc_spawn_options opt;
    opt.set_stack_size(0x40000);
    opt.dont_initialize();
    opt.set_sensitivity(event);
    sc_core::sc_spawn(func, nullptr, &opt);
    event->notify(delay);
}

} // namespace hif_systemc_extensions
