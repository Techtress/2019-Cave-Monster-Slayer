#include "viewport.h"
#include "gameconfig.h"
#include "iomod.h"
#include "clock.h"

#include <sstream>
#include <SDL.h>

Viewport& Viewport::getInstance()
{ 
  static Viewport viewport;
  return viewport;
}

Viewport::Viewport() : 
  cfg(GameConfig::getInstance()),
  viewPos(0, 0),
  viewWidth(cfg["view"]["width"].toInt()), 
  viewHeight(cfg["view"]["height"].toInt()),
  zoomFactor(1.f),
  target(Vec2f(0,0))
{
}

void Viewport::draw() const
{
  // Write FPS to screen
  /*std::stringstream fps;
  fps << static_cast<int>(Clock::getInstance().getFPS()+.5f);
  IoMod::getInstance().
  writeText("FPS: " + fps.str(), 30, 50, {255, 100, 100, 255});*/

  // Write name to screen
  //IoMod::getInstance().
  IoMod::getInstance().writeText(GameConfig::getInstance()["author"].toStr(), 30, viewHeight - 56);
}

void Viewport::update(float delta)
{
  target[0] = std::max(target[0], (viewWidth/2)/zoomFactor);
  target[1] = std::max(target[1], (viewHeight/2)/zoomFactor);
  viewPos += (target - viewPos) / (0.08f / delta);
}

Vec2f Viewport::getMouseWorldPos(float scrollFactor) const
{
  int x, y;
  SDL_GetMouseState(&x, &y);
  return Vec2f( viewPos[0]*scrollFactor+(x/zoomFactor) - (getWidth()/2)/zoomFactor,
		viewPos[1]*scrollFactor+(y/zoomFactor) - (getHeight()/2)/zoomFactor );
}

Vec2f Viewport::getScreenPos(const Vec2f &world, float scrollFactor) const
{
  return (world - viewPos*scrollFactor) * zoomFactor + Vec2f(getWidth(), getHeight())*.5;
}
