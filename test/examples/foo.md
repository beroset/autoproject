Reviewing code doesn't necessarily require actually *building* it, but it's often helpful to do so in order to evaluate fully.  I usually create a `CMake` project and build from there.  Since we're all about code here, naturally, I decided to automate part of the process.  Specifically, here's how to use this code:  

 1. Hit "edit" on the question of interest that contains one or more C or C++ source code files
 2. Select the entire editable question and paste into a local text file with some appropriate name (this one might be `autoproject.md`)
 3. run this code using the command line `autoproject project.md`

This will automatically parse the `project.md` file and extract the files it finds to a directory tree like this.

<!-- language: lang-none -->

    project
    ├── build                   (empty subdirectory)
    ├── CMakeLists.txt          (generated)
    └── src
        ├── CMakeLists.txt      (generated)
        ├── project.cpp         (extracted)
        ├── test.cpp            (extracted)
        └── project.h           (extracted)


For much code in many questions, all that is then required is to navigate to the `build` directory and then type:

    cmake ..
    make 

The executable (if succefully created) will be created in `build/src` and will be named `project` (or whatever more meaningful name you have given the original `.md` file).  Note that this will **not work** if there are special things needed by the code in question.  For instance, this code itself will **not** build unless this line is added to the `src/CMakeLists.txt` file (assuming `g++`):

    target_link_libraries(${EXECUTABLE_NAME} stdc++fs)

The reason is that it uses the C++17 `filesystem` feature which is still in the `experimental` namespace and so must, for now [must be linked with `libstdc++fs`](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using.html#manual.intro.using.flags).  Note also, that `CMake` will automatically use the environment variables `CFLAGS` and `CXXFLAGS`.  My setup, which works well for many programs including this one includes `CXXFLAGS="-Wall -Wextra -pedantic -std=c++14"`.  The important part here is that this particular program should be compiled with C++14 compatibility.  I have not yet tried this code on plaforms other than Linux.

I'm interested in a general code review, and particularly if there are improvements that could or should be made to the design or interface to the `AutoProject` class.

Here are the files:

## AutoProject.h

    #ifndef AUTOPROJECT_H
    #define AUTOPROJECT_H
    #include <string>
    #include <fstream>
    #include <vector>
    #include <exception>
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
     
    class FileExtensionException : public std::runtime_error
    {
    public:
        FileExtensionException(const std::string& msg) :
            std::runtime_error(msg)
        {} 
    };

    class AutoProject {
    public:
        AutoProject() = default;
        AutoProject(fs::path mdFilename);
        void open(fs::path mdFilename);

        bool createProject();
        void writeTopLevel() const;
        void writeSrcLevel() const;
        void copyFile() const;
        const std::vector<fs::path>&filenames() const {
            return srcnames;
        }

        static const std::string mdextension;  
    private:
        bool isIndented(const std::string& line) const;
        void emit(std::ostream& out, const std::string &line) const;
        std::string trimExtras(std::string& line) const;
        bool isSourceFilename(std::string& line) const;

        static std::string& trim(std::string& str, char ch);
        static std::string& rtrim(std::string& str, char ch);
        static constexpr unsigned indentLevel{4};
        fs::path mdfile;
        std::string projname;
        std::string srcdir;
        std::ifstream in;
        std::vector<fs::path> srcnames;
    };
    #endif // AUTOPROJECT_H

## AutoProject.cpp

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
                if (!line.empty() && line[0] != ' ') {
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
        out << (line.size() >= indentLevel ? line.substr(indentLevel) : line) << '\n';
    }

## main.cpp

    #include <iostream>
    #include "AutoProject.h"

    int main(int argc, char *argv[])
    {
        if (argc != 2) {
            std::cerr << "Usage: autoproject project.md\nCreates a CMake build tree under 'project' subdirectory\n";
            return 0;
        }
        AutoProject ap;
        try {
            ap.open(argv[1]);
        }
        catch(std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
            return 1;
        }
        if (ap.createProject()) {
            std::cout << "Successfully extracted the following source files:\n";
            for (const auto& file : ap.filenames()) {
                std::cout << file << '\n';
            }
        }
    }
