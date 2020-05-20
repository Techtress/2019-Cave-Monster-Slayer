#include "lightmanager.h"
#include "image.h"
#include "imagefactory.h"
#include "rendercontext.h"
#include "viewport.h"

#include <SDL.h>

LightManager::LightManager() :
  darknessImage(nullptr),
  lightImage(ImageFactory::getInstance().getImage("assets/effects/light.png")),
  lights(),
  ambience(0.f)
{
  darknessImage = SDL_CreateTexture( RenderContext::getInstance().getRenderer(),
				     SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
				     Viewport::getInstance().getWidth(), Viewport::getInstance().getHeight() );
  SDL_SetTextureBlendMode(darknessImage, SDL_BLENDMODE_MOD);
  SDL_SetTextureBlendMode(lightImage->getTexture(), SDL_BLENDMODE_ADD);
}

LightManager::~LightManager()
{
  SDL_DestroyTexture(darknessImage);
}

LightManager &LightManager::getInstance()
{
  static LightManager instance;
  return instance;
}

Light *LightManager::addLight( const Vec2f &pos, float s, float i )
{
  lights.emplace_back(pos, s, i);
  return &lights.back();
}

void LightManager::update()
{
  for (auto it = lights.begin(); it != lights.end();) {
    if (it->isDead()) it = lights.erase(it);
    else ++it;
  }
}

void LightManager::draw() const
{
  SDL_Renderer *renderer = RenderContext::getInstance().getRenderer();
  SDL_SetRenderTarget( renderer, darknessImage );
  Uint8 av = ambience*255;

  // First, render the ambient color all over the screen
  SDL_SetRenderDrawColor( renderer, av, av, av, 255 );
  SDL_RenderClear( renderer );

  // Set the light texture's intensity to the "anti-ambience" so that
  // it doesn't have an ugly solid circle by trying to make it too
  // bright and failing miserably.
  Uint8 nav = 255-av;
  SDL_SetTextureColorMod( lightImage->getTexture(), nav, nav, nav );

  // Now render each individual light...
  for (const Light &l : lights) l.draw();

  SDL_SetRenderTarget( renderer, nullptr );

  SDL_Rect screen = {0, 0, Viewport::getInstance().getWidth(), Viewport::getInstance().getHeight()};
  SDL_RenderCopy( renderer, darknessImage, &screen, &screen );
}

Light::Light(const Vec2f &pos, float s, float i) :
  image( LightManager::getInstance().getLightImage() ),
  position(pos),
  size(s),
  intensity(i),
  dead(false)
{
  // Should be square anyway...
  size /= image->getWidth();
}

Light::~Light() {}

void Light::draw() const
{
  Uint8 v;
  SDL_GetTextureColorMod( image->getTexture(), &v, &v, &v );
  Uint8 newv = v * intensity;
  SDL_SetTextureColorMod( image->getTexture(), newv, newv, newv );
  float half = image->getWidth()*size*.5f;
  image->draw( position[0] - half, position[1] - half, 1.f, size, size );
  SDL_SetTextureColorMod( image->getTexture(), v, v, v );
}
