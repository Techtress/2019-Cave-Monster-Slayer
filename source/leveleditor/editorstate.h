#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include <vector>
#include <list>
#include <map>
#include <functional>

#include "appstate.h"
#include "segment.h"
#include "eventmanager.h"

class Image;
class Backdrop;

class EditorState : public AppState
{
 public:
  EditorState();
  virtual ~EditorState();
  
  virtual void enter() override;
  virtual void exit() override;
  
  virtual void input(const SDL_Event&) override;
  virtual void update(float delta) override;
  virtual void draw() const override;

  EditorState(const EditorState&) = delete;
  EditorState &operator=(const EditorState&) = delete;
  
 private:

  AppState *testingState;
  bool testing;
  
  enum Mode
  {
    MODE_EDITSCENE,
    MODE_EDITLAYER,
    MODE_BACKDROP,
    MODE_WORLDSEG,
    MODE_ENTRYPOINT,
    MODE_ENEMYSPAWN
  } mode;

  std::map<int,std::function<void()> > modeKeyMap;

  enum State
  {
    STATE_NONE,
    STATE_EDIT,
  } state;

  enum EditingState
  {
    EDIT_NONE,
    EDIT_MOVE,
    EDIT_ROTATE,
    EDIT_SCALE
  } editing;

  struct Option
  {
    std::string name;
    std::function<void()> command;
  };

  bool navLeft, navRight, navUp, navDown;
  std::vector<float> zoomLevels;
  int currentZoom;
  Vec2f worldMouse, worldRelMouse, editMouseStart;

  // Current set
  std::string optionName;
  std::vector<Option> optionsList;
  std::map<int,int> optionsKeyMap;

  struct TextEntry
  {
    TextEntry()
    : contents(""), active(false), command() {}
    std::string contents;
    bool active;
    std::function<void()> command;
  } textEntry;


  template<class F>
  void registerOption(const std::string &name, int keycode, F &&f) {
    optionsKeyMap[keycode] = optionsList.size();
    optionsList.push_back( {name, std::function<void()>(std::bind(f, this))} );
  }
  
  void clearOptions() {
    optionName = "";
    optionsList.clear();
    optionsKeyMap.clear();
  }

  // Exports the current scene to an XML with the
  // name of the current string in the textEntry
  void exportScene();

  // Loads scene from textEntry
  void loadScene();

  // Last entered scene from export/load
  std::string lastScene;

  void resetMode();

  // Scene Stuff
  void switchMode_EditScene();
  void scnWidthPrompt();
  void scnWidth();
  void scnHeightPrompt();
  void scnHeight();

  // Layer stuff
  int currentLayer;
  void switchMode_EditLayer();

  void lyScrollFactor();
  void lyScrollFactorPrompt();

  void lyBackground();
  void lyBackgroundPrompt();

  void lyBackgroundAlpha();
  void lyBackgroundAlphaPrompt();

  // World Segment stuff
  void switchMode_WorldSeg();

  void ws_Extrude();
  void ws_Select1();
  void ws_Select2();
  void ws_Delete();

  Segment *ws_Selected;
  int ws_Selected_pt;

  // Backdrop stuff
  void switchMode_Backdrop();

  void bdMove();
  void bdRotate();
  void bdScale();
  void bdDelete();
  void bdToFront();
  void bdToBack();
  void bdFlipH();
  void bdFlipV();

  void bdMoveAllUp();
  void bdMoveAllDown();

  std::map<std::string, const Image*> bdImages;
  float bdListScroll;
  std::pair<const Image*, unsigned> bdHover;
  Backdrop *bdSelected;
  float bdStartScaleX, bdStartScaleY;
  float bdStartAngle;

  // Entry Point Stuff
  void switchMode_EntryPoint();

  void epCreatePrompt();
  void epCreate();
  void epGrab();
  void epDelete();

  EntryPoint *epSelected;

  // Enemy stuff
  void switchMode_EnemySpawn();
  void espCreatePrompt();
  void espCreate();
  void espGrab();
  void espRenamePrompt();
  void espRename();
  void espDelete();

  std::string espLastName;
  EntryPoint *espSelected;
};

#endif
