/// @file PrintHeaderVisitor.cpp
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
using std::vector;
using namespace hif;

namespace
{

// /////////////////////////////////////////////////////////////////////////////
// PrintHeaderVisitor
// /////////////////////////////////////////////////////////////////////////////

/// @brief Visitor to generate header files.
class PrintHeaderVisitor : public hif::GuideVisitor
{

public:
    /// @brief Constructor.
    PrintHeaderVisitor(hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList);

    /// @brief Destructor.
    ~PrintHeaderVisitor() override;

    /// @name Refinement methods.
    //@{

    auto visitDesignUnit(hif::DesignUnit &o) -> int override;
    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitSystem(hif::System &o) -> int override;

    //@}

private:
    PrintHeaderVisitor(const PrintHeaderVisitor &)                     = delete;
    auto operator=(const PrintHeaderVisitor &) -> PrintHeaderVisitor & = delete;

    /// @name Functions that print a new header file for the component.
    //@{

    auto _printHeader(hif::DesignUnit &o) -> int;
    auto _printHeader(hif::LibraryDef &o) -> int;
    auto _printHeader(hif::System &o) -> int;
    auto _printHeader(hif::Object &o, string fileName) -> int;

    //@}

    static auto _createDirectory(const string &dirName) -> int;
    auto _createDirectories(
        const string &outdirName,
        const std::string &libDefName,
        PrintSystemCVisitor::ConstTemplateMap &ctmList) -> int;
    auto _getHeaderExtensionByLanguage(Object *obj) -> std::string;
    auto
    _getFileName(const std::string &outDir, const std::string &libDefName, const std::string &fileName, Object *obj)
        -> std::string;

    /// @brief The output stream to write on.
    ///
    hif::backends::IndentedStream *_outstream{nullptr};

    /// @brief The main output directory, used to create the directory hierarchy
    /// related to this HIF tree.
    ///
    string _outdirName;

    /// @name The fields related to nested components.
    //@{

    /// @brief When printing a LibraryDef, all its contents will be included in
    /// a directory with its same name. This fields is set in visit of LibraryDef,
    /// and it is used to recognize nested components (if any).
    hif::LibraryDef *_currentLibraryDef{nullptr};

    /// @brief This fields is set in visit of DesignUnit, and it is used to
    /// recognize nested components (if any).
    std::string DUName;

    /// @brief Set to true, indicates the presence of global declarations.
    bool _includeSystemGlobals{false};

    //@}

    /// @name Command-line flags of hif2sc
    //@{

    bool _useResolved;
    bool _useHDTLib;
    const uint64_t _maxLines;
    const std::string _sourcesExtension;
    const std::string _headersExtension;

    //@}

    /// @brief The map of constant template.
    PrintSystemCVisitor::ConstTemplateMap &_ctmList;

