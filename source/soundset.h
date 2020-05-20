#ifndef SOUNDSET_H
#define SOUNDSET_H

#include <vector>
#include <cstdlib>

class Sound;
class XMLTag;

class SoundSet
{
 public:
  SoundSet();
  ~SoundSet();
  
  int playSound(int id) const;
  int playRandomSound() const { return playSound(randomSoundID()); }
  float getSoundInterval() const { return soundInterval; }
  int randomSoundID() const { return rand()%sounds.size(); }

  int count() const { return sounds.size(); }
  bool empty() const { return count() == 0; }

  // This is mainly how it's defined, since not all animations/states that contain soundset play sounds
  SoundSet &operator=(const XMLTag&);

  SoundSet(const SoundSet&) = delete;
  SoundSet &operator=(const SoundSet&) = delete;
  SoundSet(SoundSet&& rhs) : sounds(std::move(rhs.sounds)), soundInterval(rhs.soundInterval) {}

 private:
  std::vector<const Sound*> sounds;
  float soundInterval; // -1 means it plays once, anything after that
};

#endif
