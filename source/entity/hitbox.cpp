#include "hitbox.h"

#include "../xmltag.h"
#include "../gamemanager.h"
#include "../physicsmanager.h"
#include "../stringutil.h"
#include "../entity.h"

#include <iostream>


// Debug stuff...
#include "../rendercontext.h"
#include "../viewport.h"
#include <SDL.h>

HitBoxModel::HitBoxModel(const XMLTag& tag) :
  radius(tag["radius"].toFloat()),
  life(tag["life"].toFloat()),
  offset(tag.toVec2f()),
  soundSet(),

  //Game logic
  damage(tag["damage"].toFloat()),
  impact(tag["impact"].toFloat())
{
  if (tag.hasChild("soundSet"))
    soundSet = tag["soundSet"];
}

HitBoxModel::~HitBoxModel()
{
  
}

HitBox::HitBox() : model(nullptr), owner(nullptr), mask(0), life(0), position(), direction(Animation::DIR_RIGHT), hitList(), soundQueue(0), soundTimer(-1.f) {}
HitBox::~HitBox() {}

void HitBox::activate(const HitBoxModel* m, Entity *o, int hitMask, const Vec2f &pos, Animation::Direction dir)
{
  model = m;
  owner = o;
  mask = hitMask;
  life = m->getLife();
  position = pos;
  direction = dir;
  soundTimer = -1.f;
  soundQueue = 0.f;
}

void HitBox::update(float delta)
{
  if (owner != nullptr) {
    position = owner->getPosition();
  } else if (!owner->isAlive())
    owner = nullptr;

  Vec2f realPos = position + Vec2f(model->getOffset()[0] * (direction == Animation::DIR_RIGHT ? 1.f : -1.f), model->getOffset()[1]);

  if (life > 0.f) {
    PhysicsManager &physics = PhysicsManager::getInstance();
    physics.queryEntityGridArea( realPos, 1, mask, [&realPos, this](Entity *e) {
	if ( std::find(hitList.begin(), hitList.end(), e) == hitList.end() &&
	     PhysicsManager::boxCircleIntersection( e->getBoundingBox(), realPos, model->getRadius() ) ) {
	  e->registerHitBoxCollision(this);
	  hitList.push_back(e);

	  // Playe the lovely sound effect tied to this hitbox
	  //model->playSound();
	  if (soundQueue == 0) soundTimer = .1f;
	  soundQueue++;
	}
      });
  }

  if (soundTimer > 0.f)
    soundTimer -= delta;
  else if (soundQueue > 0) {
    model->playSound();
    soundQueue--;
    soundTimer = 0.03f;
  }

  life -= delta;

  if (life <= 0)
    hitList.clear();
}

HitBoxFactory &HitBoxFactory::getInstance()
{
  static HitBoxFactory instance;
  return instance;
}

HitBoxFactory::HitBoxFactory() : freeList(), activeList() {}

HitBoxFactory::~HitBoxFactory()
{
  for (HitBox *hb : freeList) delete hb;
  for (HitBox *hb : activeList) delete hb;
}

void HitBoxFactory::spawnHitBox(const HitBoxModel* model, Entity *owner, int hitMask, const Vec2f &pos, Animation::Direction dir)
{
  HitBox *hb;
  
  if (freeList.empty())
    hb = new HitBox();
  else {
    hb = freeList.front();
    freeList.pop_front();
  }

  hb->activate(model, owner, hitMask, pos, dir);
  activeList.push_front(hb);
}

void HitBoxFactory::updateActiveList(float delta)
{
  for (auto it = activeList.begin(); it != activeList.end();) {
    if ((*it)->alive()) {
      (*it++)->update(delta);
    }
    else {
      freeList.push_front(*it);
      it = activeList.erase(it);
    }
  }

  // DEBUG STUFF
  GameManager::getInstance().setDebugMessage(4, "HitBox Pool: " + StringUtil::toString(activeList.size()) + " / " + StringUtil::toString(freeList.size()) );
}

void HitBoxFactory::debugDraw() const
{
  SDL_Renderer *r = RenderContext::getInstance().getRenderer();
  SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
  for (const HitBox *hb : activeList) {
    Vec2f screenPos = Viewport::getInstance().getScreenPos(hb->getPosition());
    screenPos += Vec2f( hb->getModel()->getOffset() * Vec2f(hb->getDirection() == Animation::DIR_RIGHT ? 1 : -1,1) );
    float radius = hb->getModel()->getRadius();

    float amt = M_PI/16.f;
    for (float i = 0; i < M_PI*2; i += amt) {
      int x = screenPos[0] + cos(i)*radius,
	y = screenPos[1] + sin(i)*radius;
      int x2 = screenPos[0] + cos(i+amt)*radius,
	y2 = screenPos[1] + sin(i+amt)*radius;
      SDL_RenderDrawLine(r, x, y, x2, y2);
    }
  }
}
