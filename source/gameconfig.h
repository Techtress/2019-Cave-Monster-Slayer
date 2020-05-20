#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <string>

#include "xmlparser.h"

class XMLParser;

class GameConfig
{
 public:
  static GameConfig &getInstance();

  const XMLTag &operator[](const std::string&) const;
  
 private:
  GameConfig();
  XMLParser parser;
};

#endif
