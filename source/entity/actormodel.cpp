#include "actormodel.h"
#include "aibehavior.h"

ActorModel::ActorModel(const XMLTag& tag) :
  EntityModel(),
  animSet(tag["animset"]),
  physics(tag["actorphysics"]),
  aiBehavior(nullptr),
  attributes(tag["attributes"]),
  attackSet(),
  deathSound()
{
  if (tag.hasChild("behavior")) {
    aiBehavior = new AIBehavior(tag["behavior"]);
  }

  if (tag.hasChild("attackSet")) {
    const XMLTag &as = tag["attackSet"];
    std::for_each(as.getChildren().begin(), as.getChildren().end(), [this](const XMLTag* t) {
	attackSet.emplace_back(*t);} );
  }

  if (tag["death"].hasChild("soundSet"))
    deathSound = tag["death"]["soundSet"];
}

ActorModel::~ActorModel()
{
  if (aiBehavior != nullptr) delete aiBehavior;
}

ActorModel::Attack::Attack(const XMLTag& tag) :
  hitBox(tag["hitBox"]),
  animation(tag["animation"].toStr()),
  hitDelay(tag["hitDelay"].toFloat()) {}

ActorAttributes::ActorAttributes(const XMLTag &tag) : health(tag["health"].toFloat()), super(tag["super"].toBool()) {}
