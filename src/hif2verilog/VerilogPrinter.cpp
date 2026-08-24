/// @file VerilogPrinter.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2verilog/VerilogPrinter.hpp"

#include <hif/semantics/declarationUtils.hpp>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <utility>

// Namespace hifsuite
using namespace hif;

namespace
{
/// @brief Marks the empty Wait verilog2hif appends to a process that suspends,
/// recording that the process loops back round rather than that it stops.
///
/// Declared by value rather than shared through a header: hif-frontend defines
/// the same string in verilog2hif/support.hpp, but that is a sibling tool's
/// private header and this project does not build against it. The literal is
/// the contract between them, and hif-backend#46 is where it is written down.
const char *const PROPERTY_PROCESS_LOOP_TAIL = "PROPERTY_PROCESS_LOOP_TAIL";

/// @brief Whether a target's declaration is storage whose new value must be
/// visible to the reads that follow it in the same activation, i.e. one that
/// has to be assigned with Verilog's blocking "=".
///
/// Two declarations qualify, for the same underlying reason - the value is
/// consumed before the current time step ends, so scheduling it for the end of
/// the step publishes the *previous* one:
///
///   - a Variable, which visitProcedureCall's inlined cones read back
///     immediately at the call site (hif-backend#16);
///   - a Parameter with direction out or inout, which is task-local storage
///     copied back to the actual when the task returns. The copy-back happens
///     on return, so a non-blocking assignment updates the local only after
///     the actual has already been written (hif-backend#70).
///
/// dir_in is deliberately excluded: assigning to an input argument is local
/// scratch use with no copy-back, so it carries no such ordering requirement.
auto isBlockingAssignmentTarget(hif::Declaration *dd) -> bool
{
    if (dynamic_cast<Variable *>(dd) != nullptr) {
        return true;
    }
    auto *param = dynamic_cast<Parameter *>(dd);
    return param != nullptr &&
           (param->getDirection() == dir_out || param->getDirection() == dir_inout);
}
} // namespace

VerilogPrinter::VerilogPrinter(hif::backends::IndentedStream *stream)
    : _sem(hif::semantics::VerilogSemantics::getInstance())
    , _stream(stream)
{
    // ntd
}

VerilogPrinter::~VerilogPrinter()
{
    // ntd
}

// Check
auto VerilogPrinter::visitAggregate(Aggregate &o) -> int
{
    GuideVisitor::visitAggregate(o);
    return 0;
}

// Check
auto VerilogPrinter::visitAggregateAlt(AggregateAlt &o) -> int
{
    GuideVisitor::visitAggregateAlt(o);
    return 0;
}

// Check
auto VerilogPrinter::visitAlias(Alias &o) -> int
{
    GuideVisitor::visitAlias(o);
    return 0;
}

// Check
auto VerilogPrinter::visitArray(Array &o) -> int
{
    (*_stream) << "array( ";
    // Print Span
    o.getSpan()->acceptVisitor(*this);
    (*_stream) << ") ";
    // Print Type
    (*_stream) << "of ";
    o.getType()->acceptVisitor(*this);
    return 0;
}

auto VerilogPrinter::visitAssign(Assign &o) -> int
{
    o.getLeftHandSide()->acceptVisitor(*this);

    // Resolve the declaration of the assignment *target*, which is not
    // necessarily the left-hand side itself: a bit-select is a Member
    // wrapping the identifier and a part-select is a Slice. Neither is a
    // symbol, and handing one to getDeclaration asserts inside hif-core
    // ("Passed non-symbol object"), terminating the process after the output
    // file has been created and leaving it zero bytes (hif-backend#23).
    //
    // Recurse through members, slices and field references to reach the
    // identifier they are built on. hif-frontend resolves the same shape the
    // same way in splitLogicConesLoops.
    hif::TerminalPrefixOptions prefixOptions;
    prefixOptions.recurseIntoMembers    = true;
    prefixOptions.recurseIntoSlices     = true;
    prefixOptions.recurseIntoFieldRefs  = true;

    Value *target = hif::getTerminalPrefix(o.getLeftHandSide(), prefixOptions);

    // The lookup only chooses between "=" and "<=" below, so a target that
    // still cannot be resolved must not be fatal: fall through with a null
    // declaration and emit the assignment. Losing the whole module is a far
    // worse outcome than defaulting the assignment operator.
    hif::Declaration *dd = nullptr;
    if (dynamic_cast<hif::Identifier *>(target) != nullptr) {
        dd = hif::semantics::getDeclaration(target, _sem);
    }

    // "<=" in Verilog is called non-blocking assignment which brings a whole lot of difference than "=" which is called
    // as blocking assignment because of scheduling events in any vendor based simulators.
    //
    // It is Recommended to use non-blocking assignment for sequential logic and blocking assignment for combinational
    // logic, only then it infers correct hardware logic during synthesis.
    //
    // Non-blocking statements in sequential block will infer flip flop in actual hardware.
    //
    // Always remember do not mix blocking and non-blocking in any sequential or combinational block.
    //
    // Inside an inlined cone the choice below is load-bearing rather than
    // stylistic. visitProcedureCall expands a cone's body at its call site,
    // and the statements that follow - the reads of the cone's target that
    // motivated the call - only observe the value just computed because the
    // target is one isBlockingAssignmentTarget accepts, so it is assigned
    // with blocking "=". A cone target that reached here as anything else
    // would be emitted with "<=", and those reads would silently go back to
    // seeing the previous value: exactly the staleness hif-backend#16 was
    // about, moved inside a single process where it is harder to spot.
    //
    // The frontend guarantees this today - refineToVariables shadows a
    // target that must stay a signal into a "_sig_var" Variable and drives
    // the signal from a separate process - so this is a check on that
    // contract, not a case seen in practice. messageAssert is deliberate:
    // it survives release builds, so a future change to that contract fails
    // loudly instead of quietly emitting wrong Verilog.
    // A *delayed* assignment is the one case where the reads that follow must
    // NOT see the new value: the whole point of the delay is that the target
    // changes later, and visitStateTable makes the process re-run then by
    // adding the target to its sensitivity list. So the reasoning above does
    // not apply to it, and neither does the assert.
    const std::string delay(this->renderDelay(o.getDelay()));

    if (delay.empty() && !_inliningCones.empty()) {
        messageAssert(
            isBlockingAssignmentTarget(dd),
            "Inlined cone assigns to a target that is emitted with non-blocking '<=', which "
            "would leave the reads after the call observing a stale value (hif-backend#16).",
            &o, _sem);
    }

    if (delay.empty() && isBlockingAssignmentTarget(dd)) {
        (*_stream) << " = ";
    } else {
        (*_stream) << " <= ";
    }

    // A delayed assignment carries its delay in the HIF; it used to be read by
    // nobody, so the regenerated design responded immediately where its source
    // waited - silently, since the output parses and reparses either way
    // (hif-backend#24).
    //
    // Emitted as an *intra-assignment* delay ("t <= #2 a & b") rather than as a
    // leading one ("#2 t = a & b"): the right-hand side is evaluated when the
    // process runs and the target updated after the delay, which is what a HIF
    // delay on an Assign means, and what the source's `assign #2` meant. A
    // leading delay would instead suspend the process, so every other path
    // through it - including a reader's own direct dependence on a primary
    // input - would be delayed too, which the source did not say.
    if (!delay.empty()) {
        (*_stream) << "#" << delay << " ";
    }

    o.getRightHandSide()->acceptVisitor(*this);
    (*_stream) << ";\n";
    return 0;
}

// Check std_logic and std_ulogic
auto VerilogPrinter::visitBit(Bit &o) -> int
{
    GuideVisitor::visitBit(o);
    return 0;
}

auto VerilogPrinter::visitBitValue(BitValue &o) -> int
{
    auto value = this->getValue(&o);
    if (!value.empty()) {
        (*_stream) << value;
    }
    return 0;
}

auto VerilogPrinter::visitBitvector(Bitvector &o) -> int
{
    // Get the bitvector span.
    auto span        = o.getSpan();
    // Get the left bound.
    auto left_bound  = dynamic_cast<hif::IntValue *>(span->getLeftBound());
    // Get the right bound.
    auto right_bound = dynamic_cast<hif::IntValue *>(span->getRightBound());
    // If both bounds are valid, print the bitvector.
    if (left_bound && right_bound) {
        (*_stream) << "[" << left_bound->getValue() << ":" << right_bound->getValue() << "]";
        return 0;
    } else {
        return GuideVisitor::visitBitvector(o);
    }
}

auto VerilogPrinter::visitBitvectorValue(BitvectorValue &o) -> int
{
    auto value = this->getValue(&o);
    if (!value.empty()) {
        (*_stream) << value;
    }
    return 0;
}

auto VerilogPrinter::visitBool(Bool &o) -> int { return GuideVisitor::visitBool(o); }

auto VerilogPrinter::visitBoolValue(BoolValue &o) -> int { return GuideVisitor::visitBoolValue(o); }

auto VerilogPrinter::visitBreak(Break &o) -> int { return GuideVisitor::visitBreak(o); }

namespace
{

/// @brief Whether a Verilog part-select may be applied directly to @p value.
/// @details Verilog-2001 allows a part-select on a net or variable
/// reference - an identifier, a bit-select of one, or a slice of one - but
/// not on an arbitrary expression.
auto isPartSelectable(hif::Value *value) -> bool
{
    return dynamic_cast<hif::Identifier *>(value) != nullptr || dynamic_cast<hif::Member *>(value) != nullptr ||
           dynamic_cast<hif::Slice *>(value) != nullptr || dynamic_cast<hif::FieldReference *>(value) != nullptr;
}

} // namespace

auto VerilogPrinter::isTruncatedByAssignmentContext(hif::Cast &o, unsigned long long targetWidth) -> bool
{
    // Only the whole right-hand side qualifies. A cast nested inside a
    // larger expression is not truncated by the assignment: the enclosing
    // operator sees the untruncated operand first.
    auto *assign = dynamic_cast<hif::Assign *>(o.getParent());
    if (assign == nullptr || assign->getRightHandSide() != &o) {
        return false;
    }
    Type *lhsType = hif::semantics::getSemanticType(assign->getLeftHandSide(), _sem);
    if (lhsType == nullptr) {
        return false;
    }
    return hif::semantics::typeGetSpanBitwidth(lhsType, _sem) == targetWidth;
}

auto VerilogPrinter::visitCast(Cast &o) -> int
{
    // A Cast to a Bitvector/Bit/Signed/Unsigned narrower than the source
    // value's own type is how verilog2hif represents a slice of the source's
    // low bits (e.g. `instruction[2:0]` on an 8-bit `instruction` becomes a
    // Cast to a 3-bit Bitvector, rather than a Slice, when the slice starts
    // at bit 0). A Cast to a wider such type is a zero-extension. Same width
    // (or a type whose width cannot be determined) is printed as a plain
    // pass-through, since Verilog does not need an explicit cast there.
    Value *castedValue = o.getValue();
    Type *sourceType   = hif::semantics::getSemanticType(castedValue, _sem);
    Type *targetType   = o.getType();

    unsigned long long sourceWidth = sourceType ? hif::semantics::typeGetSpanBitwidth(sourceType, _sem) : 0;
    unsigned long long targetWidth = targetType ? hif::semantics::typeGetSpanBitwidth(targetType, _sem) : 0;

    if (sourceWidth != 0 && targetWidth != 0 && targetWidth < sourceWidth) {
        // A part-select applies to a net or a variable, not to an arbitrary
        // expression: `(a << 2)[7:0]` is not legal Verilog-2001, and without
        // the parentheses the brackets bind to the shift amount instead
        // (hif-backend#18). Only emit the part-select when the operand is
        // something it can legally apply to.
        if (isPartSelectable(castedValue)) {
            castedValue->acceptVisitor(*this);
            (*_stream) << "[" << (targetWidth - 1) << ":0]";
            return 0;
        }
        // Otherwise, if this cast is the whole right-hand side of an
        // assignment to a target of exactly the cast's width, the
        // truncation is what Verilog already does on assignment - so
        // emitting the operand alone is both legal and exact.
        if (isTruncatedByAssignmentContext(o, targetWidth)) {
            castedValue->acceptVisitor(*this);
            return 0;
        }
        // Anything else - a narrowing cast of an expression somewhere the
        // width is self-determined, such as inside a concatenation - cannot
        // be expressed without introducing a temporary, which is a tree
        // transformation rather than something this printer can do. Emit
        // parenthesised so the operands at least group as intended, and say
        // so rather than producing a silently wrong width.
        messageWarning(
            "Cannot truncate this expression to " + std::to_string(targetWidth) +
                " bits without a temporary; emitted part-select is not valid Verilog (hif-backend#18).",
            &o, _sem);
        (*_stream) << "(";
        castedValue->acceptVisitor(*this);
        (*_stream) << ")[" << (targetWidth - 1) << ":0]";
        return 0;
    }
    if (sourceWidth != 0 && targetWidth != 0 && targetWidth > sourceWidth) {
        auto difference = targetWidth - sourceWidth;
        (*_stream) << "{" << difference << "'b";
        for (unsigned long long i = 0; i < difference; ++i) {
            (*_stream) << "0";
        }
        (*_stream) << ", ";
        castedValue->acceptVisitor(*this);
        (*_stream) << "}";
        return 0;
    }
    castedValue->acceptVisitor(*this);
    return 0;
}

auto VerilogPrinter::visitChar(Char &o) -> int { return GuideVisitor::visitChar(o); }

auto VerilogPrinter::visitCharValue(CharValue &o) -> int { return GuideVisitor::visitCharValue(o); }

auto VerilogPrinter::visitConst(Const &o) -> int { return GuideVisitor::visitConst(o); }

auto VerilogPrinter::visitContents(Contents &o) -> int { return hif::GuideVisitor::visitContents(o); }

auto VerilogPrinter::visitContinue(Continue &o) -> int { return GuideVisitor::visitContinue(o); }

