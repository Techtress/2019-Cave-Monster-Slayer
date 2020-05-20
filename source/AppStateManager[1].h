#ifndef APPSTATEMANAGER_H
#define APPSTATEMANAGER_H

class AppState;

class AppStateManager
{
 public:
  ~AppStateManager();
  
  static AppStateManager &getInstance();

  // Ends the current state and enters the new specified one
  void changeState(AppState*);

  // Updates the current state
  void stateInput(const SDL_Event&);
  void stateUpdate(float delta);
  void stateDraw() const;

  // Ends the current state without entering a new one
  void stateEnd();

  AppStateManager(const AppStateManager&) = delete;
  AppStateManager &operator=(const AppStateManager&) = delete;

 private:
  AppStateManager() : currentState(nullptr) {}
  
  AppState *currentState;
};

#endif
