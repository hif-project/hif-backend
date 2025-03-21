/// @file PrintImplementationVisitor.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/backends/printFileMethods.hpp>

#include "hif2sc/PrintMethods.hpp"

using std::endl;
using std::ofstream;
using std::string;
using namespace hif;

namespace
{

// /////////////////////////////////////////////////////////////////////////////
// PrintImplementationVisitor
// /////////////////////////////////////////////////////////////////////////////

/// @brief Visitor to generate source files.
class PrintImplementationVisitor : public hif::GuideVisitor
{

public:
    /// @brief Constructor.
    PrintImplementationVisitor(hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList);

    /// @brief Destructor.
    ~PrintImplementationVisitor() override;

    /// @name Refinement methods.
    //@{

    auto visitDesignUnit(hif::DesignUnit &o) -> int override;
    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitSystem(hif::System &o) -> int override;

    //@}

private:
    PrintImplementationVisitor(const PrintImplementationVisitor &)                     = delete;
    auto operator=(const PrintImplementationVisitor &) -> PrintImplementationVisitor & = delete;

    /// @brief Wrapper for calls to PrintSystemCVisitor.
    /// @param o The object to print.
    /// @param fileName The name of the file to write on (differenced basing
    /// on the kind of object).
    /// @param headerImplementation Used to distinguish between print of
    /// templated and non-templated components.
    void _printImplementation(hif::Object &o, const string &fileName, bool headerImplementation);

    auto _getSourceExtensionByLanguage(Object *obj) -> std::string;
    auto _getSourceFilename(
        const std::string &outDir,
        const std::string &libDefName,
        Object *obj,
        const std::string &fileName) -> std::string;
    auto _getHeaderInlineFilename(
        const std::string &outDir,
        const std::string &libDefName,
        Object *obj,
        const std::string &fileName) -> std::string;

    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_outstream{nullptr};

    /// @brief The main output directory, used to create the directory hierarchy
    /// related to this HIF tree.
    string _outdirName;

    /// @name Flags related to nested components.
    //@{

    /// @brief The subdirectory relative to the library definition (if any).
    hif::LibraryDef *_currentLibraryDef{nullptr};

    //@}

    /// @name Command-line flags of hif2sc
    /// @{

    bool _useResolved;
    bool _useCpp98;
    bool _useHDTLib;
    const uint64_t _maxLines;
    const std::string _sourcesExtension;
    const std::string _headersExtension;

    /// @}

    /// @brief The map of constant template.
    PrintSystemCVisitor::ConstTemplateMap &_ctmList;