auto VerilogPrinter::visitDesignUnit(DesignUnit &o) -> int
{
    messageAssert(o.views.size() == 1, "Not supported more than one view", &o, _sem);

    // Get the name of the design unit.
    auto name    = o.getName();
    // Get the current view.
    auto view    = o.views.front();
    // Get the view entity.
    auto entity  = view->getEntity();
    // Get the view content.
    auto content = view->getContents();

    // Which of this module's nets and outputs are driven continuously - by a
    // child instance's output port, by a global action printed below as a
    // continuous "assign", or by an output port's own initial value written
    // back out as one - has to be known before anything is printed: it decides
    // wire-vs-reg for both the port list and the body declarations
    // (hif-backend#26, #32, #30).
    this->collectContinuouslyDrivenDeclarations(view);

    // Delays are emitted as plain numbers, so the unit they count in has to be
    // declared before the module that uses them (hif-backend#24). Only designs
    // that carry a delay get a directive.
    this->resolveTimescale(view);
    this->printTimescaleDirective();

    // Print the module header.
    (*_stream) << "module " << name;

    // Template parameters go in an ANSI-style `#( ... )` clause, ahead of
    // the port list. They used to be printed as body declarations, which
    // put `parameter WIDTH = 4;` *after* the port list that referenced
    // WIDTH - accepted by every tool tried, but not Verilog-2001, and
    // trivially avoidable (hif-backend#20).
    if (!view->templateParameters.empty()) {
        (*_stream) << " #(\n";
        _stream->indent();
        for (std::size_t i = 0; i < view->templateParameters.size(); ++i) {
            auto *templateParameter = dynamic_cast<hif::Declaration *>(view->templateParameters.at(i));
            if (templateParameter == nullptr) {
                continue;
            }
            (*_stream) << this->getDeclaration(templateParameter);
            if (i < (view->templateParameters.size() - 1)) {
                (*_stream) << ",";
            }
            (*_stream) << "\n";
        }
        _stream->unindent();
        (*_stream) << ")";
    }

    entity->acceptVisitor(*this);
    (*_stream) << "\n";
    _stream->indent();

    // ========================================================================
    // PRINT VARIABLE DECLARATIONS
    // ========================================================================

    // Keep track if the view has variables.
    bool has_variables = false;

    // Print the list of view declarations. Each of these tested
    // view->templateParameters rather than the list it had just printed,
    // so the trailing blank line was governed by whether the module had
    // parameters. Now that parameters are printed in the header instead,
    // that would have been not just misleading but wrong.
    this->printList(view->declarations, ";", true, true);
    has_variables = has_variables || !view->declarations.empty();

    // Print the list of content declarations.
    this->printList(content->declarations, ";", true, true);
    has_variables = has_variables || !content->declarations.empty();

    // Print the list of state table declarations.
    for (auto stateTable : content->stateTables) {
        this->printList(stateTable->declarations, ";", true, true);
        has_variables = has_variables || !stateTable->declarations.empty();
    }

    if (has_variables) {
        (*_stream) << "\n";
    }

    // ========================================================================
    // PRINT FUNCTION DECLARATIONS
    // ========================================================================

    bool has_functions = false;

    has_functions |= this->printSubprograms(view->declarations);
    has_functions |= this->printSubprograms(content->declarations);
    for (auto stateTable : content->stateTables) {
        has_functions |= this->printSubprograms(stateTable->declarations);
    }

    if (has_functions) {
        (*_stream) << "\n";
    }

    // ========================================================================
    // PRINT THE REST
    // ========================================================================

    // The view's concurrent statements. vhdl2hif puts every VHDL concurrent
    // signal assignment here, and nothing printed them, so a VHDL design
    // regenerated as a module with the right ports and an empty body - valid
    // Verilog that drives nothing, produced with exit code 0 and no
    // diagnostic (hif-backend#32). Designs that came through verilog2hif are
    // unaffected either way: that frontend rewrites continuous assignments
    // into processes, which is why every existing test still passes.
    if (content->getGlobalAction() != nullptr) {
        content->getGlobalAction()->acceptVisitor(*this);
        if (!content->getGlobalAction()->actions.empty()) {
            (*_stream) << "\n";
        }
    }

    // An output port's initial value, written back out as the continuous
    // assignment the frontend folded into it. verilog2hif does that folding
    // for any constant right-hand side, and the value was then printed
    // nowhere - Verilog-2001 has no place for an initializer in an ANSI port
    // list - so "assign c = 32'd7;" regenerated as an empty module with an
    // undriven output (hif-backend#30). collectContinuouslyDrivenDeclarations
    // has already established that nothing else drives these.
    for (auto *port : _valueDrivenPorts) {
        (*_stream) << "assign " << port->getName() << " = ";
        port->getValue()->acceptVisitor(*this);
        (*_stream) << ";\n";
    }
    if (!_valueDrivenPorts.empty()) {
        (*_stream) << "\n";
    }

    // The other half of the same field: an output port that states an initial
    // value and is written by a process. That port is a reg, so a continuous
    // assign would be a second driver on it - the value is what it holds until
    // the process first writes it, which is an `initial` assignment
    // (hif-backend#36). Blocking, and in one block, so the ports are
    // initialized before any process that reads them can run.
    if (!_initialValuePorts.empty()) {
        (*_stream) << "initial begin\n";
        _stream->indent();
        for (auto *port : _initialValuePorts) {
            (*_stream) << port->getName() << " = ";
            port->getValue()->acceptVisitor(*this);
            (*_stream) << ";\n";
        }
        _stream->unindent();
        (*_stream) << "end\n\n";
    }

    for (auto instance : content->instances) {
        instance->acceptVisitor(*this);
    }

    for (auto generate : content->generates) {
        generate->acceptVisitor(*this);
    }

    // Visit the view contents.
    for (auto stateTable : content->stateTables) {
        stateTable->acceptVisitor(*this);
    }

    _stream->unindent();
    (*_stream) << "endmodule\n";
    return 0;
}

auto VerilogPrinter::visitEnum(Enum &o) -> int { return hif::GuideVisitor::visitEnum(o); }

auto VerilogPrinter::visitEnumValue(EnumValue &o) -> int
{
    (*_stream) << o.getName();
    return 0;
}

auto VerilogPrinter::visitEvent(Event &o) -> int { return GuideVisitor::visitEvent(o); }

auto VerilogPrinter::visitExpression(Expression &o) -> int
{
    // Concatenation is not an infix operator - unlike every other case
    // below, it doesn't sit between its operands, it wraps them:
    // "{a, b}". N-way concatenation composes naturally as nested
    // op_concat expressions (e.g. {a, {b, c}}), so this needs no special
    // multi-operand handling.
    if (o.getOperator() == op_concat) {
        (*_stream) << "{";
        o.getValue1()->acceptVisitor(*this);
        (*_stream) << ", ";
        o.getValue2()->acceptVisitor(*this);
        (*_stream) << "}";
        return 0;
    }

    // Set use of parenthesis for expression operands.
    bool needOp1Paren =
        ((dynamic_cast<ConstValue *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<Identifier *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<PPAssign *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<FunctionCall *>(o.getValue1()) == nullptr) && (dynamic_cast<Cast *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<Member *>(o.getValue1()) == nullptr));

    bool needOp2Paren =
        ((dynamic_cast<ConstValue *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<Identifier *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<PPAssign *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<FunctionCall *>(o.getValue2()) == nullptr) && (dynamic_cast<Cast *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<Member *>(o.getValue2()) == nullptr));

    // Check the existence of 2nd operand. If it is present the expression
    // is binary (i.e., Op1 op Op2) otherwise it is unary (i.e., op Op1)

    // If binary expression, print Op1. Unless pow (recursive call).
    if (o.getValue2() != nullptr) {
        if (needOp1Paren) {
            (*_stream) << "(";
        }
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren) {
            (*_stream) << ")";
        }
        (*_stream) << " ";
    }

    // Print operator.
    switch (o.getOperator()) {
    // Logical (boolean) operators
    case op_not:
        (*_stream) << "!";
        break;
    case op_or:
        (*_stream) << "||";
        break;
    case op_xor:
        (*_stream) << "^";
        break;
    case op_and:
        (*_stream) << "&&";
        break;

        // Binary (bitwise) operators
    case op_bnot:
        (*_stream) << "~";
        break;
    case op_bor:
        (*_stream) << "|";
        break;
    case op_bxor:
        (*_stream) << "^";
        break;
    case op_band:
        (*_stream) << "&";
        break;
    case op_sll:
        (*_stream) << "<<";
        break;
    case op_srl:
        (*_stream) << ">>";
        break;
    case op_rol:
        (*_stream) << "rol"; // TODO
        break;
    case op_ror:
        (*_stream) << "ror"; // TODO
        break;

        // Concatenation operator: handled by the early return above, never
        // reaches this switch.

        // Equality operators
    case op_eq:
        (*_stream) << "==";
        break;
    case op_case_eq:
        (*_stream) << "===";
        break;
    case op_neq:
        (*_stream) << "!=";
        break;
    case op_case_neq:
        (*_stream) << "!==";
        break;

        // Relational operators
    case op_lt:
        (*_stream) << "<";
        break;
    case op_le:
        (*_stream) << "<=";
        break;
    case op_gt:
        (*_stream) << ">";
        break;
    case op_ge:
        (*_stream) << ">=";
        break;

        // Arithmetic operators
    case op_plus:
        (*_stream) << "+";
        break;
    case op_minus:
        (*_stream) << "-";
        break;
    case op_mult:
        (*_stream) << "*";
        break;
    case op_div:
        (*_stream) << "/";
        break;
    case op_rem:
        (*_stream) << "rem"; // TODO
        break;
    case op_pow:
        (*_stream) << "**";
        break;
    case op_abs:
        (*_stream) << "abs"; // TODO
        break;
    case op_ref:
        break; // TODO
    case op_deref:
        break; // TODO
    case op_sla:
        (*_stream) << "sla"; // TODO
        break;
    case op_sra:
        (*_stream) << "sra"; // TODO
        break;
    case op_mod:
        (*_stream) << "%";
        break;
    case op_andrd:
        // Reduction AND: same literal token as binary op_band, applied here
        // in unary (single-operand) position - e.g. ~&a lowers to
        // op_bnot(op_andrd(a)), printed as "~ &a".
        (*_stream) << "&";
        break;
    case op_orrd:
        // Reduction OR: same token as binary op_bor, unary position.
        (*_stream) << "|";
        break;
    case op_xorrd:
        // Reduction XOR: same token as binary op_bxor, unary position.
        (*_stream) << "^";
        break;
    case op_assign:
    case op_conv:
    case op_bind:
    case op_log:
    case op_reverse:
    case op_size:
    case op_none:
    default:
        messageError("This operator should be managed in refinement steps.", &o, _sem);
    }
    (*_stream) << " ";

    // If binary expression, print Op2.
    if (o.getValue2() != nullptr) {
        if (needOp2Paren) {
            (*_stream) << "(";
        }
        o.getValue2()->acceptVisitor(*this);
        if (needOp2Paren) {
            (*_stream) << ")";
        }
    }
    // If unary expression, print Op1.
    else {
        if (needOp1Paren) {
            (*_stream) << "(";
        }
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren) {
            (*_stream) << ")";
        }
    }
    return 0;
}

namespace
{

/// @brief Recovers the Verilog spelling of a call to a system function or
/// system task.
/// @details Verilog spells these with a leading '$'. verilog2hif renames
/// "$name" to "_system_name" (FixDescription_1::_fixSystemTaskCalls) and
/// standardization then prefixes standard-library symbols with
/// "hif_<semantics>_", so "$clog2" reaches this printer named
/// "hif_verilog__system_clog2". Printing that name verbatim is not valid
/// Verilog - there is no such callable function, and nothing declares it on
/// the way back in, so the round trip breaks (hif-backend#19).
///
/// The renaming is the same for tasks as for functions - one frontend pass
/// does both - so recovering the name is too. Templated on the call type
/// rather than duplicated: a FunctionCall and a ProcedureCall differ in how
/// they are *printed* (an expression versus a statement), not in how their
/// name got mangled. hif-backend#29 was the task half going unprinted
/// entirely.
/// @param call The call to inspect.
/// @param sem The semantics used to resolve @p call's declaration.
/// @return The system call's name without its '$', or an empty string if
/// @p call is not a call to a Verilog system function or task.
template <typename TCall>
auto getSystemCallName(TCall &call, hif::semantics::ILanguageSemantics *sem) -> std::string
{
    const std::string standardPrefix("hif_" + sem->getName() + "_");
    const std::string systemPrefix("_system_");

    std::string name(call.getName());
    if (name.compare(0, standardPrefix.size(), standardPrefix) == 0) {
        name.erase(0, standardPrefix.size());
    }
    if (name.compare(0, systemPrefix.size(), systemPrefix) != 0) {
        return "";
    }

    // "_system_" is a legal identifier prefix in Verilog, so a user subprogram
    // could carry this name of its own accord. Only the standard library's
    // ones are the renamed '$' ones; anything a design declares itself keeps
    // the name it has. A call whose declaration cannot be resolved is treated
    // as a system call: the name says it came from '$', and printing it
    // unchanged is known to be wrong.
    auto *decl = hif::semantics::getDeclaration(&call, sem);
    if (decl != nullptr && !hif::declarationIsPartOfStandard(decl)) {
        return "";
    }

    return name.substr(systemPrefix.size());
}

} // namespace

auto VerilogPrinter::visitFunctionCall(FunctionCall &o) -> int
{
    if (o.getName() == "hif_verilog_iterated_concat") {
        // Verilog replication ({times{expression}}), synthesized internally
        // by hif-frontend's _fixiteratedConcat as a call to hif-core's
        // standard-library iterated_concat subprogram. Printed here using
        // real Verilog replication syntax: generic function-call printing
        // would be wrong even if it worked (there is no such callable
        // function in real Verilog), and ParameterAssign/ValueTPAssign
        // values aren't printed at all by the generic path below.
        ValueTPAssign *timesAssign = nullptr;
        for (auto *tpAssign : o.templateParameterAssigns) {
            auto *vtpAssign = dynamic_cast<ValueTPAssign *>(tpAssign);
            if (vtpAssign != nullptr && vtpAssign->getName() == "times") {
                timesAssign = vtpAssign;
                break;
            }
        }
        ParameterAssign *exprAssign = nullptr;
        for (auto *pAssign : o.parameterAssigns) {
            if (pAssign->getName() == "expression") {
                exprAssign = pAssign;
                break;
            }
        }
        messageAssert(
            timesAssign != nullptr && exprAssign != nullptr, "Malformed iterated_concat call", &o, _sem);
        (*_stream) << "{";
        timesAssign->getValue()->acceptVisitor(*this);
        (*_stream) << "{";
        exprAssign->getValue()->acceptVisitor(*this);
        (*_stream) << "}}";
        return 0;
    }

    const std::string systemName(getSystemCallName(o, _sem));
    if (!systemName.empty()) {
        (*_stream) << "$" << systemName;
        // Verilog's argument-less system functions ($time, $realtime, ...)
        // are spelled without parentheses; "$time()" is not accepted.
        if (o.parameterAssigns.empty()) {
            return 0;
        }
    } else {
        (*_stream) << o.getName();
    }

    (*_stream) << "(";
    for (std::size_t i = 0; i < o.parameterAssigns.size(); ++i) {
        o.parameterAssigns.at(i)->acceptVisitor(*this);
        if (i < o.parameterAssigns.size() - 1) {
            (*_stream) << ", ";
        }
    }
    (*_stream) << ")";
    return 0;
    // return hif::GuideVisitor::visitFunctionCall(o);
}

