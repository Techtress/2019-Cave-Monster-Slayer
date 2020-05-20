#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <map>

#include "../soundset.h"

class XMLTag;
class Image;
class Sound;

class Animation
{
 public:
  Animation(const XMLTag&, const Image*);

  enum Direction {
    DIR_RIGHT,
    DIR_LEFT
  };

  unsigned getImageFrame(Direction, unsigned animFrame) const;
  const Image *getImage() const { return image; }

  unsigned getNumFrames(Animation::Direction dir) const { return frames.at(dir).size(); }
  float getSpeed() const { return speed; }
  bool loops() const { return loop; }

  const SoundSet &getSoundSet() const { return soundSet; }

  Animation() = delete;
  Animation(const Animation&) = delete;
  Animation &operator=(const Animation&) = delete;
  
 private:
  const Image *image;
  std::map<unsigned, std::vector<unsigned>> frames;
  float speed;
  bool loop;
  SoundSet soundSet;
};

#endif
