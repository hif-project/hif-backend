/// @file PostRefine_process.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/hif.hpp>

#include "hif2verilog/PostRefineMethods.hpp"

using namespace hif;

namespace
{ // anon.namespace

/// @brief Names hif-core gives the VHDL edge-detection idioms. `'event` becomes
/// hif_vhdl_event (HIFSemantics_methods.cpp), and the two IEEE 1164 helpers keep
/// their own names. analyzeProcesses recognises exactly this set when it works
/// out a process's working edge, so the two stay in step.
auto isEdgeFunctionName(const std::string &name) -> bool
{
    return name == "hif_vhdl_event" || name == "hif_vhdl_rising_edge" || name == "hif_vhdl_falling_edge";
}

/// @brief Collects every edge-detection call under an object.
class EdgeCallCollector : public hif::GuideVisitor
{
public:
    EdgeCallCollector() = default;

    auto visitFunctionCall(FunctionCall &o) -> int override
    {
        if (isEdgeFunctionName(o.getName())) {
            calls.push_back(&o);
        }
        return GuideVisitor::visitFunctionCall(o);
    }

    std::vector<FunctionCall *> calls;
};

auto collectEdgeCalls(Object *root) -> std::vector<FunctionCall *>
{
    EdgeCallCollector c;
    root->acceptVisitor(c);
    return c.calls;
}

/// @brief The alternatives of ifStm other than the clocked one - in the shapes
/// this pass accepts, that is the asynchronous reset branch.
auto altForReset(If *ifStm, IfAlt *edgeAlt) -> BList<Action> &
{
    static BList<Action> empty;
    for (IfAlt *alt : ifStm->alts) {
        if (alt != edgeAlt) {
            return alt->actions;
        }
    }
    return empty;
}

/// @brief True if the value reads anything that can change between two
/// activations of the same process - a signal, a port or a variable.
///
/// Constants, generics and literals are settled by the time the design runs, so
/// re-evaluating an expression over them yields the same result and re-running
/// the assignment is a no-op.
auto readsMutableState(Value *value, hif::semantics::ILanguageSemantics *sem) -> bool
{
    if (value == nullptr) {
        return false;
    }
    hif::semantics::ReferencesMap refs;
    hif::semantics::getAllReferences(refs, sem, value);
    for (auto &entry : refs) {
        Declaration *decl = entry.first;
        if (dynamic_cast<Signal *>(decl) != nullptr || dynamic_cast<Port *>(decl) != nullptr ||
            dynamic_cast<Variable *>(decl) != nullptr) {
            return true;
        }
    }
    return false;
}

/// @brief Rebuilds Verilog edge sensitivity for VHDL-style clocked processes.
class PostRefine_process : public hif::GuideVisitor
{
public:
    explicit PostRefine_process(semantics::ILanguageSemantics *sem);
    ~PostRefine_process() override;

    auto visitStateTable(StateTable &o) -> int override;

private:
    PostRefine_process(const PostRefine_process &)                     = delete;
    auto operator=(const PostRefine_process &) -> PostRefine_process & = delete;

    /// @brief Appends a reference to decl to the given sensitivity list.
    void _addSensitivity(BList<Value> &list, DataDeclaration *decl);

