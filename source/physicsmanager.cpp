#include "physicsmanager.h"
#include "gamemanager.h"
#include "stringutil.h"

#include <iostream>
#include <map>

PhysicsManager::PhysicsManager() : width(0), height(0), grid(), entityIndex() {}

PhysicsManager &PhysicsManager::getInstance()
{
  static PhysicsManager instance;
  return instance;
}

void PhysicsManager::updateEntityList()
{
  for (int gridPos = 0; gridPos < (width/GRID_SIZE)*(height/GRID_SIZE); gridPos++) {
    auto &list = grid[gridPos].entities;
    for (auto it = list.begin(); it != list.end();) {
      Entity *e = *it;

      // First, find out if they're dead.
      if (!e->isAlive() || !e->isActive()) {
	it = list.erase(it);
	continue;
      }

      // Next, make sure they all lie within the right grid box
      int ex = (int)(e->getPosition()[0])/GRID_SIZE,
	ey = (int)(e->getPosition()[1])/GRID_SIZE;
      int ePos = ex + ey*(width/GRID_SIZE);
      
      if (ePos != gridPos) {
	entityIndex[(unsigned long)e] = ePos;
	grid[ePos].entities.push_back(e);
	it = list.erase(it);
      }
      else ++it;
    }
  }
}

void PhysicsManager::testEntityCollisions()
{
  // Test all entity intersections in ~O(N log N) time!
  
  std::list<Entity*> testList;
  for (GridBox &b : grid) std::for_each(b.entities.begin(), b.entities.end(), [&](Entity*e) { testList.push_back(e); });
  if (testList.size() == 0) return;
  testList.sort([&](Entity*a, Entity*b) { return a->getBoundingBox()[0] < b->getBoundingBox()[0]; } );
  std::map<Entity*, std::list<Entity*>> xIntersections;

  Entity *currentX = nullptr;
  for (Entity *e : testList) {
    if (currentX == nullptr) currentX = e;
    if (e == currentX) continue;

    auto &list = xIntersections[currentX];

    for (Entity *ce : list)
      if (e->getBoundingBox()[0] < ce->getBoundingBox()[2])
	xIntersections[ce].push_back(e);

    if (e->getBoundingBox()[0] < currentX->getBoundingBox()[2]) {
      list.push_back(e);
      
    } else currentX = e;
  }

  int intersections = 0;

  for (auto &it : xIntersections) {
    for (Entity *e : it.second) {
      // If one was killed during the collision process, stop detecting them!
      if (!it.first->isAlive()) break;
      if (!e->isAlive()) continue;
      
      if (e->getBoundingBox()[1] < it.first->getBoundingBox()[3] &&
	  it.first->getBoundingBox()[1] < e->getBoundingBox()[3]) {
	intersections++;
	it.first->registerEntityCollision(e);
	e->registerEntityCollision(it.first);
      }
    }
  }

  GameManager::getInstance().setDebugMessage(3, "Entity Intersections: " + StringUtil::toString(intersections));
}

PhysicsManager::RayResult PhysicsManager::rayCast(int mask, const Vec2f &a, const Vec2f &b)
{
  RayResult result;

  (void)mask; (void)a; (void)b;

  Vec2f center( (a+b)*.5f ),
    hdim( fabs(a[0]-b[0])*.5f, fabs(a[1]-b[1])*.5f );

  if (mask & MASK_WORLD) {
    querySegmentGridArea(center, 1, [&](const Segment &s) {
      Vec2f normal = s[0]-s[1];
      normal = Vec2f(-normal[1], normal[0]);

      if (normal.dot(a-b) < 0)
	return;

      auto ray = raySegmentIntersect(a, b, s[0], s[1]);
		
      if (ray.first >= 0.f && ray.first < result.t && ray.second >= 0.f && ray.second <= 1.f) {
	result.hit = true;
	result.t = ray.first;
	//result.u = ray.second;
	result.normal = normal.normalize();
	result.c = s[0];
	result.d = s[1];
      }
    } );
  }

  return result;
}

