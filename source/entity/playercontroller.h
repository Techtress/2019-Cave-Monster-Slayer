#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "actorcontroller.h"

class PlayerController : public ActorController
{
 public:
  PlayerController();
  virtual ~PlayerController();
  
  void inputRight(bool);
  void inputLeft(bool);
  void inputMoveAxis(float);
  void inputJumping(bool);
  void inputAttack();

  // Called by actor when calling setPlayerControlled
  void setControlledActor(Actor *o) { setOwner(o); }

  virtual void initiate() override;
  virtual void update(float delta) override;

  PlayerController(const PlayerController&);
  PlayerController &operator=(const PlayerController&);
  
 private:
  enum JumpState
  {
    JUMP_NONE,
    JUMP_BEGIN,
    JUMP_SHORTHOP,
    JUMP_END
  } jumpState;

  bool jumping;
  float hopTimer;
  float landTimer; // used to determine whether it is too soon to jump or not while holding the jump button
  float jumpExceptionTimer;

  float movementRight;
  float movementLeft;
  float movementAxis;
};

#endif