    /// @brief The semantics
    hif::semantics::SystemCSemantics *_sem;
};

PrintHeaderVisitor::PrintHeaderVisitor(hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList)
    : _outdirName(cLine.getOutputDirectory())
    , DUName()
    , _useResolved(cLine.useResolved())
    , _useHDTLib(cLine.useHDTLib())
    , _maxLines(cLine.getMaxLines())
    , _sourcesExtension(cLine.getSourcesExtension())
    , _headersExtension(cLine.getHeadersExtension())
    , _ctmList(ctmList)
    , _sem(hif::semantics::SystemCSemantics::getInstance())
{
    hif::application_utils::initializeLogHeader("HIF2SC", "PrintHeaderVisitor");
}

PrintHeaderVisitor::~PrintHeaderVisitor()
{
    hif::application_utils::restoreLogHeader();
    delete _outstream;
}

auto PrintHeaderVisitor::visitDesignUnit(DesignUnit &o) -> int
{
    auto restore = o.getName();
    DUName       = o.getName();

    _printHeader(o);

    DUName = restore;
    return 0;
}

auto PrintHeaderVisitor::visitLibraryDef(LibraryDef &o) -> int
{
    // Skip standard libraries (e.g., tlm)
    if (o.isStandard()) {
        return 0;
    }

    // Note: even if LibraryDef contains only DesignUnits or other LibraryDef,
    // the file is still generated to preserve the hierarchical structure.

    LibraryDef *restore = _currentLibraryDef;
    _currentLibraryDef  = &o;

    _printHeader(o);

    // Recursive call.
    GuideVisitor::visitLibraryDef(o);

    _currentLibraryDef = restore;
    return 0;
}

auto PrintHeaderVisitor::visitSystem(System &o) -> int
{
    if (_printHeader(o) == 0) {
        _includeSystemGlobals = true;
    }

    // Recursive call.
    GuideVisitor::visitSystem(o);
    return 0;
}

auto PrintHeaderVisitor::_printHeader(DesignUnit &o) -> int
{
    // Normally there's only one View for each DesignUnit.
    messageAssert(!o.views.empty() && o.views.size() == 1, "Unsupported more than one view", &o, nullptr);
    View *duView = o.views.front();

    // skip standard ones
    if (duView->isStandard()) {
        return 0;
    }

    // Feedback message
    messageInfo(string("Generating ") + getLanguage(duView->getLanguageID()) + " code for unit " + o.getName() + ".");

    // Note: directories have already been created.
    std::string ldName;
    if (_currentLibraryDef != nullptr) {
        ldName = _currentLibraryDef->getName();
    }
    string fileName = _getFileName(_outdirName, ldName, DUName, duView);
    _printHeader(o, fileName);

    return 0;
}

auto PrintHeaderVisitor::_printHeader(LibraryDef &o) -> int
{
    std::string ldName;
    if (_currentLibraryDef != nullptr) {
        ldName = _currentLibraryDef->getName();
    }

    // Create the directories associated to this library.
    if (_createDirectories(_outdirName, ldName, _ctmList) == 0) {
        messageError("Error creating output directories.", nullptr, nullptr);
    }

    // Feedback message
    messageInfo(string("Generating ") + getLanguage(o.getLanguageID()) + " code for library " + o.getName() + ".");

    string fileName = _getFileName(_outdirName, ldName, ldName, &o);
    _printHeader(o, fileName);

    return 0;
}

auto PrintHeaderVisitor::_printHeader(System &o) -> int
{
    if (_createDirectories(_outdirName, "", _ctmList) == 0) {
        messageDebugAssert(false, "Unexpected case", nullptr, nullptr);
        messageError("Error creating output directories.", nullptr, nullptr);
    }

    if (o.declarations.empty() && o.actions.empty()) {
        return 1;
    }

    // Feedback message
    messageInfo(string("Generating ") + getLanguage(o.getLanguageID()) + " code for global declarations.");

    string fileName = _getFileName(_outdirName, nullptr, NameTable::getInstance()->hifGlobals(), &o);
    _printHeader(o, fileName);

    return 0;
}

auto PrintHeaderVisitor::_printHeader(Object &o, string fileName) -> int
{
    ofstream actualStream;
    if (hif::backends::openFileStream(fileName, &actualStream) == 0) {
        messageError(string("Error writing file ") + fileName, nullptr, nullptr);
    }
    actualStream.close();

    if (_outstream != nullptr) {
        delete _outstream;
        _outstream = nullptr;
    }

    std::string f;
    std::string ext;
    hif::backends::splitFileName(fileName, f, ext);
    _outstream = new hif::backends::IndentedStream(f, ext);
    _outstream->setComment("// ", "// ", "");

    PrintSystemCVisitorOptions opt;
    opt.printImplementation = false;
    opt.useResolved         = _useResolved;
    opt.useHDTLib           = _useHDTLib;
    opt.maxLines            = _maxLines;
    opt.sourcesExtension    = _sourcesExtension;
    opt.headersExtension    = _headersExtension;

    PrintSystemCVisitor vis(_outstream, opt, _ctmList, f, ext);
    vis.setCurrentLibraryDef(_currentLibraryDef);
    o.acceptVisitor(vis);

    *(_outstream) << '\n'; // Flush the stream

    return 0;
}

auto PrintHeaderVisitor::_createDirectory(const string &dirName) -> int
{
    hif::application_utils::FileStructure dir(dirName);
    // Empty directory if it already exists.
    if (dir.exists()) {
        vector<string> fileList = dir.list();
        for (auto &it : fileList) {
            hif::application_utils::FileStructure fileIn(it);
            fileIn.rmfile_weak();
        }
    }
    // Create new directory (if it does not already exist)
    else if (!dir.make_dir()) {
        messageError("Directory generation not successful.", nullptr, nullptr);
    }

    return 1;
}

auto PrintHeaderVisitor::_createDirectories(
    const std::string &outdirName,
    const std::string &libDefName,
    PrintSystemCVisitor::ConstTemplateMap &ctmList) -> int
{
    PrintSystemCVisitor vis(ctmList);
    // To portability under Windows, directory paths must not end with a trailing '/' character.
    // Thus, we manage the concatenation directly in ld var.
    string ld = (libDefName.empty()) ? "" : "/" + std::string(libDefName);

    string dirName = outdirName + "/inc" + ld;
    if (_createDirectory(dirName) == 0) {
        return 0;
    }

    dirName = outdirName + "/src" + ld;
    if (_createDirectory(dirName) == 0) {
        return 0;
    }

    return 1;
}

auto PrintHeaderVisitor::_getHeaderExtensionByLanguage(Object *obj) -> std::string
{
    const hif::LanguageID lang = objectGetLanguage(obj);
    auto *ld                   = dynamic_cast<LibraryDef *>(obj);

    if (lang == hif::c || (ld != nullptr && ld->getLanguageID() == hif::cpp && ld->hasCLinkage())) {
        return ".h";
    }

    // rtl, tlm, psl, cpp
    return "." + _headersExtension;
}

auto PrintHeaderVisitor::_getFileName(
    const std::string &outDir,
    const std::string &libDefName,
    const std::string &fileName,
    Object *obj) -> std::string
{
    string ret;
    string ld = (libDefName.empty()) ? "" : std::string(libDefName) + "/";
    string fn = (fileName.empty()) ? "" : fileName;

    ret = outDir + "/inc/" + ld + fn + _getHeaderExtensionByLanguage(obj);
    return ret;
}

} // namespace

void printHeaders(hif::System *sys, hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList)
{
    PrintHeaderVisitor writeHeader(cLine, ctmList);
    sys->acceptVisitor(writeHeader);
}
