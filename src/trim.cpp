#include "trim.h"
#include <algorithm>
#include <ranges>

std::string& trim(std::string& str, const std::string_view pattern) {
    auto view = str 
        | std::views::drop_while([](char x){ return std::isspace(x); })
        ;
    str = {view.begin(), view.end()};
    if (str.starts_with(pattern)) {
        str.erase(0, pattern.size());
    }
    auto view2 = str 
        | std::views::drop_while([](char x){ return std::isspace(x); })
        ;
    return str = {view2.begin(), view2.end()};
}

std::string& rtrim(std::string& str, const std::string_view pattern) {
    auto view = str 
        | std::views::reverse
        | std::views::drop_while([](char x){ return std::isspace(x); })
        | std::views::reverse
        ;
    str = {view.begin(), view.end()};
    if (str.ends_with(pattern)) {
        str.erase(str.size() - pattern.size(), pattern.size());
    }
    auto view2 = str 
        | std::views::reverse
        | std::views::drop_while([](char x){ return std::isspace(x); })
        | std::views::reverse
        ;
    return str = {view2.begin(), view2.end()};
}

std::string& trim(std::string& str, char ch) {
#if __cpp_lib_ranges_to_container
    return str = str 
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        | std::ranges::to<std::string>()
        ;
#else
    auto view = str
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        ;
    return str = {view.begin(), view.end()};
#endif
}

std::string& rtrim(std::string& str, char ch) {
#if __cpp_lib_ranges_to_container
    return str = str 
        | std::views::reverse
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        | std::views::reverse
        | std::ranges::to<std::string>()
        ;
#else
    auto view = str 
        | std::views::reverse
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        | std::views::reverse
        ;
    return str = {view.begin(), view.end()};
#endif
}

std::string& doubletrim(std::string& str, char ch) {
#if __cpp_lib_ranges_to_container
    return str = str 
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        | std::views::reverse
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        | std::views::reverse
        | std::ranges::to<std::string>()
        ;
#else
    auto view = str 
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        | std::views::reverse
        | std::views::drop_while([ch](char x){ return std::isspace(x) || x==ch; })
        | std::views::reverse
        ;
    return str = {view.begin(), view.end()};
#endif
}

std::string& doubletrim(std::string& str, const std::string_view front_pattern, const std::string_view back_pattern)  {
    return rtrim(trim(str, front_pattern), back_pattern);
}
