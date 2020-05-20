#include "playercontroller.h"
#include "actor.h"
#include "actorphysics.h"
#include "animationstate.h"

#include "../physicsmanager.h"

const float SECOND_JUMP = -600.f;
const float HOP_TIME = 0.084f;
const float LAND_TIME = 0.15f;
const float JUMP_ALLOWANCE = 0.034f;

const float THRESHOLD_WALK = 0.1f;
const float THRESHOLD_RUN = 0.7f;

PlayerController::PlayerController() :
  ActorController(nullptr),
  
  jumpState(JUMP_NONE),
  
  jumping(false),
  hopTimer(0.f),
  landTimer(0.f),
  jumpExceptionTimer(0.f),

  movementRight(0.f),
  movementLeft(0.f),
  movementAxis(0.f)
{
}

PlayerController::~PlayerController()
{
}

void PlayerController::initiate()
{
  jumpState = JUMP_NONE;
  jumping = false;
  hopTimer = 0.f;
  landTimer = 0.f;
  jumpExceptionTimer = 0.f;

  movementRight = 0.f;
  movementLeft = 0.f;
  movementAxis = 0.f;
}

void PlayerController::inputRight(bool state)
{
  movementRight = state ? 1.f : 0.f;
}

void PlayerController::inputLeft(bool state)
{
  movementLeft = state ? 1.f : 0.f;
}

void PlayerController::inputMoveAxis(float factor)
{
  (void)factor;
  /*ActorPhysics &physics = getPhysics();
  float amount = fabs(factor);
  float dir = factor/amount;
  
  if (amount > THRESHOLD_WALK && amount <= THRESHOLD_RUN)
    movementAxis = physics.getVisibleState() == ActorPhysics::STATE_AIR ?
      std::max(0.5f, static_cast<float>(fabs(movementAxis))) * dir : 0.4f * dir;
  else if (amount > THRESHOLD_RUN)
    movementAxis = dir;
  else
    movementAxis = 0.f;
  */
}

void PlayerController::inputJumping(bool j)
{
  jumping = j;
}

void PlayerController::inputAttack()
{
  Actor *actor = getOwner();
  //ActorPhysics &physics = actor->getPhysics();

  // For now, only attack on the ground and with the first basic attack
  //if (physics.getVisibleState() == ActorPhysics::STATE_GROUND)
  actor->attack(0, PhysicsManager::MASK_ENEMY);
}

void PlayerController::update(float delta)
{
  Actor *actor = getOwner();
  ActorPhysics &physics = actor->getPhysics();
  //AnimationState &animState = actor->getAnimationState();

  // Translate input (i.e. left/right kwys) into physics velocity
  actor->moveX(movementAxis + movementRight - movementLeft);
  
  if (physics.getVisibleState() == ActorPhysics::STATE_AIR) {
    // Something unique to player: just hop if we tap the button, but full jump if we hold the button
    if (jumpState != JUMP_END && jumpState != JUMP_NONE) {
      if (jumpState == JUMP_BEGIN && !jumping)
	jumpState = JUMP_SHORTHOP;
      
      hopTimer -= delta;
      if (hopTimer <= 0.f) {
	if (jumpState == JUMP_SHORTHOP && physics.getVelocity()[1] < SECOND_JUMP)
	  physics.setVelocityY( SECOND_JUMP );
	jumpState = JUMP_END;
      }
    }

    if (!jumping)
      landTimer = LAND_TIME;
    else
      landTimer -= delta;

    if (landTimer <= 0.f && hopTimer <= 0.f)
      jumpState = JUMP_END;

    jumpExceptionTimer -= delta;
  }
  // Handle ground-related input
  else if (physics.getVisibleState() == ActorPhysics::STATE_GROUND) {
    jumpExceptionTimer = JUMP_ALLOWANCE;
    if (!jumping || landTimer > 0.f) jumpState = JUMP_NONE;
  }

  // Handle special jumping conditions
  if (physics.getVisibleState() == ActorPhysics::STATE_GROUND
      || (physics.getVisibleState() == ActorPhysics::STATE_AIR && jumpExceptionTimer > 0.f) ) {
    if (jumping && jumpState == JUMP_NONE) {
      if (actor->getAnimationState().getCurrentAnimName() == "jump")
	actor->getAnimationState().playAnimation("jump");
      actor->jump(1.f);
      jumpState = JUMP_BEGIN;
      hopTimer = HOP_TIME;// - delta;
      landTimer = -1.f;
    }
  }
}
