#include "testingstate.h"
#include "gamemanager.h"
#include "viewport.h"
#include "appstatemanager.h"
#include "physicsmanager.h"

#include "entity/actor.h"
#include "entity/actorphysics.h"

#include <SDL.h>

TestingState::TestingState( AppState* rs ) :
  returnState(rs), gamemgr( GameManager::getInstance() ), playerController(), playerActor(nullptr)
{
}

TestingState::~TestingState()
{
}

void TestingState::enter()
{
  (playerActor = gamemgr.spawnActor("player", PhysicsManager::MASK_PLAYER,
				    Viewport::getInstance().getMouseWorldPos(),
				    Animation::DIR_RIGHT))->setPlayerControlled(&playerController);
  gamemgr.spawnAllEnemies();
}

void TestingState::exit()
{
  gamemgr.despawnAllEntities();
  playerActor = nullptr;
}
  
void TestingState::input(const SDL_Event& event)
{
  //Viewport &v = Viewport::getInstance();

  if (playerActor == nullptr) return;
  if (event.type == SDL_KEYDOWN) {
    switch (event.key.keysym.scancode) {
    case SDL_SCANCODE_SPACE: playerController.inputJumping(true); break;
    case SDL_SCANCODE_A:     playerController.inputLeft(true);    break;
    case SDL_SCANCODE_D:     playerController.inputRight(true);   break;
    case SDL_SCANCODE_F1:    gamemgr.toggleDebugHUD(); break;
    case SDL_SCANCODE_F2:   AppStateManager::getInstance().changeState(returnState); break;
    default: break;
    }
  }
  else if (event.type == SDL_KEYUP) {
    switch (event.key.keysym.scancode) {
    case SDL_SCANCODE_SPACE: playerController.inputJumping(false); break;
    case SDL_SCANCODE_A:     playerController.inputLeft(false);    break;
    case SDL_SCANCODE_D:     playerController.inputRight(false);   break;
    default: break;
    }
  }
}

void TestingState::update(float delta)
{
  gamemgr.update(delta);
  if (playerActor == nullptr) return;
  Viewport::getInstance().setTarget( playerActor->getPosition() - playerActor->getOffset()
				     + Vec2f(0, -250 + std::max(playerActor->getPhysics().getVelocity()[1]*.04f, 0.f)) );
}

void TestingState::draw() const
{
  gamemgr.draw();
}
