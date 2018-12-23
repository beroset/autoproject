#include "AutoProject.h"
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <regex>

using namespace std::literals;

const std::string AutoProject::mdextension{".md"};  
const std::string cmakeVersion{"VERSION 3.1"};

void AutoProject::open(fs::path mdFilename)  {
    AutoProject ap(mdFilename);
    std::swap(ap, *this);
}

AutoProject::AutoProject(fs::path mdFilename) : 
    mdfile{mdFilename},
    projname{mdfile.stem().string()},
    srcdir{projname + "/src"},
    in(mdfile)
{
    if (mdfile.extension() != mdextension) {
        throw FileExtensionException("Input file must have " + mdextension + " extension");
    }
    if (!in) {
        throw std::runtime_error("Cannot open input file "s + mdfile.string());
    }
}

/// returns true if passed file extension is an identified source code extension.
bool isSourceExtension(const std::string &ext) {
    static const std::unordered_set<std::string> source_extensions{".cpp", ".c", ".h", ".hpp"};
    return source_extensions.find(ext) != source_extensions.end();
}

void AutoProject::copyFile() const {
    // copy md file to projname/src
    fs::copy_file(mdfile, srcdir + "/" + projname + mdextension);
}

bool AutoProject::createProject() {
    std::string prevline;
    bool infile{false};
    bool firstFile{true};
    std::ofstream srcfile;
    fs::path srcfilename;
    for (std::string line; getline(in, line); ) {
        replaceLeadingTabs(line);
        // scan through looking for lines indented with indentLevel spaces
        if (infile) {
            // stop writing if non-indented line or EOF
            if (!isIndentedOrEmpty(line)) {
                std::swap(prevline, line);
                srcfile.close();
                infile = false;
            } else {
                checkRules(line);
                emit(srcfile, line);
            }
        } else {
            if (isNonEmptyIndented(line)) {
                // if previous line was filename, open that file and start writing
                if (isSourceFilename(prevline)) {
                    if (firstFile) {
                        makeTree();
                        firstFile = false;
                    }
                    srcfilename = fs::path(srcdir) / prevline;
                    srcfile.open(srcfilename);
                    if (srcfile) {
                        checkRules(line);
                        emit(srcfile, line);
                        srcnames.emplace(srcfilename.filename());
                        infile = true;
                    }
                } else if (firstFile && !line.empty()) {  // un-named source file
                    makeTree();
                    firstFile = false;
                    srcfilename = fs::path(srcdir) / "main.cpp";
                    srcfile.open(srcfilename);
                    if (srcfile) {
                        checkRules(line);
                        emit(srcfile, line);
                        srcnames.emplace(srcfilename.filename());
                        infile = true;
                    }
                }
            } else {
                if (!isEmptyOrUnderline(line)) 
                    std::swap(prevline, line);
            }
        }
    }        
    in.close();
    if (!srcnames.empty()) {
        writeSrcLevel();
        writeTopLevel();
        copyFile();
    }
    return !srcnames.empty();
}

void AutoProject::makeTree() {
    if (fs::exists(projname)) {
        throw std::runtime_error(projname + " already exists: will not overwrite.");
    }
    /*
     * I would have used create_directories here, but there appears to be a bug
     * in it that didn't exist in the experimental/filesystem version.
     * Sill tracking that down, but this will do for now.
     */
    if (!fs::create_directories(srcdir)) {
        throw std::runtime_error("Cannot create directory "s + srcdir);
    }
    fs::create_directories(projname + "/build");
}

std::string& AutoProject::trim(std::string& str, const std::string& pattern) {
    // TODO: when we get C++20, use std::string::starts_with()
    if (str.find(pattern) == 0) {
        str.erase(0, pattern.size());
    }
    return str;
}

std::string& AutoProject::rtrim(std::string& str, const std::string& pattern) {
    // TODO: when we get C++20, use std::string::ends_with()
    auto loc{str.rfind(pattern)};
    if (loc != std::string::npos && loc == str.size() - pattern.size()) {
        str.erase(str.size() - pattern.size(), pattern.size());
    }
    return str;
}

std::string& AutoProject::trim(std::string& str, char ch) {
    auto it{str.begin()};
    for ( ; (*it == ch || isspace(*it)) && it != str.end(); ++it) 
    { }
    if (it != str.end()) {
        str.erase(str.begin(), it);
    }
    return str;
}

std::string& AutoProject::rtrim(std::string& str, char ch) {
    std::reverse(str.begin(), str.end());
    trim(str, ch);
    std::reverse(str.begin(), str.end());
    return str;
}

bool AutoProject::isSourceFilename(std::string& line) const {
    trimExtras(line);
    return isSourceExtension(fs::path(line).extension().string());
}

std::string AutoProject::trimExtras(std::string& line) const
{
    if (line.empty()) {
        return line;
    }
    // remove header markup
    trim(line, '#');
    rtrim(line, '#');
    // remove bold or italic
    trim(line, '*');
    rtrim(line, '*');
    // remove html bold
    trim(line, "<b>");
    rtrim(line, "</b>");
    // remove quotes
    trim(line, '"');
    rtrim(line, '"');
    // remove trailing - or :
    rtrim(line, '-');
    rtrim(line, ':');
    return line;
}

