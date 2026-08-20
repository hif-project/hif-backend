/// @file VerilogPrinter.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <fstream>

#include <hif/hif.hpp>

class VerilogPrinter : public hif::GuideVisitor
{
public:
    using ViewSet = std::set<hif::View *>;
    using ViewMap = std::map<hif::View *, ViewSet>;

    VerilogPrinter(hif::backends::IndentedStream *stream);
    ~VerilogPrinter() override;

    VerilogPrinter(const VerilogPrinter &)                     = delete;
    auto operator=(const VerilogPrinter &) -> VerilogPrinter & = delete;

    auto visitAggregate(hif::Aggregate &o) -> int override;
    auto visitAggregateAlt(hif::AggregateAlt &o) -> int override;
    auto visitAlias(hif::Alias &o) -> int override;
    auto visitArray(hif::Array &o) -> int override;
    auto visitAssign(hif::Assign &o) -> int override;
    auto visitBit(hif::Bit &o) -> int override;
    auto visitBitValue(hif::BitValue &o) -> int override;
    auto visitBitvector(hif::Bitvector &o) -> int override;
    auto visitBitvectorValue(hif::BitvectorValue &o) -> int override;
    auto visitBool(hif::Bool &o) -> int override;
    auto visitBoolValue(hif::BoolValue &o) -> int override;
    auto visitBreak(hif::Break &o) -> int override;
    auto visitCast(hif::Cast &o) -> int override;
    auto visitChar(hif::Char &o) -> int override;
    auto visitCharValue(hif::CharValue &o) -> int override;
    auto visitConst(hif::Const &o) -> int override;
    auto visitContents(hif::Contents &o) -> int override;
    auto visitContinue(hif::Continue &o) -> int override;
    auto visitDesignUnit(hif::DesignUnit &o) -> int override;
    auto visitEnum(hif::Enum &o) -> int override;
    auto visitEnumValue(hif::EnumValue &o) -> int override;
    auto visitEvent(hif::Event &o) -> int override;
    auto visitExpression(hif::Expression &o) -> int override;
    auto visitFunctionCall(hif::FunctionCall &o) -> int override;
    auto visitField(hif::Field &o) -> int override;
    auto visitFieldReference(hif::FieldReference &o) -> int override;
    auto visitFile(hif::File &o) -> int override;
    auto visitFor(hif::For &o) -> int override;
    auto visitForGenerate(hif::ForGenerate &o) -> int override;
    auto visitFunction(hif::Function &o) -> int override;
    auto visitGlobalAction(hif::GlobalAction &o) -> int override;
    auto visitEntity(hif::Entity &o) -> int override;
    auto visitIdentifier(hif::Identifier &o) -> int override;
    auto visitIf(hif::If &o) -> int override;
    auto visitIfAlt(hif::IfAlt &o) -> int override;
    auto visitIfGenerate(hif::IfGenerate &o) -> int override;
    auto visitInstance(hif::Instance &o) -> int override;
    auto visitInt(hif::Int &o) -> int override;
    auto visitIntValue(hif::IntValue &o) -> int override;
    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitLibrary(hif::Library &o) -> int override;
    auto visitMember(hif::Member &o) -> int override;
    auto visitNull(hif::Null &o) -> int override;
    auto visitTransition(hif::Transition &o) -> int override;
    auto visitParameterAssign(hif::ParameterAssign &o) -> int override;
    auto visitParameter(hif::Parameter &o) -> int override;
    auto visitProcedureCall(hif::ProcedureCall &o) -> int override;
    auto visitPointer(hif::Pointer &o) -> int override;
    auto visitPortAssign(hif::PortAssign &o) -> int override;
    auto visitPort(hif::Port &o) -> int override;
    auto visitProcedure(hif::Procedure &o) -> int override;
    auto visitRange(hif::Range &o) -> int override;
    auto visitReal(hif::Real &o) -> int override;
    auto visitRealValue(hif::RealValue &o) -> int override;
    auto visitRecord(hif::Record &o) -> int override;
    auto visitRecordValue(hif::RecordValue &o) -> int override;
    auto visitRecordValueAlt(hif::RecordValueAlt &o) -> int override;
    auto visitReference(hif::Reference &o) -> int override;
    auto visitReturn(hif::Return &o) -> int override;
    auto visitSignal(hif::Signal &o) -> int override;
    auto visitSigned(hif::Signed &o) -> int override;
    auto visitSlice(hif::Slice &o) -> int override;
    auto visitState(hif::State &o) -> int override;
    auto visitString(hif::String &o) -> int override;
    auto visitStateTable(hif::StateTable &o) -> int override;
    auto visitSystem(hif::System &o) -> int override;
    auto visitSwitchAlt(hif::SwitchAlt &o) -> int override;
    auto visitSwitch(hif::Switch &o) -> int override;
    auto visitStringValue(hif::StringValue &o) -> int override;
    auto visitTime(hif::Time &o) -> int override;
    auto visitTimeValue(hif::TimeValue &o) -> int override;
    auto visitTypeDef(hif::TypeDef &o) -> int override;
    auto visitTypeReference(hif::TypeReference &o) -> int override;
    auto visitTypeTPAssign(hif::TypeTPAssign &o) -> int override;
    auto visitTypeTP(hif::TypeTP &o) -> int override;
    auto visitUnsigned(hif::Unsigned &o) -> int override;
    auto visitValueStatement(hif::ValueStatement &o) -> int override;
    auto visitValueTPAssign(hif::ValueTPAssign &o) -> int override;
    auto visitValueTP(hif::ValueTP &o) -> int override;
    auto visitVariable(hif::Variable &o) -> int override;
    auto visitView(hif::View &o) -> int override;
    auto visitViewReference(hif::ViewReference &o) -> int override;
    auto visitWait(hif::Wait &o) -> int override;
    auto visitWhen(hif::When &o) -> int override;
    auto visitWhenAlt(hif::WhenAlt &o) -> int override;
    auto visitWhile(hif::While &o) -> int override;
    auto visitWith(hif::With &o) -> int override;
    auto visitWithAlt(hif::WithAlt &o) -> int override;

private:
    /// @brief The reference semantics.
    hif::semantics::ILanguageSemantics *_sem;
    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_stream;
    /// @brief Cone procedures currently being inlined, innermost last.
    /// @details Cone procedures are expanded at their call sites (see
    /// visitProcedureCall), and one cone may call another. The frontend
    /// emits that call graph as a DAG, so this only guards against a
    /// malformed input looping the printer forever - it is not expected
    /// to trip in practice.
    std::set<hif::Procedure *> _inliningCones;
    /// @brief Declarations that a *continuous* driver writes, in the design
    /// unit currently being printed.
    /// @details Verilog requires these to be nets. A `reg` is written by
    /// procedural statements only, so anything that drives continuously needs
    /// a net on the other end, and declaring one as `reg` produces Verilog no
    /// simulator accepts. Repopulated per design unit.
    ///
    /// Two constructs qualify, and they are the same rule rather than two:
    ///   - a net bound to a module instance's out/inout port (hif-backend#26);
    ///   - the target of a continuous `assign`, which is how a view's
    ///     GlobalAction is emitted (hif-backend#32);
    ///   - an output port whose initial value is re-emitted as a continuous
    ///     assign because nothing else drives it (hif-backend#30).
    /// #26 saw only the first because a GlobalAction was not printed at all,
    /// so a continuous assign was not yet something hif2verilog could produce.
    std::set<hif::DataDeclaration *> _continuouslyDrivenDeclarations;

