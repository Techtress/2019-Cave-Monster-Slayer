#include "editorstate.h"

#include "physicsmanager.h"
#include "rendercontext.h"
#include "iomod.h"
#include "viewport.h"
#include "gamemanager.h"
#include "canvas.h"
#include "stringutil.h"
#include "appstatemanager.h"
#include "testingstate.h"
#include "imagefactory.h"
#include "lightmanager.h"

#include <SDL.h>
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <stack>

const float SPEED = 3000.f;
const float BG_REGION = 400;
const float RAD2DEG = 180 / M_PI;

EditorState::EditorState() :
  testingState(new TestingState(this)),
  testing(false),
  mode(MODE_BACKDROP),
  modeKeyMap(),
  state(STATE_NONE),
  editing(EDIT_NONE),
  navLeft(false), navRight(false), navUp(false), navDown(false),
  zoomLevels{ .25, .50, .75, 1, 1.5, 2, 3 },
  currentZoom( 3 ),
  worldMouse(), worldRelMouse(), editMouseStart(),
  optionName(""),
  optionsList(),
  optionsKeyMap(),
  textEntry(),
  lastScene(""),

  currentLayer(Canvas::LAYER_MAIN),
  
  ws_Selected(nullptr),
  ws_Selected_pt(-1),

  bdImages(),
  bdListScroll(0.f),
  bdHover(std::make_pair(nullptr, 0)),
  bdSelected(nullptr),
  bdStartScaleX(1.f), bdStartScaleY(1.f),
  bdStartAngle(0.f),

  epSelected(nullptr),

  espLastName(""),
  espSelected(nullptr)
{
  switchMode_WorldSeg();

  // Load all backdropsets
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir ("./assets/backdrops/")) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      std::string name = ent->d_name;
      if (name.size() > 3 && name.substr(name.length()-3, 3) == "png") {
	bdImages[name] = ImageFactory::getInstance().getImage("assets/backdrops/" + name);
      }
    }
    closedir (dir);
  }

  modeKeyMap[SDL_SCANCODE_1] = std::bind(&EditorState::switchMode_EditScene, this);
  modeKeyMap[SDL_SCANCODE_2] = std::bind(&EditorState::switchMode_WorldSeg, this);
  modeKeyMap[SDL_SCANCODE_3] = std::bind(&EditorState::switchMode_Backdrop, this);
  modeKeyMap[SDL_SCANCODE_4] = std::bind(&EditorState::switchMode_EditLayer, this);

  // TO-DO: REDESIGN "EVENTMANAGER" ENTIRELY, AND MAKE THESE ON THE SAME PAGE
  modeKeyMap[SDL_SCANCODE_5] = std::bind(&EditorState::switchMode_EntryPoint, this);
  modeKeyMap[SDL_SCANCODE_6] = std::bind(&EditorState::switchMode_EnemySpawn, this);
}

EditorState::~EditorState()
{
}

void EditorState::enter()
{
  //GameManager::getInstance().loadScene("scn_test");
  LightManager::getInstance().setAmbience(1.f);
}

void EditorState::exit() 
{
}
  
