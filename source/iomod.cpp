#include <SDL_image.h>

#include "iomod.h"
#include "gameconfig.h"
#include "rendercontext.h"

#include <iostream>
#include <fstream>
#include <SDL_ttf.h>

IoMod& IoMod::getInstance() {
  static IoMod instance;
  return instance;
}

IoMod::~IoMod() { 
  TTF_CloseFont(font);
  TTF_Quit(); 
}

IoMod::IoMod() :
  cfg(GameConfig::getInstance()),
  init(TTF_Init()),
  renderer( RenderContext::getInstance().getRenderer() ),
  font(TTF_OpenFont(cfg["font"]["file"].toStr().c_str(),
                    cfg["font"]["size"].toInt())),
  textColor({0xff, 0, 0, 0})
{
  if ( init == -1 ) {
    throw std::string("error: Couldn't init font");
  }
  if (font == NULL) {
    throw std::string("error: font not found");
  }
  textColor.r = cfg["font"]["red"].toInt();
  textColor.g = cfg["font"]["green"].toInt();
  textColor.b = cfg["font"]["blue"].toInt();
  textColor.a = cfg["font"]["alpha"].toInt();
}

SDL_Texture* IoMod::readTexture(const std::string& filename)
{
  SDL_Texture *texture = IMG_LoadTexture(renderer, filename.c_str());
  if ( texture == NULL ) {
    throw std::string("Couldn't load ") + filename;
  }
  return texture;
}

SDL_Surface* IoMod::readSurface(const std::string& filename)
{
  SDL_Surface *surface = IMG_Load(filename.c_str());
  if ( !surface ) {
    throw std::string("Couldn't load ") + filename;
  }
  return surface;
}

void IoMod::readFrameData(const std::string &name, std::vector<Image::Frame> &data)
{
  std::ifstream fin(name.substr(0, name.length()-3) + "frame", std::ios::in | std::ios::binary);
  if (fin.is_open()) {
    unsigned short count;
    fin.read((char*)&count, sizeof(unsigned short));
    data.resize(count);
    for (unsigned short i = 0; i < count; i++) {
      fin.read((char*)&data[i], sizeof(unsigned short)*6);

      // Read shape data
      unsigned vertexCount;
      fin.read((char*)&vertexCount, sizeof(unsigned));

      if (vertexCount > 0) {
	data[i].vertices.resize(vertexCount);
	float left = 100000.f, right = 0.f, top = 100000.f, bottom = 0.f;
	for (unsigned v = 0; v < vertexCount; v++) {
	  float x, y;
	  fin.read((char*)&x, sizeof(float));
	  fin.read((char*)&y, sizeof(float));

	  data[i].vertices[v][0] = x;
	  data[i].vertices[v][1] = y;

	  data[i].vbounds.first += data[i].vertices[v];

	  left   = std::min(left, x);
	  right  = std::max(right, x);
	  top    = std::min(top, y);
	  bottom = std::max(bottom, y);
	}
	data[i].vbounds.first = data[i].vbounds.first / vertexCount;
	data[i].vbounds.second = Vec2f(right-left, bottom-top) * .5f;
      }
    }

    /*std::cout << "  Frames: " << data.size() << std::endl;
    for (Image::Frame &f : data)
      std::cout << "    {"
		<< f.x << ", "
		<< f.y << ", "
		<< f.w << ", "
		<< f.h << ", "
		<< f.ox << ", "
		<< f.oy << "}" << std::endl;*/
  }
}

void IoMod::writeText(const std::string& msg, int x, int y) const
{
  writeText(msg, x, y, textColor);
}

void IoMod::writeText(const std::string& msg, int x, int y, const SDL_Color &color) const {
  SDL_Surface* surface = 
    TTF_RenderText_Solid(font, msg.c_str(), color);

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

  int textWidth = surface->w;
  int textHeight = surface->h;
  SDL_FreeSurface(surface);
  SDL_Rect dst = {x, y, textWidth, textHeight};

  SDL_RenderCopy(renderer, texture, NULL, &dst);
  SDL_DestroyTexture(texture);
}
