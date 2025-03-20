/// @file VhdlPreRefine_misc.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/hif.hpp>

#include "hif2vhdl/PreRefineMethods.hpp"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wunknown-attributes"
#elif defined __GNUC__
#pragma GCC diagnostic ignored "-Wattributes"
#endif

using namespace hif;

namespace
{ // anon.namespace

class PreRefine_misc : public hif::GuideVisitor
{
public:
    typedef std::set<Object *> RefSet;
    typedef std::map<Declaration *, RefSet> RefMap;

    /// @brief Default constructor and destructor.
    PreRefine_misc(System *system, semantics::ILanguageSemantics *sem);
    virtual ~PreRefine_misc();

    int visitArray(Array &o);
    int visitFor(For &o);
    //int visitForGenerate(ForGenerate &o);
    int visitStateTable(StateTable &o);
    int visitSystem(System &o);
    int visitWhen(When &o);

    void addRequiredLibraries();

private:
    Value *_buildAndExpression(StateTable *o, analysis::ProcessInfos &infos, const bool skipReset);
    Value *_buildSensitivityCondition(
        StateTable *o,
        Value *expr,
        hif::analysis::ProcessInfos::ReferredDeclarations &refs,
        DataDeclaration *clock,
        DataDeclaration *skipReset,
        const bool isRising);
    void _moveSensitivities(StateTable *o);
    void _fixSynchronousProcess(StateTable *o, hif::analysis::ProcessInfos &infos);

    typedef std::set<Object *> RefsSet;
    typedef std::map<Declaration *, RefsSet> RefsMap;
    typedef std::set<Declaration *> DeclsSet;
    void _keepSignals(RefsMap &map, DeclsSet &declSet);

    PreRefine_misc(const PreRefine_misc &);
    PreRefine_misc &operator=(const PreRefine_misc &);

    hif::HifFactory _factory;

    hif::semantics::ILanguageSemantics *_sem;

