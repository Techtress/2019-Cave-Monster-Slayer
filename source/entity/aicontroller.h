#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include "actorcontroller.h"
#include "../vector2.h"

class AIBehavior;
class Entity;

class AIController : public ActorController
{
 public:
  AIController(Actor*);
  virtual ~AIController();
  
  virtual void initiate() override;
  virtual void update(float delta) override;

  enum State
  {
    STATE_IDLE,
    STATE_PATROL,
    STATE_CHASE
  };

  AIController(const AIController&) = delete;
  AIController &operator=(const AIController&) = delete;

 private:
  State state;
  float stateTimer;
  Vec2f originPos;
  Entity *target;
  
  void changeState(State);
};

#endif
