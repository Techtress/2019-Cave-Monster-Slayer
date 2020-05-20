#ifndef HITBOX_H
#define HITBOX_H

#include "../vector2.h"
#include "../soundset.h"
#include "animation.h"

#include <algorithm>
#include <list>

class Entity;

class HitBoxModel
{
 public:
  HitBoxModel(const XMLTag&);
  ~HitBoxModel();

  float getLife() const { return life; }
  float getRadius() const { return radius; }
  const Vec2f &getOffset() const { return offset; }
  void playSound() const { if (!soundSet.empty()) soundSet.playRandomSound(); }
  float getDamage() const { return damage; }
  float getImpact() const { return impact; }

 HitBoxModel(HitBoxModel&& rhs) : radius(rhs.radius), life(rhs.life), offset(rhs.offset), soundSet(std::move(rhs.soundSet)), damage(rhs.damage), impact(rhs.impact) {}
  
 private:
  float radius;
  float life;
  Vec2f offset;
  SoundSet soundSet;

  // Game logic
  float damage;
  float impact;
};

class HitBox
{
 public:
  HitBox();
  ~HitBox();

  void activate(const HitBoxModel* m, Entity*o, int hitMask, const Vec2f &pos, Animation::Direction dir);
  void update(float delta);
  bool alive() const { return life > 0 || soundQueue > 0; }
  
  const HitBoxModel *getModel() const { return model; }
  const Vec2f &getPosition() const { return position; }
  Animation::Direction getDirection() const { return direction; }

  HitBox(const HitBox&) = delete;
  HitBox &operator=(const HitBox&) = delete;
  
 private:
  const HitBoxModel *model;
  Entity *owner;
  int mask;
  float life;
  Vec2f position;
  Animation::Direction direction;
  std::list<Entity*> hitList;
  int soundQueue;
  float soundTimer;
};

class HitBoxFactory
{
 public:
  static HitBoxFactory &getInstance();

  void spawnHitBox(const HitBoxModel*, Entity*, int hitMask, const Vec2f &pos, Animation::Direction dir);

  void updateActiveList(float delta);

  void debugDraw() const;
  
 private:
  HitBoxFactory();
  ~HitBoxFactory();
  std::list<HitBox*> freeList, activeList;
};

#endif
