#include <SDL_image.h>
#include <iostream>
#include <fstream>

#include "imagefactory.h"
#include "image.h"
#include "rendercontext.h"
#include "iomod.h"

ImageFactory& ImageFactory::getInstance() {
  static ImageFactory instance;
  return instance;
}

ImageFactory::~ImageFactory() {
  std::cout << "Deleting images in Factory" << std::endl;

  // Free single image containers

  // Surfaces
  std::map<std::string, SDL_Surface*>::iterator si = surfaces.begin();
  while (si != surfaces.end()) SDL_FreeSurface((si++)->second);

  // Textures
  std::map<std::string, SDL_Texture*>::iterator ti = textures.begin();
  while (ti != textures.end()) SDL_DestroyTexture((ti++)->second);

  // Images
  std::map<std::string, Image*>::iterator fi = images.begin();
  while (fi != images.end()) {
    std::cout << "deleting " << fi->first << std::endl;
    delete fi->second; ++fi;
  }
}

Image* ImageFactory::getImage(const std::string& name)
{
  std::map<std::string, Image*>::const_iterator it = images.find(name); 
  if ( it == images.end() ) {

    std::cout << "Loading image '" << name << "'" << std::endl;

    SDL_Surface * const surface = IoMod::getInstance().readSurface(name);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(
      RenderContext::getInstance().getRenderer(), surface);
    
    Image * const image = new Image(name, surface, texture);

    surfaces[name] = surface;
    textures[name] = texture;
    images[name] = image;

    // Try to find a frame file for the image
    IoMod::getInstance().readFrameData(name, image->frames);

    return image;
  }
  else
    return it->second;
}
