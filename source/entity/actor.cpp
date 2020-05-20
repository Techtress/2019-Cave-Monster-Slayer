#include "actor.h"
#include "actormodel.h"
#include "playercontroller.h"

#include "../physicsmanager.h"
#include "../image.h"

std::map<int,int> Actor::maskCounts = std::map<int,int>();

const std::vector<std::string> ANIM_MAP =
  { "idle", // ANIM_IDLE
    "walk", // ANIM_WALK
    "run",  // ANIM_RUN
    "jump", // ANIM_JUMP
    "fall" // ANIM_FALL
  };

Actor::Actor() :
  animState(),
  physics(this),
  explosion(),
  controller(nullptr),
  attributes(),
  aiController(this),
  aiControlled(false),
  currentAnim(ANIM_IDLE),
  actionState(ACTION_NORMAL),
  pauseTimer(0.f),
  attackId(-1),
  attackMask(0),
  god(false)
{}

Actor::~Actor()
{
}

void Actor::setPlayerControlled(PlayerController* controller)
{
  setController(controller);
  controller->setControlledActor(this);
}

void Actor::setAIControlled()
{
  setController(&aiController);
}

void Actor::draw(float scrollFactor) const
{
  if (!isAlive()) {
    explosion.draw(scrollFactor);
    return;
  }
  
  auto anim = animState.getDrawData();
  anim.first->draw( getPosition()[0], getPosition()[1], anim.second, scrollFactor);
}

void Actor::update(float delta)
{
  if (isAlive() && attributes.health <= 0.f) destroy();
  
  if (!isAlive()) {
    if (!explosion.isAlive())
      deactivate();
    else explosion.update(delta);
    return;
  }
  
  if (controller != nullptr) controller->update(delta);

  if (actionState == ACTION_NORMAL) {
    // Handle air-related animations
    if (physics.getVisibleState() == ActorPhysics::STATE_AIR) {
      // If we are no longer accelerating upwards, play the falling animation
      if (currentAnim != ANIM_JUMP || (physics.getVelocity()[1] > 0.f && currentAnim == ANIM_JUMP))
	changeAnimState(ANIM_FALL);
    }
  
    // Handle ground-related animations
    else if (physics.getVisibleState() == ActorPhysics::STATE_GROUND) {
      if (currentAnim == ANIM_FALL) {
	const SoundSet &landSoundSet = animState.getAnimSet()->getAnimation(ANIM_MAP[ANIM_RUN]).getSoundSet();
	if (!landSoundSet.empty()) landSoundSet.playRandomSound();
      }

      if (physics.getVelocity()[1] >= 0.f) {
	if (fabs(physics.getVelocity()[0]) > 100.f)
	  changeAnimState( ANIM_RUN );
	else
	  changeAnimState( ANIM_IDLE );
      }
    }
  }
  else if (actionState == ACTION_ATTACK) {
    if (animState.hasFinished())
      actionState = ACTION_NORMAL;

    if (attackId >= 0) {
      const ActorModel &model = *static_cast<const ActorModel*>(getModel());
      if (animState.getCurrentTime() > model.getAttack(attackId).hitDelay) {
	HitBoxFactory::getInstance().spawnHitBox( &model.getAttack(attackId).hitBox, this, attackMask, getPosition(), animState.getDirection() );
	attackId = -1;
	attackMask = 0;
      }
    }
  }

  if (pauseTimer <= 0.f)
    animState.update(delta);
  else
    pauseTimer -= delta;
  
  physics.update(delta);
}

void Actor::attack(int id, int mask)
{
  // Please do nothing if dead...
  if (!isAlive() || actionState != ACTION_NORMAL) return;

  const ActorModel &model = *static_cast<const ActorModel*>(getModel());

  actionState = ACTION_ATTACK;
  attackId = id;
  attackMask = mask;
  changeAnimState(ANIM_SPECIAL);
  animState.playAnimation( model.getAttack(id).animation ); // TO-DO: change specific to attack
  physics.setMovement(0.f);
}

void Actor::jump(float factor)
{
  if (actionState != ACTION_NORMAL) return;
  physics.jump(factor);
  changeAnimState(ANIM_JUMP);
}

void Actor::moveX(float factor)
{
  if (actionState != ACTION_NORMAL) return;
  physics.setMovement( std::min( 1.f, std::max( -1.f, factor ) ) );

  if (factor > 0.01)
    animState.setDirection( Animation::DIR_RIGHT );
  else if (factor < -0.01)
    animState.setDirection( Animation::DIR_LEFT );
}

float Actor::getHealthPercent() const
{
  return std::max(attributes.health/static_cast<const ActorModel*>(getModel())->getAttributes().health,0.f);
}

void Actor::activateImpl()
{
  const ActorModel &model = *static_cast<const ActorModel*>(getModel());

  // Start off by playing the idle animation
  animState.activate(&model.getAnimSet(), ANIM_MAP[ANIM_IDLE]);

  // Set physics properties and reset velocity, etc...
  physics.activate(&model.getPhysics());

  // Based on model, reset health, etc...
  attributes = model.getAttributes();
  
  // Set variables
  currentAnim = ANIM_IDLE;
  actionState = ACTION_NORMAL;

  god = false;

  maskCounts[getMask()]++;
}

BoundingBox Actor::getBoundingBox() const
{
  const ActorPhysicsModel &physicsModel = (*static_cast<const ActorModel*>(getModel())).getPhysics();
  return BoundingBox( getPosition()[0] - physicsModel.getHalfWidth(),
		      getPosition()[1] - physicsModel.getHeight(),
		      getPosition()[0] + physicsModel.getHalfWidth(),
		      getPosition()[1] );
}

void Actor::registerEntityCollision( Entity* other )
{
  //if (getMask()&PhysicsManager::MASK_PLAYER && other->getMask()&PhysicsManager::MASK_ENEMY)
  //  destroy();
  if (dynamic_cast<Actor*>(other)) {
    float dist = other->getPosition()[0] - getPosition()[0];
    float bad = 100.f;
    float amt = (bad*2-fabs(dist))*5.f;
    if (fabs(dist) < bad) {
      if (dist < 0 && physics.getVelocity()[0] < 0)
	physics.setVelocityX(amt);
      else if (dist > 0 && physics.getVelocity()[0] > 0)
	physics.setVelocityX(-amt);
    }
  }
}

void Actor::registerHitBoxCollision( const HitBox *hb )
{
  if (!god)
    attributes.health -= hb->getModel()->getDamage();

  float dir = getPosition()[0] - hb->getPosition()[0];
  physics.addThrowForce((dir/fabs(dir))* hb->getModel()->getImpact(), .1f );

  if (attributes.super)
    if (pauseTimer <= 0.f) pauseTimer = .1f;
  // else do flinch animation and disable movement and stuff
}

void Actor::deactivateImpl()
{
  setController(nullptr);
  maskCounts[getMask()]--;
}

void Actor::destroyImpl()
{
  explosion.spawn(animState.getDrawData().first, animState.getDrawData().second, getPosition(), physics.getVelocity()*.5f);
  static_cast<const ActorModel*>(getModel())->playDeathSound();
}

void Actor::changeAnimState( ActorAnim newState )
{
  if (currentAnim == newState || (currentAnim = newState) == ANIM_SPECIAL) return;
  animState.playAnimation( ANIM_MAP[currentAnim] );
}

void Actor::setController( ActorController *newController )
{
  if ((controller = newController) != nullptr)
    controller->initiate();
}
