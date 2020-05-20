#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <SDL_mixer.h>
#include <string>
#include <unordered_map>

class Sound {
 public:
  Sound(Mix_Chunk*);
  ~Sound();

  // Returns channel id for SoundManager to stop
  int play(int times) const;

  Sound(const Sound&) = delete;
  Sound &operator=(const Sound&) = delete;  
    
 private:
  Mix_Chunk *chunk;
};

class SoundManager {
public:
  SoundManager();
  ~SoundManager();

  static SoundManager &getInstance();

  void startMusic();
  void stopMusic();      // stop all sounds
  void toggleMusic();    // toggle music on/off

  void stopSound(int channel) const;
  
  const Sound *getSound(const std::string &name);

  SoundManager(const SoundManager&) = delete;
  SoundManager &operator=(const SoundManager&) = delete;

 private:
  int volume;
  Mix_Music *music;

  int audioRate;
  int audioChannels;
  int audioBuffers;
  
  std::unordered_map<std::string, Sound*> sounds;
};

#endif
