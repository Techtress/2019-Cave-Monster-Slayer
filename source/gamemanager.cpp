#include "gamemanager.h"

#include "entity.h"
#include "entityfactory.h"
#include "eventmanager.h"
#include "physicsmanager.h"
#include "canvas.h"
#include "image.h"
#include "lightmanager.h"
#include "imagefactory.h"
#include "gameconfig.h"
#include "clock.h"
#include "viewport.h"
#include "stringutil.h"

#include "entity/actor.h"
#include "entity/actormodel.h"
#include "entity/hitbox.h" // soon to be just factory

#include <cmath>
#include <unordered_map>
#include <iostream>

typedef EntityFactoryWrapper<Actor, ActorModel> ActorFactory;

//#include "backdropfactory.h"

GameManager &GameManager::getInstance()
{
  static GameManager instance;
  return instance;
}

GameManager::GameManager() : entityFactories(), canvas(Canvas::getInstance()), physics(PhysicsManager::getInstance()), debugHUD()
{
  // Only 1 player right now
  entityFactories[TYPE_ACTOR] = new ActorFactory("actor", 0, true);
}

GameManager::~GameManager()
{
  for (auto &it : entityFactories) delete it.second;
}

void GameManager::update(float delta)
{
  //
  // Update all entities
  //
  std::for_each(entityFactories.begin(), entityFactories.end(), [delta](auto &it) {
      it.second->updateActiveList(delta); });
  physics.updateEntityList();
  canvas.updateEntityList();

  physics.testEntityCollisions();
  HitBoxFactory::getInstance().updateActiveList(delta);
  
  //
  // Update debug info
  //
  debugHUD.setMessage(0, "FPS: " + StringUtil::toString(static_cast<int>(Clock::getInstance().getFPS()+0.5f)));
  debugHUD.setMessage(2, "Actor Pool: " + StringUtil::toString(entityFactories[TYPE_ACTOR]->getActiveCount()) + " / "
		      + StringUtil::toString(entityFactories[TYPE_ACTOR]->getFreeCount()));

  GameConfig &cfg = GameConfig::getInstance();
  int offset = 6;
  std::for_each(cfg["directions"].getChildren().begin(), cfg["directions"].getChildren().end(), [this, &offset](const XMLTag *t) {
      debugHUD.setMessage(offset++, t->toStr()); });

  //debugHUD.setMessage(2, "backdrop pool: " + StringUtil::toString(backdropFactory.getActiveCount()) + "/" +
  //		      StringUtil::toString(backdropFactory.getFreeCount()));


  //
  // Update lights
  //
  LightManager::getInstance().update();
}

void GameManager::draw() const
{
  canvas.draw();
  LightManager::getInstance().draw();
  //HitBoxFactory::getInstance().debugDraw();
  debugHUD.draw();
}

void GameManager::clearScene()
{
  physics.clearWorld();
  canvas.clear();
  despawnAllEntities();
  EventManager::getInstance().clear();
}

