#ifndef ACTORPHYSICSMODEL_H
#define ACTORPHYSICSMODEL_H

#include "../vector2.h"

#include <vector>

class XMLTag;

class ActorPhysicsModel
{
 public:
  ActorPhysicsModel(const XMLTag&);

  float getHeight() const { return dimensions[1]; }
  float getHalfWidth() const { return halfWidth; }

  ActorPhysicsModel() = delete;
  ActorPhysicsModel(const ActorPhysicsModel&) = delete;
  ActorPhysicsModel &operator=(const ActorPhysicsModel&) = delete;

 private:
  friend class ActorPhysics;
  
  const Vec2f dimensions;
  const float halfWidth; // Calculated with dimensions

  const float fallFactor; // 1.0 = normal gravity
  const float groundSpeed;
  const float groundAcc;
  const float groundDec;
  const float airSpeed;
  const float airAcc;
  const float airDec;
  const float jumpStrength;
  const std::vector<Vec2f> boxPts;
};

#endif
