/// @file PrintSystemCVisitor.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>

#include "hif2sc/PrintMethods.hpp"
#include "hif2sc/PrintSystemCVisitor.hpp"

#include <hif/application_utils/dumpVersion.hpp>
#include <utility>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#if __GNUC__ >= 5
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif
#endif

using namespace hif;

namespace
{

using Indexes = std::list<Value *>;
struct AggregateInfo {
    AggregateInfo();
    ~AggregateInfo();
    AggregateInfo(const AggregateInfo &other);
    auto operator=(AggregateInfo other) -> AggregateInfo &;
    void swap(AggregateInfo &other) noexcept;

    Aggregate *aggregate{nullptr};
    Indexes indexes;
    Value *value{nullptr};
};

AggregateInfo::AggregateInfo()
    : indexes()

{
    // ntd
}

AggregateInfo::~AggregateInfo()
{
    // ntd
}

AggregateInfo::AggregateInfo(const AggregateInfo &other)
    : aggregate(other.aggregate)
    , indexes(other.indexes)
    , value(other.value)
{
    // ntd
}

auto AggregateInfo::operator=(AggregateInfo other) -> AggregateInfo &
{
    swap(other);
    return *this;
}

void AggregateInfo::swap(AggregateInfo &other) noexcept
{
    std::swap(aggregate, other.aggregate);
    std::swap(indexes, other.indexes);
    std::swap(value, other.value);
}

using AggregateInfos = std::list<AggregateInfo>;

void _fillAggregateAltInfo(Aggregate *agg, AggregateInfos &infos, hif::semantics::ILanguageSemantics *sem)
{
    if (agg->getOthers() != nullptr) {
        AggregateInfos rec;
        auto *recAgg = dynamic_cast<Aggregate *>(agg->getOthers());
        if (recAgg != nullptr) {
            _fillAggregateAltInfo(recAgg, rec, sem);
            for (auto &k : rec) {
                k.indexes.push_front(agg);
            }
        } else {
            AggregateInfo tmp;
            tmp.aggregate = agg;
            tmp.indexes.push_back(agg);
            tmp.value = agg->getOthers();
            rec.push_back(tmp);
        }

        infos.insert(infos.end(), rec.begin(), rec.end());
    }

    for (BList<AggregateAlt>::iterator i = agg->alts.begin(); i != agg->alts.end(); ++i) {
        AggregateAlt *alt = *i;
        for (BList<Value>::iterator j = alt->indices.begin(); j != alt->indices.end(); ++j) {
            Value *currentIndex = *j;
            AggregateInfos rec;
            auto *recAgg = dynamic_cast<Aggregate *>(alt->getValue());
            if (recAgg != nullptr) {
                _fillAggregateAltInfo(recAgg, rec, sem);
                for (auto &k : rec) {
                    k.indexes.push_front(currentIndex);
                }
            } else {
                AggregateInfo tmp;
                tmp.indexes.push_front(currentIndex);
                tmp.value = alt->getValue();
                rec.push_back(tmp);
            }

            infos.insert(infos.end(), rec.begin(), rec.end());
        }
    }
}

auto _getNativeConstantModifier(long long value, bool isSigned, long long bits) -> std::string
{
    std::stringstream ss;
    if (bits <= 16) {
        // Note: for 8-bit values int8_t/uint8_t should be enough. Choose this
        // since it's treated like a char and makes a mess with operation on the stringstream.
        if (isSigned) {
            auto v = static_cast<int16_t>(value);
            ss << v;
        } else {
            auto v = static_cast<uint16_t>(value);
            ss << v << "U";
        }
    } else if (bits <= 32) {
        if (isSigned) {
            auto v = static_cast<int32_t>(value);
            ss << v << "L";
        } else {
            auto v = static_cast<uint32_t>(value);
            ss << v << "UL";
        }
    } else if (bits <= 64) {
        if (isSigned) {
            int64_t v = value;
            ss << v << "LL";
        } else {
            auto v = static_cast<uint64_t>(value);
            ss << v << "ULL";
        }
    } else {
        // Represented on 64 bits.
        if (isSigned) {
            int64_t v = value;
            ss << v << "LL";
        } else {
            auto v = static_cast<uint64_t>(value);
            ss << v << "ULL";
        }
    }

    return ss.str();
}

} // unnamed namespace

PrintSystemCVisitorOptions::PrintSystemCVisitorOptions()
    : useResolved(false)
    , useHDTLib(false)
    , useCpp98(false)
    , maxLines(0)
    , printType(false)
    , printInitVal(false)
    , printImplementation(false)
    , printImplementation_ihh(false)
    , publicDecl(false)
    , insideInitList(false)
    , insideConstructorBody(false)
    , emptyInitList(false)
    , constManagement(nullptr)
    , printFields(false)
    , printSquareSpan(false)
    , printBitFields(false)
    , printFullType(false)
    , sourcesExtension()
    , headersExtension()
{
    // ntd
}

PrintSystemCVisitorOptions::~PrintSystemCVisitorOptions()
{
    // ntd
}

PrintSystemCVisitorOptions::PrintSystemCVisitorOptions(const PrintSystemCVisitorOptions &opt)
    : useResolved(opt.useResolved)
    , useHDTLib(opt.useHDTLib)
    , useCpp98(opt.useCpp98)
    , maxLines(opt.maxLines)
    , printType(opt.printType)
    , printInitVal(opt.printInitVal)
    , printImplementation(opt.printImplementation)
    , printImplementation_ihh(opt.printImplementation_ihh)
    , publicDecl(opt.publicDecl)
    , insideInitList(opt.insideInitList)
    , insideConstructorBody(opt.insideConstructorBody)
    , emptyInitList(opt.emptyInitList)
    , constManagement(opt.constManagement)
    , printFields(opt.printFields)
    , printSquareSpan(opt.printSquareSpan)
    , printBitFields(opt.printBitFields)
    , printFullType(opt.printFullType)
    , sourcesExtension(opt.sourcesExtension)
    , headersExtension(opt.headersExtension)
{
    // ntd
}

auto PrintSystemCVisitorOptions::operator=(const PrintSystemCVisitorOptions &opt) -> PrintSystemCVisitorOptions &
{
    if (this == &opt) {
        return *this;
    }
    useResolved             = opt.useResolved;
    useHDTLib               = opt.useHDTLib;
    useCpp98                = opt.useCpp98;
    maxLines                = opt.maxLines;
    printType               = opt.printType;
    printInitVal            = opt.printInitVal;
    printImplementation     = opt.printImplementation;
    printImplementation_ihh = opt.printImplementation_ihh;
    publicDecl              = opt.publicDecl;
    insideInitList          = opt.insideInitList;
    insideConstructorBody   = opt.insideConstructorBody;
    emptyInitList           = opt.emptyInitList;
    constManagement         = opt.constManagement;
    printFields             = opt.printFields;
    printSquareSpan         = opt.printSquareSpan;
    printBitFields          = opt.printBitFields;
    printFullType           = opt.printFullType;
    sourcesExtension        = opt.sourcesExtension;
    headersExtension        = opt.headersExtension;
    return *this;
}

PrintSystemCVisitor::PrintSystemCVisitor(
    hif::backends::IndentedStream *stream,
    PrintSystemCVisitorOptions &opt,
    ConstTemplateMap &ctmList,
    std::string baseName,
    std::string extension)
    : _opt(opt)
    , _sem(hif::semantics::SystemCSemantics::getInstance())
    , _ctmList(ctmList)
    , _outstream(stream)
    , _design_unit_scope()
    , _library_def_scope()
    , _left_angular(0LL)
    , _dataTypesString("_dataTypes")
    , _baseName(std::move(baseName))
    , _current_file_extension(std::move(extension))

{
    hif::application_utils::initializeLogHeader("HIF2SC", "PrintSystemCVisitor");
    // Assures maximum precision for printing numbers.
    if (_outstream != nullptr) {
        (*_outstream) << std::setprecision(std::numeric_limits<double>::digits10 + 1);
    }
#if (defined _MSC_VER) && (_MSC_VER < 1900)
    // Forcing exponent to be printed with two digits, as in Linux
    _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
}

PrintSystemCVisitor::PrintSystemCVisitor(ConstTemplateMap &ctmList)
    : _opt()
    , _sem(hif::semantics::SystemCSemantics::getInstance())
    , _ctmList(ctmList)
    , _outstream(nullptr)
    , _design_unit_scope()
    , _library_def_scope()
    , _left_angular(0LL)
    , _dataTypesString("_dataTypes")

{
    hif::application_utils::initializeLogHeader("HIF2SC", "PrintSystemCVisitor");
}

PrintSystemCVisitor::~PrintSystemCVisitor()
{
    messageDebugAssert(_left_angular == 0LL, "Opened left angular parens should be zero.", nullptr, nullptr);
    hif::application_utils::restoreLogHeader();
}

auto PrintSystemCVisitor::getCurrentDesignUnit() -> DesignUnit * { return _design_unit_scope.back(); }

void PrintSystemCVisitor::setCurrentDesignUnit(DesignUnit *du)
{
    if (du == nullptr) {
        return;
    }
    _design_unit_scope.push_back(du);
}

auto PrintSystemCVisitor::getCurrentLibraryDef() -> LibraryDef * { return _library_def_scope.back(); }

void PrintSystemCVisitor::setCurrentLibraryDef(LibraryDef *libDef)
{
    if (libDef == nullptr) {
        return;
    }
    _library_def_scope.push_back(libDef);
}

auto PrintSystemCVisitor::getDesignUnitScope() -> std::list<DesignUnit *> & { return _design_unit_scope; }

void PrintSystemCVisitor::setDesignUnitScope(std::list<DesignUnit *> &DUScope) { _design_unit_scope = DUScope; }

auto PrintSystemCVisitor::getLibraryDefScope() -> std::list<LibraryDef *> & { return _library_def_scope; }

void PrintSystemCVisitor::setLibraryDefScope(std::list<LibraryDef *> &LDScope) { _library_def_scope = LDScope; }

void PrintSystemCVisitor::clearConstTemplateMap(PrintSystemCVisitor::ConstTemplateMap &list)
{
    for (auto &i : list) {
        ObjectList &decls = i.second;
        for (auto &decl : decls) {
            delete decl;
        }
    }
}

auto PrintSystemCVisitor::_backupVisitMode() -> PrintSystemCVisitor::BackupOpt
{
    BackupOpt backupOptions;
    backupOptions._opt               = _opt;
    backupOptions._design_unit_scope = _design_unit_scope;
    backupOptions._library_def_scope = _library_def_scope;
    return backupOptions;
}

void PrintSystemCVisitor::_restoreVisitMode(BackupOpt &backupOptions)
{
    _opt               = backupOptions._opt;
    _design_unit_scope = backupOptions._design_unit_scope;
    _library_def_scope = backupOptions._library_def_scope;
}

auto PrintSystemCVisitor::_printStateTable(StateTable *st, DesignUnit *du) -> int
{
    _outstream->openBlock();
    // safety checks
    messageAssert(du != nullptr, "Unexpected nullptr design unit", nullptr, _sem);
    messageAssert(!du->views.empty() && du->views.size() == 1, "Unexpected number of views", du, _sem);

    _printScopeTemplate();
    *(_outstream) << "void ";
    _printScopeHierarchy();

    *(_outstream) << st->getName() << "()";
    _outstream->newLine();
    *(_outstream) << "{";
    _outstream->newLine();
    _outstream->indent();

    BackupOpt backup           = _backupVisitMode();
    _opt.printImplementation   = true;
    _opt.insideConstructorBody = false;

    st->acceptVisitor(*this);

    _restoreVisitMode(backup);

    _outstream->unindent();
    *(_outstream) << "}";
    _outstream->newLine(2);

    _outstream->closeBlock();
    return 0;
}

auto PrintSystemCVisitor::visitBit(Bit &o) -> int
{
    _printComment(&o);

    messageDebugAssert(o.getTypeVariant() == Type::NATIVE_TYPE, "Only \"native\" bit should be printed", &o, _sem);

    if (o.isLogic()) {
        if (_opt.useHDTLib) {
            *(_outstream) << "hdtlib::hl_logic_t";
        } else {
            *(_outstream) << "sc_dt::sc_logic";
        }
    } else {
        if (_opt.useHDTLib) {
            *(_outstream) << "bool";
        } else {
            *(_outstream) << "sc_dt::sc_bit";
        }
    }

    return 0;
}

auto PrintSystemCVisitor::visitArray(Array &o) -> int
{
    _printComment(&o);

    // Special print management for packed and not packed arrays.

    // Packed-array span must be printed in angular brackets before the name
    // in the declaration. Not-packed-array span must be printed in square
    // brackets after the name in the declaration.

    // A 2-step visit is provided in case of a combination of packed and
    // not-packed arrays. In this case, the subtree is formed as follows:
    //		<declaration>
    //			<type array not packed> // one or more
    //				<type array packed>

    if (_opt.printFullType) {
        const hif::LanguageID lang = hif::objectGetLanguage(&o);
        if (lang == hif::c) {
            // print as pointer: int *
            o.getType()->acceptVisitor(*this);
            *(_outstream) << "*";
            return 0;
        }

        // C++: print as reference: int (&) [5]
        _opt.printFullType = false;
        bool restore = _opt.printSquareSpan;

        // First round:
        _opt.printSquareSpan = false;
        o.acceptVisitor(*this);

        // Reference:
        *(_outstream) << "(&)";

        // Second round:
        _opt.printSquareSpan = true;
        o.acceptVisitor(*this);

        _opt.printSquareSpan = restore;
        _opt.printFullType   = true;
    }
    // First call, printSquareSpan = false
    // Visit of inner type
    else if (!_opt.printSquareSpan) {
        o.getType()->acceptVisitor(*this);
    }
    // Second call, printSquareSpan = true
    // Visit of outer type
    else {
        *(_outstream) << "[";
        if (dynamic_cast<Parameter *>(o.getParent()) == nullptr) {
            _printTypeSpanSize(o.getSpan());
        }
        *(_outstream) << "]";

        // In this case, recursive call is needed to correctly print the
        // span of multi-dimensional arrays
        if (dynamic_cast<Array *>(o.getType()) != nullptr) {
            o.getType()->acceptVisitor(*this);
        }
    }

    return 0;
}

auto PrintSystemCVisitor::visitBool(Bool &o) -> int
{
    _printComment(&o);

    *(_outstream) << "bool";
    return 0;
}

auto PrintSystemCVisitor::visitChar(Char &o) -> int
{
    _printComment(&o);

    *(_outstream) << "char";
    return 0;
}

auto PrintSystemCVisitor::visitEnum(Enum &o) -> int
{
    _printComment(&o);

    *(_outstream) << "enum {";
    _outstream->newLine();
    _outstream->indent();
    for (BList<EnumValue>::iterator enum_el = o.values.begin(); enum_el != o.values.end(); enum_el++) {
        if (enum_el != o.values.begin()) {
            *(_outstream) << ",";
            _outstream->newLine();
        }
        *(_outstream) << (*enum_el)->getName();
    }
    _outstream->newLine();
    _outstream->unindent();
    *(_outstream) << "}";

    return 0;
}

auto PrintSystemCVisitor::visitEvent(Event &o) -> int
{
    GuideVisitor::visitEvent(o);

    *(_outstream) << "sc_core::sc_event";

    return 0;
}

auto PrintSystemCVisitor::visitInt(Int &o) -> int
{
    if (_opt.printBitFields) {
        // old management .. print option to be removed
        return 0;
    }

    _printComment(&o);

    bool isNativeBitField = o.getTypeVariant() == Type::SYSTEMC_INT_BITFIELD;
    if (o.getTypeVariant() == Type::NATIVE_TYPE || isNativeBitField) {
        // Is native: int / unsigned int
        unsigned long long size = hif::semantics::spanGetBitwidth(o.getSpan(), _sem);
        std::string sizeStamp;
        bool standardIntSize = true;
        switch (size) {
        case 8:
            sizeStamp = "8";
            break;
        case 16:
            sizeStamp = "16";
            break;
        case 32:
            sizeStamp = "32";
            break;
        case 64:
            sizeStamp = "64";
            break;
        default:
            standardIntSize = false;
            sizeStamp       = "64";
            break;
        }

        messageDebugAssert(
            standardIntSize || isNativeBitField, "Unexpected native int (non-bitField) span size", &o, _sem);

        std::string typePrinting = (!o.isSigned()) ? "uint" : "int";
        *(_outstream) << typePrinting << sizeStamp << "_t";

        if (isNativeBitField && !standardIntSize) {
            // print bitField
            *(_outstream) << ": ";
            _printTypeSpanSize(o.getSpan());
        }

        return 0;
    }

    // SystemC types
    bool isScInt    = o.getTypeVariant() == Type::SYSTEMC_INT_SC_INT;
    bool isScBigInt = o.getTypeVariant() == Type::SYSTEMC_INT_SC_BIGINT;
    messageAssert(isScInt || isScBigInt, "Expected sc_int or sc_bigint types for given int", &o, _sem);

    if (_opt.useHDTLib) {
        // With hdtlib don't care about is big
        std::string typePrinting = (o.isSigned()) ? "hl_int_t" : "hl_uint_t";

        *(_outstream) << "hdtlib::" << typePrinting;

        // print type span
        *(_outstream) << "< ";
        _printTypeSpanSize(o.getSpan());
        *(_outstream) << " >";
    } else // no hdtlib
    {
        std::string typePrinting;
        if (isScInt) {
            typePrinting = (o.isSigned()) ? "sc_int" : "sc_uint";
        } else // isScBigInt
        {
            typePrinting = (o.isSigned()) ? "sc_bigint" : "sc_biguint";
        }

        *(_outstream) << "sc_dt::" << typePrinting;

        // print type span
        *(_outstream) << "< ";
        _printTypeSpanSize(o.getSpan());
        *(_outstream) << " >";
    }

    return 0;
}

auto PrintSystemCVisitor::visitPointer(Pointer &o) -> int
{
    _printComment(&o);

    o.getType()->acceptVisitor(*this);
    *(_outstream) << " *";
    return 0;
}

auto PrintSystemCVisitor::visitReal(Real &o) -> int
{
    _printComment(&o);

    *(_outstream) << "double";
    return 0;
}

auto PrintSystemCVisitor::visitRecord(Record &o) -> int
{
    _printComment(&o);

    // Special management: if called from inside initialization list, this should
    // print a call to struct fake constructor, passing all Fields initial values
    // as actual parameters.
    if (_opt.printImplementation && _opt.insideInitList) {
        *(_outstream) << "(";
        for (BList<Field>::iterator it(o.fields.begin()); it != o.fields.end(); ++it) {
            if (it != o.fields.begin()) {
                *(_outstream) << ",";
            }
            messageAssert((*it)->getValue() != nullptr, "Unexpected nullptr record initial value", *it, _sem);
            *(_outstream) << " ";
            (*it)->getValue()->acceptVisitor(*this);
        }
        *(_outstream) << ")";
        return 0;
    }

    if (_opt.printFields) {
        *(_outstream) << "{";
        _outstream->newLine();
        _outstream->indent();
        for (BList<Field>::iterator it(o.fields.begin()); it != o.fields.end(); ++it) {
            (*it)->acceptVisitor(*this);
        }

        // FIXME Ad-hoc fix (3): record fields initialization.
        // See documentation of _printRecordFakeConstructor.
        _printRecordClasslikeMethods(&o);
        _printRecordSignalMethods(&o);

        _outstream->unindent();
        *(_outstream) << "}";

        if (o.isPacked() && !o.isUnion()) {
            *(_outstream) << "#if (defined _MSC_VER)" << '\n'
                          << "#pragma pack(pop)" << '\n'
                          << "#else" << '\n'
                          << "__attribute__((packed))" << '\n'
                          << "#endif" << '\n';
        }
    } else {
        // Note: visit of typedef checks if its object-type is record to print
        // correctly "struct" instead of "typedef struct"
        if (!o.isUnion()) {
            if (o.isPacked()) {
                *(_outstream) << "#if (defined (_MSC_VER)) && ! (defined __GNUC__)" << '\n'
                              << "#pragma pack(push,1)" << '\n'
                              << "#endif" << '\n';
            }

            if (hif::objectGetLanguage(&o) == hif::c) {
                auto *td = dynamic_cast<TypeDef *>(o.getParent());
                messageAssert(td != nullptr, "Expected parent typedef", o.getParent(), _sem);
                *(_outstream) << "struct " << td->getName() << ";\n";
                *(_outstream) << "typedef struct " << td->getName() << " " << td->getName() << ";\n";
            }
            *(_outstream) << "struct";
        } else {
            if (hif::objectGetLanguage(&o) == hif::c) {
                auto *td = dynamic_cast<TypeDef *>(o.getParent());
                messageAssert(td != nullptr, "Expected parent typedef", o.getParent(), _sem);
                *(_outstream) << "union " << td->getName() << ";\n";
                *(_outstream) << "typedef union " << td->getName() << " " << td->getName() << ";\n";
            }
            *(_outstream) << "union";
        }
    }

    return 0;
}

auto PrintSystemCVisitor::visitRecordValue(RecordValue &o) -> int
{
    _printComment(&o);

    auto *dd = dynamic_cast<DataDeclaration *>(o.getParent());

    // if inside init.list, parenthesis are already printed. Otherwise, print them

    bool isDataDeclarationInitialValue = dd != nullptr && dd->getValue() == &o;
    if (!_opt.insideInitList && isDataDeclarationInitialValue) {
        // In case of initial value of data declaration it must be printed using struct
        // constructor: recordName(field1val, field2val, ...)
        // Data declaration type should be a typeRef.
        auto *tr = dynamic_cast<TypeReference *>(dd->getType());
        messageAssert(tr != nullptr, "Expected typeref type", dd->getType(), _sem);
        tr->acceptVisitor(*this);
    }
    PrintListOpt opt{
        !_opt.insideInitList, // _mandatoryParen
        _opt.insideInitList,  // _mandatoryNoParen
        false,                // _angularParen
        dd == nullptr,        // _curlyParen?
        true,                 // _breakLine
    };
    _printList(o.alts, opt);
    return 0;
}

auto PrintSystemCVisitor::visitRecordValueAlt(RecordValueAlt &o) -> int
{
    _printComment(&o);

    auto *agg = dynamic_cast<Aggregate *>(o.getValue());
    if (agg != nullptr && agg->getOthers() != nullptr && agg->alts.empty()) {
        // How to print this shit??
        *(_outstream) << "nullptr";
        return 0;
    }

    o.getValue()->acceptVisitor(*this);
    return 0;
}

auto PrintSystemCVisitor::visitReference(Reference &o) -> int
{
    _printComment(&o);

    o.getType()->acceptVisitor(*this);
    *(_outstream) << " &";
    return 0;
}

auto PrintSystemCVisitor::visitString(String &o) -> int
{
    _printComment(&o);
    *(_outstream) << "std::string";
    return 0;
}

auto PrintSystemCVisitor::visitSystem(hif::System &o) -> int
{
    _printComment(&o);
    _printAdditionalKeywords(&o);

    if (!_opt.printImplementation) {
        _printSystemDeclaration(o);
    } else {
        _printSystemImplementation(o);
    }

    return 0;
}

auto PrintSystemCVisitor::visitTypeReference(TypeReference &o) -> int
{
    _printComment(&o);

    if (_isNeededTypename(&o)) {
        *(_outstream) << "typename " << '\n';
    }

    if (o.getInstance() != nullptr) {
        const int rv = o.getInstance()->acceptVisitor(*this);
        if (rv != 1) {
            *(_outstream) << "::";
        }
    }

    *(_outstream) << o.getName();

    TypeDeclaration *tdect = hif::semantics::getDeclaration(&o, _sem);

    PrintListOpt opt{false, false, true, false, false};
    if (dynamic_cast<TypeDef *>(tdect) != nullptr) {
        auto *td = dynamic_cast<TypeDef *>(tdect);
        _printList(td->templateParameters, opt);
    } else if (dynamic_cast<TypeTP *>(tdect) != nullptr) {
        // TODO CHECK
        _printList(o.templateParameterAssigns, opt);
    }

    return 0;
}

auto PrintSystemCVisitor::visitView(View &o) -> int
{
    messageDebugAssert(false, "View should not be reached", &o, _sem);
    messageError("View should not be reached by printer.", &o, _sem);
}

auto PrintSystemCVisitor::visitViewReference(ViewReference &o) -> int
{
    _printComment(&o);

    if (_isNeededTypename(&o)) {
        *(_outstream) << "typename " << '\n';
    }

    if (o.getInstance() != nullptr) {
        const int rv = o.getInstance()->acceptVisitor(*this);
        if (rv != 1) {
            *(_outstream) << "::";
        }
    }

    *(_outstream) << o.getDesignUnit();

    View *v = hif::semantics::getDeclaration(&o, _sem);
    messageAssert(v != nullptr, "Declaration not found", &o, _sem);

    bool mandatoryParen = !v->templateParameters.empty();

    // Print the template-parameter assigns if present, or print empty
    // angulars if considering the View-TPs default values.
    PrintListOpt opt{mandatoryParen, false, true, false, false};
    _printList(o.templateParameterAssigns, opt);

    return 0;
}

auto PrintSystemCVisitor::visitAggregate(Aggregate &o) -> int
{
    _printComment(&o);

    if (o.alts.empty()) {
        auto *bv         = dynamic_cast<Bitvector *>(hif::semantics::getSemanticType(&o, _sem));
        Bit *othersType  = dynamic_cast<Bit *>(hif::semantics::getSemanticType(o.getOthers(), _sem));
        auto *stringType = dynamic_cast<String *>(hif::semantics::getSemanticType(&o, _sem));
        if (bv != nullptr && othersType != nullptr) {
            bv->acceptVisitor(*this);

            // Just smoking&frac
            auto *bval = dynamic_cast<BitValue *>(o.getOthers());
            if (bval != nullptr) {
                *(_outstream) << "('" << bval->toString() << "')";
                return 0;
            }

            *(_outstream) << "(";
            o.getOthers()->acceptVisitor(*this);
            *(_outstream) << ")";
            return 0;
        }
        if (stringType != nullptr) {
            // print std::string ctor: std::string(size, char)
            if (stringType->getSpanInformation() == nullptr) {
                // dont print plain others! it will be a single char,
                // leading to invalid ctor call!
                // printing std::string concat!
                *(_outstream) << "std::string() + (";
                o.getOthers()->acceptVisitor(*this);
                *(_outstream) << ")";
                return 0;
            }
            bool printString = !_opt.insideInitList;
            if (printString) {
                *(_outstream) << "std::string(";
            }
            Value *size = hif::semantics::spanGetSize(stringType->getSpanInformation(), _sem);
            size->acceptVisitor(*this);
            *(_outstream) << ", ";
            o.getOthers()->acceptVisitor(*this);
            if (printString) {
                *(_outstream) << ")";
            }
            return 0;
        }
        messageAssert(bv == nullptr, "Aggregate type must be a bitvector", &o, _sem);
        return o.getOthers()->acceptVisitor(*this);
    }

    *(_outstream) << "{";
    _outstream->indent();
    PrintListOpt opt{false, true, false, false, true};
    _printList(o.alts, opt);
    _outstream->unindent();
    _outstream->newLine();
    *(_outstream) << "}";
    return 0;
}