void EditorState::input(const SDL_Event& event) 
{
  Viewport &v = Viewport::getInstance();
  Canvas &c = Canvas::getInstance();
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  // Changing anything in the layer will add the layer to the system and thus give the right scroll factor
  float scroll = c.hasLayer(currentLayer) ? c.getScrollFactor(currentLayer) : 1.f;

  if (textEntry.active) {
    if (event.type == SDL_KEYDOWN) {
      int key = event.key.keysym.sym;
      switch (key) {
      case SDLK_RETURN:
	textEntry.command();
	textEntry.active = false;
	break;

      case SDLK_BACKSPACE: {
	std::string &text = textEntry.contents;
	if (text.length() > 0)
	  text = text.substr(0, text.length()-1);
      } break;

      case SDLK_MINUS: {
	if (keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT])
	  textEntry.contents = textEntry.contents + "_";
      } break;

      default: {
	if ((key >= SDLK_a && key <= SDLK_z) ||
	    key == SDLK_PERIOD || (key >= SDLK_0 && key <= SDLK_9)) {
	  textEntry.contents = textEntry.contents + (char)key;
	}
      } break;
      }
    }
  }
  else {

    if (event.type == SDL_KEYDOWN) {

      int key = event.key.keysym.scancode;

      if (keystate[SDL_SCANCODE_LCTRL]) {

	if (modeKeyMap.find(key) != modeKeyMap.end())
	  modeKeyMap[key]();
    
	switch (key) {
	case SDL_SCANCODE_S:
	  textEntry.contents = lastScene;
	  textEntry.active = true;
	  textEntry.command = std::bind(&EditorState::exportScene, this);
	  break;
	case SDL_SCANCODE_L:
	  textEntry.contents = lastScene;
	  textEntry.active = true;
	  textEntry.command = std::bind(&EditorState::loadScene, this);
	  break;
	  
	case SDL_SCANCODE_PAGEUP:
	  clearOptions();
	  optionName = "WARNING: Make sure layer " + StringUtil::toString(currentLayer+1) + " is clear before moving everything!";
	  registerOption("[Y]es, proceed", SDL_SCANCODE_Y, &EditorState::bdMoveAllUp );
	  registerOption("[N]o, do not proceed", SDL_SCANCODE_N, &EditorState::resetMode );
	  break;
	  
	case SDL_SCANCODE_PAGEDOWN:
	  clearOptions();
	  optionName = "WARNING: Make sure layer " + StringUtil::toString(currentLayer-1) + " is clear before moving everything!";
	  registerOption("[Y]es, proceed", SDL_SCANCODE_Y, &EditorState::bdMoveAllDown );
	  registerOption("[N]o, do not proceed", SDL_SCANCODE_N, &EditorState::resetMode );
	  break;
	  
	default:break;
	}
      } else {
	
	// Do the thing
	auto result = optionsKeyMap.find(event.key.keysym.scancode);
	if (result != optionsKeyMap.end())
	  optionsList[result->second].command();
    
	switch (key) {
	case SDL_SCANCODE_LEFT: case SDL_SCANCODE_A:  navLeft = true; break;
	case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_D: navRight = true; break;
	case SDL_SCANCODE_UP: case SDL_SCANCODE_W:    navUp = true; break;
	case SDL_SCANCODE_DOWN: case SDL_SCANCODE_S:  navDown = true; break;
	case SDL_SCANCODE_PAGEUP: currentLayer++; bdSelected = nullptr; break;
	case SDL_SCANCODE_PAGEDOWN: currentLayer--; bdSelected = nullptr; break;
	case SDL_SCANCODE_KP_MINUS: v.setZoomFactor( zoomLevels[ currentZoom = std::max(0, currentZoom-1) ] ); break;
	case SDL_SCANCODE_KP_PLUS: v.setZoomFactor( zoomLevels[ currentZoom = std::min((int)zoomLevels.size()-1, currentZoom+1) ] ); break;
	case SDL_SCANCODE_F2:
	  PhysicsManager::getInstance().editor_UpdateSegmentList();
	  AppStateManager::getInstance().changeState(testingState);
	  break;

	default: break;
	}
      }
    }
    else if (event.type == SDL_KEYUP) {
      switch (event.key.keysym.scancode) {
      case SDL_SCANCODE_LEFT: case SDL_SCANCODE_A:  navLeft = false; break;
      case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_D: navRight = false; break;
      case SDL_SCANCODE_UP: case SDL_SCANCODE_W:    navUp = false; break;
      case SDL_SCANCODE_DOWN: case SDL_SCANCODE_S:  navDown = false; break;
      default: break;
      }
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (mode == MODE_BACKDROP) {
	if (bdSelected == nullptr && bdHover.first != nullptr) {
	  bdSelected = Canvas::getInstance().placeBackdrop(currentLayer, bdHover.first, v.getMouseWorldPos(scroll), bdHover.second,
							   0.f, 1.f, 1.f, false, false);
	  editing = EDIT_MOVE;
	}
	else if (bdSelected != nullptr) {
	  if (editing == EDIT_NONE)
	    bdSelected = nullptr;
	  // Cancel whatever state...
	  else editing = EDIT_NONE;
	}

	// Select a backdrop
	else if (bdSelected == nullptr && bdHover.first == nullptr) {
	  float bestDist = -1;
	  for (Backdrop &b : Canvas::getInstance().getBackdropList(currentLayer)) {

	    const Image *img = b.getImage();
	    int f = b.getFrame();
	    float sx = b.getScaleX(),
	      sy = b.getScaleY();
	    float angle = b.getAngle() / RAD2DEG;
	    float offsetx = img->getFrameCenterX(f) * sx,
	      offsety = img->getFrameCenterY(f) * sy;
	    float sizex = img->getFrameWidth(f)*sx,
	      sizey = img->getFrameHeight(f)*sy;
	    Vec2f start = b.getPosition()
	      + Vec2f(cos(angle) * offsetx - sin(angle) * offsety, sin(angle) * offsetx + cos(angle) * offsety);

	    Vec2f mid = start + Vec2f( cos(angle) * sizex - sin(angle) * sizey, sin(angle) * sizex + cos(angle) * sizey )*.5f;

	    float dist = (mid - v.getMouseWorldPos(scroll)).length();
	    float size = std::max(sizex, sizey);
	  
	    if (dist < size*.5f && (bestDist < 0 || dist < bestDist)) {
	      bestDist = dist;
	      bdSelected = &b;
	    }
	  }
	}
      }
      else if (mode == MODE_WORLDSEG) {
	if (ws_Selected_pt >= 0)
	  ws_Selected_pt = -1;
	else {
	  float bestDist = -1.f;
	  PhysicsManager::getInstance().editor_QueryAllSegments([&](Segment &s) {
	      int x, y;
	      SDL_GetMouseState(&x, &y);
	      Vec2f mouse_pos = Vec2f( v.getX()+(x/v.getZoomFactor()) - (v.getWidth()/2)/v.getZoomFactor(),
				       v.getY()+(y/v.getZoomFactor()) - (v.getHeight()/2)/v.getZoomFactor() );
	      float dist = (s[1]-s[0]).normalize().dot((mouse_pos-s[0]).normalize()) * (s[0]-mouse_pos).length();
	      if (dist < 0)
		dist = (s[0]-mouse_pos).length();
	      else if (dist > (s[0]-s[1]).length())
		dist = (s[1]-mouse_pos).length();
	      else {
		dist = ((s[0]+((s[1]-s[0]).normalize()*dist))-mouse_pos).length();//(((s[0]-s[1]).normalize()*dist)-mouse_pos).length();
	      }
	      if (dist < 64.f && (bestDist < 0.f || dist < bestDist)) {
		bestDist = dist;
		ws_Selected = &s;
	      } });
	  if (bestDist < 0.f)
	    ws_Selected = nullptr;
	}
      }
      else if (mode == MODE_ENTRYPOINT) {
	if (epSelected != nullptr) {
	  if (state == STATE_EDIT) {
	    state = STATE_NONE;
	    editing = EDIT_NONE;
	  }
	  else
	    epSelected = nullptr;
	}
	else {
	  float closest = -1.f;
	  for (const EntryPoint &e : EventManager::getInstance().getEntryPoints()) {
	    float dist = (e.getPosition()-v.getMouseWorldPos()).length();
	    if (dist < 64 && (closest < 0.f || dist < closest)) {
	      closest = dist;
	      epSelected = const_cast<EntryPoint*>(&e);
	    }
	  }
	}
      }
      else if (mode == MODE_ENEMYSPAWN) {
	if (espSelected != nullptr) {
	  if (state == STATE_EDIT) {
	    state = STATE_NONE;
	    editing = EDIT_NONE;
	  }
	  else
	    espSelected = nullptr;
	}
	else {
	  float closest = -1.f;
	  for (EntryPoint &e : EventManager::getInstance().editor_getEnemySpawnList()) {
	    float dist = (e.getPosition()-v.getMouseWorldPos()).length();
	    if (dist < 64 && (closest < 0.f || dist < closest)) {
	      closest = dist;
	      espSelected = &e;
	    }
	  }
	}
      }
    }
    else if (event.type == SDL_MOUSEWHEEL) {

      float region = 1920;
      if (mode == MODE_BACKDROP)
	region -= BG_REGION;

      int x, y;
      SDL_GetMouseState(&x, &y);

      if (x <= region) {
	if (event.wheel.y < 0)
	  v.setZoomFactor( zoomLevels[ currentZoom = std::max(0, currentZoom-1) ] );
	else if (event.wheel.y > 0) {
	  v.setZoomFactor( zoomLevels[ currentZoom = std::min((int)zoomLevels.size()-1, currentZoom+1) ] );
	  v.setTarget( v.getMouseWorldPos() );
	}
      } else if (mode == MODE_BACKDROP) {
	// Scroll down the backdrop list
	bdListScroll -= event.wheel.y * 128;
      }
    }
  }
}