void AutoProject::writeSrcLevel() const {
    // write CMakeLists.txt with filenames to projname/src
    std::ofstream srccmake(srcdir + "/CMakeLists.txt");
    srccmake <<
            "cmake_minimum_required(" << cmakeVersion << ")\n"
            "set(EXECUTABLE_NAME \"" << projname << "\")\n"
            "add_executable(${EXECUTABLE_NAME}";
    for (const auto& fn : srcnames) {
        srccmake << ' ' << fn;
    }
    srccmake << ")\n";
    for (const auto &rule : extraRules) {
        srccmake << rule << '\n';
    }
    srccmake.close();
}

void AutoProject::writeTopLevel() const {
    // TODO: use replaceable boilerplate in a config file
    // write CMakeLists.txt top level to projname
    std::ofstream topcmake(projname + "/CMakeLists.txt");
    topcmake << 
            "cmake_minimum_required(" << cmakeVersion << ")\n"
            "project(" << projname << ")\n"
            "set(CMAKE_CXX_STANDARD 14)\n"
            "add_subdirectory(src)\n";
}

void AutoProject::checkRules(const std::string &line) {
    // TODO: provide mechanism to load rules from file(s)
    static const struct Rule { 
        const std::regex re;
        const std::string cmake;
        Rule(std::string reg, std::string result) : re{reg}, cmake{result} {}
    } rules[]{
        { R"(\s*#include\s*<(experimental/)?filesystem>)", 
            "if (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"GNU\")\n"
            "  target_link_libraries(${EXECUTABLE_NAME} stdc++fs)\n"
            "endif()\n" 
        },
        { R"(\s*#include\s*<thread>)", "find_package(Threads REQUIRED)\n"
                "target_link_libraries(${EXECUTABLE_NAME} ${CMAKE_THREAD_LIBS_INIT})"},
        { R"(\s*#include\s*<future>)", "find_package(Threads REQUIRED)\n"
                "target_link_libraries(${EXECUTABLE_NAME} ${CMAKE_THREAD_LIBS_INIT})"},
        { R"(\s*#include\s*<SFML/Graphics.hpp>)", 
                    "find_package(SFML REQUIRED COMPONENTS System Window Graphics)\n"
                    "if(SFML_FOUND)\n"
                    "  include_directories(${SFML_INCLUDE_DIR})\n"
                    "  target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})\n"
                    "endif()" },
        { R"(\s*#include\s*<GL/glew.h>)", 
                    "find_package(GLEW REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${GLEW_LIBRARIES})" },
        { R"(\s*#include\s*<GL/glut.h>)", 
                    R"(find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES}))" },
        { R"(\s*#include\s*<OpenGL/gl.h>)", 
                    "find_package(OpenGL REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})" },
        { R"(\s*#include\s*<SDL2/SDL.h>)", 
                    "find_package(SDL2 REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${SDL2_LIBRARIES})" },
        // the SDL2_tff.cmake package doesn't yet ship with CMake
        { R"(\s*#include\s*<SDL2/SDL_ttf.h>)", 
                    "find_package(SDL2_ttf REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${SDL2_TTF_LIBRARIES})" },
        { R"(\s*#include\s*<GLFW/glfw3.h>)", 
                    "find_package(glfw3 REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} glfw)" },
        { R"(\s*#include\s*<boost/regex.hpp>)", 
                    "find_package(Boost REQUIRED COMPONENTS regex)\ntarget_link_libraries(${EXECUTABLE_NAME} ${Boost_LIBRARIES})" },
        { R"(\s*#include\s*<png.h>)", 
                    "find_package(PNG REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${PNG_LIBRARIES})" },
        { R"(\s*#include\s*<ncurses.h>)", 
                    "find_package(Curses REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${CURSES_LIBRARIES})" },
        { R"(\s*#include\s*<SDL2.SDL.h>)",
            R"(include(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${EXECUTABLE_NAME} ${SDL2_LIBRARIES})
)" },
        // experimental support for Qt5; not sure if Widgets is correct
        { R"(\s*#include\s*<QString>)", 
                    R"(find_package(Qt5Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
target_link_libraries(${EXECUTABLE_NAME} "Qt5::Widgets")
)" },
        { R"(\s*#include\s*<openssl/ssl.h>)", 
                    "find_package(OpenSSL REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${OPENSSL_LIBRARIES})" },
    };
    std::smatch sm;
    for (const auto &rule : rules) {
        if (std::regex_search(line, sm, rule.re)) {
            extraRules.emplace(rule.cmake);
        }
    }
}

bool AutoProject::isNonEmptyIndented(const std::string& line) const {
    size_t indent{line.find_first_not_of(' ')};
    return indent >= indentLevel && indent != std::string::npos;
}

bool AutoProject::isIndentedOrEmpty(const std::string& line) const {
    size_t indent{line.find_first_not_of(' ')};
    return indent >= indentLevel;
}

bool AutoProject::isEmptyOrUnderline(const std::string& line) const {
    size_t indent{line.find_first_not_of('-')};
    return line.empty() || indent == std::string::npos;
}

std::string &AutoProject::replaceLeadingTabs(std::string &line) const {
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

void AutoProject::emit(std::ostream& out, const std::string &line) const {
    if (line.size() < indentLevel) {
        out << line << '\n';
    } else {
        out << (line[0] == ' ' ? line.substr(indentLevel) : line.substr(1)) << '\n';
    }
}

std::ostream& operator<<(std::ostream& out, const AutoProject &ap) {
    out << "Successfully extracted the following source files:\n";
    std::copy(ap.srcnames.begin(), ap.srcnames.end(), std::ostream_iterator<fs::path>(out, "\n"));
    return out;
}
