#include "aicontroller.h"
#include "aibehavior.h"
#include "actor.h"
#include "actormodel.h"

#include "../physicsmanager.h"

const float SIT_DIST = 32.f;

AIController::AIController(Actor *owner) : ActorController(owner), state(STATE_IDLE), stateTimer(0.f), originPos(Vec2f(0,0)), target(nullptr) {}

AIController::~AIController() {}

void AIController::initiate()
{
  // Reset variables and stuff
  changeState(STATE_IDLE);
  originPos = getOwner()->getPosition();
  target = nullptr;
}

void AIController::update(float delta)
{
  PhysicsManager &physicsMgr = PhysicsManager::getInstance();
  const AIBehavior &behavior = *static_cast<const ActorModel*>(getOwner()->getModel())->getBehavior();
  AnimationState &animState = getOwner()->getAnimationState();
  ActorPhysics &physics = getOwner()->getPhysics();

  const Vec2f &pos = getOwner()->getPosition();
  
  // Handle switching between idle and patrol states.
  if (state == STATE_IDLE) {
    getOwner()->moveX(0.f);
    animState.setPlaySpeed(1.f);
    if (stateTimer > 0.f) stateTimer -= delta;
    else changeState(STATE_PATROL);
  }
  else if (state == STATE_PATROL) {
    if (stateTimer > 0.f) stateTimer -= delta;
    else changeState(STATE_IDLE);
    animState.setPlaySpeed(.8f);

    if (animState.getDirection() == Animation::DIR_RIGHT) {
      float range = originPos[0] + behavior.getPatrolState().range;
      physics.setMovement(0.5f);
      if (physics.getWallState() == ActorPhysics::WALL_RIGHT ||
	  physics.getLedgeState() == ActorPhysics::LEDGE_RIGHT ||
	  pos[0] >= range) {
	animState.setDirection(Animation::DIR_LEFT);
	getOwner()->moveX(-0.5f);
      }
    }
    else if (animState.getDirection() == Animation::DIR_LEFT) {
      float range = originPos[0] - behavior.getPatrolState().range;
      physics.setMovement(-0.5f);
      if (physics.getWallState() == ActorPhysics::WALL_LEFT ||
	  physics.getLedgeState() == ActorPhysics::LEDGE_LEFT ||
	  pos[0] <= range) {
	animState.setDirection(Animation::DIR_RIGHT);
	getOwner()->moveX(0.5f);
      }
    }
  }

  //
  // Player detection
  //
  //  I'll probably change this because I plan to implement friendly AI later, but for now only detect MASK_PLAYER
  //
  if (state == STATE_IDLE || state == STATE_PATROL) {
    physicsMgr.queryEntityGridArea(pos, 2, PhysicsManager::MASK_PLAYER, [&pos, &behavior, this](Entity *e) {
	if (e->isAlive() &&
	    PhysicsManager::boxCircleIntersection(e->getBoundingBox(), pos, behavior.getSight())) {
	  changeState(STATE_CHASE);
	  target = e;
	}
      });
  }

  if (state == STATE_CHASE) {
    originPos = getOwner()->getPosition();
    if (stateTimer > 0.f) stateTimer -= delta;
    animState.setPlaySpeed(1.f);
    if (!target->isAlive()) {
      target = nullptr;
      state = STATE_IDLE;
    }
    else {
      float attackDist = behavior.getChaseState().attackDist;
      float dist = target->getPosition()[0] - pos[0];
      
      if (dist > 0.f) animState.setDirection(Animation::DIR_RIGHT);
      else animState.setDirection(Animation::DIR_LEFT);
      
      if (fabs(dist) > attackDist)
        getOwner()->moveX( dist/fabs(dist) );
      else {
        getOwner()->moveX(0.f);

	if (stateTimer <= 0.f) {
	  getOwner()->attack(0, PhysicsManager::MASK_PLAYER);
	  stateTimer = behavior.getChaseState().attackIntervalS + drand48()*behavior.getChaseState().attackIntervalR;
	}
      }
    }
  }
}

void AIController::changeState(State next)
{
  const AIBehavior &behavior = *static_cast<const ActorModel*>(getOwner()->getModel())->getBehavior();
  AnimationState &animState = getOwner()->getAnimationState();
  state = next;

  switch (state) {
    
  case STATE_IDLE:
    stateTimer = behavior.getIdleState().timeStart + drand48()*behavior.getIdleState().timeRange;
    break;
    
  case STATE_PATROL:
    stateTimer = behavior.getPatrolState().timeStart + drand48()*behavior.getPatrolState().timeRange;

    if (getOwner()->getPosition()[0] >= originPos[0] + behavior.getPatrolState().range*.75)
      animState.setDirection(Animation::DIR_LEFT);
    else if (getOwner()->getPosition()[0] <= originPos[0] - behavior.getPatrolState().range*.75)
      animState.setDirection(Animation::DIR_RIGHT);
    else animState.setDirection(rand()%2==0?Animation::DIR_RIGHT:Animation::DIR_LEFT);
    
    break;
    
  case STATE_CHASE: {
    const SoundSet &soundSet = behavior.getChaseState().soundSet;

    if (soundSet.count() > 0) {
      soundSet.playSound(0);
    }

    stateTimer = 0.f;
    
  } break;
    
  default:
    break;
  }
}
