#include "gamestate.h"
#include "gamemanager.h"
#include "viewport.h"
#include "lightmanager.h"
#include "rendercontext.h"
#include "physicsmanager.h"
#include "imagefactory.h"
#include "image.h"
#include "stringutil.h"
#include "iomod.h"

#include "entity/actor.h"
#include "entity/actorphysics.h"

#include <SDL.h>

const float DEATH_TIME = .333f;
const float AMBIENCE = 0.5f;

GameState::GameState() :
  gamemgr( GameManager::getInstance() ),
  playerController(),
  playerActor(nullptr),
  state(STATE_PLAYING),
  ambience(AMBIENCE),
  light(nullptr),
  endPicture(0.f),
  picDeath(ImageFactory::getInstance().getImage("assets/youdied.png")),
  picVictory(ImageFactory::getInstance().getImage("assets/victory.png"))
{
}

GameState::~GameState()
{
}

void GameState::enter()
{
  // The previous state should load the scene
  gamemgr.loadScene("scn_area1");
  spawnPlayer();
  gamemgr.spawnAllEnemies();

  LightManager &lightmgr = LightManager::getInstance();
  light = lightmgr.addLight( playerActor->getPosition(), 2000, 1.f );
  lightmgr.setAmbience(ambience = AMBIENCE);

  state = STATE_PLAYING;
  endPicture = 0.f;
}

void GameState::exit()
{
  gamemgr.clearScene();
  playerActor = nullptr;

  light->kill();
  light = nullptr;

  //gamemgr.update(0.f);
}
  
void GameState::input(const SDL_Event& event)
{
//Viewport &v = Viewport::getInstance();
  if (event.type == SDL_KEYDOWN) {
  switch (event.key.keysym.scancode) {
    case SDL_SCANCODE_SPACE: playerController.inputJumping(true); break;
    case SDL_SCANCODE_A:     playerController.inputLeft(true);    break;
    case SDL_SCANCODE_D:     playerController.inputRight(true);   break;
    case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL: playerController.inputAttack(); break;
    case SDL_SCANCODE_DELETE: playerActor->destroy(); break;
    case SDL_SCANCODE_F1:    gamemgr.toggleDebugHUD(); break;
    case SDL_SCANCODE_G:     playerActor->setGod(!playerActor->isAGod()); break;

    // Restart the game
    case SDL_SCANCODE_F2: exit(); enter(); break;
      
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

void GameState::update(float delta)
{
  if (Actor::getMaskCounts(PhysicsManager::MASK_ENEMY) == 0)
    state = STATE_VICTORY;
  
  if (state == STATE_DEAD)
    gamemgr.update(delta*.5f);
  else
    gamemgr.update(delta);

  if (playerActor != nullptr)
    gamemgr.setDebugMessage(10, "G: toggle god mode (it's " + (playerActor->isAGod() ? std::string("ON"):std::string("OFF")) + ")");
  else
    gamemgr.setDebugMessage(10, "G: toggle god mode (it's OFF)");
  
  if (state == STATE_PLAYING) {
    Viewport::getInstance().setTarget( playerActor->getPosition() - playerActor->getOffset()
				       + Vec2f(0, -250 + std::max(playerActor->getPhysics().getVelocity()[1]*.04f, 0.f)) );

    light->setPosition( playerActor->getPosition() + Vec2f(0, -200) );

    if (!playerActor->isAlive()) {
      state = STATE_DEAD;
      playerActor = nullptr;
    }
  }

  if (state == STATE_DEAD) {
    LightManager::getInstance().setAmbience(ambience = std::max(0.f, ambience - delta * DEATH_TIME));
    light->setIntensity( std::max(0.f, light->getIntensity() - delta * DEATH_TIME ) );
  }

  if (state == STATE_VICTORY || (state == STATE_DEAD && light->getIntensity() <= 0.01f))
    endPicture = std::min(1.f, endPicture+delta);
}

void GameState::draw() const
{
  gamemgr.draw();

  if (state == STATE_DEAD) {
    picDeath->superDraw(0, 0, 1.f, endPicture);
  } else if (state == STATE_VICTORY) {
    picVictory->superDraw(0, 0, 1.f, endPicture);
  }

  IoMod::getInstance().writeText("Enemies Left: " + StringUtil::toString(Actor::getMaskCounts(PhysicsManager::MASK_ENEMY)),
				 30, Viewport::getInstance().getHeight() - 120);

  if (state != STATE_PLAYING) return;

  // Draw health (SDL HACK AND NOT REAL CLASSES 'CAUSE I'M OUT OF TIME)
  SDL_Renderer *r = RenderContext::getInstance().getRenderer();

  int xpos = Viewport::getInstance().getWidth() * .5f;
  int ypos = 32;
  int width = 256;
  int height = 32;
  
  SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
  SDL_Rect area = {xpos-width-1, ypos-1, width*2+2, height+2};
  SDL_RenderFillRect(r, &area);
  
  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
  area = {xpos-width, ypos, width*2, height};
  SDL_RenderFillRect(r, &area);
  
  SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
  if (playerActor != nullptr) {
    int w = (width*2 - 8) * playerActor->getHealthPercent();
    area = {xpos-width+4, ypos+4, w, height-8};
    SDL_RenderFillRect(r, &area);
  }
}

void GameState::spawnPlayer()
{
  (playerActor = gamemgr.spawnPlayer("player", "start"))->setPlayerControlled(&playerController);
}