auto VerilogPrinter::visitField(Field &o) -> int { return GuideVisitor::visitField(o); }

auto VerilogPrinter::visitFieldReference(FieldReference &o) -> int { return GuideVisitor::visitFieldReference(o); }

auto VerilogPrinter::visitFile(File &o) -> int { return GuideVisitor::visitFile(o); }

auto VerilogPrinter::visitFor(For &o) -> int
{
    // Print the for loop.
    (*_stream) << "for (";
    for (std::size_t i = 0; i < o.initValues.size(); ++i) {
        // Cast the init value to an assignment.
        auto assign = dynamic_cast<Assign *>(o.initValues.at(i));
        if (assign) {
            assign->getLeftHandSide()->acceptVisitor(*this);
            (*_stream) << " = ";
            assign->getRightHandSide()->acceptVisitor(*this);
        }
        if (i < o.initValues.size() - 1) {
            (*_stream) << ", ";
        }
    }
    (*_stream) << "; ";
    if (o.getCondition()) {
        (*_stream) << this->getValue(o.getCondition());
    }
    (*_stream) << "; ";
    for (std::size_t i = 0; i < o.stepActions.size(); ++i) {
        // Cast the init value to an assignment.
        auto assign = dynamic_cast<Assign *>(o.stepActions.at(i));
        if (assign) {
            assign->getLeftHandSide()->acceptVisitor(*this);
            (*_stream) << " = ";
            assign->getRightHandSide()->acceptVisitor(*this);
        }
        if (i < o.stepActions.size() - 1) {
            (*_stream) << ", ";
        }
    }
    (*_stream) << " ) begin ";
    if (!o.getName().empty()) {
        (*_stream) << ": " << o.getName();
    }
    (*_stream) << "\n";

    _stream->indent();

    for (auto for_action : o.forActions) {
        for_action->acceptVisitor(*this);
    }

    _stream->unindent();
    (*_stream) << "end\n";
    return 0;
}

auto VerilogPrinter::visitForGenerate(ForGenerate &o) -> int { return GuideVisitor::visitForGenerate(o); }

namespace
{

/// @brief The Return that leaves a subprogram body by falling off its end: the
/// last action of the last state.
/// @details Such a Return needs no `disable`, because reaching the end of the
/// block already leaves it. Distinguishing it is what keeps the emitted output
/// unchanged for the common single-Return function a VHDL source produces.
auto getTrailingReturn(hif::StateTable *stateTable) -> hif::Return *
{
    if (stateTable == nullptr || stateTable->states.empty()) {
        return nullptr;
    }
    hif::State *last = stateTable->states.back();
    if (last->actions.empty()) {
        return nullptr;
    }
    return dynamic_cast<hif::Return *>(last->actions.back());
}

} // namespace

auto VerilogPrinter::visitFunction(Function &o) -> int
{
    // Get the return type of the function.
    auto return_type = o.getType();
    // Get the state table.
    auto state_table = o.getStateTable();

    (*_stream) << "function";
    if (return_type) {
        (*_stream) << " ";
        return_type->acceptVisitor(*this);
        (*_stream) << " ";
    }
    (*_stream) << o.getName() << ";\n";

    _stream->indent();

    // ========================================================================
    // RENAME THE RETURN VARIABLE
    // ========================================================================

    hif::Identifier return_variable_id(o.getName() + "_return");
    // Get the declaration of the return variable.
    hif::Declaration *return_variable = hif::semantics::getDeclaration(&return_variable_id, _sem, {&o});
    if (!return_variable) {
        return_variable = hif::semantics::getDeclaration(&return_variable_id, _sem, {state_table});
    }
    // If we found the return variable, we can rename it.
    if (return_variable) {
        // Get all the references to the return variable.
        hif::semantics::ReferencesSet list;
        hif::semantics::getReferences(return_variable, list, _sem, &o);
        // Set the name of the return variable.
        for (auto entity : list) {
            if (auto named_object = dynamic_cast<hif::features::INamedObject *>(entity)) {
                named_object->setName(o.getName());
            }
        }
    }
    // Deliberately no diagnostic when there is none. A function built by
    // verilog2hif has a `<name>_return` variable that its body assigns, and the
    // rename above turns those assignments into the Verilog form. A function
    // built by vhdl2hif has no such variable: its body is a Return carrying the
    // value directly, which visitReturn emits (hif-backend#57). Both are
    // ordinary, so the absent variable is not an error - it used to print
    // "Cannot find the return variable" on stderr for every VHDL function.

    // ========================================================================
    // PRINT VARIABLE DECLARATIONS
    // ========================================================================

    bool has_variables = false;

    // Print only the declarations that are not the return variable.
    for (auto parameter : o.parameters) {
        if (parameter->getName() == (o.getName() + "_return")) {
            continue;
        }
        (*_stream) << this->getDeclaration(parameter) << ";\n";
        has_variables = true;
    }
    for (auto templateParameter : o.templateParameters) {
        if (templateParameter->getName() == (o.getName() + "_return")) {
            continue;
        }
        (*_stream) << this->getDeclaration(templateParameter) << ";\n";
        has_variables = true;
    }
    for (auto declaration : state_table->declarations) {
        if (declaration->getName() == (o.getName() + "_return")) {
            continue;
        }
        (*_stream) << this->getDeclaration(declaration) << ";\n";
        has_variables = true;
    }

    if (has_variables) {
        (*_stream) << "\n";
    }

    // ========================================================================
    // PRINT FUNCTION CONTENT
    // ========================================================================

    const std::string exitLabel = this->getSubprogramExitLabel(state_table, o.getName());
    (*_stream) << "begin";
    if (!exitLabel.empty()) {
        (*_stream) << " : " << exitLabel;
    }
    (*_stream) << "\n";
    _stream->indent();

    // Publish the name a Return inside this body has to assign to, and the block
    // an early one has to disable. Saved and restored rather than cleared, so a
    // nested subprogram cannot strand the outer function's.
    const std::string enclosingFunctionName = _currentFunctionName;
    const std::string enclosingLabel        = _subprogramExitLabel;
    hif::Return *enclosingTrailingReturn    = _subprogramTrailingReturn;
    const bool enclosingInsideSubprogram    = _insideSubprogramBody;
    _currentFunctionName                    = o.getName();
    _subprogramExitLabel                    = exitLabel;
    _subprogramTrailingReturn               = getTrailingReturn(state_table);
    _insideSubprogramBody                   = true;

    for (auto state : state_table->states) {
        state->acceptVisitor(*this);
    }

    _currentFunctionName      = enclosingFunctionName;
    _subprogramExitLabel      = enclosingLabel;
    _subprogramTrailingReturn = enclosingTrailingReturn;
    _insideSubprogramBody     = enclosingInsideSubprogram;

    _stream->unindent();
    (*_stream) << "end\n";
    _stream->unindent();
    (*_stream) << "endfunction\n";

    return 0;
    // return GuideVisitor::visitFunction(o);
}

auto VerilogPrinter::visitGlobalAction(GlobalAction &o) -> int
{
    // A global action is a *concurrent* statement, so it cannot be printed by
    // falling through to the Assign visitor: that one emits the procedural
    // form ("t <= rhs;"), which is only legal inside a process and would
    // assign to a reg. The concurrent equivalent is a continuous assignment,
    // which is also exactly what the VHDL this comes from means
    // (hif-backend#32).
    //
    // The targets were registered as nets by collectContinuouslyDrivenDeclarations,
    // so what is declared and what is driven agree.
    for (auto *action : o.actions) {
        auto *assign = dynamic_cast<Assign *>(action);

        // Anything else has no continuous form to be emitted as. Dropping it
        // is what this issue was about, so this fails loudly and names the
        // construct instead of producing a module that quietly does less than
        // its source. Neither frontend can currently produce one: vhdl2hif
        // puts only concurrent signal assignments here (its parser types the
        // list as BList<Assign>), and verilog2hif rewrites global actions into
        // processes before the backend sees them.
        messageAssert(
            assign != nullptr,
            "Unsupported global action: only a concurrent signal assignment has a Verilog continuous-assignment "
            "equivalent, and emitting nothing would silently drop a driver (hif-backend#32).",
            action, _sem);

        (*_stream) << "assign ";

        // A continuous assignment states its delay right after the keyword
        // ("assign #2 t = ...;"). It cannot use the intra-assignment position
        // visitAssign emits ("t <= #2 ...") - that form is procedural-only.
        // This is the first path on which a VHDL "after" delay reaches the
        // output at all: it was already carried in the HIF and already
        // handled by renderDelay/resolveTimescale (hif-backend#24), but no
        // VHDL-derived assignment was being printed to carry it
        // (hif-backend#32).
        const std::string delay(this->renderDelay(assign->getDelay()));
        if (!delay.empty()) {
            (*_stream) << "#" << delay << " ";
        }

        assign->getLeftHandSide()->acceptVisitor(*this);

        // Always "=". A continuous assignment has no blocking/non-blocking
        // distinction - "<=" here would parse as less-than-or-equal.
        (*_stream) << " = ";
        assign->getRightHandSide()->acceptVisitor(*this);
        (*_stream) << ";\n";
    }
    return 0;
}

auto VerilogPrinter::visitEntity(Entity &o) -> int
{
    // port_direction data_type [ port_size ] port_name, port_name, ...;
    if (!o.ports.empty()) {
        (*_stream) << "(" << '\n';
        // Increase indentation.
        _stream->indent();
        // Iterate the ports and print them.
        this->printList(o.ports, ",", true, false);
        // Decrese the indentation.
        _stream->unindent();
        (*_stream) << ");";
    }
    return 0;
}

auto VerilogPrinter::visitIdentifier(Identifier &o) -> int
{
    (*_stream) << o.getName();
    return 0;
}

auto VerilogPrinter::visitIf(If &o) -> int
{
    bool first = true;
    for (hif::IfAlt *alt : o.alts) {
        if (first) {
            (*_stream) << "if ( " << this->getValue(alt->getCondition()) << " ) begin\n";
            first = false;
        } else {
            (*_stream) << "else if ( " << this->getValue(alt->getCondition()) << " ) begin\n";
        }
        _stream->indent();
        for (hif::Action *action : alt->actions) {
            action->acceptVisitor(*this);
        }
        _stream->unindent();
        (*_stream) << "end\n";
    }
    if (!o.defaults.empty()) {
        (*_stream) << "else begin\n";
        _stream->indent();
        for (hif::Action *action : o.defaults) {
            action->acceptVisitor(*this);
        }
        _stream->unindent();
        (*_stream) << "end\n";
    }
    return 0;
}

auto VerilogPrinter::visitIfAlt(IfAlt &o) -> int { return GuideVisitor::visitIfAlt(o); }

auto VerilogPrinter::visitIfGenerate(IfGenerate &o) -> int { return GuideVisitor::visitIfGenerate(o); }

auto VerilogPrinter::visitInstance(Instance &o) -> int
{
    if (auto view_reference = dynamic_cast<ViewReference *>(o.getReferencedType())) {
        (*_stream) << view_reference->getDesignUnit() << " ";
        (*_stream) << o.getName() << "(" << '\n';
        // Increase indentation.
        _stream->indent();
        // Iterate the ports and print them.
        this->printList(o.portAssigns, ",", true, false);
        // Decrese the indentation.
        _stream->unindent();
        (*_stream) << ");\n\n";
    }
    return 0;
}

auto VerilogPrinter::visitInt(Int &o) -> int { return hif::GuideVisitor::visitInt(o); }

auto VerilogPrinter::visitIntValue(IntValue &o) -> int
{
    auto value = this->getValue(&o);
    if (!value.empty()) {
        (*_stream) << value;
    }
    return 0;
}

auto VerilogPrinter::visitLibraryDef(LibraryDef &o) -> int { return hif::GuideVisitor::visitLibraryDef(o); }

auto VerilogPrinter::visitLibrary(Library &o) -> int { return hif::GuideVisitor::visitLibrary(o); }

auto VerilogPrinter::visitMember(Member &o) -> int
{
    (*_stream) << this->getValue(&o);
    return 0;
}

auto VerilogPrinter::visitNull(Null &o) -> int { return hif::GuideVisitor::visitNull(o); }

auto VerilogPrinter::visitTransition(Transition &o) -> int { return hif::GuideVisitor::visitTransition(o); }

auto VerilogPrinter::visitParameterAssign(ParameterAssign &o) -> int
{
    return hif::GuideVisitor::visitParameterAssign(o);
}

