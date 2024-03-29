#include "ConfigFile.h"
#include "config.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cctype>
#include <regex>
#include <string>
#include <unordered_map>
#include <filesystem>


// helper functions
static std::string tolower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); }); 
    return str;
}

static const std::regex comment_regex{R"x(\s*[;#].*)x"};
static const std::regex section_regex{R"x(\s*\[([^\]]+)\]\s*)x"};
static const std::regex value_regex{R"x(\s*(\S[^ \t=]*)\s*=\s*((\s?\S+)+)\s*)x"};

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
    std::string current_section;
    std::smatch pieces;
    for (std::string line; std::getline(in, line);)
    {
        if (line.empty() || std::regex_match(line, pieces, comment_regex)) {
            // skip comment lines and blank lines
        }
        else if (std::regex_match(line, pieces, section_regex)) {
            if (pieces.size() == 2) { // exactly one match
                current_section = tolower(pieces[1].str());
            }
        }
        else if (std::regex_match(line, pieces, value_regex)) {
            if (pieces.size() == 4) { // exactly enough matches
                map[current_section][tolower(pieces[1].str())] = pieces[2].str();
            }
        }
    }
}

bool ConfigFile::rewrite(const std::string& filename) const {
    static const std::string suffix{".swp"};
    std::string current_section;
    std::smatch pieces;
    std::ifstream in(filename);
    std::ofstream out(filename + suffix);
    auto alt{*this};
    for (std::string line; std::getline(in, line);)
    {
        if (line.empty() || std::regex_match(line, pieces, comment_regex)) {
            // echo comment lines and blank lines
            out << line << '\n';
        }
        else if (std::regex_match(line, pieces, section_regex)) {
            if (pieces.size() == 2) { // exactly one match
                auto new_section{tolower(pieces[1].str())};
                if (current_section != new_section && has_section(new_section)) {
                    // finish up the current section before moving on
                    if (alt.has_section(current_section)) {
                        for (const auto &item : map.at(current_section)) {
                            if (alt.has_value(current_section, item.first)) {
                                out << '\t' << item.first << " = " << item.second << '\n';
                                alt.delete_key(current_section, item.first);
                            }
                        }
                    } else {
                        current_section = new_section;
                        out << line << '\n';
                    }
                }
            }
        }
        else if (std::regex_match(line, pieces, value_regex)) {
            if (pieces.size() == 4) { // exactly enough matches
                const auto key{tolower(pieces[1].str())};
                if (alt.has_value(current_section, key)) {
                    out << '\t' << pieces[1].str() << " = " << get_value(current_section, key) << '\n';
                    alt.delete_key(current_section, key);
                }
            }
        }
    }
    if (alt.has_section(current_section)) {
        for (auto &item : alt.map.at(current_section)) {
            out << '\t' << item.first << " = " << item.second << '\n';
            alt.delete_key(current_section, item.first);
        }
    }
    std::filesystem::remove(filename);
    std::filesystem::rename(filename + suffix, filename);
    return true;
}

bool ConfigFile::has_value(const std::string& sectionname, const std::string& keyname) const { 
    const auto sect = map.find(tolower(sectionname));
    if (sect != map.end()) {
        const auto item = sect->second.find(tolower(keyname));
        return item != sect->second.end(); 
    }
    return false;
}

std::string ConfigFile::get_value(const std::string& sectionname, const std::string& keyname) const { 
    std::string retval;
    const auto sect = map.find(tolower(sectionname));
    if (sect != map.end()) {
        const auto item = sect->second.find(tolower(keyname));
        if (item != sect->second.end()) {
            retval = item->second;
        }
    }
    return retval;
}

void ConfigFile::set_value(const std::string& sectionname, const std::string& keyname, const std::string& value) { 
    map[tolower(sectionname)][tolower(keyname)] = value;
}

void ConfigFile::delete_key(const std::string& sectionname, const std::string& keyname) {
    const auto sect = map.find(tolower(sectionname));
    if (sect != map.end()) {
        const auto item = sect->second.find(tolower(keyname));
        if (item != sect->second.end()) {
            sect->second.erase(item);
            if (sect->second.size() == 0) {
                map.erase(sect);
            }
        }
    }
}

bool ConfigFile::has_section(const std::string& sectionname) const {
    const auto sect = map.find(tolower(sectionname));
    return sect != map.end(); 
}

void ConfigFile::delete_section(const std::string& sectionname) {
    const auto sect = map.find(tolower(sectionname));
    if (sect != map.end()) {
        map.erase(sect);
    }
}

bool ConfigFile::operator==(const ConfigFile& other) const {
    return map == other.map;
}

std::ostream& operator<<(std::ostream& out, const ConfigFile& cfg) {
    for (const auto &sect : cfg.map) {
        out << "[" << sect.first << "]\n";
        for (const auto &item : sect.second) {
            out << '\t' << item.first << " = " << item.second << '\n';
        }
    }
    return out;
}
