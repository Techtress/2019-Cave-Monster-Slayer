#include "engine.h"

#include "rendercontext.h"
#include "iomod.h"
#include "clock.h"
#include "viewport.h"
#include "appstatemanager.h"
#include "gameconfig.h"

#include <iostream>
#include <SDL.h>

Engine::~Engine()
{ 
  std::cout << "Terminating program" << std::endl;
}

Engine::Engine() :
  rc( RenderContext::getInstance() ),
  io( IoMod::getInstance() ),
  clock( Clock::getInstance() ),
  viewport( Viewport::getInstance() ),
  appmgr( AppStateManager::getInstance() ),
  renderer( rc.getRenderer() )
{
  std::cout << "Initialized Engine" << std::endl;
}

void Engine::update(float delta)
{
  appmgr.stateUpdate(delta);
  viewport.update(delta);
}

void Engine::draw() const
{
  // Draw a blackground
  SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
  SDL_RenderClear(renderer);
  
  appmgr.stateDraw();
  viewport.draw();
  SDL_RenderPresent(renderer);
}

void Engine::play(AppState *state)
{
  GameConfig &cfg = GameConfig::getInstance();
  SDL_Event event;

  // Get some properties about the screen frame rate
  int frameCap = cfg["frameCap"].toInt();
  bool vsync = cfg["vsync"].toBool();

  // Now begin the app state to start running
  appmgr.changeState(state);

  // Start the clock here instead of counting the time it took to start+load everything
  clock.incrementTime();

  // Start the game loop
  bool done = false, paused = false;
  while ( !done ) {
    // The next loop polls for events, guarding against key bounce:
    

    // Calculate time since last time increment
    clock.updateDelta();

    // Ignore framecap if VSync is on (find a fix later to have both)
    if ( vsync || clock.getFPS() <= frameCap ) {
      // Create a new starting point in time to calculate delta from
      clock.incrementTime();

      while ( SDL_PollEvent(&event) ) {
	if (event.type ==  SDL_QUIT) {
	  done = true;
	  break;
	}
	else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
	  switch (event.key.keysym.scancode) {
	  case SDL_SCANCODE_ESCAPE:
	    done = true; break;
	    //case SDL_SCANCODE_P:
	    //paused = !paused; break;
	  default: break;
	  }
	}

	// Aside from the "super" options, input into the current game state
	// for the main input handling
	appmgr.stateInput(event);
      }

      // Update the engine only if we are not paused
      if (!paused) update(clock.getDelta());

      draw();
    }
  }

  // End the current state at the end of the loop
  appmgr.stateEnd();
}