auto VerilogPrinter::visitProcedure(Procedure &o) -> int
{
    // A user-written Verilog task. It is a Procedure with a StateTable, just
    // as a cone is, and it used to take the cone path below: inlined at its
    // call site, where the hif-backend#16 assert then fired because a task
    // assigns to the signals and ports it was written to drive rather than to
    // the "_sig_var" Variables a cone uses. The tool exited 1 and left a
    // zero-byte file, reporting a cone invariant about a design containing no
    // cone (hif-backend#38).
    //
    // Verilog does have a user-declarable procedure construct after all - a
    // task - so a task is emitted as one, and called rather than expanded.
    if (!isConeProcedure(&o)) {
        return this->printTask(o);
    }

    // Cones are frontend-synthesized "cone functions" (hif-frontend's
    // generateConeFunctions/fixLogicCones, FixDescription_3.cpp) wrapping a
    // shared combinational sub-expression - e.g. the logic a primitive gate
    // instance or a flattened combinational submodule instance lowers to.
    //
    // A cone is *not* an independent process: the frontend inserts an
    // explicit call to it into the body of every process that reads its
    // target, and gives those processes a sensitivity list over the
    // cone's transitive primary inputs precisely because the cone is
    // re-evaluated inside the caller. Nothing is printed here; the body
    // is expanded at each call site by visitProcedureCall.
    //
    // Emitting the cone as its own `always @(*)` block instead - as this
    // printer used to - hoists it out of every caller, converting an
    // intra-process dependency into an inter-process one that nothing
    // orders. The caller stays sensitive to the primary inputs while
    // reading a target some other process now writes, so it can evaluate
    // a stale value: the regenerated Verilog parses and reparses cleanly
    // but does not simulate like its source (hif-backend#16).
    static_cast<void>(o);
    return 0;
}

auto VerilogPrinter::visitParameter(Parameter &o) -> int { return hif::GuideVisitor::visitParameter(o); }

auto VerilogPrinter::visitProcedureCall(ProcedureCall &o) -> int
{
    // A Verilog system task ($display, $monitor, $finish, ...). Handled before
    // anything else below, and returning immediately, so that the
    // cone-inlining logic and the hif-backend#16 contract it carries are left
    // exactly as they were: a system task is not a cone, has no body to
    // inline, and must not participate in any of that reasoning.
    //
    // It used to fall through to the "no StateTable" early return further
    // down. hif-core declares these as subprograms with no return type and no
    // body (ILanguageSemantics::_addMultiparamFunction, called with a null
    // return type), so the call resolved to a Procedure whose getStateTable()
    // is null, and "return 0" dropped it without a word. Verified by
    // instrumenting this function: a $display call reports
    // PROCEDURE_NO_STATETABLE with declarationIsPartOfStandard true
    // (hif-backend#29).
    //
    // Dropping it regenerates a design that compiles and simulates and prints
    // nothing, which is how a round trip silently discards a testbench's or an
    // instrumented model's entire observable output.
    const std::string systemName(getSystemCallName(o, _sem));
    if (!systemName.empty()) {
        (*_stream) << "$" << systemName;
        // Argument-less system tasks are spelled without parentheses, as the
        // argument-less system *functions* are in visitFunctionCall: "$finish"
        // rather than "$finish()".
        if (!o.parameterAssigns.empty()) {
            (*_stream) << "(";
            for (std::size_t i = 0; i < o.parameterAssigns.size(); ++i) {
                o.parameterAssigns.at(i)->acceptVisitor(*this);
                if (i < o.parameterAssigns.size() - 1) {
                    (*_stream) << ", ";
                }
            }
            (*_stream) << ")";
        }
        // A task call is a statement, not an expression: it terminates itself.
        (*_stream) << ";\n";
        return 0;
    }

    // Expand the callee's body here. See visitProcedure for why cones are
    // inlined at their call sites rather than hoisted into a process of
    // their own.
    //
    // The cone's target is a Variable, so visitAssign renders its
    // assignments with blocking "=": statements printed after this call -
    // including the reads of the target that motivated the call - observe
    // the value just computed, which is exactly the HIF semantics of the
    // call being replaced.
    //
    // Inlining a cone into each of its callers means a shared cone's
    // target is written by more than one always block. That is not
    // introduced here: the HIF already has several processes calling the
    // one cone, and each write recomputes the same pure function of the
    // same inputs, so every caller reads the value it just wrote
    // regardless of how the blocks interleave.
    auto *procedure = dynamic_cast<Procedure *>(hif::semantics::getDeclaration(&o, _sem));
    if (procedure == nullptr) {
        return hif::GuideVisitor::visitProcedureCall(o);
    }

    auto *stateTable = procedure->getStateTable();
    if (stateTable == nullptr) {
        return 0;
    }

    // A user-written task is called, not expanded. Inlining one hit the
    // hif-backend#16 assert - a task assigns to the signals and ports it was
    // written to drive, not to a cone's "_sig_var" Variable - and aborted with
    // a zero-byte output file (hif-backend#38). The cone contract below is
    // untouched: this returns before reaching it.
    if (!isConeProcedure(procedure)) {
        (*_stream) << o.getName();
        if (!o.parameterAssigns.empty()) {
            (*_stream) << "(";
            for (std::size_t i = 0; i < o.parameterAssigns.size(); ++i) {
                o.parameterAssigns.at(i)->acceptVisitor(*this);
                if (i < o.parameterAssigns.size() - 1) {
                    (*_stream) << ", ";
                }
            }
            (*_stream) << ")";
        }
        (*_stream) << ";\n";
        return 0;
    }

    if (!_inliningCones.insert(procedure).second) {
        messageError("Recursive cone procedure cannot be inlined", &o, _sem);
    }
    for (auto state : stateTable->states) {
        state->acceptVisitor(*this);
    }
    _inliningCones.erase(procedure);
    return 0;
}

auto VerilogPrinter::visitPointer(Pointer &o) -> int { return hif::GuideVisitor::visitPointer(o); }

auto VerilogPrinter::visitPortAssign(PortAssign &o) -> int { return hif::GuideVisitor::visitPortAssign(o); }

auto VerilogPrinter::visitPort(Port &o) -> int { return hif::GuideVisitor::visitPort(o); }

auto VerilogPrinter::visitRange(Range &o) -> int { return hif::GuideVisitor::visitRange(o); }

auto VerilogPrinter::visitReal(Real &o) -> int { return hif::GuideVisitor::visitReal(o); }

auto VerilogPrinter::visitRealValue(RealValue &o) -> int { return hif::GuideVisitor::visitRealValue(o); }

auto VerilogPrinter::visitRecord(Record &o) -> int { return hif::GuideVisitor::visitRecord(o); }

auto VerilogPrinter::visitRecordValue(RecordValue &o) -> int { return hif::GuideVisitor::visitRecordValue(o); }

auto VerilogPrinter::visitRecordValueAlt(RecordValueAlt &o) -> int { return hif::GuideVisitor::visitRecordValueAlt(o); }

auto VerilogPrinter::visitReference(Reference &o) -> int { return hif::GuideVisitor::visitReference(o); }

auto VerilogPrinter::visitReturn(Return &o) -> int
{
    // Verilog has no `return`. A Return therefore lowers to two separate things,
    // and this used to emit only ever one of them: the value a function yields,
    // as an assignment to the function's own name (hif-backend#57), and the exit
    // itself, as `disable` on the block wrapping the subprogram body
    // (hif-backend#63, #73). A void subprogram needs only the second; a function
    // whose Return is not its last statement needs both.
    hif::Value *returned = o.getValue();

    // ---- The value, if there is one. ---------------------------------------
    if (!_currentFunctionName.empty() && returned != nullptr) {
        // A Return whose value is already the function's own name assigns
        // nothing: verilog2hif builds a `<name>_return` variable that the body
        // assigns and then returns, and the rename in visitFunction has by now
        // turned both into the function name. Emitting it would produce
        // `f = f;`. The value-carrying shape is what vhdl2hif produces, where
        // the Return *is* the body.
        auto *identifier = dynamic_cast<hif::Identifier *>(returned);
        if (identifier == nullptr || identifier->getName() != _currentFunctionName) {
            (*_stream) << _currentFunctionName << " = ";
            returned->acceptVisitor(*this);
            (*_stream) << ";\n";
        }
    }

    // ---- The exit. ---------------------------------------------------------
    if (!_insideSubprogramBody) {
        // A Return reached outside any subprogram body has no block to leave.
        // Nothing produces this today; warning rather than asserting keeps a
        // future frontend change from costing the whole design.
        messageWarning(
            "A `return` was reached outside a subprogram body, where there is no block to disable, so the "
            "exit is not emitted (hif-backend#63).",
            &o, _sem);
        return 0;
    }

    if (&o == _subprogramTrailingReturn) {
        // Falling off the end of the body already leaves it, so the common
        // single-Return function keeps the output it has always had.
        return 0;
    }

    // An early exit is `disable` naming the block that wraps the body (IEEE Std
    // 1364-2005, 9.8.1 for the named block, 9.6.4 for disable). `return` inside
    // a task is SystemVerilog, and the project's regressions build with
    // `iverilog -g2005`, so it is not available.
    //
    // The alternative lowering - restructuring the body into guarded branches -
    // preserves re-entrancy, which `disable` does not: a Verilog-2001 named
    // block is static, so disabling it affects any concurrent activation of the
    // same subprogram. That is this form's known limit, and it is not a new
    // one - a Verilog-2001 task's locals are static too, so a task called
    // concurrently from two processes already does not behave as two
    // independent calls. Nothing correct today is lost by choosing it.
    messageAssert(
        !_subprogramExitLabel.empty(),
        "A non-trailing Return is being printed with no named body block to disable, so the early exit "
        "would be dropped silently (hif-backend#63).",
        &o, _sem);
    (*_stream) << "disable " << _subprogramExitLabel << ";\n";
    return 0;
}

auto VerilogPrinter::visitSignal(Signal &o) -> int { return hif::GuideVisitor::visitSignal(o); }

auto VerilogPrinter::visitSigned(Signed &o) -> int { return hif::GuideVisitor::visitSigned(o); }

auto VerilogPrinter::visitSlice(Slice &o) -> int
{
    (*_stream) << this->getValue(&o);
    return 0;
}

auto VerilogPrinter::visitState(State &o) -> int { return hif::GuideVisitor::visitState(o); }

auto VerilogPrinter::visitString(String &o) -> int { return hif::GuideVisitor::visitString(o); }

auto VerilogPrinter::visitStateTable(StateTable &o) -> int
{
    /// @brief The list of states of the statetable.
    //      BList<State> states;
    /// @brief List of edges.
    //      BList<Transition> edges;

    if (o.getFlavour() == hif::ProcessFlavour::pf_analog) {
        (*_stream) << "analog begin\n";
        _stream->indent();

        for (auto state : o.states) {
            state->acceptVisitor(*this);
        }

        _stream->unindent();
        (*_stream) << "\n";
        (*_stream) << "end\n";
    } else if (!isRetriggerable(o)) {
        // A process with nothing to wake it up runs exactly once, at time
        // zero. `always` would make it a zero-delay infinite loop, which no
        // source ever meant and which Icarus rejects at elaboration
        // (hif-backend#40). Verilog spells "run once at startup" `initial`.
        (*_stream) << "initial begin" << '\n';
        _stream->indent();

        for (auto state : o.states) {
            state->acceptVisitor(*this);
        }

        _stream->unindent();
        (*_stream) << "\n";
        (*_stream) << "end\n";
    } else {
        (*_stream) << "always";

        // Emit the union of the three lists, qualifying each signal
        // individually.
        //
        // This used to be an if/else-if chain that printed whichever list
        // was non-empty first, and put a single `posedge`/`negedge` ahead of
        // a whole comma-separated list. Both halves lost sensitivity
        // silently (hif-backend#21): `always @(posedge clk or negedge rst_n)`
        // came back as `always @(posedge clk)`, turning an asynchronous
        // reset into a synchronous one, and `always @(posedge clk or posedge
        // rst)` came back as `always @(posedge clk, rst)`, leaving rst
        // sensitive to both of its edges.
        if (!o.sensitivity.empty() || !o.sensitivityPos.empty() || !o.sensitivityNeg.empty()) {
            bool isFirst = true;
            auto printSensitivity = [&](hif::BList<hif::Value> &list, const std::string &edge) {
                for (auto *signal : list) {
                    (*_stream) << (isFirst ? " @( " : ", ");
                    isFirst = false;
                    if (!edge.empty()) {
                        (*_stream) << edge << " ";
                    }
                    signal->acceptVisitor(*this);
                }
            };
            printSensitivity(o.sensitivity, "");
            printSensitivity(o.sensitivityPos, "posedge");
            printSensitivity(o.sensitivityNeg, "negedge");

            // A delayed assignment in this process schedules its target for
            // later instead of writing it now, so the statements after it -
            // and the whole process on its next run - would keep reading the
            // pre-delay value unless the target itself re-triggers the process
            // (hif-backend#24). The HIF sensitivity list names the delayed
            // net's *inputs*, which is correct for an immediate assignment and
            // one event too early for a delayed one.
            //
            // Only level-sensitive processes are extended. Adding a plain
            // entry to an edge-sensitive list would make a register update
            // between clock edges, which is a worse error than the one being
            // fixed.
            if (o.sensitivityPos.empty() && o.sensitivityNeg.empty()) {
                std::set<std::string> alreadySensitive;
                for (auto *signal : o.sensitivity) {
                    auto *identifier = dynamic_cast<hif::Identifier *>(signal);
                    if (identifier != nullptr) {
                        alreadySensitive.insert(identifier->getName());
                    }
                }
                std::set<std::string> delayedTargets;
                this->collectDelayedTargets(&o, delayedTargets);
                for (const auto &name : delayedTargets) {
                    if (alreadySensitive.count(name) != 0) {
                        continue;
                    }
                    (*_stream) << ", " << name;
                }
            }

            (*_stream) << " )";
        }
        (*_stream) << " begin" << '\n';
        _stream->indent();

        for (auto state : o.states) {
            state->acceptVisitor(*this);
        }

        _stream->unindent();
        (*_stream) << "\n";
        (*_stream) << "end\n";
    }
    return 0;
}

auto VerilogPrinter::visitSystem(System &o) -> int { return hif::GuideVisitor::visitSystem(o); }

auto VerilogPrinter::visitSwitchAlt(SwitchAlt &o) -> int { return GuideVisitor::visitSwitchAlt(o); }

