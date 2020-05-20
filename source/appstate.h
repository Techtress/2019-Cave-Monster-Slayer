#ifndef APPSTATE_H
#define APPSTATE_H

union SDL_Event;

class AppState
{
 public:

  AppState() {}
  virtual ~AppState() {}

  virtual void enter() = 0;
  virtual void exit() = 0;
  
  virtual void input(const SDL_Event&) = 0;
  virtual void update(float delta) = 0;
  virtual void draw() const = 0;

  // No default initialization or copying allowed.
  AppState(const AppState&) = delete;
  AppState &operator=(const AppState&) = delete;
};

#endif