auto PrintSystemCVisitor::visitAggregateAlt(AggregateAlt &o) -> int
{
    _printComment(&o);

    o.getValue()->acceptVisitor(*this);
    return 0;
}

auto PrintSystemCVisitor::visitAlias(Alias &o) -> int
{
    messageDebugAssert(false, "Alias not supported yet", &o, _sem);
    messageError("Printer does not support Alias yet.", &o, _sem);
}

auto PrintSystemCVisitor::visitBitValue(BitValue &o) -> int
{
    _printComment(&o);

    Bit *tbit = dynamic_cast<Bit *>(hif::semantics::getSemanticType(&o, _sem));
    if (tbit == nullptr) {
        return 0;
    }

    switch (o.getValue()) {
    case bit_zero:
        if (tbit->isLogic()) {
            if (_opt.useHDTLib) {
                *(_outstream) << "hdtlib::hl_logic_t('0')";
            } else {
                // Not used sc_dt::SC_LOGIC_0 to avoid "static initialization order fiasco".
                *(_outstream) << "sc_dt::sc_logic('0')";
            }
        } else {
            if (_opt.useHDTLib) {
                *(_outstream) << "false";
            } else {
                *(_outstream) << "sc_dt::sc_bit('0')";
            }
        }
        break;
    case bit_one:
        if (tbit->isLogic()) {
            if (_opt.useHDTLib) {
                *(_outstream) << "hdtlib::hl_logic_t('1')";
            } else {
                // Not used sc_dt::SC_LOGIC_1 to avoid "static initialization order fiasco".
                *(_outstream) << "sc_dt::sc_logic('1')";
            }
        } else {
            if (_opt.useHDTLib) {
                *(_outstream) << "true";
            } else {
                *(_outstream) << "sc_dt::sc_bit('1')";
            }
        }
        break;
    case bit_x:
        if (tbit->isLogic()) {
            if (_opt.useHDTLib) {
                *(_outstream) << "hdtlib::hl_logic_t('X')";
            } else {
                // Not used sc_dt::SC_LOGIC_X to avoid "static initialization order fiasco".
                *(_outstream) << "sc_dt::sc_logic('X')";
            }
        }
        break;
    case bit_z:
        if (tbit->isLogic()) {
            if (_opt.useHDTLib) {
                *(_outstream) << "hdtlib::hl_logic_t('Z')";
            } else {
                // Not used sc_dt::SC_LOGIC_Z to avoid "static initialization order fiasco".
                *(_outstream) << "sc_dt::sc_logic('Z')";
            }
        }
        break;
    case bit_u:
    case bit_w:
    case bit_l:
    case bit_h:
    case bit_dontcare:
    default: // unsupported
        messageError("Unexpected bit value", &o, _sem);
    }

    return 0;
}

inline auto __is_integer(hif::Type *type) -> bool
{
    if (auto bitvector = dynamic_cast<hif::Bitvector *>(type)) {
        if (bitvector->isSigned()) {
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
    }
    return false;
}

auto PrintSystemCVisitor::visitBitvector(Bitvector &o) -> int
{
    _printComment(&o);

    messageDebugAssert(
        o.getTypeVariant() == Type::NATIVE_TYPE, "Only \"native\" bit vector should be printed", &o, _sem);

    if (o.isLogic()) {
        if (_opt.useHDTLib) {
            *(_outstream) << "hdtlib::hl_lv_t";
        } else {
            *(_outstream) << "sc_dt::sc_lv";
        }
    } else {
        if (_opt.useHDTLib) {
            *(_outstream) << "hdtlib::hl_bv_t";
        } else {
            *(_outstream) << "sc_dt::sc_bv";
        }
    }

    *(_outstream) << "< ";
    ++_left_angular;
    _printTypeSpanSize(o.getSpan());
    --_left_angular;
    *(_outstream) << " >";

    return 0;
}

auto PrintSystemCVisitor::visitBitvectorValue(BitvectorValue &o) -> int
{
    _printComment(&o);

    if (o.getType() == nullptr) {
        messageDebugAssert(false, "BitvectorValue without syntactic type.", &o, _sem);
        messageError("Unexpected bitvectorValue without syntactic type.", &o, _sem);
    }

    // TODO call a check function on bad char in value (-uwlh)
    std::string value = o.getValue();
    messageAssert(value.find('-') == std::string::npos, "SystemC support 01XZ", &o, _sem);
    messageAssert(value.find('W') == std::string::npos, "SystemC support 01XZ", &o, _sem);
    messageAssert(value.find('H') == std::string::npos, "SystemC support 01XZ", &o, _sem);
    messageAssert(value.find('L') == std::string::npos, "SystemC support 01XZ", &o, _sem);
    messageAssert(value.find('U') == std::string::npos, "SystemC support 01XZ", &o, _sem);

    o.getType()->acceptVisitor(*this);

    bool allEq = false;
    if (value.substr(0, 2) == "0X") // See manual sect 7.3
    {
        std::string prefix = "00";
        if (hif::typeIsLogic(o.getType(), _sem)) {
            prefix = "XX";
        }

        value = prefix + value;
    } else if (!value.empty()) {
        allEq                   = true;
        char c                  = value[0];
        std::string::iterator i = value.begin();
        ++i;
        for (; i != value.end(); ++i) {
            if (c == *i) {
                continue;
            }
            allEq = false;
            break;
        }
    }

    if (allEq) {
        *(_outstream) << "('" << value[0] << "')";
    } else {
        *(_outstream) << "(\"" << value << "\")";
    }
    return 0;
}

auto PrintSystemCVisitor::visitBoolValue(BoolValue &o) -> int
{
    _printComment(&o);

    if (o.getValue()) {
        *(_outstream) << "true";
    } else {
        *(_outstream) << "false";
    }

    return 0;
}

auto PrintSystemCVisitor::visitCast(Cast &o) -> int
{
    _printComment(&o);

    // check null pointer
    if (hif::objectIsNUllPointer(&o, _sem)) {
        *(_outstream) << "nullptr";
        return 0;
    }

    // check time cast
    if (dynamic_cast<Time *>(o.getType()) != nullptr) {
        auto *r = dynamic_cast<RecordValue *>(o.getValue());
        messageAssert(r != nullptr, "Unsupported case", &o, _sem);
        o.getType()->acceptVisitor(*this);
        r->acceptVisitor(*this);
        return 0;
    }

    bool needWrapParen = _needWrapParen(&o);
    if (needWrapParen) {
        *(_outstream) << "(";
    }

    // Every cast is performed using new style: static_cast<type>(val)
    // Note: in case of cast of a name (e.g., in a concat operation),
    // old-style cast is interpreted as a call to constructor.

    bool restore         = _opt.printFullType;
    _opt.printFullType         = true;
    const hif::LanguageID lang = hif::objectGetLanguage(&o);
    if (lang == hif::c) {
        // old style cast
        *(_outstream) << "( ";
        o.getType()->acceptVisitor(*this);
        *(_outstream) << ") ";
        o.getValue()->acceptVisitor(*this);
    } else {
        if (dynamic_cast<Pointer *>(o.getType()) != nullptr || dynamic_cast<Reference *>(o.getType()) != nullptr) {
            *(_outstream) << "reinterpret_cast< ";
        } else {
            *(_outstream) << "static_cast< ";
        }
        o.getType()->acceptVisitor(*this);
        *(_outstream) << " >( ";
        o.getValue()->acceptVisitor(*this);
        *(_outstream) << " )";
    }

    if (needWrapParen) {
        *(_outstream) << ")";
    }

    _opt.printFullType = restore;
    return 0;
}

auto PrintSystemCVisitor::visitCharValue(CharValue &o) -> int
{
    _printComment(&o);

    if (o.getValue() == '\0') {
        *(_outstream) << "'\\0'";
    } else {
        *(_outstream) << '\'' << o.getValue() << '\'';
    }

    return 0;
}

auto PrintSystemCVisitor::visitExpression(Expression &o) -> int
{
    _printComment(&o);

    // FIXME Ad-hoc fix (1): wrong interpretation of contents by SystemC.
    // This piece of code does not work:
    // sc_dt::sc_lv< 9 >( sc_dt::sc_lv< 1 >(x[7]) , x )
    // This works:
    // sc_dt::sc_lv< 9 >( ( sc_dt::sc_lv< 1 >(x[7]) , x ) )
    // The first one is interpreted as a call to sc_dt::sc_lv constructor,
    // instead of a concat. Fix: manually add brackets.

    // Also op_ref/op_deref expressions must be printed inside brackets.
    bool needWrapParen = _needWrapParen(&o);
    if (o.getOperator() == op_gt && _left_angular > 0) {
        // If the operator is '>' and we are inside two template arguments
        // (i.e., at least two angle brackets are opened), add an enclosing
        // bracket around the expression to prevent a compile error.
        // Reference design: mios/TX.
        needWrapParen = true;
    }

    if (needWrapParen) {
        *(_outstream) << "(";
    }

    // Check the existence of 2nd operand. If it is present the expression
    // is binary (i.e., Op1 op Op2) otherwise it is unary (i.e., op Op1)

    // If binary expression, print Op1. Unless pow (recursive call).
    if (o.getValue2() != nullptr && o.getOperator() != op_pow) {
        o.getValue1()->acceptVisitor(*this);
        *(_outstream) << " ";
    }

    // Print operator.
    switch (o.getOperator()) {
    // Logical (boolean) operators
    case op_not:
        *(_outstream) << "!";
        break;
    case op_or:
        *(_outstream) << "||";
        break;
    case op_xor:
        *(_outstream) << "^";
        break;
    case op_and:
        *(_outstream) << "&&";
        break;

        // Binary (bitwise) operators
    case op_bnot:
        *(_outstream) << "~";
        break;
    case op_bor:
        *(_outstream) << "|";
        break;
    case op_bxor:
        *(_outstream) << "^";
        break;
    case op_band:
        *(_outstream) << "&";
        break;
    case op_sll:
        *(_outstream) << "<<";
        break;
    case op_srl:
        *(_outstream) << ">>";
        break;
    case op_rol: // not supported
        *(_outstream) << "ROL";
        break;
    case op_ror: // not supported
        *(_outstream) << "ROR";
        break;

        // Concatenation operator
    case op_concat: {
        bool isStringConcat = dynamic_cast<String *>(hif::semantics::getBaseType(
                                        hif::semantics::getSemanticType(&o, _sem), false, _sem)) != nullptr;

        if (isStringConcat) {
            *(_outstream) << "+";
        } else {
            *(_outstream) << ",";
        }
    } break;

        // Equality operators
    case op_eq:
    case op_case_eq:
        *(_outstream) << "==";
        break;
    case op_neq:
    case op_case_neq:
        *(_outstream) << "!=";
        break;

        // Relational operators
    case op_lt:
        *(_outstream) << "<";
        break;
    case op_le:
        *(_outstream) << "<=";
        break;
    case op_gt:
        *(_outstream) << ">";
        break;
    case op_ge:
        *(_outstream) << ">=";
        break;

        // Arithmetic operators
    case op_plus:
        *(_outstream) << "+";
        break;
    case op_minus:
        *(_outstream) << "-";
        break;
    case op_mult:
        *(_outstream) << "*";
        break;
    case op_div:
        *(_outstream) << "/";
        break;
    case op_rem:
        *(_outstream) << "%";
        break;
    case op_pow: {
        if (dynamic_cast<IntValue *>(o.getValue1()) != nullptr) {
            auto *intval_o       = dynamic_cast<IntValue *>(o.getValue1());
            long long int intval = intval_o->getValue();

            if (intval > 0 && intval % 2 == 0) {
                // Replace 2 ^ n by 1 << n
                raiseUniqueWarning("\"2 pow X\" is replaced by \"1 << X\".");

                // replace operand 1 and operator in expression and re-visit
                intval_o->setValue(1);
                o.setOperator(op_sla);
                visitExpression(o);

                // restore original HIF expression
                intval_o->setValue(intval);
                o.setOperator(op_pow);

                if (needWrapParen) {
                    *(_outstream) << ")";
                }

                // visit has already be performed on modified object
                return 0;
            }
        }

        messageError("Unable to replace \"2 pow X\".", &o, _sem);
    }
    case op_abs:
        *(_outstream) << "abs";
        break;
    case op_ref:
        *(_outstream) << "&";
        break;
    case op_deref:
        *(_outstream) << "*";
        break;
    case op_sla:
        *(_outstream) << "<<";
        break;
    case op_sra:
        *(_outstream) << ">>";
        break;
    case op_mod:
        *(_outstream) << "%";
        break;
    case op_andrd:
    case op_orrd:
    case op_xorrd:
    case op_assign:
    case op_conv:
    case op_bind:
    case op_log:
    case op_none:
    case op_reverse:
    case op_size:
    default:
        messageError("This operator should be managed in refinement steps.", &o, _sem);
    }

    // If binary expression, print Op2.
    if (o.getValue2() != nullptr) {
        *(_outstream) << " ";
        o.getValue2()->acceptVisitor(*this);
    }
    // If unary expression, print Op1.
    else {
        messageAssert(o.getValue1() != nullptr, "Invalid expression.", &o, _sem);
        o.getValue1()->acceptVisitor(*this);
    }

    // FIXME Ad-hoc fix (1): closing bracket.
    if (needWrapParen) {
        *(_outstream) << ")";
    }

    return 0;
}

auto PrintSystemCVisitor::visitFunctionCall(FunctionCall &o) -> int { return _printCall(o); }

auto PrintSystemCVisitor::visitFieldReference(FieldReference &o) -> int
{
    _printComment(&o);

    bool needWrapParen = _needWrapParen(&o);
    if (needWrapParen) {
        *(_outstream) << "(";
    }

    bool printedInstance = false;
    if (dynamic_cast<Instance *>(o.getPrefix()) != nullptr) {

        Type *t = hif::semantics::getSemanticType(o.getPrefix(), _sem);
        if (dynamic_cast<Library *>(t) != nullptr) {
            auto *lib = dynamic_cast<Library *>(t);

            if (!lib->isStandard()) {
                // Simply print the namespace.
                t->acceptVisitor(*this);
                printedInstance = true;
            }
        } else if (dynamic_cast<ViewReference *>(t) != nullptr) {
            t->acceptVisitor(*this);
            printedInstance = true;
        } else {
            messageError("Unsupported case", &o, _sem);
        }
    } else {
        o.getPrefix()->acceptVisitor(*this);
        printedInstance = true;
    }

    bool staticAccess     = false;
    DataDeclaration *decl = dynamic_cast<DataDeclaration *>(hif::semantics::getDeclaration(&o, _sem));

    messageAssert(decl != nullptr, "Declaration not found", &o, _sem);
    if (!hif::declarationIsInstance(decl, o.getPrefix())) {
        staticAccess = true;
    }

    if (!staticAccess) {
        Type *t = hif::semantics::getSemanticType(o.getPrefix(), _sem);
        if (dynamic_cast<Pointer *>(t) != nullptr) {
            *(_outstream) << "->";
        } else {
            *(_outstream) << ".";
        }
    } else if (printedInstance) {
        *(_outstream) << "::";
    }

    *(_outstream) << o.getName();

    if (needWrapParen) {
        *(_outstream) << ")";
    }

    return 0;
}

auto PrintSystemCVisitor::visitIntValue(IntValue &o) -> int
{
    _printComment(&o);

    Type *t             = hif::semantics::getSemanticType(&o, _sem);
    Range *span         = hif::typeGetSpan(t, _sem);
    bool isSigned = hif::typeIsSigned(t, _sem);

    if (t->getTypeVariant() == Type::NATIVE_TYPE) {
        unsigned long long size = hif::semantics::spanGetBitwidth(span, _sem);
        messageAssert(
            size == 8 || size == 16 || size == 32 || size == 64, "Unexpected size of native int constant", t, _sem);

        // With overloaded functions printing of int constant may leads abiguity.
        // E.g. : foo(0L) <-- without cast is ambiguous
        // void foo(double)
        // void foo(int32_t)
        // void foo(int16_t)
        //
        // Ref. design: vhdl/gaisler/can_oc
        bool printCast = false;
        if (size == 8 || size == 16) {
            // always print since is trunkated
            printCast = true;
        } else if (size == 32) {
            // print only if is may be ambiguous.
            printCast = _mayBeAmbiguous(&o);
        } else // size == 64
        {
            // cannot be ambiguos
            printCast = false;
        }

        const hif::LanguageID lang = hif::objectGetLanguage(&o);

        if (printCast) {
            if (lang == hif::c) {
                *_outstream << "((";
                if (!isSigned) {
                    *_outstream << "u";
                }
                *_outstream << "int" << size << "_t)";
            } else {
                if (!isSigned) {
                    *_outstream << "u";
                }
                *_outstream << "int" << size << "_t(";
            }
        }
        *(_outstream) << _getNativeConstantModifier(o.getValue(), isSigned, static_cast<long long>(size));

        if (printCast) {
            // both (lang == hif::c) and lang != hif::c
            *_outstream << ")";
        }
    } else if (t->getTypeVariant() == Type::SYSTEMC_INT_BITFIELD) {
        messageError("Unexpected constant of type int-bitfield", &o, _sem);
    } else // SystemC types
    {
        (*_outstream) << "static_cast< ";
        ++_left_angular;
        t->acceptVisitor(*this);
        --_left_angular;
        (*_outstream) << " >( ";
        *(_outstream) << _getNativeConstantModifier(o.getValue(), isSigned, 64LL);
        (*_outstream) << " )";
    }

    return 0;
}

auto PrintSystemCVisitor::visitMember(Member &o) -> int
{
    _printComment(&o);

    bool needWrapParen = _needWrapParen(&o);
    if (needWrapParen) {
        *(_outstream) << "(";
    }
    o.getPrefix()->acceptVisitor(*this);

    *(_outstream) << "[";

    messageAssert(o.getIndex() != nullptr, "Expected index", &o, _sem);
    o.getIndex()->acceptVisitor(*this);

    *(_outstream) << "]";

    if (needWrapParen) {
        *(_outstream) << ")";
    }

    return 0;
}

auto PrintSystemCVisitor::visitGlobalAction(GlobalAction &o) -> int
{
    if (_opt.printImplementation) {
        return 0;
    }

    for (BList<Action>::iterator it = o.actions.begin(); it != o.actions.end(); ++it) {
        (*it)->acceptVisitor(*this);
        if (_isStatement(*it)) {
            *(_outstream) << ";";
        }
        _outstream->newLine();
    }
    return 0;
}

auto PrintSystemCVisitor::visitIdentifier(Identifier &o) -> int
{
    _printComment(&o);

    // Print identifier name.
    *(_outstream) << o.getName();
    return 0;
}

auto PrintSystemCVisitor::visitParameterAssign(ParameterAssign &o) -> int
{
    _printComment(&o);

    o.getValue()->acceptVisitor(*this);

    return 0;
}

auto PrintSystemCVisitor::visitRealValue(RealValue &o) -> int
{
    _printComment(&o);

    const double d = o.getValue();
    std::stringstream ss;
    ss << d;
    std::string s(ss.str());
    bool hasDot = (s.find('.') != std::string::npos);
    bool hasE   = (s.find('E') != std::string::npos) || (s.find('e') != std::string::npos);
    if ((!hasDot) && (!hasE)) {
        s += ".0";
    }
    *(_outstream) << s;
    return 0;
}

auto PrintSystemCVisitor::visitSigned(Signed &o) -> int
{
    messageDebugAssert(false, "Signed not supported yet", &o, _sem);
    messageError("Signed not supported yet.", &o, _sem);
}

auto PrintSystemCVisitor::visitSlice(Slice &o) -> int
{
    _printComment(&o);

    Range *span = o.getSpan();

    o.getPrefix()->acceptVisitor(*this);
    *(_outstream) << ".range(";
    span->getLeftBound()->acceptVisitor(*this);
    *(_outstream) << ", ";
    span->getRightBound()->acceptVisitor(*this);
    *(_outstream) << ")";

    return 0;
}

auto PrintSystemCVisitor::visitStringValue(StringValue &o) -> int
{
    _printComment(&o);

    bool needString = _mayBeAmbiguous(&o);

    if (needString) {
        *(_outstream) << "std::string(";
    }
    *_outstream << hif::backends::openString;
    *(_outstream) << "\"" << o.getValue() << "\"";
    *_outstream << hif::backends::closeString;
    if (needString) {
        *(_outstream) << ")";
    }
    return 0;
}

auto PrintSystemCVisitor::visitTypeTPAssign(TypeTPAssign &o) -> int
{
    _printComment(&o);

    o.getType()->acceptVisitor(*this);
    return 0;
}

auto PrintSystemCVisitor::visitValueTPAssign(ValueTPAssign &o) -> int
{
    _printComment(&o);

    o.getValue()->acceptVisitor(*this);

    return 0;
}

auto PrintSystemCVisitor::visitWhen(When &o) -> int
{
    _printComment(&o);

    if (o.alts.empty()) {
        return 0;
    }

    bool needWrapParen = _needWrapParen(&o);
    if (needWrapParen) {
        *(_outstream) << "(";
    }

    for (BList<WhenAlt>::iterator it = o.alts.begin(); it != o.alts.end(); ++it) {
        (*it)->acceptVisitor(*this);
        *(_outstream) << " : ";
    }

    o.getDefault()->acceptVisitor(*this);

    if (needWrapParen) {
        *(_outstream) << ")";
    }

    return 0;
}

auto PrintSystemCVisitor::visitWhenAlt(WhenAlt &o) -> int
{
    _printComment(&o);

    o.getCondition()->acceptVisitor(*this);
    *(_outstream) << " ? ";
    o.getValue()->acceptVisitor(*this);
    return 0;
}

auto PrintSystemCVisitor::visitWith(With &o) -> int { messageError("With should be mapped to When", &o, _sem); }

auto PrintSystemCVisitor::visitWithAlt(WithAlt &o) -> int
{
    messageDebugAssert(false, "WithAlt should be managed in VisitWith.", &o, _sem);
    messageError("WithAlt should be already managed.", &o, _sem);
}

auto PrintSystemCVisitor::visitContents(Contents &o) -> int
{
    _printComment(&o);
    _printAdditionalKeywords(&o);

    messageDebugAssert(o.generates.empty(), "List of generate should be empty", &o, _sem);

    // Print header file declarations.
    if (!_opt.printImplementation) {
        // safety check
        messageDebugAssert(!_opt.insideInitList && !_opt.insideConstructorBody, "Unexpected print flags", &o, _sem);

        if (_opt.publicDecl) {
            for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
                // Managed by _printConstants()
                if (dynamic_cast<Const *>(*it) != nullptr) {
                    continue;
                }

                // Macro-SubProgram contained in Contents are printed in .cc files.
                auto *sp = dynamic_cast<SubProgram *>(*it);
                if (sp != nullptr && sp->getKind() == SubProgram::MACRO) {
                    continue;
                }

                (*it)->acceptVisitor(*this);
            }

            if (!o.instances.empty()) {
                _outstream->newLine();
            }
            for (BList<Instance>::iterator it = o.instances.begin(); it != o.instances.end(); ++it) {
                (*it)->acceptVisitor(*this);
            }

            if (o.getGlobalAction() != nullptr) {
                _outstream->newLine();
                o.getGlobalAction()->acceptVisitor(*this);
            }
        } else {
            if (!o.stateTables.empty()) {
                _outstream->newLine();
            }
            for (BList<StateTable>::iterator it = o.stateTables.begin(); it != o.stateTables.end(); ++it) {
                (*it)->acceptVisitor(*this);
            }
        }
    }
    // Print initializations (module constructor and initialization list).
    // Print of functions, procedures, and processes implementation is not
    // managed here, since it depends on the presence of TPs and cannot be general.
    else if (_opt.printImplementation) {
        messageDebugAssert(_opt.insideConstructorBody ^ _opt.insideInitList, "Unexpected print flags", &o, _sem);

        // Visit all declarations except:
        // - StateTable, functions and procedures (managed by function printImplementation)
        // - Const (managed by function printConstants)
        // - TypeDef (they are only declared, not instantiated!)
        for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
            if (dynamic_cast<Const *>(*it) != nullptr         // Separately print. see _printConstants()
                || dynamic_cast<Function *>(*it) != nullptr   // Separately print
                || dynamic_cast<Procedure *>(*it) != nullptr  // Separately print
                || dynamic_cast<TypeDef *>(*it) != nullptr    // Nothing to print
                || dynamic_cast<DesignUnit *>(*it) != nullptr // Always printed in a separated file
            ) {
                continue;
            }

            if (_opt.insideInitList && !_opt.emptyInitList) {
                *(_outstream) << ",";
                _outstream->newLine();
                _opt.emptyInitList = true;
            }

            // Variables are usually initialized in constructor initialization list.
            // These checks are performed here since other components may
            // exploit visit of variables.
            auto *ddecl = dynamic_cast<DataDeclaration *>(*it);
            messageDebugAssert(ddecl != nullptr, "Expected DataDeclaration", *it, _sem);
            ddecl->acceptVisitor(*this);
        }

        // Instances are visited twice:
        // 1) inside initialization list to print their constructor
        // 2) inside the constructor to print their binding
        for (BList<Instance>::iterator it = o.instances.begin(); it != o.instances.end(); ++it) {
            (*it)->acceptVisitor(*this);
        }

        if (_opt.insideConstructorBody) {
            for (BList<StateTable>::iterator it = o.stateTables.begin(); it != o.stateTables.end(); ++it) {
                (*it)->acceptVisitor(*this);
            }
        }
    }

    return 0;
}

