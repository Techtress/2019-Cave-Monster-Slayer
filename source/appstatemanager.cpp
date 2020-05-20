#include "appstatemanager.h"
#include "appstate.h"

AppStateManager &AppStateManager::getInstance()
{
  static AppStateManager instance;
  return instance;
}

AppStateManager::~AppStateManager() {}

void AppStateManager::changeState(AppState* newState)
{
  if (currentState != nullptr)
    currentState->exit();
  currentState = newState;
  currentState->enter();
}

void AppStateManager::stateInput(const SDL_Event& event)
{
  currentState->input(event);
}

void AppStateManager::stateUpdate(float delta)
{
  currentState->update(delta);
}

void AppStateManager::stateDraw() const
{
  currentState->draw();
}

void AppStateManager::stateEnd()
{
  if (currentState != nullptr)
    currentState->exit();
}