void EditorState::update(float delta) 
{
  Viewport &v = Viewport::getInstance();
  Canvas &c = Canvas::getInstance();
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  float scroll = c.hasLayer(currentLayer) ? c.getScrollFactor(currentLayer) : 1.f;

  worldRelMouse = v.getMouseWorldPos() - worldMouse;
  worldMouse = v.getMouseWorldPos();

  if (editing == EDIT_NONE)
    editMouseStart = worldMouse;

  float speed = 1.f;
  if (keystate[SDL_SCANCODE_LSHIFT]) speed = 3.f;
  else if (keystate[SDL_SCANCODE_LCTRL]) speed = 0.25f;

  if (navLeft || navRight || navUp || navDown) {
    Vec2f movement = Vec2f( -navLeft + navRight, -navUp + navDown );
    movement = movement.normalize();
    movement = movement * speed * SPEED * delta;
    v.setTarget( v.getTarget() + movement );
  }
  //v.setX( v.getX() + movement[0] );
  //v.setY( v.getY() + movement[1] );

  if (mode == MODE_BACKDROP) {
    
    bdHover = std::make_pair(nullptr, 0);
    
    int x = 1920 - BG_REGION;
    int y = 50 - bdListScroll;

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    for (auto &p : bdImages) {
      const Image *img = p.second;

      y += 50;
      float size = 64;
      float offset = 0;
      for (int f = 0; f < img->getNumFrames(); f++) {

	if (x+offset > 1920 - size - 8) {
	  y += size + 8;
	  offset = 0;
	}

	if (mx >= x + offset && mx <= x + offset + size &&
	    my >= y && my <= y + size)
	  bdHover = std::make_pair(img, f);
	
	offset += size + 8;
      }
      y += size*2;
    }

    if (bdSelected != nullptr) {
      if (editing == EDIT_MOVE) {
	bdSelected->setPosition(bdSelected->getPosition() + worldRelMouse);
      }
      else if (editing == EDIT_ROTATE) {
	Vec2f basePos = bdSelected->getPosition()/scroll;
	Vec2f startDir = (editMouseStart - basePos).normalize();
	Vec2f currentDir = (worldMouse - basePos).normalize();
	float startAngle = atan2( startDir[1], startDir[0] ) * RAD2DEG;
	float offsetAngle = atan2( currentDir[1], currentDir[0] ) * RAD2DEG;
	float angle = offsetAngle - startAngle;
	bdStartAngle += angle;
	if (bdStartAngle >= 360) bdStartAngle -= 360;
	else if (bdStartAngle < 0) bdStartAngle += 360;

	
	bdSelected->setAngle(bdStartAngle);
	if (keystate[SDL_SCANCODE_LCTRL]) {
	  bdSelected->setAngle( (int)(bdSelected->getAngle()/11.25)*11.25 );
	}
	
	editMouseStart = worldMouse;
	if (keystate[SDL_SCANCODE_SPACE]) {
	  bdSelected->setAngle(1.f);
	  editing = EDIT_NONE;
	}
      }
      else if (editing == EDIT_SCALE) {
	//const Image *img = bdSelected->getImage();
	//unsigned f = bdSelected->getFrame();
	Vec2f origin = editMouseStart - bdSelected->getPosition()/scroll;
	Vec2f end = worldMouse - bdSelected->getPosition()/scroll;
	float amt = end.length() / origin.length();
	bdSelected->setScaleX( amt * bdStartScaleX );
	bdSelected->setScaleY( amt * bdStartScaleY );

	if (keystate[SDL_SCANCODE_X])
	  bdSelected->setScaleY( bdStartScaleY );
	if (keystate[SDL_SCANCODE_Y])
	  bdSelected->setScaleX( bdStartScaleX );
	if (keystate[SDL_SCANCODE_SPACE]) {
	  bdSelected->setScaleX( 1.f );
	  bdSelected->setScaleY( 1.f );
	  editing = EDIT_NONE;
	}
      }
      
    }
  }
  else if (mode == MODE_WORLDSEG) {
    if (state == STATE_EDIT) {
      if (ws_Selected_pt >= 0) {
	Vec2f &current = (*ws_Selected)[ws_Selected_pt];
        current = v.getMouseWorldPos();

	if (keystate[SDL_SCANCODE_LCTRL]) {
	  float closestDist = -1.f;
	  PhysicsManager::getInstance().editor_QueryAllSegments([&](Segment &s) {
	      if (&s == ws_Selected)
		return;
	      int i = !ws_Selected_pt;
	      float dist = (s[i] - current).length();
	      if (dist < 64.f && (closestDist < 0.f || dist < closestDist)) {
		closestDist = dist;
		s[i] = current;
	      } });
	}
      }
    }
  }
  else if (mode == MODE_ENTRYPOINT) {
    if (state == STATE_EDIT) {
      if (editing == EDIT_MOVE) {
	epSelected->setPosition(v.getMouseWorldPos());
      }
    }
  }
  else if (mode == MODE_ENEMYSPAWN) {
    if (state == STATE_EDIT) {
      if (editing == EDIT_MOVE) {
	espSelected->setPosition(v.getMouseWorldPos());
      }
    }
  }

  GameManager::getInstance().update(delta);
}