auto PrintSystemCVisitor::visitConst(Const &o) -> int
{
    if (!_opt.printImplementation) {
        _printComment(&o);
        _printDefineMacros(&o);
        _printAdditionalKeywords(&o);

        // Defines are printed in header only
        if (o.isDefine()) {
            *(_outstream) << "#define " << o.getName() << " ";
            _outstream->setMacroMode(true);
            o.getValue()->acceptVisitor(*this);
            _outstream->setMacroMode(false);
            _outstream->newLine();
            return 0;
        }
    }

    bool isInModule =
        (dynamic_cast<StateTable *>(o.getParent()) == nullptr && hif::getNearestParent<View>(&o) != nullptr);
    bool isStatic    = (isInModule && !o.isInstance());
    bool isFullySpec = _isFullySpecifiedArrayConst(&o);

    if (!_opt.printImplementation) {
        // Constants are always public
        if (isInModule && !_opt.publicDecl) {
            return 0;
        }

        // If defined directly inside LibraryDef or System
        if (!isInModule) {
            *(_outstream) << "extern ";
        }

        // If not class member, or initial value is a fully-specified aggregate
        if (isStatic /*|| (isInModule && isFullySpec)*/) {
            *(_outstream) << "static ";
        }

        *(_outstream) << "const ";
        o.getType()->acceptVisitor(*this);
        *(_outstream) << " " << o.getName() << ";";
        _outstream->newLine();
        return 0;
    }

    // if (_opt.printImplementation)

    // "static" or "fully-specified" constants are defined outside class.
    if ((isStatic /*|| isFullySpec*/) && (_opt.insideInitList || _opt.insideConstructorBody)) {
        return 0;
    }

    // Class-member const are init only inside constructor, unless they are static
    // or array-typed.
    if (isInModule &&
        !isStatic
        /*&& !isFullySpec*/
        && !_opt.insideInitList && !_opt.insideConstructorBody) {
        return 0;
    }

    // "static", "fully-qualified" constants.
    if (!isInModule || (isInModule && !o.isInstance())
        /*|| (isInModule && o.isInstance() && isFullySpec)*/) {
        // If declared inside some component already inheriting TPs
        auto *sp           = hif::getNearestParent<SubProgram>(&o);
        auto *st           = hif::getNearestParent<StateTable>(&o);
        bool isLocal = (sp != nullptr || st != nullptr);
        if (!isLocal) {
            _printScopeTemplate();
        }

        *(_outstream) << "const ";

        if (dynamic_cast<ReferencedType *>(o.getType()) != nullptr) {
            // Needed to manage _isNeededTypename() in a different way
            Declaration *restore = _opt.constManagement;
            _opt.constManagement = &o;

            if (_isNeededTypename(dynamic_cast<ReferencedType *>(o.getType()))) {
                (*_outstream) << "typename ";
            }

            _opt.constManagement = restore;
        }

        // prefix type if needed
        auto *rt         = dynamic_cast<ReferencedType *>(o.getType());
        bool isLocalType = false;
        if (rt != nullptr) {
            Declaration *rtDecl = hif::semantics::getDeclaration(rt, _sem);
            View *currentScope  = hif::getNearestParent<View>(&o);
            isLocalType         = hif::isSubNode(rtDecl, currentScope);
        }

        if (isLocalType) {
            _printScopeHierarchy();
        }

        o.getType()->acceptVisitor(*this);
        *(_outstream) << " ";

        // prefix name if needed
        if (!isLocal) {
            _printScopeHierarchy();
        }
    }

    if (isFullySpec) {
        // In this case an individual init would cause error, and a normal
        // one would not work since aggregate cannot be inserted in parenthesis.
        *(_outstream) << o.getName() << " = ";
        o.getValue()->acceptVisitor(*this);
        *(_outstream) << ";";
        _outstream->newLine();
        return 0;
    }

    Declaration *restore = _opt.constManagement;
    if (isInModule) {
        // Needed to manage _isNeededTypename() in a different way
        _opt.constManagement = hif::getNearestParent<View>(&o);
    }

    _manageInitialization(&o);

    if (isInModule) {
        _opt.constManagement = restore;
    }

    return 0;
}

auto PrintSystemCVisitor::visitDesignUnit(DesignUnit &o) -> int
{
    std::list<DesignUnit *> restore = _design_unit_scope;

    // Sometimes it is already set before the call by header or impl. visitors.
    if (_design_unit_scope.empty() || _design_unit_scope.back()->getName() != o.getName()) {
        _design_unit_scope.push_back(&o);
    }

    if (!_opt.printImplementation) {
        _printModuleDeclaration(o);
    } else {
        _printModuleImplementation(o);
    }

    _design_unit_scope = restore;

    return 0;
}

auto PrintSystemCVisitor::visitEntity(Entity &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    for (BList<Port>::iterator it = o.ports.begin(); it != o.ports.end(); ++it) {
        if (_opt.insideInitList && !_opt.emptyInitList) {
            *(_outstream) << ",";
            _outstream->newLine();
        }
        (*it)->acceptVisitor(*this);
    }

    return 0;
}

auto PrintSystemCVisitor::visitField(Field &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    // First of 2-step to achieve a correct composite-type print
    _visitType(o.getType(), false);
    *(_outstream) << " " << o.getName();
    // Second of 2-step to achieve a correct composite-type print
    _visitType(o.getType(), true);
    *(_outstream) << ";";
    _outstream->newLine();

    return 0;
}

auto PrintSystemCVisitor::visitFunction(Function &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    auto *du = hif::getNearestParent<DesignUnit>(&o);

    if (!_opt.printImplementation) {
        if (_isCppConstructor(&o)) {
            _printSubProgramDeclaration(o, du->getName());
            _outstream->newLine();
            return 0;
        }

        if (_opt.printType) {
            switch (o.getKind()) {
            case SubProgram::INSTANCE:
            case SubProgram::IMPLICIT_INSTANCE:
                break;
            case SubProgram::VIRTUAL:
                *(_outstream) << "virtual ";
                break;
            case SubProgram::STATIC:
                *(_outstream) << "static ";
                break;
            case SubProgram::MACRO:
            default:
                messageError("Unsupported Function kind", &o, nullptr);
            }
            if (o.checkProperty(PROPERTY_CONSTEXPR)) {
                *(_outstream) << "constexpr ";
            }
            _printFullTP(o.templateParameters);
            _outstream->newLine();
            o.getType()->acceptVisitor(*this);
            *(_outstream) << " ";
        }

        _printSubProgramDeclaration(o);
    } else // if (_opt.printImplementation)
    {
        _outstream->openBlock();

        // safety check
        messageDebugAssert(!_opt.insideInitList, "Unexpected print flags", &o, _sem);

        if (o.checkProperty(PROPERTY_CONSTEXPR)) {
            *(_outstream) << "constexpr ";
        }

        // Global or library function.
        if (du == nullptr) {
            _printTypedTP(o.templateParameters);
            _outstream->newLine();
            o.getType()->acceptVisitor(*this);
            *(_outstream) << " ";
            _printSubProgramImplementation(o);
            _outstream->closeBlock();
            return 0;
        }

        messageDebugAssert(!du->views.empty() && du->views.size() == 1, "Unsupported number of views", du, _sem);
        View *view = du->views.front();

        _printScopeTemplate();
        _printTypedTP(o.templateParameters);
        _outstream->newLine();

        if (!_isCppConstructor(&o)) {
            o.getType()->acceptVisitor(*this);
            *(_outstream) << " ";
        }

        _printScopeHierarchy();

        if (_isCppConstructor(&o)) {
            // this code reply the function _printBodyImplementation,
            // TODO improve _printBodyImplementation to manage also constructor
            _printSubProgramDeclaration(o, du->getName());
            _printInitializationList(view);
            _outstream->newLine();
            *(_outstream) << "{";
            _outstream->newLine();
            _outstream->indent();

            _printOtherInitializations(view);
            if (o.getStateTable() != nullptr) {
                _opt.printInitVal = true;
                o.getStateTable()->acceptVisitor(*this);
            }

            _outstream->unindent();
            *(_outstream) << "}";
            _outstream->newLine(3);
        } else {
            _printSubProgramImplementation(o);
        }
        _outstream->closeBlock();
    }

    return 0;
}

auto PrintSystemCVisitor::visitInstance(Instance &o) -> int
{
    _printComment(&o);

    if (!_opt.printImplementation) {
        // safety check
        messageAssert(o.getReferencedType() != nullptr, "Unexpected instance without ref type", &o, _sem);

        auto *bc = dynamic_cast<BaseContents *>(o.getParent());
        bool isSubmodule =
            (o.isInBList() && bc != nullptr && o.getBList() == reinterpret_cast<BList<Object> *>(&bc->instances));

        if (isSubmodule) {
            o.getReferencedType()->acceptVisitor(*this);
            // If the instance is inside a Generate constructor, it is treated as
            // a pointer (event if it is not) to avoid useless allocations.
            *(_outstream) << " " << o.getName() << ";";
            _outstream->newLine();
        } else {
            *(_outstream) << " " << o.getName();
        }
    } else if (_opt.printImplementation && _opt.insideInitList) {
        if (!_opt.emptyInitList) {
            *(_outstream) << ",";
            _outstream->newLine();
        }

        // Print instance constructor.
        Type *o_type         = hif::semantics::getSemanticType(&o, _sem);
        bool isPointer = dynamic_cast<Pointer *>(o_type) != nullptr;
        // TODO check this after the introduction of __hif_new

        *(_outstream) << o.getName();

        if (isPointer) {
            auto *vrI  = dynamic_cast<ViewReference *>(o.getReferencedType());
            auto *trI  = dynamic_cast<TypeReference *>(o.getReferencedType());
            auto *libI = dynamic_cast<Library *>(o.getReferencedType());
            std::string instName;
            if (trI != nullptr) {
                instName = trI->getName();
            } else if (vrI != nullptr) {
                instName = vrI->getDesignUnit();
            } else if (libI != nullptr) {
                instName = libI->getName();
            }

            *(_outstream) << "(new " << instName;
        }

        if (o.getValue() != nullptr) {
            auto *fc = dynamic_cast<FunctionCall *>(o.getValue());
            messageAssert(
                fc != nullptr && fc->getName() == hif::NameTable::getInstance()->hifConstructor(),
                "Unexpected value as initial value of instance", &o, _sem);

            PrintListOpt opt;
            opt._mandatoryParen = true;
            _printList(fc->parameterAssigns, opt);
        } else {
            *(_outstream) << "(\"" << o.getName() << "\"";
            _printNotCompileTimeTemplates(o.getReferencedType(), CONST_TEMPL_CTOR_CALL);
            *(_outstream) << ")";
        }

        if (isPointer) {
            *(_outstream) << "\")";
        }

        _opt.emptyInitList = false;
    } else if (_opt.printImplementation && _opt.insideConstructorBody) {
        // Print instance binding.
        _outstream->newLine();

        for (BList<PortAssign>::iterator it = o.portAssigns.begin(); it != o.portAssigns.end(); ++it) {
            std::list<std::string> indexes;
            Type *t = hif::semantics::getBaseType(hif::semantics::getSemanticType(*it, _sem), false, _sem);
            _printPortBinding(o.getName(), *it, t, indexes);
        }
    } else {
        // If the visit of Instance does not concern the constructor (i.e., it
        // is called by the visit of a Function.

        bool needWrapParen = _needWrapParen(&o);
        if (needWrapParen) {
            *(_outstream) << "(";
        }

        *(_outstream) << o.getName();

        if (needWrapParen) {
            *(_outstream) << ")";
        }
    }

    return 0;
}

auto PrintSystemCVisitor::visitLibrary(hif::Library &o) -> int
{
    if (o.isStandard()) {
        return 1;
    }

    if (o.getInstance() != nullptr) {
        const int rv = o.getInstance()->acceptVisitor(*this);
        if (rv != 1) {
            *(_outstream) << "::";
        }
    }

    *(_outstream) << _sem->getLibraryNamespace(o.getName());
    return 0;
}

auto PrintSystemCVisitor::visitLibraryDef(hif::LibraryDef &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    std::list<LibraryDef *> restore = _library_def_scope;

    // Sometimes it is already set before the call by header or impl. visitors.
    if (_library_def_scope.empty() || _library_def_scope.back()->getName() != o.getName()) {
        _library_def_scope.push_back(&o);
    }

    if (!_opt.printImplementation) {
        _printLibraryDeclaration(o);
    } else {
        _printLibraryImplementation(o);
    }

    _library_def_scope = restore;
    return 0;
}

