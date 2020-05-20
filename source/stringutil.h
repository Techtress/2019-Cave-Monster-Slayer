#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <sstream>

class StringUtil
{
 public:
  template<class T>
  static std::string toString(T v)
  {
    return static_cast<std::stringstream&>(std::stringstream("") << v).str();
  }

  static std::string fromBool(bool v)
  {
    return v ? "true" : "false";
  }

  static int toInt(const std::string& str)
  {
    int v;
    std::stringstream(str) >> v;
    return v;
  }
  
  static float toFloat(const std::string& str)
  {
    float v;
    std::stringstream(str) >> v;
    return v;
  }

  template<class T> static T to(const std::string& str)
  {
    T v; std::stringstream(str) >> v;
    return v;
  }
};

#endif