    /// @brief Drops decl from the level-sensitivity list, if present.
    /// @return True if it was there.
    auto _removeSensitivity(BList<Value> &list, DataDeclaration *decl) -> bool;

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _factory;
};

PostRefine_process::PostRefine_process(semantics::ILanguageSemantics *sem)
    : _sem(sem)
    , _factory(sem)
{
}

PostRefine_process::~PostRefine_process() = default;

void PostRefine_process::_addSensitivity(BList<Value> &list, DataDeclaration *decl)
{
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(new Identifier(decl->getName()), list, addOpt);
}

auto PostRefine_process::_removeSensitivity(BList<Value> &list, DataDeclaration *decl) -> bool
{
    for (BList<Value>::iterator it = list.begin(); it != list.end();) {
        auto *id = dynamic_cast<Identifier *>(*it);
        if (id != nullptr && id->getName() == decl->getName()) {
            it = it.erase();
            return true;
        }
        ++it;
    }
    return false;
}

auto PostRefine_process::visitStateTable(StateTable &o) -> int
{
    GuideVisitor::visitStateTable(o);

    if (!hif::objectIsProcess(&o)) {
        return 0;
    }

    // Nothing to do for a process that never tests an edge. Checked before
    // anything else so that ordinary combinational processes - the majority -
    // cost one traversal and no analysis.
    const std::vector<FunctionCall *> edgeCalls(collectEdgeCalls(&o));
    if (edgeCalls.empty()) {
        return 0;
    }

    // Already edge-qualified. A Verilog-derived process arrives this way and
    // needs nothing; only the VHDL shape, which carries its edge as a
    // condition, is rewritten here. Reaching this point means the process has
    // both, which no frontend produces and which this pass would half-rewrite:
    // say so rather than leave the call to be printed verbatim.
    messageAssert(
        o.sensitivityPos.empty() && o.sensitivityNeg.empty(),
        "This process is already edge-qualified and *also* tests a clock edge in its body. That "
        "shape cannot be rebuilt without deciding which of the two is authoritative. See "
        "hif-backend#51.",
        &o, _sem);

    // analyzeProcesses returns true unconditionally, so its result carries no
    // information and is deliberately not checked. What the classification is
    // worth is checked below instead.
    analysis::AnalyzeProcessOptions::ProcessMap map;
    hif::analysis::analyzeProcesses(&o, map, _sem);
    analysis::ProcessInfos &infos = map[&o];

    const bool isSynchronous =
        (infos.processKind == analysis::ProcessInfos::SYNCHRONOUS ||
         infos.processKind == analysis::ProcessInfos::DERIVED_SYNCHRONOUS);
    const bool hasSingleEdge =
        (infos.workingEdge == analysis::ProcessInfos::RISING_EDGE ||
         infos.workingEdge == analysis::ProcessInfos::FALLING_EDGE);

    // Refuse loudly rather than emit a call to a function that does not exist.
    // Everything below assumes one clock, one edge and one place where that
    // edge is tested; a shape that does not match is left to a maintainer
    // rather than silently mistranslated (hif-backend#51).
    messageAssert(
        isSynchronous && hasSingleEdge && infos.clock != nullptr && edgeCalls.size() == 1,
        "This process tests a clock edge in a form hif2verilog cannot rebuild as Verilog edge "
        "sensitivity. Verilog has no way to spell the test where it stands, and emitting it "
        "verbatim would produce a call to an undeclared function. See hif-backend#51.",
        &o, _sem);

    FunctionCall *edgeCall = edgeCalls.front();

    // The edge test must be the condition of an if alternative, which is what
    // both VHDL spellings produce. Anywhere else - inside a wider expression,
    // or in a loop condition - is a shape this pass does not claim to handle.
    auto *edgeAlt = hif::getNearestParent<IfAlt>(edgeCall);
    messageAssert(
        edgeAlt != nullptr && hif::isSubNode(edgeCall, edgeAlt->getCondition()),
        "The clock-edge test is not the condition of an if alternative, so it cannot be moved into "
        "the sensitivity list. See hif-backend#51.",
        edgeCall, _sem);

    auto *ifStm = dynamic_cast<If *>(edgeAlt->getParent());
    messageAssert(ifStm != nullptr, "Malformed if alternative.", edgeAlt, _sem);

    // That if has to be the whole process body. A VHDL process runs top to
    // bottom every time any signal in its sensitivity list moves, so a
    // statement sitting outside the clocked if runs on *both* clock edges.
    // Moving the process to `always @(posedge clk)` would silently restrict it
    // to one - a real behavioural change, and an invisible one, since the
    // result still compiles and still looks like the source. Verilog has no
    // way to express "this part on any edge, that part on the rising one" in a
    // single always block, so this shape is refused rather than approximated.
    messageAssert(
        ifStm->getParent() == o.states.front() && o.states.front()->actions.size() == 1,
        "This process has statements outside the clock-edge test. They run on every edge of the "
        "clock in VHDL, and rebuilding the process as edge-sensitive would silently restrict them "
        "to one edge. See hif-backend#51.",
        &o, _sem);

    // The clocked branch has to be the last one, and there must be no existing
    // else: it is about to *become* the else. In the VHDL shapes that reach
    // here - `if reset ... elsif clk'event ...` and a bare `if clk'event` -
    // it always is.
    messageAssert(
        edgeAlt == ifStm->alts.back() && ifStm->defaults.empty(),
        "The clock-edge branch is not the last alternative, or the statement already has an else, "
        "so dropping the edge test would change which branch runs. See hif-backend#51.",
        ifStm, _sem);

    // Move the edge from the condition into the sensitivity list.
    if (infos.workingEdge == analysis::ProcessInfos::RISING_EDGE) {
        _addSensitivity(o.sensitivityPos, infos.clock);
    } else {
        _addSensitivity(o.sensitivityNeg, infos.clock);
    }
    _removeSensitivity(o.sensitivity, infos.clock);

    // An asynchronous reset is edge-sensitive in Verilog too: the process must
    // wake on the reset's *assertion*, so its active level maps to an edge. A
    // synchronous reset is deliberately left where it is - it is tested inside
    // the clocked branch and must stay there.
    if (infos.resetKind == analysis::ProcessInfos::ASYNCHRONOUS_RESET && infos.reset != nullptr) {
        // The reset branch has to be idempotent, and this is the one place
        // where the two models genuinely differ rather than merely look
        // different. A VHDL process is sensitive to its clock's *level*, so
        // while the reset is asserted the reset branch re-runs on the
        // clock's inactive edge as well. `always @(posedge clk, posedge rst)`
        // does not. Assigning a constant, which is what a reset almost always
        // does, makes that unobservable - re-running it changes nothing. A
        // reset branch that *reads* something re-samples it on the inactive
        // edge in VHDL and would not in Verilog, so the two designs diverge
        // with no diagnostic anywhere.
        for (Action *action : altForReset(ifStm, edgeAlt)) {
            auto *assign = dynamic_cast<Assign *>(action);
            messageAssert(
                assign != nullptr,
                "The asynchronous reset branch does more than assign. VHDL re-runs that branch on "
                "the clock's inactive edge while reset is held, which the rebuilt Verilog cannot "
                "do, so anything with an effect beyond a settled value would diverge. See "
                "hif-backend#51.",
                action, _sem);
            messageAssert(
                !readsMutableState(assign->getRightHandSide(), _sem),
                "The asynchronous reset branch reads a signal or variable rather than assigning a "
                "settled value. VHDL re-samples it on the clock's inactive edge while reset is "
                "held; the rebuilt Verilog samples it only on the reset assertion and the active "
                "clock edge, so the two would silently disagree. See hif-backend#51.",
                assign, _sem);
        }

        if (infos.resetPhase == analysis::ProcessInfos::LOW_PHASE) {
            _addSensitivity(o.sensitivityNeg, infos.reset);
        } else {
            _addSensitivity(o.sensitivityPos, infos.reset);
        }
        _removeSensitivity(o.sensitivity, infos.reset);
    }

    // Verilog does allow an event expression to mix levels and edges, and the
    // printer emits all three lists, so this is not a syntax limitation. The
    // problem is which event caused the activation: once the edge test is
    // dropped from the body, the clocked branch is the unconditional else, so
    // waking on an unrelated level change would run the clocked body without a
    // clock edge. Refuse rather than build that.
    messageAssert(
        o.sensitivity.empty(),
        "A clocked process is sensitive to something besides its clock and asynchronous reset. "
        "Once the edge test moves out of the body, an event on that signal would run the clocked "
        "branch without a clock edge. See hif-backend#51.",
        &o, _sem);

    // Drop the edge test: the clocked branch becomes the else.
    ifStm->defaults.merge(edgeAlt->actions);
    BList<IfAlt>::iterator altIt(edgeAlt);
    altIt.erase();

    // With no alternatives left the if is just its else - a bare `if clk'event`
    // with no reset. Splice the actions in place of the statement so that the
    // process body is the clocked body itself.
    if (ifStm->alts.empty()) {
        BList<Action>::iterator stmIt(ifStm);
        for (BList<Action>::iterator it = ifStm->defaults.begin(); it != ifStm->defaults.end();) {
            Action *action = *it;
            it             = it.remove();
            stmIt.insert_before(action);
        }
        stmIt.erase();
    }

    return 0;
}

} // namespace

void fixSynchronousProcesses(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    PostRefine_process v(sem);
    o->acceptVisitor(v);
}
