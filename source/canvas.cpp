#include "canvas.h"
#include "entity.h"
#include "image.h"
#include "viewport.h"

Canvas::Canvas() : layers()
{
  layers[0];
}

Canvas& Canvas::getInstance()
{
  static Canvas instance;
  return instance;
}

void Canvas::draw() const
{  
  const Viewport &v = Viewport::getInstance();
  for (const auto &l : layers) {
    float scroll = l.second.scroll;
    const Image *bk = l.second.background;
    
    // TO-DO: Rather than having an expensive alpha render over the whole thing, once
    // I port this from SDL to OpenGL ot Vulkan, I will have the backdrops draw in a
    // shader with an alterable "mist" value that changes the draw color. That will
    // be much more efficient.
    if (bk != nullptr) {    
      float zoom = v.getZoomFactor();
      //int vw = v.getWidth()/scroll;//((v.getWidth())/scroll)/zoom;
      //int vh = v.getHeight()/zoom;//((v.getHeight())/scroll)/zoom;

      int w = bk->getWidth() * zoom;
      int h = bk->getHeight() * zoom;

      int vtop = (v.getY())*zoom*scroll - v.getHeight()/2;
      int vleft = (v.getX())*zoom*scroll - v.getWidth()/2;
      
      int x = -vleft;
      int y = -vtop;

      x += w * (int)((vleft-(vleft<0?w:0))/w);
      y += h * (int)((vtop-(vtop<0?h:0))/h);
      
      int tx = x;
      while (tx < v.getWidth()) {
	bk->superDraw(tx, y, zoom, l.second.bgAlpha);
	//bk->superDraw(tx, y, zoom, 1.f);
	int ty = y+h;
	while (ty < v.getHeight()) {
	  bk->superDraw(tx, ty, zoom, l.second.bgAlpha);
	  //bk->superDraw(tx, ty, zoom, 1.f);
	  ty += h;
	}
	tx += w;
      }
      /*while (x < -w)
	x += w;
      while (x > 0)
	x -= w;
      while (y < -h)
	y += h;
      while (y > 0)
      y -= h;*/
      
      //x *= scroll;
      //y *= scroll;
      

      /*float zoom = v.getZoomFactor();

      float vw = v.getWidth()/zoom;
      float vh = v.getHeight()/zoom;

      int x = v.getX()*scroll;
      int y = v.getY()*scroll;
      int w = bk->getWidth();
      int h = bk->getHeight();
      
      x = (x/w)*w - vw*.5;
      y = (y/h)*h - vh*.5;

      int tx = x;
      while (tx < v.getX()*scroll+vw) {
	bk->draw(tx, y, scroll);
	int ty = y+h;
	while (ty < v.getY()*scroll+vh) {
	  bk->draw(tx, ty, scroll);
	  ty += h;
	}
	tx += w;
	}*/
    }
    for (const Entity *e : l.second.entities)
      e->draw( scroll );
    for (const Backdrop &b : l.second.backdrops)
      b.draw( scroll );
  }
}

Backdrop *Canvas::placeBackdrop(int layerID, const Image* img, const Vec2f &position, int frame, float angle, float scaleX, float scaleY, bool flipH, bool flipV)
{
  layers[layerID].backdrops.emplace_back( img, position, frame, angle, scaleX, scaleY, flipH, flipV );
  return &layers[layerID].backdrops.back();
}

void Canvas::updateEntityList()
{
  for (auto &l : layers) {
    for (auto it = l.second.entities.begin(); it != l.second.entities.end();) {
      if (!(*it)->isActive()) {
	it = l.second.entities.erase(it);
      } else ++it;
    }
  }
}
