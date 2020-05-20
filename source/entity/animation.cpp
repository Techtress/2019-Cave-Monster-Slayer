#include "animation.h"

#include "../xmltag.h"
#include "../image.h"
#include "../soundmanager.h"

#include <iostream>
#include <sstream>
#include <algorithm>

Animation::Animation(const XMLTag &tag, const Image* img) :
  image(img),
  frames(),
  speed(tag["speed"].toFloat()),
  loop(tag["loop"].toBool()),
  soundSet()
{
  if (tag.hasChild("soundSet"))
    soundSet = tag["soundSet"];
  
  std::for_each( tag.getChildren().begin() + 3, tag.getChildren().end(), [this](const XMLTag *pt) {
    const XMLTag &t = *pt;

    // Only parse the right stuff
    if (t.getName() != "direction") return;

    // Determine direction
    unsigned dirID = DIR_RIGHT;
    if (t["id"].toStr() == "left") dirID = DIR_LEFT;

    // Now load frames
    const std::string &fstr = t["frames"].toStr();
    size_t start = 0, end;
    do {
      end = std::min(fstr.find(",", start), fstr.find(":", start));

      unsigned nextFrame;
      std::stringstream(fstr.substr(start, end-start)) >> nextFrame;

      if (start == 0) {
	frames[dirID].push_back(nextFrame);
	//std::cout << "  Adding dir='" << dirID << "' frame='" << nextFrame << "'" << std::endl;
      }
      else {
	if (fstr[start-1] == ',') {
	  frames[dirID].push_back(nextFrame);
	  //std::cout << "  Adding dir='" << dirID << "' frame='" << frames[dirID].back() << "'" << std::endl;
	}
	else if (fstr[start-1] == ':') {
	  while (nextFrame-frames[dirID].back() > 0) {
	    frames[dirID].push_back(frames[dirID].back()+1);
	    //std::cout << "  Adding dir='" << dirID << "' frame='" << frames[dirID].back() << "'" << std::endl;
	  }
	}
      }

      start = end+1;
    } while (end < fstr.length());
  } );
}

unsigned Animation::getImageFrame(Direction dir, unsigned animFrame) const
{
  return frames.at(dir).at(animFrame);
}