    /// @brief Output ports whose initial value has to be re-emitted as a
    /// continuous assignment, in declaration order.
    /// @details The frontend folds a constant continuous assignment into the
    /// driven port's value, and a port declaration cannot carry an initializer
    /// in a Verilog-2001 ANSI port list, so the assignment has to be written
    /// back out as one (hif-backend#30). Only ports that nothing else drives
    /// qualify - see collectContinuouslyDrivenDeclarations.
    std::list<hif::Port *> _valueDrivenPorts;

    /// @brief Output ports whose initial value has to be re-emitted as an
    /// `initial` assignment, in declaration order.
    /// @details The complement of _valueDrivenPorts: these ports state an
    /// initial value *and* are written by a process, so they are regs and a
    /// continuous assign would be a second driver on them (hif-backend#36).
    /// The value is what the port holds until that process first writes it.
    std::list<hif::Port *> _initialValuePorts;

    /// @brief Name of the Function whose body is currently being printed, or
    /// empty when the printer is not inside one.
    /// @details Verilog has no `return`: a function yields its value by
    /// assigning to its own name, so emitting a Return needs to know that name
    /// (hif-backend#57). Empty means the Return belongs to a Procedure, which
    /// is a control-flow exit with no Verilog-2001 equivalent - see
    /// hif-backend#63.
    std::string _currentFunctionName;

