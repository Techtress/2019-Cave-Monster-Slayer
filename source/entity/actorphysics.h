#ifndef ACTORPHYSICS_H
#define ACTORPHYSICS_H

#include "actorphysicsmodel.h"

class Entity;

class ActorPhysics
{
 public:

  enum State
  {
    STATE_GROUND,
    STATE_AIR
  };

  enum LedgeState
  {
    LEDGE_NONE,
    LEDGE_LEFT,
    LEDGE_RIGHT,
    LEDGE_BOTH
  };

  enum WallState
  {
    WALL_NONE,
    WALL_LEFT,
    WALL_RIGHT
  };
  
  ActorPhysics(Entity* o);
  ~ActorPhysics();
  
  void activate(const ActorPhysicsModel* m);
  void update(float delta);

  State getState() const { return state; }
  State getVisibleState() const { return visibleState; }
  LedgeState getLedgeState() const { return ledgeState; }
  WallState getWallState() const { return wallState; }

  void setMovement( float factor ) { movement = factor; }
  void jump(float factor) { velocity[1] = -model->jumpStrength * factor; }

  void setVelocity(const Vec2f &v) { velocity = v; }
  void setVelocityX(float x) { velocity[0] = x; }
  void setVelocityY(float y) { velocity[1] = y; }
  const Vec2f &getVelocity() const { return velocity; }

  void addThrowForce(const Vec2f &dir, float time) { velocity = dir; disabledTime = time; }
  
  //
  // Retrieve model properties
  //
  
  const Vec2f &getDimensions() const { return model->dimensions; }
  float getHalfWidth() const { return model->halfWidth; }

  float getFallFactor() const { return model->fallFactor; }
  float getGroundSpeed() const { return model->groundSpeed; }
  float getGroundAcc() const { return model->groundAcc; }
  float getGroundDec() const { return model->groundDec; }
  float getAirSpeed() const { return model->airSpeed; }
  float getAirAcc() const { return model->airAcc; }
  float getAirDec() const { return model->airDec; }
  float getJumpStrength() const { return model->jumpStrength; }
  const std::vector<Vec2f> &getBoxPoints() const { return model->boxPts; }

  ActorPhysics() = delete;
  ActorPhysics(const ActorPhysics&) = delete;
  ActorPhysics &operator=(const ActorPhysics&) = delete;
  
 private:

  Entity *owner;
  const ActorPhysicsModel *model;

  enum SpriteFakeState {
    SPRITE_NONE,
    SPRITE_FALL,
    SPRITE_JUMP
  };
  
  State state;
  State visibleState; // Basically the same as state, but takes into account sprite fake-falling
  LedgeState ledgeState;
  WallState wallState;

  Vec2f velocity;
  float disabledTime;

  // Since we abruptly snap to the ground, we have this value to kind of soften the blow
  // by using the drawable's offset to create an illusion of a softer landing.
  float spriteOffsetVelocity;
  SpriteFakeState spriteFakeState;

  // The target horizontal movement (usually from -1 to 1). This is a factor
  // for the actor trying to move in a specific direction using either ground
  // or air acc depending on state, with this->speed as the target
  float movement;

  // The information of the current ground surface (point point line) we are standing on
  struct GroundData {
    GroundData() : c(Vec2f(0,0)), d(Vec2f(0,0)), right(Vec2f(0,0)), ledge(0) {}
    Vec2f c, d;
    Vec2f right;

    // Are we on a ledge? (the midpoint of the box is sticking past the edge of an obstacle)
    bool ledge;
  } ground;


};

#endif
