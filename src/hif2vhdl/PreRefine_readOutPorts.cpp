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
    typedef hif::semantics::ReferencesSet ObjectList;

    /// @brief Default constructor and destructor.
    PreRefine_readOutPorts(hif::semantics::ILanguageSemantics *sem);
    virtual ~PreRefine_readOutPorts();

    /// @brief Starter method.
    void fixReadOutPorts();

    virtual int visitLibraryDef(hif::LibraryDef &o);
    virtual int visitView(hif::View &o);
    virtual int visitPort(hif::Port &o);

private:
    /// @brief Disabled copy constructor.
    PreRefine_readOutPorts(const PreRefine_readOutPorts &);

    /// @brief Disabled assignment operator.
    PreRefine_readOutPorts &operator=(const PreRefine_readOutPorts &);

    /// @brief HifFactory object useful for some static methods.
    hif::HifFactory _factory;

    /// @brief The reference semantics.
    semantics::ILanguageSemantics *_sem;

    /// @brief Return true if the Port needs the read out port fix.
    bool _needFix(ObjectList &list);

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

int PreRefine_readOutPorts::visitLibraryDef(LibraryDef &o)
{
    if (o.isStandard())
        return 0;

    return GuideVisitor::visitLibraryDef(o);
}

int PreRefine_readOutPorts::visitView(View &o)
{
    if (o.isStandard())
        return 0;

    return GuideVisitor::visitView(o);
}

int PreRefine_readOutPorts::visitPort(Port &o)
{
    if (o.getDirection() != dir_out)
        return 0;

    View *view = getNearestParent<View>(&o);

    ObjectList list;
    semantics::getReferences(&o, list, _sem, view);

    if (!_needFix(list))
        return 0;

    _doFix(&o, view, list);

    return 0;
}

bool PreRefine_readOutPorts::_needFix(ObjectList &list)
{
    for (ObjectList::iterator it = list.begin(); it != list.end(); ++it) {
        if (hif::manipulation::isInLeftHandSide(*it))
            continue;

        PortAssign *pa = getNearestParent<PortAssign>(*it);
        if (pa == nullptr)
            return true;

        Value *val = getTerminalPrefix(pa->getValue());
        if (val == *it)
            continue;

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
    for (ObjectList::iterator it = list.begin(); it != list.end(); ++it) {
        objectSetName(*it, n);
        hif::semantics::resetDeclarations(*it, ropt);
    }

    Assign *ass = _factory.assignment(new Identifier(p->getName()), new Identifier(n));
    if (co->getGlobalAction() == nullptr)
        co->setGlobalAction(new GlobalAction());
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
