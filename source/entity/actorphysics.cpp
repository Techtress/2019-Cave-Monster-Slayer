#include "actorphysics.h"

#include "../physicsmanager.h"
#include "../entity.h"

#include <limits>
#include <cmath>

const Vec2f UP_VECTOR = Vec2f(0, -1);
const Vec2f DOWN_VECTOR = Vec2f(0, 1);
const Vec2f RIGHT_VECTOR = Vec2f(1,0);
const float MAX_SLOPE = 0.7;
const float FALL_SPEED = 8000;

float angle = 0.f;

ActorPhysics::ActorPhysics(Entity *o) :
  owner(o),
  model(nullptr),
  state(STATE_AIR),
  visibleState(state),
  ledgeState(LEDGE_NONE),
  wallState(WALL_NONE),
  velocity(Vec2f(0,0)),
  disabledTime(0.f),
  spriteOffsetVelocity(0.f),
  spriteFakeState(SPRITE_NONE),
  movement(0.f),
  ground() {}

ActorPhysics::~ActorPhysics() {}

void ActorPhysics::activate(const ActorPhysicsModel *newModel)
{
  model = newModel;
  visibleState = state = STATE_AIR;
  velocity = Vec2f(0,0);
  spriteOffsetVelocity = 0.f;
  spriteFakeState = SPRITE_NONE;
  movement = 0.f;
  ground = GroundData();
  
  owner->setOffsetY(0);
}

