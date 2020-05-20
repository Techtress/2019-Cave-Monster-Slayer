#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>

#include "vector2.h"

class SDL_Renderer;
class SDL_Surface;
class SDL_Texture;

class ImageFactory;

class Image {
public:
  Image(const std::string &n, SDL_Surface*, SDL_Texture*);

  const std::string &getName() const { return name; }

  void draw(int dx, int dy, float scrollFactor, float scaleX = 1.f, float scaleY = 1.f) const;
  void draw(int dx, int dy, unsigned frame, float scrollFactor,
	    float angle = 0.f,
	    float scaleX = 1.f, float scaleY = 1.f,
	    bool flipH = false, bool flipV = false) const;

  void drawChunk(int dx, int dy, unsigned frame, float sx, float sy, float sw, float sh, float alpha, float scrollFactor) const;

  // Unaffected by viewport position/zoom
  void superDraw(int dx, int dy, float scale, float alpha) const;
  void superDraw(int dx, int dy, unsigned frame) const;

  // Does not take frame center into account, or viewport
  void uiDraw(int dx, int dy, unsigned frame, float width, float height) const;

  int getWidth()  const;
  int getHeight() const;
  SDL_Surface *getSurface() const { return surface; }
  SDL_Texture *getTexture() const { return texture; }
  int getNumFrames() const { return frames.size(); }

  int getFrameWidth(int i) const { return frames[i].w; }
  int getFrameHeight(int i) const { return frames[i].h; }
  int getFrameCenterX(int i) const { return frames[i].ox; }
  int getFrameCenterY(int i) const { return frames[i].oy; }
  
  const std::vector<Vec2f> &getShapeVertices(unsigned frame) const { return frames.at(frame).vertices; }
  const std::pair<Vec2f,Vec2f> &getShapeBounds(unsigned frame) const { return frames.at(frame).vbounds; }

  Image() = delete;
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;

  // The format of a '.frame' file
  struct Frame
  {
    Frame() : x(0), y(0), w(0), h(0), ox(0), oy(0), vertices(), vbounds(std::make_pair(Vec2f(0,0), Vec2f(0,0))) {}
    unsigned short x, y, w, h;
    short ox, oy;
    std::vector<Vec2f> vertices;

    // center + half dimensions
    std::pair<Vec2f, Vec2f> vbounds;
  };

private:
  friend class ImageFactory;

  std::string name;
  
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;

  std::vector<Frame> frames;
  std::vector< std::vector<Vec2f> > shapes; // correspond to frames
};

#endif
