#include "entityfactory.h"
#include "entity.h"
#include "entitymodel.h"
#include "xmlparser.h"

#include <iostream>

EntityFactory::EntityFactory(const std::string &tname, bool d) : type(tname), freeList(), activeList(), dynamic(d), models()
{}

EntityFactory::~EntityFactory() {
  for (Entity *e : freeList) delete e;
  for (Entity *e : activeList) delete e;
  for (auto &it : models) delete it.second;
}

Entity *EntityFactory::activateEntity(const std::string &modelName, int mask, const Vec2f &position, float angle, const Vec2f &scale)
{
  if (freeList.size() == 0) {
    if (dynamic) pushFreeList();
    else throw std::string("EntityFactory '" + type + "' is full and of static type!. Cannot resize pool");
  }

  // Transfer entity to the active list
  Entity *e = *freeList.begin();
  freeList.erase(freeList.begin());
  activeList.push_front(e);

  // Do the things specific to the subtype
  e->activate(getModel(modelName), mask, position, angle, scale);

  return e;
}

void EntityFactory::deactivateAll()
{
  while (!activeList.empty()) {
    activeList.front()->deactivate();
    freeList.push_front(activeList.front());
    activeList.pop_front();
  }
}

void EntityFactory::updateActiveList(float delta)
{
  for (Entity *e : activeList) e->update(delta);
  for (auto it = activeList.begin(); it != activeList.end();) {
    if (!(*it)->isActive()) {
      freeList.push_front(*it);
      it = activeList.erase(it);
    } else ++it;
  }
}

const EntityModel *EntityFactory::getModel(const std::string &modelName)
{
  auto result = models.find(modelName);
  return result == models.end() ? loadModelXML(modelName) : result->second;
}

EntityModel *EntityFactory::loadModelXML(const std::string &modelName)
{
  XMLParser parser("assets/" + type + "/" + modelName + ".xml");

  // Inside the XML file, we simply look for the first "root tag" that is
  // the type of the model. (i.e. <enemy> or <backdrop>)
  return models[modelName] = createModel(parser.getTag(type));
}
