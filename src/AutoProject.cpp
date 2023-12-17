#include <config.h>
#include "AutoProject.h"
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <vector>
#include <string_view>
#include "trim.h"

using namespace std::literals;

// local definitions
struct Rule {
    const std::regex re;
    const std::string cmake;
    const std::string libraries;
    static const std::regex newline;
    Rule(std::string reg, std::string result, std::string libraries) : re{reg}, 
        cmake{std::regex_replace(result, newline, "\n")},
        libraries{libraries} {
    }
};

// helper functions
static bool isNonEmptyIndented(const std::string& line);
static bool isIndentedOrEmpty(const std::string& line);
static bool isEmptyOrUnderline(const std::string& line);
static bool isDelimited(const std::string& line);
static bool isSourceExtension(const std::string_view ext);
static bool isSourceFilename(std::string& line);
static std::string &replaceLeadingTabs(std::string& line);
static void emit(std::ostream& out, const std::string& line);
static std::vector<Rule> loadrules(const fs::path& rulesfile);

// local constants
static const std::string mdextension{".md"};
static constexpr unsigned indentLevel{4};
static constexpr unsigned delimLength{3};
const std::regex Rule::newline{R"(\\n)"};

// local variables
static std::vector<Rule> rules;

// AutoProject interface functions
void AutoProject::open(fs::path mdFilename, std::map<std::string, LangConfig> lang) {
    AutoProject ap(mdFilename, lang);
    std::swap(ap, *this);
}

AutoProject::AutoProject(fs::path mdFilename, std::map<std::string, LangConfig> lang) :
    mdfile{mdFilename},
    outdir{mdFilename.replace_extension("")},
    projname{mdfile.stem().string()},
    srcdir{outdir.string() + "/src"},
    in{mdfile},
    lang{lang}
{
    if (mdfile.extension() != mdextension) {
        throw FileExtensionException("Input file must have " + mdextension + " extension");
    }
    if (!in) {
        throw std::runtime_error("Cannot open input file "s + mdfile.string());
    }
}

/*
 * As of January 2019, according to this post:
 * https://meta.stackexchange.com/questions/125148/implement-style-fenced-markdown-code-blocks
 * an alternative of using either ```c++ or ~~~lang-c++ with a matching end
 * tag and unindented code is now supported in addition to the original
 * indented flavor.  As a result, this code is modified to also accept
 * that syntax as of April 2019.
 */
bool AutoProject::createProject(bool overwrite) {
    std::string prevline;
    bool inIndentedFile{false};
    bool inDelimitedFile{false};
    bool firstFile{true};
    std::ofstream srcfile;
    fs::path srcfilename;
    // TODO: this might be much cleaner with a state machine
    for (std::string line; getline(in, line); ) {
        replaceLeadingTabs(line);
        // scan through looking for lines indented with indentLevel spaces
        if (inIndentedFile) {
            // stop writing if non-indented line or EOF
            if (!isIndentedOrEmpty(line)) {
                std::swap(prevline, line);
                srcfile.close();
                inIndentedFile = false;
            } else {
                checkRules(line);
                emit(srcfile, line);
            }
        } else if (inDelimitedFile) {
            // stop writing if delimited line
            if (isDelimited(line)) {
                std::swap(prevline, line);
                srcfile.close();
                inDelimitedFile = false;
            } else {
                checkRules(line);
                srcfile << line << '\n';
            }
        } else {
            if (isDelimited(line)) {
                // if previous line was filename, open that file and start writing
                if (isSourceFilename(prevline)) {
                    srcfilename = fs::path(srcdir) / prevline;
                } else {
                    if (thislang == "c") {
                        srcfilename = fs::path(srcdir) / "main.c";
                    } else if (thislang == "c++") {
                        srcfilename = fs::path(srcdir) / "main.cpp";
                    } else if (thislang == "asm") {
                        srcfilename = fs::path(srcdir) / "main.asm";
                    } 
                }
                if (firstFile) {
                    makeTree(overwrite);
                    firstFile = false;
                }
                srcfile.open(srcfilename);
                if (srcfile) {
                    srcnames.emplace(srcfilename.filename());
                    inDelimitedFile = true;
                }
            } else if (isNonEmptyIndented(line)) {
                // if previous line was filename, open that file and start writing
                if (isSourceFilename(prevline)) {
                    if (firstFile) {
                        makeTree(overwrite);
                        firstFile = false;
                    }
                    srcfilename = fs::path(srcdir) / prevline;
                    srcfile.open(srcfilename);
                    if (srcfile) {
                        checkRules(line);
                        emit(srcfile, line);
                        srcnames.emplace(srcfilename.filename());
                        inIndentedFile = true;
                    }
                } else if (firstFile && !line.empty()) {  // un-named source file
                    makeTree(overwrite);
                    firstFile = false;
                    if (thislang == "c") {
                        srcfilename = fs::path(srcdir) / "main.c";
                    } else if (thislang == "c++") {
                        srcfilename = fs::path(srcdir) / "main.cpp";
                    } else if (thislang == "asm") {
                        srcfilename = fs::path(srcdir) / "main.asm";
                    }
                    srcfile.open(srcfilename);
                    if (srcfile) {
                        checkRules(line);
                        emit(srcfile, line);
                        srcnames.emplace(srcfilename.filename());
                        inIndentedFile = true;
                    }
                }
            } else {
                if (!isEmptyOrUnderline(line)) {
                    checkLanguageTags(line);
                    std::swap(prevline, line);
                }
            }
        }
    }
    in.close();
    if (!srcnames.empty()) {
        writeSrcLevel();
        copyCloneDir(overwrite);
        writeTopLevel();
        // copy md file to projname/src
        auto options = overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none;
        fs::copy_file(mdfile, srcdir + "/" + projname + mdextension, options);
    }
    return !srcnames.empty();
}

