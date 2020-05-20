#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include <list>
#include <unordered_map>
#include <algorithm>

#include "vector2.h"

class XMLTag;

class Entity;
class EntityModel;

class EntityFactory
{
 public:
  EntityFactory(const std::string &tname, bool dynamically_resize);
  virtual ~EntityFactory();

  // Takes an entity out of the free list and puts them into the active list
  Entity *activateEntity(const std::string &modelName, int mask, const Vec2f &position, float angle, const Vec2f &scale);
  void deactivateAll();

  void updateActiveList(float delta);

  int getActiveCount() const { return activeList.size(); }
  int getFreeCount() const { return freeList.size(); }

  // Returns the name for the type of entity the factory is handling. This is mostly
  // used for finding the right directory to load an xml file
  const std::string &getType() const { return type; }

  // Attempts to load a model data file (right now it's XML)
  const EntityModel *getModel(const std::string &modelName);

  EntityFactory(const EntityFactory&) = delete;
  EntityFactory &operator=(const EntityFactory&) = delete;
  
 private:
  std::string type;
  std::list<Entity*> freeList, activeList;
  bool dynamic;
  std::unordered_map<std::string, EntityModel*> models;

  EntityModel *loadModelXML(const std::string &name);
  virtual EntityModel *createModel(const XMLTag&) = 0;
  virtual Entity *createInstance() = 0;

 protected:
  void pushFreeList() { freeList.push_front(createInstance()); }
};

// Just an extra easy implementation step...
template<class T, class M>
class EntityFactoryWrapper : public EntityFactory
{
 public:
  EntityFactoryWrapper(const std::string &tname, unsigned pool, bool resizeable) : EntityFactory(tname, resizeable)
  {
    while (pool-- > 0) pushFreeList();
  }
  virtual ~EntityFactoryWrapper() {}

  EntityFactoryWrapper(const EntityFactoryWrapper&) = delete;
  EntityFactoryWrapper &operator=(const EntityFactoryWrapper&) = delete;
  
 private:
  virtual EntityModel *createModel(const XMLTag& tag) override { return new M(tag); }
  virtual Entity *createInstance() override { return new T(); }
};

#endif