void EditorState::draw() const 
{
  GameManager::getInstance().draw();
  IoMod &io = IoMod::getInstance();
  SDL_Renderer *r = RenderContext::getInstance().getRenderer();
  const Viewport &v = Viewport::getInstance();
  float zoom = v.getZoomFactor();

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  PhysicsManager::getInstance().editor_QueryAllSegments([&](Segment& w) {
    
      Vec2f a = v.getScreenPos(w[0]),
	b = v.getScreenPos(w[1]);

      if (mode == MODE_WORLDSEG && ws_Selected != &w && ws_Selected_pt >= 0) {
	SDL_SetRenderDrawColor(r, 0, 0, 255, 255);
	Vec2f current = ws_Selected_pt == 0 ? b : a;
	int x = current[0], y = current[1];
	SDL_Rect rec = { x - 5, y - 5, 10, 10 };
	SDL_RenderFillRect(r, &rec);
      }
    
      if (mode == MODE_WORLDSEG && ws_Selected == &w) {

	SDL_SetRenderDrawColor(r, 128, 128, 255, 255);
	int x = a[0], y = a[1];
	SDL_Rect rec = { x - 5, y - 5, 10, 10 };
	SDL_RenderFillRect(r, &rec);

	SDL_SetRenderDrawColor(r, 0, 0, 128, 255);
	x = b[0]; y = b[1];
	rec = { x - 5, y - 5, 10, 10 };
	SDL_RenderFillRect(r, &rec);

	SDL_SetRenderDrawColor(r, 255, 255, 0, 255);
      } else
	SDL_SetRenderDrawColor(r, 255, 0, 0, 255);

      SDL_RenderDrawLine(r,
			 a[0], a[1],
			 b[0], b[1]);
      Vec2f h1 = a+(b-a)*.5f;
      Vec2f h2 = b-a; h2 = Vec2f(-h2[1], h2[0]).normalize();
      h2 = h1 + h2 * 32;
      SDL_RenderDrawLine(r,
			 h1[0], h1[1],
			 h2[0], h2[1]); });

  io.writeText( "Zoom: " + StringUtil::toString((int)(v.getZoomFactor()*100)) + "%", 1720, 1000);

  io.writeText( "Current Layer: " + StringUtil::toString(currentLayer), 50, 900);

  io.writeText( optionName, 50, 50 );

  int i = 1;
  for (const Option &o : optionsList)
    io.writeText( o.name, 50, 50 + (i++)*35 );

  if (mode == MODE_BACKDROP) {

    if (bdSelected != nullptr) {
      const Image *img = bdSelected->getImage();
      int f = bdSelected->getFrame();
      float sx = bdSelected->getScaleX(),
	sy = bdSelected->getScaleY();
      float angle = bdSelected->getAngle() / RAD2DEG;
      float offsetx = img->getFrameCenterX(f) * sx * v.getZoomFactor(),
	offsety = img->getFrameCenterY(f) * sy * v.getZoomFactor();
      float sizex = img->getFrameWidth(f)*sx*v.getZoomFactor(),
	sizey = img->getFrameHeight(f)*sy*v.getZoomFactor();
      Vec2f start = v.getScreenPos(bdSelected->getPosition(), Canvas::getInstance().getScrollFactor(currentLayer))
	+ Vec2f(cos(angle) * offsetx - sin(angle) * offsety, sin(angle) * offsetx + cos(angle) * offsety);

      SDL_SetRenderDrawColor(r, 255, 255, 0, 255);

      SDL_RenderDrawLine(r,
			 (int)(start[0]), (int)(start[1]),
			 (int)(start[0] + cos(angle) * sizex),
			 (int)(start[1] + sin(angle) * sizex) );
      SDL_RenderDrawLine(r,
			 (int)(start[0] + cos(angle) * sizex),
			 (int)(start[1] + sin(angle) * sizex),
			 (int)(start[0] + cos(angle) * sizex - sin(angle) * sizey),
			 (int)(start[1] + sin(angle) * sizex + cos(angle) * sizey) );
      SDL_RenderDrawLine(r,
			 (int)(start[0] + cos(angle) * sizex - sin(angle) * sizey),
			 (int)(start[1] + sin(angle) * sizex + cos(angle) * sizey),
			 (int)(start[0] - sin(angle) * sizey),
			 (int)(start[1] + cos(angle) * sizey) );
      SDL_RenderDrawLine(r,
			 (int)(start[0] - sin(angle) * sizey),
			 (int)(start[1] + cos(angle) * sizey),
			 (int)(start[0]), (int)(start[1]) );


      (void)sizey;
    }

    int x = 1920 - BG_REGION;
    int y = 50 - bdListScroll;

    SDL_SetRenderDrawColor(r, 0, 0, 64, 128);
    SDL_Rect rect = { (int)(1920-BG_REGION) - 16, 0, (int)BG_REGION + 16, 1080};
    SDL_RenderFillRect(r, &rect);

    for (auto &p : bdImages) {
      const Image *img = p.second;

      io.writeText(p.first, x, y);
      y += 50;
      float size = 64;
      float offset = 0;
      for (int f = 0; f < img->getNumFrames(); f++) {

	if (x+offset > 1920 - size - 8) {
	  y += size + 8;
	  offset = 0;
	}

	img->uiDraw(x + offset, y, f, size, size);
	offset += size + 8;
      }
      y += size*2;
    }
  }

  // draw antry points
  for (const EntryPoint &e : EventManager::getInstance().getEntryPoints()) {
    if (&e == epSelected)
      SDL_SetRenderDrawColor(r, 0, 255, 0, 255);
    else
      SDL_SetRenderDrawColor(r, 0, 128, 0, 255);
    Vec2f pos = v.getScreenPos( e.getPosition() );
    int x = pos[0], y = pos[1]; 
    SDL_Rect rect = { x-5, y-5, 10, 10 };
    SDL_RenderFillRect(r, &rect);
    io.writeText( "EP: " + e.getName(), x - 40, y - 40 );
  }

  // draw enemy spawns
  for (const EntryPoint &e : EventManager::getInstance().editor_getEnemySpawnList()) {
    if (&e == espSelected)
      SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
    else
      SDL_SetRenderDrawColor(r, 128, 0, 0, 255);
    Vec2f pos = v.getScreenPos( e.getPosition() );
    int x = pos[0], y = pos[1]; 
    SDL_Rect rect = { x-5, y-5, 10, 10 };
    SDL_RenderFillRect(r, &rect);
    io.writeText( "Enemy: " + e.getName(), x - 40, y - 40 );
  }

  if (textEntry.active) {
    SDL_Rect rect = {0,0,1920,1080};
    SDL_SetRenderDrawColor(r, 0, 0, 0, 128);
    SDL_RenderFillRect(r, &rect);
    if (textEntry.contents.length() > 0)
      io.writeText(textEntry.contents, 400, 400);
  }


  // Draw realzone
  int hw = v.getWidth()/2,
    hh = v.getHeight()/2;
  int x = hw - hw*zoom, y = hh - hh*zoom,
    w = v.getWidth()*zoom, h = v.getHeight()*zoom;
  SDL_Rect rect = {x-1, y-1, w+2, h+2};
  SDL_SetRenderDrawColor(r, 200, 200, 255, 64);
  SDL_RenderDrawRect(r, &rect);

  // Draw world edges
  PhysicsManager &physics = PhysicsManager::getInstance();
  Vec2f edge = v.getScreenPos(Vec2f(physics.getWorldWidth(), physics.getWorldHeight()));
  SDL_SetRenderDrawColor(r, 0, 0, 255, 255);
  SDL_RenderDrawLine(r, edge[0], 0, edge[0], edge[1]);
  SDL_RenderDrawLine(r, edge[0], edge[1], 0, edge[1]);
}

