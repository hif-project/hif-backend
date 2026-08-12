/// @file PrintRefinementsVisitor.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2sc/PrintRefinementsVisitor.hpp"

using namespace hif;

namespace /* anon */
{

class PrintRefinementsVisitor : public hif::GuideVisitor
{

public:
    PrintRefinementsVisitor(hif::semantics::ILanguageSemantics *sem);
    ~PrintRefinementsVisitor() override;

    auto visitView(hif::View &o) -> int override;
    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitTypeReference(hif::TypeReference &o) -> int override;

private:
    hif::HifFactory _factory;
    hif::semantics::ILanguageSemantics *_sem;

    PrintRefinementsVisitor(const PrintRefinementsVisitor &)                     = delete;
    auto operator=(const PrintRefinementsVisitor &) -> PrintRefinementsVisitor & = delete;

    void _fixTypeRefPrinting(hif::TypeReference *tr);
};

PrintRefinementsVisitor::PrintRefinementsVisitor(hif::semantics::ILanguageSemantics *sem)
    : GuideVisitor()
    , _factory(sem)
    , _sem(sem)
{
    // ntd
}

PrintRefinementsVisitor::~PrintRefinementsVisitor()
{
    // ntd
}

auto PrintRefinementsVisitor::visitView(View &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }
    return GuideVisitor::visitView(o);
}

auto PrintRefinementsVisitor::visitLibraryDef(LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }
    return GuideVisitor::visitLibraryDef(o);
}

auto PrintRefinementsVisitor::visitTypeReference(TypeReference &o) -> int
{
    GuideVisitor::visitTypeReference(o);
    _fixTypeRefPrinting(&o);
    return 0;
}

void PrintRefinementsVisitor::_fixTypeRefPrinting(TypeReference *tr)
{
    if (dynamic_cast<Function *>(tr->getParent()) == nullptr) {
        return;
    }
    View *view = hif::getNearestParent<View>(tr);
    if (view == nullptr) {
        return;
    }
    TypeReference::DeclarationType *decl = hif::semantics::getDeclaration(tr, _sem);
    messageAssert(decl != nullptr, "Declaration not found", tr, _sem);
    auto *td = dynamic_cast<TypeDef *>(decl);
    if (td == nullptr) {
        return;
    }

    View *view2 = hif::getNearestParent<View>(td);
    if (view2 == nullptr) {
        return;
    }
    if (view != view2) {
        return;
    }

    hif::manipulation::PrefixTreeOptions opts;
    opts.skipPrefixingIfSameScope = false;
    opts.setContainingView        = true;
    Type *newType                 = hif::semantics::getPrefixedType(tr, _sem, opts);
    tr->replace(newType);
    delete tr;
}

} // namespace

void printRefinements(hif::System *sys, hif::semantics::ILanguageSemantics *sem)
{
    PrintRefinementsVisitor prv(sem);
    sys->acceptVisitor(prv);
}
