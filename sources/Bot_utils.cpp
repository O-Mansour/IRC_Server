#include "Bot_utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using std::ifstream;

std::string cmdHelp() {
  std::stringstream ss;

  ss << "available commands: ";
  ss << "!quote: returns a quote. ";

  return ss.str().c_str();
}

std::string cmdQuote() {
  std::map<int, std::string> quotes;
  quotes[1] =
      "The only way to do great work is to love what you do. - Steve Jobs";
  quotes[2] = "Success is not how high you have climbed, but how you make a "
              "positive difference to the world. - Roy T. Bennett";
  quotes[3] = "Your time is limited, don't waste it living someone else's "
              "life. - Steve Jobs";
  quotes[4] =
      "The best way to predict your future is to create it. - Peter Drucker";
  quotes[5] = "Do what you can, with what you have, where you are. - Theodore "
              "Roosevelt";
  quotes[6] = "You miss 100% of the shots you don't take. - Wayne Gretzky";
  quotes[7] = "Believe you can and you're halfway there. - Theodore Roosevelt";
  quotes[8] = "Hardships often prepare ordinary people for an extraordinary "
              "destiny. - C.S. Lewis";
  quotes[9] = "Don’t Let Yesterday Take Up Too Much Of Today. - Will Rogers";
  quotes[10] = "It’s Not Whether You Get Knocked Down, It’s Whether You Get "
               "Up. - Vince Lombardi";
  quotes[11] = "If you are not willing to risk the usual, you will have to "
               "settle for the ordinary. - Jim Rohn";
  quotes[12] = "The purpose of our lives is to be happy. - Dalai Lama";
  quotes[13] =
      "Life is what happens when you're busy making other plans. - John Lennon";
  quotes[14] = "Get busy living or get busy dying. - Stephen King";
  quotes[15] = "You have within you right now, everything you need to deal "
               "with whatever the world can throw at you. - Brian Tracy";

  std::srand(time(0));
  int randomIndex = std::rand() % quotes.size() + 1;

  return quotes[randomIndex];
}

std::vector<std::string> splitString(const std::string &str, char delim) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) {
    trim(token);
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

std::string getFileContent(std::string file_name) {
  std::ifstream fs(file_name.c_str());
  if (!fs.is_open()) {
    std::cerr << "Error: Could not open file " << file_name << std::endl;
    return "";
  }

  std::stringstream ss;
  ss << fs.rdbuf(); // reading from file

  if (fs.fail()) {
    std::cerr << "Error: Reading file " << file_name << " failed" << std::endl;
    return "";
  }

  fs.close();
  if (fs.fail()) {
    std::cerr << "Error: Closing file " << file_name << " failed" << std::endl;
    return "";
  }

  return ss.str();
}

std::vector<std::string> extractData() {
  std::vector<std::string> db;
  std::string line;

  std::string fileContent = getFileContent("data/bad_words.txt");
  std::stringstream ss(fileContent);

  while (getline(ss, line)) {
    std::cout << line << std::endl;
    std::vector<std::string> tmp = splitString(line, ',');
    if (!tmp.empty())
      db.insert(db.end(), tmp.begin(), tmp.end());
  }
  return db;
}