void AutoProject::makeTree(bool overwrite) {
    fs::path builddir{outdir.string() + "/build"};
    if (overwrite) {
        fs::create_directories(srcdir);
        fs::create_directories(builddir);
    } else {
        if (fs::exists(outdir)) {
            throw std::runtime_error(outdir.string() + " already exists: will not overwrite.");
        }
        if (!fs::create_directories(srcdir)) {
            throw std::runtime_error("Cannot create directory "s + srcdir);
        }
        fs::create_directories(builddir);
    }
}

void AutoProject::writeSrcLevel() const {
    static const std::regex projname_regex{"[{]projname[}]"};
    static const std::regex extras_regex{"[{]extras[}]"};
    static const std::regex srcnames_regex{"[{]srcnames[}]"};
    static const std::regex libraries_regex{"[{]libraries[}]"};
    std::ifstream in{srclevelfilename};
    if (!in) {
        std::cerr << "Error: cannot open source level filename \"" << srclevelfilename << "\"\n";
        exit(1);
    }
    std::stringstream extras;
    for (const auto &rule : extraRules) {
        extras << rule << '\n';
    }
    std::stringstream sources;
    for (const auto& fn : srcnames) {
        sources << ' ' << fn;
    }
    std::stringstream libs;
    for (const auto &lib : libraries) {
        libs << ' ' << lib;
    }
    // write CMakeLists.txt with filenames to projname/src
    std::ofstream srccmake(srcdir + "/CMakeLists.txt");
    std::string line;
    while (std::getline(in, line)) {
        line = std::regex_replace(line, projname_regex, projname);
        line = std::regex_replace(line, srcnames_regex, sources.str());
        line = std::regex_replace(line, extras_regex, extras.str());
        line = std::regex_replace(line, libraries_regex, libs.str());
        srccmake << line << '\n';
    }
}

void AutoProject::copyCloneDir(bool overwrite) const {
    if (!clonedir.empty()) {
        auto options = overwrite ? fs::copy_options::overwrite_existing|fs::copy_options::recursive : fs::copy_options::recursive;
        fs::copy(configdir / clonedir, outdir.string() / clonedir, options);
    }
}

void AutoProject::writeTopLevel() const {
    static const std::regex projname_regex{"[{]projname[}]"};
    std::ifstream in{toplevelfilename};
    if (!in) {
        std::cerr << "Error: cannot open top level filename \"" << toplevelfilename << "\"\n";
        exit(1);
    }
    std::ofstream topcmake{outdir.string() + "/CMakeLists.txt"};
    std::string line;
    while (std::getline(in, line)) {
        topcmake << std::regex_replace(line, projname_regex, projname) << '\n';
    }
}

void AutoProject::checkRules(const std::string &line) {
    std::smatch sm;
    for (const auto &rule : rules) {
        if (std::regex_search(line, sm, rule.re)) {
            extraRules.emplace(rule.cmake);
            libraries.emplace(rule.libraries);
        }
    }
}

