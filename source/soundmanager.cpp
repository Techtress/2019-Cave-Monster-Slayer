#include "soundmanager.h"
#include "gameconfig.h"

#include <SDL.h>
#include <iostream>

Sound::Sound(Mix_Chunk *c) : chunk(c) {}

Sound::~Sound()
{
  Mix_FreeChunk(chunk);
}

int Sound::play(int times) const
{
  return Mix_PlayChannel(-1, chunk, times);
}

SoundManager &SoundManager::getInstance()
{
  static SoundManager instance;
  return instance;
}

SoundManager::~SoundManager() {
  std::cout << "Cleaning up sounds ..." << std::endl;
  Mix_HaltMusic();
  Mix_FreeMusic(music);
  for (auto &it : sounds) delete it.second;
  Mix_CloseAudio();
}

SoundManager::SoundManager() : 
  volume(SDL_MIX_MAXVOLUME/10), 
  music(nullptr),
  audioRate(44100), 
  audioChannels( 2 ), 
  audioBuffers( 1024 ),
  sounds()
{
  if(Mix_OpenAudio(audioRate, MIX_DEFAULT_FORMAT, audioChannels, 
                   audioBuffers)){
    throw std::string("Unable to open audio!");
  }

  Mix_AllocateChannels(64);

  // Load music
  if ((music = Mix_LoadMUS("assets/sounds/music.ogg")) == nullptr)
    throw std::string("Enable to load music!");
  
  startMusic();
}

void SoundManager::toggleMusic() {
  if( Mix_PausedMusic() ) { 
    Mix_ResumeMusic(); 
  } 
  else { 
    Mix_PauseMusic(); 
  } 
}

void SoundManager::startMusic() {
  Mix_VolumeMusic(volume*2);
  Mix_PlayMusic(music, -1);
}

void SoundManager::stopMusic() {
  Mix_HaltMusic();
  Mix_FreeMusic(music);
}

void SoundManager::stopSound(int channel) const
{
  Mix_FadeOutChannel(channel, 200);
}

const Sound *SoundManager::getSound(const std::string &name)
{
  auto result = sounds.find(name);
  if (result == sounds.end()) {
    std::string file = "assets/sounds/" + name;
    std::cout << "SoundManager: loading sound '" + file + "'" << std::endl;
    return sounds[name] = new Sound( Mix_LoadWAV((file).c_str()) );
  }
  else return result->second;
}
