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