PhysicsManager::RayResult PhysicsManager::multiPlaneCast(int mask, const Vec2f &dir, const std::vector<Vec2f> &points)
{
  RayResult result;

  (void)mask; (void)dir; (void)points;

  Vec2f min(10000,10000), max(-10000,-10000), center, hdim;

  {
    size_t s = points.size();
    std::vector<Vec2f> pts;
    pts.reserve(s*2);
    for (size_t i = 0; i < s; i++)
      pts.emplace_back(points[i]);
    for (size_t i = 0; i < s; i++)
      pts.emplace_back(points[i] + dir);
    for (const Vec2f& p : pts) {
      min[0] = std::min(min[0], p[0]);
      min[1] = std::min(min[1], p[1]);
      max[0] = std::max(max[0], p[0]);
      max[1] = std::max(max[1], p[1]);
    }
    min[0] -= 1;
    min[1] -= 1;
    max[0] += 1;
    max[1] += 1;
    center = (min + max) * .5f;
    hdim = (max - min);
  }

  /*int xmin = center[0]/GRID_SIZE - 1,
    ymin = center[1]/GRID_SIZE - 1,
    xmax = xmin+2,
    ymax = ymin+2;

  xmin = std::max(xmin, 0);
  ymin = std::max(ymin, 0);
  xmax = std::min(xmax, width/GRID_SIZE);
  ymax = std::min(ymax, height/GRID_SIZE);*/

  // If we are to detect collisions for backdrops, do so
  if (mask & MASK_WORLD) {    
    querySegmentGridArea( center, 1, [&](const Segment &s) {
      
	// Now test to make sure there is a potential intersection between the
      // planecast and the current edge. If not, skip it.
      Vec2f cd_center( (s[0]+s[1])*.5f ),
	cd_hdim( fabs(s[0][0]-s[1][0])*.5f, fabs(s[0][1]-s[1][1])*.5f );
      if (!boxIntersection( center, hdim, cd_center, cd_hdim ))
	return;

      // Determine the direction the wall plane is facing, and cancel the check if it's
      // facing the outside direction
      Vec2f normal = s[0]-s[1];
      normal = Vec2f(-normal[1], normal[0]).normalize();
      if (normal.dot(dir) > 0) return;

      // Do a normal raycast from all the corners
      for (const Vec2f &p : points) {
	std::pair<float, float> ray = raySegmentIntersect( p, p+dir, s[0], s[1]);

	// If it turns out to be the current best result, use it
	if (ray.first < result.t &&
	    ray.second >= 0.f && ray.second <= 1.f &&
	    ray.first < 1.f && ray.first >= 0.f) {
	  result.t = ray.first;
	  //result.u = ray.second;
	  result.normal = normal;
	  result.c = s[0];
	  result.d = s[1];
	  result.corner = false;
	  result.hit = true;
	  //result.cornerData.resize(0);
	}
      }

      // Now check to see if any points are embedded inside the area that
      // the box tries to pass through as it attempts to move
      for (size_t p_id = 0; p_id < points.size()-1; ++p_id) {
	  
	const Vec2f &p1 = points[p_id];
	const Vec2f &p2 = points[p_id+1];

	// Function for detecting closest ray result for points inside the poly
	auto inBox = [&](const Vec2f &p) {
	  if (pointWithin(p, p1, p1+dir, p2+dir, p2)) {
	    // Make the fake wall really wide
	    auto ray = raySegmentIntersect( p, p - dir, p1, p2 );
	    if (ray.first < result.t) {
	      result.normal = (p1-p2);

	      // Create a fake plane where if left the corner would no longer matter
	      // Sorry for not making sense. It makes sense to me. I can't explain.
	      result.c = p + result.normal * .5f;
	      result.d = p - result.normal * .5f;
		
	      result.t = ray.first;
	      //result.u = ray.second;
	      result.normal = Vec2f(-result.normal[1], result.normal[0]).normalize();
	      result.corner = true;
	      result.hit = true;
	    }
	  }
	};

	// Check points c and d
	inBox(s[0]); inBox(s[1]);
      }
    } ); // end querySegmentGridArea
  }

  return result;
}

bool PhysicsManager::boxIntersection( const Vec2f &box1c, const Vec2f &box1h,
				    const Vec2f &box2c, const Vec2f &box2h )
{
  return (fabs(box1c[0] - box2c[0]) < (box1h[0] + box2h[0])) &&
         (fabs(box1c[1] - box2c[1]) < (box1h[1] + box2h[1]));
}

