/// @file PreRefine_identifierNames.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/hif.hpp>

#include "hif2vhdl/PreRefineMethods.hpp"

using namespace hif;

namespace
{ // anon.namespace

class PreRefine_identifierNames : public hif::GuideVisitor
{
public:
    using References = std::set<hif::Object *>;

    using DeclarationsMap = std::map<hif::Declaration *, References>;

    /// @brief Default constructor and destructor.
    PreRefine_identifierNames(hif::System *system, semantics::ILanguageSemantics *sem);
    ~PreRefine_identifierNames() override;

    auto BeforeVisit(hif::Object &o) -> bool override;
    auto AfterVisit(hif::Object &o) -> int override;

private:
    PreRefine_identifierNames(const PreRefine_identifierNames &)                     = delete;
    auto operator=(const PreRefine_identifierNames &) -> PreRefine_identifierNames & = delete;

    hif::HifFactory _factory;

    hif::semantics::ILanguageSemantics *_sem;

    DeclarationsMap _declarationsMap;

    static auto _analyzeString(const std::string &n) -> std::string;

    static auto _replaceFirstUnderscore(const std::string &nameStr) -> std::string;

    static auto _replaceDoubleUnderscore(std::string nameStr) -> std::string;
};

PreRefine_identifierNames::PreRefine_identifierNames(hif::System *system, semantics::ILanguageSemantics *sem)
    : _factory(sem)
    , _sem(sem)
    , _declarationsMap()
{
    hif::semantics::GetReferencesOptions opt;
    opt.include_unreferenced = true;
    hif::semantics::getAllReferences(_declarationsMap, _sem, system, opt);
}

PreRefine_identifierNames::~PreRefine_identifierNames() = default;

auto PreRefine_identifierNames::BeforeVisit(Object &o) -> bool
{
    auto *ld = dynamic_cast<LibraryDef *>(&o);
    auto *du = dynamic_cast<DesignUnit *>(&o);

    if (ld != nullptr && ld->isStandard()) {
        return true;
    }
    if (du != nullptr && du->views.front()->isStandard()) {
        return true;
    }

    return false;
}

auto PreRefine_identifierNames::AfterVisit(Object &o) -> int
{
    auto *symb = dynamic_cast<features::ISymbol *>(&o);

    if (symb != nullptr) {
        Declaration *d = hif::semantics::getDeclaration(&o, _sem);

        messageAssert(
            d != nullptr || (dynamic_cast<Instance *>(&o) != nullptr &&
                             dynamic_cast<Library *>(dynamic_cast<Instance *>(&o)->getReferencedType()) != nullptr),
            "Declaration not found", &o, _sem);
        if (d == nullptr) {
            return 0;
        }

        auto *ld   = dynamic_cast<LibraryDef *>(d);
        View *view = dynamic_cast<View *>(d);

        if (ld != nullptr && ld->isStandard()) {
            return 0;
        }
        if (view != nullptr && view->isStandard()) {
            return 0;
        }

        ld   = getNearestParent<LibraryDef>(d);
        view = getNearestParent<View>(d);

        if (ld != nullptr && ld->isStandard()) {
            return 0;
        }
        if (view != nullptr && view->isStandard()) {
            return 0;
        }
    }
    auto nm = objectGetName(&o);

    if (nm.empty()) {
        return 0;
    }

    objectSetName(&o, _analyzeString(nm));

    if (dynamic_cast<ViewReference *>(&o) != nullptr) {
        auto *vr = dynamic_cast<ViewReference *>(&o);
        vr->setDesignUnit(_analyzeString(vr->getDesignUnit()));
    } else if (dynamic_cast<Declaration *>(&o) != nullptr) {
        auto *decl = dynamic_cast<Declaration *>(&o);

        if (decl->getName() != nm) {
            for (auto it = _declarationsMap[decl].begin(); it != _declarationsMap[decl].end(); ++it) {
                hif::objectSetName(*it, decl->getName());
            }
        }
    }
    return 0;
}

auto PreRefine_identifierNames::_analyzeString(const std::string &n) -> std::string
{
    return _replaceDoubleUnderscore(_replaceFirstUnderscore(n));
}

auto PreRefine_identifierNames::_replaceFirstUnderscore(const std::string &nameStr) -> std::string
{
    if (!nameStr.empty()) {
        if (nameStr.at(0) == '_') {
            return "id" + nameStr;
        }
    }
    return nameStr;
}

auto PreRefine_identifierNames::_replaceDoubleUnderscore(std::string nameStr) -> std::string
{
    if (!nameStr.empty()) {
        std::size_t found = nameStr.find("__");
        while (found != std::string::npos) {
            nameStr.insert(++found, "id");
            found = nameStr.find("__");
        }
    }
    return nameStr;
}

} // namespace

void fixIdentifierNames(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2VHDL", "fixIdentifierNames");

    PreRefine_identifierNames identifierNames(o, sem);
    o->acceptVisitor(identifierNames);

    hif::application_utils::restoreLogHeader();
}