void GameManager::loadScene(const std::string &name)
{
  clearScene();
  XMLParser parser("assets/scenes/" + name + ".xml");
  const XMLTag &scene = parser.getTag("scene");
  EventManager &eventmgr = EventManager::getInstance();

  // Define world size
  physics.resizeWorld( scene["width"].toInt(), scene["height"].toInt() );

  // Find entry points
  for (const XMLTag *pe : scene.getChildren()) {
    const XMLTag &e = *pe;
    if (e.getName() != "entry") continue;
    Animation::Direction dir = Animation::DIR_RIGHT;
    if (e["dir"].toStr() == "left") dir = Animation::DIR_LEFT;
    eventmgr.createEntryPoint(e["name"].toStr(), e.toVec2f(), dir);
  }

  // Find Enemy Spawns
  if (scene.hasChild("enemies")) {
    for (const XMLTag *pe : scene["enemies"].getChildren()) {
      const XMLTag &e = *pe;
      // TO-DO: add direction
      eventmgr.createEnemySpawn(e.getName(), e.toVec2f());
    }
  }

  // Load collision segments
  const XMLTag &col = scene["collision"];
  for (const XMLTag *ps : col.getChildren()) {
    const XMLTag &s = *ps;
    physics.addWorldSegment( Vec2f(s["ax"].toFloat(), s["ay"].toFloat()),
			     Vec2f(s["bx"].toFloat(), s["by"].toFloat()) );
  }

  // Load the objects
  const XMLTag &c = scene["canvas"];
  for (const XMLTag *pl : c.getChildren()) {
    const XMLTag &l = *pl;
    int currentLayer = l["id"].toInt();

    canvas.setScrollFactor( currentLayer, l.hasChild("scroll") ? l["scroll"].toFloat() : 1.f );
    canvas.setBackground( currentLayer, l.hasChild("background") ? ImageFactory::getInstance().getImage(l["background"].toStr()) : nullptr );
    canvas.setBackgroundAlpha( currentLayer, l.hasChild("bgAlpha") ? l["bgAlpha"].toFloat() : 1.f );

    std::map<int, const Image*> bdsetlist;

    for (const XMLTag *po : l.getChildren()) {
      const XMLTag &o = *po;

      if (o.getName() == "backdropSet")
	bdsetlist[o["id"].toInt()] = ImageFactory::getInstance().getImage( o["image"].toStr() );
      
      else if (o.getName() == "bd")
	canvas.placeBackdrop( currentLayer,
			      bdsetlist[o["set"].toInt()],
			      o.toVec2f(), // ["x"] and ["y"]
			      o["frame"].toInt(),
			      o.hasChild("angle") ? o["angle"].toFloat() : 0.f,
			      o.hasChild("scaleX") ? o["scaleX"].toFloat() : 1.f,
			      o.hasChild("scaleY") ? o["scaleY"].toFloat() : 1.f,
			      o.hasChild("flipH") ? o["flipH"].toBool() : false,
			      o.hasChild("flipV") ? o["flipV"].toBool() : false );
    }
  }
}

Actor *GameManager::spawnActor(const std::string& model, int mask, const Vec2f &pos, Animation::Direction dir)
{
  Actor *p = static_cast<Actor*>(spawnEntity( TYPE_ACTOR, model, mask, Canvas::LAYER_MAIN, pos, 0.f, Vec2f(0,0) ));
  p->getAnimationState().setDirection(dir);
  return p;
}

Actor *GameManager::spawnPlayer(const std::string &model, const std::string &entry)
{
  const EntryPoint &e = EventManager::getInstance().getEntryPoint(entry);
  return spawnActor(model, PhysicsManager::MASK_PLAYER, e.getPosition(), e.getDirection());
}

Actor *GameManager::spawnEnemy(const std::string& model, const Vec2f &pos, Animation::Direction dir)
{
  Actor *a = spawnActor( model, PhysicsManager::MASK_ENEMY, pos, dir );
  a->setAIControlled();
  return a;
}

void GameManager::despawnActor(Actor* p)
{
  p->deactivate();
  //canvas.unregisterEntity(p);
  //physics.unregisterEntity(p);
}

Entity *GameManager::spawnEntity( ObjectType t, const std::string &model, int mask, int layer,
				  const Vec2f &position, float angle, const Vec2f &scale )
{
  Entity *e = entityFactories[t]->activateEntity(model, mask, position, angle, scale);
  canvas.registerEntity(layer, e);
  physics.registerEntity(e);
  return e;
}

void GameManager::spawnAllEnemies()
{
  for (const EntryPoint &e : EventManager::getInstance().getEnemySpawnList()) {
    spawnEnemy(e.getName(), e.getPosition(), e.getDirection());
  }
}

void GameManager::despawnAllEntities()
{
  std::for_each(entityFactories.begin(), entityFactories.end(), [](auto &it) {
      it.second->deactivateAll(); });
}
