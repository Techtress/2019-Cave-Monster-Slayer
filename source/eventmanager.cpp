#include "eventmanager.h"

EventManager &EventManager::getInstance()
{
  static EventManager instance;
  return instance;
}

EventManager::EventManager() : entryPoints(), enemySpawns() {}
