#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

class BoundingBox
{
 public:
  BoundingBox(float l, float t, float r, float b) : v{l,t,r,b} {}

  float &operator[](unsigned i) { return v[i]; }
  float operator[](unsigned i) const { return v[i]; }

 private:
  float v[4];
};

#endif
