#ifndef ENTITY_H
#define ENTITY_H

#include <queue>

#include "vector2.h"
#include "segment.h"
#include "boundingbox.h"

class EntityModel;
class HitBox;

class Entity
{
 public:
 Entity() : active(false), alive(false), model(nullptr), mask(0), position(Vec2f(0,0)), angle(0.f), scale(Vec2f(1.f,1.f)), offset(Vec2f(0,0)) {}
  virtual ~Entity() {}

  // Is the entity being updated and drawn?
  bool isActive() const { return active; }

  // Is the entity taking part in the world?
  // For exmaple, if it were "dead", it might be rendering an explosion, so it
  // still needs to update. However, we don't want things to interact with it while
  // it is dead so we need this flag.
  bool isAlive() const { return alive; }

  const EntityModel *getModel() const { return model; }

  int getMask() const { return mask; }
  
  const Vec2f &getPosition() const { return position; }
  float getAngle() const { return angle; }
  const Vec2f &getScale() const { return scale; }
  const Vec2f &getOffset() const { return offset; }

  // This is left for whatever type to implement
  virtual BoundingBox getBoundingBox() const { return BoundingBox(0,0,0,0); }
  virtual void registerEntityCollision(Entity*) {}
  virtual void registerHitBoxCollision(const HitBox*) {}

  void setPosition(const Vec2f &pos) { position = pos; }
  void setX(float x) { position[0] = x; }
  void setY(float y) { position[1] = y; }
  
  void setAngle(float ang) { angle = ang; }
  void setScale(const Vec2f &s) { scale = s; }
  
  void setOffset(const Vec2f &o) { offset = o; }
  void setOffsetX(float x) { offset[0] = x; }
  void setOffsetY(float y) { offset[1] = y; }

  void activate(const EntityModel* p_model, int p_mask,
		const Vec2f &pos, float ang, const Vec2f &p_scale) {
    model = p_model; mask = p_mask;
    position = pos; angle = ang; scale = p_scale;
    offset = Vec2f(0,0);
    active = alive = true;
    activateImpl(); }
  
  void deactivate() { active = alive = false; deactivateImpl(); }
  void destroy() { if (!alive) return; alive = false; destroyImpl(); }

  virtual void draw(float scrollFactor) const = 0;
  virtual void update(float delta) = 0;

  // Called by GameManager::testCollision
  virtual void notifyHitBy(Entity*) {};
  virtual void notifyEntityHit(Entity*) {};

  Entity(const Entity&) = delete;
  Entity &operator=(const Entity&) = delete;

 private:
  bool active;
  bool alive;
  const EntityModel *model;
  int mask;
  Vec2f position;
  float angle;
  Vec2f scale;
  Vec2f offset;

  // Initialize certain things that subclasses might need, like setting enemy max health, or bullet life, etc...
  // These are called by the owning EntityFactory
  virtual void activateImpl() = 0;
  virtual void deactivateImpl() = 0;

  // By default, it just deactivates...
  virtual void destroyImpl() { deactivate(); }
};

#endif
