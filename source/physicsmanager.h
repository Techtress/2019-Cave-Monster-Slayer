#ifndef PHYSICS_MANAGER
#define PHYSICS_MANAGER

#include <vector>
#include <unordered_map>
#include <list>
#include <functional>
#include <algorithm>

#include "segment.h"
#include "boundingbox.h"
#include "entity.h"

class PhysicsManager
{
 public:
  static PhysicsManager &getInstance();

  enum TypeMask
  {
    MASK_WORLD = 1<<0,
    MASK_PLAYER = 1<<1,
    MASK_ENEMY = 1<<2,
    MASK_ALL = ~0
  };

  struct RayResult
  {
    RayResult() : t(1.f), c(Vec2f(0,0)), d(Vec2f(0,0)), normal(Vec2f(0,0)), hit(false), corner(false) {}
    float t;
    Vec2f c, d, normal;
    bool hit, corner;
  };

  void resizeWorld( int w, int h ) {
    width = w; height = h;
    grid.resize( (w/GRID_SIZE)*(h/GRID_SIZE) );
  }
  int getWorldWidth() const { return width; }
  int getWorldHeight() const { return height; }

  void clearWorld() { grid.clear(); }

  Segment &addWorldSegment(const Vec2f &a, const Vec2f &b);

  void registerEntity(Entity* e) {
    int gridPos = getGridPos(e->getPosition());
    grid[gridPos].entities.push_back(e);
    entityIndex[(unsigned long)e] = gridPos; }

  void unregisterEntity(Entity* e) {
    auto it = entityIndex.find((unsigned long)e);
    grid[it->second].entities.remove(e);
    entityIndex.erase(it); }

  // Calls a funcrion for every segment in the specified range on the world grid.
  void querySegmentGridArea( const Vec2f &position, int range, std::function<void(const Segment&)> &&f ) {
    queryGridRange(position, range, [&](GridBox& b) {
	std::for_each(b.worldSegments.begin(), b.worldSegments.end(), f); }); }

  // Places entities in the right grid for queries
  void updateEntityList();
  void queryEntityGridArea( const Vec2f &position, int range, int mask, std::function<void(Entity*)> &&f ) {
    queryGridRange(position, range, [&](GridBox& b) {
	for (Entity *e : b.entities) { if (e->isAlive() && e->getMask()&mask) f(e); } }); }

  // remove stuff
  // void clearScene();
  // void removeWorldSegment(Segment*); // level editor

  static constexpr float EPSILON = 0.1f;
  static constexpr float GRAVITY = 5000.f;

  void testEntityCollisions();

  // Casts a ray. Resulting vector is the new position and normal of hit.
  RayResult rayCast(int mask, const Vec2f& a, const Vec2f& b);

  // Casts "planes" originating between the specified points
  // Used for bounding box collision
  RayResult multiPlaneCast(int mask, const Vec2f &dir, const std::vector<Vec2f> &points);

  // Generic collision tests
  static bool boxIntersection( const Vec2f &box1c, const Vec2f &box1h,
			       const Vec2f &box2c, const Vec2f &box2h );
  static bool boxIntersection( const Vec2f &boxc, const Vec2f &boxd, const Vec2f &point );
  static std::pair<float,float> raySegmentIntersect(Vec2f a, Vec2f b, Vec2f c, Vec2f d);
  static bool pointWithin(const Vec2f &p, const Vec2f &a, const Vec2f &b, const Vec2f &c, const Vec2f &d);

  static bool boxCircleIntersection( const BoundingBox&, const Vec2f &cpos, float crad);

  // Generic collision calculations
  static Vec2f rayStop( const Vec2f &direction, const Vec2f &normal, float distFactor );
  static Vec2f raySlide( const Vec2f &direction, const Vec2f &normal, float distFactor );

  PhysicsManager(const PhysicsManager&) = delete;
  PhysicsManager &operator=(const PhysicsManager&) = delete;

  // Only the level editor should use this
  void editor_QueryAllSegments(std::function<void(Segment&s)> &&f) {
    for (auto &b : grid) std::for_each(b.worldSegments.begin(), b.worldSegments.end(), f); }
  void editor_RemoveSegment(Segment *s) {
    for (auto &b : grid) {
      for (auto it = b.worldSegments.begin(); it != b.worldSegments.end(); ++it) {
	if (&(*it) == s) { it = b.worldSegments.erase(it); return; } } } }
  void editor_UpdateSegmentList();
  
 private:
  PhysicsManager();

  // For running into things...
  const static int GRID_SIZE = 512;

  // World dimensions
  int width, height;

  // Collision segments of the background
  struct GridBox
  {
  GridBox() : worldSegments(), entities() {}
    std::list<Segment> worldSegments;
    std::list<Entity*> entities;
  };
  
  std::vector< GridBox > grid;
  std::unordered_map<unsigned long, int> entityIndex;

  int getGridPos( const Vec2f &position ) {
    int x = position[0]/GRID_SIZE, y = position[1]/GRID_SIZE;
    return x + y * (width/GRID_SIZE);
  }

  std::list<Segment> &getSegmentList(const Vec2f &position) {
    return grid[ getGridPos(position) ].worldSegments;
  }

  void queryGridRange( const Vec2f &position, int range, std::function<void(GridBox&)> &&function ) {
    int xmin = position[0]/GRID_SIZE - range,
    ymin = position[1]/GRID_SIZE - range,
    xmax = xmin+range*2,
    ymax = ymin+range*2;

    xmin = std::max(xmin, 0);
    ymin = std::max(ymin, 0);
    xmax = std::min(xmax, (width/GRID_SIZE)-1);
    ymax = std::min(ymax, (height/GRID_SIZE)-1);

    for (int x = xmin; x <= xmax; x++) {
      for (int y = ymin; y <= ymax; y++) {
	function(grid[x + y * (width/GRID_SIZE)]);
      }
    }
  }
};

#endif
