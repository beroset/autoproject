#ifndef TRIM_H
#define TRIM_H
#include <string>
#include <string_view>

std::string& trim(std::string& str, const std::string_view pattern);
std::string& rtrim(std::string& str, const std::string_view pattern);
std::string& trim(std::string& str, char ch);
std::string& rtrim(std::string& str, char ch);
std::string trimExtras(std::string& line);
std::string& doubletrim(std::string& str, char ch);
std::string& doubletrim(std::string& str, const std::string_view front_pattern, const std::string_view back_pattern); 
#endif // TRIM_H
