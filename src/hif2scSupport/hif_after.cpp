/// @file hif_after.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <map>

#include "hif2scSupport/hif2scSupport/hif_after.hpp"

namespace hif_systemc_extensions
{

namespace /* anon */
{

typedef std::pair<int, std::string> AfterKey1;
typedef std::pair<AfterKey1, sc_core::sc_time> AfterKey2;
typedef std::map<AfterKey2, IHifAfter *> AfterMap;

AfterMap _afterMap;

void _hif_after_register(IHifAfter *after)
{
    AfterKey1 k1(after->_line, after->_file);
    AfterKey2 k2(k1, after->_time);
    AfterMap::iterator it(_afterMap.find(k2));
    if (it != _afterMap.end()) {
        IHifAfter *other = it->second;
        other->disable();
    }

    _afterMap[k2] = after;
}

void _hif_after_unregister(IHifAfter *after)
{
    AfterKey1 k1(after->_line, after->_file);
    AfterKey2 k2(k1, after->_time);
    AfterMap::iterator it(_afterMap.find(k2));
    if (it == _afterMap.end())
        return;
    if (it->second != after)
        return;
    _afterMap.erase(it);
}

} // namespace

hif_systemc_extensions::IHifAfter::~IHifAfter()
{
    _hif_after_unregister(this);
    delete _event;
}

void IHifAfter::disable() { _isActive = false; }

IHifAfter::IHifAfter(const char *file, const int line)
    : _event(new sc_core::sc_event())
    , _file(file)
    , _line(line)
    , _isActive(true)
    , _time(sc_core::sc_time_stamp())
{
    _hif_after_register(this);
}

hif_systemc_extensions::IHifAfter::IHifAfter(const hif_systemc_extensions::IHifAfter &other)
    : _event(other._event)
    , _file(other._file)
    , _line(other._line)
    , _isActive(other._isActive)
    , _time(other._time)
{
    // move semantics
    IHifAfter *tmp = const_cast<IHifAfter *>(&other);
    tmp->_event    = nullptr;
    _hif_after_register(this);
}

void hif_systemc_extensions::IHifAfter::swap(hif_systemc_extensions::IHifAfter &other)
{
    std::swap(_event, other._event);
    std::swap(_file, other._file);
    std::swap(_line, other._line);
    std::swap(_isActive, other._isActive);
    std::swap(_time, other._time);
}

} // namespace hif_systemc_extensions
