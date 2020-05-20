#include "rendercontext.h"
#include "gameconfig.h"

#include <string>
#include <SDL.h>

RenderContext::RenderContext() :
  window(nullptr),
  renderer(nullptr)
{
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    throw (std::string("Could not init SDL: ") + SDL_GetError());
  }
  window = initWindow();
  renderer = initRenderer();
}

RenderContext::~RenderContext() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

RenderContext& RenderContext::getInstance() {
  static RenderContext instance;
  return instance;
}

SDL_Window* RenderContext::initWindow( ) {

  GameConfig &cfg = GameConfig::getInstance();
  
  std::string title = cfg["title"].toStr();
  int width  = cfg["view"]["width"].toInt();
  int height = cfg["view"]["height"].toInt();
  
  window = SDL_CreateWindow( title.c_str(), SDL_WINDOWPOS_CENTERED, 
    SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN );

  if (cfg["view"]["fullscreen"].toBool())
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	
  if( window == nullptr ) {
    throw (std::string("Couldn't make a window: ")+SDL_GetError());
  }
  
  return window;
}

SDL_Renderer* RenderContext::initRenderer() {
  // To test the Clock class's ability to cap the frame rate, use:
  SDL_Renderer* renderer =
    GameConfig::getInstance()["vsync"].toBool() ?
    SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) :
    SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    //SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE );
  if ( renderer == nullptr ) throw std::string("Could not create renderer!\n  Message: " + std::string(SDL_GetError()));
  return renderer;
}
