#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include "vector2.h"

#include <list>

class Image;

// This is just a temporary system anyway. When I make the real game, I'll
// use OpenGL or Vulkan or something and write my own light shaders (I think)
class SDL_Texture;

class Light
{
 public:
  Light(const Vec2f &pos, float s, float i);
  ~Light();

  // Draws the "lightmap" onto the "darkness" texture
  void draw() const;

  void setPosition(const Vec2f &pos) { position = pos; }
  void setSize(float s) { size = s; }
  void setIntensity(float i) { intensity = i; }

  const Vec2f &getPosition() const { return position; }
  float getSize() const { return size; }
  float getIntensity() const { return intensity; }

  void kill() { dead = true; }
  bool isDead() const { return dead; }

  Light() = delete;
  Light(const Light&) = delete;
  Light &operator=(const Light&) = delete;
  
 private:
  const Image *image;
  Vec2f position;
  float size, intensity;
  bool dead;
};

class LightManager
{
 public:
  ~LightManager();

  static LightManager &getInstance();

  // Makes sure all lights are where they should be and dead lights are removed
  void update();

  // Sets as render target, renders all the lights, switches rendered back to normal, then renders darkness in appropriate locations
  void draw() const;

  Light *addLight(const Vec2f &pos, float s, float i = 1.f);

  void setAmbience(float v) { ambience = v; }
  float getAmbience() const { return ambience; }

  const Image *getLightImage() const { return lightImage; }

  LightManager(const LightManager&) = delete;
  LightManager &operator=(const LightManager&) = delete;  
  
 private:
  LightManager();

  SDL_Texture *darknessImage;
  const Image *lightImage;
  std::list<Light> lights;
  float ambience;
};

#endif
