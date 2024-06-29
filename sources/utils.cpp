#include "Server.hpp"

std::vector<std::string> split_line(std::string line, std::string del) {
  std::vector<std::string> res;
  char *buff = new char[line.size() + 1];
  char *word;

  buff = std::strcpy(buff, line.c_str());
  word = std::strtok(buff, del.c_str());
  while (word) {
    res.push_back(word);
    word = std::strtok(NULL, del.c_str());
  }
  delete[] buff;
  return res;
}

std::string extract_param(std::vector<std::string> &command, std::string line,
                          int argIndex) {
  size_t pos;
  for (int i = 0; i < argIndex; i++) {
    pos = line.find(command[i]);
    if (pos != std::string::npos)
      line.erase(0, pos + command[i].length());
  }
  pos = line.find(':');
  if (pos != std::string::npos)
    line.erase(0, pos + 1);
  return line;
}

std::string print_time_welcome() {
  std::time_t currentTime = std::time(NULL);
  std::tm *localTime = std::localtime(&currentTime);
  std::stringstream ss;

  ss << "[" << std::setw(4) << localTime->tm_year + 1900 << "-" << std::setw(2)
     << std::setfill('0') << localTime->tm_mon + 1 << "-" << std::setw(2)
     << std::setfill('0') << localTime->tm_mday << " " << std::setw(2)
     << std::setfill('0') << localTime->tm_hour << ":" << std::setw(2)
     << std::setfill('0') << localTime->tm_min << ":" << std::setw(2)
     << std::setfill('0') << localTime->tm_sec << "]" << std::endl;
  return ss.str();
}

void print_time() {
  std::time_t currentTime = std::time(NULL);
  std::tm *localTime = std::localtime(&currentTime);
  std::cout << "[" << std::setw(4) << localTime->tm_year + 1900 << "-"
            << std::setw(2) << std::setfill('0') << localTime->tm_mon + 1 << "-"
            << std::setw(2) << std::setfill('0') << localTime->tm_mday << " "
            << std::setw(2) << std::setfill('0') << localTime->tm_hour << ":"
            << std::setw(2) << std::setfill('0') << localTime->tm_min << ":"
            << std::setw(2) << std::setfill('0') << localTime->tm_sec << "]"
            << std::endl;
}

void print_ft_irc() {
  std::cout << GREEN
            << "██╗██████╗  ██████╗        ███████╗███████╗██████╗ ██╗   ██╗"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██╔══██╗██╔════╝        ██╔════╝██╔════╝██╔══██╗██║   ██║"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██████╔╝██║             ███████╗█████╗  ██████╔╝██║   ██║"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██╔══██╗██║             ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██║  ██║╚██████╗███████╗███████║███████╗██║  ██║ ╚████╔╝ "
            << RESET << std::endl;
  std::cout << GREEN
            << "╚═╝╚═╝  ╚═╝ ╚═════╝╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝"
            << RESET << std::endl;
}

