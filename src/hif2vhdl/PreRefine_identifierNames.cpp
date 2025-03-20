/// @file PreRefine_identifierNames.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/hif.hpp>

#include "hif2vhdl/PreRefineMethods.hpp"

using namespace hif;
using std::string;

namespace
{ // anon.namespace

class PreRefine_identifierNames : public hif::GuideVisitor
{
public:
    typedef std::set<hif::Object *> References;

    typedef std::map<hif::Declaration *, References> DeclarationsMap;

    /// @brief Default constructor and destructor.
    PreRefine_identifierNames(hif::System *system, semantics::ILanguageSemantics *sem);
    virtual ~PreRefine_identifierNames();

    virtual bool BeforeVisit(hif::Object &o);
    virtual int AfterVisit(hif::Object &o);

private:
    PreRefine_identifierNames(const PreRefine_identifierNames &);
    PreRefine_identifierNames &operator=(const PreRefine_identifierNames &);

    hif::HifFactory _factory;

    hif::semantics::ILanguageSemantics *_sem;

    DeclarationsMap _declarationsMap;

    std::string _analyzeString(const std::string &n);

    std::string _replaceFirstUnderscore(std::string nameStr);

    std::string _replaceDoubleUnderscore(std::string nameStr);
};

PreRefine_identifierNames::PreRefine_identifierNames(hif::System *system, semantics::ILanguageSemantics *sem)
    : _factory(sem)
    , _sem(sem)
    , _declarationsMap()
{
    hif::semantics::GetReferencesOptions opt;
    opt.includeUnreferenced = true;
    hif::semantics::getAllReferences(_declarationsMap, _sem, system, opt);
}

PreRefine_identifierNames::~PreRefine_identifierNames() {}

bool PreRefine_identifierNames::BeforeVisit(Object &o)
{
    LibraryDef *ld = dynamic_cast<LibraryDef *>(&o);
    DesignUnit *du = dynamic_cast<DesignUnit *>(&o);

    if (ld != nullptr && ld->isStandard())
        return true;
    if (du != nullptr && du->views.front()->isStandard())
        return true;

    return false;
}

int PreRefine_identifierNames::AfterVisit(Object &o)
{
    features::ISymbol *symb = dynamic_cast<features::ISymbol *>(&o);

    if (symb != nullptr) {
        Declaration *d = hif::semantics::getDeclaration(&o, _sem);

        messageAssert(
            d != nullptr || (dynamic_cast<Instance *>(&o) != nullptr &&
                             dynamic_cast<Library *>(static_cast<Instance *>(&o)->getReferencedType()) != nullptr),
            "Declaration not found", &o, _sem);
        if (d == nullptr)
            return 0;

        LibraryDef *ld = dynamic_cast<LibraryDef *>(d);
        View *view     = dynamic_cast<View *>(d);

        if (ld != nullptr && ld->isStandard())
            return 0;
        if (view != nullptr && view->isStandard())
            return 0;

        ld   = getNearestParent<LibraryDef>(d);
        view = getNearestParent<View>(d);

        if (ld != nullptr && ld->isStandard())
            return 0;
        if (view != nullptr && view->isStandard())
            return 0;
    }
    auto nm = objectGetName(&o);

    if (nm.empty())
        return 0;

    objectSetName(&o, _analyzeString(nm));

    if (dynamic_cast<ViewReference *>(&o) != nullptr) {
        ViewReference *vr = static_cast<ViewReference *>(&o);
        vr->setDesignUnit(_analyzeString(vr->getDesignUnit()));
    } else if (dynamic_cast<Declaration *>(&o) != nullptr) {
        Declaration *decl = static_cast<Declaration *>(&o);

        if (decl->getName() != nm) {
            for (References::iterator it = _declarationsMap[decl].begin(); it != _declarationsMap[decl].end(); ++it) {
                hif::objectSetName(*it, decl->getName());
            }
        }
    }
    return 0;
}

std::string PreRefine_identifierNames::_analyzeString(const std::string &n)
{
    std::string nameStr = _replaceFirstUnderscore(n);

    return _replaceDoubleUnderscore(nameStr);
}

string PreRefine_identifierNames::_replaceFirstUnderscore(string nameStr)
{
    if (!nameStr.empty()) {
        if (nameStr.at(0) == '_') {
            return "id" + nameStr;
        }
    }
    return nameStr;
}

string PreRefine_identifierNames::_replaceDoubleUnderscore(string nameStr)
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