void AutoProject::checkLanguageTags(const std::string& line) {
    if (!thislang.empty()) 
        return;
    static const std::regex tagcpp{"### tags: \\[.*'c\\+\\+'.*\\]"}; 
    static const std::regex tagc{"### tags: \\[.*'c'.*\\]"}; 
    static const std::regex tagasm{"### tags: \\[.*'assembly'.*\\]"}; 
    std::smatch pieces;
    if (std::regex_match(line, pieces, tagcpp)) {
        thislang = "c++";
    } else if (std::regex_match(line, pieces, tagc)) {
        thislang = "c";
    } else if (std::regex_match(line, pieces, tagasm)) {
        thislang = "asm";
    } else {
        return;
    }
    rules = loadrules(lang[thislang].rulesfilename);
    configdir = lang[thislang].configdir;
    toplevelfilename = lang[thislang].toplevelcmakefilename;
    srclevelfilename = lang[thislang].srclevelcmakefilename;
    clonedir = lang[thislang].clonedir;
}

std::ostream& operator<<(std::ostream& out, const AutoProject &ap) {
    out << "Successfully extracted the following source files to " << ap.outdir << ":\n";
    std::copy(ap.srcnames.begin(), ap.srcnames.end(), std::ostream_iterator<fs::path>(out, "\n"));
    return out;
}

// helper functions

/// returns true if passed file extension is an identified source code extension.
bool isSourceExtension(const std::string_view ext) {
    static const std::unordered_set<std::string_view> source_extensions{".cpp", ".c", ".h", ".hpp", ".asm"};
    return source_extensions.find(ext) != source_extensions.end();
}

bool isSourceFilename(std::string &line) {
    trimExtras(line);
    return isSourceExtension(fs::path(line).extension().string());
}

std::string trimExtras(std::string& line) {
    if (line.empty()) {
        return line;
    }
    // remove header markup
    doubletrim(line, '#');
    // remove bold or italic
    doubletrim(line, '*');
    // remove html bold
    doubletrim(line, "<b>", "</b>");
    // remove quotes
    doubletrim(line, '"');
    // remove trailing - or :
    rtrim(line, '-');
    rtrim(line, ':');
    return line;
}

bool isNonEmptyIndented(const std::string& line) {
    size_t indent{line.find_first_not_of(' ')};
    return indent >= indentLevel && indent != std::string::npos;
}

bool isIndentedOrEmpty(const std::string& line) {
    size_t indent{line.find_first_not_of(' ')};
    return indent >= indentLevel;
}

bool isEmptyOrUnderline(const std::string& line) {
    size_t indent{line.find_first_not_of('-')};
    return line.empty() || indent == std::string::npos;
}

bool isDelimited(const std::string& line) {
    if (line.empty() || (line[0] != '`' && line[0] != '~')) {
        return false;
    }
    size_t backtickDelim{line.find_first_not_of('`')};
    size_t tildeDelim{line.find_first_not_of('~')};
    return backtickDelim >= delimLength || tildeDelim >= delimLength;
}

std::string &replaceLeadingTabs(std::string &line) {
    std::size_t tabcount{0};
    for (auto ch: line) {
        if (ch != '\t')
            break;
        ++tabcount;
    }
    if (tabcount) {
        line.replace(0, tabcount, indentLevel*tabcount, ' ');
    }
    return line;
}

void emit(std::ostream& out, const std::string &line) {
    if (line.size() < indentLevel) {
        out << line << '\n';
    } else {
        out << (line[0] == ' ' ? line.substr(indentLevel) : line.substr(1)) << '\n';
    }
}

std::vector<Rule> loadrules(const fs::path &rulesfile) {
    std::vector<Rule> rules;
    std::ifstream in(rulesfile);
    if (!in) {
        std::cerr << "Unable to open rules file: " << rulesfile << "\n";
        return rules;
    }
    std::string line;
    unsigned linenum{0};
    static const std::regex rulefields{"([^@]+)@([^@]*)@(.*)"}; 
    while (std::getline(in, line)) {
        ++linenum;
        std::smatch pieces;
        if (std::regex_match(line, pieces, rulefields) && pieces.size() == 4) {
            try {
                rules.emplace_back(pieces[1], pieces[2], pieces[3]);
            } 
            catch (const std::regex_error& e) {
                static constexpr std::string_view labels[4]{"line", "regex", "cmake lines", "libraries"};
                std::cerr << "Error: " << e.what() << " in line " << linenum << " of rules file " << rulesfile << "\n";
                for (unsigned i{0}; i < pieces.size(); ++i ) {
                    std::cout << labels[i] << " = \"" << pieces[i] << "\"\n";
                }
            }
        }
    }
    std::cout << "Loaded " << rules.size() << " rules\n";
    return rules;
}
