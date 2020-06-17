#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <string>
#include <unordered_map>
#include <iostream>


class ConfigFile
{
public:
    ConfigFile(const std::string& filename);
    ConfigFile(std::istream& in);
    std::string get_value(const std::string& sectionname, const std::string& keyname) const;
    friend std::ostream& operator<<(std::ostream& out, const ConfigFile& cfg) {
        for (const auto &sect : cfg.map) {
            out << "[" << sect.first << "]\n";
            for (const auto &item : sect.second) {
                out << '\t' << item.first << " = " << item.second << '\n';
            }
        }
        return out;
    }

private:
    void parse(std::istream& in);
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> map;
};

#endif // CONFIGFILE_H
