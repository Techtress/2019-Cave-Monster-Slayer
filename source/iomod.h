#ifndef IOMOD_H
#define IOMOD_H

#include <string>
#include <vector>
#include <SDL.h>

#include "image.h"

class GameConfig;

typedef struct _TTF_Font TTF_Font;

class IoMod
{
 public:
  static IoMod& getInstance();
  
  ~IoMod();
  
  SDL_Texture* readTexture(const std::string& filename);
  SDL_Surface* readSurface(const std::string& filename);

  void readFrameData(const std::string&, std::vector<Image::Frame>&);

  void writeText(const std::string&, int, int) const;
  void writeText(const std::string&, int, int, const SDL_Color &color) const;
  SDL_Renderer* getRenderer() const { return renderer; }

  IoMod(const IoMod&) = delete;
  IoMod& operator=(const IoMod&) = delete;
  
 private:
  const GameConfig &cfg;
  int init;
  SDL_Renderer* renderer;
  TTF_Font* font;
  SDL_Color textColor;

  IoMod();
};

#endif
