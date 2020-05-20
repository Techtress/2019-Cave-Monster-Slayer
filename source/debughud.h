#ifndef DEBUGHUD_H
#define DEBUGHUD_H

#include <string>
#include <map>

class DebugHUD
{
 public:
  DebugHUD();
  ~DebugHUD() {}

  void draw() const;

  void setMessage(int lineID, const std::string& msg) { messages[lineID] = msg; }
  void toggle() { visible = !visible; }
  
 private:
  std::map<int, std::string> messages;
  bool visible;

  int tr, tg, tb, ta;
  int br, bg, bb, ba;
  int lr, lg, lb, la;
};

#endif
