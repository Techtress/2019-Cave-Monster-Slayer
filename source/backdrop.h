#ifndef BACKDROP_H
#define BACKDROP_H

#include "vector2.h"

class Image;
class XMLTag;

class Backdrop
{
 public:
  Backdrop(const Image*, const Vec2f &pos, int f, float angle, float sx, float sy, bool fh, bool fv);
  ~Backdrop() {}

  void draw(float scrollFactor) const;

  void setPosition(const Vec2f &pos) { position = pos; }
  const Vec2f &getPosition() const { return position; }

  void setAngle(float a) { angle = a; }
  void setScaleX(float x) { scaleX = x; }
  void setScaleY(float y) { scaleY = y; }
  void setFlipH(bool f) { flipH = f; }
  void setFlipV(bool f) { flipV = f; }

  float getAngle() const { return angle; }
  float getScaleX() const { return scaleX; }
  float getScaleY() const { return scaleY; }
  bool getFlipH() const { return flipH; }
  bool getFlipV() const { return flipV; }

  const Image *getImage() const { return image; }
  int getFrame() const { return frame; }

  Backdrop(const Backdrop&) = delete;
  Backdrop &operator=(const Backdrop&) = delete;
  
 private:
  const Image *image;
  Vec2f position;
  int frame;
  float angle;
  float scaleX, scaleY;
  bool flipH, flipV;
};

#endif
