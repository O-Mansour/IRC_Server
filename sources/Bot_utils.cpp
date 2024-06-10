#include "Bot_utils.hpp"
#include <sstream>
#include <vector>

std::string cmdHelp() {
  std::stringstream ss;

  ss << "available commands : ";
  ss << "!quote : returns a quote. ";
  ss << "!time : get time of day.";

  return ss.str().c_str();
}

std::vector<std::string> splitString(const std::string &str, char delim) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) {
    tokens.push_back(token);
  }
  return tokens;
}

void trim(std::string &line) {
  size_t start = 0;
  size_t end = line.size() - 1;
  while (start <= end &&
         std::isspace(static_cast<unsigned char>(line[start]))) {
    ++start;
  }

  while (end >= start && std::isspace(static_cast<unsigned char>(line[end])))
    --end;

  if (start > end)
    line.clear();
  else
    line = line.substr(start, end - start + 1);
}