void EditorState::resetMode()
{
  if (mode == MODE_EDITLAYER)
    switchMode_EditLayer();
  else if (mode == MODE_WORLDSEG)
    switchMode_WorldSeg();
  else if (mode == MODE_BACKDROP)
    switchMode_Backdrop();
}

void EditorState::switchMode_EditScene()
{
  mode = MODE_EDITSCENE;
  state = STATE_NONE;
  editing = EDIT_NONE;
  clearOptions();
  
  optionName = "Mode: Edit Scene";
  registerOption("S[e]t World Width", SDL_SCANCODE_E, &EditorState::scnWidthPrompt );
  registerOption("Set Wo[r]ld Height", SDL_SCANCODE_R, &EditorState::scnHeightPrompt ); 
}

void EditorState::scnWidthPrompt()
{
  textEntry.contents = StringUtil::toString(PhysicsManager::getInstance().getWorldWidth());
  textEntry.active = true;
  textEntry.command = std::bind(&EditorState::scnWidth, this);
}

void EditorState::scnWidth()
{
  PhysicsManager &physics = PhysicsManager::getInstance();
  physics.resizeWorld(StringUtil::toInt(textEntry.contents), physics.getWorldHeight());
  ws_Selected = nullptr;
  bdSelected = nullptr;
}

void EditorState::scnHeightPrompt()
{
  textEntry.contents = StringUtil::toString(PhysicsManager::getInstance().getWorldHeight());
  textEntry.active = true;
  textEntry.command = std::bind(&EditorState::scnHeight, this);
}

void EditorState::scnHeight()
{
  PhysicsManager &physics = PhysicsManager::getInstance();
  physics.resizeWorld(physics.getWorldWidth(), StringUtil::toInt(textEntry.contents));
  ws_Selected = nullptr;
  bdSelected = nullptr;
}

