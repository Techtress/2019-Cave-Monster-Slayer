#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <expat.h>
#include <string>

#include "xmltag.h"

class XMLParser
{
 public:
  XMLParser( const std::string& fn );
  virtual ~XMLParser() { XML_ParserFree(parser); }

  static const unsigned int BUFSIZE = 512;

  XMLParser( const XMLParser& ) = delete;
  XMLParser &operator=( const XMLParser& ) = delete;

  static void wrapper4Start(void *data, const char *el, const char **attr);
  static void wrapper4End(void *data, const char *el);
  static void wrapper4Chars(void *data, const char *text, int textlen);

  const XMLTag &getTag( const std::string& name ) const { return rootTag[name]; }
  
 private:
  const std::string filename;
  XML_Parser parser;
  char buff[BUFSIZE];

  XMLTag rootTag;
  XMLTag *curTag;

  void parseXML();
  void start(const char *el, const char *attr[]);
  void end(const char *);
  void chars(const char *text, int textlen);

  void stripTrailWhiteSpace(std::string&) const;
};

#endif
