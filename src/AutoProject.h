#ifndef AUTOPROJECT_H
#define AUTOPROJECT_H
#include <string>
#include <string_view>
#include <fstream>
#include <unordered_set>
#include <exception>
#include <functional>
#include <filesystem>
namespace fs = std::filesystem;

struct path_hash {
    std::size_t operator()(const fs::path &path) const {
        return hash_value(path);
    }
};

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
    friend std::ostream& operator<<(std::ostream& out, const AutoProject &ap);

    static const std::string mdextension;  
private:
    void makeTree();
    void checkRules(const std::string &line);
    bool isNonEmptyIndented(const std::string& line) const;
    bool isIndentedOrEmpty(const std::string& line) const;
    bool isEmptyOrUnderline(const std::string& line) const;
    bool isDelimited(const std::string& line) const;
    std::string &replaceLeadingTabs(std::string &line) const;
    void emit(std::ostream& out, const std::string &line) const;
    void emitVerbatim(std::ostream& out, const std::string &line) const;
    std::string trimExtras(std::string& line) const;
    bool isSourceFilename(std::string& line) const;

    static constexpr unsigned indentLevel{4};
    static constexpr unsigned delimLength{3};
    fs::path mdfile;
    std::string projname;
    std::string srcdir;
    std::ifstream in;
    std::unordered_set<fs::path, path_hash> srcnames;
    std::unordered_set<std::string> extraRules;
};
#endif // AUTOPROJECT_H