    /// @brief The semantics
    hif::semantics::SystemCSemantics *_sem;
};

PrintImplementationVisitor::PrintImplementationVisitor(
    hif2scParseLine &cLine,
    PrintSystemCVisitor::ConstTemplateMap &ctmList)
    : _outdirName(cLine.getOutputDirectory())
    , _useResolved(cLine.useResolved())
    , _useCpp98(cLine.useCpp98())
    , _useHDTLib(cLine.useHDTLib())
    , _maxLines(cLine.getMaxLines())
    , _sourcesExtension(cLine.getSourcesExtension())
    , _headersExtension(cLine.getHeadersExtension())
    , _ctmList(ctmList)
    , _sem(hif::semantics::SystemCSemantics::getInstance())
{
    hif::application_utils::initializeLogHeader("HIF2SC", "PrintImplementationVisitor");
}

PrintImplementationVisitor::~PrintImplementationVisitor()
{
    hif::application_utils::restoreLogHeader();
    delete _outstream;
}

auto PrintImplementationVisitor::visitDesignUnit(DesignUnit &o) -> int
{
    messageAssert(!o.views.empty() && o.views.size() == 1, "Unsupported more than one view", &o, nullptr);
    View *duView = o.views.front();

    // skip standard ones
    if (duView->isStandard()) {
        return 0;
    }

    std::string ldName;
    if (_currentLibraryDef != nullptr) {
        ldName = _currentLibraryDef->getName();
    }

    // If the DesignUnit contains components own TPs, print their implementation.
    std::string ihhFName = _getHeaderInlineFilename(_outdirName, ldName, &o, "");
    _printImplementation(o, ihhFName, true);

    // If the DesignUnit contains only components that own TPs, their implementation
    // has already been printed.
    std::string srcFName = _getSourceFilename(_outdirName, ldName, &o, "");
    _printImplementation(o, srcFName, false);

    return 0;
}

auto PrintImplementationVisitor::visitLibraryDef(LibraryDef &o) -> int
{
    // Standard libraries (e.g., tlm)
    if (o.isStandard()) {
        return 0;
    }

    // Note: even if LibraryDef contains only DesignUnits or other LibraryDef,
    // the file is still generated to preserve the hierarchical structure.

    LibraryDef *restore = _currentLibraryDef;
    _currentLibraryDef  = &o;

    // Recursive call.
    GuideVisitor::visitLibraryDef(o);

    std::string ldName;
    if (_currentLibraryDef != nullptr) {
        ldName = _currentLibraryDef->getName();
    }

    // If the LibraryDef contains components that own TPs, print their implementation.
    _printImplementation(o, _getHeaderInlineFilename(_outdirName, ldName, &o, ldName), true);

    // If the LibraryDef contains only components that own TPs, their implementation
    // has already been printed.
    _printImplementation(o, _getSourceFilename(_outdirName, ldName, &o, ldName), false);

    _currentLibraryDef = restore;
    return 0;
}

auto PrintImplementationVisitor::visitSystem(System &o) -> int
{
    // Recursive call.
    GuideVisitor::visitSystem(o);

    if (o.declarations.empty() && o.actions.empty()) {
        return 0;
    }

    // If the System declarations contain components that own TPs, print their
    // implementation.
    const std::string headerFile =
        _getHeaderInlineFilename(_outdirName, "", &o, NameTable::getInstance()->hifGlobals());
    _printImplementation(o, headerFile, true);

    // If the System declarations contain only components that own TPs, their
    // implementation has already been printed.
    const std::string sourceFile = _getSourceFilename(_outdirName, "", &o, NameTable::getInstance()->hifGlobals());
    _printImplementation(o, sourceFile, false);

    return 0;
}

void PrintImplementationVisitor::_printImplementation(Object &o, const string &fileName, bool headerImplementation)
{
    if (fileName.empty()) {
        return;
    }

    if (_outstream != nullptr) {
        delete _outstream;
        _outstream = nullptr;
    }

    std::string f;
    std::string ext;
    hif::backends::splitFileName(fileName, f, ext);

    auto *lib = dynamic_cast<LibraryDef *>(&o);
    if (lib == nullptr) {
        std::ofstream ofs;
        if (hif::backends::openFileStream(fileName, &ofs) == 0) {
            messageError("Unable to create implementation output stream for " + fileName, nullptr, nullptr);
        }
        ofs.close();

        _outstream = new hif::backends::IndentedStream(f, ext);
        _outstream->setComment("// ", "// ", "");
        const auto maxLines = hif::backends::IndentedStream::Size(_maxLines);
        if (!headerImplementation) {
            _outstream->setMaxLines(maxLines);
        }
    }

    PrintSystemCVisitorOptions opt;
    opt.printInitVal            = true;
    opt.printImplementation     = true;
    opt.printImplementation_ihh = headerImplementation;
    opt.useResolved             = _useResolved;
    opt.useHDTLib               = _useHDTLib;
    opt.useCpp98                = _useCpp98;
    opt.maxLines                = _maxLines;
    opt.sourcesExtension        = _sourcesExtension;
    opt.headersExtension        = _headersExtension;
    PrintSystemCVisitor vis(_outstream, opt, _ctmList, f, ext);
    vis.setCurrentLibraryDef(_currentLibraryDef);
    o.acceptVisitor(vis);

    if (_outstream != nullptr) {
        *(_outstream) << '\n'; // Flush the stream buffer.
    }
}

auto PrintImplementationVisitor::_getSourceExtensionByLanguage(Object *obj) -> std::string
{
    const hif::LanguageID lang = objectGetLanguage(obj);

    if (lang == hif::c) {
        return ".c";
    }

    // rtl, tlm, psl, cpp
    return "." + _sourcesExtension;
}

auto PrintImplementationVisitor::_getSourceFilename(
    const std::string &outDir,
    const std::string &libDefName,
    Object *obj,
    const std::string &fileName) -> std::string
{
    string ret;
    if (obj == nullptr) {
        return ret;
    }

    // A template DesignUnit will always be printed in .i.hpp file.
    // A non-template DesignUnit will always have a .cc containing at least
    // its ctor and dtor.
    // LibraryDef and System must check their children.

    if (ownTemplate(obj, false)) {
        return ret;
    }

    auto *du = dynamic_cast<DesignUnit *>(obj);
    if (du == nullptr && ownTemplateOnly(obj, true)) {
        return ret;
    }
    messageAssert(du != nullptr || !fileName.empty(), "Unexpected case", nullptr, nullptr);
    string fn = (du == nullptr) ? fileName : du->getName();
    string ld = (libDefName.empty()) ? "" : std::string(libDefName) + "/";

    ret = outDir + "/src/" + ld + fn + _getSourceExtensionByLanguage(obj);
    return ret;
}

auto PrintImplementationVisitor::_getHeaderInlineFilename(
    const std::string &outDir,
    const std::string &libDefName,
    Object *obj,
    const std::string &fileName) -> std::string
{
    string ret;
    if (obj == nullptr) {
        return ret;
    }

    // A template DesignUnit will always be printed in .i.hpp file.
    // A non-template DesignUnit must check its children.
    // LibraryDef and System must check their children.

    auto *du = dynamic_cast<DesignUnit *>(obj);
    if (du != nullptr && !ownTemplate(obj, false) && !ownTemplate(obj, true)) {
        return ret;
    }
    if (du == nullptr && !ownTemplate(obj, true)) {
        return ret;
    }

    messageAssert(du != nullptr || !fileName.empty(), "Unexpected case", nullptr, nullptr);

    PrintSystemCVisitor vis(_ctmList);
    string fn = (du == nullptr) ? fileName : du->getName();
    string ld = (libDefName.empty()) ? "" : std::string(libDefName) + "/";

    ret = outDir + "/inc/" + ld + fn + ".i.hpp";
    return ret;
}

} // namespace

void printImplementations(System *sys, hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList)
{
    PrintImplementationVisitor writeImpl(cLine, ctmList);
    sys->acceptVisitor(writeImpl);
}
