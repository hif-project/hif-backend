/// @file PostRefine_optimization.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2sc/PostRefineMethods.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

using namespace hif;

namespace
{

class PimpCodeVisitor : public hif::GuideVisitor
{
public:
    PimpCodeVisitor(semantics::ILanguageSemantics *sem, bool useHdtlib, bool useCpp98);
    ~PimpCodeVisitor() override;

    auto visitExpression(Expression &o) -> int override;

    auto hasBeenOptimized() const -> bool;

private:
    PimpCodeVisitor(const PimpCodeVisitor &)                     = delete;
    auto operator=(const PimpCodeVisitor &) -> PimpCodeVisitor & = delete;

    auto _pimpConcat(Expression *o) -> bool;

    semantics::ILanguageSemantics *_sem;
    HifFactory _factory;

    /// @brief Set whether use hdtlib types.
    bool _useHdtlib;

    /// @brief Set whether C++98 standard is required.
    bool _useCpp98;

    /// @brief True is at least an optimization is performed.
    bool _optimized{false};
};

PimpCodeVisitor::PimpCodeVisitor(semantics::ILanguageSemantics *sem, bool useHdtlib, bool useCpp98)
    : hif::GuideVisitor()
    , _sem(sem)
    , _factory(sem)
    , _useHdtlib(useHdtlib)
    , _useCpp98(useCpp98)

{
    // ntd
}

PimpCodeVisitor::~PimpCodeVisitor()
{
    // ntd
}

auto PimpCodeVisitor::visitExpression(Expression &o) -> int
{
    GuideVisitor::visitExpression(o);

    if (_pimpConcat(&o)) {
        _optimized = true;
        return 0;
    }

    return 0;
}

auto PimpCodeVisitor::hasBeenOptimized() const -> bool { return _optimized; }

auto PimpCodeVisitor::_pimpConcat(Expression *o) -> bool
{
    if (o->getOperator() != op_concat) {
        return false;
    }

    Type *exprType = hif::semantics::getBaseType(hif::semantics::getSemanticType(o, _sem), false, _sem);
    messageAssert(exprType != nullptr, "Cannot type expression", o, _sem);

    auto *parentExpr = dynamic_cast<Expression *>(o->getParent());
    if (parentExpr != nullptr && parentExpr->getOperator() != op_concat) {
        return false;
    }

    Cast *c = new Cast();
    Type *t = hif::copy(exprType);
    // Reference design: des56 (vhdl/openCores) with optimization flag (-O)
    t->setTypeVariant(Type::NATIVE_TYPE);
    c->setType(t);
    o->replace(c);
    c->setValue(o);

    return true;
}

} // namespace

void postRefinementsOptimizationStep(System *o, hif2scParseLine &cLine, semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2SC", "postRefinementsOptimization");

    PimpCodeVisitor v(sem, cLine.useHDTLib(), cLine.useCpp98());
    o->acceptVisitor(v);

    hif::application_utils::restoreLogHeader();
}
