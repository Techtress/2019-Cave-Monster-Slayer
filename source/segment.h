#ifndef SEGMENT_H
#define SEGMENT_H

#include "vector2.h"

class Segment
{
 public:
  Segment(const Vec2f &a, const Vec2f &b) : v{a, b} {}
  Segment() : v{Vec2f(), Vec2f()} {}
  
  Vec2f &operator[](int index) { return v[index]; }
  const Vec2f &operator[](int index) const { return v[index]; }

  Vec2f getCenter() const { return (v[0]+v[1])*.5f; }
  
 private:
  Vec2f v[2];
};

#endif