    /// @brief Label of the named block wrapping the subprogram body currently
    /// being printed, or empty when that block is unnamed.
    /// @details Verilog-2001 leaves a subprogram early with `disable` on a named
    /// block, so an early Return needs a label to name (hif-backend#63). Bodies
    /// with no early Return are left unnamed, so nothing that used to be emitted
    /// changes shape.
    std::string _subprogramExitLabel;

    /// @brief The Return that leaves the current subprogram body by falling off
    /// its end, and therefore needs no explicit exit. Null when there is none.
    hif::Return *_subprogramTrailingReturn{nullptr};

    /// @brief Whether the printer is inside a subprogram body at all.
    /// @details Distinguishes a Return this printer is responsible for from one
    /// reached outside any subprogram, which has no block to disable.
    bool _insideSubprogramBody{false};

    /// @brief The `timescale a file's `#N` delays are expressed in.
    /// @details Verilog delays are plain numbers scaled by the enclosing
    /// file's `timescale, so a delay cannot be emitted without also
    /// establishing the unit it counts in (hif-backend#24).
    struct Timescale {
        double unitValue{1.0};                                          ///< 1, 10 or 100.
        hif::TimeValue::TimeUnit unit{hif::TimeValue::time_ns};         ///< The unit itself.
        double precisionValue{1.0};                                     ///< 1, 10 or 100.
        hif::TimeValue::TimeUnit precision{hif::TimeValue::time_ns};    ///< The unit itself.
        bool valid{false};                                              ///< False when nothing needs one.
    };

    /// @brief The timescale of the design unit currently being printed.
    Timescale _timescale;

    /// @brief Fills _continuouslyDrivenDeclarations and _valueDrivenPorts for
    /// @p view.
    /// @details Takes the view rather than its contents because the last of
    /// the three continuous drivers is a property of a *port*, and ports live
    /// on the entity while everything that could drive them lives in the
    /// contents. Deciding one needs both.
    /// @param view The view being printed.
    void collectContinuouslyDrivenDeclarations(hif::View *view);

    /// @brief Resolves _timescale for the design unit rooted at @p view.
    /// @details Left invalid when the design unit carries no delay, so a
    /// design without delays regenerates exactly as before.
    /// @param view The view being printed.
    void resolveTimescale(hif::View *view);

    /// @brief Prints the `timescale directive for _timescale, if it is valid.
    void printTimescaleDirective();

    /// @brief Names of the targets of delayed assignments reachable from a
    /// process, including through the cone procedures it calls.
    /// @details A delayed assignment schedules its target for later, so the
    /// process has to be re-triggered when that target actually changes or the
    /// reads that follow keep the pre-delay value forever (hif-backend#24).
    /// @param stateTable The process to scan.
    /// @param names Receives the target names.
    void collectDelayedTargets(hif::StateTable *stateTable, std::set<std::string> &names);

    /// @brief Whether a Procedure is a frontend-synthesized logic cone rather
    /// than a user-written Verilog task.
    /// @details Both are a Procedure carrying a StateTable, and a cone's lack of
    /// parameters does not separate them because a task may be declared without
    /// arguments either. hif-frontend records which it meant in the name: a cone
    /// is named from the reserved stem "hif_cone_" (hif-backend#38).
    /// @param procedure The declaration to classify.
    /// @return True if the procedure is a cone, whose body is inlined at its
    /// call sites.
    static auto isConeProcedure(hif::Procedure *procedure) -> bool;

    /// @brief Prints a user-written Verilog task declaration.
    /// @param o The procedure to print as a task.
    /// @return 0.
    auto printTask(hif::Procedure &o) -> int;

    /// @brief Prints a VHDL wait that sets more than one of condition,
    /// sensitivity and timeout.
    /// @details Verilog has no single statement meaning more than one at once,
    /// so the clauses are lowered into a named block whose concurrent branches
    /// each disable it (hif-backend#45).
    /// @param o The wait to lower.
    /// @param hasCondition Whether the wait states a condition.
    /// @param hasSensitivity Whether the wait states any sensitivity.
    /// @param hasTime Whether the wait states a timeout.
    /// @return 0.
    auto printMultiClauseWait(hif::Wait &o, bool hasCondition, bool hasSensitivity, bool hasTime) -> int;

