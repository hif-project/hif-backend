/// @file VhdlPostRefine_final.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <algorithm>

#include "hif2vhdl/PostRefineMethods.hpp"
#include "hif2vhdl/PrintMethods.hpp"

using namespace hif;

namespace
{

// /////////////////////////////////////////////////////////////////////////////
// FinalRefineVisitor
// /////////////////////////////////////////////////////////////////////////////

/// @brief Visit the tree and generates scope-defined libraries, which are not
/// true libraries but the includes (DesignUnit, LibraryDef, etc) needed by printer.
/// Note: for LibraryDef, some includes are needed before defining new structures,
/// some other are the include of LibraryDef component. These last ones are marked
/// with a property.
/// It also fix the scope of objects (generally stored as 'instance' member of objects).
class FinalRefineVisitor : public GuideVisitor
{
public:
    using ScopeSet          = std::set<Scope *>;
    using LibraryMap        = std::map<std::string, ScopeSet>;
    using FieldReferenceSet = std::set<FieldReference *>;

    FinalRefineVisitor(semantics::ILanguageSemantics *sem);
    ~FinalRefineVisitor() override;

    /// @name Scope-related visits.
    /// @{
    auto visitContents(Contents &o) -> int override;
    auto visitExpression(Expression &o) -> int override;
    auto visitLibraryDef(LibraryDef &o) -> int override;
    auto visitSystem(System &o) -> int override;
    auto visitView(View &o) -> int override;
    /// @}

    /// @name Symbol-related visits.
    /// @{
    auto visitBit(Bit &o) -> int override;
    auto visitBitvector(Bitvector &o) -> int override;
    auto visitLibrary(Library &o) -> int override;
    auto visitSigned(Signed &o) -> int override;
    auto visitUnsigned(Unsigned &o) -> int override;
    /// @}

    /// @name Special cases.
    /// @{

    /// @}

    /// @brief Generate all the needed includes.
    void doFixes();

private:
    // Disabled.
    FinalRefineVisitor(const FinalRefineVisitor &)                     = delete;
    auto operator=(const FinalRefineVisitor &) -> FinalRefineVisitor & = delete;

    semantics::ILanguageSemantics *_sem;
    HifFactory _factory;

    /// @brief Keep trace of current scope.
    Scope *_currentScope{nullptr};

    LibraryMap _libraryMap;

    FieldReferenceSet _frSet;

