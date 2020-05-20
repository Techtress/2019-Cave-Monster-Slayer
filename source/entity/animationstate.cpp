#include "animationstate.h"
#include "animationset.h"

#include "../soundmanager.h"

AnimationState::AnimationState() :
  animSet(nullptr),
  currentAnim(std::make_pair("BLANK", nullptr)),
  currentSoundInterval(-1.f),
  currentTime(0.f),
  playSpeed(1.f),
  lastSound(-1),
  currentDirection(Animation::DIR_RIGHT) {}

void AnimationState::activate(const AnimationSet* set, const std::string &startAnim)
{
  animSet = set;
  playAnimation(startAnim);
}

void AnimationState::playAnimation(const std::string &name)
{
  const Animation &anim = animSet->getAnimation(name);
  currentAnim = std::make_pair(name, &anim);
  currentTime = 0.f;
  currentSoundInterval = 0.f;
  lastSound = -1;

  const SoundSet &soundSet = currentAnim.second->getSoundSet();
  if (!soundSet.empty() && soundSet.getSoundInterval() < 0.f) {
    soundSet.playSound(soundSet.randomSoundID());
  }
}

void AnimationState::setDirection(Animation::Direction dir)
{
  currentDirection = dir;
  clampTime();
}

std::pair<const Image*, unsigned> AnimationState::getDrawData() const
{
  return std::make_pair( currentAnim.second->getImage(),
			 currentAnim.second->getImageFrame( currentDirection,
							    std::min(static_cast<unsigned int>(currentTime),
								     currentAnim.second->getNumFrames(currentDirection)-1) ) );
}

void AnimationState::update(float delta)
{
  currentTime += delta * currentAnim.second->getSpeed() * playSpeed;
  clampTime();

  const SoundSet &soundSet = currentAnim.second->getSoundSet();

  if (soundSet.getSoundInterval() > 0.f) {
    if (currentSoundInterval < 0.f) {
      int nextSound;
      while ((nextSound = soundSet.randomSoundID()) == lastSound) {}
      soundSet.playSound(lastSound = nextSound);
      currentSoundInterval = soundSet.getSoundInterval();
    } else currentSoundInterval -= delta;
  }
}

void AnimationState::clampTime()
{
  const Animation &anim = *currentAnim.second;
  unsigned numFrames = anim.getNumFrames(currentDirection);
  if (currentTime > numFrames)
    currentTime = anim.loops() ? currentTime - numFrames*(static_cast<unsigned>(currentTime+.5f) / numFrames) : numFrames;
}

bool AnimationState::hasFinished() const
{
  return currentAnim.second->loops() ? false : currentTime >= currentAnim.second->getNumFrames(currentDirection);
}
