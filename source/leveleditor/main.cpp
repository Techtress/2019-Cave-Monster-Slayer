#include <iostream>

#include "gameconfig.h"
#include "engine.h"
#include "editorstate.h"

int main(int, char*[]) {
  try {
    Engine engine;
    EditorState state;
    engine.play(&state);
  }
  catch (const std::string& msg) { std::cout << msg << std::endl; }
  catch (std::exception &e) { std::cout << e.what() << std::endl; }
  catch (...) {
    std::cout << "Oops, someone threw an exception!" << std::endl;
  }
  return 0;
}