    hif::Trash _trash;
};

FinalRefineVisitor::FinalRefineVisitor(semantics::ILanguageSemantics *sem)
    : _sem(sem)
    , _factory(sem)
    , _libraryMap()
    , _frSet()
    , _trash()
{
    // ntd
}

FinalRefineVisitor::~FinalRefineVisitor()
{
    // ntd
}

auto FinalRefineVisitor::visitContents(Contents &o) -> int
{
    //    Scope * restore = _currentScope;
    //    _currentScope = &o;
    GuideVisitor::visitContents(o);
    //    _currentScope = restore;

    return 0;
}

auto FinalRefineVisitor::visitExpression(Expression &o) -> int
{
    GuideVisitor::visitExpression(o);

    if (o.getOperator() != op_pow) {
        return 0;
    }
    Type *t = hif::semantics::getSemanticType(&o, _sem);
    messageAssert(t != nullptr, "Cannot type expression", &o, _sem);

    Real *r = dynamic_cast<Real *>(t);
    if (r == nullptr) {
        return 0;
    }

    _libraryMap["ieee_math_real"].insert(_currentScope);

    return 0;
}

auto FinalRefineVisitor::visitLibraryDef(LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    Scope *restore = _currentScope;
    _currentScope  = &o;
    GuideVisitor::visitLibraryDef(o);
    _currentScope = restore;

    return 0;
}

auto FinalRefineVisitor::visitSystem(System &o) -> int
{
    _currentScope = &o;
    GuideVisitor::visitSystem(o);

    return 0;
}

auto FinalRefineVisitor::visitView(View &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    Scope *restore = _currentScope;
    _currentScope  = &o;
    GuideVisitor::visitView(o);
    _currentScope = restore;

    return 0;
}

auto FinalRefineVisitor::visitBit(Bit &o) -> int
{
    GuideVisitor::visitBit(o);

    if (!o.isLogic()) {
        return 0;
    }
    _libraryMap["ieee_std_logic_1164"].insert(_currentScope);

    return 0;
}

auto FinalRefineVisitor::visitBitvector(Bitvector &o) -> int
{
    GuideVisitor::visitBitvector(o);

    if (!o.isLogic()) {
        return 0;
    }
    _libraryMap["ieee_std_logic_1164"].insert(_currentScope);

    return 0;
}

auto FinalRefineVisitor::visitLibrary(Library &o) -> int
{
    GuideVisitor::visitLibrary(o);

    Library *lib = &o;
    if (lib->isStandard() || !lib->isSystem()) {
        return 0;
    }

    messageAssert(lib->getInstance() == nullptr, "Unexpected standard library (1)", &o, _sem);

    std::string libName = lib->getName();
    std::string libraryInclude;
    std::string package;
    if (libName.substr(0, 4) == "ieee") {
        libraryInclude = "ieee";
        package        = libName.substr(5); // skipping "ieee_"
    } else if (libName == "std_textio") {
        libraryInclude = "std";
        package        = "textio";
    } else if (libName == "standard") {
        libraryInclude = std::string();
        package        = std::string();
    } else {
        messageError("Unexpected standard library (2)", &o, _sem);
    }

    messageAssert(
        (libraryInclude.empty() && package.empty()) || (!libraryInclude.empty() && !package.empty()),
        "Unexpected standard library (3)", &o, _sem);

    if (libraryInclude.empty() && package.empty()) {
        // standard
        auto *inst         = dynamic_cast<Instance *>(o.getParent());
        FieldReference *fr = nullptr;
        if (inst != nullptr) {
            fr = dynamic_cast<FieldReference *>(inst->getParent());
        }
        _trash.insert(&o);
        if (fr != nullptr) {
            _frSet.insert(fr);
        }
        return 0;
    }

    Library *l = _factory.library(libraryInclude, nullptr, "", false, true);
    lib->setName(package);
    lib->setInstance(l);

    return 0;
}

auto FinalRefineVisitor::visitSigned(Signed &o) -> int
{
    GuideVisitor::visitSigned(o);

    _libraryMap["ieee_numeric_std"].insert(_currentScope);

    return 0;
}

auto FinalRefineVisitor::visitUnsigned(Unsigned &o) -> int
{
    GuideVisitor::visitUnsigned(o);

    _libraryMap["ieee_numeric_std"].insert(_currentScope);

    return 0;
}

void FinalRefineVisitor::doFixes()
{
    for (auto &i : _libraryMap) {
        const std::string &libName = i.first;
        for (auto s : i.second) {
            Library *lib = _factory.library(libName, nullptr, "", false, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            bool added                    = hif::manipulation::addUniqueObject(lib, s, addOpt);
            if (added) {
                lib->acceptVisitor(*this);
                lib->replace(nullptr);

                hif::manipulation::AddUniqueObjectOptions addOpt2;
                addOpt2.equalsOptions.checkOnlyNames = true;
                addOpt2.deleteIfNotAdded             = true;
                hif::manipulation::addUniqueObject(lib, s, addOpt2);
            }
        }
    }

    for (auto *fr : _frSet) {
        auto *id = new Identifier(fr->getName());
        fr->replace(id);
        delete fr;
    }

    _trash.clear();
}

} // namespace

void postRefinementsFinalStep(System *o, semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2VHDL", "postRefinementsFinalStep");

    FinalRefineVisitor irVis(sem);
    o->acceptVisitor(irVis);

    irVis.doFixes();

    hif::application_utils::restoreLogHeader();
}
