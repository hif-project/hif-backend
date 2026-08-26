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
    auto visitExpression(hif::Expression &o) -> int override;
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

    /// @brief Lowers a reduction operator into an explicit chain over the
    /// operand's bits, since VHDL before 2008 has no reduction operator.
    /// @param o The expression to inspect.
    /// @return True if it was a reduction and was rewritten.
    auto _fixReduceOperator(hif::Expression *o) -> bool;

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

auto PreRefine_utilityLibraries::visitExpression(hif::Expression &o) -> int
{
    GuideVisitor::visitExpression(o);
    _fixReduceOperator(&o);
    return 0;
}

auto PreRefine_utilityLibraries::_fixReduceOperator(hif::Expression *o) -> bool
{
    const hif::Operator op = o->getOperator();
    if (!hif::operatorIsReduce(op)) {
        return false;
    }

    // VHDL before 2008 has no reduction operator, and VHDLPrinter refuses
    // op_andrd/op_orrd/op_xorrd with "This operator should be managed in
    // refinement steps" - this is that step (hif-backend#92).
    //
    // The reduction becomes an explicit chain over the operand's bits:
    //
    //     y <= | a;   ->   y <= a(0) or a(1) or a(2) or a(3);
    //
    // Two alternatives were tried first and rejected on evidence:
    //
    //  - ieee.std_logic_misc's AND_REDUCE/OR_REDUCE/XOR_REDUCE, which is what
    //    hif2sc lowers onto and what hif-core already models. Every one of
    //    those declarations is built with the `unsupported` flag set, so the
    //    call trips checkHif's "Declaration not supported" and hif2vhdl's own
    //    checkStep rejects the tree it just produced. Clearing that flag is a
    //    maintainer decision about what the flag means, not a printing fix.
    //  - The VHDL-2008 unary form (`or a`), which would raise the language
    //    level of every output for one operator and is the spelling vhdl2hif
    //    cannot read back, breaking the round trip this backend exists to
    //    close.
    //
    // The chain needs neither a library nor a language-level bump, and it is
    // what a human writes for a fixed-width vector in VHDL-93.
    Value *operand = o->getValue1();
    messageAssert(operand != nullptr, "Expected an operand on a reduction", o, _sem);

    Type *operandType = hif::semantics::getSemanticType(operand, _sem);
    messageAssert(operandType != nullptr, "Cannot type the reduction's operand", operand, _sem);

    // A reduction of a single bit is that bit. Getting here with a scalar is
    // unusual but well defined, and the chain below would build nothing.
    Range *span = hif::typeGetSpan(operandType, _sem);
    if (span == nullptr) {
        o->replace(o->setValue1(nullptr));
        delete o;
        return true;
    }

    auto *leftBound  = dynamic_cast<IntValue *>(hif::getChildSkippingCasts(span->getLeftBound()));
    auto *rightBound = dynamic_cast<IntValue *>(hif::getChildSkippingCasts(span->getRightBound()));
    messageAssert(
        leftBound != nullptr && rightBound != nullptr,
        "A reduction operator over a vector whose bounds are not statically known cannot be lowered into a "
        "chain of bit operations, and VHDL before 2008 has no reduction operator to fall back on "
        "(hif-backend#92).",
        o, _sem);

    hif::Operator bitOperator = hif::op_bor;
    if (op == hif::op_andrd) {
        bitOperator = hif::op_band;
    } else if (op == hif::op_xorrd) {
        bitOperator = hif::op_bxor;
    }

    const std::int64_t low  = std::min(leftBound->getValue(), rightBound->getValue());
    const std::int64_t high = std::max(leftBound->getValue(), rightBound->getValue());

    // Built low index first, so the emitted chain reads in the same order for a
    // `downto` and a `to` vector and does not depend on the span's direction.
    Value *chain = nullptr;
    for (std::int64_t index = low; index <= high; ++index) {
        Value *bit = _factory.member(hif::copy(operand), _factory.intval(index));
        chain      = (chain == nullptr) ? bit : _factory.expression(chain, bitOperator, bit);
    }

    o->replace(chain);
    delete o;

    return true;
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
