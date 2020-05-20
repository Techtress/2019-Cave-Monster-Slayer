#include "soundset.h"
#include "soundmanager.h"
#include "xmltag.h"

#include <algorithm>

SoundSet::SoundSet() :
  sounds(),
  soundInterval(-1.f) {}

SoundSet::~SoundSet() {}

SoundSet &SoundSet::operator=(const XMLTag& tag)
{
  soundInterval = tag["interval"].toFloat();
  std::for_each( tag.getChildren().begin()+1, tag.getChildren().end(), [this](const XMLTag *pt) {
      sounds.push_back( SoundManager::getInstance().getSound(pt->toStr()) ); });
  return *this;
}

int SoundSet::playSound(int id) const
{
  return sounds[id]->play(0);
}