void ActorPhysics::update(float delta)
{
  const float EPSILON = PhysicsManager::EPSILON;
  PhysicsManager &physics = PhysicsManager::getInstance();
  const Vec2f &pos = owner->getPosition();
  const ActorPhysicsModel &m = *model;

  wallState = WALL_NONE;
  
  // Directional influence vector. Influenced by velocity, collisions, and angle of the ground.
  Vec2f dir;

  // These flags will help us determine which points/sides of the box to case rays from
  bool vel_r, vel_l, vel_u, vel_d, vel_straight;

  // Calculates boolean values based on the direction vector for up, down,
  // left, and right. Also sets an axis of the direction vector to 0 if
  // it is less than epsilon.
  auto setVelFlags = [&vel_r, &vel_l, &vel_u, &vel_d, &vel_straight, &dir]() {
    float e = PhysicsManager::EPSILON;//std::numeric_limits<float>::min();
    vel_r = dir[0] > e;
    vel_l = dir[0] < -e;
    vel_u = dir[1] < -e;
    vel_d = dir[1] > e;

    // Are we moving in a straight line?
    vel_straight = (vel_r || vel_l) ^ (vel_u || vel_d);

    // If we aren't really moving, don't let dir influence anything less
    // than the epsilon
    if (!vel_r && !vel_l) dir[0] = 0.f;
    if (!vel_u && !vel_d) dir[1] = 0.f;

    return vel_r || vel_l || vel_u || vel_d;
  };

  // Perform a bounding box collision check. Returns the result from the
  // ray cast.
  auto collisionCheck = [&vel_r, &vel_l, &vel_u, &vel_d, &vel_straight, &dir, &pos, &physics, this]( const std::vector<Vec2f> &corners ) {
    
    // Determine the number and order of points of the box corners we should use for
    // the multi plane cast for collision detection.
    // For some reason we travel in a counter-clockwise direction...
    int count = 3-vel_straight;
    int start = (vel_d && !vel_l) + (vel_r && !vel_d)*2 + (vel_u && !vel_r)*3;
    std::vector<Vec2f> points(count);
    for (int i = 0; i < count; i++)
      points[i] = pos + corners[ (start+i)%4 ];

    // Now, case only the corners of the box that are not obstructed by the box
    // itself while moving.
    return physics.multiPlaneCast( PhysicsManager::MASK_WORLD, dir, points );
  };

  // Sets whether or not we are at the edge of a platform, and if so, which side.
  // 1 = right, and -1 = left. Also snaps player to the ground.
  auto setGround_Edge = [&m, EPSILON, this]( int e ) {
    Vec2f &c = e == 1 ? ground.c : ground.d;
    Vec2f &d = e == 1 ? ground.d : ground.c;
    ledgeState = e == 1 ? LEDGE_RIGHT : LEDGE_LEFT;

    ground.right = Vec2f(1,0);
    ground.ledge = true;

    c = d;
    d = ground.d + ground.right * (m.halfWidth + EPSILON) * e;

    owner->setY( c[1] - EPSILON );
  };

  auto moveHoriz = [&delta, EPSILON, this](float spd, float acc, float dec) {
    if (disabledTime > 0.f) return;
    if (movement > EPSILON)
      velocity[0] = std::min( spd * movement, velocity[0] + acc * delta );
    else if (movement < -EPSILON)
      velocity[0] = std::max( spd * movement, velocity[0] - acc * delta );
    else if (fabs(velocity[0]) >= EPSILON) {
      float dir = velocity[0] / fabs(velocity[0]);
      velocity[0] *= dir;
      velocity[0] = std::max(velocity[0] - dec * delta, 0.f);
      velocity[0] *= dir;
    }
    else velocity[0] = 0.f;
  };

  auto edgeAdjustedCorner = [&m, this](const Vec2f &normal) {
    return Vec2f( m.halfWidth,
		     (m.halfWidth / normal[0]) * normal[1] );
  };

  // Returns a list of bounding box who's bottom two corners are adjusted for the slope,
  // since the actor stands on slops using the center point instead of box edges. This
  // is used for detecting wall collisions while moving along the ground.
  auto edgeAdjustedCorners = [&edgeAdjustedCorner, &m, EPSILON, this]() {
    Vec2f edgy = edgeAdjustedCorner(ground.right);
    return std::vector<Vec2f>({ m.boxPts[0], -edgy + UP_VECTOR * EPSILON, edgy + UP_VECTOR * EPSILON, m.boxPts[3] });
  };

  if (disabledTime > 0.f) disabledTime -= delta;
  
  /*
   *  - Handle air state -
   *
   * Horizontal and vertical velocity affect world position linearly, while sliding
   * against wall collisions and detecting potential ground to switch to the ground state
   *
   */
  if (state == STATE_AIR) {
    visibleState = STATE_AIR;
    ledgeState = LEDGE_NONE;
    
    // In the case of multiple collisions, keep track of these values
    Vec2f lastDir;
    float lastT = 1.f;

    // Reset sprite in case it is fake falling and we interrupted it by
    // going into the air.
    if (spriteFakeState == SPRITE_FALL) {
      spriteFakeState = SPRITE_NONE;
      spriteOffsetVelocity = 0.f;
      owner->setOffsetY(0.f);
    } else if (spriteFakeState == SPRITE_JUMP) {
      owner->setOffsetY( owner->getOffset()[1] + spriteOffsetVelocity );
      if (owner->getOffset()[1] < 0.f) {
	owner->setOffsetY( 0.f );
	spriteFakeState = SPRITE_NONE;
      }
    }

    // Move horizontally in the air in the desired direction from input
    moveHoriz( m.airSpeed, m.airAcc, m.airDec );

    // Add gravity
    velocity[1] += PhysicsManager::GRAVITY * m.fallFactor * delta;
    velocity[1] = std::min(velocity[1], FALL_SPEED);

    // Adjust the direction vector to reflect position destination
    // based on velocity
    dir = velocity * delta;

    // In case we slide, we want to move along the wall and then detect
    // another collision.
    for (int collisions = 0, maxcol = 1; collisions < maxcol; collisions++) {

      // If the vel flags were set and we're not moving, no need to do anything else.
      if (!setVelFlags()) return;

      // Do a collision check using the normal bounding box
      PhysicsManager::RayResult result = collisionCheck( m.boxPts );

      // If we have a hit, react to it by sliding or detecting ground
      if (result.hit) {

	// Set the normal and edges of the current "ground" we are touching.
	// In this case, it can also be a wall.
	ground.c = result.c;
	ground.d = result.d;
	ground.right = (ground.d - ground.c).normalize();

	// If we are touching ground that can be stood on...
	if (velocity[1] > 0 && result.normal[1] < -MAX_SLOPE) {

	  // Case a ray downward to detect the slope
	  // The reason why I use m.halfWidth is because right now the max
	  // slope is a strict 45 degree angle.
	  // FIX THIS EVENTUALLY
	  Vec2f slopeDir = Vec2f(0, m.halfWidth + EPSILON * 2);
	  Vec2f base = pos + dir * result.t - dir.normalize() * EPSILON;
	  PhysicsManager::RayResult slope =
	    physics.rayCast(PhysicsManager::MASK_WORLD, base, base + slopeDir );

	  // If we have a slope and it is standable, snap player to the ground
	  if (slope.hit && slope.normal[1] < -MAX_SLOPE) {
	    ground.c = slope.c;
	    ground.d = slope.d;
	    ground.right = (ground.d - ground.c).normalize();

	    // Illusion of falling softly to the ground instead of snapping
	    spriteFakeState = SPRITE_FALL;
	    spriteOffsetVelocity = velocity[1];
	    velocity[1] = 0;
	    owner->setOffsetY( pos[1] );
	    //owner->setPosition( Vec2f( pos[0], ground.c[1] + ((base[0] - ground.c[0]) / (ground.d[0] - ground.c[0])) * (ground.d[1] - ground.c[1]) - EPSILON ) );
	    owner->setPosition( base + slopeDir * slope.t - slopeDir.normalize() * EPSILON  );//+ UP_VECTOR * EPSILON );
	    owner->setOffsetY( owner->getOffset()[1] - pos[1] );
	    ground.ledge = false;
	    state = STATE_GROUND;

	    // Do another check along the ground to make sure we are not inside any sloped walls
	    // I.e., if we slide down a steep slope and land onto a slope, we snap straight down
	    // onto the ground slope, which WILL get us stuck inside the wall.
	    // TO-DO: Only cast the needed side
	    // I don't really like this because it still leaves potential to get stuck, say, if there
	    // was a sloped wall on the other side close enough. But that kind of situation is undefined
	    // and I will never be stupid enough to put that kind of design in my levels, so I'm not
	    // too worried.
	    if (slope.normal[1] > -1.f) {
	      auto slopedWallCheck = [&](float factor) {
		slopeDir = edgeAdjustedCorner( ground.right ) * factor;
		slope = physics.rayCast( PhysicsManager::MASK_WORLD, pos, pos + slopeDir );
		if (slope.hit && slope.normal[1] > -MAX_SLOPE) {
		  Vec2f dest = pos + slopeDir * slope.t;
		  owner->setPosition( dest - slopeDir - slopeDir.normalize() * EPSILON );
		}
	      };
	      slopedWallCheck(1.f);
	      slopedWallCheck(-1.f);
	    }
	    return;
	  }
	  // If we did not detect a slope, we are standing on the edge of a platform.
	  // Snap player to ground appropriately.
	  else {

	    wallState = result.normal[0] < 0 ? WALL_RIGHT : WALL_LEFT;
	    
	    if (result.corner) {
	      state = STATE_GROUND;
	      visibleState = STATE_GROUND;
	      ground.ledge = true;

	      // Don't let the ground plane extend too far if there is a walkable slope on
	      // one side of the corner
	      //if (result.cornerData[3][1] < -MAX_SLOPE)
	      //ground.c = result.cornerData[0];
	      //else if (result.cornerData[4][1] < -MAX_SLOPE)
	      //ground.d = result.cornerData[0];

	      owner->setPosition( Vec2f( pos[0], ground.c[1] - EPSILON ) );
	      return;
	    }
	    if (base[0] >= result.d[0]) {
	      //owner->setX(base[0]);
	      setGround_Edge(1);
	      state = STATE_GROUND;
	      visibleState = STATE_GROUND;
	      return;
	    }
	    else if (base[0] <= result.c[0]) {
	      //owner->setX(base[0]);
	      setGround_Edge(-1);
	      state = STATE_GROUND;
	      visibleState = STATE_GROUND;
	      return;
	    }
	  }
	}

	// Detect acute corners
	if (lastT < 1.f) {
	  // Snap to the closest corner edge
	  if (lastT > result.t)
	    dir = PhysicsManager::rayStop( lastDir, -lastDir.normalize(), lastT );
	  else
	    dir = PhysicsManager::rayStop( dir, -dir.normalize(), result.t );

	  if (velocity[1] < 0 && result.normal[1] > MAX_SLOPE) {
	    //velocity[1] = 0;
	  }

	  // Stop trying to move so we don't get stuck potentially
	  //if (velocity[1] < 0.f)
	  //velocity = Vec2f(0,0);

	  // Do no more collisions and do not slide after this, since we are not stuck
	  break;
	}

	// For potential acute corner detection next collision loop.
	lastDir = dir;
	lastT = result.t;

	// Make sure to bump head on flat-ish ceilings. If we bump head, don't slide
	if (velocity[1] < 0 && result.normal[1] > MAX_SLOPE) {
	  velocity[1] = 0;
	  dir = PhysicsManager::rayStop( dir, -dir.normalize(), result.t );
	}
	// Adjust direction vector to slide against surface.
	else {
	  dir = PhysicsManager::raySlide( dir, result.normal, result.t );
	  
	  //velocity[0] += result.normal[0] * gravity * delta;

	  //if (result.normal[1] > 0.f) {
	  velocity[1] += fabs(result.normal[1]) * PhysicsManager::GRAVITY * delta;

	  //if (result.normal[1] < MAX_SLOPE)
	  //  velocity[0] = result.normal[0];//result.normal[0] * PhysicsManager::GRAVITY * m.fallFactor * delta;
	    //}

	  // Only do another collision chech if we slide. Also don't do too many
	  // collision checks since they are expensive.
	  // TEMP: For now, let me know when this happens.
	  if (++maxcol > 4) throw std::string("TOO MANY COLLISIONS");
	}
      }
    }
  }

  /*
   *  - Handle ground state -
   *
   * Horizontal velocity means we are moving along the ground.
   * Vertical velocity can only be less or equal to 0. Less means jump.
   *
   */
  else if (state == STATE_GROUND) {

    // Handle sprite offset to fake-fall smoothly in case snapped to a slope while landing
    if (spriteFakeState == SPRITE_FALL) {
      owner->setOffsetY( owner->getOffset()[1] + spriteOffsetVelocity * delta/* - dir[1]*/ );
      if (owner->getOffset()[1] > 0.f) {
	spriteFakeState = SPRITE_NONE;
	owner->setOffsetY(0.f);
      }
      spriteOffsetVelocity += PhysicsManager::GRAVITY * m.fallFactor * delta;
    } else if (spriteFakeState == SPRITE_JUMP) {
      spriteFakeState = SPRITE_NONE;
      owner->setOffsetY(0.f);
    }

    // Update the state that everyone should see
    visibleState = spriteFakeState == SPRITE_NONE ? STATE_GROUND : STATE_AIR;

    // Attempt to move along the ground in the desired direction
    moveHoriz( m.groundSpeed, m.groundAcc, m.groundDec );

    // Make sure to stop downward velocity
    velocity[1] = std::min(velocity[1], 0.f);

    // Convert horizontal velocity to move along the ground
    dir = ground.right * velocity[0] * delta;
    Vec2f targetPos = pos + dir;

    // See if we were launched from the ground
    if (velocity[1] < 0.f) {
      // Since we sink into the ground a little bit for slopes, make sure to
      // try to teleport upwards a little bit. If we can't, jump is a NO-GO
      // The reason why we do this is because while sinking into the ground, if
      // the box is sliding up against a sloped wall, the bottom corner might be
      // sticking into it.
      Vec2f slopeDist = UP_VECTOR * (fabs((ground.right*(m.halfWidth/ground.right[0]))[1]) + EPSILON);
      PhysicsManager::RayResult headbump =
	physics.multiPlaneCast( PhysicsManager::MASK_WORLD, slopeDist, { pos + m.boxPts[0], pos + m.boxPts[3] } );
      
      // If there is no headbump, we are free to jump.
      // TO-DO: while standing on a slope, jumping into a slide-able slope, we currently just
      //  bump our head instead of sliding up the wall. That needs fixing, but also needs to make
      //  sure we do not descend while one of the bottom corners are still embedded in the slope
      if (!headbump.hit) {
	// Adjust owner's vertical position based on slope
	owner->setPosition( pos + slopeDist );

	// Switch the physics to the jump state and add upwards velocity
	state = STATE_AIR;

	// Adjust the sprite's offset to make jumping off a steep slope look more smooth
	spriteFakeState = SPRITE_JUMP;
	spriteOffsetVelocity = -m.halfWidth * .25f;
	owner->setOffsetY( -slopeDist[1] );

	// Instead of waiting until the next update cycle, go ahead and update as if we were in the air.
	update(delta);
	return;
      }
      // A little illusory magic here. If we do happen to bump our heads on the ceiling, make
      // the sprite fake jump+fall back to the slope
      else if (spriteFakeState == SPRITE_NONE) {
	spriteFakeState = SPRITE_FALL;
	owner->setOffsetY( owner->getOffset()[1] +
				   std::max(velocity[1] * delta, slopeDist[1] * headbump.t) );
	velocity[1] = 0.f; // We didn't actually move...
	spriteOffsetVelocity = 0.f;
      }
    }

    // See if there are any walls between us and the turn of a potential hill
    if (!setVelFlags()) return;
    PhysicsManager::RayResult collision = collisionCheck( edgeAdjustedCorners() );
    if ( collision.hit && collision.normal[1] > -MAX_SLOPE ) {
      wallState = collision.normal[0] < 0 ? WALL_RIGHT : WALL_LEFT;
      // Influence direction and target position. In case target position moves
      // past the edge, we will still detect to see if there is a new
      // walkable edge underneath.
      // TO-DO: set the position so that the ACTUAL box corner is at the edge
      // of the slope
      dir = PhysicsManager::rayStop( dir, -dir.normalize(), collision.t );
      targetPos = pos + dir;
      velocity[0] = 0.f;
    } else {

      // Now we detect the potential turn of a hill by seeing how far we walked past the
      // current ground edge's vertices (if we did at all). Snap to the midpoint between
      // the two slopes if we did, and try to move along the new hill.

      // The edge we are approaching if we are reaching the end of a line.
      // 1 = right side, -1 = left side.
      int newEdge = (targetPos[0] >= ground.d[0] - EPSILON) - (targetPos[0] <= ground.c[0] + EPSILON);
      // Now handle edges if we are indeed approaching one
      if (newEdge != 0) {
	const Vec2f &edgePoint = (newEdge == 1 ? ground.d : ground.c);

	// Since we are going to travel to the ledge of the edge, keep track of the leftover
	// distance we should travel once (if) we find new ground.
	float leftover = (targetPos - edgePoint).length();
	leftover = std::max(EPSILON, leftover - EPSILON);

	// Find out where we are in relation to the edge point
	targetPos = edgePoint + UP_VECTOR * EPSILON;

	//owner->setPosition(targetPos);
      
	// Update position to top of hill. We know it is safe based on previous
	// collision results.
	

	// Detect what the next ground might be if we continue walking off the edge
	PhysicsManager::RayResult next_ground =
	  physics.rayCast( PhysicsManager::MASK_WORLD,
			   targetPos + UP_VECTOR * (m.halfWidth + EPSILON) + ground.right * newEdge * EPSILON,
			   targetPos + DOWN_VECTOR * (m.halfWidth + EPSILON) + ground.right * newEdge * EPSILON );
      
	// If we detect new walkable ground, do not treat this ground's edge as a ledge, but instead
	// snap to the new ground
	if (next_ground.hit && next_ground.normal[1] < -MAX_SLOPE) {
	  ground.c = next_ground.c;
	  ground.d = next_ground.d;
	  ground.right = (ground.d - ground.c).normalize();
	  ground.ledge = false;
	  ledgeState = LEDGE_NONE;
	}
	// Otherwise, we are on a ledge
	else {
	  // Snap to the ledge if we were not previously on a ledge
	  if (!ground.ledge) setGround_Edge( newEdge );
	  // But if we were on a ledge and we walk past the edge of the ledge, we are now falling
	  else state = STATE_AIR;
	}
	// Try to move along the next ground a little bit to keep things smooth. We don't
	// want an awkward stop in movement when the hill changes shape.
	targetPos += ground.right * leftover * newEdge;

	// Recalculate direction vector based on new slope, and do another collision check to make
	// sure nothing gets in the way.
	dir = targetPos - pos;
	if (!setVelFlags()) return;
	collision = collisionCheck( edgeAdjustedCorners() );
	if ( collision.hit && collision.normal[1] > -MAX_SLOPE ) {
	  wallState = collision.normal[0] < 0 ? WALL_RIGHT : WALL_LEFT;
	  dir = PhysicsManager::rayStop( dir, -dir.normalize(), collision.t );
	  velocity[0] = 0.f;
	}
      }
    }
  }
  
  // Adjust final position
  owner->setPosition( pos + dir );

  // Just a little housework here to keep the player snapped to the ground. There is a chance
  // if the player is moving slow ehough. I determined this is necessary because 1) I was getting
  // this problem and 2) this is not so much of a bandaid solution because if we used
  // numeric_limits<float>::min() as an epsilon it would solve this problem but cause other
  // collision problems because of tiny calculations that are beyond the float's accuracy
  if (state == STATE_GROUND)
    owner->setY( ground.c[1] +
		 ((pos[0] - ground.c[0]) / (ground.d[0] - ground.c[0])) *
		 (ground.d[1] - ground.c[1]) -
		 EPSILON );
}
