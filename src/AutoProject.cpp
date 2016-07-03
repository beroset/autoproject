#include "AutoProject.h"
#include <unordered_set>
#include <algorithm>
#include <iostream>

const std::string AutoProject::mdextension{".md"};  

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
    if (fs::exists(srcdir)) {
        throw std::runtime_error(projname + " already exists: will not overwrite.");
    }
    if (!fs::create_directories(srcdir)) {
        throw std::runtime_error(std::string("Cannot create directory ") + srcdir);
    }
    fs::create_directories(projname + "/build/");
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
    bool infile = false;
    std::ofstream srcfile;
    fs::path srcfilename;
    for (std::string line; getline(in, line); ) {
        // scan through looking for lines indented with indentLevel spaces
        if (infile) {
            // stop writing if non-indented line or EOF
            if (!line.empty() && !isspace(line[0])) {
                prevline = line;
                srcfile.close();
                infile = false;
            } else {
                emit(srcfile, line);
            }
        } else {
            if (isIndented(line)) {
                // if previous line was filename, open that file and start writing
                if (isSourceFilename(prevline)) {
                    srcfilename = fs::path(srcdir + prevline);
                    srcfile.open(srcfilename);
                    if (srcfile) {
                        emit(srcfile, line);
                        srcnames.push_back(srcfilename.filename());
                        infile = true;
                    }
                }
            } else {
                prevline = line;
            }
        }
    }        
    in.close();
    writeSrcLevel();
    writeTopLevel();
    copyFile();
    return !srcnames.empty();
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
    // write CMakeLists.txt with filenames to projname/src
    std::ofstream srccmake(srcdir + "CMakeLists.txt");
    srccmake <<
            "cmake_minimum_required(VERSION 2.8)\n"
            "set(EXECUTABLE_NAME \"" << projname << "\")\n"
            "add_executable(${EXECUTABLE_NAME}";
    for (const auto& fn : srcnames) {
        srccmake << ' ' << fn;
    }
    srccmake << ")\n";
    srccmake.close();
}

void AutoProject::writeTopLevel() const {
    // write CMakeLists.txt top level to projname
    std::ofstream topcmake(projname + "/CMakeLists.txt");
    topcmake << 
            "cmake_minimum_required(VERSION 2.8)\n"
            "project(" << projname << ")\n"
            "add_subdirectory(src)\n";
}

bool AutoProject::isIndented(const std::string& line) const {
    size_t indent = line.find_first_not_of(' ');
    if (indent >= indentLevel && indent != std::string::npos) {
        return true;
    }
    return !(indent == 0);
}

void AutoProject::emit(std::ostream& out, const std::string &line) const {
    if (line.size() < indentLevel) {
        out << line << '\n';
    } else {
        out << (line[0] == ' ' ? line.substr(indentLevel) : line.substr(1)) << '\n';
    }
}