auto PrintSystemCVisitor::visitParameter(Parameter &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    _visitType(o.getType(), false);
    Type *paramType  = hif::semantics::getBaseType(o.getType(), false, _sem);
    auto *ref        = dynamic_cast<Reference *>(paramType);
    bool isRef = (ref != nullptr);
    bool isArray     = false;
    if (isRef) {
        auto *refType = dynamic_cast<Array *>(hif::semantics::getBaseType(ref->getType(), false, _sem));
        isArray       = refType != nullptr;
    } else {
        auto *refType = dynamic_cast<Array *>(paramType);
        isArray       = refType != nullptr;
    }

    if ((o.getDirection() == dir_out || o.getDirection() == dir_inout) && !isRef && !isArray) {
        *(_outstream) << " &";
    }
    *(_outstream) << " " << o.getName();

    _visitType(o.getType(), true);

    if (!_opt.printImplementation && o.getValue() != nullptr) {
        *(_outstream) << " = ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

auto PrintSystemCVisitor::visitPort(Port &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    if (_opt.printImplementation) {
        if (_opt.insideInitList) {
            *(_outstream) << o.getName() << "(";
            _initListInitializationSignalPort(&o);
            *(_outstream) << ")";
        } else if (_opt.insideConstructorBody) {
            if (o.isWrapper()) {
                return 0;
            }

            if (_opt.printInitVal && (o.getValue() != nullptr) &&
                (o.getDirection() == dir_out || o.getDirection() == dir_inout)) {
                std::list<std::string> indexes;
                _printInitialize(o, o.getType(), indexes, o.isWrapper());
            }
        }
    } else // if (! _opt.printImplementation)
    {
        messageDebugAssert(_opt.printType, "Unexpected print flags", &o, _sem);

        Type *baseType      = hif::semantics::getBaseType(o.getType(), false, _sem);
        auto *array         = dynamic_cast<Array *>(baseType);
        auto *record        = dynamic_cast<Record *>(baseType);
        bool isArray  = (array != nullptr);
        bool isRecord = (record != nullptr);

        bool dontExpand = false;
        auto *tr        = dynamic_cast<TypeReference *>(o.getType());
        if (tr != nullptr) {
            TypeReference::DeclarationType *decl = hif::semantics::getDeclaration(tr, _sem);
            dontExpand                           = !hif::declarationIsPartOfStandard(decl);
        }

        if (o.isWrapper()) {
            // AMS
            o.getType()->acceptVisitor(*this);
            *(_outstream) << " ";
        } else {
            if (isArray) {
                // print as sc_vector
                *(_outstream) << "sc_core::sc_vector< ";
                ++_left_angular;
            }

            switch (o.getDirection()) {
            case dir_in:
                *(_outstream) << "sc_core::sc_in";
                break;
            case dir_out:
                *(_outstream) << "sc_core::sc_out";
                break;
            case dir_inout:
                *(_outstream) << "sc_core::sc_inout";
                break;
            case dir_none:
            default:
                messageAssert(o.getDirection() != dir_none, "Unxpected port direction", &o, _sem);
                break;
            }

            if (_opt.useResolved && _isSystemCResolved(o.getType())) {
                Bit *bT = dynamic_cast<Bit *>(o.getType());
                if (bT != nullptr) {
                    *(_outstream) << "_resolved ";
                } else {
                    *(_outstream) << "_rv< ";
                    ++_left_angular;
                    _printTypeSpanSize(hif::typeGetSpan(o.getType(), _sem));
                    --_left_angular;
                    *(_outstream) << " > ";
                }
            } else if (dontExpand) {
                *(_outstream) << "< ";
                ++_left_angular;
                o.getType()->acceptVisitor(*this);
                --_left_angular;
                *(_outstream) << " > ";
            } else {
                *(_outstream) << "< ";
                ++_left_angular;
                // First of 2-step to achieve a correct composite-type print
                _visitType(baseType, false);
                --_left_angular;
                *(_outstream) << " > ";
            }

            if (isArray) {
                // end of sc_vector template type
                --_left_angular;
                *(_outstream) << "> ";
            }
        }
        *(_outstream) << o.getName();

        // Second of 2-step to achieve a correct composite-type print
        if (!isRecord && !isArray) {
            _visitType(baseType, true);
        }

        *(_outstream) << ";";
        _outstream->newLine();
    } // if (! _opt.printImplementation)

    return 0;
}

auto PrintSystemCVisitor::visitProcedure(Procedure &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    auto *du = hif::getNearestParent<DesignUnit>(&o);

    if (!_opt.printImplementation) {
        if (du != nullptr && _isCppDestructor(&o)) {
            std::string dtorName = std::string("~") + du->getName();
            _printSubProgramDeclaration(o, dtorName);
            _outstream->newLine();
        } else if (_opt.printType) {
            switch (o.getKind()) {
            case SubProgram::INSTANCE:
            case SubProgram::IMPLICIT_INSTANCE:
                break;
            case SubProgram::VIRTUAL:
                *(_outstream) << "virtual ";
                break;
            case SubProgram::STATIC:
                *(_outstream) << "static ";
                break;
            case SubProgram::MACRO:
            default:
                messageError("Unsupported Function kind", &o, nullptr);
            }
            if (o.checkProperty(PROPERTY_CONSTEXPR)) {
                messageError("Constant expression not managed yet", &o, _sem);
            }
            _printFullTP(o.templateParameters);
            _outstream->newLine();
            *(_outstream) << "void ";

            _printSubProgramDeclaration(o);
        }
    } else //if ( _opt.printImplementation )
    {
        _outstream->openBlock();
        // safety check
        messageDebugAssert(!_opt.insideInitList, "Unexpected print flags", &o, _sem);

        // Global or library procedure.
        if (du == nullptr) {
            _printTypedTP(o.templateParameters);
            *(_outstream) << "void ";
            _printSubProgramImplementation(o);
            _outstream->closeBlock();
            return 0;
        }

        messageDebugAssert(!du->views.empty() && du->views.size() == 1, "Unexpected number of view", du, _sem);

        _printScopeTemplate();
        bool hasPrinted = _printTypedTP(o.templateParameters);
        if (hasPrinted) {
            _outstream->newLine();
        }

        if (!_isCppDestructor(&o)) {
            *(_outstream) << "void ";
        }

        _printScopeHierarchy();

        if (_isCppDestructor(&o)) {
            *(_outstream) << "~";
            _printSubProgramImplementation(o, _design_unit_scope.back()->getName());
        } else {
            _printSubProgramImplementation(o);
        }
        _outstream->closeBlock();
    }

    return 0;
}

auto PrintSystemCVisitor::visitSignal(Signal &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    if (_opt.printImplementation) {
        _manageInitialization(&o);
        return 0;
    }

    // if ( ! _opt.printImplementation )

    // safety check
    //messageDebugAssert( _opt.printType, "Unexpected print flags", &o, _sem );

    _printSignalTypeAndName(&o);

    *(_outstream) << ";";
    _outstream->newLine();

    return 0;
}

auto PrintSystemCVisitor::visitStateTable(StateTable &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    if (!_opt.printImplementation) {
        if (_opt.printType) {
            *(_outstream) << "void " << o.getName() << "();";
            _outstream->newLine();
        }
    } else if (_opt.printImplementation && _opt.insideConstructorBody) {
        _outstream->openBlock();
        _outstream->newLine();
        switch (o.getFlavour()) {
        case pf_method:
            *(_outstream) << "SC_METHOD";
            break;
        case pf_thread:
            *(_outstream) << "SC_THREAD";
            break;
        case pf_initial:
        case pf_hdl:
        case pf_analog:
        default:
            messageError("Unexpected flavour: " + processFlavourToString(o.getFlavour()), &o, _sem);
        }
        *(_outstream) << "( " << o.getName() << " );";
        _outstream->newLine();

        // Even if could be not required, just to be sure we
        // always increae the stack size for threads:
        if (o.getFlavour() == pf_thread) {
            *(_outstream) << "set_stack_size(0x40000);";
            _outstream->newLine();
        }

        // If required, specify to not initialize at instant 0.
        if (o.getDontInitialize()) {
            *(_outstream) << "dont_initialize();";
            _outstream->newLine();
        }

        // Manage sensitivity lists.
        if (!o.sensitivityPos.empty()) {
            _printSensitivity(o.sensitivityPos, true, false);
        }
        if (!o.sensitivityNeg.empty()) {
            _printSensitivity(o.sensitivityNeg, false, true);
        }
        if (!o.sensitivity.empty()) {
            _printSensitivity(o.sensitivity);
        }
        _outstream->closeBlock();
    } else if (_opt.printImplementation) {
        _outstream->openBlock();
        // Print just the process body. Header and trailer (if any) are
        // managed by VisitContents, but not here since StateTable could be
        // part of a Function/Procedure.

        messageDebugAssert(o.edges.empty(), "Unexpected non-empty list of edges", &o, _sem);
        //		for ( BList<Transition>::iterator it = o.edges.begin();
        //				it != o.edges.end(); ++it)
        //		{
        //			(*it)->acceptVisitor( *this );
        //		}

        // Print eventual declarations of the process.
        BackupOpt backup         = _backupVisitMode();
        _opt.printType           = true;
        _opt.printInitVal        = true;
        _opt.printImplementation = true;
        _opt.insideInitList      = false;

        for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
            (*it)->acceptVisitor(*this);
            _outstream->newLine();
        }

        // Print the actual process implementation.
        _opt.printImplementation = true;
        for (BList<State>::iterator it = o.states.begin(); it != o.states.end(); ++it) {
            (*it)->acceptVisitor(*this);
        }

        _restoreVisitMode(backup);
        _outstream->closeBlock();
    } else {
        messageDebugAssert(false, "Unexpected case", &o, _sem);
    }

    return 0;
}

auto PrintSystemCVisitor::visitTime(Time &o) -> int
{
    _printComment(&o);
    *(_outstream) << "sc_core::sc_time";
    return 0;
}

auto PrintSystemCVisitor::visitTimeValue(TimeValue &o) -> int
{
    _printComment(&o);
    *(_outstream) << "sc_core::sc_time( ";
    *(_outstream) << o.getValue();
    *(_outstream) << ", ";
    switch (o.getUnit()) {
    case TimeValue::time_fs:
        *(_outstream) << "sc_core::SC_FS";
        break;
    case TimeValue::time_ps:
        *(_outstream) << "sc_core::SC_PS";
        break;
    case TimeValue::time_ns:
        *(_outstream) << "sc_core::SC_NS";
        break;
    case TimeValue::time_us:
        *(_outstream) << "sc_core::SC_US";
        break;
    case TimeValue::time_ms:
        *(_outstream) << "sc_core::SC_MS";
        break;
    case TimeValue::time_sec:
        *(_outstream) << "sc_core::SC_SEC";
        break;
    case TimeValue::time_min:
        *(_outstream) << "sc_core::SC_MIN";
        break;
    case TimeValue::time_hr:
        *(_outstream) << "sc_core::SC_HR";
        break;
    default:
        messageError("Unexpected unit", &o, _sem);
    }
    *(_outstream) << " )";
    return 0;
}

auto PrintSystemCVisitor::visitTypeDef(TypeDef &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    // safety check
    messageDebugAssert(_opt.printType, "Unexpected print flags", &o, _sem);

    if (!o.templateParameters.empty()) {
        // Print of template aliases according to C++11.
        _printTypedTP(o.templateParameters);
        *(_outstream) << "using " << o.getName() << " = typename ";
        // First of 2-step to achieve a correct composite-type print
        _visitType(o.getType(), false);
        // Second of 2-step to achieve a correct composite-type print
        _visitType(o.getType(), true);
        *(_outstream) << ";";
        _outstream->newLine();
    } else {
        // Print "typedef" only if the type is not a record. In that case print
        // "struct foo" instead of "typedef struct foo".

        auto *robj = dynamic_cast<Record *>(o.getType());
        if (robj == nullptr) {
            *(_outstream) << "typedef ";
        }

        // First of 2-step to achieve a correct composite-type print
        _visitType(o.getType(), false);

        *(_outstream) << " " << o.getName();

        // Second of 2-step to achieve a correct composite-type print
        _visitType(o.getType(), true);

        *(_outstream) << ";";
        _outstream->newLine();
    }

    return 0;
}

auto PrintSystemCVisitor::visitTypeTP(TypeTP &o) -> int
{
    _printComment(&o);
    _printAdditionalKeywords(&o);

    *(_outstream) << "typename " << o.getName();

    if (_opt.printInitVal && o.getType() != nullptr) {
        *(_outstream) << " = ";
        o.getType()->acceptVisitor(*this);
    }

    return 0;
}

auto PrintSystemCVisitor::visitUnsigned(Unsigned &o) -> int
{
    messageDebugAssert(false, "Unsigned not supported yet.", &o, _sem);
    messageError("Unsigned not supported yet.", &o, _sem);
}

auto PrintSystemCVisitor::visitValueStatement(ValueStatement &o) -> int
{
    o.getValue()->acceptVisitor(*this);
    return 0;
}

auto PrintSystemCVisitor::visitValueTP(ValueTP &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    // Indentation etc is managed in function printTemplateParameters.
    if (_opt.printType) {
        // First of 2-step to achieve a correct composite-type print
        _visitType(o.getType(), false);
        *(_outstream) << " ";
    }

    *(_outstream) << o.getName();

    if (_opt.printType) {
        // Second of 2-step to achieve a correct composite-type print
        _visitType(o.getType(), true);
    }

    if (_opt.printInitVal && o.getValue() != nullptr) {
        *(_outstream) << " = ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

auto PrintSystemCVisitor::visitVariable(Variable &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    if (_opt.printImplementation) {
        _manageInitialization(&o);
        return 0;
    }

    // if (! _opt.printImplementation)
    //assert( _opt.printType ); // safety check
    if (_opt.printType && !o.isInstance()) {
        *(_outstream) << "static ";
    }

    // First of 2-step to achieve a correct composite-type print
    _visitType(o.getType(), false);

    *(_outstream) << " " << o.getName();

    // Second of 2-step to achieve a correct composite-type print
    _visitType(o.getType(), true);

    *(_outstream) << ";";
    _outstream->newLine();

    return 0;
}

auto PrintSystemCVisitor::visitAssign(Assign &o) -> int
{
    _printComment(&o);

    messageAssert(o.getDelay() == nullptr, "Delay should be already refined!", &o, _sem);

    bool needWrapParen = _needWrapParen(&o);
    if (needWrapParen) {
        *(_outstream) << "(";
    }

    o.getLeftHandSide()->acceptVisitor(*this);
    *(_outstream) << " = ";
    o.getRightHandSide()->acceptVisitor(*this);

    if (needWrapParen) {
        *(_outstream) << ")";
    }

    return 0;
}

auto PrintSystemCVisitor::visitIf(If &o) -> int
{
    _printComment(&o);

    for (BList<IfAlt>::iterator it = o.alts.begin(); it != o.alts.end(); ++it) {
        if (it != o.alts.begin()) {
            _outstream->newLine();
            *(_outstream) << "else ";
        }
        (*it)->acceptVisitor(*this);
    }

    if (o.defaults.empty()) {
        return 0;
    }

    _outstream->newLine();
    *(_outstream) << "else";
    _outstream->newLine();
    *(_outstream) << "{";
    _outstream->newLine();
    _outstream->indent();
    for (BList<Action>::iterator it = o.defaults.begin(); it != o.defaults.end(); ++it) {
        (*it)->acceptVisitor(*this);
        if (_isStatement(*it)) {
            *(_outstream) << ";";
        }
        _outstream->newLine();
    }
    _outstream->unindent();
    *(_outstream) << "}";

    return 0;
}

auto PrintSystemCVisitor::visitIfAlt(IfAlt &o) -> int
{
    _printComment(&o);

    *(_outstream) << "if (";
    o.getCondition()->acceptVisitor(*this);
    *(_outstream) << ")";
    _outstream->newLine();
    *(_outstream) << "{";

    if (!o.actions.empty()) {
        _outstream->newLine();
        _outstream->indent();
        for (BList<Action>::iterator it = o.actions.begin(); it != o.actions.end(); ++it) {
            (*it)->acceptVisitor(*this);
            if (_isStatement(*it)) {
                *(_outstream) << ";";
            }
            _outstream->newLine();
        }
        _outstream->unindent();
    }
    *(_outstream) << "}";

    return 0;
}

auto PrintSystemCVisitor::visitIfGenerate(IfGenerate &o) -> int
{
    messageError("If generate statements should be managed in VHDL frontend.", &o, _sem);
}

auto PrintSystemCVisitor::visitBreak(Break &o) -> int
{
    _printComment(&o);

    bool hasName = (o.getName() != NameTable::getInstance()->none());

    if (!hasName) {
        *(_outstream) << "break";
        return 0;
    }

    *(_outstream) << "goto " << o.getName() << "_break";

    return 0;
}

auto PrintSystemCVisitor::visitFile(File & /*o*/) -> int
{
    *(_outstream) << "FILE *";
    return 1;
}

auto PrintSystemCVisitor::visitFor(For &o) -> int
{
    messageDebugAssert(o.initDeclarations.empty() || o.initValues.empty(), "Unexpected for", &o, _sem);
    _printComment(&o);

    bool hasName = (o.getName() != NameTable::getInstance()->none());

    // If more than one init declaration is present, they are printed outside
    // the loop, which is inserted in a local scope.
    bool needLocalScope = (o.initDeclarations.size() > 1);

    if (needLocalScope) {
        *(_outstream) << "{";
        _outstream->indent();
        _outstream->newLine();

        BackupOpt backup = _backupVisitMode();
        _opt.printType   = true;

        for (BList<DataDeclaration>::iterator it = o.initDeclarations.begin(); it != o.initDeclarations.end(); ++it) {
            (*it)->acceptVisitor(*this);
            _outstream->newLine();
        }

        _restoreVisitMode(backup);
    }

    *(_outstream) << "for (";

    if (needLocalScope) {
        // Declarations are present but they have already been printed outside
        // loop header. The first part of loop header must be empty.
        *(_outstream) << ";";
    } else if (!o.initDeclarations.empty()) {
        // Only a declaration is present. Print it.
        BackupOpt backup = _backupVisitMode();
        _opt.printType   = true;

        o.initDeclarations.front()->acceptVisitor(*this);

        _restoreVisitMode(backup);
    } else if (!o.initValues.empty()) {
        // Initial assignment(s) are present. Print it/them.
        PrintListOpt opt{false, true, false, false, false};
        _printList(o.initValues, opt);
        *(_outstream) << ";";
    }

    *(_outstream) << " ";

    // Loop condition.
    o.getCondition()->acceptVisitor(*this);
    *(_outstream) << "; ";

    // Loop step actions.
    PrintListOpt opt{false, true, false, false, false};
    _printList(o.stepActions, opt);

    *(_outstream) << ")";
    _outstream->newLine();

    *(_outstream) << "{";
    _outstream->newLine();
    _outstream->indent();
    for (BList<Action>::iterator it = o.forActions.begin(); it != o.stepActions.end(); ++it) {
        (*it)->acceptVisitor(*this);
        *(_outstream) << ";";
        _outstream->newLine();
    }

    if (hasName) {
        *(_outstream) << o.getName() << "_continue:;\n";
    }

    _outstream->unindent();
    *(_outstream) << "}";
    _outstream->newLine();

    if (hasName) {
        *(_outstream) << o.getName() << "_break:;\n";
    }

    if (needLocalScope) {
        _outstream->unindent();
        *(_outstream) << "}";
        _outstream->newLine();
    }

    return 0;
}

auto PrintSystemCVisitor::visitForGenerate(ForGenerate &o) -> int
{
    messageDebugAssert(false, "For generate should be managed by fronted.", &o, _sem);
    messageError("For generate statement should be managed in VHDL frontend.", &o, _sem);
}

auto PrintSystemCVisitor::visitContinue(Continue &o) -> int
{
    _printComment(&o);

    bool hasName = (o.getName() != NameTable::getInstance()->none());

    if (!hasName) {
        *(_outstream) << "continue";
        return 0;
    }

    *(_outstream) << "goto " << o.getName() << "_continue";

    return 0;
}

auto PrintSystemCVisitor::visitTransition(Transition &o) -> int
{
    messageDebugAssert(false, "Transition not supported yet.", &o, _sem);
    messageError("Transition not supported yet.", &o, _sem);
}

auto PrintSystemCVisitor::visitNull(Null &o) -> int
{
    _printComment(&o);

    *(_outstream) << "; ";
    _outstream->setCommentMode(true);
    *(_outstream) << "This statement has been intentionally left empty";
    _outstream->setCommentMode(false);
    return 0;
}

auto PrintSystemCVisitor::visitProcedureCall(ProcedureCall &o) -> int { return _printCall(o); }

auto PrintSystemCVisitor::visitReturn(Return &o) -> int
{
    _printComment(&o);

    *(_outstream) << "return";
    if (o.getValue() != nullptr) {
        *(_outstream) << " ";
        o.getValue()->acceptVisitor(*this);
    }
    return 0;
}

auto PrintSystemCVisitor::visitState(State &o) -> int
{
    _printComment(&o);
    _printDefineMacros(&o);
    _printAdditionalKeywords(&o);

    for (BList<Action>::iterator it = o.actions.begin(); it != o.actions.end(); ++it) {
        (*it)->acceptVisitor(*this);
        if (_isStatement(*it)) {
            *(_outstream) << ";";
        }
        _outstream->newLine();
    }
    return 0;
}

auto PrintSystemCVisitor::visitSwitch(Switch &o) -> int
{
    _printComment(&o);

    *(_outstream) << "switch (";
    o.getCondition()->acceptVisitor(*this);
    *(_outstream) << ")";
    _outstream->newLine();
    *(_outstream) << "{";

    _outstream->newLine();
    _outstream->indent();

    // Cases
    for (BList<SwitchAlt>::iterator it = o.alts.begin(); it != o.alts.end(); ++it) {
        (*it)->acceptVisitor(*this);
    }

    // Default case
    *(_outstream) << "default:";
    _outstream->newLine();
    _outstream->indent();

    for (BList<Action>::iterator it = o.defaults.begin(); it != o.defaults.end(); ++it) {
        (*it)->acceptVisitor(*this);
        if (_isStatement(*it)) {
            *(_outstream) << ";";
        }
        _outstream->newLine();
    }

    *(_outstream) << "break;";
    _outstream->newLine();
    _outstream->unindent();

    _outstream->unindent();
    *(_outstream) << "}";

    return 0;
}

auto PrintSystemCVisitor::visitSwitchAlt(SwitchAlt &o) -> int
{
    _printComment(&o);

    for (BList<Value>::iterator it = o.conditions.begin(); it != o.conditions.end(); ++it) {
        *(_outstream) << "case ";
        (*it)->acceptVisitor(*this);
        *(_outstream) << ":";
        _outstream->newLine();
    }
    _outstream->indent();

    for (BList<Action>::iterator it = o.actions.begin(); it != o.actions.end(); ++it) {
        (*it)->acceptVisitor(*this);
        if (_isStatement(*it)) {
            *(_outstream) << ";";
        }
        _outstream->newLine();
    }

    *(_outstream) << "break;";
    _outstream->newLine();
    _outstream->unindent();
    return 0;
}

auto PrintSystemCVisitor::visitWait(Wait &o) -> int
{
    _printComment(&o);

    messageAssert(o.getCondition() == nullptr, "Conditions are not supported in SystemC", &o, _sem);
    messageAssert(o.actions.empty(), "Actions in wait() are not supported in SystemC", &o, _sem);
    bool v1 = (o.getRepetitions() != nullptr);
    bool v2 = (o.getTime() != nullptr || !o.sensitivity.empty());
    messageAssert(!(v1 && v2), "Not valid wait statement", &o, _sem);

    *(_outstream) << "wait(";
    if (o.getRepetitions() != nullptr) {
        o.getRepetitions()->acceptVisitor(*this);
    }

    if (o.getTime() != nullptr) {
        o.getTime()->acceptVisitor(*this);
    }

    if (o.getTime() != nullptr && !o.sensitivity.empty()) {
        *(_outstream) << ", ";
    }

    o.sensitivity.merge(o.sensitivityPos);
    o.sensitivity.merge(o.sensitivityNeg);

    for (BList<Value>::iterator it = o.sensitivity.begin(); it != o.sensitivity.end(); ++it) {
        if (it != o.sensitivity.begin()) {
            *(_outstream) << " |";
        }
        *(_outstream) << " ";
        (*it)->acceptVisitor(*this);
    }
    *(_outstream) << ")";
    return 0;
}

auto PrintSystemCVisitor::visitWhile(While &o) -> int
{
    _printComment(&o);

    bool hasName = (o.getName() != NameTable::getInstance()->none());

    if (!o.isDoWhile()) {
        *(_outstream) << "while (";
        o.getCondition()->acceptVisitor(*this);
        *(_outstream) << ")";
    } else {
        *(_outstream) << "do";
    }

    _outstream->newLine();
    *(_outstream) << "{";
    _outstream->newLine();
    if (!o.actions.empty()) {
        _outstream->indent();
        for (BList<Action>::iterator it = o.actions.begin(); it != o.actions.end(); ++it) {
            (*it)->acceptVisitor(*this);
            if (_isStatement(*it)) {
                *(_outstream) << ";";
            }
            _outstream->newLine();
        }
        _outstream->unindent();
    }

    if (hasName) {
        *(_outstream) << o.getName() << "_continue:;\n";
    }

    *(_outstream) << "}";

    if (o.isDoWhile()) {
        *(_outstream) << "while (";
        o.getCondition()->acceptVisitor(*this);
        *(_outstream) << ");";
    }

    if (hasName) {
        *(_outstream) << "\n" << o.getName() << "_break:;\n";
    }

    return 0;
}

void PrintSystemCVisitor::_printSensitivity(BList<Value> &sensitivity, bool isPos, bool isNeg)
{
    sensitivity.remove_dopplegangers(true);

    bool freshStart = true;

    for (BList<Value>::iterator it = sensitivity.begin(); it != sensitivity.end(); ++it) {
        _printSensitivityItem(*it, &freshStart, isPos, isNeg);
    }

    // End sensitivity.
    if (!freshStart) {
        *(_outstream) << ";";
    }
    _outstream->newLine();
}

void PrintSystemCVisitor::_printSensitivityLoop(
    Value *name,
    Type *nameType,
    bool isPos,
    bool isNeg,
    std::list<std::string> &indexes)
{
    auto *ao = dynamic_cast<Array *>(nameType);
    if (ao != nullptr) {
        Range *aoSpan = ao->getSpan();
        if (dynamic_cast<Slice *>(name) != nullptr) {
            auto *slice = dynamic_cast<Slice *>(name);
            aoSpan      = slice->getSpan();
            name        = slice->getPrefix();
        }

        auto forIndex = hif::NameTable::getInstance()->getFreshName("ind");
        indexes.push_back(forIndex);
        _printForLoopHeader(forIndex, nullptr, aoSpan, name);

        *(_outstream) << "{";
        _outstream->newLine();
        _outstream->indent();

        _printSensitivityLoop(name, ao->getType(), isPos, isNeg, indexes);

        _outstream->unindent();
        *(_outstream) << "}";
        _outstream->newLine();
    } else {
        *(_outstream) << "sensitive << ";
        name->acceptVisitor(*this);
        for (auto &indexe : indexes) {
            *(_outstream) << "[" << indexe << "]";
        }
        _printSensitivitySuffix(name, isPos, isNeg);
        *(_outstream) << ";";
        _outstream->newLine();
    }
}

void PrintSystemCVisitor::_printSensitivitySuffix(Value *name, bool isPos, bool isNeg)
{
    Declaration *deco = hif::semantics::getDeclaration(hif::getTerminalPrefix(name), _sem);
    Port *portDecl    = dynamic_cast<Port *>(deco);
    auto *sigDecl     = dynamic_cast<Signal *>(deco);

    Type *exprType = hif::semantics::getBaseType(hif::semantics::getSemanticType(name, _sem), false, _sem);
    messageAssert(exprType != nullptr, "Cannot type expression", name, _sem);

    auto *vr     = dynamic_cast<ViewReference *>(exprType);
    bool isClock = false;
    if (vr != nullptr) {
        isClock = (vr->getDesignUnit() == "sc_clock");
    }

    auto *event = dynamic_cast<Event *>(exprType);

    bool isEvent = (event != nullptr);

    if (portDecl != nullptr) {
        if (isPos) {
            *(_outstream) << ".pos()";
        } else if (isNeg) {
            *(_outstream) << ".neg()";
        }
    } else if (sigDecl != nullptr) {
        if (isPos) {
            *(_outstream) << ".posedge_event()";
        } else if (isNeg) {
            *(_outstream) << ".negedge_event()";
        }
    } else if (isEvent || isClock) {
        // no suffix
    } else {
        messageDebugAssert(false, "Unexpected case", deco, _sem);
    }
}

void PrintSystemCVisitor::_printSensitivityItem(Value *name, bool *freshStart, bool isPos, bool isNeg)
{
    // If name is referred to an array not packed,
    // print a for cycle that makes the process sensitive to all array elements.

    auto *ao =
        dynamic_cast<Array *>(hif::semantics::getBaseType(hif::semantics::getSemanticType(name, _sem), false, _sem));

    if (ao != nullptr) {
        // Stop the previous sensitivity print, if it was on going.
        if (!*freshStart) {
            *(_outstream) << ";";
            _outstream->newLine();
        }

        std::list<std::string> indexes;
        _printSensitivityLoop(name, ao, isPos, isNeg, indexes);

        // If more sensitive items have to be printed, next time start with
        // the keyword "sensitive".
        *freshStart = true;
    } else {
        // If this is the first item of the list, start with "sensitive".
        if (*freshStart) {
            *(_outstream) << "sensitive";
        }

        *(_outstream) << " << ";
        name->acceptVisitor(*this);
        _printSensitivitySuffix(name, isPos, isNeg);

        // Next time, do not start with "sensitive".
        if (*freshStart) {
            *freshStart = false;
        }
    }
}

PrintSystemCVisitor::BackupOpt::BackupOpt()
    : _opt()
    , _design_unit_scope()
    , _library_def_scope()
{
}

PrintSystemCVisitor::BackupOpt::~BackupOpt() = default;

PrintSystemCVisitor::BackupOpt::BackupOpt(const BackupOpt &b)

    = default;

auto PrintSystemCVisitor::BackupOpt::operator=(const BackupOpt &b) -> PrintSystemCVisitor::BackupOpt &
{
    if (this == &b) {
        return *this;
    }
    _opt               = b._opt;
    _design_unit_scope = b._design_unit_scope;
    _library_def_scope = b._library_def_scope;
    return *this;
}

auto PrintSystemCVisitor::_isSystemCResolved(Type *type) -> bool
{
    Type *baseT = hif::semantics::getBaseType(type, false, _sem);
    messageDebugAssert(baseT != nullptr, "Cannot find base type", type, _sem);
    return hif::typeIsResolved(baseT, _sem);
}

void PrintSystemCVisitor::_printForLoopHeader(
    const std::string &indexName,
    Type *indexType,
    Range *range,
    Object *treeObject)
{
    // In C, let's print the possible index decl outside the for.
    // This seems required by OVP... (no ANSI, just outside!)
    // @TODO: support ANSI C
    const hif::LanguageID lang = hif::objectGetLanguage(treeObject);

    // WARNING: remember to restore treeObject.
    treeObject->replace(range);

    if (lang == hif::c) {
        if (indexType != nullptr) {
            indexType->acceptVisitor(*this);
        } else {
            *(_outstream) << "int";
        }
        *(_outstream) << " " << indexName << " = ";
        range->getLeftBound()->acceptVisitor(*this);
        *(_outstream) << ";\n";
    }

    *(_outstream) << "for (";

    if (lang != hif::c) {
        // Loop index declaration
        if (indexType != nullptr) {
            indexType->acceptVisitor(*this);
        } else {
            *(_outstream) << "int";
        }
        *(_outstream) << " " << indexName << " = ";
        range->getLeftBound()->acceptVisitor(*this);
    }
    *(_outstream) << "; ";

    // Loop condition
    *(_outstream) << indexName;
    if (range->getDirection() == dir_upto) {
        *(_outstream) << " <= ";
    } else if (range->getDirection() == dir_downto) {
        *(_outstream) << " >= ";
    }
    range->getRightBound()->acceptVisitor(*this);
    *(_outstream) << "; ";

    // Loop step
    *(_outstream) << indexName << " = " << indexName;
    if (range->getDirection() == dir_upto) {
        *(_outstream) << " + ";
    } else if (range->getDirection() == dir_downto) {
        *(_outstream) << " - ";
    }
    *(_outstream) << "1"; // Default step

    *(_outstream) << ")";
    _outstream->newLine();

    // Restoring tree object.
    range->replace(treeObject);
}

auto PrintSystemCVisitor::_isFullySpecifiedArrayConst(Const *c) -> bool
{
    Type *bt = hif::semantics::getBaseType(c->getType(), false, _sem);
    auto *t  = dynamic_cast<Array *>(bt);
    if (t == nullptr) {
        return false;
    }

    auto *agg = dynamic_cast<Aggregate *>(c->getValue());
    if (agg == nullptr) {
        return false;
    }

    bool monoDimensional = (dynamic_cast<Array *>(t->getType()) == nullptr);
    return monoDimensional;
#if 0
    // TODO fix the following code, or remove it from printer as too much complicated?
    IntValue altsN(agg->alts.size());
    Value* ss = hif::semantics::spanGetSize(t->getSpan(), _sem);
    IntValue* spanD = dynamic_cast<IntValue*>( ss );
    if (spanD == nullptr) return false;

    bool fullySpec = (altsN.getValue() == spanD->getValue());
    delete spanD;
    return fullySpec;
#endif
}

void PrintSystemCVisitor::_manageInitialization(DataDeclaration *ddo)
{
    // Peculiar cases.
    if (_printTLMInit(ddo)) {
        return;
    }

    // Check if an individual initialization is needed. This is a
    // completely different management, reserved to some types of variables.
    // Otherwise, the function _printInitialization() manages all the other variants,
    // even particular initializations (e.g., instance variables, TLM constructs).
    Type *t = _needIndividualInit(ddo);
    if (t != nullptr) {
        // Must be performed inside constructor body.
        if (_opt.insideInitList) {
            *(_outstream) << ddo->getName() << "(";
            _initListInitializationSignalPort(ddo);
            *(_outstream) << ")";
            _opt.emptyInitList = false;
            return;
        }

        auto *ao = dynamic_cast<Array *>(t);
        messageDebugAssert(ao != nullptr, "Unsupported type for individual initialization", ddo, _sem);

        _printIndividualInit(ddo, ao);
        return;
    }

    // Some components must be visited twice (TLM constructs, C++ class instances,
    // Ports are already managed elsewhere).
    bool twiceInit = (dynamic_cast<Signal *>(ddo) != nullptr);
    if (_opt.insideConstructorBody && !twiceInit) {
        return;
    }

    // Manage all other kinds of initialization. May print nothing...
    bool isPrinted = _printNormalInit(ddo);

    if (_opt.insideInitList) {
        _opt.emptyInitList = false;
    }
    if (_opt.insideConstructorBody && isPrinted) {
        _outstream->newLine();
    }
}

auto PrintSystemCVisitor::_needIndividualInit(DataDeclaration *ddo) -> Type *
{
    Type *t = hif::semantics::getBaseType(ddo->getType(), false, _sem);

    // Note: for type Record a different management is provided. Instead of printing
    // an Aggregate-fashion-individual initialization, a fake constructor is
    // printed directly inside the struct defining type.

    auto *array = dynamic_cast<Array *>(t);
    if (array != nullptr) {
        return array;
    }

    return nullptr;
}

void PrintSystemCVisitor::_printIndividualInit(DataDeclaration *ddo, Array *baseT)
{
    messageAssert(baseT != nullptr, "Expected type", baseT, _sem);

    if (_opt.printType) {
        messageAssert(!_opt.insideInitList && !_opt.insideConstructorBody, "Should never happen.", nullptr, nullptr);

        // First of 2-step to achieve a correct composite-type print
        _visitType(ddo->getType(), false);

        *(_outstream) << " " << ddo->getName();

        // Second of 2-step to achieve a correct composite-type print
        _visitType(ddo->getType(), true);

        *(_outstream) << ";";
        _outstream->newLine();
    }

    // Assume that the initial value of such declaration is an aggregate.
    // Otherwise, trust it.
    auto *agg = dynamic_cast<Aggregate *>(ddo->getValue());
    if (agg == nullptr) {
        *(_outstream) << ddo->getName();
        if (ddo->getValue() != nullptr) {
            *(_outstream) << " = ";
            ddo->getValue()->acceptVisitor(*this);
        }
        *(_outstream) << ";";
        _outstream->newLine();
        return;
    }

    bool isScVector = dynamic_cast<Signal *>(ddo) != nullptr || dynamic_cast<Port *>(ddo) != nullptr;
    AggregateInfos infos;
    _fillAggregateAltInfo(agg, infos, _sem);
    for (auto &info : infos) {
        typedef std::map<Value *, std::string> IndexNames;
        IndexNames forIndexes;
        // 1- printing for loop headers
        std::string printVectorInitialization;
        for (auto j = info.indexes.begin(); j != info.indexes.end(); ++j) {
            Value *ind      = *j;
            Range *arrRange = nullptr;

            if (dynamic_cast<Aggregate *>(ind) != nullptr) {
                auto *subAggr     = dynamic_cast<Aggregate *>(ind);
                Type *subAggrType = hif::semantics::getSemanticType(subAggr, _sem);
                messageAssert(subAggrType != nullptr, "Cannot type sub aggregate", subAggr, _sem);
                auto *arr = dynamic_cast<Array *>(hif::semantics::getBaseType(subAggrType, false, _sem));
                messageAssert(arr != nullptr, "Unexpected sub aggregate base type", subAggrType, _sem);
                arrRange = arr->getSpan();
            } else if (dynamic_cast<Range *>(ind) != nullptr) {
                auto *rangeInd = dynamic_cast<Range *>(ind);
                arrRange       = rangeInd;
            }

            if (arrRange == nullptr) {
                continue;
            }

            if (!printVectorInitialization.empty()) {
                *(_outstream) << printVectorInitialization;
                *(_outstream) << ".init(";
                _printTypeSpanSize(arrRange);
                *(_outstream) << ");";
                _outstream->newLine();
                printVectorInitialization = std::string();
            }

            auto forIndex   = hif::NameTable::getInstance()->getFreshName("ind");
            forIndexes[ind] = forIndex;
            _printForLoopHeader(forIndex, nullptr, arrRange, ddo);
            *(_outstream) << "{";
            _outstream->newLine();
            _outstream->indent();

            // In case of signals and ports of array type, printer prints them
            // as sc_vector. The constructor initialization list init the
            // sc_vector with it size but cannot initialize all the signal
            // in case of sc_vector< sc_vector <T> >.
            // Ref. design: vhdl/custom/generates.
            if (isScVector && (ind != info.indexes.back())) {
                printVectorInitialization = ddo->getName();
                printVectorInitialization += "[";
                printVectorInitialization += forIndex;
                printVectorInitialization += "]";
            }
        }

        // 2- print for body
        auto *rv = dynamic_cast<RecordValue *>(info.value);
        std::string varName;
        if (_opt.useCpp98 && rv != nullptr) {
            varName = _printAggregateRecordValue(rv, ddo);
        }

        *(_outstream) << ddo->getName();
        for (auto *ind : info.indexes) {
            *(_outstream) << "[";
            if (dynamic_cast<Aggregate *>(ind) != nullptr || dynamic_cast<Range *>(ind) != nullptr) {
                // print the for index
                *(_outstream) << forIndexes[ind];
            } else {
                // normal case
                ind->acceptVisitor(*this);
            }
            *(_outstream) << "]";
        }
        *(_outstream) << " = ";

        if (varName.empty()) {
            info.value->acceptVisitor(*this);
        } else {
            *(_outstream) << varName;
        }
        *(_outstream) << ";";
        _outstream->newLine();

        // 3- closing for loops
        for (auto j = forIndexes.begin(); j != forIndexes.end(); ++j) {
            _outstream->unindent();
            *(_outstream) << "}";
            _outstream->newLine();
        }
    }
}

auto PrintSystemCVisitor::_printTLMInit(DataDeclaration *o) -> bool
{
    if (dynamic_cast<Variable *>(o) == nullptr) {
        return false;
    }

    // Check if it is a TLM component.
    if (!_isTLMComponent(o)) {
        return false;
    }

    auto *vr                     = dynamic_cast<ViewReference *>(o->getType());
    bool isTargetSocket    = vr != nullptr && vr->getDesignUnit() == std::string("tlm_target_socket");
    bool isInitiatorSocket = vr != nullptr && vr->getDesignUnit() == std::string("tlm_initiator_socket");
    bool isIOEvent         = objectMatchName(o, "io_event");
    messageAssert((isTargetSocket || isInitiatorSocket || isIOEvent), "Unexpected TLM component", o, _sem);

    if (_opt.insideInitList) {
        if (!_opt.emptyInitList) {
            *(_outstream) << ",";
            _outstream->newLine();
        }

        if (isTargetSocket) {
            *(_outstream) << o->getName() << "(\"" << o->getName() << "\")";
        } else if (isInitiatorSocket) {
            *(_outstream) << o->getName() << "(\"" << o->getName() << "\")";
        } else if (isIOEvent) {
            *(_outstream) << o->getName() << "()";
        }

        _opt.emptyInitList = false;
    } else if (_opt.insideConstructorBody) {
        if (isInitiatorSocket) {
            *(_outstream) << o->getName() << "(*this);";
            _outstream->newLine();

            if (o->getValue() != nullptr) {
                Value *valueToPrint = o->getValue();
                if (dynamic_cast<Cast *>(valueToPrint) != nullptr) {
                    Cast *cc     = dynamic_cast<Cast *>(valueToPrint);
                    valueToPrint = cc->getValue();
                }
                *(_outstream) << o->getName() << "(";
                valueToPrint->acceptVisitor(*this);
                *(_outstream) << ");";
                _outstream->newLine();
            }
        } else if (isTargetSocket) {
            *(_outstream) << o->getName() << "(*this);";
            _outstream->newLine();
        }
    }
    return true;
}

void PrintSystemCVisitor::_printRecordClasslikeMethods(hif::Record *obj)
{
    if (objectGetLanguage(obj) == hif::c) {
        return;
    }
    auto *decl = hif::getNearestParent<Declaration>(obj);
    messageAssert(decl != nullptr, "Declaration not found", obj, _sem);

    if (obj->fields.empty()) // Possible for some manipulations.
    {
        // Empty constructor and destructor. Needed at least for regression main.cc.
        _outstream->newLine();
        *(_outstream) << decl->getName() << "() {}";
        _outstream->newLine();
        *(_outstream) << "~" << decl->getName() << "() {}";
        _outstream->newLine();
        return;
    }

    // Default constructor.
    _outstream->newLine();
    *(_outstream) << decl->getName() << "():";
    _outstream->newLine();
    _outstream->indent();
    for (BList<Field>::iterator it(obj->fields.begin()); it != obj->fields.end(); ++it) {
        if (it != obj->fields.begin()) {
            *(_outstream) << ",\n";
        }
        Field *f = *it;
        *(_outstream) << f->getName() << "(";
        Type *base = hif::semantics::getBaseType(f->getType(), false, _sem);
        if (dynamic_cast<Array *>(base) == nullptr) {
            if (f->getValue() != nullptr) {
                f->getValue()->acceptVisitor(*this);
            }
        }
        *(_outstream) << ")";
    }
    _outstream->unindent();
    _outstream->newLine();
    *(_outstream) << "{}";
    _outstream->newLine();

    // Second constructor.
    _outstream->newLine();
    *(_outstream) << decl->getName() << "(";
    std::list<std::string> params;
    for (BList<Field>::iterator it(obj->fields.begin()); it != obj->fields.end(); ++it) {
        if (it != obj->fields.begin()) {
            *(_outstream) << ",";
        }
        *(_outstream) << " ";
        // First of 2-step to achieve a correct composite-type print

        // Ref design: verilog/openCores/camellia + a2t
        if (dynamic_cast<Pointer *>(hif::semantics::getBaseType((*it)->getType(), false, _sem)) == nullptr) {
            *(_outstream) << "const ";
        }
        _visitType((*it)->getType(), false);
        auto n = hif::NameTable::getInstance()->getFreshName((*it)->getName());
        params.push_back(n);
        *(_outstream) << " " << n;
        // Second of 2-step to achieve a correct composite-type print
        _visitType((*it)->getType(), true);
    }
    *(_outstream) << " ):";
    _outstream->newLine();
    _outstream->indent();
    BList<Field>::iterator fields(obj->fields.begin());
    auto pars(params.begin());
    for (; fields != obj->fields.end(); ++fields, ++pars) {
        if (fields != obj->fields.begin()) {
            *(_outstream) << ",\n";
        }
        Type *base = hif::semantics::getBaseType((*fields)->getType(), false, _sem);
        if (dynamic_cast<Array *>(base) == nullptr) {
            *(_outstream) << (*fields)->getName() << "(" << (*pars) << ")";
        } else {
            *(_outstream) << (*fields)->getName() << "()";
        }
    }
    _outstream->unindent();
    _outstream->newLine();
    *(_outstream) << "{}";
    _outstream->newLine();

    // Destructor.
    _outstream->newLine();
    *(_outstream) << "~" << decl->getName() << "()";
    _outstream->newLine();
    *(_outstream) << "{}";
    _outstream->newLine();
}

void PrintSystemCVisitor::_printRecordSignalMethods(Record *obj)
{
    const hif::LanguageID lang = objectGetLanguage(obj);
    if (lang == hif::c) {
        return;
    }
    auto *decl = hif::getNearestParent<Declaration>(obj);
    messageAssert(decl != nullptr, "Declaration not found", obj, _sem);

    // 1- operator ==
    _outstream->newLine();
    *(_outstream) << "bool operator == (const " << decl->getName() << " & other) const\n";
    *(_outstream) << "{\n";
    _outstream->indent();
    for (BList<Field>::iterator it(obj->fields.begin()); it != obj->fields.end(); ++it) {
        std::string fieldName = (*it)->getName();
        *(_outstream) << "if (" << fieldName << " != other." << fieldName << ") return false;\n";
    }
    *(_outstream) << "return true;\n";
    _outstream->unindent();
    *(_outstream) << "}\n";

    if (lang == hif::cpp) {
        return;
    }

    // 2- operator <<
    _outstream->newLine();
    *(_outstream) << "friend std::ostream & operator << (std::ostream & out, const " << decl->getName()
                  << " & other)\n";
    *(_outstream) << "{\n";
    _outstream->indent();
    for (BList<Field>::iterator it(obj->fields.begin()); it != obj->fields.end(); ++it) {
        std::string fieldName = (*it)->getName();
        *(_outstream) << "out << other." << fieldName << ";\n";
    }
    *(_outstream) << "return out;\n";
    _outstream->unindent();
    *(_outstream) << "}\n";

    // 3- sc_trace
    /*
        inline friend void sc_trace(sc_trace_file *tf, const MyType & v,
        const std::string & NAME ) {
          sc_trace(tf,v.info, NAME + ".info");
          sc_trace(tf,v.flag, NAME + ".flag");
        }
        */
    _outstream->newLine();
    *(_outstream) << "friend void sc_trace(sc_core::sc_trace_file *tf, const " << decl->getName() << " & other, "
                  << "const std::string & name)\n";
    *(_outstream) << "{\n";
    _outstream->indent();
    *(_outstream) << "using namespace sc_core;\n";
    for (BList<Field>::iterator it(obj->fields.begin()); it != obj->fields.end(); ++it) {
        Type *base = hif::semantics::getBaseType((*it)->getType(), false, _sem);

        // sc_trace of strings does not exits.
        if (dynamic_cast<String *>(base) != nullptr) {
            continue;
        }
        std::string fieldName = (*it)->getName();
        *(_outstream) << "sc_trace(tf, other." << fieldName << ", name + \"." << fieldName << "\");\n";
    }
    _outstream->unindent();
    *(_outstream) << "}\n";
}

auto PrintSystemCVisitor::_printAggregateRecordValue(RecordValue *rv, DataDeclaration *ddo) -> std::string
{
    // 1- Getting recordvalue type name
    Type *type = hif::semantics::getSemanticType(rv, _sem);
    auto *tr   = dynamic_cast<TypeReference *>(type);
    if (tr == nullptr) {
        messageError("Unexpected type", type, _sem);
    }
    auto recordTypeName = tr->getName();

    // 3- create and print support record variable: recordTypeName decl_tmp;
    auto varName = hif::NameTable::getInstance()->getFreshName(ddo->getName(), "_tmp");
    *(_outstream) << recordTypeName << " " << varName << ";";
    _outstream->newLine();

    // 4- filling all support record variable fields.
    for (BList<RecordValueAlt>::iterator i = rv->alts.begin(); i != rv->alts.end(); ++i) {
        RecordValueAlt *rva = *i;

        // print var name
        *(_outstream) << varName;

        // print .field
        *(_outstream) << "." << rva->getName();

        // print value
        *(_outstream) << " = ";
        rva->acceptVisitor(*this);

        // print ;
        *(_outstream) << ";";
        _outstream->newLine();
    }

    return varName;
}

auto PrintSystemCVisitor::_initListInitializationSignalPort(DataDeclaration *dd) -> bool
{
    auto *sig  = dynamic_cast<Signal *>(dd);
    Port *port = dynamic_cast<Port *>(dd);
    if (sig == nullptr && port == nullptr) {
        return false;
    }

    Type *t     = hif::semantics::getBaseType(dd->getType(), false, _sem);
    auto *array = dynamic_cast<Array *>(t);

    bool isArray = (array != nullptr);
    bool isWrapper =
        ((sig != nullptr && sig->isWrapper() && sig->getValue() != nullptr) ||
         (port != nullptr && port->isWrapper() && port->getValue() != nullptr));

    *(_outstream) << "\"" << dd->getName() << "\"";
    messageAssert(!(isWrapper && isArray), "Printing of sign/port wrapper of array type not supported yet", dd, _sem);

    if (isWrapper) {
        auto *ctor = dynamic_cast<FunctionCall *>(dd->getValue());
        if (ctor == nullptr || (_isCppConstructor(ctor) && !ctor->parameterAssigns.empty())) {
            *(_outstream) << ", ";
            dd->getValue()->acceptVisitor(*this);
        }
    } else if (isArray) {
        // port/signal of array type is printed as sc_vector therefore
        // the initialization use the constructor:
        // sc_vector (const char *prefix, size_type n)
        *(_outstream) << ", ";
        _printTypeSpanSize(array->getSpan());
    }

    _opt.emptyInitList = false;

    return true;
}

auto PrintSystemCVisitor::_printNormalInit(DataDeclaration *o) -> bool
{
    // "Unroll" of Pointers and ViewReferences
    auto *po   = dynamic_cast<Pointer *>(o->getType());
    auto *vref = dynamic_cast<ViewReference *>(o->getType());
    if (vref == nullptr && po != nullptr && dynamic_cast<ViewReference *>(po->getType()) != nullptr) {
        vref = dynamic_cast<ViewReference *>(po->getType());
    }

    if (_opt.insideInitList) {
        if (!_opt.emptyInitList) {
            *(_outstream) << ",";
            _outstream->newLine();
        }

        *(_outstream) << o->getName() << "(";
        if (o->getValue() != nullptr) {
            if (_initListInitializationSignalPort(o)) {
                // Signal managed. ntd
            } else {
                if (o->getValue() != nullptr) {
                    o->getValue()->acceptVisitor(*this);
                }
            }
        } else if (vref != nullptr) {
            // Type is Viewref. For RTL/TLM designs, directly instantiate them
            // calling their constructor.
            View *view = hif::semantics::getDeclaration(vref, _sem);
            messageAssert(view != nullptr, "Declaration not found", vref, _sem);
            if (view->getLanguageID() == hif::rtl || view->getLanguageID() == hif::tlm) {
                *(_outstream) << "\"" << o->getName() << "\"";
            }
        } else if (po != nullptr) {
            // Type is Pointer, initialize with nullptr and then manage it inside the
            // constructor body.
            *(_outstream) << "nullptr";
        }
        *(_outstream) << ")";
    } else // may be inside constructor body or inside other scopes.
    {
        // "Classic" initialization: var = value
        // [optional] print of type (For declarations, StateTable declarations).
        // [optional] print of initial value (sometimes missing).
        auto *vr = dynamic_cast<ViewReference *>(
            hif::typeGetNestedType(hif::semantics::getBaseType(o->getType(), false, _sem, true), _sem));
        messageDebugAssert(
            _opt.printType || o->getValue() != nullptr || vr != nullptr, "Unexpected print flags", o, _sem);

        bool isSignal = (dynamic_cast<Signal *>(o) != nullptr);
        if (dynamic_cast<Const *>(o) != nullptr) {
            // Type is already managed in visit of const
            const hif::LanguageID lang = hif::objectGetLanguage(o);
            if (lang == hif::c) {
                *(_outstream) << o->getName();
                *(_outstream) << " = ";
                o->getValue()->acceptVisitor(*this);
                *(_outstream) << ";";
                _outstream->newLine();
            } else {
                *(_outstream) << o->getName();
                *(_outstream) << "(";
                o->getValue()->acceptVisitor(*this);
                *(_outstream) << ")";
                *(_outstream) << ";";
                _outstream->newLine();
            }
            return true;
        }
        if (isSignal && dynamic_cast<Signal *>(o)->isWrapper()) {
            return false;
        }

        if (!_opt.printType && o->getValue() == nullptr && vr != nullptr) {
            return false;
        }

        if (_opt.printType) {
            if (isSignal) {
                _printSignalTypeAndName(dynamic_cast<Signal *>(o));
                *(_outstream) << "(\"";
                *(_outstream) << o->getName();
                *(_outstream) << "\");";
                return true;
            }
            o->getType()->acceptVisitor(*this);
            *(_outstream) << " ";
        }

        if (!isSignal || !_opt.printType) {
            *(_outstream) << o->getName();
        }

        if (o->getValue() == nullptr) {
            *(_outstream) << ";";
            return true;
        }

        if (dynamic_cast<RecordValue *>(o->getValue()) != nullptr) {
            // This is valid in C++98 when records have no methods.
            // So here we must use cotr instead :(
            // See visitRecord() printing.
            // Ref. design mlite + ddt + c++ + systemvue
            // and can_oc
            *(_outstream) << " = ";
            o->getValue()->acceptVisitor(*this);
            *(_outstream) << ";";
            return true;
        }
        if (_isCppConstructor(o->getValue()) && dynamic_cast<Pointer *>(o->getType()) == nullptr) {
            // Stack allocation - Note: second part of condition should be useless
            // since a call to hif_new would be expected
            if (!dynamic_cast<FunctionCall *>(o->getValue())->parameterAssigns.empty()) {
                o->getValue()->acceptVisitor(*this);
            }
            *(_outstream) << ";";
            return true;
        }

        // "Classic" initialization: var = value
        *(_outstream) << " = ";
        o->getValue()->acceptVisitor(*this);
        *(_outstream) << ";";
    }

    return true;
}

void PrintSystemCVisitor::_printSignalTypeAndName(Signal *o)
{
    Type *t            = hif::semantics::getBaseType(o->getType(), false, _sem);
    auto *array        = dynamic_cast<Array *>(t);
    bool isArray = (array != nullptr);

    if (o->isWrapper()) {
        // AMS
        // Note: assuming a "simple" type, not a composed one.
        o->getType()->acceptVisitor(*this);
        *(_outstream) << " ";
    } else {
        if (isArray) {
            Array *tmp = array;
            while (tmp != nullptr) {
                // print as sc_vector
                *(_outstream) << "sc_core::sc_vector< ";
                ++_left_angular;
                tmp = dynamic_cast<Array *>(hif::semantics::getBaseType(tmp->getType(), false, _sem, true));
            }
        }

        *(_outstream) << "sc_core::sc_signal";
        // Check for resolved signals.
        if (_opt.useResolved && _isSystemCResolved(o->getType())) {
            Bit *bT = dynamic_cast<Bit *>(o->getType());
            if (bT == nullptr) {
                *(_outstream) << "_rv< ";
                ++_left_angular;
                _printTypeSpanSize(hif::typeGetSpan(o->getType(), _sem));
                --_left_angular;
                *(_outstream) << " > ";
            } else {
                *(_outstream) << "_resolved ";
            }
        } else {
            *(_outstream) << "< ";
            ++_left_angular;
            // First of 2-step to achieve a correct composite-type print
            _visitType(o->getType(), false);
            --_left_angular;
            *(_outstream) << " > ";
        }

        if (isArray) {
            Array *tmp = array;
            while (tmp != nullptr) {
                // end of sc_vector template type
                --_left_angular;
                *(_outstream) << "> ";
                tmp = dynamic_cast<Array *>(hif::semantics::getBaseType(tmp->getType(), false, _sem, true));
            }
        }
    }

    *(_outstream) << o->getName();

    if (!o->isWrapper() && !isArray) {
        // Second of 2-step to achieve a correct composite-type print
        _visitType(o->getType(), true);
    }
}

void PrintSystemCVisitor::_visitType(Type *type, bool different_management)
{
    // Special managements.
    if (dynamic_cast<Array *>(type) != nullptr) {
        _visitTypeArray(dynamic_cast<Array *>(type), different_management);
        return;
    }
    if (dynamic_cast<Record *>(type) != nullptr) {
        _visitTypeRecord(dynamic_cast<Record *>(type), different_management);
        return;
    }
    if (dynamic_cast<Int *>(type) != nullptr) {
        _visitTypeBitField(dynamic_cast<Int *>(type), different_management);
        return;
    }

    // Other types: not affected, this call should not be forwarded.
    if (different_management) {
        return;
    }

    type->acceptVisitor(*this);
}

void PrintSystemCVisitor::_visitTypeArray(Array *type, bool use_printSquareSpan)
{
    _opt.printSquareSpan = use_printSquareSpan;
    type->acceptVisitor(*this);
    _opt.printSquareSpan = false; // Do not change this!
}

void PrintSystemCVisitor::_visitTypeRecord(Record *type, bool print_fields)
{
    _opt.printFields = print_fields;
    type->acceptVisitor(*this);
    _opt.printFields = false; // Do not change this!
}

void PrintSystemCVisitor::_visitTypeBitField(Int *type, bool print_bitField)
{
    _opt.printBitFields = print_bitField;
    type->acceptVisitor(*this);
    _opt.printBitFields = false; // Do not change this!
}

auto PrintSystemCVisitor::_isNeededTypename(ReferencedType *refType) -> bool
{
    auto *lib = dynamic_cast<Library *>(refType);
    if (lib != nullptr) {
        return false;
    }

    // Allowed ReferencedTypes
    auto *vR = dynamic_cast<ViewReference *>(refType);
    auto *tR = dynamic_cast<TypeReference *>(refType);

    // Scope nodes that own TPs
    View *viewP    = hif::getNearestParent<View>(refType->getParent());
    auto *subprogP = hif::getNearestParent<SubProgram>(refType->getParent());

    BList<Declaration> *parentList = nullptr;
    if (subprogP != nullptr) {
        parentList = &subprogP->templateParameters;
    } else if (viewP != nullptr) {
        parentList = &viewP->templateParameters;
    }

    // If the scope has template
    bool templateScope = (parentList != nullptr && !parentList->empty());

    // In case of ViewReference, its declaration may in turn own TPs
    View *vrDecl      = hif::semantics::getDeclaration(vR, _sem);
    bool tpView = (vrDecl != nullptr && !vrDecl->templateParameters.empty());

    // i.e., we are printing outside the normal scope
    if (dynamic_cast<Const *>(_opt.constManagement) != nullptr) {
        return (templateScope || tpView);
    }

    // Search for Identifiers (referred to TPs)
    hif::HifTypedQuery<Identifier> query;
    std::list<Object *> ids;
    hif::search(ids, refType, query);

    Object *inst = (vR != nullptr) ? vR->getInstance() : tR->getInstance();

    for (auto &j : ids) {
        auto *id                          = dynamic_cast<Identifier *>(j);
        Identifier::DeclarationType *decl = hif::semantics::getDeclaration(id, _sem);

        auto *vtp = dynamic_cast<ValueTP *>(decl);
        auto *ttp = dynamic_cast<TypeTP *>(decl);
        if (vtp == nullptr && ttp == nullptr) {
            continue;
        }

        // i.e., we are printing inside module constructor, no need of typename.
        if (dynamic_cast<View *>(_opt.constManagement) != nullptr) {
            continue;
        }

        if (decl->getBList() == reinterpret_cast<BList<Object> *>(parentList) && !hif::isSubNode(id, inst)) {
            continue;
        }

        return true;
    }

    return false;
}

void PrintSystemCVisitor::_printCppConstructor(FunctionCall &o)
{
    // For C++-style modeled designs, an object representing another class is
    // initialized with a call to the class constructor, and its type is
    // equal to the type of the FunctionCall instance.

    messageDebugAssert(o.templateParameterAssigns.empty(), "Unexpected template parameter assings", &o, _sem);
    messageAssert(o.getInstance() != nullptr, "Unexpected constructor without instace", &o, _sem);

    // Expecting new or init.list management to print wrapping parenthesis,
    // unless
    // (1) we are allocating on the heap (child of a "new")
    // (2) 'o' is not the initial value of some other object.
    // e.g.
    // (1) new Pippo(5)
    // (2) 5 + Pippo(4) + ...

    bool insideNew = (o.getParent() != nullptr && objectMatchName(o.getParent()->getParent(), "new"));

    auto *dd             = dynamic_cast<DataDeclaration *>(o.getParent());
    auto *pp             = dynamic_cast<Instance *>(o.getParent());
    bool isInitVal = ((dd != nullptr && &o == dd->getValue()) || (pp != nullptr && &o == pp->getValue()));

    bool independentValue = (insideNew || !isInitVal);

    if (independentValue) {
        BackupOpt backup = _backupVisitMode();
        _opt.printType   = true;

        auto *inst = dynamic_cast<Instance *>(o.getInstance());
        messageAssert(inst != nullptr, "Expected constructor instance", &o, _sem);

        inst->getReferencedType()->acceptVisitor(*this);
        _restoreVisitMode(backup);
    }

    PrintListOpt opt{independentValue, _opt.insideInitList, false, false, false};
    _printList(o.parameterAssigns, opt);

    // It can be a ctor + port binding description.
    auto *inst = dynamic_cast<Instance *>(o.getInstance());
    _printInstanceBindingStatements(inst, &o, false);
}

void PrintSystemCVisitor::_printCppDestructor(ProcedureCall &o)
{
    // Here we are modeling just explicit dtor calls, e.g.:
    // obj.~MyClass()
    o.getInstance()->acceptVisitor(*this);

    Type *type = hif::semantics::getSemanticType(o.getInstance(), _sem);
    messageAssert(type != nullptr, "Cannot find instance type", &o, _sem);
    Type *baseType = hif::semantics::getBaseType(type, false, _sem, false);
    messageAssert(baseType != nullptr, "Cannot find instance base type", &o, _sem);

    auto *pointer        = dynamic_cast<Pointer *>(baseType);
    bool isPointer = pointer != nullptr;

    Type *typeToPrint = nullptr;
    if (isPointer) {
        *(_outstream) << "->";
        typeToPrint = pointer->getType();
    } else {
        *(_outstream) << ".";
        typeToPrint = type;
    }

    auto *refType = dynamic_cast<ReferencedType *>(typeToPrint);
    if (refType == nullptr) {
        *(_outstream) << "~";
        typeToPrint->acceptVisitor(*this);
    } else {
        ReferencedType *subRef = refType->getInstance();
        if (subRef == nullptr) {
            *(_outstream) << "~";
            refType->acceptVisitor(*this);
        } else {
            //subRef->acceptVisitor(*this);
            //*(_outstream) << "::";
            *(_outstream) << "~";
            refType->setInstance(nullptr);
            refType->acceptVisitor(*this);
            refType->setInstance(subRef);
        }
    }

    *(_outstream) << "()";
}

auto PrintSystemCVisitor::_printNativeFunctionCall_new(FunctionCall &o) -> bool
{
    if (!objectMatchName(&o, "new") && !objectMatchName(&o, "placement_new")) {
        return false;
    }

    *(_outstream) << "new ";

    if (o.parameterAssigns.size() == 1 && o.templateParameterAssigns.empty() &&
        _isCppConstructor(o.parameterAssigns.front()->getValue())) {
        // Prints "new " + CLASS_NAME

        auto *ctor = dynamic_cast<FunctionCall *>(o.parameterAssigns.front()->getValue());
        messageDebugAssert(ctor != nullptr, "Expected constructor inside new", &o, _sem);

        ctor->acceptVisitor(*this);

        auto *inst = dynamic_cast<Instance *>(ctor->getInstance());
        _printInstanceBindingStatements(inst, &o, true);
    } else if (o.templateParameterAssigns.size() == 1) {
        // constructor of native types like std::string * l = new std::string("any");
        // ref. design: vhdl/gaisler/can_oc
        messageAssert(
            dynamic_cast<TypeTPAssign *>(o.templateParameterAssigns.front()), "Expected one template type parameter",
            &o, _sem);
        auto *ttpa = dynamic_cast<TypeTPAssign *>(o.templateParameterAssigns.front());
        ttpa->getType()->acceptVisitor(*this);

        PrintListOpt opt;
        opt._mandatoryParen = true;
        _printList(o.parameterAssigns, opt);
    } else if (o.templateParameterAssigns.size() == 2 && o.parameterAssigns.size() == 2) {
        // placement new: first parameter is the address
        messageAssert(!o.parameterAssigns.empty(), "Expected at least one parameter parameters", &o, _sem);
        messageAssert(
            dynamic_cast<TypeTPAssign *>(o.templateParameterAssigns.back()), "Expected one template type parameter", &o,
            _sem);

        ParameterAssign *pa = o.parameterAssigns.front();
        Value *address      = pa->getValue();
        o.parameterAssigns.remove(pa);
        *(_outstream) << "(";
        address->acceptVisitor(*this);
        *(_outstream) << ") ";

        PrintListOpt opt;
        opt._mandatoryParen   = false;
        opt._mandatoryNoParen = true;
        _printList(o.parameterAssigns, opt);

        o.parameterAssigns.push_front(pa);
    } else if (o.templateParameterAssigns.size() == 2 && o.parameterAssigns.size() == 1) {
        // placement new: first parameter is the address
        messageAssert(!o.parameterAssigns.empty(), "Expected at least one parameter parameters", &o, _sem);
        messageAssert(
            dynamic_cast<TypeTPAssign *>(o.templateParameterAssigns.back()), "Expected one template type parameter", &o,
            _sem);

        ParameterAssign *pa = o.parameterAssigns.front();
        Value *address      = pa->getValue();
        o.parameterAssigns.remove(pa);
        *(_outstream) << "(";
        address->acceptVisitor(*this);
        *(_outstream) << ") ";

        PrintListOpt opt;
        opt._mandatoryParen   = false;
        opt._mandatoryNoParen = true;

        o.templateParameterAssigns.front()->acceptVisitor(*this);
        *(_outstream) << "()";

        o.parameterAssigns.push_front(pa);
    } else {
        messageError("Wrong C++ new expression", &o, _sem);
    }
    return true;
}

auto PrintSystemCVisitor::_printNativeFunctionCall_malloc(FunctionCall &o) -> bool
{
    if (!objectMatchName(&o, "malloc")) {
        return false;
    }

    messageAssert(!o.parameterAssigns.empty(), "Expected two parameter assigns", &o, _sem);
    messageDebugAssert(o.parameterAssigns.size() == 2, "Unexpected parameterAssigns list size", &o, _sem);
    messageDebugAssert(o.templateParameterAssigns.empty(), "Unexpected template parameter assings", &o, _sem);

    // First parameter is the size, second parameter is an instance of the
    // required type.
    *(_outstream) << "(";
    o.parameterAssigns.back()->acceptVisitor(*this);
    *(_outstream) << "*) malloc ( ";
    o.parameterAssigns.front()->acceptVisitor(*this);
    *(_outstream) << " * sizeof( ";
    o.parameterAssigns.back()->acceptVisitor(*this);
    *(_outstream) << " ) )";

    if (_isStatement(&o)) {
        *(_outstream) << ";";
        _outstream->newLine();
    }

    return true;
}

void PrintSystemCVisitor::_printNativeFunctionCall(FunctionCall &o)
{
    if (_opt.insideConstructorBody) {
        return;
    }

    if (_printNativeFunctionCall_new(o)) {
        return;
    }
    if (_printNativeFunctionCall_malloc(o)) {
        return;
    }

    // Otherwise, it is not managed
    messageDebugAssert(false, "Unexpected FunctionCall", &o, _sem);
}

void PrintSystemCVisitor::_printNativeProcedureCall(ProcedureCall &o)
{
    if (objectMatchName(&o, "delete")) {
        messageAssert(!o.parameterAssigns.empty(), "Expected one parameter assign", &o, _sem);
        messageDebugAssert(o.parameterAssigns.size() == 1, "Unexpected parameterAssigns list size", &o, _sem);

        Type *paType = hif::semantics::getSemanticType(o.parameterAssigns.front()->getValue(), _sem);
        messageAssert(paType != nullptr, "Cannot type param value", o.parameterAssigns.front()->getValue(), _sem);
        Type *paBaseType = hif::semantics::getBaseType(paType, false, _sem);
        messageAssert(paBaseType != nullptr, "Cannot type param value 2", o.parameterAssigns.front()->getValue(), _sem);

        *(_outstream) << "delete ";
        if (dynamic_cast<Array *>(paBaseType) != nullptr) {
            *(_outstream) << "[] ";
        }
        PrintListOpt opt{false, false, false, false, false};
        _printList(o.parameterAssigns, opt);
    } else if (objectMatchName(&o, "free")) {
        messageAssert(!o.parameterAssigns.empty(), "Expected one parameter assign", &o, _sem);
        messageDebugAssert(o.parameterAssigns.size() == 1, "Unexpected parameterAssigns list size", &o, _sem);

        *(_outstream) << "free ";
        PrintListOpt opt{true, false, false, false, false};
        _printList(o.parameterAssigns, opt);
    } else if (objectMatchName(&o, "placement_new") && o.parameterAssigns.size() == 2) {
        // placement new: first parameter is the address
        messageAssert(!o.parameterAssigns.empty(), "Expected at least one parameter", &o, _sem);

        ParameterAssign *pa = o.parameterAssigns.front();
        Value *address      = pa->getValue();
        o.parameterAssigns.remove(pa);
        *(_outstream) << "new (";
        address->acceptVisitor(*this);
        *(_outstream) << ") ";

        PrintListOpt opt;
        opt._mandatoryParen   = false;
        opt._mandatoryNoParen = true;
        _printList(o.parameterAssigns, opt);

        o.parameterAssigns.push_front(pa);

    } else if (objectMatchName(&o, "placement_new") && o.parameterAssigns.size() == 1) {
        // placement new: first parameter is the address
        messageAssert(o.templateParameterAssigns.size() == 1, "Expected one template parameter", &o, _sem);

        ParameterAssign *pa = o.parameterAssigns.front();
        Value *address      = pa->getValue();
        o.parameterAssigns.remove(pa);
        *(_outstream) << "new (";
        address->acceptVisitor(*this);
        *(_outstream) << ") ";

        PrintListOpt opt;
        opt._mandatoryParen   = false;
        opt._mandatoryNoParen = true;

        o.templateParameterAssigns.front()->acceptVisitor(*this);
        *(_outstream) << "()";

        o.parameterAssigns.push_front(pa);

    } else {
        // Otherwise, it is not managed
        messageDebugAssert(false, "Unexpected ProcedureCall", &o, _sem);
    }
}

void PrintSystemCVisitor::_printInstanceBindingStatements(Instance *inst, Object *o, bool useArrow)
{
    if (inst == nullptr || o == nullptr || inst->portAssigns.empty()) {
        return;
    }

    auto *parentAss        = dynamic_cast<Assign *>(o->getParent());
    bool isAssignRhs = parentAss != nullptr && parentAss->getRightHandSide() == o;

    auto *var = dynamic_cast<Variable *>(o->getParent());
    Identifier id;
    bool isLocalVar = var != nullptr && getNearestParent<StateTable>(var) != nullptr;

    if (!_isStatement(o) && !isLocalVar) {
        return;
    }

    *(_outstream) << ";";
    _outstream->newLine();

    Value *lhs = nullptr;
    if (isAssignRhs) {
        lhs = parentAss->getLeftHandSide();

    } else if (isLocalVar) {
        id.setName(var->getName());
        lhs = &id; // trick to factorize following code
    } else {
        messageError("Expected parent assign or local variable", o, _sem);
    }

    for (BList<PortAssign>::iterator i = inst->portAssigns.begin(); i != inst->portAssigns.end(); ++i) {
        PortAssign *pa = *i;
        lhs->acceptVisitor(*this);
        if (useArrow) {
            *(_outstream) << "->";
        } else {
            *(_outstream) << ".";
        }
        *(_outstream) << pa->getName();
        *(_outstream) << "(";
        pa->getValue()->acceptVisitor(*this);
        *(_outstream) << ")";
        if (pa != inst->portAssigns.back()) {
            *(_outstream) << ";";
            _outstream->newLine();
        }
    }
}

auto PrintSystemCVisitor::_isStatement(Object *obj) -> bool
{
    if (dynamic_cast<FunctionCall *>(obj) != nullptr) {
        auto *fc = dynamic_cast<FunctionCall *>(obj);
        return (dynamic_cast<Action *>(fc->getParent()) != nullptr);
    }
    if (dynamic_cast<Action *>(obj) != nullptr) {
        if (dynamic_cast<Assign *>(obj) != nullptr || dynamic_cast<ProcedureCall *>(obj) != nullptr ||
            dynamic_cast<Return *>(obj) != nullptr || dynamic_cast<Break *>(obj) != nullptr ||
            dynamic_cast<Wait *>(obj) != nullptr || dynamic_cast<Continue *>(obj) != nullptr ||
            dynamic_cast<ValueStatement *>(obj) != nullptr || dynamic_cast<Null *>(obj) != nullptr) {
            return true;
        }

        // E.g. : for, if, while
        return false;
    }
    messageDebugAssert(false, "Unexpected object for _isStatament", obj, _sem);
    messageError("Unexpected Object for _isStatement", obj, _sem);
}

auto PrintSystemCVisitor::_isTLMComponent(DataDeclaration *obj) -> bool
{
    // Check the implementation language.
    if (!checkLanguage(obj, hif::tlm)) {
        return false;
    }

    auto *vr = dynamic_cast<ViewReference *>(obj->getType());
    return (vr != nullptr && vr->getDesignUnit() == std::string("tlm_target_socket")) ||
           (vr != nullptr && vr->getDesignUnit() == std::string("tlm_initiator_socket")) ||
           objectMatchName(obj, "io_event");
}

auto PrintSystemCVisitor::_isCppConstructor(Object *obj) -> bool
{
    // Call from Function may have not set name.
    auto *f = dynamic_cast<Function *>(obj);
    if (f != nullptr) {
        return (f->getName() == hif::NameTable::getInstance()->hifConstructor());
    }

    auto *fc = dynamic_cast<FunctionCall *>(obj);
    if (fc != nullptr) {
        return (fc->getName() == hif::NameTable::getInstance()->hifConstructor());
    }

    return false;
}

auto PrintSystemCVisitor::_isCppDestructor(Object *obj) -> bool
{
    return hif::objectGetName(obj) == hif::NameTable::getInstance()->hifDestructor();
}

auto PrintSystemCVisitor::_isNativeHifFunction(Object *obj) -> bool
{
    if (dynamic_cast<FunctionCall *>(obj) == nullptr) {
        return false;
    }
    if (objectMatchName(obj, "new") || objectMatchName(obj, "placement_new") || objectMatchName(obj, "malloc")) {
        return true;
    }

    return false;
}

auto PrintSystemCVisitor::_isNativeHifProcedure(Object *obj) -> bool
{
    if (dynamic_cast<ProcedureCall *>(obj) == nullptr) {
        return false;
    }
    if (objectMatchName(obj, "delete") || objectMatchName(obj, "placement_new") || objectMatchName(obj, "free")) {
        return true;
    }

    return false;
}

template <typename T> auto PrintSystemCVisitor::_needTemplateAsQualifier(T *o) -> bool
{
    // Check that FunctionCall has an instance of type ViewReference.
    if (o->getInstance() == nullptr) {
        return false;
    }
    if (!_isTemplateInstance(o->getInstance())) {
        return false;
    }

    // Check that FunctionCall has a template parameter assign (thus, it is explicit).
    if (o->templateParameterAssigns.empty()) {
        return false;
    }

    // Check that at least a parent scope is templated.
    bool found = false;
    auto *curr = hif::getNearestParent<Scope>(o);

    while (curr != nullptr) {
        View *v  = dynamic_cast<View *>(curr);
        auto *sp = dynamic_cast<SubProgram *>(curr);
        auto *td = dynamic_cast<TypeDef *>(curr);

        if (v != nullptr && !v->templateParameters.empty()) {
            found = true;
            break;
        }
        if (sp != nullptr && !sp->templateParameters.empty()) {
            found = true;
            break;
        }
        if (td != nullptr && !td->templateParameters.empty()) {
            found = true;
            break;
        }

        curr = hif::getNearestParent<Scope>(curr);
    }

    return found;
}

auto PrintSystemCVisitor::_containsOnlyIndependentComponents(LibraryDef &o) -> bool
{
    for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
        if (dynamic_cast<DesignUnit *>(*it) == nullptr && dynamic_cast<LibraryDef *>(*it) == nullptr) {
            return false;
        }
    }
    return true;
}

auto PrintSystemCVisitor::_isTemplateInstance(Value *o) -> bool
{
    Type *t = hif::semantics::getSemanticType(o, _sem);
    if (t == nullptr) {
        return false;
    }

    auto *vr = dynamic_cast<ViewReference *>(t);
    Int *ii  = dynamic_cast<Int *>(t);
    auto *bv = dynamic_cast<Bitvector *>(t);
    if (vr == nullptr && ii == nullptr && bv == nullptr) {
        return false;
    }

    if (vr != nullptr) {
        // Explicit template parameters.
        if (!vr->templateParameterAssigns.empty()) {
            return true;
        }

        // Assuming that declaration of FunctionCall is member of declaration of
        // ViewReference.
        // Check that declaration of ViewReference is templated (may be implicit).
        View *view = hif::semantics::getDeclaration(vr, _sem);
        if (view == nullptr) {
            messageDebug("Missing declaration", vr, _sem);
            messageError("Current object", o, _sem);
        }
        if (view->templateParameters.empty()) {
            return false;
        }
    } else if (ii != nullptr) {
        // if it is not sc_int or sc_uint, return.
        if (ii->getTypeVariant() == Type::NATIVE_TYPE || ii->getTypeVariant() == Type::SYSTEMC_INT_BITFIELD) {
            return false;
        }

        // sanity check
        messageAssert(
            ii->getTypeVariant() == Type::SYSTEMC_INT_SC_INT || ii->getTypeVariant() == Type::SYSTEMC_INT_SC_BIGINT,
            "Expected SystemC types", ii, _sem);
    }

    return true;
}

void PrintSystemCVisitor::_printModuleDeclaration(DesignUnit &o)
{
    // Normally there's only one View for each DesignUnit.
    messageDebugAssert(!o.views.empty() && o.views.size() == 1, "Unexpected number of views", &o, _sem);
    View *duView = o.views.front();

    if (_design_unit_scope.size() == 1) // Top module.
    {
        _printCommonHeader(o.getName());
        _printHeaderGuardBegin(o.getName());
        _printIncludes(duView->libraries, &o);

        // If the DesignUnit is part of a LibraryDef, put it in the proper namespace.
        if (!_library_def_scope.empty() && _library_def_scope.back() != nullptr) {
            _openLibraryDefNamespace(_library_def_scope.back());
        }
    } else {
        _outstream->newLine(2); // Inner module.
    }

    _printComment(&o);

    _printDefineMacros(&o);

    // Print macro and constants moved as define.
    Contents *c = duView->getContents();
    _printMacro(duView->declarations); // Note: macros are placed inside view->declarations only!
    if (c != nullptr) {
        _printConstants(c->declarations, &o, true);
    }
    _printConstants(duView->declarations, &o, true);

    _printModuleBegin(duView);

    // Public declarations.
    _outstream->newLine();
    *(_outstream) << "public:";
    _outstream->newLine(2);
    _outstream->indent();
    _opt.publicDecl = true;

    _printModuleContents(duView);

    // If RTL or TLM module, default constructor and destructor are printed.
    // CPP classes, instead, should contain the correspondent Function and Procedure.
    if (duView->getLanguageID() == hif::rtl || duView->getLanguageID() == hif::tlm) {
        _printModuleCtorDtor(duView);
    }

    _outstream->unindent();

    // Private declarations.
    _outstream->newLine();
    *(_outstream) << "private:";
    _outstream->newLine(2);
    _outstream->indent();
    _opt.publicDecl = false;

    _printModuleCopyCtorAssignOp();
    _outstream->newLine();

    _printModuleContents(duView);

    _outstream->unindent();
    _printModuleEnd();

    if (_design_unit_scope.size() == 1) // Top module
    {
        // If the DesignUnit is part of a LibraryDef, put it in the proper namespace.
        if (!_library_def_scope.empty() && _library_def_scope.back() != nullptr) {
            _closeLibraryDefNamespace(_library_def_scope.back());
        }

        _printHeaderGuardEnd(o);
    }
}

void PrintSystemCVisitor::_printModuleBegin(View *duView)
{
    _printFullTP(duView->templateParameters);
    if (!duView->templateParameters.empty()) {
        _outstream->newLine();
    }

    *(_outstream) << "class " << _design_unit_scope.back()->getName();
    _outstream->indent();

    if ((duView->getLanguageID() == hif::rtl || duView->getLanguageID() == hif::tlm) || !duView->inheritances.empty()) {
        *(_outstream) << " :";

        if (duView->getLanguageID() == hif::rtl || duView->getLanguageID() == hif::tlm) {
            _outstream->newLine();
            *(_outstream) << "public sc_core::sc_module";

            if (!duView->inheritances.empty()) {
                *(_outstream) << ",";
            }
        }

        _printModuleInheritance(duView);
    }

    _outstream->unindent();
    _outstream->newLine();
    *(_outstream) << "{";
    _outstream->newLine();
}

void PrintSystemCVisitor::_printModuleInheritance(View *view)
{
    BackupOpt backup = _backupVisitMode();
    _opt.printType   = true;

    for (BList<ViewReference>::iterator it = view->inheritances.begin(); it != view->inheritances.end(); ++it) {
        if (it != view->inheritances.begin()) {
            *(_outstream) << ",";
        }
        _outstream->newLine();
        *(_outstream) << "public ";
        (*it)->acceptVisitor(*this);
    }

    _restoreVisitMode(backup);
}

void PrintSystemCVisitor::_printModuleEnd()
{
    _outstream->newLine();
    *(_outstream) << "};";
    _outstream->newLine(2);
}

void PrintSystemCVisitor::_printModuleCtorDtor(View *duView)
{
    _outstream->openBlock();
    messageDebugAssert(
        duView->getLanguageID() == hif::rtl || duView->getLanguageID() == hif::tlm, "Unexpected language id", duView,
        _sem);

    if (!_opt.printImplementation) {
        // Constructor
        _outstream->newLine();
        *(_outstream) << "SC_HAS_PROCESS( " << _design_unit_scope.back()->getName() << " );";
        _outstream->newLine(2);
        *(_outstream) << _design_unit_scope.back()->getName() << "( sc_core::sc_module_name name_";
        _printNotCompileTimeTemplates(duView, CONST_TEMPL_CTOR_DECL);
        *(_outstream) << " );";

        _outstream->newLine();

        // Destructor
        *(_outstream) << "~" << _design_unit_scope.back()->getName() << "();";
        _outstream->newLine(2);
    } else {
        // Constructor
        _outstream->newLine();
        _printScopeTemplate();
        _printScopeHierarchy();
        *(_outstream) << _design_unit_scope.back()->getName() << "( sc_core::sc_module_name name_";
        _printNotCompileTimeTemplates(duView, CONST_TEMPL_CTOR_IMPL);
        *(_outstream) << " )";

        _printInitializationList(duView);
        _outstream->newLine();
        *(_outstream) << "{";
        _outstream->newLine();
        _outstream->indent();
        _printOtherInitializations(duView);
        _outstream->unindent();
        *(_outstream) << "}";
        _outstream->newLine(2);

        // Destructor
        _printScopeTemplate();
        _printScopeHierarchy();
        *(_outstream) << "~" << _design_unit_scope.back()->getName() << "()";
        _outstream->newLine();
        *(_outstream) << "{";
        _outstream->indent();
        _printDestructorInstanceDelete(duView);
        _outstream->unindent();
        *(_outstream) << "}";
        _outstream->newLine(2);
    }
    _outstream->closeBlock();
}

void PrintSystemCVisitor::_printModuleCopyCtorAssignOp()
{
    // Copy constructor
    *(_outstream) << _design_unit_scope.back()->getName() << "( const " << _design_unit_scope.back()->getName()
                  << " & );";
    _outstream->newLine();

    // Assignment operator
    *(_outstream) << "const " << _design_unit_scope.back()->getName() << "& operator= ( const "
                  << _design_unit_scope.back()->getName() << " & );";
    _outstream->newLine();
}

auto PrintSystemCVisitor::_printTemplateParameters(BList<Declaration> &temp_params, bool typed, bool init) -> bool
{
    BackupOpt backup         = _backupVisitMode();
    _opt.printType           = typed;
    _opt.printInitVal        = init;
    _opt.printImplementation = false;

    if (temp_params.empty()) {
        _restoreVisitMode(backup);
        return false;
    }

    if (_opt.printType) {
        *(_outstream) << "template";
    }

    PrintListOpt opt{true, false, true, false, false};
    _printList(temp_params, opt);

    _restoreVisitMode(backup);
    return true;
}

void PrintSystemCVisitor::_printNotCompileTimeTemplates(Object *o, PrintSystemCVisitor::ConstTemplateContext c)
{
    BList<Object> *refList = nullptr;
    if (dynamic_cast<View *>(o) != nullptr) {
        View *v = dynamic_cast<View *>(o);
        refList = &v->templateParameters.toOtherBList<Object>();
    } else if (dynamic_cast<ViewReference *>(o) != nullptr) {
        auto *vr = dynamic_cast<ViewReference *>(o);
        refList  = &vr->templateParameterAssigns.toOtherBList<Object>();
    } else {
        messageError("Unexpected object", o, _sem);
    }

    const hif::LanguageID lang   = hif::objectGetLanguage(o);
    bool isLangWithParents = lang == hif::rtl || lang == hif::tlm;
    bool hasDefaultParents =
        dynamic_cast<View *>(o) != nullptr && (isLangWithParents || !dynamic_cast<View *>(o)->inheritances.empty());

    ObjectList &list = _ctmList[o];
    switch (c) {
    case CONST_TEMPL_CTOR_DECL: {
        bool printTypeRestore    = _opt.printType;
        bool printInitValRestore = _opt.printInitVal;
        _opt.printType           = true;
        _opt.printInitVal        = true;
        for (auto &i : list) {
            auto n = hif::objectGetName(i);
            *(_outstream) << ", const ";
            std::string tmpName = std::string(n) + "_";
            hif::objectSetName(i, tmpName);
            refList->push_back(i);
            ;
            i->acceptVisitor(*this);
            refList->remove(i);
            hif::objectSetName(i, n);
        }
        _opt.printType    = printTypeRestore;
        _opt.printInitVal = printInitValRestore;
        break;
    }
    case CONST_TEMPL_CTOR_IMPL: {
        bool printTypeRestore    = _opt.printType;
        bool printInitValRestore = _opt.printInitVal;
        _opt.printType           = true;
        _opt.printInitVal        = false;
        for (auto &i : list) {
            auto n = hif::objectGetName(i);
            *(_outstream) << ", const ";
            std::string tmpName = std::string(n) + "_";
            hif::objectSetName(i, tmpName);
            refList->push_back(i);
            ;
            i->acceptVisitor(*this);
            refList->remove(i);
            hif::objectSetName(i, n);
        }
        _opt.printType    = printTypeRestore;
        _opt.printInitVal = printInitValRestore;
        break;
    }
    case CONST_TEMPL_CTOR_INIT_LIST: {
        bool first = true;
        for (auto &i : list) {
            if (!first || hasDefaultParents) {
                *(_outstream) << ",\n";
            }
            first  = false;
            auto n = hif::objectGetName(i);
            *(_outstream) << n << "(" << n << "_)";
        }
        if (!isLangWithParents && !list.empty() && dynamic_cast<View *>(o) != nullptr &&
            !dynamic_cast<View *>(o)->getContents()->declarations.empty()) {
            *(_outstream) << ",\n";
        }
        break;
    }
    case CONST_TEMPL_CTOR_CALL: {
        for (auto &i : list) {
            *(_outstream) << ", ";
            refList->push_back(i);
            ;
            i->acceptVisitor(*this);
            refList->remove(i);
        }
        break;
    }
    case CONST_TEMPL_DECL: {
        if (!list.empty()) {
            _outstream->unindent();
            *(_outstream) << "\nprivate:\n";
            _outstream->indent();
            for (auto &i : list) {
                *(_outstream) << "const ";
                refList->push_back(i);
                ;
                i->acceptVisitor(*this);
                refList->remove(i);
                *(_outstream) << ";\n";
            }
            _outstream->unindent();
            *(_outstream) << "\npublic:\n";
            _outstream->indent();
        }
        break;
    }
    default:
        messageError("Unexpected context", nullptr, nullptr);
    }
}

auto PrintSystemCVisitor::_needWrapParen(Object *v) -> bool
{
    Object *parent = v->getParent();

    const OperatorPecedenceEnum prec       = _getOperatorPrecedence(v);
    const OperatorPecedenceEnum parentPrec = _getOperatorPrecedence(parent);
    if (prec == prec_min) {
        return false;
    }
    if (prec == prec_concat && parentPrec != prec_concat) {
        return true;
    }
    if (parentPrec == prec_member && dynamic_cast<Member *>(parent)->getPrefix() != v) {
        return false;
    }
    if (prec >= prec_access && prec <= prec_scope) {
        return false;
    }
    if (prec == parentPrec && dynamic_cast<Expression *>(parent) != nullptr &&
        dynamic_cast<Expression *>(parent)->getValue2() == v) {
        // Needed to respect something as: a + (b + c)
        return true;
    }
    if (prec == parentPrec && (dynamic_cast<When *>(parent) != nullptr || dynamic_cast<WhenAlt *>(parent) != nullptr) &&
        dynamic_cast<When *>(v) != nullptr) {
        return true;
    }

    return (prec < parentPrec);
}

auto PrintSystemCVisitor::_getOperatorPrecedence(Object *v) -> PrintSystemCVisitor::OperatorPecedenceEnum
{
    if (v == nullptr) {
        return prec_min;
    }

    if (dynamic_cast<Expression *>(v) != nullptr) {
        auto *e = dynamic_cast<Expression *>(v);
        switch (e->getOperator()) {
        case op_or:
            return prec_or;
        case op_bor:
            return prec_bor;
        case op_and:
            return prec_and;
        case op_band:
            return prec_band;
        case op_xor:
            return prec_bxor;
        case op_bxor:
            return prec_bxor;
        case op_not:
            return prec_not_bnot;
        case op_bnot:
            return prec_not_bnot;
        case op_eq:
            return prec_eq_neq;
        case op_neq:
            return prec_eq_neq;
        case op_case_eq:
            return prec_eq_neq;
        case op_case_neq:
            return prec_eq_neq;
        case op_le:
            return prec_lt_le;
        case op_ge:
            return prec_gt_ge;
        case op_lt:
            return prec_lt_le;
        case op_gt:
            return prec_gt_ge;
        case op_sll:
            return prec_shifts;
        case op_srl:
            return prec_shifts;
        case op_sla:
            return prec_shifts;
        case op_sra:
            return prec_shifts;
        case op_rol:
            return prec_min;
        case op_ror:
            return prec_min;
        case op_plus:
            return (e->getValue2() == nullptr) ? prec_unary_plus_minus : prec_plus_minus;
        case op_minus:
            return (e->getValue2() == nullptr) ? prec_unary_plus_minus : prec_plus_minus;
        case op_mult:
            return prec_mult_div_mod;
        case op_div:
            return prec_mult_div_mod;
        case op_mod:
            return prec_mult_div_mod;
        case op_rem:
            return prec_mult_div_mod;
        case op_pow:
            return prec_min;
        case op_abs:
            return prec_min;
        case op_concat:
            return dynamic_cast<String *>(hif::semantics::getBaseType(
                       hif::semantics::getSemanticType(e, _sem), false,
                       _sem)) != nullptr
                       ? prec_plus_minus // concat between strings
                       : prec_concat;
        case op_ref:
            return prec_ref;
        case op_deref:
            return prec_deref;
        case op_andrd:
            return prec_min;
        case op_orrd:
            return prec_min;
        case op_xorrd:
            return prec_min;
        case op_assign:
            return prec_assign;
        case op_conv:
            return prec_assign;
        case op_bind:
            return prec_assign;
        case op_log:
            return prec_min;
        case op_reverse:
            return prec_min;
        case op_size:
            return prec_min;

        case op_none:
        default:
            break;
        }

        return prec_min;
    }
    if (dynamic_cast<Member *>(v) != nullptr) {
        return prec_member;
    }
    if (dynamic_cast<FunctionCall *>(v) != nullptr || dynamic_cast<ProcedureCall *>(v) != nullptr ||
        dynamic_cast<Slice *>(v) != nullptr) {
        // Slices will be translated as function calls (.range())
        return prec_call;
    }
    if (dynamic_cast<FieldReference *>(v) != nullptr) {
        FieldReference *fr = static_cast<FieldReference *>(v);

        if (dynamic_cast<Instance *>(fr->getPrefix()) != nullptr) {
            Instance *inst = static_cast<Instance *>(fr->getPrefix());
            if (dynamic_cast<Library *>(inst->getReferencedType()) != nullptr)
                return prec_scope;
            else
                return prec_access;
        }

        return prec_access;
    } else if (dynamic_cast<Instance *>(v) != nullptr) {
        Instance *inst = static_cast<Instance *>(v);
        if (dynamic_cast<Library *>(inst->getReferencedType()) != nullptr)
            return prec_scope;
        else
            return prec_access;
    } else if (
        dynamic_cast<When *>(v) != nullptr || dynamic_cast<WhenAlt *>(v) != nullptr ||
        dynamic_cast<With *>(v) != nullptr || dynamic_cast<WithAlt *>(v) != nullptr) {
        return prec_when;
    } else if (dynamic_cast<Assign *>(v) != nullptr) {
        return prec_assign;
    } else if (dynamic_cast<Cast *>(v) != nullptr) {
        const hif::LanguageID lang = hif::objectGetLanguage(v);
        return lang == hif::c ? prec_cast : prec_min;
    } else if (dynamic_cast<ValueTPAssign *>(v) != nullptr || dynamic_cast<Range *>(v) != nullptr) {
        hif::HifTypedQuery<Expression> q;
        std::list<Expression *> list;
        hif::search(list, v, q);

        bool needParen = false;
        for (std::list<Expression *>::iterator i = list.begin(); i != list.end(); ++i) {
            if ((*i)->getOperator() != op_gt)
                continue;
            needParen = true;
            break;
        }

        return (needParen) ? prec_max : prec_min;
    } else if (
        dynamic_cast<Switch *>(v) != nullptr || dynamic_cast<SwitchAlt *>(v) != nullptr ||
        dynamic_cast<For *>(v) != nullptr || dynamic_cast<IfAlt *>(v) != nullptr ||
        dynamic_cast<Aggregate *>(v) != nullptr || dynamic_cast<AggregateAlt *>(v) != nullptr ||
        dynamic_cast<RecordValueAlt *>(v) != nullptr || dynamic_cast<ParameterAssign *>(v) != nullptr ||
        dynamic_cast<DataDeclaration *>(v) != nullptr || dynamic_cast<State *>(v) != nullptr ||
        dynamic_cast<If *>(v) != nullptr || dynamic_cast<GlobalAction *>(v) != nullptr ||
        dynamic_cast<Return *>(v) != nullptr || dynamic_cast<Wait *>(v) != nullptr ||
        dynamic_cast<While *>(v) != nullptr || dynamic_cast<StateTable *>(v) != nullptr // sensitivity
    ) {
        return prec_min;
    } else if (dynamic_cast<ValueStatement *>(v) != nullptr) {
        return _getOperatorPrecedence(v->getParent());
    } else {
        messageError("Unexpected object", v, _sem);
    }
}

auto PrintSystemCVisitor::_printFullTP(BList<Declaration> &temp_params) -> int
{
    return static_cast<int>(_printTemplateParameters(temp_params, true, true));
}

auto PrintSystemCVisitor::_printTypedTP(BList<Declaration> &temp_params) -> bool
{
    return _printTemplateParameters(temp_params, true, false);
}

auto PrintSystemCVisitor::_printUntypedTP(BList<Declaration> &temp_params) -> int
{
    return static_cast<int>(_printTemplateParameters(temp_params, false, false));
}

void PrintSystemCVisitor::_printMacro(BList<Declaration> &list)
{
    bool atleastOne = false;
    for (BList<Declaration>::iterator it(list.begin()); it != list.end(); ++it) {
        auto *sp = dynamic_cast<SubProgram *>(*it);
        if (sp == nullptr || sp->getKind() != SubProgram::MACRO) {
            continue;
        }

        sp->acceptVisitor(*this);
        atleastOne = true;
    }
    if (atleastOne) {
        _outstream->newLine(2);
    }
}

auto PrintSystemCVisitor::_isAMSPort(Port *o) -> bool
{
    Type *portType = o->getType();

    // Recursively resolve Pointers and References - just for checking
    auto *p = dynamic_cast<Pointer *>(portType);
    auto *r = dynamic_cast<Reference *>(portType);
    do {
        if (p != nullptr) {
            portType = p->getType();
            p        = dynamic_cast<Pointer *>(portType);
        } else if (r != nullptr) {
            portType = r->getType();
            r        = dynamic_cast<Reference *>(portType);
        }
    } while (p != nullptr || r != nullptr);

    auto *vr = dynamic_cast<ViewReference *>(portType);
    if (vr == nullptr) {
        return false;
    }

    View *v = hif::semantics::getDeclaration(vr, _sem);
    messageAssert(v != nullptr, "Cannot find declaration", vr, _sem);

    if (!v->isStandard()) {
        return false;
    }

    auto *amsLib = hif::getNearestParent<LibraryDef>(v);
    return amsLib != nullptr && objectMatchName(amsLib, "systemc_sca_eln");
}

void PrintSystemCVisitor::_printImplementationBegin(Object *obj)
{
    std::string guardName;

    auto *du  = dynamic_cast<DesignUnit *>(obj);
    auto *ld  = dynamic_cast<LibraryDef *>(obj);
    auto *sys = dynamic_cast<System *>(obj);
    if (du != nullptr) {
        messageDebugAssert(!du->views.empty() && du->views.size() == 1, "Unexpected number of view", du, _sem);
        guardName = du->getName();
    } else if (ld != nullptr) {
        guardName = ld->getName();
    } else if (sys != nullptr) {
        guardName = NameTable::getInstance()->hifGlobals();
    } else {
        messageDebugAssert(false, "Unexpected case", obj, _sem);
    }

    _printCommonHeader(guardName);

    if (_opt.printImplementation_ihh) {
        if (ld != nullptr) {
            guardName += _dataTypesString;
        }
        _printHeaderGuardBegin(guardName);
    }

    if (!_opt.printImplementation_ihh) {
        if (!obj->checkProperty(PROPERTY_IMPLEMENTATION_INCLUDE)) {
            // could be not present in case of same scope..
            return;
        }

        TypedObject *props = obj->getProperty(PROPERTY_IMPLEMENTATION_INCLUDE);
        auto *tex          = dynamic_cast<StringValue *>(props);
        messageAssert(tex != nullptr, "Unexpected include", props, _sem);
        *(_outstream) << "#include \"" << tex->getValue() << "\"";
        _outstream->newLine();
    } else {
        std::string includeName(objectGetName(obj));
        if (ld != nullptr) {
            includeName += _dataTypesString;
        }
        *(_outstream) << "#include \"" << includeName << ".hpp\"";
        _outstream->newLine();
    }

    _outstream->newLine();
}

void PrintSystemCVisitor::_printHeaderGuardBegin(const std::string &guardName)
{
    if (_opt.printImplementation && !_opt.printImplementation_ihh) {
        return;
    }

    std::string suffix = (!_opt.printImplementation) ? "_HH" : "__I_HH";

    std::string header = _capitalize(guardName.c_str());

    *(_outstream) << "#ifndef " << header << suffix;
    _outstream->newLine();
    *(_outstream) << "#define " << header << suffix;
    _outstream->newLine(2);
}

void PrintSystemCVisitor::_printHeaderGuardEnd(Object &o, const std::string &suggestedName)
{
    if (!_opt.printImplementation && (ownTemplate(&o, false) || ownTemplate(&o, true))) {
        std::string name;
        bool isLibDef = false;
        if (dynamic_cast<DesignUnit *>(&o) != nullptr) {
            name = dynamic_cast<DesignUnit *>(&o)->getName();
        } else if (dynamic_cast<LibraryDef *>(&o) != nullptr) {
            name     = dynamic_cast<LibraryDef *>(&o)->getName();
            isLibDef = true;
        }

        messageAssert(!name.empty(), "Cannot find name", &o, _sem);

        if (!isLibDef || !suggestedName.empty()) {
            std::string tmp(suggestedName);
            if (tmp.empty()) {
                tmp = name;
            }

            _outstream->newLine();
            *(_outstream) << "#include \"" << tmp << ".i.hpp\"";
            _outstream->newLine();
        }
    }

    _outstream->newLine();
    *(_outstream) << "#endif";
    _outstream->newLine();
}

void PrintSystemCVisitor::_printModuleContents(View *view)
{
    BackupOpt backup = _backupVisitMode();

    if (!_opt.printImplementation) {
        _printModuleContents_H(view);
    } else {
        _printModuleContents_I(view);
    }

    _restoreVisitMode(backup);
}

void PrintSystemCVisitor::_printModuleContents_H(View *view)
{
    _opt.printType = true;

    if (_opt.publicDecl) {
        view->getEntity()->acceptVisitor(*this);
        _printNotCompileTimeTemplates(view, CONST_TEMPL_DECL);
        //        _outstream->newLine();
    }

    Contents *c = view->getContents();
    if (c != nullptr) {
        c->acceptVisitor(*this);
    }

    // Note: this is intended as a debug to catch unsupported components
#ifndef NDEBUG
    for (BList<Declaration>::iterator it = view->declarations.begin(); it != view->declarations.end(); ++it) {
        bool isConst = (dynamic_cast<Const *>(*it) != nullptr);

        auto *sp           = dynamic_cast<SubProgram *>(*it);
        bool isMacro = (sp != nullptr && sp->getKind() == SubProgram::MACRO);

        messageAssert(isConst || isMacro, "Unexpected object in View declarations", *it, _sem);
    }
#endif

    // Print constants not moved as define (note: AFTER other declarations).
    if (c != nullptr) {
        _printConstants(c->declarations, view->getParent(), false);
    }
    _printConstants(view->declarations, view->getParent(), false);
}

void PrintSystemCVisitor::_printModuleContents_I(View *view)
{
    _opt.printInitVal   = true;
    _opt.insideInitList = false;

    Contents *c = view->getContents();

    // Print of components that may contain template parameters by their own.
    // Note: TypeDef does not need implementation
    if (c != nullptr) {
        for (BList<Declaration>::iterator it(c->declarations.begin()); it != c->declarations.end(); ++it) {
            if (dynamic_cast<SubProgram *>(*it) == nullptr && dynamic_cast<DesignUnit *>(*it) == nullptr) {
                continue;
            }

            // Check if the component own TPs (even if View does not).
            if (!_opt.printImplementation_ihh && (ownTemplate(*it, false) || ownTemplate(view, false))) {
                continue;
            }
            if (_opt.printImplementation_ihh && !ownTemplate(*it, false) && !ownTemplate(view, false)) {
                continue;
            }

            (*it)->acceptVisitor(*this);
        }
    }

    // Print of components that inherit template parameters from View

    if (!_opt.printImplementation_ihh && ownTemplate(view, false)) {
        return;
    }
    if (_opt.printImplementation_ihh && !ownTemplate(view, false) && !ownTemplate(view, true)) {
        return;
    }

    if (c != nullptr && (!_opt.printImplementation_ihh || ownTemplate(view, false))) {
        auto *du = hif::getNearestParent<DesignUnit>(view);
        for (BList<StateTable>::iterator it(c->stateTables.begin()); it != c->stateTables.end(); ++it) {
            _printStateTable(*it, du);
        }
    }

    if (c != nullptr) {
        for (BList<Generate>::iterator it(c->generates.begin()); it != c->generates.end(); ++it) {
            if ((*it)->stateTables.empty()) {
                continue;
            }
            (*it)->acceptVisitor(*this);
        }
    }

    for (BList<Declaration>::iterator it(view->declarations.begin()); it != view->declarations.end(); ++it) {
        // skip components that are separately print

        if (dynamic_cast<Const *>(*it) != nullptr) {
            continue;
        }

        auto *sub = dynamic_cast<SubProgram *>(*it);
        if (sub != nullptr && sub->getKind() == SubProgram::MACRO) {
            continue;
        }

        if (_opt.printImplementation_ihh) {
            auto *sp = dynamic_cast<SubProgram *>(*it);
            // View's SubPrograms have already been printed in .hpp files.
            if (sp == nullptr || sp->getKind() == SubProgram::MACRO) {
                continue;
            }

            (*it)->acceptVisitor(*this);
        } else {
            // Check if the component own TPs (even if View does not).
            if (ownTemplate(*it, false)) {
                continue;
            }
            (*it)->acceptVisitor(*this);
            //            _outstream->newLine(2);
        }
    }
}

void PrintSystemCVisitor::_printModuleImplementation(DesignUnit &o)
{
    messageDebugAssert(_opt.printImplementation, "Unexpected print flags", &o, _sem);

    // Normally there's only one View for each DesignUnit.
    messageDebugAssert(!o.views.empty() && o.views.size() == 1, "Unexpected number of views", &o, _sem);
    View *duView     = o.views.front();
    BackupOpt backup = _backupVisitMode();
    Contents *c      = duView->getContents();

    // Printing top block:
    _outstream->openCommonTopBlock();

    // If nested in another DesignUnit, does not require some info.
    bool requireInclusions = (_design_unit_scope.size() == 1);
    if (requireInclusions) {
        _printImplementationBegin(&o);

        // Some libraries are added to contents and not to view, since they are
        // exploited in implementation only. Thus, print them now in addition
        // to the ones already printed in the header file.
        if (c != nullptr) {
            _printIncludes(c->libraries, &o);
        }
    }

    // Open namespace related to LibraryDef (if needed).
    if (requireInclusions && !_library_def_scope.empty()) {
        _openLibraryDefNamespace(_library_def_scope.back());
    }

    _outstream->closeCommonTopBlock();

    // Printing bottom block:
    _outstream->openCommonBottomBlock();
    *_outstream << '\n';
    // Close namespace related to LibraryDef (if needed).
    if (requireInclusions && !_library_def_scope.empty()) {
        _closeLibraryDefNamespace(_library_def_scope.back());
    }

    if (requireInclusions && _opt.printImplementation_ihh) {
        _printHeaderGuardEnd(o);
    }

    _outstream->closeCommonBottomBlock();

    // Printing content:

    // Print non-member constants
    _printConstants(duView->declarations, &o, false);
    if (c != nullptr) {
        _printConstants(c->declarations, &o, false);
    }

    // Print default module constructor and destructor for RTL and TLM modules.
    // CPP classes should contain related Function and Procedure.
    if (duView->getLanguageID() == rtl || duView->getLanguageID() == tlm) {
        if ((_opt.printImplementation_ihh && ownTemplate(duView, false)) ||
            (!_opt.printImplementation_ihh && !ownTemplate(duView, false))) {
            _printModuleCtorDtor(duView);
        }
    }

    _printModuleContents(duView);

    _restoreVisitMode(backup);
}

void PrintSystemCVisitor::_printLibraryImplementation(LibraryDef &o)
{
    messageDebugAssert(_opt.printImplementation, "Unexpected print flags", &o, _sem);

    std::string filename(_baseName);
    bool isIHH = filename.size() >= 3 && filename.substr(filename.size() - 2) == ".i";
    if (isIHH) {
        filename = filename.substr(0, filename.size() - 2);
    }
    filename += _dataTypesString;
    if (isIHH) {
        filename += ".i";
    }
    hif::backends::IndentedStream *const restore = _outstream;
    _outstream                                   = new hif::backends::IndentedStream(filename, _current_file_extension);
    _outstream->setComment("// ", "// ", "");
    // Useless cast to int64_t is to shutup a gcc 4.9.x compile warning
    const auto maxLines = hif::backends::IndentedStream::Size(int64_t(_opt.maxLines));
    if (!isIHH) {
        _outstream->setMaxLines(maxLines);
    }

    BackupOpt visRestore       = _backupVisitMode();
    _opt.insideConstructorBody = false;
    _opt.insideInitList        = false;

    bool isIndependentComps = _containsOnlyIndependentComponents(o);

    // Printing top block:
    _outstream->openCommonTopBlock();
    _printImplementationBegin(&o);
    if (!isIndependentComps) {
        // Open namespace related to LibraryDef.
        _openLibraryDefNamespace(_library_def_scope.back(), o.getName());
        _outstream->closeCommonTopBlock();
    }
    _outstream->closeCommonTopBlock();

    // Printing bottom:
    // Close namespace related to LibraryDef.
    _outstream->openCommonBottomBlock();
    if (!isIndependentComps) {
        _closeLibraryDefNamespace(&o, o.getName());
    }
    if (_opt.printImplementation_ihh) {
        _printHeaderGuardEnd(o);
    }
    _outstream->closeCommonBottomBlock();

    // Print body:
    if (!isIndependentComps) {
        _printConstants(o.declarations, &o, false);
        _printDeclarations(o.declarations, &o);
    }

    _restoreVisitMode(visRestore);

    delete _outstream;
    _outstream = restore;
}

void PrintSystemCVisitor::_printSystemImplementation(System &o)
{
    messageDebugAssert(_opt.printImplementation, "Unexpected print flags", &o, _sem);

    // Printing top block:
    _outstream->openCommonTopBlock();
    _printImplementationBegin(&o);
    _outstream->closeCommonTopBlock();

    // Printing bottom block:
    _outstream->openCommonBottomBlock();
    if (_opt.printImplementation_ihh) {
        _printHeaderGuardEnd(o);
    }
    _outstream->closeCommonBottomBlock();

    // Print body:

    _printConstants(o.declarations, &o, false);

    // Manage print of not-templated components.
    _printDeclarations(o.declarations, &o);

    for (BList<Action>::iterator it(o.actions.begin()); it != o.actions.end(); ++it) {
        _outstream->openBlock();
        (*it)->acceptVisitor(*this);
        if (_isStatement(*it)) {
            *(_outstream) << ";";
        }
        _outstream->newLine();
        _outstream->closeBlock();
    }
}

void PrintSystemCVisitor::_openLibraryDefNamespace(LibraryDef *ld, const std::string &libraryName)
{
    auto _library_name = libraryName;
    if (_library_name.empty() && !_library_def_scope.empty()) {
        _library_name = _library_def_scope.back()->getName();
    }

    if (ld->getLanguageID() == hif::c || (ld->getLanguageID() == hif::cpp && ld->hasCLinkage())) {
        (*_outstream) << "#ifdef __cplusplus\n";
        (*_outstream) << "extern \"C\" {\n";
        (*_outstream) << "#endif\n\n";
    } else {
        messageAssert(!_library_name.empty(), "Unexpected case", nullptr, nullptr);
        *(_outstream) << "namespace " << _library_name << " {";
        _outstream->newLine(2);
    }
}

void PrintSystemCVisitor::_closeLibraryDefNamespace(LibraryDef *ld, const std::string &libraryName)
{
    auto _library_name = libraryName;

    if (_library_name.empty() && !_library_def_scope.empty()) {
        _library_name = _library_def_scope.back()->getName();
    }

    if (ld->getLanguageID() == hif::c || (ld->getLanguageID() == hif::cpp && ld->hasCLinkage())) {
        (*_outstream) << "#ifdef __cplusplus\n";
        (*_outstream) << "} // end extern \"C\"\n";
        (*_outstream) << "#endif\n\n";
    } else {
        messageAssert(!_library_name.empty(), "Unexpected case", nullptr, nullptr);
        *(_outstream) << "} // end namespace " << _library_name;
        _outstream->newLine(2);
    }
}

void PrintSystemCVisitor::_printLibraryComponentInclusion(
    LibraryDef *ld,
    const std::string &guardName,
    const std::string &includeName)
{
    _printCommonHeader(guardName);
    _printHeaderGuardBegin(guardName);

    (*_outstream) << "#include \"" << includeName << "\"\n\n";

    for (BList<Declaration>::iterator i = ld->declarations.begin(); i != ld->declarations.end(); ++i) {
        Declaration *decl = *i;
        auto *du          = dynamic_cast<DesignUnit *>(decl);
        auto *libDef      = dynamic_cast<LibraryDef *>(decl);
        if (du == nullptr && libDef == nullptr) {
            continue;
        }

        if (du != nullptr) {
            if (du->checkProperty(PROPERTY_TYPDEF_DESIGN_UNIT)) {
                continue; // typedef
            }
            View *view = nullptr;
            if (!du->views.empty()) {
                view = du->views.front();
            }
            std::string ext = "." + _opt.headersExtension;
            if (view == nullptr || view->isStandard()) {
                continue;
            }
            if (view->getLanguageID() == hif::c || (ld->getLanguageID() == hif::cpp && ld->hasCLinkage())) {
                ext = ".h";
            }
            (*_outstream) << "#include \"" << du->getName() << ext << "\"\n";
        } else // libDef
        {
            if (libDef->isStandard()) {
                continue;
            }
            std::string ext = "." + _opt.headersExtension;
            if (libDef->getLanguageID() == hif::c || (ld->getLanguageID() == hif::cpp && ld->hasCLinkage())) {
                ext = ".h";
            }
            (*_outstream) << "#include \"" << libDef->getName() << "/" << libDef->getName() << ext << "\"\n";
        }
    }

    _printHeaderGuardEnd(*ld);
}

void PrintSystemCVisitor::_printDestructorInstanceDelete(View *duView)
{
    // TODO upgrade this considering also __hif_delete
    bool del = false;

    Contents *c = duView->getContents();

    if (c != nullptr) {
        for (BList<Instance>::iterator it(c->instances.begin()); it != c->instances.end(); ++it) {
            Type *it_type = hif::semantics::getSemanticType(*it, _sem);

            if (dynamic_cast<Pointer *>(it_type) == nullptr) {
                continue;
            }

            _outstream->newLine();
            *(_outstream) << "delete " << (*it)->getName() << ";";
            del = true;
        }
    }

    if (del) {
        _outstream->newLine();
    }
}

auto PrintSystemCVisitor::_needInitializationList(View *view) -> bool
{
    if (!view->getEntity()->ports.empty()) {
        return true;
    }

    Contents *c = view->getContents();

    if (c != nullptr) {
        for (BList<Declaration>::iterator it = c->declarations.begin(); it != c->declarations.end(); ++it) {
            if (dynamic_cast<Function *>(*it) != nullptr || dynamic_cast<Procedure *>(*it) != nullptr ||
                dynamic_cast<TypeDef *>(*it) != nullptr) {
                continue;
            }

            auto *ddo = dynamic_cast<DataDeclaration *>(*it);
            if (ddo == nullptr) {
                continue;
            }

            // Individual initialization.
            if (_needIndividualInit(ddo) != nullptr) {
                continue;
            }

            // Member constants are initialized in constructor initialization list.
            if (dynamic_cast<Const *>(*it) != nullptr && dynamic_cast<Const *>(*it)->isInstance() &&
                !_isFullySpecifiedArrayConst(dynamic_cast<Const *>(*it))) {
                return true;
            }

            // Other type of Variable are initialized in constructor initialization list.
            if (dynamic_cast<Variable *>(*it) != nullptr) {
                return true;
            }
        }

        if (!c->instances.empty()) {
            return true;
        }
    }

    for (BList<Declaration>::iterator it = view->declarations.begin(); it != view->declarations.end(); ++it) {
        if (dynamic_cast<Function *>(*it) != nullptr || dynamic_cast<Procedure *>(*it) != nullptr ||
            dynamic_cast<TypeDef *>(*it) != nullptr) {
            continue;
        }

        auto *ddo = dynamic_cast<DataDeclaration *>(*it);
        if (ddo == nullptr) {
            continue;
        }

        // Individual initialization.
        if (_needIndividualInit(ddo) != nullptr) {
            continue;
        }

        // Member constants are initialized in constructor initialization list.
        if (dynamic_cast<Const *>(*it) != nullptr && dynamic_cast<Const *>(*it)->isInstance() &&
            !_isFullySpecifiedArrayConst(dynamic_cast<Const *>(*it))) {
            return true;
        }

        // Other type of Variable are initialized in constructor initialization list.
        if (dynamic_cast<Variable *>(*it) != nullptr) {
            return true;
        }
    }

    return false;
}

auto PrintSystemCVisitor::_printInitializationList(View *view) -> int
{
    bool isModule = view->getLanguageID() == hif::rtl || view->getLanguageID() == hif::tlm;

    if (!isModule && !_needInitializationList(view)) {
        return 0;
    }

    *(_outstream) << " :";
    _outstream->newLine();
    _outstream->indent();

    if (isModule) {
        *(_outstream) << "sc_core::sc_module( name_ )";
    }

    BackupOpt backup           = _backupVisitMode();
    _opt.printInitVal          = true;
    _opt.insideInitList        = true;
    _opt.insideConstructorBody = false;
    _opt.emptyInitList         = !isModule;

    view->getEntity()->acceptVisitor(*this);
    _printNotCompileTimeTemplates(view, CONST_TEMPL_CTOR_INIT_LIST);
    Contents *c = view->getContents();
    if (c != nullptr) {
        c->acceptVisitor(*this);
    }

    _printConstants(view->declarations, view->getParent(), false);
    if (c != nullptr) {
        _printConstants(c->declarations, view->getParent(), false);
    }

    _outstream->unindent();

    _restoreVisitMode(backup);
    return 0;
}

auto PrintSystemCVisitor::_printOtherInitializations(View *view) -> int
{
    BackupOpt backup           = _backupVisitMode();
    _opt.printInitVal          = true;
    _opt.insideInitList        = false;
    _opt.insideConstructorBody = true;

    view->getEntity()->acceptVisitor(*this);
    Contents *c = view->getContents();
    if (c != nullptr) {
        c->acceptVisitor(*this);
    }
    _printConstants(view->declarations, view->getParent(), false);
    if (c != nullptr) {
        _printConstants(c->declarations, view->getParent(), false);
    }

    _restoreVisitMode(backup);
    return 0;
}

void PrintSystemCVisitor::_printConstants(BList<Declaration> &declarations, Object *scope, bool onlyDefines)
{
    bool isInModule = (dynamic_cast<DesignUnit *>(scope) != nullptr);

    // LibraryDef, System do not own template parameters
    if (!isInModule && _opt.printImplementation_ihh) {
        return;
    }

    // Constants of non-template modules must be printed inside .cc
    bool templateModule =
        (isInModule && !dynamic_cast<DesignUnit *>(scope)->views.front()->templateParameters.empty());

    bool spacing = false;
    _outstream->openBlock(); // print all constants in same file
    for (BList<Declaration>::iterator it(declarations.begin()); it != declarations.end(); ++it) {
        if (dynamic_cast<Const *>(*it) == nullptr) {
            continue;
        }
        if (!templateModule && _opt.printImplementation_ihh) {
            continue; // ref.design: vhdl/openCores/can_top
        }

        auto *co = dynamic_cast<Const *>(*it);

        // Some constants have to be translated as define for further use.
        if (co->isDefine() && !onlyDefines) {
            continue;
        }
        if (!co->isDefine() && onlyDefines) {
            continue;
        }

        co->acceptVisitor(*this);
        spacing = true;
    }
    _outstream->closeBlock();

    if (spacing) {
        _outstream->newLine();
    }
}

void PrintSystemCVisitor::_printCommonHeader(const std::string &filename)
{
    *(_outstream) << "/// @file " << filename << "\n";
    *(_outstream) << "/// @brief This file was generated by hif2sc.\n";
    *(_outstream) << "/// @details\n";
    *(_outstream) << "/// Generate with HIF version " << hif::application_utils::getHIFVersion() << ".\n\n";
}

void PrintSystemCVisitor::_printLibraryDeclaration(LibraryDef &o)
{
    const std::string filename(_outstream->getBaseName() + _dataTypesString);
    hif::backends::IndentedStream *const restore = _outstream;
    _outstream = new hif::backends::IndentedStream(filename, _outstream->getExtension());
    _outstream->setComment("// ", "// ", "");

    const std::string guardName = o.getName() + _dataTypesString;
    _printCommonHeader(filename);
    _printHeaderGuardBegin(guardName);

    _printIncludes(o.libraries, nullptr);
    if (!o.libraries.empty()) {
        _outstream->newLine();
    }

    // Print constants moved as define.
    _printConstants(o.declarations, &o, true);

    _printDeclarations(o.declarations, &o, false);

    if (!_containsOnlyIndependentComponents(o)) {
        _openLibraryDefNamespace(&o);

        _printDeclarations(o.declarations, &o, true);

        // Print constants not moved as define (note: AFTER other declarations).
        _printConstants(o.declarations, &o, false);

        _closeLibraryDefNamespace(&o);
    }

    _printHeaderGuardEnd(o, guardName);

    delete _outstream;
    _outstream = restore;

    _printLibraryComponentInclusion(
        &o, o.getName() + _outstream->getExtension(),
        o.getName() + _dataTypesString + "." + _outstream->getExtension());
}

void PrintSystemCVisitor::_printSystemDeclaration(System &o)
{
    _printCommonHeader(NameTable::getInstance()->hifGlobals());
    _printHeaderGuardBegin(NameTable::getInstance()->hifGlobals());
    _printIncludes(o.libraries, nullptr);

    // Print constants moved as define.
    _printConstants(o.declarations, &o, true);

    _printDeclarations(o.declarations, &o, true);

    // Print constants not moved as define (note: AFTER other declarations).
    _printConstants(o.declarations, &o, false);

    _printHeaderGuardEnd(o);
}

void PrintSystemCVisitor::_printDeclarations(
    BList<Declaration> &declarations,
    Object *startingObj,
    bool insideNamespace)
{
    BackupOpt backup = _backupVisitMode();

    if (!_opt.printImplementation) {
        _printDeclarations_H(declarations, startingObj, insideNamespace);
    } else {
        _printDeclarations_I(declarations, startingObj);
    }

    _restoreVisitMode(backup);
}

void PrintSystemCVisitor::_printDeclarations_H(
    BList<Declaration> &declarations,
    Object *startingObj,
    bool insideNamespace)
{
    _opt.printType    = true;
    _opt.printInitVal = true;

    // Some conditionals are considered for LibraryDef only
    auto *ld = dynamic_cast<LibraryDef *>(startingObj);

    for (BList<Declaration>::iterator it = declarations.begin(); it != declarations.end(); ++it) {
        // DesignUnit contained in the LibraryDef or System are printed into separated files.
        if (dynamic_cast<DesignUnit *>(*it) != nullptr) {
            auto *du = dynamic_cast<DesignUnit *>(*it);
            if (!du->checkProperty(PROPERTY_TYPDEF_DESIGN_UNIT)) {
                continue;
            }
            if (!insideNamespace) {
                continue;
            }

            // typedef inside design unit
            if (dynamic_cast<DesignUnit *>(startingObj) != nullptr) {
                continue;
            }

            bool prevIsTDefDU = false;
            bool nextIsTDefDU = false;
            if (it != declarations.begin()) {
                BList<Declaration>::iterator tmp(it);
                --tmp;
                auto *prevDU = dynamic_cast<DesignUnit *>(*tmp);
                prevIsTDefDU = prevDU != nullptr && prevDU->checkProperty(PROPERTY_TYPDEF_DESIGN_UNIT);
            }
            if (it != declarations.end()) {
                BList<Declaration>::iterator tmp(it);
                ++tmp;
                auto *nextDU = dynamic_cast<DesignUnit *>(*tmp);
                nextIsTDefDU = nextDU != nullptr && nextDU->checkProperty(PROPERTY_TYPDEF_DESIGN_UNIT);
            }

            if (ld != nullptr && !prevIsTDefDU) {
                _closeLibraryDefNamespace(ld);
            }

            *(_outstream) << "#include \"" << du->getName() << ".hpp\"\n";
            if (ld != nullptr && !nextIsTDefDU) {
                *(_outstream) << "\n";
                _openLibraryDefNamespace(ld);
            }

            continue;
        }
        // Const are managed by _printConstants()
        if (dynamic_cast<Const *>(*it) != nullptr) {
            continue;
        }
        if (dynamic_cast<Variable *>(*it) != nullptr) {
            if (ld != nullptr && !insideNamespace) {
                continue;
            }
            _opt.printType = false;
            *(_outstream) << "extern ";
        } else if (dynamic_cast<Signal *>(*it) != nullptr) {
            if (ld != nullptr && !insideNamespace) {
                continue;
            }
            _opt.printType = false;
            *(_outstream) << "extern ";
        } else {
            _opt.printType = true;
        }

        // Do not print other declarations outside LibraryDef namespace.
        if (ld != nullptr && !insideNamespace) {
            continue;
        }

        _opt.printInitVal = true;
        (*it)->acceptVisitor(*this);
    }

    _outstream->newLine();
}

void PrintSystemCVisitor::_printDeclarations_I(BList<Declaration> &declarations, Object *startingObj)
{
    // Note (about template params) this method is called from LibraryDef and System only.

    // As first, check if the visit is unnecessary.
    if (!_opt.printImplementation_ihh && ownTemplateOnly(startingObj, true)) {
        return;
    }
    if (_opt.printImplementation_ihh && !ownTemplate(startingObj, true)) {
        return;
    }

    _opt.printInitVal           = true;
    _opt.insideInitList         = false;
    bool restorePrintType = _opt.printType;

    for (BList<Declaration>::iterator it(declarations.begin()); it != declarations.end(); ++it) {
        bool isMethod   = (dynamic_cast<Function *>(*it) != nullptr || dynamic_cast<Procedure *>(*it) != nullptr);
        bool isVariable = (dynamic_cast<Variable *>(*it) != nullptr);
        bool isSignal   = (dynamic_cast<Signal *>(*it) != nullptr);
        if (!isMethod && !isVariable && !isSignal) {
            continue;
        }

        // Check if the component own TPs.
        if (!_opt.printImplementation_ihh && ownTemplate(*it, false)) {
            continue;
        }
        if (_opt.printImplementation_ihh && !ownTemplate(*it, false)) {
            continue;
        }

        if (isVariable || isSignal) {
            _outstream->openBlock();
            // needed by Variables only, which are defined as "extern" in header
            _opt.printType = true;
        }

        (*it)->acceptVisitor(*this);

        if (isVariable || isSignal) {
            _opt.printType = restorePrintType;
            _outstream->newLine(2);
            _outstream->closeBlock();
        }
    }
}

auto PrintSystemCVisitor::_calculateInclude(Object *where, Library *lib) -> std::string
{
    if (lib->isStandard()) {
        return "";
    }

    typedef std::vector<LibraryDef *> Libs;
    Libs includes;

    // Calculating include info:
    Library *currLib = lib;
    while (currLib != nullptr) {
        Library::DeclarationType *l = hif::semantics::getDeclaration(currLib, _sem);
        //messageAssert(l != nullptr, "Declaration not found", currLib, _sem);
        if (l == nullptr) {
            return "";
        }
        includes.push_back(l);
        currLib = dynamic_cast<Library *>(currLib->getInstance());
    }

    // Calculating include relative path:
    auto *currPrefix = hif::getNearestParent<LibraryDef>(where, true);
    std::string dots;
    Libs::size_type i = 0;
    if (!lib->isSystem()) {
        if (currPrefix == nullptr) {
            i = 1;
        }
        while (currPrefix != nullptr) {
            bool found = false;
            for (i = 0; i < includes.size(); ++i) {
                if (includes[i] != currPrefix) {
                    continue;
                }
                found = true;
                break;
            }
            if (found) {
                break;
            }
            if (dots.empty()) {
                dots = "..";
            } else {
                dots += "/..";
            }
            currPrefix = hif::getNearestParent<LibraryDef>(currPrefix, false);
        }
    }

    if (!includes.empty() && includes.back()->isStandard()) {
        // Printing "absolute" path for standard libs.
        // Ref design OSTC step 3.
        dots = std::string();
    }

    // Calculating descending std::string:
    Library *l = lib;
    std::string dirs;
    for (; i > 0; --i) {
        if (dirs.empty()) {
            dirs = std::string(l->getName());
        } else {
            dirs = std::string(l->getName()) + "/" + dirs;
        }
        ReferencedType *rt = l->getInstance();
        l                  = dynamic_cast<Library *>(rt);
    }

    if (dots.empty()) {
        return dirs;
    }
    if (dirs.empty()) {
        return dots;
    }
    return dots + "/" + dirs;
}

void PrintSystemCVisitor::_printIncludes(BList<Library> &list, DesignUnit *du)
{
    typedef std::set<std::string> Includes;
    Includes system;
    Includes local;
    for (BList<Library>::iterator it(list.begin()); it != list.end(); ++it) {
        Library *lib = *it;

        // Avoid the print of standard libraries.
        if (lib->isStandard()) {
            continue;
        }

        // Print required macro (if any).
        _printDefineMacros(lib);

        LibraryDef *decl        = hif::semantics::getDeclaration(lib, _sem);
        std::string includePath = _calculateInclude(list.getParent(), lib);
        bool isSystem     = lib->isSystem();

        if (lib->getFilename().empty()) {
            if (!includePath.empty()) {
                includePath += "/";
            }
            includePath += lib->getName();
            if (du != nullptr && decl != nullptr && !decl->isStandard()) {
                // decl can be null for example in case of including of cstdint.
                // In this case it is assumed declaration is standard.
                includePath += _dataTypesString;
            }
            std::string ext = ".";
            if (objectGetLanguage(decl) == hif::c) {
                ext += "h";
            } else if (isSystem) {
                ext += "hpp";
            } else {
                ext += _opt.headersExtension;
            }
            includePath += ext;
        } else {
            includePath = lib->getFilename();
        }

        // For std decls, ask to sem!
        if (decl != nullptr && decl->isStandard()) {
            std::string tmp(_sem->getStandardFilename(lib->getName()));
            if (!tmp.empty()) {
                includePath = tmp;
            }
        }

        if (isSystem) {
            std::string tmp("#include <" + includePath + ">");
            system.insert(tmp);
        } else {
            std::string tmp("#include \"" + includePath + "\"");
            local.insert(tmp);
        }
    }

    for (const auto &inc : system) {
        *_outstream << inc << '\n';
    }
    for (const auto &inc : local) {
        *_outstream << inc << '\n';
    }

    _outstream->newLine();
}

void PrintSystemCVisitor::_printSubProgramDeclaration(SubProgram &o, const std::string &altName)
{
    if (o.isStandard()) {
        return;
    }

    if (altName.empty()) {
        *(_outstream) << o.getName();
    } else {
        *(_outstream) << altName;
    }

    *(_outstream) << "(";

    if (!o.parameters.empty()) {
        *(_outstream) << " ";
        for (BList<Parameter>::iterator it = o.parameters.begin(); it != o.parameters.end(); ++it) {
            if (it != o.parameters.begin()) {
                *(_outstream) << ", ";
            }
            (*it)->acceptVisitor(*this);
        }
        *(_outstream) << " ";
    }

    *(_outstream) << ")";

    if (_opt.printType) {
        *(_outstream) << ";";
        _outstream->newLine(2);
    }
}

void PrintSystemCVisitor::_printSubProgramImplementation(hif::SubProgram &o, const std::string &altName)
{
    if (o.isStandard()) {
        return;
    }
    bool restore_pType = _opt.printType;
    _opt.printType     = false;
    _printSubProgramDeclaration(o, altName);
    _opt.printType = restore_pType;

    _outstream->newLine();
    *(_outstream) << "{";

    if (o.getStateTable() != nullptr) {
        _outstream->newLine();
        _outstream->indent();
        bool restore = _opt.printInitVal;
        _opt.printInitVal  = true;
        o.getStateTable()->acceptVisitor(*this);
        _opt.printInitVal = restore;
        _outstream->unindent();
    }

    *(_outstream) << "}";
    _outstream->newLine(3);
}

void PrintSystemCVisitor::_printTypeSpanSize(Range *span)
{
    Value *size = hif::semantics::spanGetSize(span, _sem);
    Value *tmp  = span->setLeftBound(size);
    size->acceptVisitor(*this);
    delete span->setLeftBound(tmp);
}

template <class T> void PrintSystemCVisitor::_printList(BList<T> &list, PrintListOpt opt)
{
    if (list.empty() && !opt._mandatoryParen) {
        return;
    }

    if (list.empty() && opt._mandatoryParen) {
        if (opt._angularParen) {
            *(_outstream) << "<>";
        } else if (opt._curlyParen) {
            *(_outstream) << "{}";
        } else {
            *(_outstream) << "()";
        }
        return;
    }

    if (!opt._mandatoryNoParen) {
        if (opt._angularParen) {
            *(_outstream) << "<";
            ++_left_angular;
        } else if (opt._curlyParen) {
            *(_outstream) << "{";
        } else {
            *(_outstream) << "(";
        }

        if (!opt._breakLine) {
            *(_outstream) << " ";
        }
        _outstream->indent();
    }

    for (typename BList<T>::iterator it(list.begin()); it != list.end(); ++it) {
        if (opt._breakLine) {
            if (it != list.begin()) {
                *(_outstream) << ",";
            }
            _outstream->newLine();
        } else // inline
        {
            if (it != list.begin()) {
                *(_outstream) << ", ";
            }
        }
        (*it)->acceptVisitor(*this);
    }

    if (!opt._mandatoryNoParen) {
        _outstream->unindent();

        if (!opt._breakLine) {
            *(_outstream) << " ";
        }
        if (opt._angularParen) {
            *(_outstream) << ">";
            --_left_angular;
        } else if (opt._curlyParen) {
            *(_outstream) << "}";
        } else {
            *(_outstream) << ")";
        }
    }
}

void PrintSystemCVisitor::_printComment(Object *o)
{
    if (!o->hasComments()) {
        return;
    }

    if (_opt.printImplementation && !_opt.printType) {
        return;
    }

    _outstream->newLine();
    for (auto &it : o->getComments()) {
        // A new line is necessary to not interfere with current printing.
        _outstream->setCommentMode(true);
        *(_outstream) << it;
        _outstream->setCommentMode(false);
        _outstream->newLine();
    }
}

void PrintSystemCVisitor::_printDefineMacros(Object *o)
{
    if (o->checkProperty(PROPERTY_REQUIRED_MACRO)) {
        TypedObject *pp = o->getProperty(PROPERTY_REQUIRED_MACRO);
        auto *def       = dynamic_cast<StringValue *>(pp);
        *(_outstream) << def->getValue();
        _outstream->newLine();
    }
    if (!_opt.printImplementation && o->checkProperty(PROPERTY_REQUIRED_MACRO_HH)) {
        TypedObject *pp = o->getProperty(PROPERTY_REQUIRED_MACRO_HH);
        auto *def       = dynamic_cast<StringValue *>(pp);
        *(_outstream) << def->getValue();
        _outstream->newLine();
    }
    if (_opt.printImplementation && o->checkProperty(PROPERTY_REQUIRED_MACRO_CC)) {
        TypedObject *pp = o->getProperty(PROPERTY_REQUIRED_MACRO_CC);
        auto *def       = dynamic_cast<StringValue *>(pp);
        *(_outstream) << def->getValue();
        _outstream->newLine();
    }
}

void PrintSystemCVisitor::_printAdditionalKeywords(Declaration *o)
{
    if (!o->hasAdditionalKeywords()) {
        return;
    }
    //Sanity check
    if (dynamic_cast<Variable *>(o) == nullptr && dynamic_cast<SubProgram *>(o) == nullptr &&
        dynamic_cast<Field *>(o) == nullptr && dynamic_cast<Parameter *>(o) == nullptr) {
        messageError("Unexpected object with addtional keywods", o, _sem);
    }
    if (dynamic_cast<Variable *>(o) != nullptr && _opt.printImplementation) {
        return;
    }
    auto it = o->getAdditionalKeywordsBeginIterator();
    for (; it != o->getAdditionalKeywordsEndIterator(); ++it) {
        *(_outstream) << *it << " ";
    }
}

void PrintSystemCVisitor::_printScopeTemplate()
{
    // Note: libraries are managed using namespaces.

    for (auto it(_design_unit_scope.begin()); it != _design_unit_scope.end(); ++it) {
        messageDebugAssert(!(*it)->views.empty() && (*it)->views.size() == 1, "Unexpected number of views", *it, _sem);

        View *v = (*it)->views.front();
        if (v->templateParameters.empty()) {
            continue;
        }
        bool hasPrinted = _printTypedTP(v->templateParameters);
        if (hasPrinted) {
            _outstream->newLine();
        }
    }
}

void PrintSystemCVisitor::_printScopeHierarchy()
{
    for (auto it(_design_unit_scope.begin()); it != _design_unit_scope.end(); ++it) {
        (*_outstream) << (*it)->getName();

        messageDebugAssert(!(*it)->views.empty() && (*it)->views.size() == 1, "Unexpected number of views", *it, _sem);
        View *v = (*it)->views.front();
        if (!v->templateParameters.empty()) {
            _printUntypedTP(v->templateParameters);
        }

        (*_outstream) << "::";
    }
}

void PrintSystemCVisitor::_printInitialize(Port &o, Type *portType, std::list<std::string> &indexes, bool isAMS)
{
    // FIXME Ad-hoc fix (2). An additional cast is needed, because
    // SystemC does not allow "direct" initialization.
    // E.g.:
    // sc_dt::sc_bv<4> a;
    // a.initialize("0000");                is not correct
    // a.initialize(sc_dt::sc_bv("0000"));  is correct

    Type *baseT = hif::semantics::getInstantiatedType(portType, _sem);
    messageAssert(baseT != nullptr, "Instantiated type not found", portType, _sem);
    auto *arrT = dynamic_cast<Array *>(baseT);
    if (arrT == nullptr) {
        // Base cases.
        if (indexes.empty()) {
            // Simple initialize.
            *(_outstream) << o.getName();

            if (dynamic_cast<Pointer *>(portType) != nullptr) {
                *(_outstream) << "->";
            } else {
                *(_outstream) << ".";
            }

            *(_outstream) << "initialize( ";

            if (isAMS) {
                *(_outstream) << "\"" << o.getName() << "\", ";
            }

            o.getValue()->acceptVisitor(*this);
            *(_outstream) << " );";
            _outstream->newLine();
        } else {
            messageAssert(!isAMS, "Unsupported", &o, _sem);

            // Base type of array.
            *(_outstream) << o.getName();

            for (auto &indexe : indexes) {
                *(_outstream) << "[" << indexe << "]";
            }

            if (dynamic_cast<Pointer *>(portType) != nullptr) {
                *(_outstream) << "->";
            } else {
                *(_outstream) << ".";
            }

            *(_outstream) << "initialize( ";
            o.getValue()->acceptVisitor(*this);
            *(_outstream) << " );";
            _outstream->newLine();
        }
    } else {
        // Need of a for statement.
        Range *aoSpan = arrT->getSpan();

        auto forIndex = hif::NameTable::getInstance()->getFreshName("ind");
        indexes.push_back(forIndex);
        _printForLoopHeader(forIndex, nullptr, aoSpan, &o);

        *(_outstream) << "{";
        _outstream->newLine();
        _outstream->indent();

        _printInitialize(o, arrT->getType(), indexes, isAMS);

        _outstream->unindent();
        *(_outstream) << "}";
        _outstream->newLine();
    }

    delete baseT;
}

void PrintSystemCVisitor::_printPortBinding(
    const std::string &instName,
    PortAssign *o,
    Type *t,
    std::list<std::string> &indexes)
{
    auto *arrT = dynamic_cast<Array *>(t);
    if (arrT == nullptr) {
        // Base cases.
        if (indexes.empty()) {
            // Simple initialize.
            *(_outstream) << instName << "." << o->getName() << "( ";
            o->getValue()->acceptVisitor(*this);
            *(_outstream) << " );";
            _outstream->newLine();
        } else {
            *(_outstream) << instName << "." << o->getName();

            // Base type of array.
            for (auto &indexe : indexes) {
                *(_outstream) << "[" << indexe << "]";
            }

            *(_outstream) << "( ";
            o->getValue()->acceptVisitor(*this);

            for (auto &indexe : indexes) {
                *(_outstream) << "[" << indexe << "]";
            }

            *(_outstream) << " );";
            _outstream->newLine();
        }
    } else {
        // Need of a for statement.
        Range *aoSpan = arrT->getSpan();

        auto forIndex = hif::NameTable::getInstance()->getFreshName("ind");
        indexes.push_back(forIndex);
        _printForLoopHeader(forIndex, nullptr, aoSpan, o);

        *(_outstream) << "{";
        _outstream->newLine();
        _outstream->indent();

        _printPortBinding(instName, o, arrT->getType(), indexes);

        _outstream->unindent();
        *(_outstream) << "}";
        _outstream->newLine();
    }
}

auto PrintSystemCVisitor::_capitalize(const char *str) -> std::string
{
    std::string ret(str);
    bool fresh = false;
    for (size_t i = 0; i < ret.length(); ++i) {
        ret[i] = static_cast<char>(toupper(ret[i]));
        if ((::isalnum(ret[i]) == 0) && ret[i] != '_') {
            ret[i] = '_';
            fresh  = true;
        }
    }

    if (fresh) {
        ret = NameTable::getInstance()->getFreshName(ret);
    }
    return ret;
}

auto PrintSystemCVisitor::_isInt32Type(Expression *e) -> bool
{
    Type *t = hif::semantics::getSemanticType(e, _sem);
    messageAssert(e != nullptr, "Cannot type expressione", e, _sem);
    Int *i = dynamic_cast<Int *>(t);
    if (i == nullptr) {
        return false;
    }
    unsigned long long size = hif::semantics::spanGetBitwidth(i->getSpan(), _sem);
    return (size == 32ULL);
}

auto PrintSystemCVisitor::_mayBeAmbiguous(IntValue *o) -> bool
{
    Object *lastValue = o;
    auto *parentExpr  = dynamic_cast<Expression *>(lastValue->getParent());
    while (parentExpr != nullptr) {
        if (!_isInt32Type(parentExpr)) {
            // Found not int32 expression.. cannot be ambiguous!
            return false;
        }

        lastValue  = parentExpr;
        parentExpr = dynamic_cast<Expression *>(lastValue->getParent());
    }

    Object *parent = lastValue->getParent();
    if (parent == nullptr) {
        // assume is not ambiguous
        return false;
    }

    auto *rr = dynamic_cast<Range *>(parent);
    if (rr != nullptr) {
        return false;
    }

    auto *aa = dynamic_cast<Assign *>(parent);
    if (aa != nullptr) {
        return (aa->getRightHandSide() != lastValue);
    }

    auto *ret = dynamic_cast<Return *>(parent);
    if (ret != nullptr) {
        return false;
    }

    auto *agg = dynamic_cast<Aggregate *>(parent); // ref. design: itc99/b12
    if (agg != nullptr) {
        return false;
    }

    auto *mm = dynamic_cast<Member *>(parent);
    if (mm != nullptr) {
        return (mm->getIndex() != lastValue);
    }

    auto *dd = dynamic_cast<DataDeclaration *>(parent);
    if (dd != nullptr) {
        return (dd->getValue() != lastValue);
    }

    Cast *cc = dynamic_cast<Cast *>(parent);
    if (cc != nullptr) {
        return (cc->getValue() != lastValue);
    }

    auto *pp = dynamic_cast<ParameterAssign *>(parent);
    if (pp != nullptr) {
        Parameter *param = hif::semantics::getDeclaration(pp, _sem);
        messageAssert(param != nullptr, "Declaration not found", pp, _sem);
        return (!hif::declarationIsPartOfStandard(param));
    }

    auto *vtpa = dynamic_cast<ValueTPAssign *>(parent);
    if (vtpa != nullptr) {
        ValueTP *vtp = hif::semantics::getDeclaration(vtpa, _sem);
        messageAssert(vtp != nullptr, "Declaration not found", pp, _sem);
        return (!hif::declarationIsPartOfStandard(vtp));
    }

    return true;
}

auto PrintSystemCVisitor::_mayBeAmbiguous(StringValue *o) -> bool
{
    Object *parent = o->getParent();
    if (parent == nullptr) {
        // assume is not ambiguous
        return false;
    }
    Type *base = hif::semantics::getBaseType(o->getType(), false, _sem, true);
    if (dynamic_cast<String *>(base) == nullptr) {
        // E.g. char *
        return false;
    }

    auto *e = dynamic_cast<Expression *>(parent);
    if (e != nullptr) {
        return true;
    }

    auto *fc = dynamic_cast<FunctionCall *>(parent);
    if (fc != nullptr) {
        return true;
    }

    auto *pc = dynamic_cast<ProcedureCall *>(parent);
    if (pc != nullptr) {
        return true;
    }

    auto *pp = dynamic_cast<ParameterAssign *>(parent);
    if (pp != nullptr) {
        Parameter *param = hif::semantics::getDeclaration(pp, _sem);
        messageAssert(param != nullptr, "Declaration not found", pp, _sem);
        return (!hif::declarationIsPartOfStandard(param));
    }

    return false;
}

template <typename T> auto PrintSystemCVisitor::_printCall(T &o) -> int
{
    _printComment(&o);

    if (_isNativeHifFunction(&o)) {
        auto *fc = dynamic_cast<FunctionCall *>(&o);
        messageAssert(fc != nullptr, "Unexpected object.", &o, _sem);
        _printNativeFunctionCall(*fc);
        return 0;
    }

    if (_isNativeHifProcedure(&o)) {
        auto *fc = dynamic_cast<ProcedureCall *>(&o);
        messageAssert(fc != nullptr, "Unexpected object.", &o, _sem);
        _printNativeProcedureCall(*fc);
        return 0;
    }

    if (_isCppConstructor(&o)) {
        auto *fc = dynamic_cast<FunctionCall *>(&o);
        messageAssert(fc != nullptr, "Unexpected object.", &o, _sem);
        _printCppConstructor(*fc);
        return 0;
    }

    if (_isCppDestructor(&o)) {
        auto *pc = dynamic_cast<ProcedureCall *>(&o);
        messageAssert(pc != nullptr, "Unexpected object.", &o, _sem);
        _printCppDestructor(*pc);
        return 0;
    }

    SubProgram *dec = hif::semantics::getDeclaration(&o, _sem);
    if (dec == nullptr) {
        messageError("Function declaration not found", &o, _sem);
    }

    bool needWrapParen = _needWrapParen(&o);
    if (needWrapParen) {
        *(_outstream) << "(";
    }

    if (o.getInstance() != nullptr) {
        // Print instance and FunctionCall access type (static/instance).
        if (dec != nullptr && dec->getKind() == SubProgram::STATIC) {
            auto *inst = dynamic_cast<Instance *>(o.getInstance());
            messageAssert((inst != nullptr), "Unexpected SubProgram instance", &o, _sem);

            Type *t    = hif::semantics::getSemanticType(inst, _sem);
            auto *vref = dynamic_cast<ViewReference *>(t);
            auto *lib  = dynamic_cast<Library *>(t);
            messageAssert(vref != nullptr || lib != nullptr, "Unexpected type for SubProgram instance.", t, _sem);

            if (vref != nullptr) {
                vref->acceptVisitor(*this);
                *(_outstream) << "::";
            } else if (lib != nullptr) {
                if (!lib->isStandard()) {
                    lib->acceptVisitor(*this);
                    *(_outstream) << "::";
                }
            }
        } else {
            Type *t = hif::semantics::getSemanticType(o.getInstance(), _sem);
            if (dynamic_cast<Library *>(t) != nullptr) {
                auto *lib = dynamic_cast<Library *>(t);

                if (!lib->isStandard() && dec->getKind() != SubProgram::MACRO) {
                    // Simply print the namespace.
                    t->acceptVisitor(*this);
                    *(_outstream) << "::";
                }
            } else {
                o.getInstance()->acceptVisitor(*this);
                // The property forceArrow is set by TLM refinement to manage
                // corner cases
                if (o.checkProperty(PROPERTY_TLM_FORCEARROW)) {
                    *(_outstream) << "->";
                } else {

                    if (dynamic_cast<Pointer *>(t) != nullptr) {
                        *(_outstream) << "->";
                    } else {
                        *(_outstream) << ".";
                    }
                }
            }
        }
    }

    if (_needTemplateAsQualifier(&o)) {
        *(_outstream) << "template ";
    }

    *(_outstream) << o.getName();

    PrintListOpt opt{false, false, true, false, false};
    _printList(o.templateParameterAssigns, opt);

    opt._mandatoryParen = true;
    opt._angularParen   = false;
    _printList(o.parameterAssigns, opt);

    if (needWrapParen) {
        *(_outstream) << ")";
    }

    // Management of final ';' is performed by o parent, if any.
    return 0;
}
