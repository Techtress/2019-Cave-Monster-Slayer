#include <sstream>

#include "xmltag.h"

XMLTag::XMLTag(const std::string &name) :
  parent(nullptr), children(), childIndex(), data(std::make_pair(name, "")) {}

int XMLTag::toInt() const
{
  int v;
  std::stringstream(data.second) >> v;
  return v;
}

float XMLTag::toFloat() const
{
  float v;
  std::stringstream(data.second) >> v;
  return v;
}

bool XMLTag::toBool() const
{
  return data.second == "true";
}

Vec2f XMLTag::toVec2f() const
{
  return Vec2f(getChild("x").toFloat(), getChild("y").toFloat());
}

const XMLTag& XMLTag::getChild( const std::string &name ) const
{
  auto itr = childIndex.find(name);
  if (itr == childIndex.end())
    throw std::string("Cannot find XML tag '" + name + "'");
  return *children[itr->second];
}

bool XMLTag::hasChild( const std::string &name ) const
{
  return childIndex.find(name) != childIndex.end();
}