bool PhysicsManager::boxIntersection( const Vec2f &boxc, const Vec2f &boxh,
				    const Vec2f &point )
{
  return (fabs(boxc[0] - point[0]) < (boxh[0])) &&
         (fabs(boxc[1] - point[1]) < (boxh[1]));
}

bool PhysicsManager::boxCircleIntersection( const BoundingBox &bb, const Vec2f &cpos, float crad )
{  
  Vec2f a = Vec2f(bb[0], bb[1]),
    b = Vec2f(bb[2], bb[1]),
    c = Vec2f(bb[2], bb[3]),
    d = Vec2f(bb[0], bb[3]);
  
  if (pointWithin(cpos, a + Vec2f(-crad,-crad), b + Vec2f(crad,-crad), c + Vec2f(crad,crad), d + Vec2f(-crad,crad))) {
    if (pointWithin(cpos, a, b, c, d))
      return true;
    if ((a-cpos).length() < crad ||
	(b-cpos).length() < crad ||
	(c-cpos).length() < crad ||
	(d-cpos).length() < crad)
      return true;
  }

  return false;
}

std::pair<float,float> PhysicsManager::raySegmentIntersect(Vec2f a, Vec2f b, Vec2f c, Vec2f d)
{
  Vec2f r = b-a;
  Vec2f s = d-c;

  float rxs = r.cross(s);
  Vec2f cma = c-a;
  
  float t = cma.cross(s) / rxs;
  float u = cma.cross(r) / rxs;
  
  return std::make_pair(t, u);
}

bool PhysicsManager::pointWithin(const Vec2f &p, const Vec2f &a, const Vec2f &b, const Vec2f &c, const Vec2f &d)
{
  float x1 = (a-d).cross(a-p);
  float x2 = (b-a).cross(b-p);
  float x3 = (c-b).cross(c-p);
  float x4 = (d-c).cross(d-p);

  if ((x1 < 0.f && x2 < 0.f && x3 < 0.f && x4 < 0.f) ||
      (x1 > 0.f && x2 > 0.f && x3 > 0.f && x4 > 0.f) ) {
    return true;
  }

  return false;
}

Vec2f PhysicsManager::rayStop( const Vec2f &direction, const Vec2f &normal, float distFactor )
{
  if (direction.length() <= PhysicsManager::EPSILON)
    return Vec2f(0,0);
  Vec2f result = direction * distFactor + normal * EPSILON;
  //return result;
  return result.length() > PhysicsManager::EPSILON ? result : Vec2f(0,0);
}

Vec2f PhysicsManager::raySlide( const Vec2f &direction, const Vec2f &normal, float distFactor )
{
  if (direction.length() <= PhysicsManager::EPSILON)
    return Vec2f(0,0);
  Vec2f result = direction + normal * ( (1.f-distFactor) * direction.dot(-normal) + EPSILON );
  //return result;
  return result.length() > PhysicsManager::EPSILON ? result : Vec2f(0,0);
}

Segment &PhysicsManager::addWorldSegment(const Vec2f &a, const Vec2f &b) {
  Vec2f center = (a+b)*.5f;
  int x = center[0]/GRID_SIZE;
  int y = center[1]/GRID_SIZE;
  int gridPos = x + y * (width/GRID_SIZE);
  auto &list = grid[gridPos].worldSegments;//getSegmentList((a+b)*.5f);
  list.emplace_back(a, b);
  return list.back();
}

void PhysicsManager::editor_UpdateSegmentList()
{
  for (int gridPos = 0; gridPos < (width/GRID_SIZE)*(height/GRID_SIZE); gridPos++) {
    auto &list = grid[gridPos].worldSegments;
    for (auto it = list.begin(); it != list.end();) {
      Segment &s = *it;
      int sx = s.getCenter()[0]/GRID_SIZE,
	sy = s.getCenter()[1]/GRID_SIZE;
      int seggrid = sx + sy*(width/GRID_SIZE);
      if (seggrid != gridPos) {
	addWorldSegment(s[0], s[1]);
	it = list.erase(it);
      } else ++it;
    }
  } 
}
