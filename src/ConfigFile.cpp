#include "ConfigFile.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <regex>

ConfigFile::ConfigFile(const std::string& filename)
: map{} {
    std::ifstream fstrm;
    fstrm.open(filename);
    parse(fstrm);
}

ConfigFile::ConfigFile(std::istream& in) : map{} {
    parse(in);
}

void ConfigFile::parse(std::istream& in) {
    static const std::regex comment_regex{R"x(\s*[;#])x"};
    static const std::regex section_regex{R"x(\s*\[([^\]]+)\])x"};
    static const std::regex value_regex{R"x(\s*(\S[^ \t=]*)\s*=\s*((\s?\S+)+)\s*$)x"};
    std::string current_section;
    std::smatch pieces;
    for (std::string line; std::getline(in, line);)
    {
        if (line.empty() || std::regex_match(line, pieces, comment_regex)) {
            // skip comment lines and blank lines
                
        }
        else if (std::regex_match(line, pieces, section_regex)) {
            if (pieces.size() == 2) { // exactly one match
                current_section = pieces[1].str();
            }
        }
        else if (std::regex_match(line, pieces, value_regex)) {
            if (pieces.size() == 4) { // exactly enough matches
                map[current_section][pieces[1].str()] = pieces[2].str();
            }
        }
    }
}

bool ConfigFile::has_value(const std::string& sectionname, const std::string& keyname) const { 
    const auto sect = map.find(sectionname);
    if (sect != map.end()) {
        const auto item = sect->second.find(keyname);
        return item != sect->second.end(); 
    }
    return false;
}

std::string ConfigFile::get_value(const std::string& sectionname, const std::string& keyname) const { 
    std::string retval;
    const auto sect = map.find(sectionname);
    if (sect != map.end()) {
        const auto item = sect->second.find(keyname);
        if (item != sect->second.end()) {
            retval = item->second;
        }
    }
    return retval;
}

void ConfigFile::set_value(const std::string& sectionname, const std::string& keyname, const std::string& value) { 
    map[sectionname][keyname] = value;
}

void ConfigFile::delete_key(const std::string& sectionname, const std::string& keyname) {
    const auto sect = map.find(sectionname);
    if (sect != map.end()) {
        const auto item = sect->second.find(keyname);
        if (item != sect->second.end()) {
            sect->second.erase(item);
            if (sect->second.size() == 0) {
                map.erase(sect);
            }
        }
    }
}

bool ConfigFile::has_section(const std::string& sectionname) const {
    const auto sect = map.find(sectionname);
    return sect != map.end(); 
}

void ConfigFile::delete_section(const std::string& sectionname) {
    const auto sect = map.find(sectionname);
    if (sect != map.end()) {
        map.erase(sect);
    }
}
