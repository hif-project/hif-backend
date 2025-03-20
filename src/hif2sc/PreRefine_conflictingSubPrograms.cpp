/// @file PreRefine_conflictingSubPrograms.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <iostream>

#include <hif/hif.hpp>

#include "hif2sc/PreRefineMethods.hpp"
#include "hif2sc/globals.hpp"

using namespace hif;

namespace
{

typedef std::map<Declaration *, std::set<Object *>> RefMap;
typedef std::set<SubProgram *> Signatures;

enum CompareType { COMPATIBLE, CONFLICTING, UNCOMPATIBLE };

class FixSubProgramVisitor : public hif::GuideVisitor
{

public:
    /// @brief Default constructor.
    FixSubProgramVisitor(RefMap &map, const bool keepBit, hif::semantics::ILanguageSemantics *sem);

    /// @brief Destructor.
    virtual ~FixSubProgramVisitor();

    virtual int visitFunction(hif::Function &o);
    virtual int visitProcedure(hif::Procedure &o);
    virtual int visitLibraryDef(hif::LibraryDef &o);

    bool _found;

private:
    FixSubProgramVisitor(const FixSubProgramVisitor &);
    FixSubProgramVisitor &operator=(const FixSubProgramVisitor &);

    template <typename T> void _checkCall(T *call, SubProgram *startingObj);

    bool _areSigsConflict(SubProgram *s1, SubProgram *s2);

    void _updateSignatures();

    CompareType _areConflictingTypes(Type *t1, Type *t2);

    ProcedureCall *_createFakeCall(SubProgram *o);

    RefMap _refMap;
    hif::semantics::ILanguageSemantics *_sem;
    Signatures _sigToChange;

