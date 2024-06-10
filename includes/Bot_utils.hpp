#pragma once

#include <string>
#include <vector>

std::string cmdHelp();
std::vector<std::string> splitString(const std::string &str, char delim);
void trim(std::string &line);
