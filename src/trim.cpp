#include "trim.h"
#include <algorithm>

std::string& trim(std::string& str, const std::string_view pattern) {
    if (str.starts_with(pattern)) {
        str.erase(0, pattern.size());
    }
    return str;
}

std::string& rtrim(std::string& str, const std::string_view pattern) {
    if (str.ends_with(pattern)) {
        str.erase(str.size() - pattern.size(), pattern.size());
    }
    return str;
}

std::string& trim(std::string& str, char ch) {
    auto it{str.begin()};
    for ( ; (*it == ch || isspace(*it)) && it != str.end(); ++it)
    { }
    if (it != str.end()) {
        str.erase(str.begin(), it);
    }
    return str;
}

std::string& rtrim(std::string& str, char ch) {
    std::reverse(str.begin(), str.end());
    trim(str, ch);
    std::reverse(str.begin(), str.end());
    return str;
}