auto VerilogPrinter::visitSwitch(Switch &o) -> int
{

    (*_stream) << "case ( " << this->getValue(o.getCondition()) << " )\n";
    _stream->indent();
    for (hif::SwitchAlt *alternative : o.alts) {
        // A SwitchAlt can carry more than one condition: alternatives sharing
        // the same body are merged into a single alt holding every matching
        // value. Verilog requires those labels to be comma-separated
        // ("STATE_2, FINAL :"); emitting them space-separated produces
        // "STATE_2 FINAL :", which is a syntax error. The VHDL printer already
        // joins them explicitly (with " |"), so only this printer was affected.
        bool firstCondition = true;
        for (hif::Value *condition : alternative->conditions) {
            if (!firstCondition) {
                (*_stream) << ", ";
            }
            (*_stream) << this->getValue(condition);
            firstCondition = false;
        }
        (*_stream) << " : begin\n";
        _stream->indent();
        for (hif::Action *action : alternative->actions) {
            action->acceptVisitor(*this);
        }
        _stream->unindent();
        (*_stream) << "end\n";
    }
    if (!o.defaults.empty()) {
        (*_stream) << "default" << ": begin\n";
        _stream->indent();
        for (hif::Action *action : o.defaults) {
            action->acceptVisitor(*this);
        }
        _stream->unindent();
        (*_stream) << "end\n";
    }
    _stream->unindent();
    (*_stream) << "endcase\n";
    return 0;
}

auto VerilogPrinter::visitStringValue(StringValue &o) -> int
{
    // String literals were printed nowhere at all: this delegated to
    // GuideVisitor, which visits children and writes nothing.
    //
    // This is fixed here rather than filed separately because restoring
    // $display without it does not actually restore anything (hif-backend#29).
    // A "$display(, a)" - which is what emitting the call alone produces - has
    // lost the format string that carries the message, so the regenerated
    // design still prints nothing useful, and verilog2hif rejects it on the
    // way back in (exit 1). A fix that re-emits system task calls has to own
    // how their arguments are written, and the first argument of the task this
    // issue is about is always a string.
    //
    // A "plain" StringValue is an opaque passthrough - text that is already
    // target-language source and must not be quoted at all.
    if (o.isPlain()) {
        (*_stream) << o.getValue();
        return 0;
    }

    // Emitted verbatim between quotes, NOT re-escaped. verilog2hif stores the
    // literal's text in source form, with its escape sequences left as they
    // were written: `\"` is stored as backslash-quote and `\\` as
    // backslash-backslash, as the XML shows. Escaping again would double every
    // backslash and turn `"a\"b"` into `"a\\\"b"`, which says something
    // different. Writing the stored text back out is what makes the literal
    // round trip byte for byte.
    (*_stream) << "\"" << o.getValue() << "\"";
    return 0;
}

auto VerilogPrinter::visitTime(Time &o) -> int { return hif::GuideVisitor::visitTime(o); }

auto VerilogPrinter::visitTimeValue(TimeValue &o) -> int { return hif::GuideVisitor::visitTimeValue(o); }

auto VerilogPrinter::visitTypeDef(TypeDef &o) -> int { return GuideVisitor::visitTypeDef(o); }

auto VerilogPrinter::visitTypeReference(TypeReference &o) -> int { return GuideVisitor::visitTypeReference(o); }

auto VerilogPrinter::visitTypeTPAssign(TypeTPAssign &o) -> int { return hif::GuideVisitor::visitTypeTPAssign(o); }

auto VerilogPrinter::visitTypeTP(TypeTP &o) -> int { return hif::GuideVisitor::visitTypeTP(o); }

auto VerilogPrinter::visitUnsigned(Unsigned &o) -> int { return hif::GuideVisitor::visitUnsigned(o); }

auto VerilogPrinter::visitValueStatement(ValueStatement &o) -> int { return GuideVisitor::visitValueStatement(o); }

auto VerilogPrinter::visitValueTPAssign(ValueTPAssign &o) -> int { return hif::GuideVisitor::visitValueTPAssign(o); }

auto VerilogPrinter::visitValueTP(ValueTP &o) -> int { return hif::GuideVisitor::visitValueTP(o); }

auto VerilogPrinter::visitVariable(Variable &o) -> int { return hif::GuideVisitor::visitVariable(o); }

auto VerilogPrinter::visitView(View &o) -> int { return hif::GuideVisitor::visitView(o); }

auto VerilogPrinter::visitViewReference(ViewReference &o) -> int { return GuideVisitor::visitViewReference(o); }

auto VerilogPrinter::visitWait(Wait &o) -> int
{
    // This used to be a bare `return GuideVisitor::visitWait(o);`. That printed
    // no `wait` keyword, but the guide visitor still descended into the node's
    // children, so the wait's condition or sensitivity was printed *bare* and
    // ran straight into the statement that followed it: `wait (en); o = 4'd7;`
    // came back as `eno <= 4'b0111;`, inventing an undeclared identifier by
    // concatenation while hif2verilog exited 0 (hif-backend#42).
    //
    // Nothing below delegates to GuideVisitor: every branch prints the whole
    // statement itself, and falling through to the guide visitor afterwards
    // would print the children a second time.
    const bool hasCondition = o.getCondition() != nullptr;
    const bool hasSensitivity =
        !o.sensitivity.empty() || !o.sensitivityPos.empty() || !o.sensitivityNeg.empty();
    const bool hasTime = o.getTime() != nullptr;

    // Verilog spells each of the three suspension forms with a different
    // construct - `wait (c)`, `@( ... )`, `#t` - and has no single statement
    // meaning more than one at once. VHDL does: `wait on a until c for 10 ns;`
    // sets all three, because its three clauses are independently optional
    // (vhdl2hif's parse_WaitStatement). Such a wait needs a lowering rather
    // than a keyword, which is what printMultiClauseWait does (hif-backend#45).
    // It used to be refused outright here.
    const int forms = static_cast<int>(hasCondition) + static_cast<int>(hasSensitivity) + static_cast<int>(hasTime);
    if (o.getRepetitions() != nullptr) {
        messageError(
            "Unsupported Wait shape: a repeated wait is not emitted by this printer (hif-backend#45). "
            "verilog2hif lowers `repeat (n) @( ... )` to a For around a plain wait instead of setting "
            "this field, so reaching here means the HIF came from somewhere else.",
            &o, _sem);
    }
    if (!o.actions.empty()) {
        messageError(
            "Unsupported Wait shape: a wait carrying its own actions is not emitted by this printer "
            "(hif-backend#45). verilog2hif's _manageWaitActions lifts those actions out to be following "
            "siblings for every RTL process, leaving this list empty.",
            &o, _sem);
    }

    if (forms > 1) {
        return this->printMultiClauseWait(o, hasCondition, hasSensitivity, hasTime);
    }

    if (forms == 0) {
        // A wait that says nothing. Two different producers make one, and the
        // node records nothing that tells them apart:
        //
        //  - verilog2hif appends an empty Wait to the end of every non-initial
        //    process that contains a wait (_fixProcessesWithWait,
        //    FixDescription_1.cpp). It is a marker for the SystemC lowering,
        //    where the process becomes an SC_THREAD wrapped in `while (true)`
        //    and this becomes the loop-tail `wait()` that reapplies the static
        //    sensitivity. An `always` block already loops, so in Verilog the
        //    marker has no syntax of its own. It now says so: verilog2hif tags
        //    it PROPERTY_PROCESS_LOOP_TAIL (hif-backend#46).
        //
        //  - vhdl2hif emits one for VHDL `wait;`, which suspends the process
        //    permanently. That one carries no such tag.
        //
        // Neither prints a statement here, and for opposite reasons: an
        // `always` block already loops, so the marker has nothing to add, and a
        // process that suspends permanently is emitted as `initial`, where
        // reaching the end of the block is what stops it. isRetriggerable is
        // where the distinction has an effect.
        return 0;
    }

    if (hasCondition) {
        // Verilog's `wait (c)` is level-sensitive: it falls through at once
        // when c already holds. That is exactly what verilog2hif recorded from
        // a source `wait (c)`. It is *not* what vhdl2hif's `wait until c`
        // meant - VHDL suspends until an event makes the condition true, even
        // if it is true on arrival - but the two land in the same field with
        // nothing to separate them, and hif2sc collapses them the same way.
        // hif-core#16 tracks that divergence.
        (*_stream) << "wait ( ";
        o.getCondition()->acceptVisitor(*this);
        (*_stream) << " );\n";
        return 0;
    }

    if (hasTime) {
        // renderDelay rather than visiting the value: it maps both shapes a
        // timeout arrives in - verilog2hif's "n * hif_verilog_timescale_unit"
        // count and vhdl2hif's absolute TimeValue - onto a count of the unit
        // the emitted `timescale declares. Visiting a TimeValue directly
        // prints nothing at all.
        (*_stream) << "#" << this->renderDelay(o.getTime()) << ";\n";
        return 0;
    }

    this->printEventControl(o);
    return 0;
}

void VerilogPrinter::printEventControl(Wait &o)
{
    // Each signal is qualified individually, for the reason visitStateTable
    // qualifies its sensitivity individually (hif-backend#21): one leading
    // `posedge` ahead of a comma-separated list silently applies to the first
    // entry only.
    bool isFirst = true;
    auto printSensitivity = [&](hif::BList<hif::Value> &list, const std::string &edge) {
        for (auto *signal : list) {
            (*_stream) << (isFirst ? "@( " : ", ");
            isFirst = false;
            if (!edge.empty()) {
                (*_stream) << edge << " ";
            }
            signal->acceptVisitor(*this);
        }
    };
    printSensitivity(o.sensitivity, "");
    printSensitivity(o.sensitivityPos, "posedge");
    printSensitivity(o.sensitivityNeg, "negedge");
    (*_stream) << " );\n";
}

auto VerilogPrinter::printMultiClauseWait(Wait &o, bool hasCondition, bool hasSensitivity, bool hasTime) -> int
{
    // The block has to be named so that whatever resumes the process can leave
    // it. Fresh, for the same reason getSubprogramExitLabel is: a Verilog block
    // label shares its scope with the declarations around it.
    const std::string label = hif::NameTable::getInstance()->getFreshName("hif_wait");

    (*_stream) << "begin : " << label << "\n";
    _stream->indent();

    // A timeout has to be able to fire while nothing at all is happening on the
    // sensitivity list. A single sequential block cannot do that - it sits in
    // the event control and never reaches the deadline test, so the wait never
    // resumes. Measured, on the shape this issue originally proposed: with `a`
    // idle and the condition false, `wait on a until b for 10 ns` never
    // resumed. So the reasons to resume are concurrent branches, and whichever
    // happens first disables this block, which terminates the fork and its join
    // along with it.
    const bool needsFork = hasTime && (hasCondition || hasSensitivity);
    if (needsFork) {
        (*_stream) << "fork\n";
        _stream->indent();
    }

    if (hasCondition && hasSensitivity) {
        // `wait on a until c`: the condition *gates* the event rather than
        // being a second way to resume, so the event control is re-armed until
        // a wakeup finds the condition true. `forever` is already a single
        // statement, so it needs no begin/end of its own to be a fork branch.
        (*_stream) << "forever begin\n";
        _stream->indent();
        this->printEventControl(o);
        (*_stream) << "if ( ";
        o.getCondition()->acceptVisitor(*this);
        (*_stream) << " ) disable " << label << ";\n";
        _stream->unindent();
        (*_stream) << "end\n";
    } else if (hasCondition) {
        // `wait until c for T`: no sensitivity was stated, so the signals to
        // wake on are the ones the condition reads. Verilog's own `wait`
        // derives that set for itself, which is why it is used here rather than
        // an event control this printer would have to compute - and it keeps
        // this path spelling the condition exactly as the single-condition path
        // does, including its level-sensitive reading (hif-core#16). That
        // divergence is neither introduced nor widened here.
        (*_stream) << "begin\n";
        _stream->indent();
        (*_stream) << "wait ( ";
        o.getCondition()->acceptVisitor(*this);
        (*_stream) << " );\n";
        (*_stream) << "disable " << label << ";\n";
        _stream->unindent();
        (*_stream) << "end\n";
    } else {
        // `wait on a for T`: the first event on the sensitivity list resumes,
        // with no condition to satisfy, so the event control fires once.
        (*_stream) << "begin\n";
        _stream->indent();
        this->printEventControl(o);
        (*_stream) << "disable " << label << ";\n";
        _stream->unindent();
        (*_stream) << "end\n";
    }

    if (needsFork) {
        // renderDelay rather than visiting the value, for the reason the
        // timeout-only path gives: it maps both shapes a timeout arrives in
        // onto a count of the unit the emitted `timescale declares.
        (*_stream) << "begin\n";
        _stream->indent();
        (*_stream) << "#" << this->renderDelay(o.getTime()) << ";\n";
        (*_stream) << "disable " << label << ";\n";
        _stream->unindent();
        (*_stream) << "end\n";
        _stream->unindent();
        (*_stream) << "join\n";
    }

    _stream->unindent();
    (*_stream) << "end\n";
    return 0;
}

auto VerilogPrinter::visitWhen(When &o) -> int
{
    (*_stream) << "(";
    for (hif::WhenAlt *alt : o.alts) {
        (*_stream) << "(";
        alt->getCondition()->acceptVisitor(*this);
        (*_stream) << ") ? (";
        alt->getValue()->acceptVisitor(*this);
        (*_stream) << ") : ";
    }
    if (o.getDefault() != nullptr) {
        (*_stream) << "(";
        o.getDefault()->acceptVisitor(*this);
        (*_stream) << ")";
    } else {
        (*_stream) << "'bx";
    }
    (*_stream) << ")";
    return 0;
}

auto VerilogPrinter::visitWhenAlt(WhenAlt &o) -> int { return GuideVisitor::visitWhenAlt(o); }

auto VerilogPrinter::visitWhile(While &o) -> int { return hif::GuideVisitor::visitWhile(o); }

auto VerilogPrinter::visitWith(With &o) -> int
{
    (*_stream) << "(";
    for (hif::WithAlt *alt : o.alts) {
        (*_stream) << "(";
        for (std::size_t i = 0; i < alt->conditions.size(); ++i) {
            o.getCondition()->acceptVisitor(*this);
            (*_stream) << " == ";
            alt->conditions.at(i)->acceptVisitor(*this);
            if (i + 1 < alt->conditions.size()) {
                (*_stream) << " || ";
            }
        }
        (*_stream) << ") ? (";
        alt->getValue()->acceptVisitor(*this);
        (*_stream) << ") : ";
    }
    if (o.getDefault() != nullptr) {
        (*_stream) << "(";
        o.getDefault()->acceptVisitor(*this);
        (*_stream) << ")";
    } else {
        (*_stream) << "'bx";
    }
    (*_stream) << ")";
    return 0;
}

