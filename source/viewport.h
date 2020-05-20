#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "vector2.h"

class GameConfig;

class Viewport {
public:
  static Viewport& getInstance();
  
  void draw() const;
  void update(float delta);

  void setTarget(const Vec2f &t) { target = t; }
  const Vec2f &getTarget() const { return target; }

  Vec2f getMouseWorldPos(float scrollFactor = 1.f) const;
  Vec2f getScreenPos(const Vec2f &world, float scrollFactor = 1.f) const;

  Vec2f getPosition() const { return viewPos; }
  float getX() const  { return viewPos[0]; }
  void  setX(float x) { viewPos[0] = x; }
  float getY() const  { return viewPos[1]; }
  void  setY(float y) { viewPos[1] = y; }

  void setZoomFactor(float z) { zoomFactor = z; }
  
  int getWidth() const { return viewWidth; }
  int getHeight() const { return viewHeight; }
  float getZoomFactor() const { return zoomFactor; }

  Viewport(const Viewport&) = delete;
  Viewport& operator=(const Viewport&) = delete;

private:
  const GameConfig& cfg;
  Vec2f viewPos;
  int viewWidth;
  int viewHeight;
  float zoomFactor;
  Vec2f target;

  Viewport();
};
#endif