    const bool _keepBit;
};

FixSubProgramVisitor::FixSubProgramVisitor(RefMap &map, const bool keepBit, hif::semantics::ILanguageSemantics *sem)
    : GuideVisitor()
    , _found(false)
    , _refMap(map)
    , _sem(sem)
    , _sigToChange()
    , _keepBit(keepBit)
{
    // ntd
}

FixSubProgramVisitor::~FixSubProgramVisitor()
{
    // ntd
}

int FixSubProgramVisitor::visitFunction(hif::Function &o)
{
    GuideVisitor::visitFunction(o);

    ProcedureCall *call = _createFakeCall(&o);
    _checkCall(call, &o);

    return 0;
}

int FixSubProgramVisitor::visitProcedure(hif::Procedure &o)
{
    GuideVisitor::visitProcedure(o);

    ProcedureCall *call = _createFakeCall(&o);
    _checkCall(call, &o);

    return 0;
}

int FixSubProgramVisitor::visitLibraryDef(LibraryDef &o)
{
    if (o.isStandard())
        return 0;

    GuideVisitor::visitLibraryDef(o);

    return 0;
}

template <typename T> void FixSubProgramVisitor::_checkCall(T *call, SubProgram *startingObj)
{
    // get all candidates
    std::list<typename T::DeclarationType *> candidates;
    hif::semantics::GetCandidatesOptions opt;
    opt.forceRefresh = true;
    opt.location     = startingObj;
    hif::semantics::getCandidates(candidates, call, _sem, opt);
    delete call;

    // check only when candidates are at least two
    if (candidates.size() < 2)
        return;

    // If a couple of candidates are identical up to types of parameter
    // bit vector / signed / unsigned, change function name
    for (typename std::list<typename T::DeclarationType *>::iterator i = candidates.begin(); i != candidates.end();
         ++i) {
        // TODO: add check candidate already fixed
        typename std::list<typename T::DeclarationType *>::iterator j = i;
        ++j;
        for (; j != candidates.end(); ++j) {
            typename T::DeclarationType *s1 = *i;
            typename T::DeclarationType *s2 = *j;

            if (!_areSigsConflict(s1, s2))
                continue;

            _sigToChange.insert(s1);
            _sigToChange.insert(s2);
        }
    }

    // Update signatures
    _updateSignatures();
}

CompareType FixSubProgramVisitor::_areConflictingTypes(Type *t1, Type *t2)
{
    // Conflicting types in systemc are:
    // - bitvector (that is array packed of bit logic/not logic)
    // - signed
    // - unsigned

    // If IsA are the same there are certainly no conflicts and types are
    // compatible/uncompatible basing on result of hif::equals.
    hif::EqualsOptions opt1;
    opt1.checkOnlyTypes = true;
    if (hif::equals(t1, t2, opt1)) {
        hif::EqualsOptions opt;
        opt.checkSpans = false; // TODO: may be wrong?

        if (hif::equals(t1, t2, opt)) {
            return COMPATIBLE;
        } else {
            // handle case bitvector->signed vs bitvector->unsiged that conflicts!
            // Using IsA to increase performance
            if (dynamic_cast<Bitvector *>(t1) != nullptr && dynamic_cast<Bitvector *>(t2) != nullptr) {
                Bitvector *arr1 = static_cast<Bitvector *>(t1);
                Bitvector *arr2 = static_cast<Bitvector *>(t2);
                if (arr1->isSigned() != arr2->isSigned()) {
                    // Searched case!
                    return CONFLICTING;
                } else if (arr1->isResolved() != arr2->isResolved()) {
                    // Searched case!
                    return CONFLICTING;
                }
            }
            return UNCOMPATIBLE;
        }
        // Unreachable:
        //return hif::equals( t1, t2, opt ) ? COMPATIBLE : UNCOMPATIBLE;
    }

    // Otherwise we must check conflicting types:
    Bit *b1       = dynamic_cast<Bit *>(t1);
    Bool *bool1   = dynamic_cast<Bool *>(t1);
    Bitvector *a1 = dynamic_cast<Bitvector *>(t1);
    Signed *s1    = dynamic_cast<Signed *>(t1);
    Unsigned *u1  = dynamic_cast<Unsigned *>(t1);

    if (b1 == nullptr && bool1 == nullptr && a1 == nullptr && s1 == nullptr && u1 == nullptr) {
        // Different types but not of conflicting types on t1 (e.g. int, signed)
        // result is certainly uncompatible.
        return UNCOMPATIBLE;
    }

    Bit *b2       = dynamic_cast<Bit *>(t2);
    Bool *bool2   = dynamic_cast<Bool *>(t2);
    Bitvector *a2 = dynamic_cast<Bitvector *>(t2);
    Signed *s2    = dynamic_cast<Signed *>(t2);
    Unsigned *u2  = dynamic_cast<Unsigned *>(t2);

    if (b2 == nullptr && bool2 == nullptr && a2 == nullptr && s2 == nullptr && u2 == nullptr) {
        // Different types but not of conflicting types on t2 (e.g. int, signed)
        // result is certainly uncompatible.
        return UNCOMPATIBLE;
    }

    if (b1 != nullptr && !_keepBit) {
        if (bool2 != nullptr)
            return CONFLICTING;
    } else if (bool1 != nullptr && !_keepBit) {
        if (b2 != nullptr)
            return CONFLICTING;
    } else if (a1 != nullptr) {
        if (s2 != nullptr)
            return CONFLICTING;
        if (u2 != nullptr)
            return CONFLICTING;
    } else if (s1 != nullptr) {
        if (a2 != nullptr)
            return CONFLICTING;
        if (u2 != nullptr)
            return CONFLICTING;
    } else // u1 != nullptr
    {
        if (a2 != nullptr)
            return CONFLICTING;
        if (s2 != nullptr)
            return CONFLICTING;
    }

    return UNCOMPATIBLE;
}

bool FixSubProgramVisitor::_areSigsConflict(SubProgram *s1, SubProgram *s2)
{
    if (s1->parameters.size() != s2->parameters.size())
        return false;

    bool conflicts      = false;
    bool all_compatible = true;

    // check parameter
    BList<Parameter>::iterator l = s1->parameters.begin();
    BList<Parameter>::iterator m = s2->parameters.begin();
    for (; l != s1->parameters.end(); ++l, ++m) {
        Type *t1 = hif::semantics::getBaseType((*l)->getType(), false, _sem);
        Type *t2 = hif::semantics::getBaseType((*m)->getType(), false, _sem);

        CompareType t = _areConflictingTypes(t1, t2);
        if (t == COMPATIBLE) {
            // e.g. signed - signed
            // may conflicts on other params
            continue;
        } else if (t == UNCOMPATIBLE) {
            // e.g. signed - bool
            //all_compatible = false;
            //conflicts = false;
            return false;
        } else // t == CONFLICTING
        {
            // e.g. signed - unsigned
            all_compatible = false;
            conflicts      = true;
        }
    }

    if (conflicts || all_compatible) {
        // conflict example:
        // foo( int, signed, bool )
        // foo( int, unsigned, bool )
        //
        // all_compatible example:
        // foo( int, signed )
        // foo( int, signed )

        _found = true;
        return true;
    }

    return false;
}

void FixSubProgramVisitor::_updateSignatures()
{
    messageDebugAssert(_sigToChange.empty() || _sigToChange.size() >= 2, "Unexpected list size", nullptr, _sem);
    for (Signatures::iterator i = _sigToChange.begin(); i != _sigToChange.end(); ++i) {
        // skip first!
        if (i == _sigToChange.begin())
            continue;

        // Create new fresh name
        auto n = hif::NameTable::getInstance()->getFreshName((*i)->getName() + "_renamed");

        // Rise warning
        messageWarning(
            std::string("Subprogram ") + (*i)->getName() + " and its references are renamed in " + n +
                " because has conflicting SystemC types.",
            nullptr, nullptr);

        // Setting new name to function
        (*i)->setName(n);

        // Update references
        for (std::set<Object *>::iterator j = _refMap[*i].begin(); j != _refMap[*i].end(); ++j) {
            if (dynamic_cast<FunctionCall *>(*j) != nullptr) {
                FunctionCall *call = static_cast<FunctionCall *>(*j);
                call->setName(n);
            } else if (dynamic_cast<ProcedureCall *>(*j) != nullptr) {
                ProcedureCall *call = static_cast<ProcedureCall *>(*j);
                call->setName(n);
            } else {
                messageDebugAssert(false, "Unexpected case", *j, _sem);
            }
        }
    }

    _sigToChange.clear();
}

ProcedureCall *FixSubProgramVisitor::_createFakeCall(SubProgram *o)
{
    ProcedureCall *pcall = new ProcedureCall();
    pcall->setName(o->getName());

    return pcall;
}

} // namespace

bool fixConflictingSubPrograms(hif::System *o, const bool keepBit, hif::semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2SC", "fixConflictingSubPrograms");

    RefMap refMap;
    hif::semantics::getAllReferences(refMap, sem, o);

    FixSubProgramVisitor fspv(refMap, keepBit, sem);
    o->acceptVisitor(fspv);

    hif::application_utils::restoreLogHeader();

    return fspv._found;
}