void EditorState::switchMode_EditLayer()
{
  mode = MODE_EDITLAYER;
  state = STATE_NONE;
  editing = EDIT_NONE;
  clearOptions();
  optionName = "Mode: Edit Layer";
  //registerOption("[TAB]: to World Segment Mode", SDL_SCANCODE_TAB, &EditorState::switchMode_WorldSeg );
  registerOption("Set S[c]roll factor", SDL_SCANCODE_C, &EditorState::lyScrollFactorPrompt );
  registerOption("Set [B]ackground", SDL_SCANCODE_B, &EditorState::lyBackgroundPrompt ); 
  registerOption("Set Background [T]ransparency", SDL_SCANCODE_T, &EditorState::lyBackgroundAlphaPrompt ); 
}

void EditorState::lyScrollFactor()
{
  Canvas::getInstance().setScrollFactor(currentLayer, StringUtil::to<float>(textEntry.contents));
}

void EditorState::lyScrollFactorPrompt()
{
  textEntry.contents = "";
  textEntry.command = std::bind(&EditorState::lyScrollFactor, this);
  textEntry.active = true;
}

void EditorState::lyBackground()
{
  if (textEntry.contents.length() == 0)
    Canvas::getInstance().setBackground(currentLayer, nullptr);
  else
    Canvas::getInstance().setBackground(currentLayer,
					ImageFactory::getInstance().getImage( "assets/scenes/" + textEntry.contents + ".png") );
}

void EditorState::lyBackgroundPrompt()
{
  textEntry.contents = "";
  textEntry.command = std::bind(&EditorState::lyBackground, this);
  textEntry.active = true;
}

void EditorState::lyBackgroundAlpha()
{
  Canvas::getInstance().setBackgroundAlpha(currentLayer, StringUtil::to<float>(textEntry.contents));
}

void EditorState::lyBackgroundAlphaPrompt()
{
  textEntry.contents = StringUtil::toString(Canvas::getInstance().getBackgroundAlpha(currentLayer));
  textEntry.command = std::bind(&EditorState::lyBackgroundAlpha, this);
  textEntry.active = true;  
}

void EditorState::switchMode_WorldSeg()
{
  mode = MODE_WORLDSEG;
  state = STATE_EDIT;
  editing = EDIT_NONE;

  clearOptions();
  optionName = "Mode: World Segments";
  //registerOption("[TAB]: to Backdrop Mode", SDL_SCANCODE_TAB, &EditorState::switchMode_Backdrop );
  registerOption("[E]xtrude Segment", SDL_SCANCODE_E, &EditorState::ws_Extrude );
  registerOption("Move [1]st point", SDL_SCANCODE_1, &EditorState::ws_Select1 );
  registerOption("Move [2]nd point", SDL_SCANCODE_2, &EditorState::ws_Select2 );
  registerOption("[Delete] point", SDL_SCANCODE_DELETE, &EditorState::ws_Delete );
}

void EditorState::ws_Extrude()
{
  Viewport &v = Viewport::getInstance();
  state = STATE_EDIT;

  Vec2f base = ws_Selected != nullptr ? (*ws_Selected)[1] : v.getMouseWorldPos();
  ws_Selected = &PhysicsManager::getInstance().addWorldSegment( base, base + Vec2f(128, 0) );
  ws_Selected_pt = 1;
}

void EditorState::ws_Select1()
{
  if (ws_Selected_pt == 0)
    ws_Selected_pt = -1;
  else ws_Selected_pt = 0;
}

void EditorState::ws_Select2()
{
  if (ws_Selected_pt == 1)
    ws_Selected_pt = -1;
  else ws_Selected_pt = 1;
}

void EditorState::ws_Delete()
{
  if (ws_Selected == nullptr) return;
  PhysicsManager::getInstance().editor_RemoveSegment(ws_Selected);
  ws_Selected = nullptr;
  ws_Selected_pt = -1;
}

void EditorState::switchMode_Backdrop()
{
  mode = MODE_BACKDROP;
  state = STATE_NONE;
  editing = EDIT_NONE;

  clearOptions();
  optionName = "Mode: Backdrops";
  //registerOption("[TAB]: to Edit Layer Mode", SDL_SCANCODE_TAB, &EditorState::switchMode_EditLayer );
  //registerOption("[C]reate", SDL_SCANCODE_C, &EditorState::bd_Create );
  registerOption("[G]rab", SDL_SCANCODE_G, &EditorState::bdMove );
  registerOption("[E]xpand", SDL_SCANCODE_E, &EditorState::bdScale );
  registerOption("[R]otate", SDL_SCANCODE_R, &EditorState::bdRotate );
  registerOption("[Delete]", SDL_SCANCODE_DELETE, &EditorState::bdDelete );
  registerOption("Push to [F]ront", SDL_SCANCODE_F, &EditorState::bdToFront );
  registerOption("Push to [B]ack", SDL_SCANCODE_B, &EditorState::bdToBack );

  registerOption("Flip [H]oriz", SDL_SCANCODE_H, &EditorState::bdFlipH );
  registerOption("Flip [V]ert", SDL_SCANCODE_V, &EditorState::bdFlipV );
  //registerOption("[J]Sink", SDL_SCANCODE_R, &EditorState::bd_Sink );
  //registerOption("[U]Raise", SDL_SCANCODE_R, &EditorState::bd_Raise );
  //registerOption("[L]Layer Sink", SDL_SCANCODE_R, &EditorState::bd_Sink );
  //registerOption("[O]Layer Raise", SDL_SCANCODE_R, &EditorState::bd_Raise );
}

void EditorState::bdMove()
{
  if (editing == EDIT_MOVE)
    editing = EDIT_NONE;
  else editing = EDIT_MOVE;
}

