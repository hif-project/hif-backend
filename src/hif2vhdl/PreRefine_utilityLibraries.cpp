/// @file PreRefine_utilityLibraries.cpp
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
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

using namespace hif;
using namespace semantics;

namespace
{

// /////////////////////////////////////////////////////////////////////////////
// Refine introducing utility libraries
// /////////////////////////////////////////////////////////////////////////////

class PreRefine_utilityLibraries : public hif::GuideVisitor
{
public:
    using LibraryDefSet = std::set<LibraryDef *>;

    /// @brief Default constructor and destructor.
    PreRefine_utilityLibraries(System *root, hif::semantics::ILanguageSemantics *sem);
    ~PreRefine_utilityLibraries() override;

    auto visitContents(hif::Contents &o) -> int override;
    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitSigned(hif::Signed &o) -> int override;
    auto visitSystem(hif::System &o) -> int override;
    auto visitUnsigned(hif::Unsigned &o) -> int override;
    auto visitView(hif::View &o) -> int override;

    /// @brief Tells whether support libraries have been introduced.
    auto hasIntroducedLibraries() const -> bool;

    void addLibraryDefs();

private:
    // Disabled.
    PreRefine_utilityLibraries(const PreRefine_utilityLibraries &)                     = delete;
    auto operator=(const PreRefine_utilityLibraries &) -> PreRefine_utilityLibraries & = delete;

    /// @name Support creation common methods.
    //@{

    /// Adds the "hif_" + s library.
    void _addHifLibrary(const char *s);

    //@}

    /// Pointer to root, needed to add support standard libraries.
    System *_root;

    /// @brief The current scope (View) where library references will be added.
    Object *_scope{nullptr};

    /// @brief Tells whether at least a library reference has been added.
    bool _introducedLibraries{false};

    LibraryDefSet _libraryDefSet;

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _factory;
};

PreRefine_utilityLibraries::PreRefine_utilityLibraries(System *root, hif::semantics::ILanguageSemantics *sem)
    : _root(root)
    , _libraryDefSet()
    , _sem(sem)
    , _factory(sem)
{
    // ntd
}

PreRefine_utilityLibraries::~PreRefine_utilityLibraries()
{
    // ntd
}

auto PreRefine_utilityLibraries::hasIntroducedLibraries() const -> bool { return _introducedLibraries; }

void PreRefine_utilityLibraries::addLibraryDefs()
{
    for (auto *i : _libraryDefSet) {
        hif::manipulation::addUniqueObject(i, _root->libraryDefs);
    }
}

auto PreRefine_utilityLibraries::visitContents(hif::Contents &o) -> int
{
    //Object * restore = _scope;
    //_scope = &o;
    GuideVisitor::visitContents(o);
    //_scope = restore;
    return 0;
}

auto PreRefine_utilityLibraries::visitLibraryDef(hif::LibraryDef &o) -> int
{
    Object *restore = _scope;
    _scope          = &o;
    GuideVisitor::visitLibraryDef(o);
    _scope = restore;
    return 0;
}

auto PreRefine_utilityLibraries::visitSigned(Signed &o) -> int
{
    GuideVisitor::visitSigned(o);
    _libraryDefSet.insert(_sem->getStandardLibrary("hif_vhdl_ieee_numeric_std"));
    return 0;
}

auto PreRefine_utilityLibraries::visitSystem(hif::System &o) -> int
{
    Object *restore = _scope;
    _scope          = &o;
    GuideVisitor::visitSystem(o);
    _scope = restore;
    return 0;
}

auto PreRefine_utilityLibraries::visitUnsigned(Unsigned &o) -> int
{
    GuideVisitor::visitUnsigned(o);
    _libraryDefSet.insert(_sem->getStandardLibrary("hif_vhdl_ieee_numeric_std"));
    return 0;
}

auto PreRefine_utilityLibraries::visitView(hif::View &o) -> int
{
    Object *restore = _scope;
    _scope          = &o;
    GuideVisitor::visitView(o);
    _scope = restore;
    return 0;
}

void PreRefine_utilityLibraries::_addHifLibrary(const char *c)
{
    std::string nn("hif_");
    nn += c;

    _introducedLibraries |= hif::backends::addHifLibrary(nn, _scope, _root, _sem);
}

} // namespace

auto fixUtilityLibraries(hif::System *o, hif::semantics::ILanguageSemantics *sem) -> bool
{
    hif::application_utils::initializeLogHeader("HIF2VHDL", "fixUtilityLibraries");

    PreRefine_utilityLibraries vis(o, sem);
    o->acceptVisitor(vis);

    vis.addLibraryDefs();

    hif::application_utils::restoreLogHeader();

    return vis.hasIntroducedLibraries();
}
