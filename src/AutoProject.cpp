#include "AutoProject.h"
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <regex>

const std::string AutoProject::mdextension{".md"};  
const std::string cmakeVersion{"VERSION 3.0"};

void AutoProject::open(fs::path mdFilename)  {
    AutoProject ap(mdFilename);
    std::swap(ap, *this);
}

AutoProject::AutoProject(fs::path mdFilename) : 
    mdfile{mdFilename},
    projname{mdfile.stem()},
    srcdir{projname + "/src/"},
    in(mdfile)
{
    if (mdfile.extension() != mdextension) {
        throw FileExtensionException("Input file must have " + mdextension + " extension");
    }
    if (!in) {
        throw std::runtime_error(std::string("Cannot open input file ") + mdfile.c_str());
    }
}

/// returns true if passed file extension is an identified source code extension.
bool isSourceExtension(const std::string &ext) {
    static const std::unordered_set<std::string> source_extensions{".cpp", ".c", ".h", ".hpp"};
    return source_extensions.find(ext) != source_extensions.end();
}

void AutoProject::copyFile() const {
    // copy md file to projname/src
    fs::copy_file(mdfile, srcdir + projname + mdextension);
}

bool AutoProject::createProject() {
    std::string prevline;
    bool infile{false};
    bool firstFile{true};
    std::ofstream srcfile;
    fs::path srcfilename;
    for (std::string line; getline(in, line); ) {
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
                    srcfilename = fs::path(srcdir + prevline);
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
                    srcfilename = fs::path(srcdir + "main.cpp");
                    srcfile.open(srcfilename);
                    if (srcfile) {
                        checkRules(line);
                        emit(srcfile, line);
                        srcnames.emplace(srcfilename.filename());
                        infile = true;
                    }
                }
            } else {
                if (!line.empty())
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
    if (fs::exists(srcdir)) {
        throw std::runtime_error(projname + " already exists: will not overwrite.");
    }
    if (!fs::create_directories(srcdir)) {
        throw std::runtime_error(std::string("Cannot create directory ") + srcdir);
    }
    fs::create_directories(projname + "/build/");
}

std::string& AutoProject::trim(std::string& str, char ch) {
    auto it = str.begin();
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
    return isSourceExtension(fs::path(line).extension());
}

std::string AutoProject::trimExtras(std::string& line) const
{
    if (line.empty()) {
        return line;
    }
    // remove header markup
    trim(line, '#');
    // remove bold or italic
    trim(line, '*');
    rtrim(line, '*');
    // remove trailing - or :
    rtrim(line, '-');
    rtrim(line, ':');
    return line;
}

void AutoProject::writeSrcLevel() const {
    // TODO: add rules to augment CMake file for things like SFML
    // write CMakeLists.txt with filenames to projname/src
    std::ofstream srccmake(srcdir + "CMakeLists.txt");
    srccmake <<
            "cmake_minimum_required(" << cmakeVersion << ")\n"
            "set(EXECUTABLE_NAME \"" << projname << "\")\n"
            "add_executable(${EXECUTABLE_NAME}";
    for (const auto& fn : srcnames) {
        srccmake << ' ' << fn;
    }
    srccmake << ")\n";
    for (const auto rule : extraRules) {
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
            "add_subdirectory(src)\n";
}

void AutoProject::checkRules(const std::string &line) {
    static const struct Rule { 
        const std::regex re;
        const std::string cmake;
        Rule(std::string reg, std::string result) : re{reg}, cmake{result} {}
    } rules[]{
        { R"(\s*#include\s*<experimental/filesystem>)", R"(target_link_libraries(${EXECUTABLE_NAME} stdc++fs))" },
        { R"(\s*#include\s*<thread>)", "find_package(Threads REQUIRED)\n"
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
        { R"(\s*#include\s*<GLFW/glfw3.h>)", 
                    "find_package(glfw3 REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} glfw)" },
        { R"(\s*#include\s*<png.h>)", 
                    "find_package(PNG REQUIRED)\ntarget_link_libraries(${EXECUTABLE_NAME} ${PNG_LIBRARIES})" },
    };
    std::smatch sm;
    for (const auto &rule : rules) {
        if (std::regex_search(line, sm, rule.re)) {
            extraRules.emplace(rule.cmake);
        }
    }
}

bool AutoProject::isNonEmptyIndented(const std::string& line) const {
    size_t indent = line.find_first_not_of(' ');
    return indent >= indentLevel && indent != std::string::npos;
}

bool AutoProject::isIndentedOrEmpty(const std::string& line) const {
    size_t indent = line.find_first_not_of(' ');
    return indent >= indentLevel;
}

void AutoProject::emit(std::ostream& out, const std::string &line) const {
    if (line.size() < indentLevel) {
        out << line << '\n';
    } else {
        out << (line[0] == ' ' ? line.substr(indentLevel) : line.substr(1)) << '\n';
    }
}