void EditorState::bdRotate()
{
  if (editing == EDIT_ROTATE)
    editing = EDIT_NONE;
  else editing = EDIT_ROTATE;
  bdStartAngle = bdSelected->getAngle();
}

void EditorState::bdScale()
{
  if (editing == EDIT_SCALE)
    editing = EDIT_NONE;
  else {
    editing = EDIT_SCALE;
    if (bdSelected != nullptr) {
      bdStartScaleX = bdSelected->getScaleX();
      bdStartScaleY = bdSelected->getScaleY();
    }
  }
}

void EditorState::bdFlipH()
{
  if (bdSelected != nullptr)
    bdSelected->setFlipH(!bdSelected->getFlipH());
}

void EditorState::bdFlipV()
{
  if (bdSelected != nullptr)
    bdSelected->setFlipV(!bdSelected->getFlipV());
}

void EditorState::bdDelete()
{
  auto &blist = Canvas::getInstance().getBackdropList(currentLayer);
  for (auto it = blist.begin(); it != blist.end(); it++) {
    if (&(*it) == bdSelected) {
      blist.erase(it);
      bdSelected = nullptr;
      return;
    }
  }
}

void EditorState::bdToFront()
{
  std::list<Backdrop> copy;
  auto &blist = Canvas::getInstance().getBackdropList(currentLayer);

  for (auto it = blist.begin(); it != blist.end(); it++) {
    if (&(*it) == bdSelected) {
      copy.splice(copy.begin(), blist, it);
      blist.splice(blist.end(), copy);
      //      bdSelected = nullptr;
      return;
    }
  }
}

void EditorState::bdToBack()
{
  std::list<Backdrop> copy;
  auto &blist = Canvas::getInstance().getBackdropList(currentLayer);

  for (auto it = blist.begin(); it != blist.end(); it++) {
    if (&(*it) == bdSelected) {
      copy.splice(copy.begin(), blist, it);
      blist.splice(blist.begin(), copy);
      //bdSelected = nullptr;
      return;
    }
  }
}

void EditorState::bdMoveAllUp()
{
  Canvas &c = Canvas::getInstance();
  for (Backdrop &b : c.getBackdropList(currentLayer)) {
    c.placeBackdrop( currentLayer+1, b.getImage(), b.getPosition(), b.getFrame(),
		     b.getAngle(), b.getScaleX(), b.getScaleY(), b.getFlipH(), b.getFlipV() );
  }
  c.setBackground(currentLayer+1, c.getBackground(currentLayer));
  c.setBackgroundAlpha(currentLayer+1, c.getBackgroundAlpha(currentLayer));
  c.setScrollFactor(currentLayer+1, c.getScrollFactor(currentLayer));
  c.clearLayer(currentLayer++);
  bdSelected = nullptr;
  resetMode();
}

void EditorState::bdMoveAllDown()
{
  Canvas &c = Canvas::getInstance();
  for (Backdrop &b : c.getBackdropList(currentLayer)) {
    c.placeBackdrop( currentLayer-1, b.getImage(), b.getPosition(), b.getFrame(),
		     b.getAngle(), b.getScaleX(), b.getScaleY(), b.getFlipH(), b.getFlipV() );
  }
  c.setBackground(currentLayer-1, c.getBackground(currentLayer));
  c.setBackgroundAlpha(currentLayer-1, c.getBackgroundAlpha(currentLayer));
  c.setScrollFactor(currentLayer-1, c.getScrollFactor(currentLayer));
  c.clearLayer(currentLayer--);
  bdSelected = nullptr;
  resetMode();
}

void EditorState::switchMode_EntryPoint()
{
  mode = MODE_ENTRYPOINT;
  state = STATE_NONE;
  editing = EDIT_NONE;

  clearOptions();
  optionName = "Mode: Entry Point Editor";
  registerOption("[C]reate", SDL_SCANCODE_C, &EditorState::epCreatePrompt );
  registerOption("[G]rab", SDL_SCANCODE_G, &EditorState::epGrab );
  //registerOption("[R]ename", SDL_SCANCODE_G, &EditorState::epRenamePrompt );
  registerOption("[Delete]", SDL_SCANCODE_DELETE, &EditorState::epDelete );
}

void EditorState::epCreatePrompt()
{
  textEntry.contents = "";
  textEntry.command = std::bind(&EditorState::epCreate, this);
  textEntry.active = true;
}

void EditorState::epCreate()
{
  EventManager &eventmgr = EventManager::getInstance();
  eventmgr.createEntryPoint( textEntry.contents,
			     Viewport::getInstance().getMouseWorldPos(),
			     Animation::DIR_RIGHT );
  epSelected = &eventmgr.getEntryPoint( textEntry.contents );
  epGrab();
}

void EditorState::epGrab()
{
  if (epSelected == nullptr) return;
  state = STATE_EDIT;
  editing = EDIT_MOVE;
}

void EditorState::epDelete()
{
  state = STATE_NONE;
  editing = EDIT_NONE;
  if (epSelected == nullptr) return;
  EventManager::getInstance().removeEntryPoint(epSelected->getName());
  epSelected = nullptr;
}

// Enemy stuff
void EditorState::switchMode_EnemySpawn()
{
  mode = MODE_ENEMYSPAWN;
  state = STATE_NONE;
  editing = EDIT_NONE;

  clearOptions();
  optionName = "Mode: Enemy Spawn Editor";
  registerOption("[C]reate", SDL_SCANCODE_C, &EditorState::espCreatePrompt );
  registerOption("[G]rab", SDL_SCANCODE_G, &EditorState::espGrab );
  //registerOption("[R]ename", SDL_SCANCODE_G, &EditorState::epRenamePrompt );
  registerOption("[Delete]", SDL_SCANCODE_DELETE, &EditorState::espDelete );
}

