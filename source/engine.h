#ifndef ENGINE_H
#define ENGINE_H

class SDL_Renderer;

class RenderContext;
class IoMod;
class Clock;
class Viewport;
class AppState;
class AppStateManager;

class Engine {
public:
  Engine ();
  ~Engine ();

  // Start the application under a specific AppState
  void play(AppState*);

  Engine(const Engine&) = delete;
  Engine &operator=(const Engine&) = delete;

private:
  const RenderContext &rc;
  IoMod &io;
  Clock &clock;
  Viewport &viewport;
  AppStateManager &appmgr;

  SDL_Renderer * const renderer;

  void draw() const;
  void update(float delta);
};

#endif
