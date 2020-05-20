#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "vector2.h"
#include "entity/animation.h"

#include <list>

class EntryPoint
{
 public:
  EntryPoint(const std::string &n, const Vec2f &pos, Animation::Direction dir) :
    name(n), position(pos), direction(dir) {}

  const std::string &getName() const { return name; }
  void setName(const std::string &n) { name = n; }

  const Vec2f &getPosition() const { return position; }
  void setPosition(const Vec2f &pos) { position = pos; }

  Animation::Direction getDirection() const { return direction; }
  void setDirection(Animation::Direction dir) { direction = dir; }
  
 private:
  std::string name;
  Vec2f position;
  Animation::Direction direction;
};

class EventManager
{
 public:
  static EventManager &getInstance();

  const std::list<EntryPoint> &getEntryPoints() const { return entryPoints; }
  const std::list<EntryPoint> &getEnemySpawnList() const { return enemySpawns; }

  void createEntryPoint(const std::string &name, const Vec2f &pos, Animation::Direction dir) {
    entryPoints.emplace_back(name, pos, dir);
  }
  
  bool hasEntryPoint(const std::string &name) const {
    for (const EntryPoint &e : entryPoints)
      if (e.getName() == name) return false;
    return true;
  }

  EntryPoint &getEntryPoint(const std::string &name) {
    for (EntryPoint &e : entryPoints)
      if (e.getName() == name) return e;
    throw std::string("Entry point '" + name + "' not found!");
  }

  void removeEntryPoint(const std::string &name) {
    for (auto it = entryPoints.begin(); it != entryPoints.end(); it++)
      if (it->getName() == name) { entryPoints.erase(it); return; }
  }
  
  void clear() { entryPoints.clear(); enemySpawns.clear(); }

  EntryPoint &createEnemySpawn(const std::string &model, const Vec2f &pos) {
    enemySpawns.emplace_back(model, pos, Animation::DIR_RIGHT);
    return enemySpawns.back();
  }

  // Need for level editor. Never use elsewhere...
  std::list<EntryPoint> &editor_getEnemySpawnList() {
    return enemySpawns;
  }

 private:
  EventManager();
  std::list<EntryPoint> entryPoints;//wtf is this not a map????
  std::list<EntryPoint> enemySpawns; // name = actor model
};

#endif
