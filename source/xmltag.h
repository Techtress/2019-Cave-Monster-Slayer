#ifndef XMLTAG_H
#define XMLTAG_H

#include "vector2.h"

#include <vector>
#include <unordered_map>

class XMLParser;

class XMLTag
{
 public:
  XMLTag(const std::string& n);
  ~XMLTag() { for (XMLTag* c : children) delete c; }
  
  const std::string &getName() const { return data.first; }

  const std::string &toStr() const { return data.second; }
  int toInt() const;
  float toFloat() const;
  bool toBool() const;
  Vec2f toVec2f() const;

  const XMLTag& getParent() const { return *parent; }
  const XMLTag& getChild(const std::string& name) const;
  const std::vector<XMLTag*> &getChildren() const { return children; }

  const XMLTag& operator[](const std::string& name) const { return getChild(name); }

  bool hasChild( const std::string &name ) const;

  XMLTag( const XMLTag& ) = delete;
  XMLTag &operator=( const XMLTag& ) = delete;
  
 private:
  friend class XMLParser;
  XMLTag *parent;
  std::vector<XMLTag*> children;
  std::unordered_map<std::string, unsigned> childIndex;
  std::pair<std::string, std::string> data;
};

#endif
