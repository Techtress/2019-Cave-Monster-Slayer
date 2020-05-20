#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#include "debughud.h"
#include "vector2.h"

#include "entity/animation.h"

class Entity;
class EntityFactory;
class Actor;
class Canvas;
class PhysicsManager;

class GameManager
{
 public:
  ~GameManager();
  static GameManager &getInstance();

  enum ObjectType
  {
    TYPE_ACTOR
  };

  void update(float delta);
  void draw() const;

  void clearScene();
  void loadScene(const std::string&);
  void toggleDebugHUD() { debugHUD.toggle(); }

  void setDebugMessage(int lineID, const std::string& msg) { debugHUD.setMessage(lineID, msg); }

  Actor *spawnPlayer(const std::string &model, const std::string &entry);
  Actor *spawnEnemy(const std::string &model, const Vec2f &pos, Animation::Direction);
  Actor *spawnActor(const std::string &model, int mask, const Vec2f &pos, Animation::Direction);
  void despawnActor(Actor*);

  // Based on enemy spawns in the event manager
  void spawnAllEnemies();

  void despawnAllEntities();

  GameManager(const GameManager&) = delete;
  GameManager &operator=(const GameManager&) = delete;

 private:
  GameManager();

  std::map<ObjectType, EntityFactory*> entityFactories;
  Canvas &canvas;
  PhysicsManager &physics;

  DebugHUD debugHUD;

  Entity *spawnEntity( ObjectType t, const std::string &model, int mask, int layer, const Vec2f &position, float angle, const Vec2f &scale );
};

#endif
