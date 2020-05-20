#ifndef ANIMATIONSET_H
#define ANIMATIONSET_H

#include "animation.h"
#include "../xmltag.h"

#include <unordered_map>

class AnimationSet
{
 public:
  AnimationSet(const XMLTag&);

  const Animation &getAnimation(const std::string&) const;
  
  AnimationSet() = delete;
  AnimationSet(const AnimationSet&) = delete;
  AnimationSet &operator=(const AnimationSet&) = delete;
  
 private:
  std::unordered_map<std::string, Animation> animations;
};

#endif
