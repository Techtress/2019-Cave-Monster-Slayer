#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "appstate.h"
#include "entity/playercontroller.h"

class GameManager;
class Actor;
class Light;
class Image;

class GameState : public AppState
{
 public:
  GameState();
  ~GameState();
  
  virtual void enter() override;
  virtual void exit() override;
  
  virtual void input(const SDL_Event&) override;
  virtual void update(float delta) override;
  virtual void draw() const override;

  GameState(const GameState&) = delete;
  GameState &operator=(const GameState&) = delete;

 private:
  GameManager &gamemgr;
  
  PlayerController playerController;
  Actor           *playerActor;

  enum State
  {
    STATE_PLAYING,
    STATE_DEAD,
    STATE_VICTORY
  } state;

  float ambience;
  Light *light;
  float endPicture;

  const Image *picDeath;
  const Image *picVictory;

  void spawnPlayer();
};

#endif
