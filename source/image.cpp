#include <SDL.h>

#include "image.h"
#include "viewport.h"
#include "rendercontext.h"

Image::Image( const std::string &n, SDL_Surface* surf, SDL_Texture *tex) :
  name(n),
  renderer(RenderContext::getInstance().getRenderer()),
  surface( surf ),
  texture( tex ),
  frames(),
  shapes() {}

void Image::draw(int dx, int dy, float scrollFactor, float sx, float sy) const
{
  const Viewport &v = Viewport::getInstance();
  float zoom = v.getZoomFactor();

  float viewx = v.getX(),
    viewy = v.getY();

  dx -= viewx * scrollFactor;
  dy -= viewy * scrollFactor;
  
  dx *= zoom;
  dy *= zoom;

  dx += v.getWidth()/2;
  dy += v.getHeight()/2;
  
  SDL_Rect src = {0, 0, surface->w, surface->h};
  SDL_Rect dest  = { dx, dy,
		     static_cast<int>(surface->w * sx * zoom + 0.5),
		     static_cast<int>(surface->h * sy * zoom + 0.5) };
  SDL_Point center = {0,0};
  SDL_SetTextureAlphaMod(texture, 255);
  SDL_RenderCopyEx(renderer, texture, &src, &dest, 0.f, &center, SDL_FLIP_NONE);
}

void Image::draw(int dx, int dy, unsigned frame, float scrollFactor,
		 float angle, float sx, float sy,
		 bool flipH, bool flipV) const
{
  const Viewport &v = Viewport::getInstance();
  float zoom = v.getZoomFactor();

  float viewx = v.getX(),
    viewy = v.getY();

  dx -= viewx * scrollFactor;
  dy -= viewy * scrollFactor;
  
  dx *= zoom;
  dy *= zoom;

  dx += v.getWidth()/2;
  dy += v.getHeight()/2;

  const Frame &f = frames.at(frame);
  
  SDL_Rect src = { f.x, f.y, f.w, f.h };
  SDL_Point center = { -static_cast<int>(f.ox * sx * zoom + 0.5),
		       -static_cast<int>(f.oy * sy * zoom + 0.5) };
  SDL_Rect dest  = { dx - center.x, dy - center.y,
		     static_cast<int>(f.w * sx * zoom + 0.5),
		     static_cast<int>(f.h * sy * zoom + 0.5) };
  
  SDL_SetTextureAlphaMod(texture, 255);
  SDL_RenderCopyEx(renderer, texture, &src, &dest, angle, &center,
		   static_cast<SDL_RendererFlip>( (flipH ? SDL_FLIP_HORIZONTAL : 0) |
						  (flipV ? SDL_FLIP_VERTICAL : 0) ) );
  //SDL_Rect src = {f.x, f.y, f.w, f.h};
  //SDL_Rect dest  = {dx + (int)(f.ox*scrollFactor), dy + (int(f.oy*scrollFactor)), (int)(f.w*scrollFactor), (int)(f.h*scrollFactor)};
  //SDL_Point center = {(int)(f.ox * scrollFactor), (int)(f.oy * scrollFactor)};
  //SDL_RenderCopyEx(renderer, texture, &src, &dest, 0.f, &center, SDL_FLIP_NONE);
}

void Image::drawChunk(int dx, int dy, unsigned frame, float sx, float sy, float sw, float sh, float alpha, float scrollFactor) const
{
  const Viewport &v = Viewport::getInstance();
  float zoom = v.getZoomFactor();

  float viewx = v.getX(),
    viewy = v.getY();

  dx -= viewx * scrollFactor;
  dy -= viewy * scrollFactor;
  
  dx *= zoom;
  dy *= zoom;

  dx += v.getWidth()/2;
  dy += v.getHeight()/2;

  const Frame &f = frames.at(frame);
  
  SDL_Rect src = { (int)(f.x+sx*f.w),
		   (int)(f.y+sy*f.h),
		   (int)(f.w*sw),
		   (int)(f.h*sh) };
  
  SDL_Point center = { -static_cast<int>(f.ox * zoom + 0.5),
		       -static_cast<int>(f.oy * zoom + 0.5) };
  SDL_Rect dest  = { dx - center.x + (int)(sx*f.w), dy - center.y + (int)(sy*f.h),
		     static_cast<int>(f.w * sw * zoom + 0.5),
		     static_cast<int>(f.h * sh * zoom + 0.5) };
  
  SDL_SetTextureAlphaMod(texture, alpha*255);
  SDL_RenderCopyEx(renderer, texture, &src, &dest, 0, &center,
		   SDL_FLIP_NONE );
}

void Image::superDraw(int dx, int dy, float scale, float alpha) const
{
  SDL_Rect src = {0, 0, surface->w, surface->h};
  SDL_Rect dest  = { dx, dy,
		     static_cast<int>(surface->w * scale + 0.5),
		     static_cast<int>(surface->h * scale + 0.5) };
  SDL_Point center = {0,0};
  (void)alpha;
  SDL_SetTextureAlphaMod(texture, alpha*255);
  SDL_RenderCopyEx(renderer, texture, &src, &dest, 0.f, &center, SDL_FLIP_NONE);
}

void Image::superDraw(int dx, int dy, unsigned frame) const
{
  const Frame &f = frames.at(frame);

  SDL_Rect src = { f.x, f.y, f.w, f.h };
  SDL_Point center = { -f.ox, -f.oy };
  SDL_Rect dest  = { dx - center.x, dy - center.y, f.w, f.h };
  
  SDL_SetTextureAlphaMod(texture, 255);
  SDL_RenderCopyEx(renderer, texture, &src, &dest, 0.f, &center, SDL_FLIP_NONE );
}

void Image::uiDraw(int dx, int dy, unsigned frame, float width, float height) const
{
  const Frame &f = frames.at(frame);

  SDL_Rect src = { f.x, f.y, f.w, f.h };
  SDL_Point center = { 0, 0 };
  SDL_Rect dest  = { dx, dy, f.w, f.h };

  if (f.w > f.h) {
    dest.w *= width/(float)f.w;
    dest.h *= width/(float)f.w;
  }
  else {
    dest.w *= height/(float)f.h;
    dest.h *= height/(float)f.h;
  }
  
  SDL_SetTextureAlphaMod(texture, 255);
  SDL_RenderCopyEx(renderer, texture, &src, &dest, 0.f, &center, SDL_FLIP_NONE );
}

int Image::getWidth()  const { return surface->w; }
int Image::getHeight() const { return surface->h; }
