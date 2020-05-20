#ifndef ACTOR_H
#define ACTOR_H

#include "animationstate.h"
#include "actormodel.h"
#include "actorphysics.h"
#include "aicontroller.h"
#include "chunkexplosion.h"

#include "../entity.h"
#include "../entitymodel.h"

class ActorController;
class PlayerController;

class Actor : public Entity
{
 public:
  Actor();
  virtual ~Actor();
  
  virtual void draw(float scrollFactor) const override;
  virtual void update(float delta) override;

  virtual BoundingBox getBoundingBox() const override;
  void registerEntityCollision(Entity*) override;
  void registerHitBoxCollision(const HitBox*) override;

  AnimationState &getAnimationState() { return animState; }
  ActorPhysics &getPhysics() { return physics; }

  void setPlayerControlled(PlayerController*);
  void setAIControlled(); // gets info based on model

  // Attacks using the current model's attack with the given id
  void attack(int id, int mask);
  void jump(float factor);
  void moveX(float factor);

  float getHealthPercent() const;

  void setGod(bool g) { god = g; }
  bool isAGod() const { return god; }

  Actor(const Actor&) = delete;
  Actor &operator=(const Actor&) = delete;

  enum ActorAnim
  {
    ANIM_IDLE,
    ANIM_WALK,
    ANIM_RUN,
    ANIM_JUMP,
    ANIM_FALL,
    ANIM_TOTAL,
    ANIM_SPECIAL
  };

  void changeAnimState( ActorAnim );

  static int getMaskCounts(int m) { return maskCounts[m]; }
  
 private:
  static std::map<int,int> maskCounts;
  
  AnimationState animState;
  ActorPhysics physics;
  ChunkExplosion explosion;
  ActorController *controller;

  // Current attribute level
  ActorAttributes attributes;
  
  // Every actor should allocate one, since only a tiny percentage are players.
  // Plus things might take over them...
  AIController aiController;
  bool aiControlled;

  virtual void activateImpl() override;
  virtual void deactivateImpl() override;
  virtual void destroyImpl() override;

  void setController(ActorController*);

  ActorAnim currentAnim;

  enum ActionState
  {
    ACTION_NORMAL, // ground idle and moving
    ACTION_ATTACK // attempting to strike at something. Cannot move during this time.
  } actionState;

  float pauseTimer;

  // The id of the currently attacking attack
  int attackId;
  int attackMask;

  // Very temporary thing
  bool god;
};

#endif