auto VerilogPrinter::visitWithAlt(WithAlt &o) -> int { return GuideVisitor::visitWithAlt(o); }

// ==============================================================================
// Private methods
// ==============================================================================

inline auto is_integer(hif::Type *type) -> bool
{
    if (auto bitvector = dynamic_cast<hif::Bitvector *>(type)) {
        if (!bitvector->isSigned()) {
            return false;
        }
        // get the span of the bitvector type.
        auto span = bitvector->getSpan();
        if (span) {
            // Get the left bound of the span.
            auto left_bound  = dynamic_cast<hif::IntValue *>(span->getLeftBound());
            // Get the right bound of the span.
            auto right_bound = dynamic_cast<hif::IntValue *>(span->getRightBound());
            // If both bounds are valid, compute the width.
            if (left_bound && right_bound) {
                return left_bound->getValue() == 31 && right_bound->getValue() == 0;
            }
        }
    }
    return false;
}

namespace
{

/// @brief Name verilog2hif gives the constant holding a file's timescale unit.
const char *const timescaleUnitName = "hif_verilog_timescale_unit";
/// @brief ... and its precision.
const char *const timescalePrecisionName = "hif_verilog_timescale_precision";

/// @brief Looks up a Time-valued constant by name.
/// @param declarations The declaration list to search.
/// @param name The constant's name.
/// @return Its TimeValue, or nullptr if there is no such constant.
auto findTimeConstant(hif::BList<hif::Declaration> &declarations, const std::string &name) -> hif::TimeValue *
{
    for (auto *declaration : declarations) {
        auto *constant = dynamic_cast<hif::Const *>(declaration);
        if (constant == nullptr || constant->getName() != name) {
            continue;
        }
        return dynamic_cast<hif::TimeValue *>(constant->getValue());
    }
    return nullptr;
}

/// @brief The Verilog spelling of a time unit.
/// @param unit The HIF time unit.
/// @return "fs", "ps", ... or an empty string for a unit Verilog cannot
/// express (minutes and hours have no `timescale spelling).
auto timeUnitToVerilog(hif::TimeValue::TimeUnit unit) -> std::string
{
    switch (unit) {
    case hif::TimeValue::time_fs:
        return "fs";
    case hif::TimeValue::time_ps:
        return "ps";
    case hif::TimeValue::time_ns:
        return "ns";
    case hif::TimeValue::time_us:
        return "us";
    case hif::TimeValue::time_ms:
        return "ms";
    case hif::TimeValue::time_sec:
        return "s";
    default:
        return "";
    }
}

/// @brief Prints a delay count without a trailing ".000000".
/// @param value The count, in timescale units.
/// @return Its shortest exact decimal spelling.
auto formatDelayCount(double value) -> std::string
{
    std::stringstream ss;
    ss << std::setprecision(17) << value;
    return ss.str();
}

/// @brief The factor of @p value that a Verilog timescale can name.
/// @details Verilog only accepts 1, 10 or 100 there, so anything else is
/// folded into the value printed at each delay instead.
/// @param value The constant's numeric value.
/// @return 1, 10 or 100.
auto timescaleFactor(double value) -> double
{
    if (value >= 100.0) {
        return 100.0;
    }
    if (value >= 10.0) {
        return 10.0;
    }
    return 1.0;
}

/// @brief Converts a TimeValue into a number of timescale units.
/// @param time The absolute time.
/// @param unit The timescale unit.
/// @param unitValue The timescale unit's factor.
/// @return How many timescale units @p time is.
auto toTimescaleUnits(const hif::TimeValue &time, hif::TimeValue::TimeUnit unit, double unitValue) -> double
{
    // changeUnit mutates, so work on a copy: the delay stays in the tree.
    auto *converted = hif::copy(&time);
    converted->changeUnit(unit);
    const double result = converted->getValue() / unitValue;
    delete converted;
    return result;
}

/// @brief The unit-scaled operand of a "<count> * hif_verilog_timescale_unit"
/// delay, which is how verilog2hif records "#<count>".
/// @param delay The HIF delay value.
/// @return The count operand, or nullptr if @p delay is not of that shape.
auto getTimescaleScaledCount(hif::Value *delay) -> hif::Value *
{
    auto *expression = dynamic_cast<hif::Expression *>(delay);
    if (expression == nullptr || expression->getOperator() != hif::op_mult) {
        return nullptr;
    }

    auto *value1 = dynamic_cast<hif::Identifier *>(expression->getValue1());
    auto *value2 = dynamic_cast<hif::Identifier *>(expression->getValue2());
    if (value1 != nullptr && value1->getName() == timescaleUnitName) {
        return expression->getValue2();
    }
    if (value2 != nullptr && value2->getName() == timescaleUnitName) {
        return expression->getValue1();
    }
    return nullptr;
}

} // namespace

void VerilogPrinter::resolveTimescale(hif::View *view)
{
    _timescale = Timescale();
    if (view == nullptr) {
        return;
    }

    // Only designs that actually carry a delay get a `timescale directive, so
    // everything else regenerates byte-for-byte as before.
    std::list<hif::Value *> delays;

    std::list<hif::Assign *> assigns;
    hif::HifTypedQuery<hif::Assign> assignQuery;
    hif::search(assigns, view, assignQuery);
    for (auto *assign : assigns) {
        if (assign->getDelay() != nullptr) {
            delays.push_back(assign->getDelay());
        }
    }

    // A timed wait - Verilog `#5;`, VHDL `wait for 10 ns;` - is a delay too,
    // and visitWait emits it as a count of the unit this directive declares
    // (hif-backend#42). Scanning only Assign delays left a design whose only
    // delay was a wait with no `timescale at all, so that count was measured
    // in whatever the simulator defaults to rather than in the source's unit.
    std::list<hif::Wait *> waits;
    hif::HifTypedQuery<hif::Wait> waitQuery;
    hif::search(waits, view, waitQuery);
    for (auto *wait : waits) {
        if (wait->getTime() != nullptr) {
            delays.push_back(wait->getTime());
        }
    }

    if (delays.empty()) {
        return;
    }

    // A delay that is an absolute time rather than a count of timescale units
    // has to be converted, and the unit it converts into must be fine enough
    // for every such delay in the file.
    std::list<hif::TimeValue *> absoluteDelays;
    for (auto *delay : delays) {
        auto *absolute = dynamic_cast<hif::TimeValue *>(delay);
        if (absolute != nullptr) {
            absoluteDelays.push_back(absolute);
        }
    }

    // verilog2hif records the source's own `timescale (or its 1ns/10ps
    // default) as two constants, on the view when the source declared one and
    // on the System when it did not.
    hif::TimeValue *unit      = findTimeConstant(view->declarations, timescaleUnitName);
    hif::TimeValue *precision = findTimeConstant(view->declarations, timescalePrecisionName);
    auto *system              = hif::getNearestParent<hif::System>(view);
    if (unit == nullptr && system != nullptr) {
        unit      = findTimeConstant(system->declarations, timescaleUnitName);
        precision = findTimeConstant(system->declarations, timescalePrecisionName);
    }

    if (unit != nullptr && !timeUnitToVerilog(unit->getUnit()).empty()) {
        _timescale.unit      = unit->getUnit();
        _timescale.unitValue = timescaleFactor(unit->getValue());
    } else if (!absoluteDelays.empty()) {
        // No usable timescale on record, so take the finest unit any absolute
        // delay in this file uses. Every delay is then a whole number of it.
        auto finest = (*absoluteDelays.begin())->getUnit();
        for (auto *absolute : absoluteDelays) {
            finest = std::min(finest, absolute->getUnit());
        }
        _timescale.unit      = finest;
        _timescale.unitValue = 1.0;
    } else {
        // A count of timescale units with no timescale to count in: there is
        // nothing to derive one from, so fall back to what verilog2hif uses
        // when a source declares none.
        _timescale.unit      = hif::TimeValue::time_ns;
        _timescale.unitValue = 1.0;
    }

    if (precision != nullptr && !timeUnitToVerilog(precision->getUnit()).empty()) {
        _timescale.precision      = precision->getUnit();
        _timescale.precisionValue = timescaleFactor(precision->getValue());
    } else {
        _timescale.precision      = _timescale.unit;
        _timescale.precisionValue = _timescale.unitValue;
    }

    _timescale.valid = true;
}

auto VerilogPrinter::isConeProcedure(hif::Procedure *procedure) -> bool
{
    if (procedure == nullptr) {
        return false;
    }
    // hif-frontend names every cone it synthesizes from a reserved stem -
    // getFreshName("hif_cone_" + the driven declaration's name) - and gives the
    // cone's StateTable the literal name "hif_cone" (generateConeFunctions,
    // FixDescription_3.cpp). A user-written task keeps the name the source gave
    // it.
    //
    // The distinction cannot be drawn from shape instead. A cone takes no
    // parameters, but so does a task declared without arguments, and both are a
    // Procedure carrying a StateTable of states. The name is the only thing
    // that records which of the two the frontend meant, and it is reserved
    // rather than incidental.
    static const std::string conePrefix("hif_cone_");
    const std::string name(procedure->getName());
    return name.rfind(conePrefix, 0) == 0;
}

auto VerilogPrinter::getSubprogramExitLabel(hif::StateTable *stateTable, const std::string &subprogramName)
    -> std::string
{
    if (stateTable == nullptr) {
        return "";
    }

    // Only the states, deliberately not the declarations: a Return belonging to
    // a subprogram nested in this one is that subprogram's business, and would
    // otherwise name a block this body has no early exit from.
    std::list<hif::Return *> returns;
    hif::HifTypedQuery<hif::Return> returnQuery;
    for (auto *state : stateTable->states) {
        hif::search(returns, state, returnQuery);
    }

    hif::Return *trailing = getTrailingReturn(stateTable);
    for (auto *ret : returns) {
        if (ret != trailing) {
            // Fresh rather than a fixed suffix: the label shares Verilog's
            // scope with the subprogram's own declarations, so a body that
            // happens to declare `<name>_body` must not collide with it.
            return hif::NameTable::getInstance()->getFreshName(subprogramName + "_body");
        }
    }
    return "";
}

auto VerilogPrinter::printTask(hif::Procedure &o) -> int
{
    auto *stateTable = o.getStateTable();
    if (stateTable == nullptr) {
        // Nothing to declare. A bodiless procedure that is not a system task
        // reaches here; visitProcedureCall drops its calls the same way.
        return 0;
    }

    (*_stream) << "task " << o.getName() << ";\n";
    _stream->indent();

    // A task's arguments are declared inside its body in Verilog, as a
    // function's are, and carry their own direction.
    bool hasDeclarations = false;
    for (auto parameter : o.parameters) {
        (*_stream) << this->getDeclaration(parameter) << ";\n";
        hasDeclarations = true;
    }
    for (auto declaration : stateTable->declarations) {
        (*_stream) << this->getDeclaration(declaration) << ";\n";
        hasDeclarations = true;
    }
    if (hasDeclarations) {
        (*_stream) << "\n";
    }

    // The body block carries a label only when something inside it exits early,
    // so a task without an early Return is emitted exactly as before.
    const std::string exitLabel = this->getSubprogramExitLabel(stateTable, o.getName());
    (*_stream) << "begin";
    if (!exitLabel.empty()) {
        (*_stream) << " : " << exitLabel;
    }
    (*_stream) << "\n";
    _stream->indent();

    // Saved and restored rather than cleared, so a subprogram nested in this
    // one cannot strand the outer body's label.
    const std::string enclosingLabel     = _subprogramExitLabel;
    hif::Return *enclosingTrailingReturn = _subprogramTrailingReturn;
    const bool enclosingInsideSubprogram = _insideSubprogramBody;
    _subprogramExitLabel                 = exitLabel;
    _subprogramTrailingReturn            = getTrailingReturn(stateTable);
    _insideSubprogramBody                = true;

    for (auto state : stateTable->states) {
        state->acceptVisitor(*this);
    }

    _subprogramExitLabel      = enclosingLabel;
    _subprogramTrailingReturn = enclosingTrailingReturn;
    _insideSubprogramBody     = enclosingInsideSubprogram;

    _stream->unindent();
    (*_stream) << "end\n";
    _stream->unindent();
    (*_stream) << "endtask\n";

    return 0;
}

auto VerilogPrinter::isUnknownLiteral(const std::string &literal) -> bool
{
    // Skip a sized-literal prefix, so that the "4" and the "b" of 4'bxxxx are
    // not mistaken for known digits.
    auto start = literal.find('\'');
    start      = (start == std::string::npos) ? 0 : start + 2;

    bool sawDigit = false;
    for (auto index = start; index < literal.size(); ++index) {
        const char character = literal[index];
        if (character == '_') {
            continue;
        }
        sawDigit = true;
        if (character != 'x' && character != 'X' && character != 'z' && character != 'Z') {
            return false;
        }
    }
    return sawDigit;
}

auto VerilogPrinter::isRetriggerable(hif::StateTable &stateTable) -> bool
{
    if (!stateTable.sensitivity.empty() || !stateTable.sensitivityPos.empty() ||
        !stateTable.sensitivityNeg.empty()) {
        return true;
    }

    // A VHDL process may carry no sensitivity list and suspend on an explicit
    // `wait` instead. That process does run repeatedly, so it stays an
    // `always`: the sensitivity lives on the Wait rather than on the
    // StateTable, and reading only the StateTable's own lists would silently
    // demote it to a run-once `initial`.
    std::list<hif::Wait *> waits;
    hif::HifTypedQuery<hif::Wait> waitQuery;
    hif::search(waits, &stateTable, waitQuery);

    bool hasResumableWait = false;
    for (hif::Wait *wait : waits) {
        // The loop-tail marker verilog2hif appends is not a suspension the
        // source wrote - it records that the process goes back round - so it
        // says nothing about whether this process can be woken.
        if (wait->checkProperty(PROPERTY_PROCESS_LOOP_TAIL)) {
            continue;
        }

        // An untagged wait with nothing set is VHDL's `wait;`: suspend and
        // never resume. A process containing one cannot loop, whatever else it
        // waits on along the way, so it is decided here rather than weighed
        // against the others. Emitting `always` for it produces a zero-delay
        // infinite loop that no source meant and that Icarus rejects at
        // elaboration (hif-backend#46) - the same failure hif-backend#40 fixed
        // for a process with no wait at all, which survived that fix because
        // this function counted every Wait as a way to be woken up, including
        // one that by definition is not.
        const bool saysNothing =
            wait->getCondition() == nullptr && wait->getTime() == nullptr && wait->getRepetitions() == nullptr &&
            wait->sensitivity.empty() && wait->sensitivityPos.empty() && wait->sensitivityNeg.empty();
        if (saysNothing) {
            return false;
        }

        hasResumableWait = true;
    }

    return hasResumableWait;
}

