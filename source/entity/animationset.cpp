#include "animationset.h"

#include "../xmltag.h"
#include "../imagefactory.h"
#include "../image.h"
#include "../stringutil.h"

#include <sstream>

#include <iostream>

AnimationSet::AnimationSet(const XMLTag &tag) :
  animations()
{
  std::unordered_map<unsigned, Image*> images;
  
  for (const XMLTag *pt : tag.getChildren()) {
    const XMLTag &t = *pt;
    if (t.getName().substr(0,5) == "image") {
      images[StringUtil::to<int>(t.getName().substr(5,2))] = ImageFactory::getInstance().getImage(t["name"].toStr());;
    }
    else if (t.getName() == "anim") {
      //std::cout << "Found anim '" << t["name"].toStr() << "'" << std::endl;
      int imgID = t.hasChild("image") ? t["image"].toInt() : 0;
      animations.emplace( std::piecewise_construct,
			  std::forward_as_tuple(t["name"].toStr()),
			  std::forward_as_tuple(t, images.at(imgID)) );
    }
  }
}

const Animation &AnimationSet::getAnimation(const std::string &anim) const
{
  return animations.at(anim);
}