    RefMap _refMap;
    System *_system;
    bool _addVhdlStandardLib;
    hif::Trash _trash;
};

PreRefine_misc::PreRefine_misc(System *system, semantics::ILanguageSemantics *sem)
    : _factory(sem)
    , _sem(sem)
    , _refMap()
    , _system(system)
    , _addVhdlStandardLib(false)
    , _trash()
{
    hif::semantics::getAllReferences(_refMap, _sem, system);
}

PreRefine_misc::~PreRefine_misc() { _trash.clear(); }

int PreRefine_misc::visitArray(Array &o)
{
    GuideVisitor::visitArray(o);

    // VHDL allows array only as opaque typedefs.

    // Skipping when is already in a typedef.
    if (dynamic_cast<TypeDef *>(o.getParent()) != nullptr) {
        TypeDef *td = static_cast<TypeDef *>(o.getParent());
        td->setOpaque(true); // ensuring opaqueness
        // @TODO: check since maybe now some ops are no more allowed...
        return 0;
    }
    // Skipping when is not the type of a data declaration
    DataDeclaration *decl = dynamic_cast<DataDeclaration *>(o.getParent());
    if (decl == nullptr)
        return 0;

    Scope *s = hif::getNearestScope(&o, true, false, false);
    if (dynamic_cast<View *>(s) != nullptr) {
        View *v = static_cast<View *>(s);
        s       = hif::getNearestScope(v->getParent(), true, false, false);
    }
    messageAssert(s != nullptr, "Cannot find suitable scope", &o, _sem);

    auto tdName = hif::NameTable::getInstance()->getFreshName("array_type");

    hif::semantics::updateDeclarations(&o, _sem);
    TypeDef *td = new TypeDef();
    td->setName(tdName);
    td->setType(hif::copy(&o));
    td->setOpaque(true);

    // Ensuring correct scoping of internal symbols
    hif::manipulation::SimplifyOptions opt;
    opt.simplify_constants      = true;
    opt.simplify_typereferences = true;
    opt.context                 = s;
    hif::manipulation::simplify(td->getType(), _sem, opt);

    BList<Declaration> *decls = hif::objectGetDeclarationList(s);
    decls->push_front(td);

    TypeReference *tr = new TypeReference();
    tr->setName(tdName);

    o.replace(tr);
    _refMap[td].insert(tr);

    for (RefSet::iterator i = _refMap[decl].begin(); i != _refMap[decl].end(); ++i) {
        if (dynamic_cast<PortAssign *>(*i) != nullptr) {
            PortAssign *pa = static_cast<PortAssign *>(*i);
            Cast *c        = new Cast();
            c->setType(hif::copy(tr));
            c->setValue(pa->setValue(nullptr));
            pa->setValue(c);
            _refMap[td].insert(c->getType());

            continue;
        }
        Value *obj = dynamic_cast<Value *>(*i);
        messageAssert(obj != nullptr, "Not suitable reference.", *i, _sem);
        if (hif::manipulation::isInLeftHandSide(obj)) {
            Assign *ass = getNearestParent<Assign>(obj);
            messageAssert(ass != nullptr, "Cannot found nearest assign", obj, _sem);
            // Skipping targets as slice and member since no cast should be required.
            if (obj != ass->getLeftHandSide())
                continue;

            Cast *c = new Cast();
            c->setType(hif::copy(tr));
            c->setValue(ass->setRightHandSide(nullptr));
            ass->setRightHandSide(c);
            _refMap[td].insert(c->getType());

            continue;
        }

        Cast *c = new Cast();
        c->setType(hif::copy(&o));
        obj->replace(c);
        c->setValue(obj);
    }

    delete &o;
    return 0;
}

int PreRefine_misc::visitFor(For &o)
{
    GuideVisitor::visitFor(o);

    Expression *condition = dynamic_cast<Expression *>(o.getCondition());
    if (condition == nullptr)
        return 0;
    if (condition->getOperator() != op_ge && condition->getOperator() != op_le)
        return 0;
    if (!o.initValues.empty())
        return 0;

    // - Checking initial declarations
    const BList<DataDeclaration>::size_t sizeDecl = o.initDeclarations.size();
    if (sizeDecl != 1 && sizeDecl != 2)
        return 0;

    Variable *v1 = dynamic_cast<Variable *>(o.initDeclarations.front());
    if (v1 == nullptr)
        return 0;
    Variable *v2 = nullptr;
    if (sizeDecl == 2) {
        v2 = dynamic_cast<Variable *>(o.initDeclarations.back());
        if (v2 == nullptr)
            return 0;

        // Check form var + _hif_support + [fresh]
        std::string n1(v1->getName());
        std::string n2(v2->getName());

        std::string nc = n1 + "_hif_support";
        if (n2.substr(0, nc.size()) != nc)
            return 0;
    }

    // - Checking step actions
    const BList<Action>::size_t sizeStepAct = o.stepActions.size();
    if (sizeStepAct != 1)
        return 0;

    BList<DataDeclaration> initDecls;
    hif::copy(o.initDeclarations, initDecls);

    BList<Action> initValues;
    hif::copy(o.initValues, initValues);

    BList<Action> forActions;
    hif::copy(o.forActions, forActions);
    if (sizeDecl == 2) {
        if (forActions.empty())
            return 0;
        BList<Action>::iterator it(forActions.front());
        it.erase();
        forActions.push_front(_factory.assignment(new Identifier(v1->getName()), new Identifier(v2->getName())));
    }

    BList<Action> actionList;
    actionList.push_back(_factory.assignment(
        new Identifier((sizeDecl == 1) ? v1->getName() : v2->getName()),
        _factory.expression(
            new Identifier((sizeDecl == 1) ? v1->getName() : v2->getName()),
            (condition->getOperator() == op_le) ? op_plus : op_minus, _factory.intval(1))));

    For *templ = _factory.forLoop(
        o.getName(), initDecls, initValues,
        _factory.expression(condition->getOperator(), new Identifier((sizeDecl == 1) ? v1->getName() : v2->getName())),
        actionList, forActions);

    hif::EqualsOptions opt;
    opt.skipNullBranches = true;
    const bool equals    = hif::equals(templ, &o, opt);
    delete templ;
    if (!equals)
        return 0;

    // Now we are sure that we can translate the condition with a range.
    Range *forRange = _factory.range(
        hif::copy(v1->getValue()), (condition->getOperator() == op_le) ? dir_upto : dir_downto,
        condition->setValue2(nullptr));

    delete o.setCondition(forRange);
    o.stepActions.clear();

    if (sizeDecl == 2) {
        DataDeclaration *d = o.initDeclarations.back();
        d->replace(nullptr);
        delete d;

        Action *a = o.forActions.front();
        a->replace(nullptr);
        delete a;
    }

    // Ensuring Range bound correct types.
    Type *lType = hif::semantics::getSemanticType(forRange->getLeftBound(), _sem);
    messageAssert(lType != nullptr, "Cannot type left bound", forRange->getLeftBound(), _sem);
    Type *rType = hif::semantics::getSemanticType(forRange->getRightBound(), _sem);
    messageAssert(rType != nullptr, "Cannot type right bound", forRange->getRightBound(), _sem);

    Type *lt = _sem->isTypeAllowedAsBound(lType);
    Type *rt = _sem->isTypeAllowedAsBound(rType);
    if (lt != nullptr) {
        Cast *c = new Cast();
        c->setType(lt);
        c->setValue(forRange->setLeftBound(nullptr));
        forRange->setLeftBound(c);
    }

    if (rt != nullptr) {
        Cast *c = new Cast();
        c->setType(rt);
        c->setValue(forRange->setRightBound(nullptr));
        forRange->setRightBound(c);
    }

    return 0;
}

int PreRefine_misc::visitStateTable(StateTable &o)
{
    GuideVisitor::visitStateTable(o);

    if (!hif::objectIsProcess(&o))
        return 0;
    // Optimization:
    if (o.sensitivityNeg.empty() && o.sensitivityPos.empty())
        return 0;

    using namespace hif::manipulation;

    // Fix sensitivity in case of pos/neg:
    analysis::AnalyzeProcessOptions::ProcessMap map;
    const bool isOk = hif::analysis::analyzeProcesses(&o, map, _sem);
    messageAssert(isOk, "Unable to fix process.", &o, _sem);

    analysis::ProcessInfos &infos = map[&o];
    switch (infos.processKind) {
    case analysis::ProcessInfos::ASYNCHRONOUS: {
        Value *expr = _buildAndExpression(&o, infos, false);
        If *ifStm   = new If();
        ifStm->addComment("Asynch");
        IfAlt *ifAlt = new IfAlt();
        ifStm->alts.push_back(ifAlt);
        ifAlt->setCondition(expr);
        ifAlt->actions.merge(o.states.front()->actions);
        o.states.front()->actions.push_back(ifStm);
        break;
    }
    case analysis::ProcessInfos::SYNCHRONOUS:
    case analysis::ProcessInfos::DERIVED_SYNCHRONOUS: {
        _fixSynchronousProcess(&o, infos);
        break;
    }
    case analysis::ProcessInfos::MIXED:
    case analysis::ProcessInfos::DERIVED_MIXED:
        messageError("Unsupported process kind.", &o, _sem);
    default:
        messageError("Unexpected process kind.", &o, _sem);
    }

    _moveSensitivities(&o);

    return 0;
}

int PreRefine_misc::visitSystem(System &o)
{
    GuideVisitor::visitSystem(o);

    // If there is at least one declaration in system declaration list
    // must be moved inside a package
    if (o.declarations.empty())
        return 0;

    LibraryDef *ld = new LibraryDef();
    ld->setName(NameTable::getInstance()->hifGlobals());
    ld->declarations.merge(o.declarations);
    ld->libraries.merge(o.libraries);
    ld->setLanguageID(o.getLanguageID());
    o.libraryDefs.push_back(ld);

    for (BList<Declaration>::iterator i = ld->declarations.begin(); i != ld->declarations.end(); ++i) {
        Declaration *d = *i;
        for (RefSet::iterator j = _refMap[d].begin(); j != _refMap[d].end(); ++j) {
            Object *obj = *j;
            if (hif::getNearestParent<LibraryDef>(obj) == ld)
                continue;

            Library *lib = new Library();
            lib->setName(NameTable::getInstance()->hifGlobals());

            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            addOpt.position                     = 0;
            hif::manipulation::addUniqueObject(lib, obj, addOpt);
        }
    }

    return 0;
}

int PreRefine_misc::visitWhen(When &o)
{
    GuideVisitor::visitWhen(o);

    // WHEN is directly supported only as
    // direct source of assign global action.
    // FIX:
    // 1- GlobalAction source --> OK
    // 2- Single assign inside a process --> globact
    // 3- Assign source/Return source  --> IF
    // 4- Otherwise      --> FunctionCall.
    messageAssert(!hif::manipulation::isInLeftHandSide(&o), "Wrong description with unexpected when.", &o, _sem);

    Assign *aoC    = dynamic_cast<Assign *>(&o);
    Return *roC    = dynamic_cast<Return *>(&o);
    Object *parent = aoC;
    if (parent == nullptr)
        parent = roC;

    // 1
    if (aoC != nullptr && dynamic_cast<GlobalAction *>(aoC->getParent()) != nullptr)
        return 0;

    // 2
    if (aoC != nullptr && aoC->getBList()->size() == 1 && dynamic_cast<State *>(aoC->getParent()) != nullptr &&
        dynamic_cast<StateTable *>(aoC->getParent()->getParent()) != nullptr &&
        dynamic_cast<BaseContents *>(aoC->getParent()->getParent()->getParent()) != nullptr) {
        StateTable *st = static_cast<StateTable *>(aoC->getParent()->getParent());
        RefsMap refsMap;
        hif::semantics::getAllReferences(refsMap, _sem, aoC);
        RefsMap sensMap;
        hif::semantics::getAllReferences(sensMap, _sem, st->sensitivity);
        hif::semantics::getAllReferences(sensMap, _sem, st->sensitivityPos);
        hif::semantics::getAllReferences(sensMap, _sem, st->sensitivityNeg);
        DeclsSet refSet;
        DeclsSet sensSet;
        _keepSignals(refsMap, refSet);
        _keepSignals(sensMap, sensSet);
        if (refSet == sensSet) {
            // Refine to globact!
            BaseContents *bc = static_cast<BaseContents *>(aoC->getParent()->getParent()->getParent());
            if (bc->getGlobalAction() == nullptr)
                bc->setGlobalAction(new GlobalAction());
            bc->getGlobalAction()->actions.push_back(hif::copy(aoC));
            _trash.insert(aoC);
            return 0;
        }
    }

    // 3
    if (parent != nullptr) {
        If *ifStm = hif::manipulation::transformWhenToIf(&o, _sem);
        parent->replace(ifStm);
        _trash.insert(parent);
        return 0;
    }

    // 4
    {
        hif::semantics::updateDeclarations(&o, _sem);

        // Creating function call
        FunctionCall *fc;
        fc = new FunctionCall();
        fc->setName(NameTable::getInstance()->getFreshName("when_function"));
        o.replace(fc);

        Type *t = hif::semantics::getSemanticType(&o, _sem);
        messageAssert(t != nullptr, "Cannot type When.", &o, _sem);

        // Creating function
        Function *f = static_cast<Function *>(
            _factory.subprogram(hif::copy(t), fc->getName(), _factory.noTemplates(), _factory.noParameters()));

        Return *ro = new Return();
        ro->setValue(&o);
        f->setStateTable(_factory.stateTable(fc->getName(), _factory.noDeclarations(), (ro)));

        // Transforming When to If
        If *ifStm = hif::manipulation::transformWhenToIf(&o, _sem);
        ro->replace(ifStm);
        delete ro;

        // Adding Function to the scope.
        Function *func     = hif::getNearestParent<Function>(fc);
        Contents *contents = hif::getNearestParent<Contents>(fc);
        if (func != nullptr) {
            BList<Object>::iterator it(func);
            it.insert_before(f);
        } else if (contents != nullptr) {
            contents->declarations.push_back(f);
        } else {
            messageError("Unsupported scope", fc, nullptr);
        }

        // Adding symbols as parameters.
        // TODO: managing symbols not found (typedefs, etc.).
        Scope *parentScope = hif::getNearestScope(fc, true, false, false);
        if (dynamic_cast<SubProgram *>(parentScope->getParent()) != nullptr)
            parentScope = static_cast<Scope *>(parentScope->getParent());

        RefsMap refsMap;
        hif::semantics::getAllReferences(refsMap, _sem, ifStm);
        for (RefsMap::iterator i = refsMap.begin(); i != refsMap.end(); ++i) {
            Declaration *decl = i->first;
            Signal *s         = dynamic_cast<Signal *>(decl);
            Port *po          = dynamic_cast<Port *>(decl);
            if (s == nullptr && po == nullptr && !hif::isSubNode(decl, parentScope))
                continue;
            DataDeclaration *ddecl = dynamic_cast<DataDeclaration *>(decl);
            if (ddecl == nullptr) {
                // Unsupported?
                continue;
            }

            Const *c     = dynamic_cast<Const *>(decl);
            Variable *v  = dynamic_cast<Variable *>(decl);
            ValueTP *vtp = dynamic_cast<ValueTP *>(decl);
            Parameter *p = dynamic_cast<Parameter *>(decl);
            if (c == nullptr && v == nullptr && vtp == nullptr && p == nullptr && po == nullptr && s == nullptr) {
                messageError("Unsupported declaration to be managed.", ddecl, _sem);
            }

            if (p != nullptr) {
                //                Parameter * np = hif::copy(p);
                //                f->parameters.push_back(np);
                //                ParameterAssign * pa = new ParameterAssign();
                //                pa->setName(np->getName());
                //                pa->setValue(new Identifier(np->getName()));
                //                fc->parameterAssigns.push_back(pa);
                continue;
            } else if (vtp != nullptr) {
                ValueTP *np = hif::copy(vtp);
                f->templateParameters.push_back(np);
                ValueTPAssign *pa = new ValueTPAssign();
                pa->setName(np->getName());
                pa->setValue(new Identifier(np->getName()));
                fc->templateParameterAssigns.push_back(pa);
                continue;
            } else if (v != nullptr) {
                Parameter *np = new Parameter();
                np->setType(hif::copy(v->getType()));
                np->setName(v->getName());
                f->parameters.push_back(np);
                ParameterAssign *pa = new ParameterAssign();
                pa->setName(np->getName());
                pa->setValue(new Identifier(np->getName()));
                fc->parameterAssigns.push_back(pa);
                continue;
            } else if (c != nullptr) {
                Parameter *np = new Parameter();
                np->setType(hif::copy(c->getType()));
                np->setName(c->getName());
                f->parameters.push_back(np);
                ParameterAssign *pa = new ParameterAssign();
                pa->setName(np->getName());
                pa->setValue(new Identifier(np->getName()));
                fc->parameterAssigns.push_back(pa);
                continue;
            } else if (po != nullptr) {
                Parameter *np = new Parameter();
                np->setType(hif::copy(po->getType()));
                np->setName(po->getName());
                f->parameters.push_back(np);
                ParameterAssign *pa = new ParameterAssign();
                pa->setName(np->getName());
                pa->setValue(new Identifier(np->getName()));
                fc->parameterAssigns.push_back(pa);
                continue;
            } else if (s != nullptr) {
                Parameter *np = new Parameter();
                np->setType(hif::copy(s->getType()));
                np->setName(s->getName());
                f->parameters.push_back(np);
                ParameterAssign *pa = new ParameterAssign();
                pa->setName(np->getName());
                pa->setValue(new Identifier(np->getName()));
                fc->parameterAssigns.push_back(pa);
                continue;
            } else {
                messageError("Unexpected case.", ddecl, _sem);
            }
        }
        hif::semantics::resetDeclarations(f);
    }

    return 0;
}

void PreRefine_misc::addRequiredLibraries()
{
    if (_addVhdlStandardLib) {
        hif::manipulation::AddUniqueObjectOptions addOpt;
        addOpt.equalsOptions.checkOnlyNames = true;
        hif::manipulation::addUniqueObject(
            _sem->getStandardLibrary("hif_vhdl_standard"), _system->libraryDefs, addOpt);
    }
}

Value *PreRefine_misc::_buildAndExpression(StateTable *o, analysis::ProcessInfos &infos, const bool skipReset)
{
    Value *expr = nullptr;

    DataDeclaration *rst = nullptr;
    if (skipReset)
        rst = infos.reset;
    expr = _buildSensitivityCondition(o, expr, infos.sensitivity, infos.clock, rst, true);
    expr = _buildSensitivityCondition(o, expr, infos.sensitivity, infos.clock, rst, false);
    expr = _buildSensitivityCondition(o, expr, infos.risingSensitivity, infos.clock, rst, true);
    expr = _buildSensitivityCondition(o, expr, infos.fallingSensitivity, infos.clock, rst, false);

    return expr;
}

Value *PreRefine_misc::_buildSensitivityCondition(
    StateTable *o,
    Value *expr,
    hif::analysis::ProcessInfos::ReferredDeclarations &refs,
    DataDeclaration *clock,
    DataDeclaration *skipReset,
    const bool isRising)
{
    using hif::analysis::ProcessInfos;

    for (ProcessInfos::ReferredDeclarations::iterator i = refs.begin(); i != refs.end(); ++i) {
        if (*i == skipReset)
            continue;
        Value *v = nullptr;

        Type *t = hif::semantics::getBaseType((*i)->getType(), false, _sem);
        messageAssert(t != nullptr, "Cannot get base type.", (*i)->getType(), _sem);
        Range *s = hif::typeGetSpan(t, _sem);
        messageAssert(s != nullptr, "Cannot get span type.", t, _sem);
        const unsigned long long bw = hif::semantics::spanGetBitwidth(s, _sem);
        messageAssert(bw == 1ULL, "Unexpected span size.", t, _sem);

        Bool *b = dynamic_cast<Bool *>(t);

        if (isRising)
            v = new Identifier((*i)->getName());
        else
            v = _factory.expression(b != nullptr ? op_not : op_bnot, new Identifier((*i)->getName()));

        if (b == nullptr) {
            Cast *c = new Cast();
            c->setValue(v);
            c->setType(_factory.boolean());
            v = c;
        }

        if (*i == clock) {
            // adding the 'event attribute
            Expression *clkE = new Expression();
            clkE->setOperator(op_and);
            clkE->setValue2(v);
            clkE->setValue1(_factory.functionCall(
                "hif_vhdl_event", new Identifier((*i)->getName()), _factory.noTemplateArguments(),
                _factory.noParameterArguments()));
            _addVhdlStandardLib = true;
            Library *lib        = new Library();
            lib->setName("hif_vhdl_standard");
            lib->setSystem(true);

            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            hif::manipulation::addUniqueObject(lib, o, addOpt);
            v = clkE;
        }

        if (expr == nullptr) {
            expr = v;
            continue;
        }

        Expression *e = new Expression();
        e->setOperator(op_or);
        e->setValue1(expr);
        e->setValue2(v);
        expr = e;
    }

    return expr;
}

void PreRefine_misc::_moveSensitivities(StateTable *o)
{
    for (BList<Value>::iterator i = o->sensitivityPos.begin(); i != o->sensitivityPos.end();) {
        Value *v = *i;
        i        = i.remove();
        o->sensitivity.push_back(v);
    }

    for (BList<Value>::iterator i = o->sensitivityNeg.begin(); i != o->sensitivityNeg.end();) {
        Value *v = *i;
        i        = i.remove();
        o->sensitivity.push_back(v);
    }
}

void PreRefine_misc::_fixSynchronousProcess(StateTable *o, analysis::ProcessInfos &infos)
{
    using hif::analysis::ProcessInfos;

    switch (infos.processStyle) {
    case ProcessInfos::NO_STYLE:
        messageError("Unexpected case (1).", o, _sem);
    case ProcessInfos::STYLE_1:
    case ProcessInfos::STYLE_2:
        // ntd
        break;
    case ProcessInfos::STYLE_4:
        // Mixed!
        messageError("Unexpected case (2).", o, _sem);
    case ProcessInfos::STYLE_3:
    case ProcessInfos::STYLE_5: {
        // Wrap with if:
        Value *expr = _buildAndExpression(o, infos, false);
        If *ifStm   = new If();
        ifStm->addComment("Style 3/5");
        IfAlt *ifAlt = new IfAlt();
        ifStm->alts.push_back(ifAlt);
        ifAlt->setCondition(expr);
        ifAlt->actions.merge(o->states.front()->actions);
        o->states.front()->actions.push_back(ifStm);
        break;
    }
    case ProcessInfos::STYLE_6: {
        // nested if:
        If *ifStm = dynamic_cast<If *>(o->states.front()->actions.front());
        ifStm->addComment("Style 6");
        messageAssert(ifStm != nullptr, "Unexpected object. (1)", o->states.front()->actions.front(), _sem);
        messageAssert(ifStm->alts.size() == 1, "Unexpected object. (2)", ifStm, _sem);
        IfAlt *ifa  = new IfAlt();
        Value *expr = _buildAndExpression(o, infos, true);
        ifa->setCondition(expr);
        ifa->actions.merge(ifStm->defaults);
        ifStm->alts.push_back(ifa);
        break;
    }
    default:
        messageError("Unexpected style.", o, _sem);
    }
}

void PreRefine_misc::_keepSignals(PreRefine_misc::RefsMap &map, DeclsSet &declSet)
{
    for (RefsMap::iterator i = map.begin(); i != map.end(); ++i) {
        Declaration *d = i->first;
        for (RefsSet::iterator j = i->second.begin(); j != i->second.end();) {
            if (hif::manipulation::isInLeftHandSide(*j)) {
                i->second.erase(j++);
            } else {
                ++j;
            }
        }

        if (i->second.empty()) {
            continue;
        }

        if (dynamic_cast<Port *>(d) != nullptr) {
            declSet.insert(d);
            continue;
        }
        if (dynamic_cast<Signal *>(d) != nullptr) {
            declSet.insert(d);
            continue;
        }
    }
}

} // namespace

void fixMiscIssues(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2VHDL", "fixMiscIssues");

    PreRefine_misc v(o, sem);
    o->acceptVisitor(v);
    v.addRequiredLibraries();

    hif::application_utils::restoreLogHeader();
}
