#include "xmlparser.h"

#include <fstream>
#include <iostream>
#include <cstring>

XMLParser::XMLParser( const std::string &fn ) :
  filename(fn),
  parser(NULL),
  rootTag("root"),
  curTag(&rootTag)
{
  parseXML();
}

void XMLParser::parseXML()
{
  parser = XML_ParserCreate(nullptr);
  if (!parser) {
    throw std::string("Couldn't allocate memory for parser");
  }

  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, wrapper4Start, wrapper4End);
  XML_SetCharacterDataHandler(parser, wrapper4Chars);
  std::fstream in;
  in.open(filename.c_str(), std::ios::in);
  if (!in) { 
    throw std::string("Cannot open xml file: ")+filename;
  }

  int length = 0;
  in.getline(buff, BUFSIZE);
  while ( true ) {
    if (! XML_Parse(parser, buff, strlen(buff), length)) {
      std::cout << "Parse error at line "
	         << XML_GetCurrentLineNumber(parser)
	         << XML_ErrorString(XML_GetErrorCode(parser))
           << std::endl;
      throw std::string("Couldn't parse file: ") + filename;
    }

    if ( in.eof() ) break;
    else in.getline(buff, BUFSIZE);
  }
}

void XMLParser::start(const char *el, const char *attr[])
{
  XMLTag *parent = curTag;
  
  curTag->childIndex[el] = curTag->children.size();
  curTag->children.emplace_back(new XMLTag(el));
  
  curTag = curTag->children.back();
  curTag->parent = parent;
  
  for (int i = 0; attr[i]; i += 2) {
    curTag->childIndex[attr[i]] = curTag->children.size();
    curTag->children.emplace_back(new XMLTag(attr[i]));
    curTag->children.back()->data.second = attr[i+1];
  }
}

void XMLParser::end(const char *tagEnd)
{
  if ( tagEnd != curTag->getName() ) { 
    throw std::string("Tags ") + tagEnd +" and " + curTag->getName() +
          std::string(" don't match");
  }
  curTag = curTag->parent;
}

void XMLParser::stripTrailWhiteSpace(std::string& str) const
{
  int length = str.size();   
  int i = length-1;
  while (i >= 0) { 
    if (str[i] != ' ' && str[i] != '\n' && str[i] != '\t') {
      break;
    }
    else if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t') {
      str.erase(i, 1);
    }
    --i;
  }
}

void XMLParser::chars(const char *text, int textlen)
{
  // The text is not zero terminated; thus we need the  length:
  std::string str(text, textlen);
  stripTrailWhiteSpace(str);
  if ( str.size() ) {
    curTag->data.second = str;
  }
}

void XMLParser::wrapper4Start(void *data, const char *el, const char **attr)
{
  XMLParser *parser = static_cast<XMLParser*>(data);
  parser->start(el, attr);
}

void XMLParser::wrapper4End(void *data, const char *el)
{
  XMLParser *parser = static_cast<XMLParser*>(data);
  parser->end(el);
}

void XMLParser::wrapper4Chars(void *data, const char *text, int textlen)
{
  XMLParser *parser = static_cast<XMLParser*>(data);
  parser->chars(text, textlen);
}
