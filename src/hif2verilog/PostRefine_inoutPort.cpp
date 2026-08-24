/// @file PostRefine_inoutPort.cpp
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

/// @brief Gives a procedurally driven `inout` port a reg to be driven through.
class PostRefine_inoutPort : public hif::GuideVisitor
{
public:
    explicit PostRefine_inoutPort(semantics::ILanguageSemantics *sem);
    ~PostRefine_inoutPort() override;

    auto visitView(View &o) -> int override;

private:
    PostRefine_inoutPort(const PostRefine_inoutPort &)                     = delete;
    auto operator=(const PostRefine_inoutPort &) -> PostRefine_inoutPort & = delete;

    /// @brief Collects the assignments inside a process or subprogram whose
    /// target is @p port. A global action's assignment is deliberately not one
    /// of them: it is already a continuous driver, and a net is the right
    /// declaration for it.
    void collectProceduralAssigns(Contents *contents, Port *port, std::list<Assign *> &result);

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _factory;
    hif::TerminalPrefixOptions _prefixOptions;
};

PostRefine_inoutPort::PostRefine_inoutPort(semantics::ILanguageSemantics *sem)
    : _sem(sem)
    , _factory(sem)
    , _prefixOptions()
{
    // A bit-select or a part-select of the port is still a drive of the port.
    _prefixOptions.recurseIntoMembers   = true;
    _prefixOptions.recurseIntoSlices    = true;
    _prefixOptions.recurseIntoFieldRefs = true;
}

PostRefine_inoutPort::~PostRefine_inoutPort() = default;

void PostRefine_inoutPort::collectProceduralAssigns(Contents *contents, Port *port, std::list<Assign *> &result)
{
    std::list<Assign *> assigns;
    hif::HifTypedQuery<Assign> query;
    hif::search(assigns, contents, query);

    for (auto *assign : assigns) {
        // Inside a process or a subprogram body, as opposed to a global
        // action. A global action has no StateTable ancestor, which is the
        // whole distinction being drawn here: it is emitted as a continuous
        // "assign", and that may legally drive a net.
        if (hif::getNearestParent<StateTable>(assign) == nullptr) {
            continue;
        }
        auto *target = hif::getTerminalPrefix(assign->getLeftHandSide(), _prefixOptions);
        if (dynamic_cast<Identifier *>(target) == nullptr) {
            continue;
        }
        if (hif::semantics::getDeclaration(target, _sem) == port) {
            result.push_back(assign);
        }
    }
}

auto PostRefine_inoutPort::visitView(View &o) -> int
{
    GuideVisitor::visitView(o);

    Entity *entity     = o.getEntity();
    Contents *contents = o.getContents();
    if (entity == nullptr || contents == nullptr) {
        return 0;
    }

    for (auto *port : entity->ports) {
        if (port->getDirection() != PortDirection::dir_inout) {
            continue;
        }

        std::list<Assign *> proceduralAssigns;
        this->collectProceduralAssigns(contents, port, proceduralAssigns);
        if (proceduralAssigns.empty()) {
            // Nothing drives it procedurally, so a net is already right and
            // the port is left exactly as it was.
            continue;
        }

        // The reg the process will drive instead. Named from the port so the
        // regenerated source stays readable, and made fresh so a design that
        // already declares that name is not silently captured.
        const std::string driverName =
            hif::NameTable::getInstance()->getFreshName((std::string(port->getName()) + "_drv").c_str());

        auto *driver = new Signal();
        driver->setName(driverName);
        driver->setType(hif::copy(port->getType()));
        contents->declarations.push_back(driver);

        // Only the assignment *targets* move. Reads of the port stay on the
        // port, and that is the point of the direction: what a process reads
        // from an inout is the resolved value on the net, including whatever
        // an external driver is contributing, not the value this module last
        // drove.
        for (auto *assign : proceduralAssigns) {
            auto *target = dynamic_cast<Identifier *>(hif::getTerminalPrefix(assign->getLeftHandSide(), _prefixOptions));
            target->setName(driverName);
        }

        // The continuous driver. Unconditional, with no separate enable: in
        // VHDL the high-impedance state is a *value* of the resolved type, so
        // a process releases the net by assigning 'Z' and that reaches here as
        // the ordinary value 1'bz. An enable signal would have to be derived
        // from the same information and would model nothing extra.
        if (contents->getGlobalAction() == nullptr) {
            contents->setGlobalAction(new GlobalAction());
        }
        contents->getGlobalAction()->actions.push_back(
            _factory.assignment(new Identifier(port->getName()), new Identifier(driverName)));
    }

    // The retargeted identifiers used to resolve to the port and now resolve
    // to the driver, so anything that cached the old answer has to forget it.
    hif::semantics::resetDeclarations(&o);

    return 0;
}

} // namespace

void lowerProcedurallyDrivenInoutPorts(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    PostRefine_inoutPort v(sem);
    o->acceptVisitor(v);
}
