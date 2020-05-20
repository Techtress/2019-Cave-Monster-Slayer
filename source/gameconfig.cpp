#include "gameconfig.h"

GameConfig &GameConfig::getInstance()
{
  static GameConfig instance;
  return instance;
}

GameConfig::GameConfig() :
  parser("xmlSpec/config.xml") {}

const XMLTag &GameConfig::operator[](const std::string& name) const
{
  return parser.getTag("Configuration")[name];
}