void EditorState::espCreatePrompt()
{
  textEntry.contents = espLastName;
  textEntry.command = std::bind(&EditorState::espCreate, this);
  textEntry.active = true;
}

void EditorState::espCreate()
{
  EventManager &eventmgr = EventManager::getInstance();
  espSelected = &eventmgr.createEnemySpawn( espLastName = textEntry.contents,
					    Viewport::getInstance().getMouseWorldPos() );
  espGrab();
}

void EditorState::espGrab()
{
  if (espSelected == nullptr) return;
  state = STATE_EDIT;
  editing = EDIT_MOVE;
}

void EditorState::espRenamePrompt()
{
}

void EditorState::espRename()
{
}

void EditorState::espDelete()
{
  state = STATE_NONE;
  editing = EDIT_NONE;
  if (espSelected == nullptr) return;
  std::list<EntryPoint> &list = EventManager::getInstance().editor_getEnemySpawnList();
  for (auto it = list.begin(); it != list.end(); it++) {
    if (&(*it) == espSelected) {
      list.erase(it);
      break;
    }
  }
  espSelected = nullptr;
}

void EditorState::loadScene()
{
  GameManager::getInstance().loadScene(lastScene=textEntry.contents);
  ws_Selected = nullptr;
  bdSelected = nullptr;
}

void EditorState::exportScene()
{
  Canvas &canvas = Canvas::getInstance();
  PhysicsManager &physics = PhysicsManager::getInstance();
  std::ofstream out("assets/scenes/" + (lastScene=textEntry.contents) + ".xml");
  int level = 0;
  std::stack<std::string> currentTag;
  auto fillSpaces = [&]() { for (int i = 0; i < level; i++) out << "  "; };
  auto openTag = [&](const std::string &name, const std::vector<std::pair<std::string,std::string>> &values, bool close) {
    fillSpaces();
    out << "<" << name << "";
    for (const auto &p : values)
      out << " " << p.first << "=\"" << p.second << "\"";
    if (close) out << " /";
    else {
      level++;
      currentTag.push(name);
    }
    out << ">" << std::endl;
  };
  auto closeTag = [&]() {
    level--;
    fillSpaces();
    out << "</" << currentTag.top() << ">" << std::endl;
    currentTag.pop();
  };

  out << "<?xml version = \"1.0\"?>" << std::endl;
  openTag("scene", { {"width",  StringUtil::toString(physics.getWorldWidth())},
           	     {"height", StringUtil::toString(physics.getWorldHeight())} }, false);

  for (const EntryPoint &e : EventManager::getInstance().getEntryPoints()) {
    openTag("entry", {
	{"name", e.getName()},
	{"x", StringUtil::toString(e.getPosition()[0])},
	{"y", StringUtil::toString(e.getPosition()[1])},
        {"dir", e.getDirection() == Animation::DIR_RIGHT ? "right" : "left"}}, true);
  }
  
  openTag("collision", {}, false);
  physics.editor_QueryAllSegments([&](Segment &s) {
      openTag("seg",
	    {   {"ax", StringUtil::toString(s[0][0])},
		{"ay", StringUtil::toString(s[0][1])},
		{"bx", StringUtil::toString(s[1][0])},
		{"by", StringUtil::toString(s[1][1])} }, true ); });
  closeTag();

  openTag("enemies", {}, false);
  for (const EntryPoint &e : EventManager::getInstance().getEnemySpawnList()) {
    openTag(e.getName(), {
	{"x", StringUtil::toString(e.getPosition()[0])},
	{"y", StringUtil::toString(e.getPosition()[1])},
        {"dir", e.getDirection() == Animation::DIR_RIGHT ? "right" : "left"}}, true);
  }
  closeTag();

  openTag("canvas", {}, false);
  for (int l_id : canvas.getLayerList()) {
    auto op1 = std::make_pair("id", StringUtil::toString(l_id)),
      op2 = std::make_pair("scroll", StringUtil::toString(canvas.getScrollFactor(l_id)));
    const Image *bg = canvas.getBackground(l_id);
    if (bg == nullptr)
      openTag("layer", { op1, op2 }, false);
    else
      openTag("layer", { op1, op2,
	{"background", bg->getName()},
	{"bgAlpha", StringUtil::toString(canvas.getBackgroundAlpha(l_id))}}, false);

    int setnum = 0;
    std::map<std::string, int> bdsetlist;
    for (Backdrop &b : canvas.getBackdropList(l_id)) {
      const std::string &n = b.getImage()->getName();
      if (bdsetlist.find(n) == bdsetlist.end()) {
	bdsetlist[n] = setnum;
	openTag("backdropSet", {
	    {"id", StringUtil::toString(setnum++)},
	    {"image", n} }, true);
      }
    }

    for (const auto &b : canvas.getBackdropList(l_id)) {
      openTag("bd",
	      {   {"set", StringUtil::toString(bdsetlist[b.getImage()->getName()])},
		  {"frame", StringUtil::toString(b.getFrame())},
		  {"x", StringUtil::toString((int)b.getPosition()[0])},
		  {"y", StringUtil::toString((int)b.getPosition()[1])},
		  {"scaleX", StringUtil::toString(b.getScaleX())},
		  {"scaleY", StringUtil::toString(b.getScaleY())},
		  {"angle", StringUtil::toString(b.getAngle())},
	          {"flipH", StringUtil::fromBool(b.getFlipH())},
		  {"flipV", StringUtil::fromBool(b.getFlipV())} }, true );
    }
    
    closeTag();
  }
  closeTag();
  
  closeTag();
}
