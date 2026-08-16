/// @file PostRefine_forIndex.cpp
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

/// @brief Moves a For's own index declaration out of the loop header.
class PostRefine_forIndex : public hif::GuideVisitor
{
public:
    explicit PostRefine_forIndex(semantics::ILanguageSemantics *sem);
    ~PostRefine_forIndex() override;

    auto visitFor(For &o) -> int override;

private:
    PostRefine_forIndex(const PostRefine_forIndex &)                     = delete;
    auto operator=(const PostRefine_forIndex &) -> PostRefine_forIndex & = delete;

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _factory;
};

PostRefine_forIndex::PostRefine_forIndex(semantics::ILanguageSemantics *sem)
    : _sem(sem)
    , _factory(sem)
{
}

PostRefine_forIndex::~PostRefine_forIndex() = default;

auto PostRefine_forIndex::visitFor(For &o) -> int
{
    GuideVisitor::visitFor(o);

    if (o.initDeclarations.empty()) {
        return 0;
    }

    // The declaration has to end up somewhere the printer emits. It prints the
    // enclosing process's declaration list - hoisting it to the module body,
    // since Verilog has no place for declarations inside an unnamed block - so
    // that is where these go. Anything else would have to invent a named block
    // to hold them.
    auto *stateTable = hif::getNearestParent<StateTable>(&o);
    messageAssert(
        stateTable != nullptr,
        "A for loop declaring its own index is not inside a process, so there is no declaration "
        "list to move the index to. See hif-backend#47.",
        &o, _sem);

    for (BList<DataDeclaration>::iterator it = o.initDeclarations.begin(); it != o.initDeclarations.end();) {
        DataDeclaration *decl = *it;
        it                    = it.remove();

        // The index carries its starting value on the declaration, which is
        // where HIF puts it and where Verilog cannot: `for (integer i = 1; ...)`
        // is SystemVerilog. Detach it and make it the loop's init assignment,
        // so the loop still starts where the source said. Without this the
        // index would be declared but never initialised - a distinct defect
        // from not declaring it, and just as silent.
        if (decl->getValue() != nullptr) {
            o.initValues.push_back(
                _factory.assignment(new Identifier(decl->getName()), decl->setValue(nullptr)));
        }

        stateTable->declarations.push_back(decl);
    }

    return 0;
}

} // namespace

void hoistForInitDeclarations(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    PostRefine_forIndex v(sem);
    o->acceptVisitor(v);
}
