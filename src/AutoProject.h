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
    // create the project
    bool createProject();
    /// print final status to `out`
    friend std::ostream& operator<<(std::ostream& out, const AutoProject &ap);

private:
    void writeTopLevel() const;
    void writeSrcLevel() const;
    void makeTree();
    /*! check the passed line against the rule set.
     *
     * If it matches, add the corresponding rule to `extraRules`.
     */
    void checkRules(const std::string &line);

    fs::path mdfile;
    std::string projname;
    std::string srcdir;
    std::ifstream in;
    std::unordered_set<fs::path, path_hash> srcnames;
    std::unordered_set<std::string> extraRules;
    std::unordered_set<std::string> libraries;
};
#endif // AUTOPROJECT_H
