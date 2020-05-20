#ifndef ENTITYMODEL_H
#define ENTITYMODEL_H

class EntityModel
{
 public:
  EntityModel() {}
  virtual ~EntityModel() {}

  EntityModel(const EntityModel&) = delete;
  EntityModel &operator=(const EntityModel&) = delete;
};

#endif
