#pragma once

#include <string>
#include <vector>

std::string cmdHelp();
std::string cmdQuote();
std::string getFileContent(std::string file_name);
std::vector<std::string> extractData();
std::vector<std::string> splitString(const std::string &str, char delim);
void trim(std::string &line);
