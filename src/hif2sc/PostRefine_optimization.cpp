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
    PimpCodeVisitor(semantics::ILanguageSemantics *sem, const bool useHdtlib, const bool useCpp98);
    virtual ~PimpCodeVisitor();

    virtual int visitExpression(Expression &o);

    bool hasBeenOptimized();

private:
    PimpCodeVisitor(const PimpCodeVisitor &);
    PimpCodeVisitor &operator=(const PimpCodeVisitor &);

    bool _pimpConcat(Expression *o);

    semantics::ILanguageSemantics *_sem;
    HifFactory _factory;

    /// @brief Set whether use hdtlib types.
    const bool _useHdtlib;

    /// @brief Set whether C++98 standard is required.
    const bool _useCpp98;

    /// @brief True is at least an optimization is performed.
    bool _optimized;
};

PimpCodeVisitor::PimpCodeVisitor(semantics::ILanguageSemantics *sem, const bool useHdtlib, const bool useCpp98)
    : hif::GuideVisitor()
    , _sem(sem)
    , _factory(sem)
    , _useHdtlib(useHdtlib)
    , _useCpp98(useCpp98)
    , _optimized(false)
{
    // ntd
}

PimpCodeVisitor::~PimpCodeVisitor()
{
    // ntd
}

int PimpCodeVisitor::visitExpression(Expression &o)
{
    GuideVisitor::visitExpression(o);

    if (_pimpConcat(&o)) {
        _optimized = true;
        return 0;
    }

    return 0;
}

bool PimpCodeVisitor::hasBeenOptimized() { return _optimized; }

bool PimpCodeVisitor::_pimpConcat(Expression *o)
{
    if (o->getOperator() != op_concat)
        return false;

    Type *exprType = hif::semantics::getBaseType(hif::semantics::getSemanticType(o, _sem), false, _sem);
    messageAssert(exprType != nullptr, "Cannot type expression", o, _sem);

    Expression *parentExpr = dynamic_cast<Expression *>(o->getParent());
    if (parentExpr != nullptr && parentExpr->getOperator() != op_concat)
        return false;

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
