/// @file hif_after.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <systemc>

#include "config.hpp"

namespace hif_systemc_extensions
{

class HIF2SCSUPPORT_EXPORT IHifAfter
{
public:
    virtual ~IHifAfter();

    auto operator=(const IHifAfter &) -> IHifAfter & = delete;

    void disable();

    sc_core::sc_event *_event;
    std::string _file;
    int _line;
    bool _isActive;
    sc_core::sc_time _time;

protected:
    IHifAfter(const char *file, int line);
    IHifAfter(const IHifAfter &other);
    void swap(IHifAfter &other) noexcept;
};

template <class Target, class Value> class HifAfter : public IHifAfter
{
public:
    HifAfter(Target &t, Value v, const char *file, int line);
    ~HifAfter() override;
    HifAfter(const HifAfter<Target, Value> &other);
    auto operator=(const HifAfter<Target, Value> &other) -> HifAfter<Target, Value> &;
    void swap(HifAfter<Target, Value> &other) noexcept;

    void operator()();

private:
    Target *_target;
    Value _value;
};

template <typename Target, typename Value>
void _hif_after(Target &target, Value v, const sc_core::sc_time &delay, const char *file, int line);

#define hif_after(target, v, delay) _hif_after(target, v, delay, __FILE__, __LINE__)

} // namespace hif_systemc_extensions

#include "hif_after.i.hpp"
