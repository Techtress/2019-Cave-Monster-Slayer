#ifndef TESTINGSTATE_H
#define TESTINGSTATE_H

#include "appstate.h"
#include "entity/playercontroller.h"

class GameManager;
class Actor;

class TestingState : public AppState
{
 public:
  TestingState(AppState* rs);
  ~TestingState();
  
  virtual void enter() override;
  virtual void exit() override;
  
  virtual void input(const SDL_Event&) override;
  virtual void update(float delta) override;
  virtual void draw() const override;

  TestingState(const TestingState&) = delete;
  TestingState &operator=(const TestingState&) = delete;

 private:
  AppState *returnState;
  GameManager &gamemgr;
  PlayerController playerController;
  Actor           *playerActor;
};

#endif