    /// @brief Prints a wait's sensitivity lists as a Verilog event control.
    /// @param o The wait whose sensitivity to print.
    void printEventControl(hif::Wait &o);

    /// @brief The label the body block of @p stateTable has to carry, if any.
    /// @details Empty unless the body holds a Return that needs an explicit
    /// exit, i.e. one that is not the trailing one. Fresh, so it cannot collide
    /// with a declaration the subprogram already has.
    /// @param stateTable The subprogram's state table.
    /// @param subprogramName The subprogram's name, used to build the label.
    /// @return The label, or the empty string when the body needs none.
    auto getSubprogramExitLabel(hif::StateTable *stateTable, const std::string &subprogramName) -> std::string;

    /// @brief Whether a rendered Verilog literal is entirely unknown.
    /// @details Both frontends give a port that stated no initial value one
    /// anyway - 'U' from vhdl2hif, all-x from verilog2hif - and an all-unknown
    /// value says nothing a declaration does not already say, since an
    /// uninitialized reg and an undriven net both read x (hif-backend#36).
    /// @param literal The rendered literal, which may carry a size prefix.
    /// @return True if every digit is x or z.
    static auto isUnknownLiteral(const std::string &literal) -> bool;

    /// @brief Whether a process can be woken up again after it finishes.
    /// @details A process is re-triggerable if it has a sensitivity list, or if
    /// it suspends on a `wait`. One with neither runs exactly once, which is an
    /// `initial` block rather than an `always` one (hif-backend#40).
    /// @param stateTable The process to classify.
    /// @return True if the process can run more than once.
    static auto isRetriggerable(hif::StateTable &stateTable) -> bool;

    /// @brief Renders an assignment delay as a Verilog delay expression.
    /// @param delay The HIF delay value.
    /// @return The text to print after '#', in _timescale units.
    auto renderDelay(hif::Value *delay) -> std::string;

    /// @brief Whether @p declaration must be emitted as a net rather than a
    /// variable, because something drives it continuously.
    /// @param declaration The declaration being printed.
    /// @return True if it is bound to an instance's out/inout port, or is the
    /// target of a global action emitted as a continuous assign.
    auto isContinuouslyDriven(hif::Declaration *declaration) -> bool;

    std::string getDeclaration(hif::Declaration *declaration);

    std::string getBitwidth(hif::Type *type);

    std::string getSymbolicValue(hif::Value *value);

    std::string getValue(hif::Value *value);

    /// @brief Render an object through the visitor, capturing its output
    /// instead of writing it to the current stream.
    /// @details getValue() assembles a string, so any branch of it that
    /// needs the visitor must not let the visitor write to _stream: doing
    /// so emits the nested text at the point the *enclosing* construct has
    /// reached, ahead of the string being assembled.
    /// @param object The object to render.
    /// @return The text the visitor produced for \p object.
    std::string renderToString(hif::Object *object);

    /// @brief Whether a narrowing cast needs no explicit truncation because
    /// the assignment it sits in already truncates to the same width.
    /// @param o The narrowing cast.
    /// @param targetWidth The width \p o casts to.
    /// @return True if \p o is the whole right-hand side of an assignment
    /// whose left-hand side is exactly \p targetWidth bits wide.
    auto isTruncatedByAssignmentContext(hif::Cast &o, unsigned long long targetWidth) -> bool;

    std::string getPortAssign(hif::PortAssign *port_assign);

    /// @brief Print a list of objects.
    /// @param list The list to be printed.
    /// @param separator The separator among elements of \p list.
    /// @param print_new_line If true, a new line is added at the end of each.
    /// @param print_last_separator When print_new_line is true, if true, the separator is printed also after the last element.
    void printList(
        const hif::BList<hif::Object> &list,
        const std::string &separator,
        bool print_new_line       = false,
        bool print_last_separator = false);

    /// @brief Calls the visitor on any Function or user-written task inside the
    /// list.
    /// @details Cone Procedures are skipped: their bodies belong at their call
    /// sites rather than in a declaration (hif-backend#38).
    /// @param list The list of objects to be printed.
    /// @return true if anything was printed, false otherwise.
    bool printSubprograms(const hif::BList<hif::Object> &list);
};
