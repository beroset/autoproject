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
