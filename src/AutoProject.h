#ifndef AUTOPROJECT_H
#define AUTOPROJECT_H
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>

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
    AutoProject(fs::path mdFilename, fs::path rulesfilename, fs::path toplevelfilename, fs::path srclevelfilename);
    void open(fs::path mdFilename, fs::path rulesfilename, fs::path toplevelfilename, fs::path srclevelfilename);
    // create the project
    bool createProject(bool overwrite);
    /// print final status to `out`
    friend std::ostream& operator<<(std::ostream& out, const AutoProject &ap);

private:
    void writeTopLevel() const;
    void writeSrcLevel() const;
    void makeTree(bool overwrite);
    /*! check the passed line against the rule set.
     *
     * If it matches, add the corresponding rule to `extraRules`.
     */
    void checkRules(const std::string &line);

    fs::path mdfile;
    std::string projname;
    std::string srcdir;
    std::ifstream in;
    fs::path rulesfilename;
    fs::path toplevelfilename;
    fs::path srclevelfilename;
    std::unordered_set<fs::path, path_hash> srcnames;
    std::unordered_set<std::string> extraRules;
    std::unordered_set<std::string> libraries;
};
#endif // AUTOPROJECT_H
