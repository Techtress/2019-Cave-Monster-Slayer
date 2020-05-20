#ifndef CHUNKEXPLOSION_H
#define CHUNKEXPLOSION_H

/*
 * This is kind of hacked in here. I don't think I really plan to keep this
 * for the game, but
 */

#include "../vector2.h"

#include <list>

class Image;

class Chunk
{
 public:
 Chunk() : image(nullptr), frame(0), life(0.f), decaySpeed(0.f), position(), velocity(), angle(0.f), angVel(0.f), sx(0.f), sy(0.f), sw(0.f), sh(0.f) {}

  void update(float delta);
  void reset(const Image*, int f, const Vec2f &pos, const Vec2f &vel, float av, float ds, float x, float y, float w, float h);
  void draw(float scroll) const;

  bool isAlive() { return life > 0.f; }
  
 private:
  const Image *image;
  int frame;
  float life;
  float decaySpeed;
  Vec2f position;
  Vec2f velocity;
  float angle;
  float angVel;
  float sx, sy, sw, sh;
};

class ChunkExplosion
{
 public:
  ChunkExplosion();
  ~ChunkExplosion();

  void spawn(const Image*, int f, const Vec2f &pos, const Vec2f &velocity);

  void draw(float scrollFactor) const;
  void update(float delta);

  bool isAlive() const;

  ChunkExplosion(const ChunkExplosion&) = delete;
  ChunkExplosion &operator=(const ChunkExplosion&) = delete;

 private:
  const Image *image; int frame;
  std::list<Chunk*> freeChunks, activeChunks;
};


#endif
