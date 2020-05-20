#include "actorphysicsmodel.h"

#include "../xmltag.h"

ActorPhysicsModel::ActorPhysicsModel(const XMLTag& tag) :
  dimensions( Vec2f(tag["width"].toFloat(), tag["height"].toFloat()) ),
  halfWidth(dimensions[0]*.5f),
  fallFactor(tag["fallFactor"].toFloat()),
  groundSpeed(tag["groundSpeed"].toFloat()),
  groundAcc(tag["groundAcc"].toFloat()),
  groundDec(tag["groundDec"].toFloat()),
  airSpeed(tag["airSpeed"].toFloat()),
  airAcc(tag["airAcc"].toFloat()),
  airDec(tag["airDec"].toFloat()),
  jumpStrength(tag["jumpStrength"].toFloat()),
  boxPts( { Vec2f(-halfWidth, -dimensions[1]),
	Vec2f(-halfWidth, 0),
	Vec2f(halfWidth, 0),
	Vec2f(halfWidth, -dimensions[1]) } )
{}
