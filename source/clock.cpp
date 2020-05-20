//#include <cmath>
//#include <iostream>
//#include <string>
//#include <sstream>
#include "clock.h"
//#include "gameData.h"
//#include "ioMod.h"

Clock& Clock::getInstance() {
  static Clock clock; 
  return clock;
}

Clock::Clock() :
  chrono(chrono_clock::now()),
  delta(0.f)
{}

void Clock::updateDelta() {
  delta = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(chrono_clock::now() - chrono).count() / 1000000000;
}

void Clock::incrementTime() {
  chrono = chrono_clock::now();
}
