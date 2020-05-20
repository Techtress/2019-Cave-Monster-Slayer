#include "aibehavior.h"
#include "../xmltag.h"
#include "../stringutil.h"

AIBehavior::AIBehavior(const XMLTag& tag) :
  sight(tag["sight"].toFloat()),
  idleState(tag["state_idle"]),
  patrolState(tag["state_patrol"]),
  chaseState(tag["state_chase"]) {}

AIBehavior::~AIBehavior() {}

AIBehavior::IdleState::IdleState( const XMLTag& tag ) :
  timeStart(0), timeRange(0)
{
  const std::string &time = tag["time"].toStr();
  int middle = time.find(":");
  timeStart = StringUtil::to<float>(time.substr( 0, middle ));
  timeRange = StringUtil::to<float>(time.substr( middle+1, time.length()-(middle+1) ));
}

AIBehavior::PatrolState::PatrolState( const XMLTag& tag ) :
  IdleState(tag),
  range(tag["range"].toFloat()) {}

AIBehavior::ChaseState::ChaseState( const XMLTag& tag ) :
  attackDist(tag["attackDist"].toFloat()),
  attackID(tag["attackID"].toStr()),
  soundSet(),
  attackIntervalS(0.f),
  attackIntervalR(0.f)
{
  if (tag.hasChild("soundSet"))
    soundSet = tag["soundSet"];

  const std::string &interval = tag["attackInterval"].toStr();
  int middle = interval.find(":");
  attackIntervalS = StringUtil::to<float>(interval.substr( 0, middle ));
  attackIntervalR = StringUtil::to<float>(interval.substr( middle+1, interval.length()-(middle+1) ));
}