void VerilogPrinter::collectDelayedTargets(hif::StateTable *stateTable, std::set<std::string> &names)
{
    if (stateTable == nullptr) {
        return;
    }

    hif::TerminalPrefixOptions prefixOptions;
    prefixOptions.recurseIntoMembers   = true;
    prefixOptions.recurseIntoSlices    = true;
    prefixOptions.recurseIntoFieldRefs = true;

    std::list<hif::Assign *> assigns;
    hif::HifTypedQuery<hif::Assign> assignQuery;
    hif::search(assigns, stateTable, assignQuery);
    for (auto *assign : assigns) {
        if (assign->getDelay() == nullptr) {
            continue;
        }
        auto *target = hif::getTerminalPrefix(assign->getLeftHandSide(), prefixOptions);
        auto *identifier = dynamic_cast<hif::Identifier *>(target);
        if (identifier != nullptr) {
            names.insert(identifier->getName());
        }
    }

    // Cone procedures are expanded at their call sites (visitProcedureCall),
    // so a delayed assignment inside one lands in this process's body and
    // needs the same treatment as one written here directly.
    std::list<hif::ProcedureCall *> calls;
    hif::HifTypedQuery<hif::ProcedureCall> callQuery;
    hif::search(calls, stateTable, callQuery);
    for (auto *call : calls) {
        auto *procedure = dynamic_cast<hif::Procedure *>(hif::semantics::getDeclaration(call, _sem));
        if (procedure == nullptr || procedure->getStateTable() == nullptr) {
            continue;
        }
        if (!_inliningCones.insert(procedure).second) {
            continue;
        }
        this->collectDelayedTargets(procedure->getStateTable(), names);
        _inliningCones.erase(procedure);
    }
}

void VerilogPrinter::printTimescaleDirective()
{
    if (!_timescale.valid) {
        return;
    }
    (*_stream) << "`timescale " << formatDelayCount(_timescale.unitValue) << timeUnitToVerilog(_timescale.unit) << " / "
               << formatDelayCount(_timescale.precisionValue) << timeUnitToVerilog(_timescale.precision) << "\n\n";
}

auto VerilogPrinter::renderDelay(hif::Value *delay) -> std::string
{
    if (delay == nullptr) {
        return "";
    }

    // "#2" reaches HIF as "2 * hif_verilog_timescale_unit", and the emitted
    // `timescale re-establishes that unit, so the count goes back out as it
    // came in - including when it is a parameter rather than a literal.
    if (auto *count = getTimescaleScaledCount(delay)) {
        return this->renderToString(count);
    }

    // An absolute time - what vhdl2hif produces for "after 2 ns" - has to be
    // expressed as a count of the unit the directive declares.
    if (auto *absolute = dynamic_cast<hif::TimeValue *>(delay)) {
        return formatDelayCount(toTimescaleUnits(*absolute, _timescale.unit, _timescale.unitValue));
    }

    // Any other shape is printed as-is, parenthesised. Verilog accepts an
    // expression here, and printing what the HIF says beats dropping it.
    return "(" + this->renderToString(delay) + ")";
}

void VerilogPrinter::collectContinuouslyDrivenDeclarations(hif::View *view)
{
    _continuouslyDrivenDeclarations.clear();
    _valueDrivenPorts.clear();
    _initialValuePorts.clear();
    if (view == nullptr) {
        return;
    }
    hif::Contents *contents = view->getContents();
    if (contents == nullptr) {
        return;
    }

    hif::TerminalPrefixOptions prefixOptions;
    prefixOptions.recurseIntoMembers   = true;
    prefixOptions.recurseIntoSlices    = true;
    prefixOptions.recurseIntoFieldRefs = true;

    // ------------------------------------------------------------------
    // Driver 1: a global action, emitted as a continuous "assign".
    // ------------------------------------------------------------------
    // These are collected first because visitGlobalAction is what makes them
    // continuous drivers: if that ever stops emitting an "assign" for an
    // action, this has to stop claiming its target is a net.
    if (contents->getGlobalAction() != nullptr) {
        for (auto *action : contents->getGlobalAction()->actions) {
            auto *assign = dynamic_cast<hif::Assign *>(action);
            if (assign == nullptr) {
                continue;
            }
            auto *target = hif::getTerminalPrefix(assign->getLeftHandSide(), prefixOptions);
            if (dynamic_cast<hif::Identifier *>(target) == nullptr) {
                continue;
            }
            auto *decl = dynamic_cast<hif::DataDeclaration *>(hif::semantics::getDeclaration(target, _sem));
            if (decl != nullptr) {
                _continuouslyDrivenDeclarations.insert(decl);
            }
        }
    }

    // ------------------------------------------------------------------
    // Driver 2: a child instance's output or inout port.
    // ------------------------------------------------------------------
    // Search the whole contents rather than only contents->instances, so an
    // instance inside a generate block is covered too - it is bound to the
    // enclosing module's nets exactly the same way.
    //
    // The search also turns up the Instance nodes that carry a call's library
    // scope (the "standard" instance on a standard-library FunctionCall).
    // Those have no portAssigns, so they contribute nothing.
    std::list<hif::Instance *> instances;
    hif::HifTypedQuery<hif::Instance> query;
    hif::search(instances, contents, query);

    for (auto *instance : instances) {
        for (auto *portAssign : instance->portAssigns) {
            // An unresolvable formal is left alone: the direction is what
            // decides here, and guessing it would be worse than keeping the
            // previous emission.
            auto *formal = dynamic_cast<hif::Port *>(hif::semantics::getDeclaration(portAssign, _sem));
            if (formal == nullptr) {
                continue;
            }
            if (formal->getDirection() != PortDirection::dir_out &&
                formal->getDirection() != PortDirection::dir_inout) {
                continue;
            }
            if (portAssign->getValue() == nullptr) {
                continue;
            }
            // The actual may be a bit-select or part-select of the net rather
            // than the net itself; it is still that net that gets driven.
            auto *actual = hif::getTerminalPrefix(portAssign->getValue(), prefixOptions);
            if (dynamic_cast<hif::Identifier *>(actual) == nullptr) {
                continue;
            }
            auto *decl = dynamic_cast<hif::DataDeclaration *>(hif::semantics::getDeclaration(actual, _sem));
            if (decl != nullptr) {
                _continuouslyDrivenDeclarations.insert(decl);
            }
        }
    }

    // ------------------------------------------------------------------
    // Driver 3: an output port's own initial value (hif-backend#30).
    // ------------------------------------------------------------------
    // verilog2hif folds a constant continuous assignment into the value of the
    // port it drives, and Verilog-2001 does not accept an initializer inside
    // an ANSI port list, so the only way that assignment survives is to be
    // written back out as the continuous assignment it came from.
    //
    // Strictly gated on the port having no driver of its own, and that gate is
    // load-bearing rather than defensive: vhdl2hif gives EVERY out port a
    // value - the 'U' default - whether the source wrote one or not. Without
    // this check every VHDL output would get a second continuous driver on top
    // of the one it already has, which resolves to x. The check has to cover
    // procedural drivers too, not just the two continuous kinds above: a port
    // a process writes is a reg, and a continuous assign cannot drive a reg.
    if (view->getEntity() == nullptr) {
        return;
    }

    std::set<hif::DataDeclaration *> assignedAnywhere;
    std::list<hif::Assign *> assigns;
    hif::HifTypedQuery<hif::Assign> assignQuery;
    hif::search(assigns, contents, assignQuery);
    for (auto *assign : assigns) {
        auto *target = hif::getTerminalPrefix(assign->getLeftHandSide(), prefixOptions);
        if (dynamic_cast<hif::Identifier *>(target) == nullptr) {
            continue;
        }
        auto *decl = dynamic_cast<hif::DataDeclaration *>(hif::semantics::getDeclaration(target, _sem));
        if (decl != nullptr) {
            assignedAnywhere.insert(decl);
        }
    }

    for (auto *port : view->getEntity()->ports) {
        // Outputs only. An inout with a value must not be permanently driven:
        // that would defeat the direction it was declared with.
        if (port->getDirection() != PortDirection::dir_out || port->getValue() == nullptr) {
            continue;
        }
        // Tell a value the source actually wrote from one a frontend supplied
        // for a port that stated none. Both frontends supply one: vhdl2hif
        // gives every out port the 'U' default, which has no Verilog literal
        // and renders empty, and verilog2hif gives a reg port an all-x value.
        // Neither says anything a declaration does not already say - an
        // uninitialized reg and an undriven net both read x - and emitting the
        // first would produce "assign q = ;".
        const std::string rendered = this->renderToString(port->getValue());
        if (rendered.empty() || isUnknownLiteral(rendered)) {
            continue;
        }
        if (_continuouslyDrivenDeclarations.count(port) != 0) {
            // Something already drives it continuously, so it is a net and its
            // value is not ours to restate.
            continue;
        }
        if (assignedAnywhere.count(port) != 0) {
            // Driven procedurally, so the port is a reg. A continuous assign
            // would be a second driver on it; the value is what it holds until
            // that process first writes it, which is an `initial` assignment
            // (hif-backend#36). Deliberately does NOT mark the port as
            // continuously driven: it stays a reg.
            _initialValuePorts.push_back(port);
            continue;
        }
        _valueDrivenPorts.push_back(port);
        _continuouslyDrivenDeclarations.insert(port);
    }
}

auto VerilogPrinter::isContinuouslyDriven(hif::Declaration *declaration) -> bool
{
    auto *dataDeclaration = dynamic_cast<hif::DataDeclaration *>(declaration);
    return dataDeclaration != nullptr && _continuouslyDrivenDeclarations.count(dataDeclaration) != 0;
}

std::string VerilogPrinter::getDeclaration(hif::Declaration *declaration)
{
    std::stringstream ss;
    // The timescale constants are verilog2hif's record of the source's
    // `timescale, not declarations the design made. Verilog spells that as a
    // directive, which printTimescaleDirective now emits, so printing them
    // here would state the timescale twice - and state it invalidly: their
    // value is a Time, which has no Verilog literal, so they came out as
    // "localparam hif_verilog_timescale_unit;" and Icarus rejected the file
    // ("localparam must have a value"). Reparsing failed on them too, so every
    // design carrying an explicit `timescale regenerated unusable (part of
    // hif-backend#24: emitting a delay means owning how its unit is written).
    if (declaration != nullptr &&
        (declaration->getName() == timescaleUnitName || declaration->getName() == timescalePrecisionName)) {
        return "";
    }

    if (auto variable = dynamic_cast<hif::Variable *>(declaration)) {
        if (is_integer(variable->getType())) {
            ss << "integer ";
        } else {
            ss << "reg ";
            ss << this->getBitwidth(variable->getType());
        }
        ss << variable->getName();
        auto value = this->getValue(variable->getValue());
        if (!value.empty()) {
            ss << " = " << value;
        }
    } else if (auto signal = dynamic_cast<hif::Signal *>(declaration)) {
        // A continuously driven net must be a wire; everything else is written
        // by a process, which needs a reg (hif-backend#26, #32).
        ss << (this->isContinuouslyDriven(declaration) ? "wire " : "reg ");
        ss << this->getBitwidth(signal->getType());
        ss << signal->getName();
        auto value = this->getValue(signal->getValue());
        if (!value.empty()) {
            ss << " = " << value;
        }
    } else if (auto port = dynamic_cast<hif::Port *>(declaration)) {
        switch (port->getDirection()) {
        case PortDirection::dir_in:
            ss << "input wire ";
            break;
        case PortDirection::dir_out:
            // Same rule as for signals: a continuously driven output is a
            // wire, one a process drives is a reg (hif-backend#26, #32).
            ss << (this->isContinuouslyDriven(declaration) ? "output wire " : "output reg ");
            break;
        case PortDirection::dir_inout:
            ss << "inout ";
            break;
        default:
            messageError("Unexpected PortDirection", declaration, _sem);
        }
        ss << this->getBitwidth(port->getType());
        ss << port->getName();
    } else if (auto parameter = dynamic_cast<hif::Parameter *>(declaration)) {
        // A task's arguments carry a direction, exactly as a module's ports do,
        // and Verilog spells all three. Only dir_in used to be handled, so an
        // `out` or `inout` parameter - VHDL's ordinary way for a procedure to
        // drive its actual - rendered as the empty string. printTask writes the
        // terminator unconditionally, so that surfaced as a bare `;` and the
        // task referenced a name it never declared (hif-backend#64).
        switch (parameter->getDirection()) {
        case PortDirection::dir_in:
            ss << "input ";
            break;
        case PortDirection::dir_out:
            ss << "output ";
            break;
        case PortDirection::dir_inout:
            ss << "inout ";
            break;
        default:
            messageError("Unexpected PortDirection", declaration, _sem);
        }
        ss << this->getBitwidth(parameter->getType());
        ss << parameter->getName();
    } else if (auto valueTP = dynamic_cast<hif::ValueTP *>(declaration)) {
        // Module-level generic/template parameter (e.g. `parameter WIDTH = 8`).
        // Printed as a plain (non-ANSI) body declaration, matching how
        // view->templateParameters is already fed through this same
        // getDeclaration()/printList() path elsewhere in this file.
        ss << "parameter ";
        ss << valueTP->getName();
        auto value = this->getValue(valueTP->getValue());
        if (!value.empty()) {
            ss << " = " << value;
        }
    } else if (auto constDecl = dynamic_cast<hif::Const *>(declaration)) {
        // Verilog `localparam` (e.g. state-machine state encodings). Was
        // missing entirely: this function returned an empty string for
        // Const, which printList() then silently dropped - no declaration,
        // no separator, no diagnostic - while code elsewhere kept
        // referencing the (now undeclared) name.
        ss << "localparam ";
        ss << constDecl->getName();
        auto value = this->getValue(constDecl->getValue());
        if (!value.empty()) {
            ss << " = " << value;
        }
    }
    // else {
    //     messageError("Unexpected Declaration", declaration, _sem);
    // }
    return ss.str();
}

