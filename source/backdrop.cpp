#include "backdrop.h"
#include "image.h"

Backdrop::Backdrop(const Image *img, const Vec2f &pos, int f, float a, float sx, float sy, bool fh, bool fv) :
  image(img),
  position(pos),
  frame(f),
  angle(a),
  scaleX(sx),
  scaleY(sy),
  flipH(fh),
  flipV(fv) {}

void Backdrop::draw(float scrollFactor) const
{
  image->draw( position[0], position[1], frame, scrollFactor, angle, scaleX, scaleY, flipH, flipV);
}
