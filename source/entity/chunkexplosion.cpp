#include "chunkexplosion.h"

#include "../gameconfig.h"
#include "../physicsmanager.h"
#include "../image.h"

ChunkExplosion::ChunkExplosion() : image(nullptr), frame(0), freeChunks(), activeChunks()
{
  int total = GameConfig::getInstance()["chunkSplits"].toInt();
  total = total*total;
  while (total-- > 0) freeChunks.push_back(new Chunk());
}

ChunkExplosion::~ChunkExplosion()
{
  for (Chunk *c : freeChunks) delete c;
  for (Chunk *c : activeChunks) delete c;
}

void ChunkExplosion::spawn(const Image *img, int f, const Vec2f &position, const Vec2f &velocity)
{
  image = img;
  frame = f;

  //int cx = image->getFrameCenterX(f),
  //  cy = image->getFrameCenterY(f);;

  int splitSize = GameConfig ::getInstance()["chunkSplits"].toInt();
  float secSize = 1.f/splitSize;

  float xspeed = 1000, yspeed = 1300;

  for (float x = 0; x < 1.f; x += secSize) {
    for (float y = 0; y < 1.f; y += secSize) {
      freeChunks.front()->reset( image, frame, position,
				 Vec2f((x-.5f)*(xspeed + drand48()*xspeed), (y-.75f)*(yspeed+drand48()*yspeed)) + velocity, -1000+drand48()*2000,
				 .5f + drand48()*.5,
			       x, y, secSize, secSize);
      activeChunks.push_front(freeChunks.front());
      freeChunks.pop_front();
    }
  }
}

void ChunkExplosion::update(float delta)
{
  for (auto it = activeChunks.begin(); it != activeChunks.end();) {
    Chunk *c = *it;
    if (!c->isAlive()) {
      it = activeChunks.erase(it);
      freeChunks.push_front(c);
      continue;
    }
    c->update(delta);
    ++it;
  }
}

void ChunkExplosion::draw(float scrollFactor) const
{
  for (Chunk *c : activeChunks) {
    c->draw(scrollFactor);
  }
}

bool ChunkExplosion::isAlive() const { return activeChunks.size() > 0; }

void Chunk::draw(float scroll) const
{
  image->drawChunk(position[0], position[1], frame,
  		   sx, sy, sw, sh, std::max(life,0.f), scroll);
  //image->draw(position[0], position[1], frame, scroll);
}

void Chunk::update(float delta)
{
  life -= delta * decaySpeed;
  velocity += Vec2f(0,1) * PhysicsManager::GRAVITY * delta;

  Vec2f pos = position + Vec2f(image->getFrameCenterX(frame) + sx * image->getFrameWidth(frame),
			       image->getFrameCenterY(frame) + sy * image->getFrameHeight(frame));

  Vec2f dir = velocity * delta;

  auto result = PhysicsManager::getInstance().rayCast(PhysicsManager::MASK_WORLD,
						      pos, pos+dir);
  if (result.hit)
    dir = (velocity = velocity.reflect(result.normal)*.5f) * delta;
  
  position += dir * .5;
}

 void Chunk::reset(const Image *img, int f, const Vec2f &pos, const Vec2f &vel, float av, float ds, float x, float y, float w, float h)
{
  image = img;
  frame = f;
  position = pos;
  velocity = vel;
  life = 1.f;
  decaySpeed = ds;
  angVel = av;
  angle = 0.f;
  sx = x;
  sy = y;
  sw = w;
  sh = h;
}
