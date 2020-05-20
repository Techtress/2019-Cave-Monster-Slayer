#include <chrono>

typedef std::chrono::high_resolution_clock chrono_clock;

class Engine;

class Clock
{
public:
  static Clock& getInstance();
  unsigned int getTicks() const;

  double getFPS() const { return 1.0/delta; }

  // time (in seconds) since last frame
  double getDelta() const { return delta; }

  Clock(const Clock&) = delete;
  Clock&operator=(const Clock&) = delete;

private:
  friend class Engine;

  std::chrono::time_point<chrono_clock> chrono;
  double delta;

  void updateDelta();
  void incrementTime();

  Clock();
};
