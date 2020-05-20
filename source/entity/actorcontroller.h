#ifndef ACTORCONTROLLER_H
#define ACTORCONTROLLER_H

class Actor;

class ActorController
{
 public:
  ActorController(Actor*o) : owner(o) {}
  virtual ~ActorController() {}
  virtual void initiate() = 0;
  virtual void update(float delta) = 0;
  ActorController(const ActorController&) = delete;
  ActorController &operator=(const ActorController&) = delete;

  Actor *getOwner() { return owner; }
  
 protected:
  void setOwner(Actor *o) { owner = o; }
  
 private:
  Actor *owner;
};

#endif
