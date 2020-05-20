#ifndef ACTORMODEL_H
#define ACTORMODEL_H

#include "animationset.h"
#include "actorphysicsmodel.h"
#include "hitbox.h" // soon to be just model
#include "soundset.h"

#include "../entitymodel.h"

class XMLTag;
class AIBehavior;

struct ActorAttributes
{
  ActorAttributes() : health(0.f), super(false) {}
  ActorAttributes(const XMLTag&);
  float health;
  bool super;
};

class ActorModel : public EntityModel
{
 public:
  ActorModel(const XMLTag&);
  virtual ~ActorModel();

  const AnimationSet &getAnimSet() const { return animSet; }
  const ActorPhysicsModel &getPhysics() const { return physics; }
  const AIBehavior *getBehavior() const { return aiBehavior; }
  const ActorAttributes &getAttributes() const { return attributes; }

  struct Attack
  {
    Attack(const XMLTag&);
    
    // TO-DO: add support for many with a priority list (or have that list within hitbox)
    HitBoxModel hitBox;
    std::string animation;
    float hitDelay;

  Attack(Attack&& rhs) : hitBox(std::move(rhs.hitBox)), animation(std::move(rhs.animation)), hitDelay(rhs.hitDelay) {}
  };

  // After I add animations, this will be a bit more than just a "hit box"
  // I also need an option for firing projectiles, etc...
  const Attack &getAttack(int id) const { return attackSet.at(id); }

  void playDeathSound() const { if (!deathSound.empty()) deathSound.playRandomSound(); }

  ActorModel() = delete;
  ActorModel(const ActorModel&) = delete;
  ActorModel &operator=(const ActorModel&) = delete;

 private:
  const AnimationSet animSet;
  const ActorPhysicsModel physics;
  const AIBehavior *aiBehavior;

  // Closer to game logic stuff...
  const ActorAttributes attributes;
  std::vector<Attack> attackSet;

  SoundSet deathSound;
};

#endif
