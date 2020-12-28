#ifndef AUTOPROJECT_H
#define AUTOPROJECT_H
#include "config.h"
#include <exception>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <unordered_set>

#if HAS_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

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

struct LangConfig {
    fs::path configdir;
    fs::path rulesfilename;
    fs::path toplevelcmakefilename;
    fs::path srclevelcmakefilename;
    fs::path clonedir;
};

class AutoProject {
public:
    AutoProject() = default;
    AutoProject(fs::path mdFilename, std::map<std::string, LangConfig> lang);
    void open(fs::path mdFilename, std::map<std::string, LangConfig> lang);
    // create the project
    bool createProject(bool overwrite);
    /// print final status to `out`
    friend std::ostream& operator<<(std::ostream& out, const AutoProject &ap);

private:
    void writeTopLevel() const;
    void copyCloneDir() const;
    void writeSrcLevel() const;
    void makeTree(bool overwrite);
    /*! check the passed line against the rule set.
     *
     * If it matches, add the corresponding rule to `extraRules`.
     */
    void checkRules(const std::string &line);
    void checkLanguageTags(const std::string& line);

    // full path to input md file, e.g. "/tmp/248232.md"
    fs::path mdfile;
    // output directory name, e.g. "/tmp/248232"
    fs::path outdir;
    // project name, e.g. "248232"
    std::string projname;
    std::string srcdir;
    std::ifstream in;
    fs::path configdir;
    fs::path toplevelfilename;
    fs::path srclevelfilename;
    fs::path clonedir;
    std::unordered_set<fs::path, path_hash> srcnames;
    std::unordered_set<std::string> extraRules;
    std::unordered_set<std::string> libraries;
    std::string thislang;
    std::map<std::string, LangConfig> lang;
};
#endif // AUTOPROJECT_H
