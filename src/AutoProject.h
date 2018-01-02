#ifndef AUTOPROJECT_H
#define AUTOPROJECT_H
#include <string>
#include <fstream>
#include <unordered_set>
#include <exception>
#include <functional>

// when we get C++17, we can use that include and namespace and 
// can eliminate the hash function.
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
 
namespace std {
    template <>
    struct hash<fs::path> {
        std::size_t operator()(const fs::path &path) const {
            return std::hash<std::string>()(path.c_str());
        }
    };
}

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
    const std::unordered_set<fs::path>&filenames() const {
        return srcnames;
    }

    static const std::string mdextension;  
private:
    void makeTree();
    void checkRules(const std::string &line);
    bool isNonEmptyIndented(const std::string& line) const;
    bool isIndentedOrEmpty(const std::string& line) const;
    bool isEmptyOrUnderline(const std::string& line) const;
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
    std::unordered_set<fs::path> srcnames;
    std::unordered_set<std::string> extraRules;
};
#endif // AUTOPROJECT_H
