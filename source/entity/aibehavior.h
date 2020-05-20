#ifndef AIBEHAVIOR_H
#define AIBEHAVIOR_H

#include "../soundset.h"

#include <string>

class AIController;
class XMLTag;

class AIBehavior
{
 public:
  AIBehavior(const XMLTag&);
  ~AIBehavior();

  struct IdleState
  {
    IdleState(const XMLTag&);
    float timeStart, timeRange;
  };

  struct PatrolState : IdleState
  {
    PatrolState(const XMLTag&);
    float range; // the amount they can walk from their origin point
  };

  struct ChaseState
  {
    ChaseState(const XMLTag&);
    float attackDist;
    std::string attackID;
    SoundSet soundSet;
    float attackIntervalS, attackIntervalR;
  };

  float              getSight() const { return sight; }
  const IdleState   &getIdleState() const { return idleState; }
  const PatrolState &getPatrolState() const { return patrolState; }
  const ChaseState  &getChaseState() const { return chaseState; }

  AIBehavior(const AIBehavior&) = delete;
  AIBehavior &operator=(const AIBehavior&) = delete;

 private:
  float sight;
  IdleState idleState;
  PatrolState patrolState;
  ChaseState chaseState;
};

#endif
