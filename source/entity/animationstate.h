#ifndef ANIMATIONSTATE_H
#define ANIMATIONSTATE_H

#include "animation.h"

#include <string>

class AnimationSet;
class Image;

class AnimationState
{
 public:
  AnimationState();
  ~AnimationState() {}

  void activate(const AnimationSet*, const std::string &startAnim);
  void update(float delta);

  bool hasFinished() const;

  void setPlaySpeed(float s) { playSpeed = s; }
  float getPlaySpeed() const { return playSpeed; }

  void playAnimation(const std::string&);
  const std::string &getCurrentAnimName() const { return currentAnim.first; }
  const Animation &getCurrentAnim() const { return *currentAnim.second; }
  float getCurrentTime() const { return currentTime; }
  
  void setDirection(Animation::Direction);
  Animation::Direction getDirection() const { return currentDirection; }

  const AnimationSet *getAnimSet() const { return animSet; }

  std::pair<const Image*, unsigned> getDrawData() const;

  AnimationState(const AnimationState&) = delete; 
  AnimationState &operator=(const AnimationState&) = delete;
  
 private:
  const AnimationSet *animSet;

  // The current animation
  std::pair<std::string, const Animation*> currentAnim; // Should never be null while active
  float currentSoundInterval;
  float currentTime;
  float playSpeed;
  int lastSound;
  Animation::Direction currentDirection;

  void clampTime();
};

#endif
