/// @file PreRefine_readOutPorts.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/hif.hpp>

#include "hif2vhdl/PreRefineMethods.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#endif

using namespace hif;

namespace
{ // anon.namespace

class PreRefine_readOutPorts : public hif::GuideVisitor
{
public:
    using ObjectList = hif::semantics::ReferencesSet;

    /// @brief Default constructor and destructor.
    PreRefine_readOutPorts(hif::semantics::ILanguageSemantics *sem);
    ~PreRefine_readOutPorts() override;

    /// @brief Starter method.
    void fixReadOutPorts();

    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitView(hif::View &o) -> int override;
    auto visitPort(hif::Port &o) -> int override;

private:
    /// @brief Disabled copy constructor.
    PreRefine_readOutPorts(const PreRefine_readOutPorts &);

    /// @brief Disabled assignment operator.
    auto operator=(const PreRefine_readOutPorts &) -> PreRefine_readOutPorts &;

    /// @brief HifFactory object useful for some static methods.
    hif::HifFactory _factory;

    /// @brief The reference semantics.
    semantics::ILanguageSemantics *_sem;

    /// @brief Return true if the Port needs the read out port fix.
    static auto _needFix(ObjectList &list) -> bool;

    /// @brief Perform the read out port fix.
    void _doFix(hif::Port *p, hif::View *view, ObjectList &list);
};

PreRefine_readOutPorts::PreRefine_readOutPorts(hif::semantics::ILanguageSemantics *sem)
    : _factory(sem)
    , _sem(sem)
{
    // ntd
}

PreRefine_readOutPorts::~PreRefine_readOutPorts()
{
    // ntd
}

auto PreRefine_readOutPorts::visitLibraryDef(LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    return GuideVisitor::visitLibraryDef(o);
}

auto PreRefine_readOutPorts::visitView(View &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    return GuideVisitor::visitView(o);
}

auto PreRefine_readOutPorts::visitPort(Port &o) -> int
{
    if (o.getDirection() != dir_out) {
        return 0;
    }

    View *view = getNearestParent<View>(&o);

    ObjectList list;
    semantics::getReferences(&o, list, _sem, view);

    if (!_needFix(list)) {
        return 0;
    }

    _doFix(&o, view, list);

    return 0;
}

auto PreRefine_readOutPorts::_needFix(ObjectList &list) -> bool
{
    for (auto *it : list) {
        if (hif::manipulation::isInLeftHandSide(it)) {
            continue;
        }

        auto *pa = getNearestParent<PortAssign>(it);
        if (pa == nullptr) {
            return true;
        }

        Value *val = getTerminalPrefix(pa->getValue());
        if (val == it) {
            continue;
        }

        return true;
    }
    return false;
}

void PreRefine_readOutPorts::_doFix(Port *p, View *view, ObjectList &list)
{
    auto n       = hif::NameTable::getInstance()->getFreshName((std::string("sig_") + p->getName()));
    Signal *sig  = _factory.signal(hif::copy(p->getType()), n, hif::copy(p->getValue()));
    Contents *co = view->getContents();
    co->declarations.push_back(sig);

    hif::semantics::ResetDeclarationsOptions ropt;
    ropt.onlyCurrent = true;
    for (auto *it : list) {
        objectSetName(it, n);
        hif::semantics::resetDeclarations(it, ropt);
    }

    Assign *ass = _factory.assignment(new Identifier(p->getName()), new Identifier(n));
    if (co->getGlobalAction() == nullptr) {
        co->setGlobalAction(new GlobalAction());
    }
    co->getGlobalAction()->actions.push_back(ass);
}

} // namespace

void fixReadOutPorts(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2VHDL", "fixReadOutPorts");

    PreRefine_readOutPorts readOutPorts(sem);
    o->acceptVisitor(readOutPorts);

    hif::application_utils::restoreLogHeader();
}