std::string VerilogPrinter::getBitwidth(hif::Type *type)
{
    std::stringstream ss;
    unsigned long long bw = hif::semantics::typeGetSpanBitwidth(type, _sem);
    if (bw == 1) {
        return ss.str();
    }
    if (bw == 0) {
        // Bitwidth could not be statically resolved (e.g. an unresolved,
        // top-level parametric port with no instantiation providing a
        // concrete value - see fixtures/parametric_port_width.v). Falling
        // through to `bw - 1` here would wrap around in unsigned
        // arithmetic and print a nonsensical literal like
        // [18446744073709551615:0]. Print the original symbolic range
        // bounds instead - both correct and valid Verilog.
        auto *typeSpan = dynamic_cast<hif::features::ITypeSpan *>(type);
        hif::Range *span = typeSpan != nullptr ? typeSpan->getSpan() : nullptr;
        if (span != nullptr && span->getLeftBound() != nullptr && span->getRightBound() != nullptr) {
            ss << "[" << this->getSymbolicValue(span->getLeftBound()) << ":"
               << this->getSymbolicValue(span->getRightBound()) << "] ";
        }
        return ss.str();
    }
    ss << "[" << (bw - 1) << ":0] ";
    return ss.str();
}

std::string VerilogPrinter::getSymbolicValue(hif::Value *value)
{
    // Print an arbitrary sub-expression to a string by temporarily
    // redirecting this visitor's output stream into a local buffer -
    // getValue() only handles constant-like values, not general
    // Expression trees (e.g. the "WIDTH - 1" this function exists for).
    std::stringbuf buf;
    hif::backends::IndentedStream localStream(&buf);
    hif::backends::IndentedStream *saved = _stream;
    _stream                              = &localStream;
    value->acceptVisitor(*this);
    _stream = saved;
    return buf.str();
}

namespace
{

/// @brief Whether @p digits is spelled entirely with digits Verilog defines.
/// @details HIF stores bit values in IEEE 1164 form, whose alphabet is
/// U/X/0/1/Z/W/L/H/-. Verilog's binary literal syntax has only 0, 1, x and z,
/// so a value containing any of the others has no Verilog spelling at all and
/// must not be emitted as though it had one.
auto isVerilogBinaryLiteral(const std::string &digits) -> bool
{
    for (char character : digits) {
        if (character == '_') {
            continue;
        }
        const char lowered = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        if (lowered != '0' && lowered != '1' && lowered != 'x' && lowered != 'z') {
            return false;
        }
    }
    return true;
}

} // namespace

std::string VerilogPrinter::getValue(hif::Value *value)
{
    std::stringstream ss;
    if (auto bit_value = dynamic_cast<BitValue *>(value)) {
        // A one-bit value is emitted as a sized literal, `1'b<d>`, not as a
        // bare digit. Verilog gives an unsized `1` a self-determined 32-bit
        // width, which a concatenation rejects outright - "Concatenation
        // operand has indefinite width" - so `{1'b1, 1'b0}` regenerated as
        // `{1, 0}` and did not elaborate (hif-backend#61). Sizing it is also
        // simply more exact everywhere else this function is used: `1'b1` is
        // legal wherever a bare `1` was.
        //
        // Only for a digit Verilog defines. HIF stores bits in IEEE 1164 form
        // (U/X/0/1/Z/W/L/H/-) and Verilog has only 0/1/x/z; the rest have no
        // spelling and must render empty, exactly as the BitvectorValue branch
        // below does. That emptiness is load-bearing: it is what keeps
        // vhdl2hif's implicit 'U' port default out of the output
        // (hif-backend#36/#55). An x/z bit now renders, but isUnknownLiteral
        // still classifies `1'bx` as unknown, so verilog2hif's all-x reg
        // default stays suppressed too.
        const std::string bit_digit = bit_value->toString();
        if (bit_digit.size() == 1 && isVerilogBinaryLiteral(bit_digit)) {
            ss << "1'b" << static_cast<char>(std::tolower(static_cast<unsigned char>(bit_digit[0])));
        }
    } else if (auto int_value = dynamic_cast<hif::IntValue *>(value)) {
        ss << int_value->getValue();
    } else if (auto bitvector_value = dynamic_cast<BitvectorValue *>(value)) {
        if (bitvector_value->is01()) {
            if (is_integer(bitvector_value->getType())) {
                ss << bitvector_value->getValueAsSigned();
            } else {
                // Cast the type to Bitvector to get the width.
                auto bitvector_type = dynamic_cast<hif::Bitvector *>(bitvector_value->getType());
                // If the type is valid, get the width.
                if (bitvector_type) {
                    // get the span of the bitvector type.
                    auto span = bitvector_type->getSpan();
                    if (span) {
                        // Get the left bound of the span.
                        auto left_bound  = dynamic_cast<hif::IntValue *>(span->getLeftBound());
                        // Get the right bound of the span.
                        auto right_bound = dynamic_cast<hif::IntValue *>(span->getRightBound());
                        // If both bounds are valid, compute the width.
                        if (left_bound && right_bound) {
                            auto width = left_bound->getValue() - right_bound->getValue() + 1;
                            ss << width << "'b";
                        }
                    }
                    ss << bitvector_value->getValue();
                }
            }
        } else if (auto *bitvector_type = dynamic_cast<hif::Bitvector *>(bitvector_value->getType())) {
            // Four-state value (contains X/Z, e.g. an all-Z {N{1'bz}}
            // replication fill value) - the is01() branch above only
            // handles clean 0/1 values. Verilog literal syntax accepts
            // 0/1/x/z digits; HIF stores them uppercase (IEEE 1164
            // style), so lowercase them on the way out.
            //
            // Only when every digit is one Verilog actually has. IEEE 1164
            // also defines U/W/L/H/-, and vhdl2hif gives every signal and
            // port the 'U' default whether the source stated one or not, so
            // this branch was spelling that default 4'buuuu - not a Verilog
            // literal, and the file no longer parsed (hif-backend#55).
            // Rendering nothing instead is what the BitValue branch above
            // already does for a non-0/1 bit, and it is why a scalar port's
            // implicit 'U' has always been left out correctly: an empty
            // result makes collectContinuouslyDrivenDeclarations skip the
            // port, so no initializer is emitted at all. Same rule, same
            // outcome, now regardless of width.
            std::string raw = bitvector_value->getValue();
            if (isVerilogBinaryLiteral(raw)) {
                auto span = bitvector_type->getSpan();
                if (span) {
                    auto left_bound  = dynamic_cast<hif::IntValue *>(span->getLeftBound());
                    auto right_bound = dynamic_cast<hif::IntValue *>(span->getRightBound());
                    if (left_bound && right_bound) {
                        auto width = left_bound->getValue() - right_bound->getValue() + 1;
                        ss << width << "'b";
                    }
                }
                for (char c : raw) {
                    ss << static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
            }
        }
    } else if (auto identifier = dynamic_cast<hif::Identifier *>(value)) {
        if (!identifier->getName().empty()) {
            ss << identifier->getName();
        }
    } else if (auto member = dynamic_cast<hif::Member *>(value)) {
        // Get the member prefix.
        auto prefix = member->getPrefix();
        // Get the index.
        auto index  = member->getIndex();
        // If both the prefix and the index are valid, build the string.
        if (prefix && index) {
            // Visit the prefix.
            ss << this->getValue(prefix);
            // Add the access operator.
            ss << "[";
            // Visit the index.
            ss << this->getValue(index);
            // Close the access operator.
            ss << "]";
        }
    } else if (auto slice = dynamic_cast<hif::Slice *>(value)) {
        // Get the prefix.
        auto prefix = slice->getPrefix();
        // Get span.
        auto span   = slice->getSpan();
        // If both the prefix and the span are valid, build the string.
        if (prefix && span) {
            // Visit the prefix.
            ss << this->getValue(prefix);
            // Add the access operator.
            ss << "[";
            // Visit the span.
            ss << this->getValue(span);
            // Close the access operator.
            ss << "]";
        }
    } else if (auto range = dynamic_cast<hif::Range *>(value)) {
        // Get the left bound.
        auto left_bound  = range->getLeftBound();
        // Get the right bound.
        auto right_bound = range->getRightBound();
        // If both the left and right bounds are valid, build the string.
        if (left_bound && right_bound) {
            // Visit the left bound.
            ss << this->getValue(left_bound);
            // Add the range operator.
            ss << ":";
            // Visit the right bound.
            ss << this->getValue(right_bound);
        }
    } else if (auto expression = dynamic_cast<hif::Expression *>(value)) {
        // Capture the expression rather than letting it write straight to
        // _stream. Streaming it here emitted the expression at whatever
        // point the enclosing construct had reached, ahead of the string
        // being assembled - so a slice with expression bounds came out as
        // "DEPTH * WIDTH - 1(DEPTH - 1) * WIDTHchain[:]", every piece
        // present, in the wrong order, with empty brackets (hif-backend#18).
        ss << this->renderToString(expression);
    } else if (auto cast = dynamic_cast<hif::Cast *>(value)) {
        ss << this->getValue(cast->getValue());
    } else if (value != nullptr) {
        // Terminal case. Without it, a value whose kind is absent from the
        // chain above returned the empty string, and every caller of this
        // function renders a position where nothing is not a legal spelling:
        // an if/else-if condition, a case selector, a case label, a for
        // condition. A FunctionCall took that path, so `if rising_edge(clk)`
        // - the most common sequential VHDL idiom there is - regenerated as
        // `if (  )` at exit code 0, which no simulator parses (hif-backend#50).
        //
        // Delegating to the full printer is what the Expression branch above
        // already does; the two now agree instead of the same construct
        // rendering differently depending on whether something wrapped it.
        //
        // This is reachable only for kinds absent from the chain above, which
        // is why the BitValue branch had to join that chain rather than stand
        // as its own `if`. While it stood apart, every BitValue fell through
        // to here, and the delegation below re-entered visitBitValue, which
        // calls this function again: unbounded recursion, SIGSEGV on any
        // design containing a bit literal.
        //
        // Deliberately not a hard failure when the printer renders nothing
        // either. An empty result is legitimate for some callers - a port or
        // signal whose initial value is an all-'Z'/'X' Aggregate reaches here,
        // and rendering it as nothing is what correctly leaves the initializer
        // out of an ANSI port list, which Verilog-2001 has no place for. A
        // value kind the printer cannot spell is a gap in the printer, not
        // something this function can decide to abort on.
        ss << this->renderToString(value);
    }
    return ss.str();
}

std::string VerilogPrinter::renderToString(hif::Object *object)
{
    if (object == nullptr) {
        return {};
    }
    std::stringbuf buffer;
    hif::backends::IndentedStream captured(&buffer);

    hif::backends::IndentedStream *previous = _stream;
    _stream                                 = &captured;
    object->acceptVisitor(*this);
    _stream = previous;

    return buffer.str();
}

std::string VerilogPrinter::getPortAssign(hif::PortAssign *port_assign)
{
    std::stringstream ss;
    if (auto identifier = dynamic_cast<hif::Identifier *>(port_assign->getValue())) {
        ss << "." << port_assign->getName() << "(" << identifier->getName() << ")";
    } else {
        ss << "." << port_assign->getName() << "(";
        port_assign->getValue()->acceptVisitor(*this);
        ss << ")";
    }
    return ss.str();
}

void VerilogPrinter::printList(
    const hif::BList<hif::Object> &list,
    const std::string &separator,
    bool print_new_line,
    bool print_last_separator)
{
    if (list.empty()) {
        return;
    }
    if (!_stream) {
        return;
    }
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (auto declaration = dynamic_cast<hif::DataDeclaration *>(list.at(i))) {
            // Get the declaration string.
            auto declaration_string = this->getDeclaration(declaration);
            // If the string is not empty, print it.
            if (!declaration_string.empty()) {
                (*_stream) << declaration_string;
            } else {
                continue;
            }
        } else if (auto value = dynamic_cast<hif::Value *>(list.at(i))) {
            value->acceptVisitor(*this);
        } else if (dynamic_cast<hif::Function *>(list.at(i))) {
            // Skip functions, print them at the beginning after the variables
            // declaration in the module.
            continue;
        } else if (auto port_assign = dynamic_cast<hif::PortAssign *>(list.at(i))) {
            // Get the port assign as a string.
            auto port_assign_string = this->getPortAssign(port_assign);
            // If the string is not empty, print it.
            if (!port_assign_string.empty()) {
                (*_stream) << port_assign_string;
            } else {
                continue;
            }
        } else if (dynamic_cast<hif::Procedure *>(list.at(i))) {
            // Skip procedures for the same reason as functions above: they are
            // printed by printSubprograms, in the section after the variable
            // declarations. Visiting one here used to be harmless because
            // visitProcedure printed nothing at all - a cone's body belongs at
            // its call sites - but a user-written task does print, and printing
            // it here as well declared it twice (hif-backend#38).
            continue;
        }
        if (i < (list.size() - 1)) {
            if (!separator.empty()) {
                (*_stream) << separator;
            }
            if (print_new_line) {
                (*_stream) << "\n";
            } else {
                (*_stream) << " ";
            }
        } else {
            if (!separator.empty() && print_last_separator) {
                (*_stream) << separator;
            }
            if (print_new_line) {
                (*_stream) << "\n";
            }
        }
    }
}

bool VerilogPrinter::printSubprograms(const hif::BList<hif::Object> &list)
{
    bool printed = false;
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (auto function = dynamic_cast<hif::Function *>(list.at(i))) {
            function->acceptVisitor(*this);
            printed = true;
            continue;
        }
        // Procedures too, since a user-written task is one and has to be
        // declared before it can be called (hif-backend#38). A cone is a
        // Procedure as well, but visitProcedure prints nothing for it: its body
        // belongs at its call sites, not here.
        if (auto procedure = dynamic_cast<hif::Procedure *>(list.at(i))) {
            if (isConeProcedure(procedure)) {
                continue;
            }
            procedure->acceptVisitor(*this);
            printed = true;
        }
    }
    return printed;
}
